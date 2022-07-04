#pragma once
#include "juce_core/juce_core.h"

namespace makeRange
{
	using Range = juce::NormalisableRange<float>;

	inline Range biased(float start, float end, float bias/*[-1, 1]*/) noexcept
	{
		// https://www.desmos.com/calculator/ps8q8gftcr
		const auto a = bias * .5f + .5f;
		const auto a2 = 2.f * a;
		const auto aM = 1.f - a;
		
		const auto r = end - start;
		const auto aR = r * a;
		if (bias != 0.f)
			return
		{
				start, end,
				[a2, aM, aR](float min, float, float x)
				{
					const auto denom = aM - x + a2 * x;
					if (denom == 0.f)
						return min;
					return min + aR * x / denom;
				},
				[a2, aM, aR](float min, float, float x)
				{
					const auto denom = a2 * min + aR - a2 * x - min + x;
					if (denom == 0.f)
						return 0.f;
					return aM * (x - min) / denom;
				},
				[](float min, float max, float x)
				{
					return x < min ? min : x > max ? max : x;
				}
		};
		else return { start, end };
	}

	inline Range toggle() noexcept
	{
		return
		{
			0.f, 1.f,
			[](float, float, float x)
			{
				return x;
			},
			[](float, float, float x)
			{
				return x;
			},
			[](float, float, float x)
			{
				return x > .5 ? 1.f : 0.f;
			}
		};
	}

	inline Range stepped(float start, float end, float steps = 1.f) noexcept
	{
		return
		{
				start, end,
				[range = end - start](float min, float, float normalized)
				{
					return min + normalized * range;
				},
				[rangeInv = 1.f / (end - start)](float min, float, float denormalized)
				{
					return (denormalized - min) * rangeInv;
				},
				[steps, stepsInv = 1.f / steps](float min, float max, float val)
				{
					return juce::jlimit(min, max, std::rint(val * stepsInv) * steps);
				}
		};
	}

	// advanced one(s):

	inline Range withCentre(float start, float end, float centre) noexcept
	{
		const auto r = end - start;
		const auto v = (centre - start) / r;

		return makeRange::biased(start, end, 2.f * v - 1.f);
	}

	// probably deprecated
	/*
	inline Range withCentre(float start, float end, float centre, float tolerance = .0001f) noexcept
	{
		auto b = 0.f;
		auto bInc = 1.f;
		auto range = biased(start, end, b);
		auto nVal = range.convertFrom0to1(.5f);
		auto dist = nVal - centre;
		auto dif = std::abs(dist);
		if (dif < tolerance)
			return range;
		do
		{
			bInc *= .5f;
			b += dist > 0.f ? -bInc : bInc;
			
			range = biased(start, end, b);
			nVal = range.convertFrom0to1(.5f);
			dist = nVal - centre;
			dif = std::abs(dist);

		} while (dif > tolerance);

		return range;
	}
	*/

}