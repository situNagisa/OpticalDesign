#pragma once

#include "NGS/NGS.h"

#if NGS_PLATFORM == NGS_WINDOWS
#include "OpticalDesign/WindowsDevice.h"
#elif NGS_PLATFORM == NGS_LINUX
#include "OpticalDesign/LinuxDevice.h"
#endif
