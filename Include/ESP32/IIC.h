#pragma once

#include "ESP32/BusConcept.h"
#include "NGS/Embedded/Embedded.h"

OPT_BEGIN

class IICMaster : public ngs::I2CMaster {
private:
public:
	void SetTickToWait(ngs::uint32 ms);

	bool WriteRead(ngs::byte_ptr_cst writeData, size_t writeDataSize, ngs::byte_ptr readData, size_t readDataSize);
};

class IICSlave : public ngs::I2CSlave {
public:
	void SetTickToWait(ngs::uint32 ms);
};

OPT_END
