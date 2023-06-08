#pragma once

#include "ESP32/Config.h"
#include "ESP32/Setting.h"
#include "NGS/Embedded/Embedded.h"


OPT_BEGIN

class Grayscale {
public:

public:
	bool Open(const std::array<ngs::pin_t, 8>&);
	bool IsOpened()const;
	void Close();

	std::array<bool, 8> GetLevels()const;
	ngs::byte GetBinaryLevels()const {
		auto levels = GetLevels();
		ngs::byte bin = 0;
		for (size_t i = 0; i < levels.size(); i++) {
			if (levels[i])bin |= ngs::bit(i);
		}
		return bin;
	}
private:

private:
	std::array<ngs::GPIO*, 8> _gpios = {};
	bool _is_opened = false;
};

OPT_END

#include "ESP32/Grayscale.hpp"