#pragma once

#include "NSL/media/FrameBuffer.h"
#include "NSL/LinuxHeader.h"

NSL_BEGIN

struct _FrameBufferData {
	fb_var_screeninfo var = {};
	fb_fix_screeninfo fix = {};
};

bool FrameBuffer::Open(const ngs::FilePath& path) {
	_data = ngs::New(new _FrameBufferData());
	auto& data = *reinterpret_cast<_FrameBufferData*>(_data);

	if (!_file.IsOpened() && !_file.Open(path)) {
		ngs::nos.Error("open path %s fail!\b", path.GetFilePath().c_str());
		goto err_new;
	}

	if (!_file.IOCtrl(FBIOGET_VSCREENINFO, &data.var)) {
		ngs::nos.Error("io ctrl var fail!\n");
		goto err_opened;
	}
	if (!_file.IOCtrl(FBIOGET_FSCREENINFO, &data.fix)) {
		ngs::nos.Error("io ctrl fix fail!\n");
		goto err_opened;
	}

	if (!(_screen = _file.MemoryMap(GetSize(), PROT_READ | PROT_WRITE, MAP_SHARED))) {
		ngs::nos.Error("memory map fail!\n");
		goto err_opened;
	}
	ngs::nos.Log("FrameBuffer::Open", "successfully!\n");

	return true;

err_opened:;
	_file.Close();
err_new:;
	ngs::Delete(&data);
	_data = nullptr;

	return false;
}

inline void FrameBuffer::Close() {
	auto& data = *reinterpret_cast<_FrameBufferData*>(_data);

	_file.MemoryUnMap(_screen, GetSize());
	_file.Close();
	ngs::Delete(&data);
	_data = nullptr;
}

inline size_t FrameBuffer::GetSize() const
{
	auto& data = *reinterpret_cast<_FrameBufferData*>(_data);
	return data.fix.line_length * data.var.yres;
}

inline size_t FrameBuffer::GetWidth() const
{
	auto& data = *reinterpret_cast<_FrameBufferData*>(_data);
	return data.var.xres;
}

inline size_t FrameBuffer::GetHeight() const
{
	auto& data = *reinterpret_cast<_FrameBufferData*>(_data);
	return data.var.yres;
}


NSL_END
