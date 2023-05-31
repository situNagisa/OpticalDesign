#pragma once

#include "ESP32/Config.h"

OPT_BEGIN

class PCNT {
public:

public:

	bool Open(
		ngs::pin_t a, ngs::pin_t b,
		int high, int low
	);

	bool IsOpened()const;
	void Close();

	void SetGlitchFilter(ngs::uint32 max_glitch_ns);

	int GetPulseCount()const;

	void Enable();
	void Disable();
	void Start();
	void Stop();
private:

private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
