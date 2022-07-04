#pragma once
#include "AudioUtils.h"
#include "Phasor.h"

namespace audio
{
	struct OscSine
	{
		OscSine() :
			phasor()
		{}

		float operator()() noexcept
		{
			const auto phase = phasor().phase;
			return std::sin(phase * Tau);
		}

		Phasor<float> phasor;
	};
}