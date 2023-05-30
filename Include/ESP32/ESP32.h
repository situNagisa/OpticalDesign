#pragma once

#include "ESP32/Register.h"
#include "NGS/NGS.h"

struct ESP32_ProtocolsStruct {
	static constexpr std::array<ngs::byte, 1> Header = { EOD_HEADER };
	static constexpr size_t Size = EOD_PROTOCOL_SIZE;
	static size_t Parser(std::ranges::sized_range auto data) {
		ngs::Assert(std::ranges::size(data) == Size);
		if (ngs::protocols_parsers::parse_range<EOD_TEST, EOD_COMMAND_MAX - 1>(data))return 0;

		ngs::byte sum = 0;
		std::ranges::for_each_n(std::ranges::begin(data), Size - 1, [&sum](ngs::byte byte) { sum += byte; });
		if (sum != *std::ranges::rbegin(data))
			return 0;

		return Size - Header.size();
	}
	static ngs::ByteArray Command(esp32_optical_design_command_e command, ngs::float32 floating) {
		ngs::ByteArray byteArray;
		byteArray.Write<ngs::byte>(EOD_HEADER);
		byteArray.Write<ngs::byte>(command);
		byteArray.Write(floating);
		ngs::byte sum = 0;
		std::ranges::for_each_n(byteArray.Data(), ESP32_ProtocolsStruct::Size - 1, [&sum](ngs::byte byte) { sum += byte; });
		byteArray.Write(sum);
		return byteArray;
	}
};