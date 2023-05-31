#pragma once

#include "ESP32/Gyroscope.h"
#include "ESP32/Motor.h"
#include "ESP32/Setting.h"
#include "ESP32/ESP32.h"
#include "ESP32/GPIO.h"

OPT_BEGIN

class Slave {
public:

public:

	void Initialize() {
		//////ngs::nos.Log("Slave::Initialize", "ESP32 支持的线程数为:%d\n", std::jthread::hardware_concurrency());
	}
	bool Open() {
		_is_opened = false;
		if (!_gpio.IsOpened() && !_gpio.Open(LED_TEST))return false;
		//_gpio.High();
		if (!_motor.IsOpened() && !_motor.Open(
			MOTOR_FL_GPIO0, MOTOR_FL_GPIO1, MOTOR_FL_PWM,
			MOTOR_BL_GPIO0, MOTOR_BL_GPIO1, MOTOR_BL_PWM,
			MOTOR_FR_GPIO0, MOTOR_FR_GPIO1, MOTOR_FR_PWM,
			MOTOR_BR_GPIO0, MOTOR_BR_GPIO1, MOTOR_BR_PWM
		))return false;
		if (!_gyro.IsOpened() && !_gyro.Open(GYRO_SDA, GYRO_SCLK))return false;
		if (!_iic.IsOpened() && !_iic.Open(SLAVE_SDA, SLAVE_SCLK, SLAVE_ADDRESS))return false;
		_iic.SetTickToWait(3);
		_is_opened = true;
		return true;
	}
	bool IsOpened()const { return _is_opened; }
	void Update() {
		_UpdateIIC();
		_gyro.Update();
		_motor.Update();
	}
	void Close() {
		_is_opened = false;
		_gyro.Close();
		_motor.Close();
		_iic.Close();
		_gpio.Close();
	}

	void SetLinearVelocityPercent(float percent) {
		_motor.SetMode(Motor::Mode::Linear);
		_motor.SetVelocityPercent(percent);
	}
	void SetAngularVelocityPercent(float percent) {
		_motor.SetMode(Motor::Mode::Angular);
		_motor.SetVelocityPercent(percent);
	}
	auto GetAcceleration() {
		return _gyro.GetAcceleration();
	}
	auto GetAngularVelocity() {
		return _gyro.GetAngularVelocity();
	}
private:
	void _UpdateIIC() {
		ngs::byte data;
		while (_iic.Read(data)) {
			_parser << data;
		}
		_parser.Update();
		while (_parser.ParsedSize())
			_Parse(_parser.ParsedPop());
	}
	void _Command(esp32_optical_design_command_e command, ngs::float32 value) {
		ngs::ByteArray byteArray = ESP32_ProtocolsStruct::Command(command, value);
		_iic.Write(byteArray.Data(), byteArray.Size());
		ngs::nos.Log("Slave::_Command", "command 0x%02x value %f\n", command, value);
		for (size_t i = 0; i < byteArray.Size(); i++)
		{
			ngs::nos.Trace("0x%02x ", byteArray.Data()[i]);
		}
		ngs::nos.Trace("\n");
	}
	void _Parse(std::array<ngs::byte, ESP32_ProtocolsStruct::Size> data) {
		ngs::ByteArray byteArray;
		byteArray.Bind(data);
		byteArray.MovePosition(1);
		esp32_optical_design_command_e command = (esp32_optical_design_command_e)byteArray.Read<ngs::byte>();
		ngs::float32 percent = byteArray.Read<ngs::float32>();
		ngs::float32 value = 0.0f;

		switch (command) {
		case EOD_TEST:
		case EOD_COMMAND_MAX:
			return;
		case EOD_SET_LINEAR_VELOCITY_PERCENT:
			SetLinearVelocityPercent(percent);
			return;
		case EOD_SET_ANGULAR_VELOCITY_PERCENT:
			SetAngularVelocityPercent(percent);
			return;
		case EOD_GET_ACCELERATION_X:
			value = GetAcceleration().x;
			break;
		case EOD_GET_ACCELERATION_Y:
			value = GetAcceleration().y;
			break;
		case EOD_GET_ACCELERATION_Z:
			value = GetAcceleration().z;
			break;
		case EOD_GET_ANGULAR_VELOCITY_X:
			value = GetAngularVelocity().x;
			break;
		case EOD_GET_ANGULAR_VELOCITY_Y:
			value = GetAngularVelocity().y;
			break;
		case EOD_GET_ANGULAR_VELOCITY_Z:
			value = GetAngularVelocity().z;
			break;
		case EOD_GET_LINEAR_VELOCITY:
			value = _motor.GetLinearVelocity();
			break;
		}
		_Command(command, value);
	}
private:
	bool _is_opened;
	GPIO _gpio;
	IICSlave _iic;
	Gyroscope _gyro;
	Motor _motor;

	std::vector<ngs::byte> _buffer;

	ngs::ProtocolsStructParser<ESP32_ProtocolsStruct> _parser;
};

OPT_END
