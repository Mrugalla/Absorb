#include "PluginWrapper.h"

namespace juce
{

    class StandaloneFilterApp : public JUCEApplication
    {
    public:
        StandaloneFilterApp()
        {
            PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_Standalone;

            PropertiesFile::Options options;

            options.applicationName = getApplicationName();
            options.filenameSuffix = ".settings";
            options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX || JUCE_BSD
            options.folderName = "~/.config";
#else
            options.folderName = "";
#endif

            appProperties.setStorageParameters(options);
        }

        const String getApplicationName() override { return CharPointer_UTF8(JucePlugin_Name); }
        const String getApplicationVersion() override { return JucePlugin_VersionString; }
        bool moreThanOneInstanceAllowed() override { return true; }
        void anotherInstanceStarted(const String&) override {}

        virtual StandaloneFilterWindow* createWindow()
        {
#ifdef JucePlugin_PreferredChannelConfigurations
            StandalonePluginHolder::PluginInOuts channels[] = { JucePlugin_PreferredChannelConfigurations };
#endif

            return new StandaloneFilterWindow(getApplicationName(),
                LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
                appProperties.getUserSettings(),
                false, {}, nullptr
#ifdef JucePlugin_PreferredChannelConfigurations
                , juce::Array<StandalonePluginHolder::PluginInOuts>(channels, juce::numElementsInArray(channels))
#else
                , {}
#endif
#if JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                , false
#endif
            );
        }

        //==============================================================================
        void initialise(const String&) override
        {
            mainWindow.reset(createWindow());

#if JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
            Desktop::getInstance().setKioskModeComponent(mainWindow.get(), false);
#endif

            mainWindow->setVisible(true);
        }

        void shutdown() override
        {
            mainWindow = nullptr;
            appProperties.saveIfNeeded();
        }

        //==============================================================================
        void systemRequestedQuit() override
        {
            if (mainWindow.get() != nullptr)
                mainWindow->pluginHolder->savePluginState();

            if (ModalComponentManager::getInstance()->cancelAllModalComponents())
            {
                Timer::callAfterDelay(100, []()
                    {
                        if (auto app = JUCEApplicationBase::getInstance())
                            app->systemRequestedQuit();
                    });
            }
            else
            {
                quit();
            }
        }

    protected:
        ApplicationProperties appProperties;
        std::unique_ptr<StandaloneFilterWindow> mainWindow;
    };

    START_JUCE_APPLICATION(StandaloneFilterApp);

} // namespace juce

#if JucePlugin_Build_Standalone && JUCE_IOS

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wmissing-prototypes")

using namespace juce;

bool JUCE_CALLTYPE juce_isInterAppAudioConnected()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->isInterAppAudioConnected();

    return false;
}

void JUCE_CALLTYPE juce_switchToHostApplication()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        holder->switchToHostApplication();
}

Image JUCE_CALLTYPE juce_getIAAHostIcon(int size)
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->getIAAHostIcon(size);

    return Image();
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif