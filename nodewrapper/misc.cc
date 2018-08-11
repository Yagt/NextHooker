#include "misc.h"
#include <iostream>

DWORD Hash(std::wstring module)
{
	for (std::wstring::iterator it = module.begin(); it != module.end(); ++it)
		*it = towlower(*it);
	DWORD hash = 0;
	for (auto i : module) hash = _rotr(hash, 7) + i;
	return hash;
}

std::wstring GenerateHCodeWstring(HookParam hp, DWORD processId)
{
	std::wstringstream wss;
	std::wstring code(L"/H");
	if (hp.type & USING_UNICODE)
	{
		if (hp.type & USING_STRING)
			code.append(L"Q");
		else
			code.append(L"W");
	}
	else
	{
		if (hp.type & USING_STRING)
			code.append(L"S");
		else if (hp.type & BIG_ENDIAN)
			code.append(L"A");
		else
			code.append(L"B");
	}
	if (hp.type & NO_CONTEXT)
		code.append(L"N");
	if (hp.offset >> 31) 
	{
		wss << std::hex << -(hp.offset + 4);
		code.append(L"-" + wss.str());
		std::wstringstream().swap(wss);
	}
	else
	{
		wss << std::hex << hp.offset;
		code.append(wss.str());
		std::wstringstream().swap(wss);
	}
	if (hp.type & DATA_INDIRECT)
	{
		if (hp.index >> 31)
		{
			wss << std::hex << -hp.index;
			code.append(L"*-" + wss.str());
			std::wstringstream().swap(wss);
		}
		else
		{
			wss << std::hex << hp.index;
			code.append(L"*" + wss.str());
			std::wstringstream().swap(wss);
		}
	}
	if (hp.type & USING_SPLIT)
	{
		if (hp.split >> 31)
		{
			wss << std::hex << -(hp.split + 4);
			code.append(L":-" + wss.str());
			std::wstringstream().swap(wss);
		}
		else
		{
			wss << std::hex << hp.split;
			code.append(L":" + wss.str());
			std::wstringstream().swap(wss);
		}
	}
	if (hp.type & SPLIT_INDIRECT)
	{
		if (hp.split_index >> 31)
		{
			wss << std::hex << -hp.split_index;
			code.append(L"*-" + wss.str());
			std::wstringstream().swap(wss);
		}
		else
		{
			wss << std::hex << hp.split_index;
			code.append(L"*" + wss.str());
			std::wstringstream().swap(wss);
		}
	}
	code.append(L"@");
	wss << std::hex << hp.address;
	std::wstring badCode = code + wss.str();
	std::wstringstream().swap(wss);
	for (std::wstring::iterator it = badCode.begin(); it != badCode.end(); ++it)
		*it = towupper(*it);

	HANDLE processHandle;
	if (!(processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId))) return badCode;
	MEMORY_BASIC_INFORMATION info;
	if (!VirtualQueryEx(processHandle, (LPCVOID)hp.address, &info, sizeof(info))) return badCode;
	wchar_t buffer[MAX_PATH];
	if (!GetModuleFileNameExW(processHandle, (HMODULE)info.AllocationBase, buffer, MAX_PATH)) return badCode;

	wss << std::hex << hp.address - (DWORD)info.AllocationBase;
	code.append(wss.str() + L":");
	for (std::wstring::iterator it = code.begin(); it != code.end(); ++it)
		*it = towupper(*it);
	code.append(wcsrchr(buffer, L'\\') + 1);
	return code;
}

HookParam ParseHCodeWstring(std::wstring HCode)
{
	HookParam hp = {};
	for (std::wstring::iterator it = HCode.begin(); it != HCode.end(); ++it)
		*it = towupper(*it);

	if (HCode.find(L"/H") != 0) return {};
	HCode.erase(0, 2);

	if (HCode.length() < 1) return {};
	switch (HCode[0])
	{
	case L'S':
		hp.type |= USING_STRING;
		break;
	case L'A':
		hp.type |= BIG_ENDIAN;
		hp.length_offset = 1;
		break;
	case L'B':
		hp.length_offset = 1;
		break;
	case L'Q':
		hp.type |= USING_STRING | USING_UNICODE;
		break;
	case L'W':
		hp.type |= USING_UNICODE;
		hp.length_offset = 1;
		break;
	case L'V':
		hp.type |= USING_STRING | USING_UTF8;
		break;
	default:
		return {};
	}
	HCode.erase(0, 1);

	if (HCode.length() < 1) return {};
	if (HCode[0] == L'N')
	{
		hp.type |= NO_CONTEXT;
		HCode.erase(0, 1);
	}

	std::wstringstream wss;
	std::wsmatch wsm;

	std::wregex dataOffset(L"\\-?[\\dA-F]+");
	std::regex_search(HCode.cbegin(), HCode.cend(), wsm, dataOffset);
	if (wsm.size() == 0) return {};
	wss << std::hex << wsm[0].str();
	wss >> hp.offset;
	std::wstringstream().swap(wss);
	HCode.erase(0, wsm[0].str().length());

	std::wregex dataIndirect(L"^\\*(\\-?[\\dA-F]+)");
	std::regex_search(HCode.cbegin(), HCode.cend(), wsm, dataIndirect);
	if (wsm.size() != 0)
	{
		hp.type |= DATA_INDIRECT;
		wss << std::hex << wsm[1].str();
		wss >> hp.index;
		std::wstringstream().swap(wss);
		HCode.erase(0, wsm[0].str().length());
	}

	std::wregex split(L"^\\:(\\-?[\\dA-F]+)");
	std::regex_search(HCode.cbegin(), HCode.cend(), wsm, split);
	if (wsm.size() != 0)
	{
		hp.type |= USING_SPLIT;
		wss << std::hex << wsm[1].str();
		wss >> hp.split;
		std::wstringstream().swap(wss);
		HCode.erase(0, wsm[0].str().length());

		std::wregex splitIndirect(L"^\\*(\\-?[\\dA-F]+)");
		std::regex_search(HCode.cbegin(), HCode.cend(), wsm, splitIndirect);
		if (wsm.size() != 0)
		{
			hp.type |= SPLIT_INDIRECT;
			wss << std::hex << wsm[1].str();
			wss >> hp.split_index;
			std::wstringstream().swap(wss);
			HCode.erase(0, wsm[0].str().length());
		}
	}

	if (HCode.length() < 1 || HCode[0] != L'@') return {};
	HCode.erase(0, 1);

	std::wregex address(L"^([\\dA-F]+):?");
	std::regex_search(HCode.cbegin(), HCode.cend(), wsm, address);
	if (wsm.size() == 0) return {};
	wss << std::hex << wsm[1].str();
	wss >> hp.address;
	std::wstringstream().swap(wss);
	HCode.erase(0, wsm[0].str().length());

	if (HCode.length())
	{
		hp.type |= MODULE_OFFSET;
		hp.module = Hash(HCode);
	}
	if (hp.offset & 0x80000000)
		hp.offset -= 4;
	if (hp.split & 0x80000000)
		hp.split -= 4;
	return hp;
}