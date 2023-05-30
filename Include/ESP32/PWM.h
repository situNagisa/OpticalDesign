#pragma once

#include "ESP32/Config.h"
#include "ESP32/BusConcept.h"
#include "NGS/Embedded/Embedded.h"

OPT_BEGIN

class PWM {
public:

public:

	bool Open(ngs::pin_t pwm_num);
	bool IsOpened()const;
	void Set(float percent);
	void Close();
private:

private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
