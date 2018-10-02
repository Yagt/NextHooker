#pragma once

#include "../include/const.h"
#include "../vnrhook/host/host.h"
#include <Psapi.h>

#include <string>
#include <sstream>
#include <regex>

std::wstring GenerateHCodeWstring(HookParam hp, DWORD processId);
HookParam ParseHCodeWstring(std::wstring HCode);