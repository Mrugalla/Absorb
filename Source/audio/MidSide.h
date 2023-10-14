#pragma once

namespace audio
{
	/*samples, numSamples, chStart*/
	void encodeMS(float* const*, int, int) noexcept;
	
	/*samples, numSamples, chStart*/
	void decodeMS(float* const*, int, int) noexcept;
}
