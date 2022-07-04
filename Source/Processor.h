#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

#include "audio/MIDILearn.h"
#include "audio/ProcessSuspend.h"
#include "audio/DryWetMix.h"
#include "audio/MidSide.h"
#include "audio/Oversampling.h"
#include "audio/Meter.h"
#include "audio/Rectifier.h"
#include "audio/Bitcrusher.h"
#include "audio/NullNoiseSynth.h"

#include "audio/AudioUtils.h"

namespace audio
{
    using MacroProcessor = param::MacroProcessor;
    using Timer = juce::Timer;

    struct ProcessorBackEnd :
        public juce::AudioProcessor,
        public Timer
    {
        using ChannelSet = juce::AudioChannelSet;
        using AppProps = juce::ApplicationProperties;

        ProcessorBackEnd();

        const juce::String getName() const override;
        double getTailLengthSeconds() const override;
        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int) override;
        const juce::String getProgramName(int) override;
        void changeProgramName(int, const juce::String&) override;
        bool isBusesLayoutSupported(const BusesLayout&) const override;
        AppProps* getProps() noexcept;
        bool canAddBus(bool) const override;

        void savePatch();
        void loadPatch();

        bool hasEditor() const override;
        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;

        juce::AudioProcessor::BusesProperties makeBusesProperties();

        /////////////////////////////////////////////
        /////////////////////////////////////////////
        void getStateInformation(juce::MemoryBlock&) override;
        void setStateInformation(const void* /*data*/, int /*sizeInBytes*/) override;

        AppProps props;
        ProcessSuspender sus;

        State state;
        Params params;
        MacroProcessor macroProcessor;
        MIDILearn midiLearn;

        DryWetMix dryWetMix;
#if PPDHasHQ
        Oversampler oversampler;
#endif
        Meters meters;
#if PPDHasSidechain
        AudioBuffer scBuffer;
#endif

        void forcePrepareToPlay();

        void timerCallback() override;

        void processBlockBypassed(AudioBuffer&, juce::MidiBuffer&) override;

    protected:
        AudioBuffer* processBlockStart(AudioBuffer&, juce::MidiBuffer&) noexcept;

        void processBlockEnd(AudioBuffer&) noexcept;
    
private:
#if PPDHasStereoConfig
        bool midSideEnabled;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorBackEnd)
    };

    struct Processor :
        public ProcessorBackEnd
    {
        Processor();

        void prepareToPlay(double, int) override;

        void processBlock(AudioBuffer&, juce::MidiBuffer&);
        
        void processBlockCustom(float** /*samples*/ , int /*numChannels*/, int /*numSamples*/) noexcept;

        void releaseResources() override;

        void savePatch();

        void loadPatch();

        juce::AudioProcessorEditor* createEditor() override;

    };
}