#include "DryWetMix.h"

namespace audio
{
	// DryWetMix::LatencyCompensation

	DryWetMix::LatencyCompensation::LatencyCompensation() :
		ring(),
		wHead(),
		latency(0)
	{}

	void DryWetMix::LatencyCompensation::prepare(int blockSize, int _latency)
	{
		latency = _latency;
		if (latency != 0)
		{
			ring.setSize(2, latency, false, true, false);
			wHead.prepare(blockSize, latency);
		}
		else
		{
			ring.setSize(0, 0);
			wHead.prepare(0, 0);
		}
	}

	void DryWetMix::LatencyCompensation::operator()(float** dry, float** inputSamples, int numChannels, int numSamples) noexcept
	{
		if (latency != 0)
		{
			wHead(numSamples);

			for (auto ch = 0; ch < numChannels; ++ch)
			{
				const auto smpls = inputSamples[ch];

				auto rng = ring.getWritePointer(ch);
				auto dr = dry[ch];

				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];
					const auto r = (w + 1) % latency;

					rng[w] = smpls[s];
					dr[s] = rng[r];
				}
			}
		}
		else
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::copy(dry[ch], inputSamples[ch], numSamples);
	}

	// DryWetMix

	DryWetMix::DryWetMix() :
		latencyCompensation(),

		bufs(),

		mixSmooth(1.f),
		gainSmooth(1.f),

		dryBuf()
	{}

	void DryWetMix::prepare(float sampleRate, int blockSize, int latency)
	{
		latencyCompensation.prepare(blockSize, latency);

		mixSmooth.makeFromDecayInMs(20.f, sampleRate);
		gainSmooth.makeFromDecayInMs(20.f, sampleRate);

		dryBuf.setSize(2, blockSize, false, true, false);

		for (auto& buf : bufs)
			buf.resize(blockSize);
	}

	void DryWetMix::saveDry(float** samples, int numChannels, int numSamples,
#if PPDHasGainIn
		float gainInP,
#endif
		float mixP, float gainP
#if PPDHasPolarity
		, float polarityP
#endif
#if PPDHasUnityGain && PPDHasGainIn
		, float unityGainP
#endif
	) noexcept
	{
		latencyCompensation(
			dryBuf.getArrayOfWritePointers(),
			samples,
			numChannels,
			numSamples
		);

#if PPDHasGainIn
		auto gainInBuf = bufs[GainIn].data();
		gainInSmooth(gainInBuf, juce::Decibels::decibelsToGain(gainInP), numSamples);
		for (auto ch = 0; ch < numChannels; ++ch)
			for (auto s = 0; s < numSamples; ++s)
				samples[ch][s] *= gainInBuf[s];
#endif
#if PPDHasUnityGain && PPDHasGainIn
		gainP -= gainInP * unityGainP;
#endif

		auto mixBuf = bufs[MixW].data();
		mixSmooth(mixBuf, mixP, numSamples);

#if PPDHasPolarity
		gainP *= polarityP;
#endif
		gainSmooth(bufs[Gain].data(), juce::Decibels::decibelsToGain(gainP), numSamples);

#if PPDEqualLoudnessMix
		for (auto s = 0; s < numSamples; ++s)
		{
			bufs[MixD][s] = std::sqrt(1.f - mixBuf[s]);
			bufs[MixW][s] = std::sqrt(mixBuf[s]);
		}
#else
		for (auto s = 0; s < numSamples; ++s)
		{
			bufs[MixD][s] = 1.f - mixBuf[s];
			bufs[MixW][s] = mixBuf[s];
		}
#endif
	}

	void DryWetMix::processBypass(float** samples, int numChannels, int numSamples) noexcept
	{
		latencyCompensation(
			dryBuf.getArrayOfWritePointers(),
			samples,
			numChannels,
			numSamples
		);

		for (auto ch = 0; ch < numChannels; ++ch)
		{
			const auto dry = dryBuf.getReadPointer(ch);

			auto smpls = samples[ch];

			for (auto s = 0; s < numSamples; ++s)
				smpls[s] = dry[s];
		}
	}

	void DryWetMix::processOutGain(float** samples, int numChannels, int numSamples) const noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			const auto gainBuf = bufs[Gain].data();
			SIMD::multiply(samples[ch], gainBuf, numSamples);
		}
	}

	void DryWetMix::processMix(float** samples, int numChannels, int numSamples) const noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			const auto dry = dryBuf.getReadPointer(ch);
			const auto mixD = bufs[MixD].data();
			const auto mixW = bufs[MixW].data();

			auto smpls = samples[ch];

			for (auto s = 0; s < numSamples; ++s)
				smpls[s] = dry[s] * mixD[s] + smpls[s] * mixW[s];
		}
	}
}