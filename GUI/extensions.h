#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <Windows.h>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <QComboBox>

std::map<int, QString> LoadExtensions();
std::wstring DispatchSentenceToExtensions(std::wstring sentence, std::unordered_map<std::string, int> miscInfo);
struct InfoForExtension
{
	char* propertyName;
	int propertyValue;
	InfoForExtension* nextProperty;
};
typedef const wchar_t*(*ExtensionFunction)(const wchar_t*, const InfoForExtension*);
extern QComboBox* ttCombo;

#endif // EXTENSIONS_H
