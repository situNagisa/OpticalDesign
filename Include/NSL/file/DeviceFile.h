#pragma once

#include "NSL/LinuxHeader.h"
#include "NSL/Config.h"
#include "NSL/file/File.h"

NSL_BEGIN

class DeviceFile : public File {
public:
	using File::File;

};

NSL_END
