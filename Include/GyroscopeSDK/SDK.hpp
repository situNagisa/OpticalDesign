#pragma once

#include "GyroscopeSDK/Defined.h"
#include "GyroscopeSDK/Register.h"

WIT_BEGIN

#define WINDOWS_CNM_NULL NULL
#undef NULL

class _JY901B_Base {
public:

public:

	ngs::Point3f GetAcceleration()const {
		return {
			float(_register[Register::Table::AX]) / 32768.0f * 16.0f * 9.8f,
			float(_register[Register::Table::AY]) / 32768.0f * 16.0f * 9.8f,
			float(_register[Register::Table::AZ]) / 32768.0f * 16.0f * 9.8f,
		};
	}
	ngs::Point3f GetAngularVelocity()const {
		return {
			float(_register[Register::Table::GX]) / 32768.0f * 2000,
			float(_register[Register::Table::GY]) / 32768.0f * 2000,
			float(_register[Register::Table::GZ]) / 32768.0f * 2000,
		};
	}
	ngs::uint16 Read(Register::Table reg) { return _register.at(reg); }
protected:
	void _Record(Register::Table reg, ngs::byte low, ngs::byte high) {
		_register.at(reg) = (high << 8) | low;
	}

protected:
	std::array<ngs::int16, Register::SIZE> _register = {};
};

class _JY901B_Sync : public _JY901B_Base {
public:
	using WriteCB = size_t(ngs::byte_ptr_cst, size_t);
	using ReadCB = size_t(ngs::byte_ptr, size_t);
	using WriteReadCB = bool(ngs::byte_ptr_cst, size_t, ngs::byte_ptr, size_t);
public:
	_JY901B_Sync() = default;
	_JY901B_Sync(
		std::function<WriteCB> writeCallback,
		std::function<WriteReadCB> writeReadCallback
	)
		: _write_cb(writeCallback)
		, _write_read_cb(writeReadCallback)
	{}

	void Update() {
		_isUpdate = false;
		for (auto reg : _activeRegister) {
			_isUpdate |= _write_read_cb((ngs::byte_ptr)&reg, 1, (ngs::byte_ptr)&_register[reg], sizeof(_register[reg]));
		}
	}

	void ActiveRegister(Register::Table reg) {
		_activeRegister.insert(reg);
	}
	bool IsActive()const { return _isUpdate; }

	void Write(Register::Table reg, ngs::uint16 data) {
		_WriteRegister(reg, data);
	}
private:
	void _WriteRegister(Register::Table reg, ngs::uint16 data) {
		std::array<ngs::byte, 3> writeData;
		writeData.at(0) = (ngs::byte)reg;
		writeData.at(1) = (ngs::byte)(data & 0xFF);
		writeData.at(2) = (ngs::byte)(data >> 8);
		_write_cb(writeData.data(), writeData.size());
	}
private:
	bool _isUpdate = false;
	std::function<WriteCB> _write_cb = nullptr;
	std::function<WriteReadCB> _write_read_cb = nullptr;

	std::set<Register::Table> _activeRegister = {};
};

struct ReadDataDefined {
	enum Type {
		TIME = 0x50,
		ACC,
		GYRO,
		ANGLE,
		MAGNETIC,
		DPORT,
		PRESS,
		GPS,
		VELOCITY,
		QUATER,
		GSA,
		REGISTER_VALUE = 0x5F,
	};

	static constexpr std::array<ngs::byte, 1> Header = { 0x55 };
	static constexpr size_t Size = 11;
	static size_t Parser(std::ranges::sized_range auto data) {
		ngs::Assert(std::ranges::size(data) == Size);
		if (
			!ngs::protocols_parsers::parse_range < (ngs::byte)Type::TIME, (ngs::byte)Type::GSA>(data | std::views::drop(Header.size())) &&
			!ngs::protocols_parsers::parse_equal_of<(ngs::byte)Type::REGISTER_VALUE>(data | std::views::drop(Header.size()))
			)
			return 0;

		ngs::byte sum = 0;
		std::ranges::for_each_n(std::ranges::begin(data), Size - 1, [&sum](ngs::byte byte) { sum += byte; });
		if (sum != *std::ranges::rbegin(data))
			return 0;

		return Size - Header.size();
	}
	using ParsedSequence = std::array<ngs::byte, Size>;
};

struct WriteDataDefined {
	static constexpr std::array<ngs::byte, 2> HEADER = { 0xFF,0xAA };
	static constexpr size_t SIZE = 5;
};

class _JY901B_Async : public _JY901B_Base {
public:
	using WriteCB = void(ngs::byte_ptr_cst, size_t);
	using ReadCB = bool(ngs::byte_ref);

	using ParsedSequence = std::array<ngs::byte, ReadDataDefined::Size>;
public:
	_JY901B_Async(
		const std::function<WriteCB>& writeCallback,
		const std::function<ReadCB>& readCallback
	) {
		Bind(writeCallback, readCallback);
	}
	_JY901B_Async() = default;

	void Bind(
		const std::function<WriteCB>& writeCallback,
		const std::function<ReadCB>& readCallback
	) {
		_write_cb = writeCallback;
		_read_cb = readCallback;
		_isUpdate = false;

		Write(Register::Table::RSW, _activeRDType);
	}

	void Update() {
		_ReadRegister(_activeRegister);
		constexpr size_t max_read = 4;
		_isUpdate = false;
		ngs::byte data;
		size_t i = 0;
		while (_read_cb(data) && i < max_read * ReadDataDefined::Size) {
			_parser << data;
			i++;
		}
		_parser.Update();
		while (_parser.ParsedSize())
			_Parse(_parser.ParsedPop());
	}

	template<typename _Number>
		requires (_CPT Integral<_Number>) || (std::is_enum_v<_Number>)
	void Write(Register::Table reg, _Number data) {
		_Unlock();
		_WriteRegister(reg, data);
		_Save();
	}

	template<typename... RDTypes>
		requires (std::same_as<RDTypes, ReadDataDefined::Type> && ...)
	void ReadTable(RDTypes... tables) {
		_activeRDType = 0;
		(ngs::Bits<true>(_activeRDType, ngs::bit((ngs::byte)tables - (ngs::byte)ReadDataDefined::TIME)), ...);
		if (IsActive()) {
			Write(Register::Table::RSW, _activeRDType);
		}
	}

	bool IsActive() { return _isUpdate && _write_cb && _read_cb; }

	void ActiveRegister(Register::Table reg) { _activeRegister = reg; }
private:
	void _ReadRegister(Register::Table reg) {
		//ngs::nos.Log("WIT::_ReadRegister", "read 0x%02x\n", (int)reg);
		_WriteRegister(Register::Table::READADDR, reg);
	}

	template<typename _Number>
		requires (_CPT Integral<_Number>) || (std::is_enum_v<_Number>)
	void _WriteRegister(Register::Table reg, _Number data) {
		std::array<ngs::byte, WriteDataDefined::SIZE> writeData;
		writeData.at(0) = WriteDataDefined::HEADER[0];
		writeData.at(1) = WriteDataDefined::HEADER[1];
		writeData.at(2) = (ngs::byte)reg;
		writeData.at(3) = (ngs::byte)((ngs::uint16)data & 0xFF);
		writeData.at(4) = (ngs::byte)((ngs::uint16)data >> 8);
		_write_cb(writeData.data(), WriteDataDefined::SIZE);
	}
	void _Unlock() { _WriteRegister(Register::Table::KEY, Register::Config::KEY::Unlock); }
	void _Lock() { _WriteRegister(Register::Table::KEY, Register::Config::KEY::Lock); }
	void _Save() { _WriteRegister(Register::Table::SAVE, Register::Config::SAVE::Save); }

	void _Parse(const ParsedSequence& data) {
		ReadDataDefined::Type type = (ReadDataDefined::Type)data[1];
		auto registers = _GetRegisterMapByType(type);
		for (size_t i = 0; i < 4; i++) {
			if (registers[i] == Register::Table::NULL)continue;
			_Record(registers[i], data[2 * i + 2], data[2 * i + 3]);
		}
		_isUpdate = true;
	}
	std::array<Register::Table, 4> _GetRegisterMapByType(ReadDataDefined::Type type)const {
		std::array<Register::Table, 4> registers = { Register::Table::NULL, Register::Table::NULL, Register::Table::NULL, Register::Table::NULL };
		//ngs::nos.Trace("type:%d\n", (int)type);
		switch (type)
		{
		case ReadDataDefined::TIME:
			registers = { Register::Table::YYMM, Register::Table::DDHH, Register::Table::MNSS, Register::Table::MS };
			break;
		case ReadDataDefined::REGISTER_VALUE:
		case ReadDataDefined::ACC:
			//ngs::nos.Trace("ACC\n");
			registers = { Register::Table::AX,Register::Table::AY,Register::Table::AZ,Register::Table::TEMP };
			break;
		case ReadDataDefined::GYRO:
			//ngs::nos.Trace("GYRO\n");
			registers = { Register::Table::GX, Register::Table::GY, Register::Table::GZ ,Register::Table::NULL };
			break;
		case ReadDataDefined::ANGLE:
			registers = { Register::Table::Roll, Register::Table::Pitch, Register::Table::Yaw, Register::Table::VERSION };
			break;
		case ReadDataDefined::MAGNETIC:
			registers = { Register::Table::HX, Register::Table::HY, Register::Table::HZ, Register::Table::TEMP };
			break;
		case ReadDataDefined::DPORT:
			registers = { Register::Table::D0Status, Register::Table::D1Status, Register::Table::D2Status, Register::Table::D3Status };
			break;
		case ReadDataDefined::PRESS:
			registers = { Register::Table::PressureL, Register::Table::PressureH, Register::Table::HeightL, Register::Table::HeightH };
			break;
		case ReadDataDefined::GPS:
			registers = { Register::Table::LonL, Register::Table::LonH, Register::Table::LatL, Register::Table::LatH };
			break;
		case ReadDataDefined::VELOCITY:
			registers = { Register::Table::GPSHeight, Register::Table::GPSYAW, Register::Table::GPSVL, Register::Table::GPSVH };
			break;
		case ReadDataDefined::QUATER:
			registers = { Register::Table::Q0, Register::Table::Q1, Register::Table::Q2, Register::Table::Q3 };
			break;
		case ReadDataDefined::GSA:
			registers = { Register::Table::SVNUM, Register::Table::PDOP, Register::Table::HDOP, Register::Table::VDOP };
			break;
		default:
			ngs::nos.Error("unknown type!\n");
			break;
		}
		return registers;
	}
private:
	std::function<WriteCB> _write_cb = nullptr;
	std::function<ReadCB> _read_cb = nullptr;

	bool _isUpdate = false;

	ngs::uint16 _activeRDType = {};

	Register::Table _activeRegister = {};

	ngs::ProtocolsStructParser<ReadDataDefined> _parser;
};

enum class JY901B_Mode {
	SYNC,
	ASYNC,
};

template<JY901B_Mode _Mode>
class JY901B;

template<>
class JY901B<JY901B_Mode::SYNC> : public _JY901B_Sync {
public:
	using _JY901B_Sync::_JY901B_Sync;
};

template<>
class JY901B<JY901B_Mode::ASYNC> : public _JY901B_Async {
	using _JY901B_Async::_JY901B_Async;
};

#define NULL WINDOWS_CNM_NULL
#undef WINDOWS_CNM_NULL

WIT_END
