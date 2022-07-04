#pragma once

namespace smooth
{
	template<typename Float>
	struct Smooth
	{
		static constexpr Float Pi = static_cast<Float>(3.14159265359);
		static constexpr Float Tau = Pi * static_cast<Float>(2);

		void makeFromDecayInSamples(Float) noexcept;
		void makeFromDecayInSecs(Float, Float/*Fs*/) noexcept;
		void makeFromDecayInFc(Float) noexcept;
		void makeFromDecayInHz(Float, Float/*Fs*/) noexcept;
		void makeFromDecayInMs(Float, Float/*Fs*/) noexcept;

		void copyCutoffFrom(const Smooth<Float>&) noexcept;

		Smooth(const Float /*_startVal*/ = static_cast<Float>(0));

		void reset();

		void operator()(Float*, Float/*val*/, int/*numSamples*/) noexcept;
		void operator()(Float*, int/*numSamples*/) noexcept;
		Float operator()(Float) noexcept;

	protected:
		Float a0, b1, y1, eps, startVal;

		Float processSample(Float) noexcept;

		void setX(Float) noexcept;
	};

}