#pragma once
#include "../arch/Smooth.h"
#include "../param/Param.h"
#include "../arch/State.h"

namespace audio
{
    static constexpr float Tau = 6.28318530718f;
    static constexpr float Pi = 3.14159265359f;
    static constexpr float PiHalf = Pi * .5f;
    static constexpr float PiHalfInv = 1.f / PiHalf;

    using AudioBuffer = juce::AudioBuffer<float>;
    using SIMD = juce::FloatVectorOperations;
    using MIDIBuffer = juce::MidiBuffer;
    using String = juce::String;
    using MIDIIt = juce::MidiBufferIterator;
    using MIDIRef = MIDIIt::reference;
    using Decibels = juce::Decibels;
    using ScopedNoDenormals = juce::ScopedNoDenormals;
    using MidiMessage = juce::MidiMessage;

    using Smooth = smooth::Smooth<float>;
    using PID = param::PID;
    using Params = param::Params;
    using Param = param::Param;
    using State = sta::State;

    template<typename Float>
    inline Float secsInSamples(Float secs, Float Fs) noexcept
    {
        return secs * Fs;
    }

    template<typename Float>
    inline Float msInSamples(Float ms, Float Fs) noexcept
    {
        return secsInSamples(ms * static_cast<Float>(.001), Fs);
    }

    template<typename Float>
    inline Float freqHzInSamples(Float hz, Float Fs) noexcept
    {
        return Fs / hz;
    }

    template<typename Float>
    inline float getRMS(const Float* ar, const int size) noexcept
    {
        auto rms = static_cast<Float>(0);
        for (auto i = 0; i < size; ++i)
            rms += ar[i] * ar[i];
        rms /= static_cast<Float>(size);
        return std::sqrt(rms);
    }

    template<typename Float>
    inline Float noteInFreqHz(Float note, Float rootNote = 69.f, Float xen = 12.f, Float masterTune = 440.f) noexcept
    {
        return std::pow(2.f, (note - rootNote) / xen) * masterTune;
    }

    template<typename Float>
    inline Float noteInFreqHz(Float note, Float rootNote = 69.f, Float masterTune = 440.f) noexcept
    {
        return std::pow(2.f, (note - rootNote) * .08333333333f) * masterTune;
    }

}