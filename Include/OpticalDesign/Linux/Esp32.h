#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Linux/NSLConfig.h"
#include "ESP32/ESP32.h"

namespace devices {

	class Esp32 {
	public:

		bool Open() {
			return _driver.Open("/dev/esp32");
		}
		bool IsOpened() {
			return _driver.IsOpened();
		}
		void Close() {
			_driver.Close();
		}
		void Update() {
			constexpr std::array<esp32_optical_design_command_e, 1> commands = {
				/*EOD_GET_ACCELERATION_X,
				EOD_GET_ACCELERATION_Y,
				EOD_GET_ACCELERATION_Z,*/
				/*EOD_GET_ANGULAR_VELOCITY_X,
				EOD_GET_ANGULAR_VELOCITY_Y,
				EOD_GET_ANGULAR_VELOCITY_Z,
				EOD_GET_LINEAR_VELOCITY,*/
				EOD_GET_GRAY,
			};
			_linearVelocity = 0.0f;
			_acceleration.SetZero();
			_angularVelocity.SetZero();
			for (auto command : commands) {
				_Command(command);
				size_t count = 0;
				constexpr size_t size = ESP32_ProtocolsStruct::Size * (2 * commands.size() + 2);
				ngs::byte data[size];
				count = _driver.Read(data, sizeof(data));
				if (count <= 0)return;
				_parser.In(std::ranges::take_view(data, count));
				_parser.Update();
				while (_parser.ParsedSize()) _Parse(_parser.ParsedPop());
			}
		}
		const ngs::Point3f& GetAcceleration()const { return _acceleration; }
		const ngs::Point3f& GetAngularVelocity()const { return _angularVelocity; }

		void SetLinearVelocityPercent(ngs::float32 percent) {
			_Command(EOD_SET_LINEAR_VELOCITY_PERCENT, percent);
		}
		void SetAngularVelocityPercent(ngs::float32 percent) {
			_Command(EOD_SET_ANGULAR_VELOCITY_PERCENT, percent);
		}
		ngs::float32 GetLinearVelocity()const {
			return _linearVelocity;
		}
		auto GetLevels()const { return _levels; }
	private:
		void _Command(esp32_optical_design_command_e command, ngs::float32 percent = 0.0f) {
			ngs::ByteArray byteArray = ESP32_ProtocolsStruct::Command(command, percent);
			_driver.Write(byteArray.Data(), byteArray.Size());
		}
		void _Parse(std::array<ngs::byte, ESP32_ProtocolsStruct::Size> data) {
			ngs::ByteArray byteArray;
			byteArray.Bind(data);
			byteArray.MovePosition(1);
			esp32_optical_design_command_e command = (esp32_optical_design_command_e)byteArray.Read<ngs::byte>();
			ngs::float32 value = byteArray.Read<ngs::float32>();

			switch (command) {
			case EOD_TEST:
			case EOD_COMMAND_MAX:
			case EOD_SET_LINEAR_VELOCITY_PERCENT:
			case EOD_SET_ANGULAR_VELOCITY_PERCENT:
				ngs::nos.Warning("Esp32::_Parse type error\n");
				return;
				break;
			case EOD_GET_ACCELERATION_X:
				_acceleration.x = value;
				break;
			case EOD_GET_ACCELERATION_Y:
				_acceleration.y = value;
				break;
			case EOD_GET_ACCELERATION_Z:
				_acceleration.z = value;
				break;
			case EOD_GET_ANGULAR_VELOCITY_X:
				_angularVelocity.x = value;
				break;
			case EOD_GET_ANGULAR_VELOCITY_Y:
				_angularVelocity.y = value;
				break;
			case EOD_GET_ANGULAR_VELOCITY_Z:
				_angularVelocity.z = value;
				break;
			case EOD_GET_LINEAR_VELOCITY:
				_linearVelocity = value;
				break;
			case EOD_GET_GRAY:
				ngs::byte_ptr b = reinterpret_cast<ngs::byte_ptr>(&value);
				for (size_t i = 0; i < _levels.size(); i++)
				{
					_levels[i] = ngs::Bits(*b, i);
				}
				//ngs::nos.Trace("0x%02x %f\n", *b, value);
				break;
			}
			//ngs::nos.Log("Esp32::_Parse", "command %02x value:%f\n", command, value);
		}
	private:
		ngs::float32 _linearVelocity = 0.0f;
		ngs::Point3f
			_acceleration = {},
			_angularVelocity = {};
		nsl::DeviceFile _driver = {};
		ngs::ProtocolsStructParser<ESP32_ProtocolsStruct> _parser = {};

		std::array<bool, 8> _levels = {};
	};
	inline Esp32* g_esp32 = ngs::New(new Esp32());
};

