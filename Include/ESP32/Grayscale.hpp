#include "ESP32/Grayscale.h"

OPT_BEGIN

inline bool Grayscale::Open(const std::array<ngs::pin_t, 8>& pins) {
	for (size_t i = 0; i < pins.size(); i++) {
		auto& gpio = *(_gpios[i] = ngs::New(new ngs::GPIO()));

		if (!gpio.Open(pins[i], ngs::GPIO::Mode::input)) {
			ngs::nos.Error("open gpio [%d]%d fail!\n", i, pins[i]);
			goto err_open;
		}
	}
	_is_opened = true;
	ngs::nos.Log("Grayscale::Open", "successfully!\n");
	return true;
err_open:;
	for (auto& gpio : _gpios) {
		if (!gpio)continue;
		ngs::Delete(gpio);
		gpio = nullptr;
	}
	return false;
}

inline bool Grayscale::IsOpened() const
{
	return _is_opened;
}

inline void Grayscale::Close()
{
	for (auto& gpio : _gpios) {
		ngs::Delete(gpio);
		gpio = nullptr;
	}
	_is_opened = false;
}

inline std::array<bool, 8> Grayscale::GetLevels() const
{
	std::array<bool, 8> levels = {};
	for (size_t i = 0; i < _gpios.size(); i++)
	{
		levels[i] = _gpios[i]->Get();
	}
	return levels;
}

OPT_END
