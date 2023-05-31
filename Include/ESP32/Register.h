#pragma once


// | 0 | 1 | 2 | 3 | 4 | 5 |
// | h | c | dl| dl| dh| dh|

enum esp32_optical_design_command_e : unsigned char {
	EOD_TEST,
	EOD_SET_LINEAR_VELOCITY_PERCENT,
	EOD_SET_ANGULAR_VELOCITY_PERCENT,
	EOD_GET_ACCELERATION_X,
	EOD_GET_ACCELERATION_Y,
	EOD_GET_ACCELERATION_Z,
	EOD_GET_ANGULAR_VELOCITY_X,
	EOD_GET_ANGULAR_VELOCITY_Y,
	EOD_GET_ANGULAR_VELOCITY_Z,
	EOD_GET_LINEAR_VELOCITY,
	EOD_COMMAND_MAX,
};

enum esp32_optical_design_config_e : unsigned char {
	EOD_ADDRESS = 0x1B,
	EOD_HEADER = 0x66,
	EOD_PROTOCOL_SIZE = 7,
};

