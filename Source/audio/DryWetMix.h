#pragma once
#include "AudioUtils.h"
#include "WHead.h"
#include <array>

namespace audio
{
	class DryWetMix
	{
		struct LatencyCompensation
		{
			LatencyCompensation();

			/*blockSize, latency*/
			void prepare(int, int);

			/*dry,inputSamples,numChannels,numSamples*/
			void operator()(float**, float**, int, int) noexcept;

		protected:
			AudioBuffer ring;
			WHead wHead;
			int latency;
		};

		enum
		{
#if PPDHasGainIn
			GainIn,
#endif
			MixD,
			MixW,
			Gain,
			NumBufs
		};

	public:
		DryWetMix();

		/*sampleRate, blockSize, latency*/
		void prepare(float, int, int);

		/*samples, numChannels, numSamples, gainInP, mixP, gainP, polarityP, unityGainP*/
		void saveDry(
			float**, int, int,
#if PPDHasGainIn
			float,
#endif
			float, float
#if PPDHasPolarity
			, float
#endif
#if PPDHasUnityGain && PPDHasGainIn
			, float
#endif
		) noexcept;

		/*samples, numChannels, numSamples*/
		void processBypass(float**, int, int) noexcept;

		/*samples, numChannels, numSamples*/
		void processOutGain(float**, int, int) const noexcept;

		/*samples, numChannels, numSamples*/
		void processMix(float**, int, int) const noexcept;

	protected:
		LatencyCompensation latencyCompensation;

		std::array<std::vector<float>, NumBufs> bufs;
		
#if PPDHasGainIn
		Smooth gainInSmooth;
#endif
		Smooth mixSmooth, gainSmooth;

		AudioBuffer dryBuf;
	};
}