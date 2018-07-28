#include "misc.h"

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
