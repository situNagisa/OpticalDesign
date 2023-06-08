#include "GyroscopeSDK/SDK.hpp"
#include "GyroscopeSDK/Com.h"

#include "NGS/NGS.h"

int main222()
{
	ngs::uint32 port = 9;
	//std::cout << "请输入端口号：COM";
	//std::cin >> port;

	COM COM;
	wit::JY901B<wit::JY901B_Mode::ASYNC> jy901B;

	jy901B.ActiveRegister(wit::Register::Table::AX);
	jy901B.ReadTable(wit::ReadDataDefined::ACC, wit::ReadDataDefined::GYRO);

	while (true) {
		std::array<ngs::uint32, 7> bauds = { 4800, 9600, 19200, 38400, 57600, 115200, 230400 };
		ngs::uint32 tryTimes;
		if (!COM.IsOpened()) {
			if (!COM.Open(port, 9600))
				continue;
			jy901B.Bind(
				std::bind(&COM::Send, &COM, std::placeholders::_1, std::placeholders::_2),
				std::bind(&COM::Receive, &COM, std::placeholders::_1)
			);
		}
		ngs::nos.Log("main", "try %d baud\r\n", 9600);

		jy901B.Update();
		if (jy901B.IsActive()) {
			ngs::nos.Log("main", "%d baud find sensor\r\n\r\n", 9600);
			break;
		}
	}

	jy901B.Write(wit::Register::Table::SAVE, wit::Register::Config::SAVE::Reset);
	jy901B.Write(wit::Register::Table::AXIS6, wit::Register::Config::AXIS6::_9);
	jy901B.Write(wit::Register::Table::RRATE, wit::Register::Config::RRATE::_200);
	jy901B.Write(wit::Register::Table::BANDWIDTH, wit::Register::Config::BANDWIDTH::_256);
	jy901B.Write(wit::Register::Table::FILTK, wit::Register::Config::FILTK::_2000);
	jy901B.Write(wit::Register::Table::ACCFILT, wit::Register::Config::ACCFILT::_2000);

	while (true) {
		using std::chrono_literals::operator""ms;
		jy901B.Update();
		auto acceleration = jy901B.GetAcceleration();
		auto angularVelocity = jy901B.GetAngularVelocity();
		if (!jy901B.IsActive())continue;
		ngs::nos.Log("main", "\t\n acc :(%.5f\t  %.5f\t  %.5f)\t\n gyro:(%.5f\t%.5f\t%.5f)\r\n",
			acceleration.x, acceleration.y, acceleration.z,
			angularVelocity.x, angularVelocity.y, angularVelocity.z);
	}
	COM.Close();

	return 0;
}
