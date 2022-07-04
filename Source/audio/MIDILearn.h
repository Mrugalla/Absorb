#pragma once
#include <array>

#include "AudioUtils.h"

namespace audio
{
	class MIDILearn
	{
		static constexpr float ValInv = 1.f / 128.f;

		struct CC
		{
			CC();

			void setValue(int);

			std::atomic<param::Param*> param;
		};

	public:
		MIDILearn(Params&, State&);

		void savePatch() const;
		
		void loadPatch();

		void operator()(const MIDIBuffer&) noexcept;

		void assignParam(param::Param*) noexcept;
		
		void removeParam(param::Param*) noexcept;

		std::array<CC, 120> ccBuf;
		std::atomic<int> ccIdx;
	protected:
		std::atomic<param::Param*> assignableParam;
		Params& params;
		State& state;

		String getIDString(int) const;
	};
}