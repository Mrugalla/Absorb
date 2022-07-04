#pragma once
#include <vector>
#include "../arch/Smooth.h"

namespace audio
{
	struct PRM
	{
		/* startVal */
		PRM(float);

		/*Fs, blockSize, smoothLenMs */
		void prepare(float, int, float);

		/* value, numSamples */
		float* operator()(float, int) noexcept;

		/* numSamples */
		float* operator()(int) noexcept;

		/* value */
		float operator()(float) noexcept;

		smooth::Smooth<float> smooth;
		std::vector<float> buf;
	};
}