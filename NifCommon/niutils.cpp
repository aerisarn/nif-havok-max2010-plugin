/**********************************************************************
*<
FILE: NIUtils.cpp

DESCRIPTION:	NifImporter Utilities

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#include "pch.h"
#include "niutils.h"
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <malloc.h>
#include <sstream>
#include <modstack.h>
#include <iparamb2.h>
#include <iskin.h>
#include "../NifProps/bhkRigidBodyInterface.h"
#include <stdmat.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/RigidBody/hctRigidBodyModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Objects/TaperedCapsule/hctTaperedCapsuleObjectInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctConstraintModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctBallAndSocketConstraintModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctHingeConstraintModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctPrismaticConstraintModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctRagdollConstraintModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctStiffSpringConstraintModifierInterface.h>
//#include <ContentTools/Max/MaxFpInterfaces/Physics/Constraints/hctWheelConstraintModifierInterface.h>

#ifdef USE_BIPED
#  include <cs/BipedApi.h>
#  include <cs/OurExp.h> 
#endif
#if VERSION_3DSMAX < (14000<<16) // Version 14 (2012)
#include "maxscrpt\Strings.h"
#include "maxscrpt\Parser.h"
#else
#include <maxscript/maxscript.h>
#include <maxscript/compiler/parser.h>
#endif

using namespace std;
using namespace Niflib;

// Macro to create a dynamically allocated strdup on the stack
#define STRDUPA(p) (strcpy((char*)alloca((strlen(p)+1)*sizeof(*p)),p))
#define STRDUPW(p) (wcscpy((wchar_t*)alloca((wcslen(p)+1)*sizeof(*p)),p))

// sprintf for TSTR without having to worry about buffer size.
TSTR FormatText(const char* format, ...)
{
	USES_CONVERSION;
	TSTR text;
	va_list args;
	va_start(args, format);
#ifdef UNICODE
	size_t Size = _vscprintf(format, args)+1;
	text.Resize(int(Size));
	LPSTR buffer = static_cast<LPSTR>(alloca(Size*sizeof(char)));
	_vsnprintf(buffer, Size, format, args);
	mbstowcs(DataForWrite(text), buffer, Size);
#else
	char buffer[512];
	int nChars = _vsnprintf(buffer, _countof(buffer), format, args);
	if (nChars != -1) {
		return TSTR(A2T(buffer));
	}
	size_t Size = _vscprintf(format, args);
	text.Resize(Size);
	nChars = _vsnprintf(DataForWrite(text), Size, format, args);
#endif
	va_end(args);
	return text;
}
TSTR FormatText(const wchar_t* format, ...)
{
	USES_CONVERSION;
	TSTR text;
	va_list args;
	va_start(args, format);
#ifndef UNICODE
	size_t Size = _vscwprintf(format, args) + 1;
	text.Resize(Size);
	LPWSTR buffer = static_cast<LPWSTR>(alloca(Size*sizeof(wchar_t)));
	_vsnwprintf(buffer, Size, format, args);
	wcstombs(DataForWrite(text), buffer, Size);
#else
	wchar_t buffer[512];
	int nChars = _vsnwprintf(buffer, _countof(buffer), format, args);
	if (nChars != -1) {
		return TSTR(W2T(buffer));
	}
	size_t Size = _vscwprintf(format, args);
	text.Resize(int(Size));
	nChars = _vsnwprintf(DataForWrite(text), Size, format, args);
#endif
	va_end(args);
	return text;
}

// sprintf for std::string without having to worry about buffer size.
std::string FormatString(const char* format, ...)
{
	char buffer[512];
	std::string text;
	va_list args;
	va_start(args, format);
	int nChars = _vsnprintf(buffer, _countof(buffer), format, args);
	if (nChars != -1) {
		text = buffer;
	}
	else {
		size_t Size = _vscprintf(format, args);
		LPSTR pbuf = LPSTR(_alloca(Size));
		_vsnprintf(pbuf, Size, format, args);
		text = pbuf;
	}
	va_end(args);
	return text;
}

// sprintf for std::string without having to worry about buffer size.
std::wstring FormatString(const wchar_t* format, ...)
{
	wchar_t buffer[512];
	std::wstring text;
	va_list args;
	va_start(args, format);
	int nChars = _vsnwprintf(buffer, _countof(buffer), format, args);
	if (nChars != -1) {
		text = buffer;
	}
	else {
		size_t Size = _vscwprintf(format, args);
		LPWSTR pbuf = LPWSTR(_alloca(Size*sizeof(wchar_t)));
		_vsnwprintf(pbuf, Size, format, args);
		text = pbuf;
	}
	va_end(args);
	return text;
}


// routine for parsing white space separated lines.  Handled like command line parameters w.r.t quotes.
static void parse_line(const char *start, char **argv, char *args, int *numargs, int *numchars)
{
	const char NULCHAR = '\0', SPACECHAR = ' ', TABCHAR = '\t', RETURNCHAR = '\r', LINEFEEDCHAR = '\n', DQUOTECHAR = '\"', SLASHCHAR = '\\';
	const char *p;
	int inquote, copychar;          
	unsigned int numslash;
	*numchars = *numargs = 0;          
	p = start;
	inquote = 0;
	for (;;)
	{
		if (*p) { while (*p == SPACECHAR || *p == TABCHAR || *p == RETURNCHAR || *p == LINEFEEDCHAR) ++p; }
		if (*p == NULCHAR) break;
		if (argv) *argv++ = args;
		++*numargs;
		for (;;)
		{
			copychar = 1;
			numslash = 0;
			while (*p == SLASHCHAR) { ++p; ++numslash; }
			if (*p == DQUOTECHAR)
			{
				if (numslash % 2 == 0) {
					if (inquote && p[1] == DQUOTECHAR) { p++; }
					else copychar = 0;
					inquote = !inquote;
				}
				numslash /= 2;
			}
			while (numslash--) {
				if (args) *args++ = SLASHCHAR;
				++*numchars;
			}
			if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR || *p == RETURNCHAR || *p == LINEFEEDCHAR)))
				break;
			if (copychar) {
				if (args) *args++ = *p;
				++*numchars;
			}
			++p;
		}
		if (args) *args++ = NULCHAR;
		++*numchars;
	}
	if (argv) *argv++ = nullptr;
	++*numargs;
}

static void parse_line(const wchar_t *start, wchar_t **argv, wchar_t *args, int *numargs, int *numchars)
{
	const wchar_t NULCHAR = L'\0', SPACECHAR = L' ', TABCHAR = L'\t', RETURNCHAR = L'\r', LINEFEEDCHAR = L'\n', DQUOTECHAR = L'\"', SLASHCHAR = L'\\';
	const wchar_t *p;
	int inquote, copychar;       
	unsigned int numslash;  
	*numchars = *numargs = 0;       
	p = start;
	inquote = 0;
	for (;;)
	{
		if (*p) { while (*p == SPACECHAR || *p == TABCHAR || *p == RETURNCHAR || *p == LINEFEEDCHAR) ++p; }
		if (*p == NULCHAR) break;
		if (argv) *argv++ = args;
		++*numargs;
		for (;;)
		{
			copychar = 1;
			numslash = 0;
			while (*p == SLASHCHAR) { ++p; ++numslash; }
			if (*p == DQUOTECHAR) {
				if (numslash % 2 == 0) {
					if (inquote && p[1] == DQUOTECHAR) { p++; }
					else copychar = 0;
					inquote = !inquote;
				}
				numslash /= 2;
			}
			while (numslash--) {
				if (args) *args++ = SLASHCHAR;
				++*numchars;
			}
			if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR || *p == RETURNCHAR || *p == LINEFEEDCHAR)))
				break;
			if (copychar) {
				if (args) *args++ = *p;
				++*numchars;
			}
			++p;
		}
		if (args) *args++ = NULCHAR;
		++*numchars;
	}
	if (argv) *argv++ = nullptr;
	++*numargs;
}

// Tokenize a string using strtok and return it as a stringlist
stringlist TokenizeString(LPCSTR str, LPCSTR delims, bool trim)
{
	stringlist values;
	LPSTR buf = STRDUPA(str);
	for (LPSTR p = strtok(buf, delims); p && *p; p = strtok(nullptr, delims)) {
		values.push_back(string((trim) ? Trim(p) : p));
	}
	return values;
}
wstringlist TokenizeString(LPCWSTR str, LPCWSTR delims, bool trim)
{
	wstringlist values;
	LPWSTR buf = STRDUPW(str);
	for (LPWSTR p = wcstok(buf, delims); p && *p; p = wcstok(nullptr, delims)) {
		values.push_back(wstring((trim) ? Trim(p) : p));
	}
	return values;
}

// Tokenize a string using strtok and return it as a stringlist
stringlist TokenizeCommandLine(LPCSTR str, bool trim)
{
	stringlist values;
	int nargs = 0, nchars = 0;
	parse_line(str, nullptr, nullptr, &nargs, &nchars);
	void *buffer = _alloca(nargs * sizeof(char *) + nchars * sizeof(char));
	char **largv = static_cast<char **>(buffer);
	parse_line(str, largv, static_cast<char*>(buffer) + nargs * sizeof(char*), &nargs, &nchars);
	for (int i = 0; i < nargs; ++i) {
		LPSTR p = largv[i];
		if (p == nullptr) continue;
		values.push_back(string((trim) ? Trim(p) : p));
	}
	return values;
}
wstringlist TokenizeCommandLine(LPCWSTR str, bool trim)
{
	wstringlist values;
	int nargs = 0, nchars = 0;
	parse_line(str, nullptr, nullptr, &nargs, &nchars);
	void *buffer = _alloca(nargs * sizeof(wchar_t *) + nchars * sizeof(wchar_t));
	wchar_t **largv = static_cast<wchar_t **>(buffer);
	parse_line(str, largv, reinterpret_cast<wchar_t*>(static_cast<char*>(buffer) + nargs * sizeof(wchar_t*)), &nargs, &nchars);
	for (int i = 0; i < nargs; ++i) {
		LPWSTR p = largv[i];
		if (p == nullptr) continue;
		values.push_back(wstring((trim) ? Trim(p) : p));
	}
	return values;
}

string JoinCommandLine(stringlist args)
{
	std::stringstream str;
	for (stringlist::iterator itr = args.begin(); itr != args.end(); ++itr) {
		if (itr != args.begin()) str << ' ';
		str << (*itr);
	}
	return str.str();
}

wstring JoinCommandLine(wstringlist args)
{
	wstringstream str;
	for (wstringlist::iterator itr = args.begin(); itr != args.end(); ++itr) {
		if (itr != args.begin()) str << ' ';
		str << (*itr);
	}
	return str.str();
}

// Parse and ini file section and return the results as s NameValueCollection.
NameValueCollectionA ReadIniSection(LPCSTR Section, LPCSTR iniFileName)
{
	NameValueCollectionA map;
	DWORD len = 2048 * sizeof(char);
	LPSTR buf = (LPSTR)calloc(len + 2, 1);
	while (nullptr != buf) {
		DWORD rlen = GetPrivateProfileSectionA(Section, buf, len, iniFileName);
		if (rlen != (len - 2)) break;
		len += 2;
		buf = (LPSTR)realloc(buf, len);
	}
	if (nullptr != buf) {
		for (LPSTR line = buf, next = line + strlen(line) + 1; *line; line = next, next = line + strlen(line) + 1) {
			Trim(line);
			if (line[0] == ';' || line[0] == 0)
				continue;
			if (LPSTR equals = strchr(line, '=')) {
				*equals++ = 0;
				Trim(line), Trim(equals);
				map[string(line)] = string(equals);
			}
		}
	}
	return map;
}

NameValueCollectionW ReadIniSection(LPCWSTR Section, LPCWSTR iniFileName)
{
	NameValueCollectionW map;
	DWORD len = 2048 * sizeof(wchar_t);
	LPWSTR buf = (LPWSTR)calloc(len + 2, 1);
	while (nullptr != buf) {
		DWORD rlen = GetPrivateProfileSectionW(Section, buf, len, iniFileName);
		if (rlen != (len - 2)) break;
		len += 2;
		buf = (LPWSTR)realloc(buf, len*sizeof(wchar_t));
	}
	if (nullptr != buf) {
		for (LPWSTR line = buf, next = line + wcslen(line) + 1; *line; line = next, next = line + wcslen(line) + 1) {
			Trim(line);
			if (line[0] == ';' || line[0] == 0)
				continue;
			if (LPWSTR equals = wcschr(line, L'=')) {
				*equals++ = 0;
				Trim(line), Trim(equals);
				map[wstring(line)] = wstring(equals);
			}
		}
	}
	return map;
}


// Parse and ini file section and return the results as s NameValueCollection.
bool ReadIniSectionAsList(LPCSTR Section, LPCSTR iniFileName, NameValueListA& map)
{
	
	DWORD len = 2048 * sizeof(char);
	LPSTR buf = (LPSTR)calloc(len + 2, 1);
	while (nullptr != buf) {
		DWORD rlen = GetPrivateProfileSectionA(Section, buf, len, iniFileName);
		if (rlen != (len - 2)) break;
		len += 2;
		buf = (LPSTR)realloc(buf, len);
	}
	if (nullptr == buf) 
		return false;

	for (LPSTR line = buf, next = line + strlen(line) + 1; *line; line = next, next = line + strlen(line) + 1) {
		Trim(line);
		if (line[0] == ';' || line[0] == 0)
			continue;
		if (LPSTR equals = strchr(line, '=')) {
			*equals++ = 0;
			Trim(line), Trim(equals);
			map.push_back(KeyValuePairA(line, equals));
		}
	}
	return true;
}

bool ReadIniSectionAsList(LPCWSTR Section, LPCWSTR iniFileName, NameValueListW& map)
{
	DWORD len = 2048 * sizeof(wchar_t);
	LPWSTR buf = (LPWSTR)calloc(len + 2, 1);
	while (nullptr != buf) {
		DWORD rlen = GetPrivateProfileSectionW(Section, buf, len, iniFileName);
		if (rlen != (len - 2)) break;
		len += 2;
		buf = (LPWSTR)realloc(buf, len*sizeof(wchar_t));
	}
	if (nullptr == buf)
		return false;
	for (LPWSTR line = buf, next = line + wcslen(line) + 1; *line; line = next, next = line + wcslen(line) + 1) {
		Trim(line);
		if (line[0] == ';' || line[0] == 0)
			continue;
		if (LPWSTR equals = wcschr(line, L'=')) {
			*equals++ = 0;
			Trim(line), Trim(equals);
			map.push_back(KeyValuePairW(line,equals));
		}
	}
	return true;
}

// Expand Qualifiers in string using a ${Name} syntax.  Name will be looked up in the
//    NameValueCollection and expand in place.  Missing names will expand to empty.
//    - Please dont give self-referential strings
string ExpandQualifiers(const string& src, const NameValueCollectionA& map)
{
	string value;
	bool hasPercent = false;
	string::size_type end = src.length();
	value.reserve(src.length());
	for (string::size_type i = 0; i < end; ++i) {
		if (src[i] == '$') {
			if (++i < end) {
				if (src[i] == '$') {
					value.append(1, src[i]);
				}
				else if (src[i] == '{') {
					string::size_type term = src.find_first_of('}', i);
					if (term == string::npos) {
						i = end;
					}
					else {
						string key = src.substr(i + 1, term - i - 1);
						NameValueCollectionA::const_iterator kvp = map.find(key);
						if (kvp != map.end()) {
							value.append(ExpandQualifiers(kvp->second, map));
						}
						i = term;
					}
				}
				else if (src[i] == '(') {
					string::size_type term = src.find_first_of(')', i);
					if (term == string::npos) {
						i = end;
					}
					else {
						string key = src.substr(i + 1, term - i - 1);
						NameValueCollectionA::const_iterator kvp = map.find(key);
						if (kvp != map.end()) {
							value.append(ExpandQualifiers(kvp->second, map));
						}
						i = term;
					}
				}
			}
			else {
				value.append(1, '$');
			}
		}
		else {
			value.append(1, src[i]);
		}
	}
	return value;
}
wstring ExpandQualifiers(const wstring& src, const NameValueCollectionW& map)
{
	wstring value;
	bool hasPercent = false;
	wstring::size_type end = src.length();
	value.reserve(src.length());
	for (wstring::size_type i = 0; i < end; ++i) {
		if (src[i] == '$') {
			if (++i < end) {
				if (src[i] == '$') {
					value.append(1, src[i]);
				}
				else if (src[i] == '{') {
					wstring::size_type term = src.find_first_of('}', i);
					if (term == string::npos) {
						i = end;
					}
					else {
						wstring key = src.substr(i + 1, term - i - 1);
						NameValueCollectionW::const_iterator kvp = map.find(key);
						if (kvp != map.end()) {
							value.append(ExpandQualifiers(kvp->second, map));
						}
						i = term;
					}
				}
				else if (src[i] == '(') {
					wstring::size_type term = src.find_first_of(')', i);
					if (term == string::npos) {
						i = end;
					}
					else {
						wstring key = src.substr(i + 1, term - i - 1);
						NameValueCollectionW::const_iterator kvp = map.find(key);
						if (kvp != map.end()) {
							value.append(ExpandQualifiers(kvp->second, map));
						}
						i = term;
					}
				}
			}
			else {
				value.append(1, '$');
			}
		}
		else {
			value.append(1, src[i]);
		}
	}
	return value;
}

// Call ExpandEnvironmentStrings but with std string instead
string ExpandEnvironment(const string& src)
{
	DWORD Size = ExpandEnvironmentStringsA(src.c_str(), nullptr, 0);
	int nChar = (Size + 2)*sizeof(char);
	if (char* pbuf = (char*)_alloca(nChar)) {
		pbuf[0] = 0;
		ExpandEnvironmentStringsA(src.c_str(), pbuf, Size+2);
		return string(pbuf);
	}
	return src;
}
wstring ExpandEnvironment(const wstring& src)
{
	DWORD Size = ExpandEnvironmentStringsW(src.c_str(), nullptr, 0);
	int nChar = (Size + 2)*sizeof(wchar_t);
	if (wchar_t* pbuf = (wchar_t*)_alloca(nChar)) {
		pbuf[0] = 0;
		ExpandEnvironmentStringsW(src.c_str(), pbuf, Size + 2);
		return wstring(pbuf);
	}
	return src;
}


// Helper struct and method for dealing with standard registry handles
struct str2hdlA { const char *str; HANDLE key; };
const static struct str2hdlA RegKeyMapA[] = {
   {"HKLM", HKEY_LOCAL_MACHINE},
   {"HKCU", HKEY_CURRENT_USER},
   {"HKCR", HKEY_CLASSES_ROOT},
   {"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
   {"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
   {"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT},
};

static HANDLE GetRegKey(LPCSTR key) {
	for (int i = 0; i < _countof(RegKeyMapA); ++i)
		if (0 == strcmp(RegKeyMapA[i].str, key))
			return RegKeyMapA[i].key;
	return 0;
}
struct str2hdlW { const wchar_t *str; HANDLE key; };
const static struct str2hdlW RegKeyMapW[] = {
	{ L"HKLM", HKEY_LOCAL_MACHINE },
	{ L"HKCU", HKEY_CURRENT_USER },
	{ L"HKCR", HKEY_CLASSES_ROOT },
	{ L"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE },
	{ L"HKEY_CURRENT_USER", HKEY_CURRENT_USER },
	{ L"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT },
};
static HANDLE GetRegKey(LPCWSTR key) {
	for (int i = 0; i < _countof(RegKeyMapW); ++i)
		if (0 == wcscmp(RegKeyMapW[i].str, key))
			return RegKeyMapW[i].key;
	return 0;
}

// Returns value from indirect source
//  Currently only supports registry string values using '[HKEY\Key]=@"Value"' 
//  where  HKEY is HKLM,HKCU,HKCR  
//         Key is the registry key to lookup
//         Value is the data value to lookup.  
string GetIndirectValue(LPCSTR path)
{
	if (!path || !*path)
		return string();
	string value;
	LPSTR p = STRDUPA(path);
	Trim(p);
	if (*p == '[') {
		LPSTR end = strchr(++p, ']');
		if (end != nullptr) {
			*end++ = 0;
			// Advance unsafely past unnecessary qualifiers
			LPSTR valueName = end;
			end = valueName + strlen(end) - 1;
			if (*valueName == '=') ++valueName;
			if (*valueName == '@') ++valueName;
			if (*valueName == '\"' || *valueName == '\'') ++valueName;
			if (*end == '\"' || *end == '\'') *end-- = 0;
			Trim(valueName);
			if (strlen(valueName) == 0)
				valueName = nullptr;

			LPSTR keyEnd = strchr(p, '\\');
			if (keyEnd != nullptr) {
				*keyEnd++ = 0;
				HANDLE hRoot = GetRegKey(p);
				if (hRoot != 0) {
					HKEY hKey = nullptr;
					if (ERROR_SUCCESS == RegOpenKeyExA((HKEY)hRoot, keyEnd, 0, KEY_READ, &hKey)) {
						BYTE buffer[MAX_PATH*sizeof(*path)];
						DWORD dwLen = _countof(buffer);
						if (ERROR_SUCCESS == RegQueryValueExA(hKey, valueName, nullptr, nullptr, (LPBYTE)buffer, &dwLen) && dwLen > 0) {
							value = (char*)buffer;
						}
						RegCloseKey(hKey);
					}
				}
			}
		}
	}
	else {
		value = path;
	}
	return value;
}

wstring GetIndirectValue(LPCWSTR path)
{
	if (!path || !*path)
		return wstring();
	wstring value;
	LPWSTR p = STRDUPW(path);
	Trim(p);
	if (*p == L'[') {
		LPWSTR end = wcschr(++p, L']');
		if (end != nullptr) {
			*end++ = 0;
			// Advance unsafely past unnecessary qualifiers
			LPWSTR valueName = end;
			end = valueName + wcslen(end) - 1;
			if (*valueName == L'=') ++valueName;
			if (*valueName == L'@') ++valueName;
			if (*valueName == L'\"' || *valueName == L'\'') ++valueName;
			if (*end == L'\"' || *end == L'\'') *end-- = 0;
			Trim(valueName);
			if (wcslen(valueName) == 0)
				valueName = nullptr;

			LPWSTR keyEnd = wcschr(p, L'\\');
			if (keyEnd != nullptr) {
				*keyEnd++ = 0;
				HANDLE hRoot = GetRegKey(p);
				if (hRoot != 0) {
					HKEY hKey = nullptr;
					if (ERROR_SUCCESS == RegOpenKeyExW((HKEY)hRoot, keyEnd, 0, KEY_READ, &hKey)) {
						BYTE buffer[MAX_PATH*sizeof(*path)];
						DWORD dwLen = _countof(buffer);
						if (ERROR_SUCCESS == RegQueryValueExW(hKey, valueName, nullptr, nullptr, (LPBYTE)buffer, &dwLen) && dwLen > 0) {
							value = (wchar_t*)buffer;
						}
						RegCloseKey(hKey);
					}
				}
			}
		}
	}
	else {
		value = path;
	}
	return value;
}

// Original Source: Jack Handy www.codeproject.com
int wildcmp(const char *wild, const char *string) {
	const char *cp, *mp;

	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?')) {
			return 0;
		}
		wild++, string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild, cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == '?')) {
			wild++, string++;
		}
		else {
			wild = mp, string = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	return !*wild;
}
int wildcmp(const wchar_t *wild, const wchar_t *string) {
	const wchar_t *cp, *mp;

	while ((*string) && (*wild != L'*')) {
		if ((*wild != *string) && (*wild != L'?')) {
			return 0;
		}
		wild++, string++;
	}

	while (*string) {
		if (*wild == L'*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild, cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == L'?')) {
			wild++, string++;
		}
		else {
			wild = mp, string = cp++;
		}
	}

	while (*wild == L'*') {
		wild++;
	}
	return !*wild;
}

// Same as above but case insensitive
int wildcmpi(const char *wild, const char *string) {
	const char *cp, *mp;
	int f, l;
	while ((*string) && (*wild != '*')) {
		f = tolower(*string);
		l = tolower(*wild);
		if ((f != l) && (l != '?')) {
			return 0;
		}
		wild++, string++;
	}
	while (*string) {
		if (*wild == '*') {
			if (!*++wild) return 1;
			mp = wild, cp = string + 1;
		}
		else {
			f = tolower(*string);
			l = tolower(*wild);
			if ((f == l) || (l == '?')) {
				wild++, string++;
			}
			else {
				wild = mp, string = cp++;
			}
		}
	}
	while (*wild == '*') wild++;
	return !*wild;
}
int wildcmpi(const wchar_t *wild, const wchar_t *string) {
	const wchar_t *cp, *mp;
	int f, l;
	while ((*string) && (*wild != L'*')) {
		f = towlower(*string);
		l = towlower(*wild);
		if ((f != l) && (l != L'?')) {
			return 0;
		}
		wild++, string++;
	}
	while (*string) {
		if (*wild == L'*') {
			if (!*++wild) return 1;
			mp = wild, cp = string + 1;
		}
		else {
			f = towlower(*string);
			l = towlower(*wild);
			if ((f == l) || (l == L'?')) {
				wild++, string++;
			}
			else {
				wild = mp, string = cp++;
			}
		}
	}
	while (*wild == '*') wild++;
	return !*wild;
}
bool wildmatch(const char* match, const char* value)
{
	return (wildcmpi(match, value)) ? true : false;
}

bool wildmatch(const wchar_t* match, const wchar_t* value)
{
	return (wildcmpi(match, value)) ? true : false;
}

bool wildmatch(const string& match, const std::string& value)
{
	return (wildcmpi(match.c_str(), value.c_str())) ? true : false;
}

bool wildmatch(const wstring& match, const wstring& value)
{
	return (wildcmpi(match.c_str(), value.c_str())) ? true : false;
}

bool wildmatch(const stringlist& matches, const std::string& value)
{
	for (stringlist::const_iterator itr = matches.begin(), end = matches.end(); itr != end; ++itr) {
		if (wildcmpi((*itr).c_str(), value.c_str()))
			return true;
	}
	return false;
}

bool wildmatch(const wstringlist& matches, const std::wstring& value)
{
	for (wstringlist::const_iterator itr = matches.begin(), end = matches.end(); itr != end; ++itr) {
		if (wildcmpi((*itr).c_str(), value.c_str()))
			return true;
	}
	return false;
}
//! Renames Max Node if it exists
void RenameNode(Interface *gi, LPCSTR SrcName, LPCSTR DstName)
{
	USES_CONVERSION;
	INode *node = gi->GetINodeByName(A2T(SrcName));
	if (node != nullptr) node->SetName(const_cast<LPTSTR>(A2T(DstName)));
}
void RenameNode(Interface *gi, LPCWSTR SrcName, LPCWSTR DstName)
{
	USES_CONVERSION;
	INode *node = gi->GetINodeByName(W2T(SrcName));
	if (node != nullptr) node->SetName(const_cast<LPTSTR>(W2T(DstName)));
}

Point3 TOEULER(const Matrix3 &m)
{
	Point3 rv(0.0f, 0.0f, 0.0f);
	if (m.GetRow(2)[0] < 1.0)
	{
		if (m.GetRow(2)[0] > -1.0)
		{
			rv[2] = atan2(-m.GetRow(1)[0], m.GetRow(0)[0]);
			rv[1] = asin(m.GetRow(2)[0]);
			rv[0] = atan2(-m.GetRow(2)[1], m.GetRow(2)[2]);
		}
		else
		{
			rv[2] = -atan2(-m.GetRow(1)[2], m.GetRow(1)[1]);
			rv[1] = -PI / 2;
			rv[0] = 0.0;
		}
	}
	else
	{
		rv[2] = atan2(m.GetRow(1)[2], m.GetRow(1)[1]);
		rv[1] = PI / 2;
		rv[0] = 0.0;
	}
	return rv;
}

inline Point3 TODEG(const Point3& p) {
	return Point3(TODEG(p[0]), TODEG(p[1]), TODEG(p[2]));
}

inline Point3 TORAD(const Point3& p) {
	return Point3(TORAD(p[0]), TORAD(p[1]), TORAD(p[2]));
}

inline TSTR TOSTRING(const Point3& p) {
	return FormatText(TEXT("[%g,%g,%g]"), p[0], p[1], p[2]);
}

inline TSTR TOSTRING(float p) {
	return FormatText(TEXT("%g"), p);
}

inline TSTR TOSTRING(const Matrix3& m) {
	return TOSTRING(TODEG(TOEULER(m)));
}

inline TSTR TOSTRING(const Quat& q) {
	Matrix3 m; q.MakeMatrix(m);
	return TOSTRING(m);
}

void PosRotScaleNode(INode *n, Matrix3& m3, PosRotScale prs, TimeValue t)
{
	Point3 p = m3.GetTrans();
	Quat q = m3;
	Matrix3 stm = m3 * Inverse(m3);
	float s = (sqrt(stm.GetRow(0)[0]) + sqrt(stm.GetRow(1)[1]) + sqrt(stm.GetRow(1)[1])) / 3.0f;
	PosRotScaleNode(n, p, q, s, prs, t);
}

// Set Position and Rotation on a standard controller will need to handle bipeds
//   Always in World Transform coordinates
void PosRotScaleNode(INode *n, Point3 p, Quat& q, float s, PosRotScale prs, TimeValue t)
{
	if (Control *c = n->GetTMController()) {
		if (prs & prsRot && q.w == FloatNegINF) prs = PosRotScale(prs & ~prsRot);
		if (prs & prsPos && p.x == FloatNegINF) prs = PosRotScale(prs & ~prsPos);
		if (prs & prsScale && s == FloatNegINF) prs = PosRotScale(prs & ~prsScale);
#ifdef USE_BIPED
		// Bipeds are special.  And will crash if you dont treat them with care
		if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID)
			|| (c->ClassID() == BIPBODY_CONTROL_CLASS_ID)
			|| (c->ClassID() == FOOTPRINT_CLASS_ID))
		{
			ScaleValue sv(Point3(s, s, s));
			// Get the Biped Export Interface from the controller 
			//IBipedExport *BipIface = (IBipedExport *) c->GetInterface(I_BIPINTERFACE);
			IOurBipExport *BipIface = (IOurBipExport *)c->GetInterface(I_OURINTERFACE);
			if (prs & prsScale)
				BipIface->SetBipedScale(sv, t, n);
			if (prs & prsRot)
				BipIface->SetBipedRotation(q, t, n, 0/*???*/);
			if (prs & prsPos)
				BipIface->SetBipedPosition(p, t, n);
			return;
		}
#endif
		PosRotScaleNode(c, p, q, s, prs, t);

		//#ifdef _DEBUG
		//      static TSTR sEmpty = "<Empty>";
		//      TSTR spos = (prs & prsPos) ? TOSTRING(p) : sEmpty;
		//      TSTR srot = (prs & prsRot) ? TOSTRING(q) : sEmpty;
		//      TSTR sscl = (prs & prsScale) ? TOSTRING(s) : sEmpty;
		//      OutputDebugString(FormatText("Transform(%s, %s, %s, %s)\n", n->GetName(), spos.data(), srot.data(), sscl.data()));
		//#endif
	}
}

void PosRotScaleNode(Control *c, Point3 p, Quat& q, float s, PosRotScale prs, TimeValue t)
{
	if (c) {
		if (prs & prsRot && q.w == FloatNegINF) prs = PosRotScale(prs & ~prsRot);
		if (prs & prsPos && p.x == FloatNegINF) prs = PosRotScale(prs & ~prsPos);
		if (prs & prsScale && s == FloatNegINF) prs = PosRotScale(prs & ~prsScale);

		ScaleValue sv(Point3(s, s, s));
		if (prs & prsScale)
			if (Control *sclCtrl = c->GetScaleController())
				sclCtrl->SetValue(t, &sv, 1, CTRL_ABSOLUTE);
		if (prs & prsRot)
			if (Control *rotCtrl = c->GetRotationController())
				rotCtrl->SetValue(t, &q, 1, CTRL_ABSOLUTE);
		if (prs & prsPos)
			if (Control *posCtrl = c->GetPositionController())
				posCtrl->SetValue(t, &p, 1, CTRL_ABSOLUTE);
	}
}

// Search NiNode collection for a specific name
NiNodeRef FindNodeByName(const vector<NiNodeRef>& blocks, const string& name)
{
	for (vector<NiNodeRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr)
	{
		const NiNodeRef& block = (*itr);
		if (name == block->GetName())
			return block;
	}
	return NiNodeRef();
}
NiNodeRef FindNodeByName(const vector<NiNodeRef>& blocks, const wstring& name)
{
	USES_CONVERSION;
	return FindNodeByName(blocks, string(W2A(name.c_str())));
}

// Search NiNode collection names that match a wildcard 
vector<NiNodeRef> SelectNodesByName(const vector<NiNodeRef>& blocks, LPCSTR match)
{
	vector<NiNodeRef> nodes;
	for (vector<NiNodeRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr)
	{
		const NiNodeRef& block = (*itr);
		if (wildcmpi(match, block->GetName().c_str()))
			nodes.insert(nodes.end(), block);
	}
	return nodes;
}
vector<NiNodeRef> SelectNodesByName(const vector<NiNodeRef>& blocks, LPCWSTR match)
{
	USES_CONVERSION;
	return SelectNodesByName(blocks, W2A(match));
}

// Count number of NiNodes that match a wildcard 
int CountNodesByName(const vector<NiNodeRef>& blocks, LPCSTR match)
{
	int count = 0;
	for (vector<NiNodeRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr) {
		const NiNodeRef& block = (*itr);
		if (wildcmpi(match, block->GetName().c_str()))
			++count;
	}
	return count;
}
int CountNodesByName(const vector<NiNodeRef>& blocks, LPCWSTR match)
{
	USES_CONVERSION;
	return CountNodesByName(blocks, W2A(match));
}

int CountNodesByType(const vector<NiObjectRef>& blocks, Type type)
{
	int count = 0;
	for (vector<NiObjectRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr) {
		const NiObjectRef& block = (*itr);
		if (block->IsDerivedType(type))
			++count;
	}
	return count;
}

// Get a vector of names from an NiNode vector
vector<string> GetNamesOfNodes(const vector<Niflib::NiNodeRef>& nodes)
{
	vector<string> slist;
	for (vector<NiNodeRef>::const_iterator itr = nodes.begin(), end = nodes.end(); itr != end; ++itr) {
		const NiNodeRef& block = (*itr);
		slist.push_back(block->GetName());
	}
	return slist;
}

// Recursively search through directories applying a filter on what to return
template <typename FileMatch>
void BuildFileNameMapA(NameValueCollectionA & collection, const char *root, const char *path, FileMatch pred)
{
	char buffer[MAX_PATH], buffer2[MAX_PATH], search[MAX_PATH];
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	ZeroMemory(&FindFileData, sizeof(FindFileData));
	if (path == nullptr || path[0] == 0)
		return;
	PathCanonicalizeA(search, path);
	PathAddBackslashA(search);
	strcat(search, "*");

	hFind = FindFirstFileA(search, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		stringlist list;
		for (BOOL ok = TRUE; ok; ok = FindNextFileA(hFind, &FindFileData)) {
			if (FindFileData.cFileName[0] == '.' || (FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
				continue;
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				PathCombineA(buffer, path, FindFileData.cFileName);
				PathAddBackslashA(buffer);
				list.push_back(buffer);
			}
			else {
				if (pred(FindFileData.cFileName)) {
					if (collection.find(FindFileData.cFileName) == collection.end()) {
						PathCombineA(buffer, path, FindFileData.cFileName);
						GetLongPathNameA(buffer, buffer, MAX_PATH);
						PathRemoveExtensionA(FindFileData.cFileName);
						if (PathRelativePathToA(buffer2, root, FILE_ATTRIBUTE_DIRECTORY, buffer, FILE_ATTRIBUTE_NORMAL))
						{
							char *p = buffer2; while (*p == '\\') ++p;
							collection.insert(KeyValuePairA(FindFileData.cFileName, p));
						}
						else
						{
							collection.insert(KeyValuePairA(FindFileData.cFileName, buffer));
						}
					}
				}
			}
		}
		FindClose(hFind);
		for (stringlist::iterator itr = list.begin(), end = list.end(); itr != end; ++itr) {
			BuildFileNameMapA(collection, root, (*itr).c_str(), pred);
		}
	}
}
// Recursively search through directories applying a filter on what to return
template <typename FileMatch>
void BuildFileNameMapW(NameValueCollectionW & collection, const wchar_t *root, const wchar_t *path, FileMatch pred)
{
	wchar_t buffer[MAX_PATH], buffer2[MAX_PATH], search[MAX_PATH];
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;
	ZeroMemory(&FindFileData, sizeof(FindFileData));
	if (path == nullptr || path[0] == 0)
		return;
	PathCanonicalizeW(search, path);
	PathAddBackslashW(search);
	wcscat(search, L"*");

	hFind = FindFirstFileW(search, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		wstringlist list;
		for (BOOL ok = TRUE; ok; ok = FindNextFileW(hFind, &FindFileData)) {
			if (FindFileData.cFileName[0] == L'.' || (FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
				continue;
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				PathCombineW(buffer, path, FindFileData.cFileName);
				PathAddBackslashW(buffer);
				list.push_back(buffer);
			}
			else {
				if (pred(FindFileData.cFileName)) {
					if (collection.find(FindFileData.cFileName) == collection.end()) {
						PathCombineW(buffer, path, FindFileData.cFileName);
						GetLongPathNameW(buffer, buffer, MAX_PATH);
						PathRemoveExtensionW(FindFileData.cFileName);
						if (PathRelativePathToW(buffer2, root, FILE_ATTRIBUTE_DIRECTORY, buffer, FILE_ATTRIBUTE_NORMAL))
						{
							wchar_t *p = buffer2; while (*p == '\\') ++p;
							collection.insert(KeyValuePairW(FindFileData.cFileName, p));
						}
						else
						{
							collection.insert(KeyValuePairW(FindFileData.cFileName, buffer));
						}
					}
				}
			}
		}
		FindClose(hFind);
		for (wstringlist::iterator itr = list.begin(), end = list.end(); itr != end; ++itr) {
			BuildFileNameMapW(collection, root, (*itr).c_str(), pred);
		}
	}
}
// Implementation for BuildFileNameMap which will search for a specific set of extensions
struct ExtensionMatchA : public std::unary_function<LPCSTR, bool>
{
	stringlist extns;
	ExtensionMatchA(string extnlist) {
		extns = TokenizeString(extnlist.c_str(), ";");
	}
	ExtensionMatchA(const stringlist& extnlist) : extns(extnlist) {
	}
	bool operator()(LPCSTR name) const {
		LPCSTR ext = PathFindExtensionA(name);
		for (stringlist::const_iterator itr = extns.begin(), end = extns.end(); itr != end; ++itr) {
			if (0 == strcmp(ext, (*itr).c_str()))
				return true;
		}
		return false;
	}
};
// Implementation for BuildFileNameMap which will search for a specific set of extensions
struct ExtensionMatchW : public std::unary_function<LPCSTR, bool>
{
	wstringlist extns;
	ExtensionMatchW(wstring extnlist) {
		extns = TokenizeString(extnlist.c_str(), L";");
	}
	ExtensionMatchW(const wstringlist& extnlist) : extns(extnlist) {
	}
	bool operator()(LPCWSTR name) const {
		LPCWSTR ext = PathFindExtensionW(name);
		for (wstringlist::const_iterator itr = extns.begin(), end = extns.end(); itr != end; ++itr) {
			if (0 == wcscmp(ext, (*itr).c_str()))
				return true;
		}
		return false;
	}
};

// Run through the search paths and add them to the image collection
void FindImages(NameValueCollectionA& images, const string& rootPath, const stringlist& searchpaths, const stringlist& extensions)
{
	ExtensionMatchA ddsMatch(extensions);
	for (stringlist::const_iterator itr = searchpaths.begin(), end = searchpaths.end(); itr != end; ++itr) {
		if (PathIsRelativeA((*itr).c_str()))
		{
			char texPath[MAX_PATH];
			PathCombineA(texPath, rootPath.c_str(), (*itr).c_str());
			PathAddBackslashA(texPath);
			BuildFileNameMapA(images, rootPath.c_str(), texPath, ddsMatch);
		}
		else
		{
			BuildFileNameMapA(images, rootPath.c_str(), (*itr).c_str(), ddsMatch);
		}
	}
}
void FindImages(NameValueCollectionW& images, const wstring& rootPath, const wstringlist& searchpaths, const wstringlist& extensions)
{
	ExtensionMatchW ddsMatch(extensions);
	for (wstringlist::const_iterator itr = searchpaths.begin(), end = searchpaths.end(); itr != end; ++itr) {
		if (PathIsRelativeW((*itr).c_str()))
		{
			wchar_t texPath[MAX_PATH];
			PathCombineW(texPath, rootPath.c_str(), (*itr).c_str());
			PathAddBackslashW(texPath);
			BuildFileNameMapW(images, rootPath.c_str(), texPath, ddsMatch);
		}
		else
		{
			BuildFileNameMapW(images, rootPath.c_str(), (*itr).c_str(), ddsMatch);
		}
	}
}

// Debugger Trace Window Utilities
TSTR PrintMatrix3(Matrix3& m)
{
	Point3 pt = m.GetTrans();
	float y, p, r;
	m.GetYawPitchRoll(&y, &p, &r);
	return FormatText(TEXT("Matrix3: [%g,%g,%g] <%g,%g,%g>\n")
		, pt.x, pt.y, pt.z
		, TODEG(y), TODEG(p), TODEG(r)
		);
}

void DumpMatrix3(Matrix3& m)
{
	OutputDebugString(PrintMatrix3(m));
}

TSTR PrintMatrix44(Matrix44& m)
{
	Vector3 p; Matrix33 rot; float sc;
	m.Decompose(p, rot, sc);
	Quaternion q = rot.AsQuaternion();
	Float3 f = q.AsEulerYawPitchRoll();

	return FormatText(TEXT("Matrix3: [%g,%g,%g] <%g,%g,%g> (%g)\n")
		, p.x, p.y, p.z
		, TODEG(f[0]), TODEG(f[1]), TODEG(f[2])
		, sc
		);
}
void DumpMatrix44(Matrix44& m)
{
	OutputDebugString(PrintMatrix44(m));
}

INode* FindINode(Interface *i, const string& name)
{
	USES_CONVERSION;
	return i->GetINodeByName(A2T(name.c_str()));
}

INode* FindINode(Interface *i, const wstring& name)
{
	USES_CONVERSION;
	return i->GetINodeByName(W2T(name.c_str()));
}

INode* FindINode(Interface *i, NiObjectNETRef node)
{
	if (node != nullptr)
	{
		return FindINode(i, node->GetName());
	}
	return nullptr;
}

Matrix3 GetNodeLocalTM(INode *n, TimeValue t)
{
	Matrix3 m3 = n->GetNodeTM(t);
	Matrix3 m3p = n->GetParentTM(t);
	m3p.Invert();
	return m3 * m3p;
}

// Locate a TriObject in an Object if it exists
TriObject* GetTriObject(Object *o)
{
	if (o && o->CanConvertToType(triObjectClassID))
		return (TriObject *)o->ConvertToType(0, triObjectClassID);
	while (o->SuperClassID() == GEN_DERIVOB_CLASS_ID && o)
	{
		IDerivedObject* dobj = (IDerivedObject *)(o);
		o = dobj->GetObjRef();
		if (o && o->CanConvertToType(triObjectClassID))
			return (TriObject *)o->ConvertToType(0, triObjectClassID);
	}
	return nullptr;
}

// Get or Create the Skin Modifier
Modifier *GetOrCreateSkin(INode *node)
{
	Modifier *skinMod = GetSkin(node);
	if (skinMod)
		return skinMod;

	Object *pObj = node->GetObjectRef();
	IDerivedObject *dobj = nullptr;
	if (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		dobj = static_cast<IDerivedObject*>(pObj);
	else {
		dobj = CreateDerivedObject(pObj);
	}
	//create a skin modifier and add it
	skinMod = (Modifier*)CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
	dobj->SetAFlag(A_LOCK_TARGET);
	dobj->AddModifier(skinMod);
	dobj->ClearAFlag(A_LOCK_TARGET);
	node->SetObjectRef(dobj);
	return skinMod;
}

Modifier *GetSkin(INode *node)
{
	Object* pObj = node->GetObjectRef();
	if (!pObj) return nullptr;
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);
			if (mod->ClassID() == SKIN_CLASSID)
			{
				// is this the correct Physique Modifier based on index?
				return mod;
			}
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	return nullptr;
}


TSTR GetFileVersion(const wchar_t *fileName)
{
	USES_CONVERSION;
	TSTR retval;
	wchar_t fileVersion[MAX_PATH];
	if (fileName == nullptr)
	{
		GetModuleFileNameW(hInstance, fileVersion, MAX_PATH);
		fileName = fileVersion;
	}
	HMODULE ver = GetModuleHandleW(L"version.dll");
	if (!ver) ver = LoadLibraryW(L"version.dll");
	if (ver != nullptr)
	{
		DWORD(APIENTRY *GetFileVersionInfoSizeW)(LPCWSTR, LPDWORD) = nullptr;
		BOOL(APIENTRY *GetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID) = nullptr;
		BOOL(APIENTRY *VerQueryValueW)(const LPVOID, LPWSTR, LPVOID *, PUINT) = nullptr;
		*(FARPROC*)&GetFileVersionInfoSizeW = GetProcAddress(ver, "GetFileVersionInfoSizeW");
		*(FARPROC*)&GetFileVersionInfoW = GetProcAddress(ver, "GetFileVersionInfoW");
		*(FARPROC*)&VerQueryValueW = GetProcAddress(ver, "VerQueryValueW");
		if (GetFileVersionInfoSizeW && GetFileVersionInfoW && VerQueryValueW)
		{
			DWORD vLen = 0;
			DWORD vSize = GetFileVersionInfoSizeW(fileName, &vLen);
			if (vSize)
			{
				LPVOID versionInfo = malloc(vSize + 1);
				if (GetFileVersionInfoW(fileName, vLen, vSize, versionInfo))
				{
					LPVOID version = nullptr;
					if (VerQueryValueW(versionInfo, L"\\VarFileInfo\\Translation", &version, (UINT *)&vLen) && vLen == 4)
					{
						DWORD langD = *(DWORD*)version;
						swprintf(fileVersion, L"\\StringFileInfo\\%02X%02X%02X%02X\\ProductVersion",
							(langD & 0xff00) >> 8, langD & 0xff, (langD & 0xff000000) >> 24, (langD & 0xff0000) >> 16);
					}
					else
					{
						swprintf(fileVersion, L"\\StringFileInfo\\%04X04B0\\ProductVersion", GetUserDefaultLangID());
					}
					LPCTSTR value = nullptr;
					if (VerQueryValueW(versionInfo, fileVersion, &version, (UINT *)&vLen))
						value = LPCTSTR(version);
					else if (VerQueryValueW(versionInfo, L"\\StringFileInfo\\040904B0\\ProductVersion", &version, (UINT *)&vLen))
						value = LPCTSTR(version);
					if (value != nullptr)
					{
						tstringlist val = TokenizeString(value, TEXT(","), true);
						if (val.size() >= 4) {
							retval = FormatText(TEXT("%s.%s.%s.%s"), val[0].c_str(), val[1].c_str(), val[2].c_str(), val[3].c_str());
						}
					}
					free(versionInfo);
				}
			}
		}
	}
	return retval;
}

TSTR GetFileVersion(const char *fileName)
{
	USES_CONVERSION;
	return GetFileVersion(A2W(fileName));
}

// Calculate bounding sphere using minimum-volume axis-align bounding box.  Its fast but not a very good fit.
void CalcAxisAlignedSphere(const vector<Vector3>& vertices, Vector3& center, float& radius)
{
	//--Calculate center & radius--//

	//Set lows and highs to first vertex
	Vector3 lows = vertices[0];
	Vector3 highs = vertices[0];

	//Iterate through the vertices, adjusting the stored values
	//if a vertex with lower or higher values is found
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		const Vector3 & v = vertices[i];

		if (v.x > highs.x) highs.x = v.x;
		else if (v.x < lows.x) lows.x = v.x;

		if (v.y > highs.y) highs.y = v.y;
		else if (v.y < lows.y) lows.y = v.y;

		if (v.z > highs.z) highs.z = v.z;
		else if (v.z < lows.z) lows.z = v.z;
	}

	//Now we know the extent of the shape, so the center will be the average
	//of the lows and highs
	center = (highs + lows) / 2.0f;

	//The radius will be the largest distance from the center
	Vector3 diff;
	float dist2(0.0f), maxdist2(0.0f);
	for (unsigned int i = 0; i < vertices.size(); ++i) {
		const Vector3 & v = vertices[i];

		diff = center - v;
		dist2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (dist2 > maxdist2) maxdist2 = dist2;
	};
	radius = sqrt(maxdist2);
}

// Calculate bounding sphere using average position of the points.  Better fit but slower.
void CalcCenteredSphere(const vector<Vector3>& vertices, Vector3& center, float& radius)
{
	size_t nv = vertices.size();
	Vector3 sum;
	for (size_t i = 0; i < nv; ++i)
		sum += vertices[i];
	center = sum / float(nv);
	radius = 0.0f;
	for (size_t i = 0; i < nv; ++i) {
		Vector3 diff = vertices[i] - center;
		float mag = diff.Magnitude();
		radius = max(radius, mag);
	}
}



static void TransformVector3(Matrix44& tm, vector<Vector3>& pts)
{
	for (vector<Vector3>::iterator itr = pts.begin(); itr != pts.end(); ++itr)
	{
		Matrix44 m4(*itr, Matrix33::IDENTITY, 1.0f);
		Matrix44 ntm = m4 * tm;
		Vector3 v = ntm.GetTranslation();
		(*itr) = v;
	}
}

void CollapseGeomTransform(NiNode node) {

}

void CollapseGeomTransform(NiTriBasedGeomRef shape)
{
	NiTriBasedGeomDataRef data = shape->GetData();
	vector<Vector3> verts = data->GetVertices();
	vector<Vector3> norms = data->GetNormals();
	int nuvsets = data->GetUVSetCount();
	vector< vector<TexCoord> > uvSets;
	uvSets.resize(nuvsets);
	for (int i = 0; i < nuvsets; ++i)
		uvSets[i] = data->GetUVSet(i);

	Matrix44 ltm = shape->GetLocalTransform();
	Matrix44 invtm = ltm.Inverse();
	Matrix44 tm = ltm * invtm;
	shape->SetLocalTransform(tm);

	TransformVector3(ltm, verts);

	data->SetVertices(verts);
	data->SetNormals(norms);
	data->SetUVSetCount(nuvsets);
	for (int i = 0; i < nuvsets; ++i)
		data->SetUVSet(i, uvSets[i]);
}

void CollapseGeomTransforms(vector<NiTriBasedGeomRef>& shapes)
{
	for (vector<NiTriBasedGeomRef>::iterator itr = shapes.begin(); itr != shapes.end(); ++itr) {
		CollapseGeomTransform(*itr);
	}
}

void FixNormals(vector<Triangle>& tris, vector<Vector3>& verts, vector<Vector3>& norms)
{
	if (tris.size() != norms.size())
		return;

	size_t n = tris.size();
	for (size_t i = 0; i < n; ++i)
	{
		Triangle& tri = tris[i];
		Vector3 v1 = verts[tri.v1];
		Vector3 v2 = verts[tri.v2];
		Vector3 v3 = verts[tri.v3];
		Vector3 n1 = (v2 - v1).CrossProduct(v3 - v1).Normalized();
		Vector3 n2 = norms[i];
		float dp = n1.DotProduct(n2);
		if (dp < 0.0f) {
			std::swap(tri.v3, tri.v2);
		}
	}
}


static Value* LocalExecuteScript(CharStream* source, bool *res) {

	*res = true;

	init_thread_locals();
	push_alloc_frame();
	three_typed_value_locals(Parser* parser, Value* code, Value* result);
	CharStream* out = thread_local(current_stdout);
	vl.parser = new Parser(out);

	try {

		source->flush_whitespace();
		while (!source->at_eos()) {
			vl.code = vl.parser->compile(source);
			vl.result = vl.code->eval()->get_heap_ptr();
			source->flush_whitespace();
		}
		source->close();

	}
	catch (...) {
		*res = false;
	}

	if (vl.result == nullptr)
		vl.result = &ok;

	pop_alloc_frame();
	return_value(vl.result);
}

// CallMaxscript
// Send the string to maxscript 
//
void CallMaxscript(const TCHAR *s)
{
	static bool script_initialized = false;
	if (!script_initialized) {
		init_MAXScript();
		script_initialized = TRUE;
	}
	init_thread_locals();

	push_alloc_frame();
	two_typed_value_locals(StringStream* ss, Value* result);

	vl.ss = new StringStream(const_cast<TCHAR*>(s));
	bool res = false;
	try {
		vl.result = LocalExecuteScript(vl.ss, &res);
	}
	catch (...) {
		res = false;
	}
	thread_local(current_result) = vl.result;
	thread_local(current_locals_frame) = vl.link;
	pop_alloc_frame();
}

void GetIniFileName(LPTSTR iniName)
{
#if VERSION_3DSMAX >= ((5000<<16)+(15<<8)+0) // Version 5+
	Interface *gi = GetCOREInterface();
#else
	Interface *gi = nullptr;
#endif
	*iniName = 0;
	if (gi) {
		LPCTSTR pluginDir = gi->GetDir(APP_PLUGCFG_DIR);
		PathCombine(iniName, pluginDir, TEXT("MaxNifTools.ini"));

		int forcePlugcfg = GetIniValue(TEXT("System"), TEXT("ForcePlugcfg"), 0, iniName);
		if (forcePlugcfg == 1 || _taccess(iniName, 06) != 0) {
			TCHAR iniPath[MAX_PATH];
			GetModuleFileName(nullptr, iniPath, MAX_PATH);
			if (LPTSTR fname = PathFindFileName(iniPath))
				*fname = 0;
			PathAddBackslash(iniPath);
			PathAppend(iniPath, TEXT("plugcfg"));
			PathAppend(iniPath, TEXT("MaxNifTools.ini"));

			// Use plugcfg directory ini 
			if (_taccess(iniPath, 06) == 0) {
				_tcscpy(iniName, iniPath);
			}
		}
	}
	else {
		GetModuleFileName(nullptr, iniName, MAX_PATH);
		if (LPTSTR fname = PathFindFileName(iniName))
			*fname = 0;
		PathAddBackslash(iniName);
		PathAppend(iniName, TEXT("plugcfg"));
		PathAppend(iniName, TEXT("MaxNifTools.ini"));
	}

	if (_taccess(iniName, 06) != 0) {
		MessageBox(nullptr, TEXT("MaxNifTools could not find a valid INI.  The plugin may not work correctly.\nPlease check for proper installation."),
			TEXT("MaxNifTools"), MB_OK | MB_ICONWARNING);
	}
}
#ifdef UNICODE
#define GetIniFileNameW(x) GetIniFileName(x)
void GetIniFileNameA(char *iniName)
{
	wchar_t tmpName[MAX_PATH];
	GetIniFileName(tmpName);
	wcstombs(iniName, tmpName, wcslen(tmpName) + 1);
}
#else
#define GetIniFileNameA(x) GetIniFileName(x)
void GetIniFileNameW(wchar_t *iniName)
{
	char tmpName[MAX_PATH];
	GetIniFileName(tmpName);
	mbstowcs(iniName, tmpName, strlen(tmpName) + 1);
}
#endif

Modifier *GetbhkCollisionModifier(INode* node)
{
	extern Class_ID BHKRIGIDBODYMODIFIER_CLASS_ID;

	Object* pObj = node->GetObjectRef();
	if (!pObj) return nullptr;
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);
			if (mod->ClassID() == BHKRIGIDBODYMODIFIER_CLASS_ID)
			{
				return mod;
			}
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	return nullptr;
}

Modifier *GethctRigidBodyModifier(INode* node)
{
//	extern Class_ID HK_RIGIDBODY_MODIFIER_CLASS_ID;

	Object* pObj = node->GetObjectRef();
	if (!pObj) return nullptr;
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);
//			if (mod->ClassID() == HK_RIGIDBODY_MODIFIER_CLASS_ID)
//			{
//				return mod;
//			}
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	return nullptr;
}


Modifier *CreatebhkCollisionModifier(INode* node, int type, int materialIndex, int layerIndex, unsigned char filter /*= 0*/)
{
	enum { havok_params };
	enum { PB_BOUND_TYPE, PB_MATERIAL, PB_OPT_ENABLE, PB_MAXEDGE, PB_FACETHRESH, PB_EDGETHRESH, PB_BIAS, PB_LAYER, PB_FILTER, };
	extern Class_ID BHKRIGIDBODYMODIFIER_CLASS_ID;

	Modifier *rbMod = GetbhkCollisionModifier(node);
	if (rbMod == nullptr)
	{
		Object *pObj = node->GetObjectRef();
		IDerivedObject *dobj = nullptr;
		if (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
			dobj = static_cast<IDerivedObject*>(pObj);
		else {
			dobj = CreateDerivedObject(pObj);
		}
		rbMod = (Modifier*)CreateInstance(OSM_CLASS_ID, BHKRIGIDBODYMODIFIER_CLASS_ID);
		dobj->SetAFlag(A_LOCK_TARGET);
		dobj->AddModifier(rbMod);
		dobj->ClearAFlag(A_LOCK_TARGET);
		node->SetObjectRef(dobj);
	}

	if (IParamBlock2* pblock2 = rbMod->GetParamBlockByID(havok_params))
	{
		pblock2->SetValue(PB_BOUND_TYPE, INFINITE, type, 0);
		pblock2->SetValue(PB_MATERIAL, INFINITE, materialIndex, 0);
		pblock2->SetValue(PB_LAYER, INFINITE, layerIndex, 0);
		pblock2->SetValue(PB_FILTER, INFINITE, filter, 0);
	}
	return rbMod;
}

TSTR GetNodeName(INode* node)
{
	return node->GetName();
}

Matrix3 GetLocalTM(INode *node)
{
	if (INode *parent = node->GetParentNode())
	{
		Matrix3 parentTM, nodeTM;
		nodeTM = node->GetNodeTM(0);
		parent = node->GetParentNode();
		parentTM = parent->GetNodeTM(0);
		return nodeTM*Inverse(parentTM);
	}
	else
	{
		return node->GetNodeTM(0);
	}
}


// Enumeration Support
TSTR EnumToString(int value, const EnumLookupType *table) {
	for (const EnumLookupType *itr = table; itr->name != nullptr; ++itr) {
		if (itr->value == value) return TSTR(itr->name);
	}
	return FormatText(TEXT("%x"), value);
}

LPCTSTR EnumToStringRaw(int value, const EnumLookupType *table) {
	for (const EnumLookupType *itr = table; itr->name != nullptr; ++itr) {
		if (itr->value == value) return itr->name;
	}
	return TEXT("");
}


int EnumToIndex(int value, const EnumLookupType *table) {
	int i = 0;
	for (const EnumLookupType *itr = table; itr->name != nullptr; ++itr, ++i) {
		if (itr->value == value) return i;
	}
	return -1;
}

int StringToEnum(TSTR value, const EnumLookupType *table) {
	//Trim(value);
	if (value.isNull()) return 0;

	for (const EnumLookupType *itr = table; itr->name != nullptr; ++itr) {
		if (0 == _tcsicmp(value, itr->name)) return itr->value;
	}
	TCHAR *end = nullptr;
	return (int)_tcstol(value, &end, 0);
}

TSTR FlagsToString(int value, const EnumLookupType *table) {
	TSTR sstr;
	for (const EnumLookupType *itr = table; itr->name != nullptr; ++itr) {
		if (itr->value && (itr->value & value) == itr->value) {
			if (!sstr.isNull()) sstr += TEXT(" | ");
			sstr += itr->name;
			value ^= itr->value;
		}
	}
	if (value == 0 && sstr.isNull()) {
		return EnumToString(value, table);
	}
	if (value != 0) {
		if (!sstr.isNull()) sstr += TEXT("|");
		sstr += EnumToString(value, table);
	}
	return sstr;
}

int StringToFlags(TSTR value, const EnumLookupType *table) {
	int retval = 0;
	LPCTSTR start = value.data();
	LPCTSTR end = value.data() + value.Length();
	while (start < end) {
		LPCTSTR bar = _tcschr(start, '|');
		int offset = int(start - value.data());
		int len = int((bar != nullptr) ? bar - start : end - start);
		TSTR subval = value.Substr(offset, len);
		retval |= StringToEnum(subval, table);
		start += (len + 1);
	}
	return retval;
}

wstring A2WString(const string& str)
{
	USES_CONVERSION;
	return A2W(str.c_str());
}

string W2AString(const wstring& str)
{
	USES_CONVERSION;
	return W2A(str.c_str());
}

bool GetTexFullName(Texmap *texMap, TSTR& fName)
{
	if (texMap && texMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
		TSTR fileName = static_cast<BitmapTex*>(texMap)->GetMapName();
		if (fileName.isNull()) {
			fileName = static_cast<BitmapTex*>(texMap)->GetFullName();
			int idx = fileName.last('(');
			if (idx >= 0) {
				fileName.remove(idx, fileName.length() - idx + 1);
				while (--idx > 0) {
					if (isspace(fileName[idx]))
						fileName.remove(idx);
				}
			}
		}
		fName = fileName;
		return true;
	}
	return false;
}


bool GetTexFullName(Texmap *texMap, tstring& fName)
{
	if (texMap && texMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
		TSTR fileName = static_cast<BitmapTex*>(texMap)->GetMapName();
		if (fileName.isNull()) {
			fileName = static_cast<BitmapTex*>(texMap)->GetFullName();
			int idx = fileName.last('(');
			if (idx >= 0) {
				fileName.remove(idx, fileName.length() - idx + 1);
				while (--idx > 0) {
					if (isspace(fileName[idx]))
						fileName.remove(idx);
				}
			}
		}
		fName = fileName;
		return true;
	}
	return false;
}
