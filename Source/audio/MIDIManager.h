#pragma once
#include "MIDILearn.h"
#include <functional>

namespace audio
{
	struct MIDIManager
	{
		MIDIManager(Params& params, State& state) :
			midiLearn(params, state),
			onInit(),
			onEnd(),
			onCC(),
			onNoteOn(),
			onNoteOff(),
			onNoEvt()
		{
			onInit.push_back([&learn = midiLearn]()
			{
				learn.processBlockInit();
			});

			onCC.push_back([&learn = midiLearn](const MidiMessage& msg, int)
			{
				learn.processBlockMIDICC(msg);
			});

			onEnd.push_back([&learn = midiLearn]()
			{
				learn.processBlockEnd();
			});
		}

		void savePatch()
		{
			midiLearn.savePatch();
		}

		void loadPatch()
		{
			midiLearn.loadPatch();
		}

		void operator()(MIDIBuffer& midi, int numSamples) noexcept
		{
			if (midi.isEmpty())
				processEmpty(numSamples);
			else
				processBlock(midi, numSamples);
		}

		MIDILearn midiLearn;

		std::vector<std::function<void()>> onInit, onEnd;
		std::vector<std::function<void(const MidiMessage&, int/*sample index*/)>> onCC, onNoteOn, onNoteOff;
		std::vector<std::function<void(int/*sample index*/)>> onNoEvt;
	protected:

		void processEmpty(int/*numSamples*/) noexcept
		{

		}

		void processBlock(MIDIBuffer& midi, int numSamples) noexcept
		{
			for (auto& func : onInit)
				func();

			auto evt = midi.begin();
			auto ref = *evt;
			auto ts = ref.samplePosition;
			for (auto s = 0; s < numSamples; ++s)
			{
				if (ts > s)
				{
					for (auto& func : onNoEvt)
						func(s);
				}
				else
				{
					//bool noteOn = env.noteOn;
					while (ts == s)
					{
						const auto msg = ref.getMessage();

						if (msg.isNoteOn())
						{
							for (auto& func : onNoteOn)
								func(msg, s);
						}
						else if (msg.isNoteOff())
						{
							for (auto& func : onNoteOff)
								func(msg, s);
							//if (static_cast<int>(noteValue) == msg.getNoteNumber())
							//	noteOn = false;
						}
						else if (msg.isPitchWheel())
						{
							//const auto pwv = msg.getPitchWheelValue();
							//pitchbendValue = static_cast<float>(pwv) * PBGain - 1.f;
							//currentValue = noteValue + pitchbendValue;
						}
						else if (msg.isController())
						{
							for (auto& func : onCC)
								func(msg, s);
						}
						++evt;
						if (evt == midi.end())
							ts = numSamples;
						else
						{
							ref = *evt;
							ts = ref.samplePosition;
						}
					}
					//bufNotes[s] = currentValue;
					//bufEnv[s] = env(noteOn) * SafetyCoeff;
				}
			}

			for (auto& func : onEnd)
				func();
		}


	};
}