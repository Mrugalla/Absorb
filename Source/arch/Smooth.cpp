#include "Smooth.h"
#include <cmath>

template<typename Float>
void smooth::Smooth<Float>::makeFromDecayInSamples(Float d) noexcept
{
	setX(std::exp(static_cast<Float>(-1) / d));
}

template<typename Float>
void smooth::Smooth<Float>::makeFromDecayInSecs(Float d, Float Fs) noexcept
{
	makeFromDecayInSamples(d * Fs);
}

template<typename Float>
void smooth::Smooth<Float>::makeFromDecayInFc(Float fc) noexcept
{
	setX(std::exp(-Tau * fc));
}

template<typename Float>
void smooth::Smooth<Float>::makeFromDecayInHz(Float d, Float Fs) noexcept
{
	makeFromDecayInFc(d / Fs);
}

template<typename Float>
void smooth::Smooth<Float>::makeFromDecayInMs(Float d, Float Fs) noexcept
{
	makeFromDecayInSamples(d * Fs * static_cast<Float>(.001));
}

template<typename Float>
void smooth::Smooth<Float>::copyCutoffFrom(const smooth::Smooth<Float>& other) noexcept
{
	a0 = other.a0;
	b1 = other.b1;
	eps = other.eps;
}

template<typename Float>
smooth::Smooth<Float>::Smooth(const Float _startVal) :
	a0(static_cast<Float>(1)),
	b1(static_cast<Float>(0)),
	y1(_startVal),
	eps(static_cast<Float>(0)),
	startVal(_startVal)
{}

template<typename Float>
void smooth::Smooth<Float>::reset()
{
	a0 = static_cast<Float>(1);
	b1 = static_cast<Float>(0);
	y1 = startVal;
	eps = static_cast<Float>(0);
}

template<typename Float>
void smooth::Smooth<Float>::operator()(Float* buffer, Float val, int numSamples) noexcept
{
	for (auto s = 0; s < numSamples; ++s)
		buffer[s] = processSample(val);
}

template<typename Float>
void smooth::Smooth<Float>::operator()(Float* buffer, int numSamples) noexcept
{
	for (auto s = 0; s < numSamples; ++s)
		buffer[s] = processSample(buffer[s]);
}

template<typename Float>
Float smooth::Smooth<Float>::operator()(Float sample) noexcept
{
	return processSample(sample);
}

template<typename Float>
Float smooth::Smooth<Float>::processSample(Float x0) noexcept
{
	y1 = x0 * a0 + y1 * b1;
	return y1;
}

template<typename Float>
void smooth::Smooth<Float>::setX(Float x) noexcept
{
	a0 = static_cast<Float>(1) - x;
	b1 = x;
	eps = a0 * static_cast<Float>(1.5);
}

template struct smooth::Smooth<float>;
template struct smooth::Smooth<double>;