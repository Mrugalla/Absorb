#pragma once

namespace audio
{
	template<typename Float>
	struct PhaseInfo
	{
		PhaseInfo(Float _phase, Float _retrig) :
			phase(_phase),
			retrig(_retrig)
		{}

		Float phase;
		bool retrig;
	};

	template<typename Float>
	struct Phasor
	{
		using Phase = PhaseInfo<Float>;

		Phasor(Float _phase = static_cast<Float>(0), Float _inc = static_cast<Float>(0)) :
			phase(_phase, false),
			inc(_inc)
		{

		}

		Phase operator()() noexcept
		{
			phase.phase += inc;
			if (phase.phase >= static_cast<Float>(1))
			{
				--phase.phase;
				phase.retrig = true;
				return phase;
			}
			phase.retrig = false;
			return phase;
		}

		Phase phase;
		float inc;
	};
}