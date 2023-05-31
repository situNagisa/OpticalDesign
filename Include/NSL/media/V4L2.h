#pragma once

#include "NSL/Config.h"
#include "NSL/file/DeviceFile.h"
#include "NSL/Defined.h"

NSL_BEGIN

class V4L2 : public NSLDevice {
public:

public:

	bool Open(const ngs::FilePath& path);
	bool IsOpened()const override;
	void Close();

	void LoadDeviceDescription();

	std::vector<ngs::PixelFormat> GetSupportPixelFormat()const;
	bool IsSupport(ngs::PixelFormat)const;

	void Update();

	bool Initialize();
	bool Initialize(size_t width, size_t height, ngs::PixelFormat format) {
		if (!SetFormat(width, height, format))return false;
		return Initialize();
	}
	bool Initialize(ngs::byte frameRate) {
		if (!SetFrameRate(frameRate))return false;
		return Initialize();
	}
	bool Initialize(size_t width, size_t height, ngs::PixelFormat format, ngs::byte frameRate) {
		if (!SetFormat(width, height, format))return false;
		if (!SetFrameRate(frameRate))return false;
		return Initialize();
	}

	bool SetFormat(size_t width, size_t height, ngs::PixelFormat format);
	bool SetFrameRate(ngs::byte frameRate);

	bool Read(ngs::void_ptr data);

	const ngs::Point2s& GetSize()const;
	size_t GetFrameBufferSize()const;

	const size_t GetWidth()const { return GetSize().x; }
	const size_t GetHeight()const { return GetSize().y; }

private:

private:

};

NSL_END
