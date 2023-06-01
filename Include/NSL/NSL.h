#pragma once

#include "NGS/NGS.h"

#if NGS_PLATFORM != NGS_LINUX
#error "This system is not Linux. This library can only be run on Linux."
#endif

#include "NSL/Config.h"

#include "NSL/file/File.h"
#include "NSL/file/DeviceFile.h"

#include "NSL/media/V4L2.h"

#include "NSL/media/FrameBuffer.h"


#ifdef NSL_USE_HEADER_IMPL

#include "NSL/LinuxHeader.h"

#include "NSL/file/File.hpp"
#include "NSL/io/GPIO.h"
#include "NSL/io/MultiplexIO.hpp"
#include "NSL/media/V4L2.hpp"

#include "NSL/media/FrameBuffer.hpp"

#endif
