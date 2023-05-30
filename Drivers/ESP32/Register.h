#pragma once


// | 0 | 1 | 2 | 3 | 4 | 5 |
// | h | c | dl| dl| dh| dh|

enum esp32_optical_design_command_e{
	EOD_TEST,
	EOD_SET_LINEAR_VELOCITY_PERCENT,
	EOD_SET_ANGULAR_VELOCITY_PERCENT,
	EOD_GET_ACCELERATION_X,
	EOD_GET_ACCELERATION_Y,
	EOD_GET_ACCELERATION_Z,
	EOD_GET_ANGULAR_VELOCITY_X,
	EOD_GET_ANGULAR_VELOCITY_Y,
	EOD_GET_ANGULAR_VELOCITY_Z,
	EOD_COMMAND_MAX,
};
//大小端问题暂时没想到怎么解决

// void EOD_Encode(unsigned char* dst, float data) {
// 	dst[0] = (unsigned char)((unsigned long)data >> (sizeof(unsigned char) * 0));
// 	dst[1] = (unsigned char)((unsigned long)data >> (sizeof(unsigned char) * 1));
// 	dst[2] = (unsigned char)((unsigned long)data >> (sizeof(unsigned char) * 2));
// 	dst[3] = (unsigned char)((unsigned long)data >> (sizeof(unsigned char) * 3));
// }

// float EOD_Decode(const unsigned char* data) {
// 	return
// 		data[0] << (sizeof(unsigned char) * 0) |
// 		data[1] << (sizeof(unsigned char) * 1) |
// 		data[2] << (sizeof(unsigned char) * 2) |
// 		data[3] << (sizeof(unsigned char) * 3);
// }

enum esp32_optical_design_config_e {
	EOD_ADDRESS = 0x1B,
	EOD_HEADER = 0x66,
	EOD_PROTOCOL_READ_SIZE = 6,
	EOD_PROTOCOL_WRITE_SIZE = EOD_PROTOCOL_READ_SIZE,
};

