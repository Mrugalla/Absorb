#pragma once
#include "ButtonParameterRandomizer.h"

#if PPDHasPatchBrowser
#include "PatchBrowser.h"
#endif

#include "LogoComp.h"
#include "Knob.h"
#include "Menu.h"
#include "MIDICCMonitor.h"
#include "LowLevel.h"

namespace gui
{
	struct HighLevel :
		public Comp
	{
		HighLevel(Utils&, LowLevel*);

		void init();

		void paint(Graphics& g) override;

		void resized() override;

	protected:
#if PPDHasPatchBrowser
		PatchBrowser patchBrowser;
		ButtonPatchBrowser patchBrowserButton;
#endif
		Knob macro;
		Button modDepthLocked;
		Button swapParamWithModDepth;
		Button saveModPatch, loadModPatch, removeCurModPatch;

		ButtonParameterRandomizer parameterRandomizer;
#if PPDHasGainIn
		Knob gainIn;
#endif
		Knob gainOut;
		Knob mix;
#if PPDHasUnityGain && PPDHasGainIn
		Button unityGain;
#endif
#if PPDHasHQ
		Button hq;
#endif
#if PPDHasStereoConfig
		Button stereoConfig;
#endif
		Button power;
#if PPDHasPolarity
		Button polarity;
#endif
		MIDICCMonitor ccMonitor;

		LowLevel* lowLevel;
		std::unique_ptr<Menu> menu;
		Button menuButton;

		std::unique_ptr<FileChooser> fileChooser;
	};
}