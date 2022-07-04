#pragma once
#include "AudioUtils.h"

namespace audio
{
	struct MIDIManager
	{
		void operator()(MIDIBuffer& midi, int numSamples) noexcept
		{
			if (midi.isEmpty())
				processEmpty(numSamples);
			else
				processBlock(midi, numSamples);
		}

	protected:
		void processEmpty(int/*numSamples*/) noexcept
		{

		}

		void processBlock(MIDIBuffer& midi, int numSamples) noexcept
		{
			auto evt = midi.begin();
			auto ref = *evt;
			auto ts = ref.samplePosition;
			for (auto s = 0; s < numSamples; ++s)
			{
				if (ts > s)
				{
					//bufNotes[s] = currentValue;
					//bufEnv[s] = env() * SafetyCoeff;
				}
				else
				{
					//bool noteOn = env.noteOn;
					while (ts == s)
					{
						const auto msg = ref.getMessage();

						if (msg.isNoteOn())
						{
							//noteValue = static_cast<float>(msg.getNoteNumber());
							//currentValue = noteValue + pitchbendValue;
							//noteOn = true;
							//env.retrig();
						}
						else if (msg.isNoteOff())
						{
							//if (static_cast<int>(noteValue) == msg.getNoteNumber())
							//	noteOn = false;
						}
						else if (msg.isPitchWheel())
						{
							const auto pwv = msg.getPitchWheelValue();
							//pitchbendValue = static_cast<float>(pwv) * PBGain - 1.f;
							//currentValue = noteValue + pitchbendValue;
						}
						else if (msg.isController())
						{
							//c = msg.getControllerNumber();
							//if (c < ccBuf.size())
							//{
							//	auto& cc = ccBuf[c];
							//	auto ap = assignableParam.load();
							//	if (ap != nullptr)
							//	{
							//		cc.param.store(ap);
							//		assignableParam.store(nullptr);
							//	}
							//	cc.setValue(msg.getControllerValue());
							//}
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
		}


	};
}