#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/DeviceConfig.h"
#include "OpticalDesign/Linux/NSLConfig.h"

namespace devices {
	class Key {
	public:
		static constexpr const char* GPIO_KEY_FILE = "/dev/input/event2";
		enum class Type {
			down,
			up,
			hold_down,
		};
	public:
		bool Open() {
			nsl::File::OpenMode mode = {};
			mode.read = true;
			return _driver.Open(GPIO_KEY_FILE, mode);
		}
		bool IsOpened()const { return _driver.IsOpened(); }

		void Update() {
			struct input_event in_ev = {};
			if (sizeof(in_ev) != _driver.Read((ngs::byte_ptr)&in_ev, sizeof(struct input_event))) {
				ngs::nos.Error("input read fail!\n");
				return;
			}
			if (in_ev.type != EV_KEY)return;
			switch (in_ev.type) {
			case 0:
				_state = Type::up;
				break;
			case 1:
				_state = Type::down;
				break;
			case 2:
				_state = Type::hold_down;
				break;
			}
		}
		Type GetState()const { return _state; }

	private:
		nsl::DeviceFile _driver;
		Type _state;
	};
};
