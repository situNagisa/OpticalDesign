#pragma once

#include "NSL/file/File.h"
#include "NSL/LinuxHeader.h"

NSL_BEGIN

inline bool File::Access(__path_ref_cst path, AccessMode mode) {
	ngs::byte_<sizeof(mode)> flag = 0;

	if (mode.exist) flag |= F_OK;
	if (mode.read)flag |= R_OK;
	if (mode.write)flag |= W_OK;
	if (mode.exe) flag |= X_OK;

	ngs::nos.Log("File::Access", "flag:%x\n", flag);
	return !access(path.GetFilePath().c_str(), flag);
}


inline bool File::Open(__path_ref_cst path, OpenMode mode) {
	_path = path;
	ngs::byte_<sizeof(mode)> flag = 0;

	if (mode.read)flag |= O_RDONLY;
	if (mode.write)flag |= O_WRONLY;
	if (mode.read_write)flag |= O_RDWR;
	if (mode.append)flag |= O_APPEND;
	if (mode.create)flag |= O_CREAT;
	if (mode.trunc)flag |= O_TRUNC;
	if (mode.excl)flag |= O_EXCL;
	//if (mode.text)flag |= O_TEXT;
	//if (mode.binary)flag |= O_BINARY;
	//if (mode.raw)flag |= O_RAW;
	//if (mode.temporary)flag |= O_TEMPORARY;
	//if (mode.no_inherit)flag |= O_NOINHERIT;
	//if (mode.sequntial)flag |= O_SEQUENTIAL;
	//if (mode.random)flag |= O_RANDOM;

	ngs::nos.Log("File::Open", "flag:%x\n", flag);
	_fd = open(path.GetFilePath().c_str(), flag);

	if (_fd < 0)
		ngs::nos.Error("打开 %s 失败\n", path.GetFilename().c_str());
	else
		ngs::nos.Log("File::Open", "打开 %s 成功!\n", path.GetFilename().c_str());

	return _fd >= 0;
}

inline int File::Offset(int offset) { return lseek(_fd, offset, SEEK_CUR); }
inline int File::OffsetHead(int offset) { return lseek(_fd, offset, SEEK_SET); }
inline int File::OffsetTail(int offset) { return lseek(_fd, offset, SEEK_END); }

inline bool File::IsOpened() const { return _fd >= 0; }

inline void File::Close() {
	close(_fd);
	_path.Clear();
	_fd = -1;
}

inline int File::Write(ngs::byte_ptr_cst data, size_t size) { return write(_fd, data, size); }
inline int File::Read(ngs::byte_ptr data, size_t size) { return read(_fd, data, size); }

inline ngs::void_ptr File::MemoryMap(ngs::void_ptr address, size_t length, int port, int flag, std::ptrdiff_t offset)
{
	auto map = mmap(address, length, port, flag, _fd, offset);
#if NGS_BUILD_TYPE == NGS_DEBUG
	if (map == MAP_FAILED)return nullptr;
#endif
	return map;
}


NSL_END
