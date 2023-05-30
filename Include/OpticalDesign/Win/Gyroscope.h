#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"
#include "OpticalDesign/DeviceConfig.h"
#include "GyroscopeSDK/Com.h"
#include "GyroscopeSDK/SDK.hpp"

namespace devices {

	struct _GyroscopeData {
		COM com;
		wit::JY901B<wit::JY901B_Mode::ASYNC> jy901B;
		ngs::Point3f acceleration;
		ngs::Point3f angularVelocity;
	};

	inline bool Gyroscope::Open() {
		_data = ngs::New(new _GyroscopeData());
		_GyroscopeData& data = *(_GyroscopeData*)_data;

		data.jy901B.ReadTable(wit::ReadDataDefined::ACC, wit::ReadDataDefined::GYRO);
		data.jy901B.ActiveRegister(wit::Register::AX);

		{
			size_t times = 10;
			while (times--) {
				ngs::uint32 tryTimes;
				if (!data.com.IsOpened()) {
					if (!data.com.Open(9, 9600))
						continue;
				}
				data.jy901B.Bind(
					std::bind(&COM::Send, &((_GyroscopeData*)_data)->com, std::placeholders::_1, std::placeholders::_2),
					std::bind(&COM::Receive, &((_GyroscopeData*)_data)->com, std::placeholders::_1)
				);

				ngs::nos.Log("Gyroscope::Open", "尝试打开串口Com%d:%d\r\n", 9, 9600);

				data.jy901B.Update();
				if (data.jy901B.IsActive()) {
					ngs::nos.Log("Gyroscope::Open", "串口Com%d:%d 发现陀螺仪\r\n\r\n", 9, 9600);
					break;
				}
			}
			if (!data.jy901B.IsActive()) {
				ngs::nos.Error("打开陀螺仪失败\n");
				ngs::Delete(&data);
				_data = nullptr;
				return false;
			}
			ngs::nos.Log("Gyroscope::Open", "打开陀螺仪成功\n");
		}
		{
			data.jy901B.Write(wit::Register::Table::SAVE, wit::Register::Config::SAVE::Reset);
			data.jy901B.Write(wit::Register::Table::AXIS6, wit::Register::Config::AXIS6::_9);
			data.jy901B.Write(wit::Register::Table::RRATE, wit::Register::Config::RRATE::_200);
			data.jy901B.Write(wit::Register::Table::BANDWIDTH, wit::Register::Config::BANDWIDTH::_256);
			data.jy901B.Write(wit::Register::Table::FILTK, wit::Register::Config::FILTK::_2000);
			data.jy901B.Write(wit::Register::Table::ACCFILT, wit::Register::Config::ACCFILT::_2000);
		}
		return true;
	}
	inline void Gyroscope::Close() {
		auto& data = *(_GyroscopeData*)_data;
		data.com.Close();

		ngs::Delete(&data);
		_data = nullptr;
	}
	inline bool Gyroscope::IsOpened() const {
		if (!_data)return false;
		auto& data = *(_GyroscopeData*)_data;
		return data.jy901B.IsActive();
	}
	inline void Gyroscope::Update() {
		auto& data = *(_GyroscopeData*)_data;
		data.jy901B.Update();

		data.acceleration = data.jy901B.GetAcceleration();
		data.angularVelocity = data.jy901B.GetAngularVelocity();

		ngs::nos.Log("Gyroscope::Update", "角速度: %4.4f\t\t %4.4f\t\t %4.4f\t\t\r\n", data.angularVelocity.x, data.angularVelocity.y, data.angularVelocity.z);
		ngs::nos.Log("Gyroscope::Update", "加速度: %4.4f\t\t %4.4f\t\t %4.4f\t\t\r\n", data.acceleration.x, data.acceleration.y, data.acceleration.z);
	}
	inline float Gyroscope::GetAccelerationX() const {
		auto& data = *(_GyroscopeData*)_data;
		return data.acceleration.x;
	}
	inline float Gyroscope::GetAccelerationY() const {
		auto& data = *(_GyroscopeData*)_data;
		return data.acceleration.y;
	}
	inline float Gyroscope::GetAccelerationZ() const {
		auto& data = *(_GyroscopeData*)_data;
		return data.acceleration.z;
	}
	inline float Gyroscope::GetAngularVelocityX() const {
		auto& data = *(_GyroscopeData*)_data;
		return ngs::AsRadian(data.angularVelocity.x);
	}
	inline float Gyroscope::GetAngularVelocityY() const {
		auto& data = *(_GyroscopeData*)_data;
		return ngs::AsRadian(data.angularVelocity.y);
	}
	inline float Gyroscope::GetAngularVelocityZ() const {
		auto& data = *(_GyroscopeData*)_data;
		return ngs::AsRadian(data.angularVelocity.z);
	}

};