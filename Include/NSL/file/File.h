#pragma once

#include "NSL/LinuxHeader.h"
#include "NSL/Config.h"
#include "NSL/io/MultiplexIO.h"

NSL_BEGIN

class File : public ngs::DeleteCopy {
	NGS_TYPE_DEFINE(ngs::FilePath, path);
public:
	struct AccessMode {
		bool
			exist : 1 = false,
			read : 1 = false,
			write : 1 = false,
			exe : 1 = false
			;
		void Clear() {
			exist = false;
			read = false;
			write = false;
			exe = false;
		}
	};
	struct OpenMode {
		bool
			read : 1 = false,
			write : 1 = false,
			read_write : 1 = false,
			append : 1 = false,
			create : 1 = false,
			trunc : 1 = false,
			excl : 1 = false,
			text : 1 = false,
			binary : 1 = false,
			raw : 1 = false,
			temporary : 1 = false,
			no_inherit : 1 = false,
			sequntial : 1 = false,
			random : 1 = false
			;
		void Clear() {
			read = false;
			write = false;
			read_write = false;
			append = false;
			create = false;
			trunc = false;
			excl = false;
			text = false;
			binary = false;
			raw = false;
			temporary = false;
			no_inherit = false;
			sequntial = false;
			random = false;
		}
	};
	static bool Access(__path_ref_cst path) {
		AccessMode mode = {};
		mode.exist = true;
		Access(path, mode);
	}
	static bool Access(__path_ref_cst path, AccessMode flag);

	static constexpr OpenMode DefaultOpenMode() {
		return { false,false,true };
	}
public:

	File(__path_ref_cst path)
		: _path(path)
	{}
	File() = default;

	~File() {
		if (IsOpened())
			Close();
	}
	__path_ref_cst GetCurrentFilePath()const { return _path; }

	bool OpenDirectory(std::string_view dir, OpenMode mode) {
		_path.OpenDirectory(dir);
		return Open(mode);
	}
	bool OpenDirectory(std::string_view dir) { return OpenDirectory(dir, DefaultOpenMode()); }
	bool Open() { return Open(DefaultOpenMode()); }
	bool Open(OpenMode mode) { return Open(_path, mode); }
	bool Open(__path_ref_cst path) { return Open(path, DefaultOpenMode()); }
	bool Open(__path_ref_cst path, OpenMode mode);

	int Offset(int offset);
	int OffsetHead(int offset = 0);
	int OffsetTail(int offset = 0);

	bool IsOpened()const;
	void Close();

	template<size_t _N>
	int Write(const char(&data)[_N]) {
		return Write(std::string(data));
	}
	template<typename _Rng>
		requires std::ranges::contiguous_range<_Rng>&& std::ranges::sized_range<_Rng>&& std::convertible_to<std::ranges::range_value_t<_Rng>, ngs::byte>
	int Write(_Rng range) {
		return Write((ngs::byte_ptr_cst)std::ranges::cdata(range), std::ranges::size(range));
	}
	int Write(ngs::byte_ptr_cst data, size_t size);
	int Read(ngs::byte_ptr data, size_t size);
private:

private:
	__path _path = {};
	int _fd = -1;

	friend class MultIO;
};

NSL_END
