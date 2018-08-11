#pragma once

#include "../texthook/host.h"
#include "../vnrhook/include/const.h"
#include <Psapi.h>

#include <string>
#include <sstream>
#include <regex>

std::wstring GenerateHCodeWstring(HookParam hp, DWORD processId);
HookParam ParseHCodeWstring(std::wstring HCode);