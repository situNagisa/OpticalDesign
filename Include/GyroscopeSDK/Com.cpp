#include "GyroscopeSDK/Com.h"
#include <Windows.h>
#include <tchar.h>

static constexpr size_t _BUFFER_SIZE = 1024;

using namespace std::chrono_literals;

struct WindowsData {
	HANDLE COM;
	OVERLAPPED readStatus;
	OVERLAPPED writeStatus;
	ngs::byte sendBuffer[_BUFFER_SIZE];
	ngs::byte readBuffer[_BUFFER_SIZE];
};

void COM::Send(ngs::byte_ptr_cst message, size_t length)
{
	WindowsData& data = *(WindowsData*)_windowsData;

	DCB DCB;

	GetCommState(data.COM, &DCB);
	DCB.fDtrControl = 0;//DTR = 1;sending
	SetCommState(data.COM, &DCB);
	//return;
	{
		DWORD iR;
		if (WriteFile(data.COM, message, length, &iR, &(data.writeStatus)) || GetLastError() != ERROR_IO_PENDING)
			return;
	}

	//auto err = WaitForSingleObject(data.writeStatus.hEvent, 1);
	std::this_thread::sleep_for(10us);
	DCB.fDtrControl = 1;//DTR = 0;receive
	SetCommState(data.COM, &DCB);
}

bool COM::Receive(ngs::byte_ref message) {
	WindowsData& data = *(WindowsData*)_windowsData;
	//return false;
	DWORD length = 0;
	ngs::byte buffer[_BUFFER_SIZE];
	if (!ReadFile(data.COM, buffer, 1, &length, &(data.readStatus)))
		return false;

	/*if (GetLastError()) {
		COMSTAT COMStatus;
		DWORD errorFlags;
		ClearCommError(data.COM, &errorFlags, &COMStatus);
		return false;
	}*/
	message = buffer[0];
	//WaitForSingleObject(data.readStatus.hEvent, INFINITE);
	//if (!GetOverlappedResult(data.COM, &(data.readStatus), &length, FALSE))return false;
	//if (length <= 0)return false;

	return true;
}

void COM::SetBaudRate(ngs::uint32 rate)
{
	WindowsData& data = *(WindowsData*)_windowsData;

	DCB DCB;
	GetCommState(data.COM, &DCB);
	DCB.BaudRate = rate;
	SetCommState(data.COM, &DCB);
}

bool COM::Open(ngs::uint32 port, ngs::uint32 rate)
{
	HANDLE COM;
	{
		TCHAR PortName[10] = { '\\','\\','.','\\','C','O','M',0,0,0 };//"\\\\.\\COM";
		TCHAR chrTemple[5] = {};

		_itot_s(port, chrTemple, 10);
		_tcscat_s(PortName, chrTemple);

		if ((COM = CreateFile(PortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0)) == INVALID_HANDLE_VALUE)return false;
	}

	_windowsData = ngs::New(new WindowsData());
	WindowsData& data = *(WindowsData*)_windowsData;

	data.COM = COM;
	{
		DCB DCB;
		SetupComm(COM, _BUFFER_SIZE, _BUFFER_SIZE);
		GetCommState(COM, &DCB);
		DCB.BaudRate = rate;
		DCB.fParity = NOPARITY;
		DCB.ByteSize = 8;
		DCB.fDtrControl = 0;//DTR = 0;receive
		DCB.fRtsControl = 0;//RTS = 0;receive
		DCB.StopBits = ONESTOPBIT;

		COMSTAT state;
		DWORD errorFlag;
		SetCommState(COM, &DCB);
		ClearCommError(COM, &errorFlag, &state);
	}
	{
		COMMTIMEOUTS COMTimeOut;
		COMTimeOut.ReadIntervalTimeout = 5;
		COMTimeOut.ReadTotalTimeoutMultiplier = 10;
		COMTimeOut.ReadTotalTimeoutConstant = 100;
		COMTimeOut.WriteTotalTimeoutMultiplier = 5;
		COMTimeOut.WriteTotalTimeoutConstant = 5;
		SetCommTimeouts(COM, &COMTimeOut);
	}
	{
		data.writeStatus.hEvent = CreateEvent(0, TRUE, FALSE, 0);
		data.readStatus.hEvent = CreateEvent(0, TRUE, FALSE, 0);
		data.readStatus.Internal = 0;
		data.readStatus.InternalHigh = 0;
		data.readStatus.Offset = 0;
		data.readStatus.OffsetHigh = 0;
	}
	_isOpen = true;
	return true;
}

void COM::Close()
{
	WindowsData& data = *(WindowsData*)_windowsData;

	PurgeComm(data.COM, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	CloseHandle(data.readStatus.hEvent);
	CloseHandle(data.writeStatus.hEvent);
	CloseHandle(data.COM);

	ngs::Delete(_windowsData);
	_windowsData = nullptr;
	_isOpen = false;
}
