#pragma once

#include "ESP32/Config.h"
#include "GyroscopeSDK/SDK.hpp"
#include "ESP32/IIC.h"

OPT_BEGIN

class Gyroscope {
public:
	NGS_TYPE_DEFINE(wit::JY901B<wit::JY901B_Mode::SYNC>, gyro);
public:

	bool Open(ngs::pin_t SDA, ngs::pin_t SCLK) {
		ngs::nos.Log("Gyroscope Open", "open gyro\n");
		if (!_iic.IsOpened() && !_iic.Open(SDA, SCLK, 0x50))return false;
		_iic.SetTickToWait(1);
		_iic.SetACK(false, ngs::I2CMaster::ACK_Type::last_nack);
		_InitGyro();
		return true;
	}
	bool IsOpened()const { return _iic.IsOpened(); }
	void Close() {
		if (_gyro) {
			ngs::Delete(_gyro);
			_gyro = nullptr;
		}
		_iic.Close();
	}

	void Update() {
		if (!_gyro && !_InitGyro())return;
		_gyro->Update();
		_acceleration = _gyro->GetAcceleration();
		_angularVelocity = _gyro->GetAngularVelocity();
		_angularVelocity.x = ngs::AsRadian(_angularVelocity.x);
		_angularVelocity.y = ngs::AsRadian(_angularVelocity.y);
		_angularVelocity.z = ngs::AsRadian(_angularVelocity.z);
		/*if (!_gyro->IsActive())return;
		ngs::nos.Log("Gyroscope::Update", "\nacc:%.2f \t%.2f\t %.2f\t\ngyro:%.2f\t %.2f\t %.2f\t\n",
			_acceleration.x, _acceleration.y, _acceleration.z,
			_angularVelocity.x, _angularVelocity.y, _angularVelocity.z
		);*/
	}

	auto GetAcceleration()const { return _acceleration; }
	auto GetAngularVelocity()const { return _angularVelocity; }
private:

	bool _InitGyro() {
		using write_type = size_t(IICMaster::*)(ngs::byte_ptr_cst, size_t);
		write_type write = &IICMaster::Write;
		auto writeRead = &IICMaster::WriteRead;

		using namespace std::placeholders;

		_gyro = ngs::New(new __gyro(
			std::bind(write, &_iic, _1, _2),
			std::bind(writeRead, &_iic, _1, _2, _3, _4)
		));

		//_gyro->ReadTable(wit::ReadDataDefined::ACC, wit::ReadDataDefined::GYRO);
		//_gyro->ActiveRegister(wit::Register::AX);
		//_gyro->ActiveRegister(wit::Register::AY);
		//_gyro->ActiveRegister(wit::Register::AZ);
		_gyro->ActiveRegister(wit::Register::GX);
		_gyro->ActiveRegister(wit::Register::GY);
		_gyro->ActiveRegister(wit::Register::GZ);

		ngs::nos.Log("GyroScope::_InitGyro()", "try to find gyroscope\n");
		constexpr auto tryTimes = 10;
		for (size_t i = 0; i < tryTimes; i++)
		{
			ngs::nos.Log("GyroScope::_InitGyro()", "try times: %d\n", i);
			_gyro->Update();
			if (_gyro->IsActive()) break;
		}
		if (!_gyro->IsActive()) {
			ngs::nos.Error("active gyroscope failed!\n");
			ngs::Delete(_gyro);
			_gyro = nullptr;
			return false;
		}
		ngs::nos.Log("GyroScope::_InitGyro()", "success!\n");

		_gyro->Write(wit::Register::Table::SAVE, (ngs::uint16)wit::Register::Config::SAVE::Reset);
		_gyro->Write(wit::Register::Table::AXIS6, (ngs::uint16)wit::Register::Config::AXIS6::_9);
		_gyro->Write(wit::Register::Table::RRATE, (ngs::uint16)wit::Register::Config::RRATE::_200);
		_gyro->Write(wit::Register::Table::BANDWIDTH, (ngs::uint16)wit::Register::Config::BANDWIDTH::_256);
		_gyro->Write(wit::Register::Table::FILTK, (ngs::uint16)wit::Register::Config::FILTK::Default);

		return true;
	}
private:
	IICMaster _iic;
	__gyro_ptr _gyro = nullptr;
	ngs::Point3f
		_acceleration = {},
		_angularVelocity = {};
};

OPT_END
