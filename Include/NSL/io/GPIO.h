#pragma once

#include "NSL/Config.h"
#include "NSL/file/DeviceFile.h"
#include "NSL/io/MultiplexIO.h"

NSL_BEGIN
static const std::string GPIO_DIR = "/sys/class/gpio/";

struct GPIO_Data {
	ngs::pin_t pin;
	std::string directory;
	nsl::DeviceFile
		direction{"direction"},
		active_low{ "active_low" },
		value{ "value" },
		edge{ "edge" }
	;
};

NSL_END

static bool _Export(ngs::pin_t pin) {
	auto pin_s = std::to_string(pin);
	std::string dir = nsl::GPIO_DIR + "gpio" + pin_s;

	if (!nsl::File::Access(dir)) {
		nsl::File exp("export");
		nsl::File::OpenMode mode = {};
		mode.write = true;

		if (!exp.OpenDirectory(nsl::GPIO_DIR, mode)) {
			ngs::nos.Log("::_Export", "注册gpio %s 失败！\n", pin_s.c_str());
			return false;
		}

		int ret = exp.Write(pin_s);
		ngs::nos.Log("::_Export", "ret:%d size:%d\n", ret, pin_s.size());
		if (pin_s.size() != ret) {
			exp.Close();
			ngs::nos.Log("::_Export", "注册gpio %s 失败！\n", pin_s.c_str());
			return false;
		}
		exp.Close();
	}
	return true;
}
static bool _UnExport(ngs::pin_t pin) {
	auto pin_s = std::to_string(pin);
	std::string dir = nsl::GPIO_DIR + "gpio" + pin_s;

	if (!nsl::File::Access(dir)) {
		nsl::File unExp("unexport");
		nsl::File::OpenMode mode = {};
		mode.write = true;

		if (!unExp.OpenDirectory(nsl::GPIO_DIR, mode)) {
			ngs::nos.Log("::_UnExport", "注销gpio %s 失败！\n", pin_s.c_str());
			return false;
		}
		int ret = unExp.Write(pin_s);
		ngs::nos.Log("::_UnExport", "ret:%d size:%d\n", ret, pin_s.size());
		if (pin_s.size() != ret) {
			unExp.Close();
			ngs::nos.Log("::_UnExport", "注销gpio %s 失败！\n", pin_s.c_str());
			return false;
		}
		unExp.Close();
	}
	return true;
}

bool ngs::GPIO::Open(ngs::pin_t pin, Mode mode) {
	using nsl::GPIO_Data;
	using nsl::GPIO_DIR;

	if (!_Export(pin)) return false;
	_data = ngs::New(new nsl::GPIO_Data());
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);

	//初始化设备文件
	data.directory = GPIO_DIR + "gpio" + std::to_string(pin);
	data.pin = pin;

	nsl::File::OpenMode open_mode = {};
	open_mode.read_write = true;
	if (!data.direction.OpenDirectory(data.directory, open_mode))goto err;
	if (!data.edge.OpenDirectory(data.directory, open_mode))goto err;

	open_mode.Clear();
	open_mode.write = true;
	if (!data.active_low.OpenDirectory(data.directory, open_mode))goto err;
	data.active_low.Write("0");

	SetMode(mode);
	if (!data.value.IsOpened())goto err;

	ngs::nos.Log("GPIO::Open", "pin:%d successfully!\n", pin);

	return true;
err:;
	if (data.direction.IsOpened())data.direction.Close();
	if (data.active_low.IsOpened())data.active_low.Close();
	if (data.value.IsOpened())data.value.Close();
	if (data.edge.IsOpened())data.edge.Close();

	_UnExport(data.pin);

	ngs::Delete(&data);
	_data = nullptr;
	return false;
}

bool ngs::GPIO::IsOpened()const {
	return _data;
}

void ngs::GPIO::Close() {
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);

	if (data.direction.IsOpened())data.direction.Close();
	if (data.active_low.IsOpened())data.active_low.Close();
	if (data.value.IsOpened())data.value.Close();
	if (data.edge.IsOpened())data.edge.Close();

	_UnExport(data.pin);

	ngs::Delete(&data);
	_data = nullptr;
}

void ngs::GPIO::SetInterrupt(bool enable) {
	if (enable)return;
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);
	data.edge.Write("none");
}
void ngs::GPIO::SetInterrupt(Interrupt type) {
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);

	switch (type)
	{
	case ngs::GPIO::Interrupt::disable:
		data.edge.Write("none");
		break;
	case ngs::GPIO::Interrupt::positive:
		data.edge.Write("rising");
		break;
	case ngs::GPIO::Interrupt::nagative:
		data.edge.Write("falling");
		break;
	case ngs::GPIO::Interrupt::any:
		data.edge.Write("both");
		break;
	case ngs::GPIO::Interrupt::low:

		break;
	case ngs::GPIO::Interrupt::high:

		break;
	case ngs::GPIO::Interrupt::max:
	default:
		return;
	}
}

void ngs::GPIO::SetMode(Mode mode) {
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);

	nsl::File::OpenMode open_mode = {};
	switch (mode)
	{
	case ngs::GPIO::Mode::disable:
		return;
		break;
	case ngs::GPIO::Mode::input:
		open_mode.read = true;
		data.direction.Write("in");
		break;
	case ngs::GPIO::Mode::output:
	case ngs::GPIO::Mode::output_open_drain:
		open_mode.write = true;
		data.direction.Write("out");
		break;
	case ngs::GPIO::Mode::input_output_open_drain:
	case ngs::GPIO::Mode::input_output:
		open_mode.read_write = true;
		break;
	default:
		break;
	}
	if (data.value.IsOpened())data.value.Close();
	data.value.OpenDirectory(data.directory, open_mode);
}

bool ngs::GPIO::Get()const {
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);
	char level = '0';
	data.value.OffsetHead();
	data.value.Read((ngs::byte_ptr)&level, 1);
	//ngs::nos.Log("GPIO::Get", "level:%c\n", level);

	return level == '1';
}
void ngs::GPIO::Set(bool level) {
	nsl::GPIO_Data& data = *reinterpret_cast<nsl::GPIO_Data*>(_data);

	if (level)
		data.value.Write("1");
	else
		data.value.Write("0");
}


