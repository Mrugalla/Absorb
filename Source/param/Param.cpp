#include "Param.h"
#include "../arch/FormularParser.h"

namespace param
{
	String toID(const String& name)
	{
		return name.removeCharacters(" ").toLowerCase();
	}

	PID ll(PID pID, int offset) noexcept
	{
		auto i = static_cast<int>(pID);
		i += (NumLowLevelParams - 1) * offset;
		return static_cast<PID>(i);
	}

	String toString(PID pID)
	{
		switch (pID)
		{
		case PID::Macro: return "Macro";
#if PPDHasGainIn
		case PID::GainIn: return "Gain In";
#endif
		case PID::Mix: return "Mix";
		case PID::Gain: return "Gain Out";
#if PPDHasHQ
		case PID::HQ: return "HQ";
#endif
#if PPDHasPolarity
		case PID::Polarity: return "Polarity";
#endif
#if PPDHasUnityGain && PPDHasGainIn
		case PID::UnityGain: return "Unity Gain";
#endif
#if PPDHasStereoConfig
		case PID::StereoConfig: return "Stereo Config";
#endif

		case PID::Power: return "Power";

			// LOW LEVEL PARAMS:
		case PID::AbsorbRM: return "Absorb RM";
		case PID::AbsorbAM: return "Absorb AM";
		case PID::AbsorbShapr: return "Absorb Shapr";
		case PID::AbsorbCrushr: return "Absorb Crushr";
		case PID::AbsorbFoldr: return "Absorb Foldr";

		default: return "Invalid Parameter Name";
		}
	}

	PID toPID(const String& id)
	{
		const auto nID = toID(id);
		for (auto i = 0; i < NumParams; ++i)
		{
			auto pID = static_cast<PID>(i);
			if (nID == toID(toString(pID)))
				return pID;
		}
		return PID::NumParams;
	}

	String toTooltip(PID pID)
	{
		switch (pID)
		{
		case PID::Macro: return "Dial in the desired amount of macro modulation depth.";
#if PPDHasGainIn
		case PID::GainIn: return "Apply input gain to the wet signal.";
#endif
		case PID::Mix: return "Mix the dry with the wet signal.";
		case PID::Gain: return "Apply output gain to the wet signal.";
#if PPDHasHQ
		case PID::HQ: return "Turn on HQ to apply 2x Oversampling to the signal.";
#endif
#if PPDHasPolarity
		case PID::Polarity: return "Invert the wet signal's polarity.";
#endif
#if PPDHasUnityGain && PPDHasGainIn
		case PID::UnityGain: return "If enabled the inversed input gain gets added to the output gain.";
#endif
#if PPDHasStereoConfig
		case PID::StereoConfig: return "Define the stereo-configuration. L/R or M/S.";
#endif

		case PID::Power: return "Bypass the plugin with this parameter.";

		case PID::AbsorbRM: return "Controls the volume of Absorb's Ring Modulator.";
		case PID::AbsorbAM: return "Controls the volume of Absorb's Amplitude Modulator.";
		case PID::AbsorbShapr: return "Controls the volume of Absorb's Waveshaper.";
		case PID::AbsorbCrushr: return "Controls the volume of Absorb's Bitcrusher.";
		case PID::AbsorbFoldr: return "Controls the volume of Absorb's Wavefolder.";

		default: return "Invalid Tooltip.";
		}
	}

	String toString(Unit pID)
	{
		switch (pID)
		{
		case Unit::Power: return "";
		case Unit::Solo: return "S";
		case Unit::Mute: return "M";
		case Unit::Percent: return "%";
		case Unit::Hz: return "hz";
		case Unit::Beats: return "x";
		case Unit::Degree: return CharPtr("\xc2\xb0");
		case Unit::Octaves: return "oct";
		case Unit::Semi: return "semi";
		case Unit::Fine: return "fine";
		case Unit::Ms: return "ms";
		case Unit::Decibel: return "db";
		case Unit::Ratio: return "ratio";
		case Unit::Polarity: return CharPtr("\xc2\xb0");
		case Unit::StereoConfig: return "";
		case Unit::Voices: return "v";
		case Unit::Pan: return "%";
		default: return "";
		}
	}

	// PARAM:

	Param::Param(const PID pID, const Range& _range, const float _valDenormDefault,
		const ValToStrFunc& _valToStr, const StrToValFunc& _strToVal,
		State& _state, const Unit _unit) :

		AudioProcessorParameter(),
		id(pID),
		range(_range),

		state(_state),
		valDenormDefault(_valDenormDefault),

		valNorm(range.convertTo0to1(_valDenormDefault)),
		maxModDepth(0.f),
		valMod(valNorm.load()),
		modBias(.5f),

		valToStr(_valToStr),
		strToVal(_strToVal),
		unit(_unit),

		locked(false),
		inGesture(false),

		modDepthLocked(false)
	{
	}

	void Param::savePatch(juce::ApplicationProperties& appProps) const
	{
		const auto idStr = getIDString(id);

		const auto v = range.convertFrom0to1(getValue());
		state.set(idStr, "value", v, true);
		const auto mdd = getMaxModDepth();
		state.set(idStr, "maxmoddepth", mdd, true);
		const auto mb = getModBias();
		state.set(idStr, "modbias", mb, true);

		auto user = appProps.getUserSettings();
		if (user->isValidFile())
		{
			user->setValue(idStr + "valDefault", valDenormDefault);
		}
	}

	void Param::loadPatch(juce::ApplicationProperties& appProps)
	{
		const auto idStr = getIDString(id);

		const auto lckd = isLocked();
		if (!lckd)
		{
			auto var = state.get(idStr, "value");
			if (var)
			{
				const auto val = static_cast<float>(*var);
				const auto valD = range.convertTo0to1(range.snapToLegalValue(val));
				setValueNotifyingHost(valD);
			}
			var = state.get(idStr, "maxmoddepth");
			if (var)
			{
				const auto val = static_cast<float>(*var);
				setMaxModDepth(val);
			}
			var = state.get(idStr, "modbias");
			if (var)
			{
				const auto val = static_cast<float>(*var);
				setModBias(val);
			}

			auto user = appProps.getUserSettings();
			if (user->isValidFile())
			{
				const auto vdd = user->getDoubleValue(idStr + "valDefault", static_cast<double>(valDenormDefault));
				setDefaultValue(range.convertTo0to1(range.snapToLegalValue(static_cast<float>(vdd))));
			}
		}
	}

	//called by host, normalized, thread-safe
	float Param::getValue() const { return valNorm.load(); }
	float Param::getValueDenorm() const noexcept { return range.convertFrom0to1(getValue()); }

	// called by host, normalized, avoid locks, not used (directly) by editor
	void Param::setValue(float normalized)
	{
		if (isLocked())
			return;

		if (!modDepthLocked)
			return valNorm.store(normalized);

		const auto p0 = valNorm.load();
		const auto p1 = normalized;

		const auto d0 = getMaxModDepth();
		const auto d1 = d0 - p1 + p0;

		valNorm.store(p1);
		setMaxModDepth(d1);
	}

	// called by editor
	bool Param::isInGesture() const noexcept
	{
		return inGesture.load();
	}

	void Param::setValueWithGesture(float norm)
	{
		if (isInGesture())
			return;
		beginChangeGesture();
		setValueNotifyingHost(norm);
		endChangeGesture();
	}

	void Param::beginGesture()
	{
		inGesture.store(true);
		beginChangeGesture();
	}

	void Param::endGesture()
	{
		inGesture.store(false);
		endChangeGesture();
	}

	float Param::getMaxModDepth() const noexcept
	{
		return maxModDepth.load();
	};

	void Param::setMaxModDepth(float v) noexcept
	{
		if (isLocked())
			return;

		maxModDepth.store(juce::jlimit(-1.f, 1.f, v));
	}

	float Param::calcValModOf(float macro) const noexcept
	{
		const auto norm = getValue();

		const auto mmd = maxModDepth.load();
		const auto pol = mmd > 0.f ? 1.f : -1.f;
		const auto md = mmd * pol;
		const auto mdSkew = biased(0.f, md, modBias.load(), macro);
		const auto mod = mdSkew * pol;

		return juce::jlimit(0.f, 1.f, norm + mod);
	}

	float Param::getValMod() const noexcept
	{
		return valMod.load();
	}

	float Param::getValModDenorm() const noexcept
	{
		return range.convertFrom0to1(valMod.load());
	}

	void Param::setModBias(float b) noexcept
	{
		if (isLocked())
			return;

		b = juce::jlimit(BiasEps, 1.f - BiasEps, b);
		modBias.store(b);
	}

	float Param::getModBias() const noexcept
	{
		return modBias.load();
	}

	void Param::setModDepthLocked(bool e) noexcept
	{
		modDepthLocked = e;
	}

	void Param::setDefaultValue(float norm) noexcept
	{
		valDenormDefault = range.convertFrom0to1(norm);
	}

	// called by processor to update modulation value(s)
	void Param::modulate(float macro) noexcept
	{
		valMod.store(calcValModOf(macro));
	}

	float Param::getDefaultValue() const { return range.convertTo0to1(valDenormDefault); }

	String Param::getName(int) const { return toString(id); }

	// units of param (hz, % etc.)
	String Param::getLabel() const { return toString(unit); }

	// string of norm val
	String Param::getText(float norm, int) const
	{
		return valToStr(range.snapToLegalValue(range.convertFrom0to1(norm)));
	}

	// string to norm val
	float Param::getValueForText(const String& text) const
	{
		const auto val = juce::jlimit(range.start, range.end, strToVal(text));
		return range.convertTo0to1(val);
	}

	// string to denorm val
	float Param::getValForTextDenorm(const String& text) const { return strToVal(text); }

	String Param::_toString()
	{
		auto v = getValue();
		return getName(10) + ": " + String(v) + "; " + getText(v, 10);
	}

	bool Param::isLocked() const noexcept { return locked.load(); }

	void Param::setLocked(bool e) noexcept { locked.store(e); }

	void Param::switchLock() noexcept { setLocked(!isLocked()); }

	String Param::getIDString(PID pID)
	{
		return "params/" + toID(toString(pID));
	}

	float Param::biased(float start, float end, float bias/*[0,1]*/, float x) const noexcept
	{
		const auto r = end - start;
		if (r == 0.f)
			return 0.f;
		const auto a2 = 2.f * bias;
		const auto aM = 1.f - bias;
		const auto aR = r * bias;
		return start + aR * x / (aM - x + a2 * x);
	}

}

namespace param::strToVal
{
	std::function<float(String, const float/*altVal*/)> parse()
	{
		return [](const String& txt, const float altVal)
		{
			parser::Parser parse;
			if (parse(txt))
				return parse[0];

			return altVal;
		};
	}

	StrToValFunc power()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Power));
			const auto val = p(text, 0.f);
			return val > .5f ? 1.f : 0.f;
		};
	}

	StrToValFunc solo()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Solo));
			const auto val = p(text, 0.f);
			return val > .5f ? 1.f : 0.f;
		};
	}

	StrToValFunc mute()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Mute));
			const auto val = p(text, 0.f);
			return val > .5f ? 1.f : 0.f;
		};
	}

	StrToValFunc percent()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Percent));
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc hz()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Hz));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc phase()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Degree));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc oct()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Octaves));
			const auto val = p(text, 0.f);
			return std::floor(val);
		};
	}

	StrToValFunc oct2()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Octaves));
			const auto val = p(text, 0.f);
			return val / 12.f;
		};
	}

	StrToValFunc semi()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Semi));
			const auto val = p(text, 0.f);
			return std::floor(val);
		};
	}

	StrToValFunc fine()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Fine));
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc ratio()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Ratio));
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc lrms()
	{
		return [](const String& txt)
		{
			return txt[0] == 'l' ? 0.f : 1.f;
		};
	}

	StrToValFunc freeSync()
	{
		return [](const String& txt)
		{
			return txt[0] == 'f' ? 0.f : 1.f;
		};
	}

	StrToValFunc polarity()
	{
		return [](const String& txt)
		{
			return txt[0] == '0' ? 0.f : 1.f;
		};
	}

	StrToValFunc ms()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Ms));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc db()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Decibel));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc voices()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Voices));
			const auto val = p(text, 1.f);
			return val;
		};
	}

	StrToValFunc pan(const Params& params)
	{
		return[p = parse(), &prms = params](const String& txt)
		{
			if (txt == "center" || txt == "centre")
				return 0.f;

			const auto text = txt.trimCharactersAtEnd("MSLR").toLowerCase();
#if PPDHasStereoConfig
			const auto sc = prms[PID::StereoConfig];
			if (sc->getValMod() < .5f)
#endif
			{
				if (txt == "l" || txt == "left")
					return -1.f;
				else if (txt == "r" || txt == "right")
					return 1.f;
			}
#if PPDHasStereoConfig
			else
			{

				if (txt == "m" || txt == "mid")
					return -1.f;
				else if (txt == "s" || txt == "side")
					return 1.f;
			}
#endif
				
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}


}

namespace param::valToStr
{
	ValToStrFunc mute()
	{
		return [](float v) { return v > .5f ? "Mute" : "Not Mute"; };
	}

	ValToStrFunc solo()
	{
		return [](float v) { return v > .5f ? "Solo" : "Not Solo"; };
	}

	ValToStrFunc power()
	{
		return [](float v) { return v > .5f ? "Enabled" : "Disabled"; };
	}

	ValToStrFunc percent()
	{
		return [](float v) { return String(std::floor(v * 100.f)) + " " + toString(Unit::Percent); };
	}

	ValToStrFunc hz()
	{
		return [](float v)
		{
			if (v >= 10000.f)
				return String(v).substring(0, 5) + " " + toString(Unit::Hz);
			else if (v >= 1000.f)
				return String(v).substring(0, 4) + " " + toString(Unit::Hz);
			else
				return String(v).substring(0, 5) + " " + toString(Unit::Hz);
		};
	}

	ValToStrFunc phase()
	{
		return [](float v) { return String(std::floor(v * 180.f)) + " " + toString(Unit::Degree); };
	}

	ValToStrFunc phase360()
	{
		return [](float v) { return String(std::floor(v * 360.f)) + " " + toString(Unit::Degree); };
	}

	ValToStrFunc oct()
	{
		return [](float v) { return String(std::floor(v)) + " " + toString(Unit::Octaves); };
	}

	ValToStrFunc oct2()
	{
		return [](float v) { return String(std::floor(v / 12.f)) + " " + toString(Unit::Octaves); };
	}

	ValToStrFunc semi()
	{
		return [](float v) { return String(std::floor(v)) + " " + toString(Unit::Semi); };
	}

	ValToStrFunc fine()
	{
		return [](float v) { return String(std::floor(v * 100.f)) + " " + toString(Unit::Fine); };
	}

	ValToStrFunc ratio()
	{
		return [](float v)
		{
			const auto y = static_cast<int>(std::floor(v * 100.f));
			return String(100 - y) + " : " + String(y);
		};
	}

	ValToStrFunc lrms()
	{
		return [](float v) { return v > .5f ? String("m/s") : String("l/r"); };
	}

	ValToStrFunc freeSync()
	{
		return [](float v) { return v > .5f ? String("sync") : String("free"); };
	}

	ValToStrFunc polarity()
	{
		return [](float v) { return v > .5f ? String("on") : String("off"); };
	}

	ValToStrFunc ms()
	{
		return [](float v) { return String(std::floor(v * 10.f) * .1f) + " " + toString(Unit::Ms); };
	}

	ValToStrFunc db()
	{
		return [](float v) { return String(std::floor(v * 100.f) * .01f) + " " + toString(Unit::Decibel); };
	}

	ValToStrFunc empty()
	{
		return [](float) { return String(""); };
	}

	ValToStrFunc voices()
	{
		return [](float v)
		{
			return String(static_cast<int>(v)) + toString(Unit::Voices);
		};
	}

	ValToStrFunc pan(const Params& params)
	{
		return [&prms = params](float v)
		{
			if (v == 0.f)
				return String("C");

#if PPDHasStereoConfig
			const auto sc = prms[PID::StereoConfig];
			const auto vm = sc->getValMod();
			const auto isMidSide = vm > .5f;

			if (!isMidSide)
#endif
			{
				if (v == -1.f)
					return String("Left");
				else if (v == 1.f)
					return String("Right");
				else
					return String(std::floor(v * 100.f)) + (v < 0.f ? " L" : " R");
			}
#if PPDHasStereoConfig
			else
			{
				if (v == -1.f)
					return String("Mid");
				else if (v == 1.f)
					return String("Side");
				else
					return String(std::floor(v * 100.f)) + (v < 0.f ? " M" : " S");
			}
#endif
		};
	}

}

namespace param
{
	Param* makeParam(PID id, State& state,
		float valDenormDefault, const Range& range,
		Unit unit)
	{
		ValToStrFunc valToStrFunc;
		StrToValFunc strToValFunc;

		switch (unit)
		{
		case Unit::Power:
			valToStrFunc = valToStr::power();
			strToValFunc = strToVal::power();
			break;
		case Unit::Solo:
			valToStrFunc = valToStr::solo();
			strToValFunc = strToVal::solo();
			break;
		case Unit::Mute:
			valToStrFunc = valToStr::mute();
			strToValFunc = strToVal::mute();
			break;
		case Unit::Decibel:
			valToStrFunc = valToStr::db();
			strToValFunc = strToVal::db();
			break;
		case Unit::Ms:
			valToStrFunc = valToStr::ms();
			strToValFunc = strToVal::ms();
			break;
		case Unit::Percent:
			valToStrFunc = valToStr::percent();
			strToValFunc = strToVal::percent();
			break;
		case Unit::Hz:
			valToStrFunc = valToStr::hz();
			strToValFunc = strToVal::hz();
			break;
		case Unit::Ratio:
			valToStrFunc = valToStr::ratio();
			strToValFunc = strToVal::ratio();
			break;
		case Unit::Polarity:
			valToStrFunc = valToStr::polarity();
			strToValFunc = strToVal::polarity();
			break;
		case Unit::StereoConfig:
			valToStrFunc = valToStr::lrms();
			strToValFunc = strToVal::lrms();
			break;
		case Unit::Semi:
			valToStrFunc = valToStr::semi();
			strToValFunc = strToVal::semi();
			break;
		case Unit::Fine:
			valToStrFunc = valToStr::fine();
			strToValFunc = strToVal::fine();
			break;
		case Unit::Voices:
			valToStrFunc = valToStr::voices();
			strToValFunc = strToVal::voices();
			break;
		default:
			valToStrFunc = valToStr::empty();
			strToValFunc = strToVal::percent();
			break;
		}

		return new Param(id, range, valDenormDefault, valToStrFunc, strToValFunc, state, unit);
	}

	Param* makeParamPan(PID id, State& state, const Params& params)
	{
		ValToStrFunc valToStrFunc = valToStr::pan(params);
		StrToValFunc strToValFunc = strToVal::pan(params);

		return new Param(id, { -1.f, 1.f }, 0.f, valToStrFunc, strToValFunc, state, Unit::Pan);
	}

	// PARAMS

	Params::Params(AudioProcessor& audioProcessor, State& _state) :
		params(),
		state(_state),
		modDepthLocked(false)
	{
		params.push_back(makeParam(PID::Macro, state, 0.f));
#if PPDHasGainIn
		params.push_back(makeParam(PID::GainIn, state, 0.f, makeRange::withCentre(PPD_GainIn_Min, PPD_GainIn_Max, 0.f), Unit::Decibel));
#endif
		params.push_back(makeParam(PID::Mix, state));
		params.push_back(makeParam(PID::Gain, state, 0.f, makeRange::withCentre(PPD_GainOut_Min, PPD_GainOut_Max, 0.f), Unit::Decibel));
#if PPDHasPolarity
		params.push_back(makeParam(PID::Polarity, state, 0.f, makeRange::toggle(), Unit::Polarity));
#endif
#if PPDHasUnityGain && PPDHasGainIn
		params.push_back(makeParam(PID::UnityGain, state, (PPD_UnityGainDefault ? 1.f : 0.f), makeRange::toggle(), Unit::Polarity));
#endif
#if PPDHasHQ
		params.push_back(makeParam(PID::HQ, state, 0.f, makeRange::toggle()));
#endif
#if PPDHasStereoConfig
		params.push_back(makeParam(PID::StereoConfig, state, 1.f, makeRange::toggle(), Unit::StereoConfig));
#endif
		params.push_back(makeParam(PID::Power, state, 1.f, makeRange::toggle(), Unit::Power));

		// LOW LEVEL PARAMS:
		params.push_back(makeParam(PID::AbsorbRM, state, -20.f, makeRange::withCentre(-20.f, 60.f, 0.f), Unit::Decibel));
		params.push_back(makeParam(PID::AbsorbAM, state, -20.f, makeRange::withCentre(-20.f, 60.f, 0.f), Unit::Decibel));
		params.push_back(makeParam(PID::AbsorbShapr, state, -40.f, makeRange::withCentre(-40.f, 40.f, 0.f), Unit::Decibel));
		params.push_back(makeParam(PID::AbsorbCrushr, state, -40.f, makeRange::withCentre(-40.f, 40.f, 0.f), Unit::Decibel));
		params.push_back(makeParam(PID::AbsorbFoldr, state, -40.f, makeRange::withCentre(-40.f, 40.f, 0.f), Unit::Decibel));

		// LOW LEVEL PARAMS END

		for (auto param : params)
			audioProcessor.addParameter(param);
	}

	void Params::loadPatch(juce::ApplicationProperties& appProps)
	{
		const auto idStr = getIDString();
		const auto mdl = state.get(idStr, "moddepthlocked");
		if (mdl != nullptr)
			setModDepthLocked(static_cast<int>(*mdl) != 0);

		for (auto param : params)
			param->loadPatch(appProps);
	}

	void Params::savePatch(juce::ApplicationProperties& appProps) const
	{
		for (auto param : params)
			param->savePatch(appProps);

		const auto idStr = getIDString();
		state.set(idStr, "moddepthlocked", isModDepthLocked() ? 1 : 0);
	}

	String Params::getIDString()
	{
		return "params";
	}

	int Params::getParamIdx(const String& nameOrID) const
	{
		for (auto p = 0; p < params.size(); ++p)
		{
			const auto pName = toString(params[p]->id);
			if (nameOrID == pName || nameOrID == toID(pName))
				return p;
		}
		return -1;
	}

	size_t Params::numParams() const noexcept { return params.size(); }

	Param* Params::operator[](int i) noexcept { return params[i]; }
	const Param* Params::operator[](int i) const noexcept { return params[i]; }
	Param* Params::operator[](PID p) noexcept { return params[static_cast<int>(p)]; }
	const Param* Params::operator[](PID p) const noexcept { return params[static_cast<int>(p)]; }

	Params::Parameters& Params::data() noexcept { return params; }

	const Params::Parameters& Params::data() const noexcept { return params; }

	bool Params::isModDepthLocked() const noexcept { return modDepthLocked.load(); }

	void Params::setModDepthLocked(bool e) noexcept
	{
		modDepthLocked.store(e);
		for (auto& p : params)
			p->setModDepthLocked(e);
	}

	void Params::switchModDepthLocked() noexcept
	{
		setModDepthLocked(!isModDepthLocked());
	}

	// MACRO PROCESSOR

	MacroProcessor::MacroProcessor(Params& _params) :
		params(_params)
	{
	}

	void MacroProcessor::operator()() noexcept
	{
		const auto modDepth = params[PID::Macro]->getValue();
		for (auto i = 1; i < NumParams; ++i)
			params[i]->modulate(modDepth);
	}
}