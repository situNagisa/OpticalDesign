#pragma once

#include "ESP32/BusConcept.h"
#include "ESP32/Defined.h"

OPT_BEGIN

class SPI {
public:

public:

	bool Open(
		ngs::pin_t MISO,
		ngs::pin_t MOSI,
		ngs::pin_t SCLK,
		ngs::pin_t SS
	);
	void Close();
	bool IsOpened()const;

	void Write(ngs::byte_ptr_cst data, size_t size);
	void Write(ngs::byte data);

	void Read(ngs::byte_ptr data, size_t size);
private:

private:

};

OPT_END
