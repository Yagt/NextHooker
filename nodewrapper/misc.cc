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

void SplitWstring(const std::wstring& s, std::vector<std::wstring>& v, const std::wstring& c)
{
	std::wstring::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::wstring::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
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

	// Attempt to make the address relative
	if (!(hp.type & MODULE_OFFSET))
		if (AutoHandle<> process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId))
			if (MEMORY_BASIC_INFORMATION info = {}; VirtualQueryEx(process, (LPCVOID)hp.address, &info, sizeof(info)))
				if (auto moduleName = Util::GetModuleFileName(processId, (HMODULE)info.AllocationBase))
				{
					hp.type |= MODULE_OFFSET;
					hp.address -= (uint64_t)info.AllocationBase;
					wcscpy_s<MAX_MODULE_SIZE>(hp.module, moduleName->c_str() + moduleName->rfind(L'\\') + 1);
				}

	code.append(L"@");
	wss << std::hex << hp.address;
	std::wstring badCode = code + wss.str();
	std::wstringstream().swap(wss);
	for (std::wstring::iterator it = badCode.begin(); it != badCode.end(); ++it)
		*it = towupper(*it);

	if (hp.type & MODULE_OFFSET) code.append(L":").append(hp.module);
	wchar_t wsFunction[120];
	swprintf(wsFunction, 120, L"%hs", hp.function);
	if (hp.type & FUNCTION_OFFSET) code.append(L":").append(std::wstring(wsFunction));

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
	
	std::wregex codepage(L"^([0-9]+)#");
	std::regex_search(HCode.cbegin(), HCode.cend(), wsm, codepage);
	if (wsm.size() != 0)
	{
		wss << std::hex << wsm[1].str();
		wss >> hp.codepage;
		std::wstringstream().swap(wss);
		HCode.erase(0, wsm[0].str().length());
	}

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


	std::vector<std::wstring> addressPieces;
	SplitWstring(HCode, addressPieces, std::wstring(L":"));

	std::wregex address(L"^@([\\dA-F]+)$");
	std::regex_search(addressPieces[0].cbegin(), addressPieces[0].cend(), wsm, address);
	if (wsm.size() == 0) return {};
	wss << std::hex << wsm[1].str();
	wss >> hp.address;
	std::wstringstream().swap(wss);

	if (addressPieces.size() > 1)
	{
		hp.type |= MODULE_OFFSET;
		wcscpy_s<MAX_MODULE_SIZE>(hp.module, addressPieces[1].c_str());
	}

	if (addressPieces.size() > 2)
	{
		hp.type |= FUNCTION_OFFSET;
		std::string sFunction;
		for (char x : addressPieces[2])
			sFunction += x;
		strcpy_s<MAX_MODULE_SIZE>(hp.function, sFunction.c_str());
	}

	if (hp.offset < 0) hp.offset -= 4;
	if (hp.split < 0) hp.split -= 4;

	return hp;
}