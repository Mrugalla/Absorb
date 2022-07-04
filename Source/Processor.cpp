#include "Processor.h"
#include "Editor.h"

namespace audio
{

    juce::AudioProcessorEditor* Processor::createEditor()
    {
        return new gui::Editor(*this);
    }

    juce::AudioProcessor::BusesProperties ProcessorBackEnd::makeBusesProperties()
    {
        BusesProperties bp;
        bp.addBus(true, "Input", ChannelSet::stereo(), true);
        bp.addBus(false, "Output", ChannelSet::stereo(), true);
#if PPDHasSidechain
        if (!juce::JUCEApplicationBase::isStandaloneApp())
        {
            bp.addBus(true, "Sidechain", ChannelSet::stereo(), true);
        }
#endif
        return bp;
    }

    ProcessorBackEnd::ProcessorBackEnd() :
        juce::AudioProcessor(makeBusesProperties()),
        props(),
        sus(*this),
        state(),
        params(*this, state),
        macroProcessor(params),
        midiLearn(params, state),
#if PPDHasHQ
        oversampler(),
#endif
        meters()
#if PPDHasStereoConfig
        , midSideEnabled(false)
#endif
#if PPDHasSidechain
        , scBuffer()
#endif
    {
        {
            juce::PropertiesFile::Options options;
            options.applicationName = JucePlugin_Name;
            options.filenameSuffix = ".settings";
            options.folderName = "Mrugalla" + juce::File::getSeparatorString() + JucePlugin_Name;
            options.osxLibrarySubFolder = "Application Support";
            options.commonToAllUsers = false;
            options.ignoreCaseOfKeyNames = false;
            options.doNotSave = false;
            options.millisecondsBeforeSaving = 20;
            options.storageFormat = juce::PropertiesFile::storeAsXML;

            props.setStorageParameters(options);
        }

        startTimerHz(6);
    }

    const String ProcessorBackEnd::getName() const
    {
        return JucePlugin_Name;
    }

    double ProcessorBackEnd::getTailLengthSeconds() const { return 0.; }

    int ProcessorBackEnd::getNumPrograms() { return 1; }

    int ProcessorBackEnd::getCurrentProgram() { return 0; }

    void ProcessorBackEnd::setCurrentProgram(int) {}

    const String ProcessorBackEnd::getProgramName(int) { return {}; }

    void ProcessorBackEnd::changeProgramName(int, const juce::String&) {}

    bool ProcessorBackEnd::canAddBus(bool isInput) const
    {
        return PPDHasSidechain ? isInput : false;
    }

    bool ProcessorBackEnd::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        const auto mono = ChannelSet::mono();
        const auto stereo = ChannelSet::stereo();
        
        const auto mainIn = layouts.getMainInputChannelSet();
        const auto mainOut = layouts.getMainOutputChannelSet();

        if (mainOut != stereo && mainOut != mono)
            return false;

        if (mainIn != mainOut)
            return false;

#if PPDHasSidechain
        if (wrapperType != wrapperType_Standalone)
        {
            const auto scIn = layouts.getChannelSet(true, 1);
            if (!scIn.isDisabled())
                if (scIn != stereo && scIn != mono)
                    return false;

            if (mainOut != scIn)
                return false;
        }
#endif
        return true;
    }

    ProcessorBackEnd::AppProps* ProcessorBackEnd::getProps() noexcept
    {
        return &props;
    }

    void ProcessorBackEnd::savePatch()
    {
        params.savePatch(props);
        midiLearn.savePatch();
    }

    void ProcessorBackEnd::loadPatch()
    {
        params.loadPatch(props);
        midiLearn.loadPatch();
        forcePrepareToPlay();
    }

    bool ProcessorBackEnd::hasEditor() const { return PPDHasEditor; }
    bool ProcessorBackEnd::acceptsMidi() const { return true; }
    bool ProcessorBackEnd::producesMidi() const { return true; }
    bool ProcessorBackEnd::isMidiEffect() const { return false; }

    /////////////////////////////////////////////
    /////////////////////////////////////////////;
    void ProcessorBackEnd::getStateInformation(juce::MemoryBlock& destData)
    {
        savePatch();
        state.savePatch(*this, destData);
    }

    void ProcessorBackEnd::setStateInformation(const void* data, int sizeInBytes)
    {
        state.loadPatch(*this, data, sizeInBytes);
        loadPatch();
    }

    void ProcessorBackEnd::forcePrepareToPlay()
    {
        sus.suspend();
    }

    void ProcessorBackEnd::timerCallback()
    {
#if PPDHasHQ
        const auto ovsrEnabled = params[PID::HQ]->getValMod() > .5f;
        if (oversampler.isEnabled() != ovsrEnabled)
            forcePrepareToPlay();
#endif
    }

    void ProcessorBackEnd::processBlockBypassed(AudioBuffer& buffer, juce::MidiBuffer&)
    {
        macroProcessor();
        if (sus.suspendIfNeeded(buffer))
            return;
        const auto numSamples = buffer.getNumSamples();
        if (numSamples == 0)
            return;
        auto samples = buffer.getArrayOfWritePointers();
        const auto constSamples = buffer.getArrayOfReadPointers();
        const auto numChannels = buffer.getNumChannels();

        dryWetMix.processBypass(samples, numChannels, numSamples);
#if PPDHasGainIn
        meters.processIn(constSamples, numChannels, numSamples);
#endif
        meters.processOut(constSamples, numChannels, numSamples);
    }

    AudioBuffer* ProcessorBackEnd::processBlockStart(AudioBuffer& buffer, juce::MidiBuffer& midi) noexcept
    {
        midiLearn(midi);

        macroProcessor();

        const auto numSamples = buffer.getNumSamples();
        if (numSamples == 0)
            return nullptr;

        if (params[PID::Power]->getValMod() < .5f)
        {
            processBlockBypassed(buffer, midi);
            return nullptr;
        }

        const auto numChannels = buffer.getNumChannels() == 1 ? 1 : 2;

        auto samples = buffer.getArrayOfWritePointers();

        dryWetMix.saveDry(
            samples,
            numChannels,
            numSamples,
#if PPDHasGainIn
            params[PID::GainIn]->getValueDenorm(),
#endif
            params[PID::Mix]->getValMod(),
            params[PID::Gain]->getValModDenorm()
#if PPDHasPolarity
            , (params[PID::Polarity]->getValMod() > .5f ? -1.f : 1.f)
#endif
#if PPDHasUnityGain && PPDHasGainIn
            , params[PID::UnityGain]->getValMod()
#endif
        );
#if PPDHasGainIn
        const auto constSamples = buffer.getArrayOfReadPointers();
        meters.processIn(constSamples, numChannels, numSamples);
#endif
#if PPDHasStereoConfig
        midSideEnabled = numChannels == 2 && params[PID::StereoConfig]->getValMod() > .5f;
        if (midSideEnabled)
        {
            encodeMS(samples, numSamples);
            {
#if PPDHasHQ
                return &oversampler.upsample(buffer);
#else
                return &buffer;
#endif
            }
        }
        else
#endif
        {
#if PPDHasHQ
            return &oversampler.upsample(buffer);
#else
            return &buffer;
#endif
        }
    }

    void ProcessorBackEnd::processBlockEnd(AudioBuffer& buffer) noexcept
    {
#if PPDHasHQ
        oversampler.downsample(buffer);
#endif
        const auto samples = buffer.getArrayOfWritePointers();
        const auto constSamples = buffer.getArrayOfReadPointers();
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

#if PPDHasStereoConfig
        if (midSideEnabled)
            decodeMS(samples, numSamples);
#endif

        dryWetMix.processOutGain(samples, numChannels, numSamples);
        meters.processOut(constSamples, numChannels, numSamples);
        dryWetMix.processMix(samples, numChannels, numSamples);

#if JUCE_DEBUG
        for (auto ch = 0; ch < numChannels; ++ch)
        {
            auto smpls = samples[ch];

            for (auto s = 0; s < numSamples; ++s)
            {
                if (smpls[s] > 1.f)
                    smpls[s] = 1.f;
                else if (smpls[s] < -1.f)
                    smpls[s] = -1.f;
            }
        }
#endif
    }

    // PROCESSOR

    Processor::Processor() :
        ProcessorBackEnd()
    {
    }

    void Processor::prepareToPlay(double sampleRate, int maxBlockSize)
    {
#if PPDHasSidechain
        scBuffer.setSize(2, maxBlockSize, false, false, false);
#endif

        auto latency = 0;
#if PPDHasHQ
        oversampler.setEnabled(params[PID::HQ]->getValMod() > .5f);
        oversampler.prepare(sampleRate, maxBlockSize);
        const auto sampleRateUp = oversampler.getFsUp();
        const auto sampleRateUpF = static_cast<float>(sampleRateUp);
        const auto blockSizeUp = oversampler.getBlockSizeUp();
        latency = oversampler.getLatency();
#endif
        const auto sampleRateF = static_cast<float>(sampleRate);

        dryWetMix.prepare(sampleRateF, maxBlockSize, latency);

        meters.prepare(sampleRateF, maxBlockSize);

        setLatencySamples(latency);

        sus.prepareToPlay();
    }

    void Processor::processBlock(AudioBuffer& buffer, juce::MidiBuffer& midi)
    {
        const juce::ScopedNoDenormals noDenormals;

        if (sus.suspendIfNeeded(buffer))
            return;

        auto buf = processBlockStart(buffer, midi);
        if (buf == nullptr)
            return;

#if PPDHasSidechain
        //const auto scBus = getBus(true, 1);
        //if (scBus == nullptr || !scBus->isEnabled())
        //    return;

        //scBuffer = scBus->getBusBuffer(scBuffer);
#endif

        processBlockCustom(
            buf->getArrayOfWritePointers(),
            buf->getNumChannels(),
            buf->getNumSamples()
        );

        processBlockEnd(buffer);
    }

    void Processor::processBlockCustom(float** samples, int numChannels, int numSamples) noexcept
    {
        const auto crushGain = params[PID::CrushGain]->getValModDenorm();
        crush(samples, numChannels, numSamples, crushGain);
    }

    void Processor::releaseResources() {}

    void Processor::savePatch()
    {
        ProcessorBackEnd::savePatch();
    }

    void Processor::loadPatch()
    {
        ProcessorBackEnd::loadPatch();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new audio::Processor();
}
