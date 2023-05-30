#pragma once

#include "ESP32/Config.h"
#include "ESP32/BusConcept.h"
#include "NGS/Embedded/Embedded.h"

OPT_BEGIN

class GPIO {
public:
	enum class Level {
		Low,
		High,
	};
	enum class Mode {
		Output,
		Input,
	};
public:

	bool Open(ngs::pin_t gpio_num);
	bool Open(ngs::pin_t gpio_num, Mode mode);
	bool IsOpened()const;
	void Close();

	void SetMode(GPIO::Mode mode);

	Level Get() const;
	void Set(Level value);

	void High() { Set(Level::High); }
	void Low() { Set(Level::Low); }
private:

private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
