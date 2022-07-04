#include "MidSide.h"
#include "AudioUtils.h"

namespace audio
{
	void encodeMS(float** samples, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto mid = samples[0][s] + samples[1][s];
			const auto side = samples[0][s] - samples[1][s];

			samples[0][s] = mid;
			samples[1][s] = side;
		}

		for (auto ch = 0; ch < 2; ++ch)
			SIMD::multiply(samples[ch], .5f, numSamples);
	}

	void decodeMS(float** samples, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto left = samples[0][s] + samples[1][s];
			const auto right = samples[0][s] - samples[1][s];

			samples[0][s] = left;
			samples[1][s] = right;
		}
	}
}