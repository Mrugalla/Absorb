#pragma once

namespace audio
{
	/*samples, numSamples*/
	void encodeMS(float**, int) noexcept;
	
	/*samples, numSamples*/
	void decodeMS(float**, int) noexcept;
}