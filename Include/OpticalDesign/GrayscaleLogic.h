#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/Linux/Esp32.h"

OPT_BEGIN

class GrayscaleLogic {
public:
public:

	void Update() {
		_levels = devices::g_esp32->GetLevels();
	}

	bool IsOnInflectionPoint()const {
		size_t count = 0;
		for (auto& boolean : _levels) {
			if (boolean)count++;
		}
		//std::system("clear");
		if (count)ngs::nos.Trace("%d\n", count);
		return count > 4;
	}
	bool Left()const {
		size_t count = 0;
		for (size_t i = 0; i < 4; i++)
		{
			if (_levels[i])count++;
		}
		return count;
	}
	bool Right()const {
		size_t count = 0;
		for (size_t i = 4; i < 7; i++)
		{
			if (_levels[i])count++;
		}
		return count > 2;
	}
private:

private:
	GrayData _levels = {};
};

OPT_END
