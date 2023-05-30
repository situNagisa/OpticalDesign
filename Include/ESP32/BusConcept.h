#pragma once

#include "ESP32/Config.h"
#include "ESP32/Defined.h"

OPT_BEGIN

template<typename T>
concept Bus_c = requires(T bus, const T bus_cst) {
	std::is_function_v<decltype(T::Open)>;
	{bus.Close()};
	{bus_cst.IsOpened()} -> std::convertible_to<bool>;
	{bus.Write(ngs::byte_ptr_cst(), size_t()) };
	{bus.Write(ngs::byte())};
	{bus.Read(ngs::byte_ptr(), size_t())};
};

OPT_END;
