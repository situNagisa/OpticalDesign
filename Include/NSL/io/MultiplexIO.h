#pragma once

#include "NSL/Config.h"

NSL_BEGIN

class File;

class MultIO {
	template<class T>
	using Container = std::vector<T>;
public:
	struct Tag {
		bool
			in : 1 = false,
			read_norm : 1 = false,
			read_band : 1 = false,
			pri : 1 = false,
			read_hup : 1 = false,

			out : 1 = false,
			write_norm : 1 = false,
			write_band : 1 = false,

			error : 1 = false,
			hup : 1 = false,
			no_val : 1 = false,

			message : 1 = false
			;
		void Clear() { *this = {}; }
		operator ngs::int16()const;
		Tag& operator=(ngs::int16);
	};
private:
	NGS_TYPE_DEFINE(File, file);
	using ___element = std::tuple<__file_ptr_cst, Tag, Tag>;
	NGS_TYPE_DEFINE(___element, element);
public:
	void Add(__file_ptr_cst file, Tag tag) { _files.emplace_back(std::make_tuple(file, tag, Tag{})); }

	int Poll(int timeout);

	const Container<__element>& GetFiles()const { return _files; }
private:

private:
	Container < __element> _files;
	ngs::void_ptr _data = nullptr;
};

NSL_END
