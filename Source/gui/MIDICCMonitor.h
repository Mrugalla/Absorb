#pragma once
#include "Label.h"
#include "../audio/MIDILearn.h"

namespace gui
{
	struct MIDICCMonitor :
		public Comp,
		public Timer
	{
		using Learn = audio::MIDILearn;

		MIDICCMonitor(Utils& u, const Learn& _learn) :
			Comp(u, "Monitors your input controller number for MIDI Learn.", CursorType::Default),
			learn(_learn),
			idx(learn.ccIdx.load()),
			label(u, idx < 0 ? "cc: .." : toString())
		{
			addAndMakeVisible(label);
			label.textCID = ColourID::Hover;
			label.just = Just::left;
			startTimerHz(24);
		}
	protected:
		const Learn& learn;
		int idx;
		Label label;

		void paint(Graphics&) {}

		void resized() override
		{
			label.setBounds(getLocalBounds());
		}

		void timerCallback() override
		{
			const auto nIdx = learn.ccIdx.load();
			if (nIdx < 0)
				return;

			if (idx != nIdx)
			{
				idx = nIdx;
				
				label.setText(toString());
				label.repaint();
			}
		}

		String toString()
		{
			return "cc: " + String(idx);
		}
	};
}