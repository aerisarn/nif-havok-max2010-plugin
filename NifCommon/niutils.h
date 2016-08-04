/**********************************************************************
*<
FILE: NIUtils.h

DESCRIPTION:	NifImporter Utilities

CREATED BY: tazpn (Theo)

HISTORY: 

INFO: See Implementation for minimalist comments

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#ifndef _NIUTILS_H_
#define _NIUTILS_H_

#ifndef _WINDOWS_
#  include <windows.h>
#endif
#include <tchar.h>
#include <memory.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <map>

// Max Headers
#include <Max.h>
#include <strclass.h>
#include <color.h>

// Niflib Headers
#include <niflib.h>
#include <obj\NiObject.h>
#include <obj\NiAVObject.h>
#include <obj\NiObjectNET.h>
#include <obj\NiNode.h>
#include <obj\NiTriBasedGeom.h>
#include <gen\QuaternionXYZW.h>
#include <gen\HalfTexCoord.h>
#include <gen\HalfVector3.h>
#include <nif_math.h>
#include <nif_io.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

#ifdef UNICODE
typedef std::wstring tstring;
typedef std::wstringstream tstringstream;
typedef std::wistringstream tistringstream;
typedef std::wostringstream tostringstream;
#else
typedef std::string tstring;
typedef std::stringstream tstringstream;
typedef std::istringstream tistringstream;
typedef std::ostringstream tostringstream;
#endif

#if VERSION_3DSMAX < (15000<<16) // Version 15 (2013)
#define p_end ParamTags::end
#endif


const unsigned int IntegerInf = 0x7f7fffff;
const unsigned int IntegerNegInf = 0xff7fffff;
const float FloatINF = *(float*)&IntegerInf;
const float FloatNegINF = *(float*)&IntegerNegInf;


inline LPWSTR A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars) {
	if (lpw == nullptr || lpa == nullptr) return L"";
	*lpw = '\0';
	if (0 > mbstowcs(lpw, lpa, nChars)) return L"";
	return lpw;
}

inline LPSTR W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars) {
	if (lpa == nullptr || lpw == nullptr) return "";
	*lpa = '\0';
	if (0 > wcstombs(lpa, lpw, nChars)) return "";
	return lpa;
}

#define USES_CONVERSION int _convert; (_convert); LPCWSTR _lpw; (_lpw); LPCSTR _lpa; (_lpa)

#define A2W(lpa) (\
	((_lpa = lpa) == nullptr) ? nullptr : (\
		_convert = (static_cast<int>(strlen(_lpa))+1),\
		A2WHelper((LPWSTR) alloca(_convert*sizeof(WCHAR)), _lpa, _convert)))

#define W2A(lpw) (\
	((_lpw = lpw) == nullptr) ? nullptr : (\
		_convert = (static_cast<int>(wcslen(_lpw))+1), \
		W2AHelper((LPSTR)alloca(_convert*sizeof(WCHAR)), _lpw, _convert*sizeof(WCHAR))))

#define A2W_EX(lpa, n) (\
	((_lpa = lpa) == nullptr) ? nullptr : (\
		_convert = (static_cast<int>(n)+1),\
		A2WHelper((LPWSTR) alloca(_convert*sizeof(WCHAR)), _lpa, _convert)))

#define W2A_EX(lpw, n) (\
	((_lpw = lpw) == nullptr) ? nullptr : (\
		_convert = (static_cast<int>(n)+1), \
		W2AHelper((LPSTR)alloca(_convert*sizeof(WCHAR)), _lpw, _convert*sizeof(WCHAR))))

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

wstring A2WString(const string& str);
string W2AString(const wstring& str);

#ifdef UNICODE
#define A2T(lpa) A2W(lpa)
#define W2T(lpa) static_cast<LPCWSTR>(lpa)
#define T2A(lpa) W2A(lpa)
#define T2W(lpa) static_cast<LPCWSTR>(lpa)
#define A2T_EX(lpa,n) A2W(lpa,n)
#define W2T_EX(lpa,n) static_cast<LPCWSTR>(lpa)
#define T2A_EX(lpa,n) W2A(lpa,n)
#define T2W_EX(lpa,n) static_cast<LPCWSTR>(lpa)
#define T2AHelper W2AHelper
#define T2WHelper(d,s,n) (s)
#define A2THelper A2WHelper
#define W2THelper(d,s,n) (s)
#define T2AString W2AString
#define T2WString(s) (s)
#define A2TString A2WString
#define W2TString(s) (s)
#else
#define A2T(lpa) static_cast<LPCSTR>(lpa)
#define W2T(lpa) W2A(lpa)
#define T2A(lpa) static_cast<LPCSTR>(lpa)
#define T2W(lpa) A2W(lpa)
#define A2THelper(d,s,n) static_cast<LPCSTR>(s) 
#define W2THelper W2AHelper
#define T2AHelper(d,s,n) static_cast<LPCSTR>(s)
#define T2WHelper A2WHelper
#define T2AString(s) (s)
#define T2WString A2WString
#define A2TString(s) (s)
#define W2TString W2AString
#endif

#if VERSION_3DSMAX < (15000<<16) // Version 15 (2013)
inline LPTSTR DataForWrite(TSTR& str) { return str.data(); }
#else
inline LPTSTR DataForWrite(TSTR& str) { return str.dataForWrite(); }
#endif

// Trim whitespace before and after a string
inline char *Trim(char*&p) { 
   while(isspace(*p)) *p++ = 0; 
   char *e = p + strlen(p) - 1;
   while (e > p && isspace(*e)) *e-- = 0;
   return p;
}
inline wchar_t *Trim(wchar_t*&p) {
	while (iswspace(*p)) *p++ = 0;
	wchar_t *e = p + wcslen(p) - 1;
	while (e > p && iswspace(*e)) *e-- = 0;
	return p;
}

// Case insensitive string equivalence test for collections
struct ltstr
{
   bool operator()(const char* s1, const char* s2) const
   { return _stricmp(s1, s2) < 0; }

   bool operator()(const string& s1, const string& s2) const
   { return _stricmp(s1.c_str(), s2.c_str()) < 0; }

   bool operator()(const string& s1, const char * s2) const
   { return _stricmp(s1.c_str(), s2) < 0; }

   bool operator()(const char * s1, const string& s2) const
   { return _stricmp(s1, s2.c_str()) >= 0; }

   bool operator()(const wchar_t* s1, const wchar_t* s2) const
   { return _wcsicmp(s1, s2) < 0; }

   bool operator()(const wstring& s1, const wstring& s2) const
   { return _wcsicmp(s1.c_str(), s2.c_str()) < 0; }

   bool operator()(const wstring& s1, const wchar_t * s2) const
   { return _wcsicmp(s1.c_str(), s2) < 0; }

   bool operator()(const wchar_t * s1, const wstring& s2) const
   { return _wcsicmp(s1, s2.c_str()) >= 0; }
};


// Case insensitive string equivalence but numbers are sorted together
struct NumericStringEquivalence
{
   bool operator()(const char* s1, const char* s2) const
   { return numstrcmp(s1, s2) < 0; }

   bool operator()(const std::string& s1, const char* s2) const
   { return numstrcmp(s1.c_str(), s2) < 0; }

   bool operator()(const char* s1, const std::string& s2) const
   { return numstrcmp(s1, s2.c_str()) < 0; }

   bool operator()(const std::string& s1, const std::string& s2) const
   { return numstrcmp(s1.c_str(), s2.c_str()) < 0; }

   bool operator()(const wchar_t* s1, const wchar_t* s2) const
   { return numstrcmp(s1, s2) < 0; }

   bool operator()(const std::wstring& s1, const wchar_t* s2) const
   { return numstrcmp(s1.c_str(), s2) < 0; }

   bool operator()(const wchar_t* s1, const std::wstring& s2) const
   { return numstrcmp(s1, s2.c_str()) < 0; }

   bool operator()(const std::wstring& s1, const std::wstring& s2) const
   { return numstrcmp(s1.c_str(), s2.c_str()) < 0; }

   static int numstrcmp(const char *str1, const char *str2)
   {
      char *p1, *p2;
      int c1, c2;
	  size_t lcmp;
      for(;;)
      {
         c1 = tolower(*str1), c2 = tolower(*str2);
         if ( c1 == 0 || c2 == 0 )
            break;
         else if (isdigit(c1) && isdigit(c2))
         {			
            lcmp = strtol(str1, &p1, 10) - strtol(str2, &p2, 10);
            if ( lcmp == 0 )
               lcmp = (p2 - str2) - (p1 - str1);
            if ( lcmp != 0 )
               return (lcmp > 0 ? 1 : -1);
            str1 = p1, str2 = p2;
         }
         else
         {
            lcmp = (c1 - c2);
            if (lcmp != 0)
               return (lcmp > 0 ? 1 : -1);
            ++str1, ++str2;
         }
      }
      lcmp = (c1 - c2);
      return ( lcmp < 0 ) ? -1 : (lcmp > 0 ? 1 : 0);
   }

   static int numstrcmp(const wchar_t *str1, const wchar_t *str2)
   {
      wchar_t *p1, *p2;
      int c1, c2;
	  size_t lcmp;
      for(;;)
      {
         c1 = towlower(*str1), c2 = towlower(*str2);
         if ( c1 == 0 || c2 == 0 )
            break;
         else if (iswdigit(c1) && iswdigit(c2))
         {			
            lcmp = wcstol(str1, &p1, 10) - wcstol(str2, &p2, 10);
            if ( lcmp == 0 )
               lcmp = (p2 - str2) - (p1 - str1);
            if ( lcmp != 0 )
               return (lcmp > 0 ? 1 : -1);
            str1 = p1, str2 = p2;
         }
         else
         {
            lcmp = (c1 - c2);
            if (lcmp != 0)
               return (lcmp > 0 ? 1 : -1);
            ++str1, ++str2;
         }
      }
      lcmp = (c1 - c2);
      return ( lcmp < 0 ) ? -1 : (lcmp > 0 ? 1 : 0);
   }

};

// Common collections that I use
typedef std::map<std::string, std::string, ltstr> NameValueCollectionA;
typedef std::pair<std::string, std::string> KeyValuePairA;
typedef std::map<std::wstring, std::wstring, ltstr> NameValueCollectionW;
typedef std::pair<std::wstring, std::wstring> KeyValuePairW;
typedef std::vector<std::string> stringlist;
typedef std::vector<std::wstring> wstringlist;
typedef std::list<KeyValuePairA> NameValueListA;
typedef std::list<KeyValuePairW> NameValueListW;
#ifdef UNICODE
typedef NameValueCollectionW NameValueCollection;
typedef KeyValuePairW KeyValuePair;
typedef wstringlist tstringlist;
typedef NameValueListW NameValueList;
#else
typedef NameValueCollectionA NameValueCollection;
typedef KeyValuePairA KeyValuePair;
typedef stringlist tstringlist;
typedef NameValueListA NameValueList;
#endif
extern int wildcmp(const char *wild, const char *string);
extern int wildcmpi(const char *wild, const char *string);
extern int wildcmp(const wchar_t *wild, const wchar_t *string);
extern int wildcmpi(const wchar_t *wild, const wchar_t *string);

inline bool strmatch(const string& lhs, const std::string& rhs) {
   return (0 == _stricmp(lhs.c_str(), rhs.c_str()));
}
inline bool strmatch(const char* lhs, const std::string& rhs) {
   return (0 == _stricmp(lhs, rhs.c_str()));
}
inline bool strmatch(const string& lhs, const char* rhs) {
   return (0 == _stricmp(lhs.c_str(), rhs));
}
inline bool strmatch(const char* lhs, const char* rhs) {
   return (0 == _stricmp(lhs, rhs));
}
inline bool strmatch(const wstring& lhs, const std::wstring& rhs) {
   return (0 == _wcsicmp(lhs.c_str(), rhs.c_str()));
}
inline bool strmatch(const wchar_t* lhs, const std::wstring& rhs) {
   return (0 == _wcsicmp(lhs, rhs.c_str()));
}
inline bool strmatch(const wstring& lhs, const wchar_t* rhs) {
   return (0 == _wcsicmp(lhs.c_str(), rhs));
}
inline bool strmatch(const wchar_t* lhs, const wchar_t* rhs) {
   return (0 == _wcsicmp(lhs, rhs));
}

bool wildmatch(const char* match, const char* value);
bool wildmatch(const string& match, const std::string& value);
bool wildmatch(const stringlist& matches, const std::string& value);
bool wildmatch(const wchar_t* match, const wchar_t* value);
bool wildmatch(const wstring& match, const std::wstring& value);
bool wildmatch(const wstringlist& matches, const std::wstring& value);

// Generic IniFile reading routine
template<typename T>
inline T GetIniValue(LPCSTR Section, LPCSTR Setting, T Default, LPCSTR iniFileName){
   T v;
   char buffer[1024];
   stringstream sstr;
   sstr << Default;
   buffer[0] = 0;
   if (0 < GetPrivateProfileStringA(Section, Setting, sstr.str().c_str(), buffer, sizeof(buffer), iniFileName)){
      stringstream sstr(buffer);
      sstr >> v;
      return v;
   }
   return Default;
}
template<typename T>
inline T GetIniValue(LPCWSTR Section, LPCWSTR Setting, T Default, LPCWSTR iniFileName) {
	T v;
	wchar_t buffer[1024];
	wstringstream sstr;
	sstr << Default;
	buffer[0] = 0;
	if (0 < GetPrivateProfileStringW(Section, Setting, sstr.str().c_str(), buffer, sizeof(buffer), iniFileName)) {
		wstringstream sstr(buffer);
		sstr >> v;
		return v;
	}
	return Default;
}
// Specific override for int values
template<>
inline int GetIniValue<int>(LPCSTR Section, LPCSTR Setting, int Default, LPCSTR iniFileName){
   return GetPrivateProfileIntA(Section, Setting, Default, iniFileName);
}
template<>
inline int GetIniValue<int>(LPCWSTR Section, LPCWSTR Setting, int Default, LPCWSTR iniFileName) {
	return GetPrivateProfileIntW(Section, Setting, Default, iniFileName);
}

// Specific override for string values
template<>
inline std::string GetIniValue<std::string>(LPCSTR Section, LPCSTR Setting, std::string Default, LPCSTR iniFileName){
   char buffer[1024];
   buffer[0] = 0;
   if (0 < GetPrivateProfileStringA(Section, Setting, Default.c_str(), buffer, sizeof(buffer), iniFileName)){
      return std::string(buffer);
   }
   return Default;
}
template<>
inline std::wstring GetIniValue<std::wstring>(LPCWSTR Section, LPCWSTR Setting, std::wstring Default, LPCWSTR iniFileName) {
	wchar_t buffer[1024];
	buffer[0] = 0;
	if (0 < GetPrivateProfileStringW(Section, Setting, Default.c_str(), buffer, sizeof(buffer), iniFileName)) {
		return std::wstring(buffer);
	}
	return Default;
}

// Specific override for TSTR values
#ifdef UNICODE
template<>
inline TSTR GetIniValue<TSTR>(LPCWSTR Section, LPCWSTR Setting, TSTR Default, LPCWSTR iniFileName){
   wchar_t buffer[1024];
   buffer[0] = 0;
   if (0 < GetPrivateProfileStringW(Section, Setting, Default.data(), buffer, sizeof(buffer), iniFileName)){
      return TSTR(buffer);
   }
   return Default;
}
#else
template<>
inline TSTR GetIniValue<TSTR>(LPCSTR Section, LPCSTR Setting, TSTR Default, LPCSTR iniFileName) {
	char buffer[1024];
	buffer[0] = 0;
	if (0 < GetPrivateProfileStringA(Section, Setting, Default.data(), buffer, sizeof(buffer), iniFileName)) {
		return TSTR(buffer);
	}
	return Default;
}
#endif
// Generic IniFile reading routine
template<typename T>
inline void SetIniValue(LPCSTR Section, LPCSTR Setting, T value, LPCSTR iniFileName){
   stringstream sstr;
   sstr << value;
   WritePrivateProfileStringA(Section, Setting, sstr.str().c_str(), iniFileName);
}
template<typename T>
inline void SetIniValue(LPCWSTR Section, LPCWSTR Setting, T value, LPCWSTR iniFileName) {
	wstringstream sstr;
	sstr << value;
	WritePrivateProfileStringW(Section, Setting, sstr.str().c_str(), iniFileName);
}

// Specific override for string values
template<>
inline void SetIniValue<std::wstring>(LPCWSTR Section, LPCWSTR Setting, std::wstring value, LPCWSTR iniFileName){
   WritePrivateProfileStringW(Section, Setting, value.c_str(), iniFileName);
}
template<>
inline void SetIniValue<std::string>(LPCSTR Section, LPCSTR Setting, std::string value, LPCSTR iniFileName) {
	WritePrivateProfileStringA(Section, Setting, value.c_str(), iniFileName);
}

// Specific override for TSTR values
#ifdef UNICODE
template<>
inline void SetIniValue<TSTR>(LPCWSTR Section, LPCWSTR Setting, TSTR value, LPCWSTR iniFileName){
   WritePrivateProfileStringW(Section, Setting, value.data(), iniFileName);
}
#else
template<>
inline void SetIniValue<TSTR>(LPCSTR Section, LPCSTR Setting, TSTR value, LPCSTR iniFileName) {
	WritePrivateProfileStringA(Section, Setting, value.data(), iniFileName);
}
#endif

#ifdef UNICODE
extern TSTR FormatText(const wchar_t* format,...);
#else
extern TSTR FormatText(const char* format, ...);
#endif
extern std::string FormatString(const char* format,...);
extern std::wstring FormatString(const wchar_t* format, ...);

extern stringlist TokenizeString(LPCSTR str, LPCSTR delims, bool trim=false);
extern wstringlist TokenizeString(LPCWSTR str, LPCWSTR delims, bool trim = false);
extern stringlist TokenizeCommandLine(LPCSTR str, bool trim);
extern wstringlist TokenizeCommandLine(LPCWSTR str, bool trim);
extern string JoinCommandLine(stringlist args);
extern wstring JoinCommandLine(wstringlist args);

extern string GetIndirectValue(LPCSTR path);
extern wstring GetIndirectValue(LPCWSTR path);
extern NameValueCollectionA ReadIniSection(LPCSTR Section, LPCSTR iniFileName );
extern NameValueCollectionW ReadIniSection(LPCWSTR Section, LPCWSTR iniFileName);
extern bool ReadIniSectionAsList(LPCSTR Section, LPCSTR iniFileName, NameValueListA& map);
extern bool ReadIniSectionAsList(LPCWSTR Section, LPCWSTR iniFileName, NameValueListW& map);

extern string ExpandQualifiers(const string& src, const NameValueCollectionA& map);
extern wstring ExpandQualifiers(const wstring& src, const NameValueCollectionW& map);
extern string ExpandEnvironment(const string& src);
extern wstring ExpandEnvironment(const wstring& src);

extern void FindImages(NameValueCollectionA& images, const string& rootPath, const stringlist& searchpaths, const stringlist& extensions);
extern void FindImages(NameValueCollectionW& images, const wstring& rootPath, const wstringlist& searchpaths, const wstringlist& extensions);

extern void RenameNode(Interface *gi, LPCSTR SrcName, LPCSTR DstName);
extern void RenameNode(Interface *gi, LPCWSTR SrcName, LPCWSTR DstName);

enum PosRotScale
{
   prsPos = 0x1,
   prsRot = 0x2,
   prsScale = 0x4,
   prsDefault = prsPos | prsRot | prsScale,
};
extern void PosRotScaleNode(INode *n, Point3 p, Quat& q, float s, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void PosRotScaleNode(Control *c, Point3 p, Quat& q, float s, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void PosRotScaleNode(INode *n, Matrix3& m3, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void PosRotScaleNode(Control *c, Matrix3& m3, PosRotScale prs = prsDefault, TimeValue t = 0);
extern Matrix3 GetNodeLocalTM(INode *n, TimeValue t = 0);

extern Niflib::NiNodeRef FindNodeByName( const vector<Niflib::NiNodeRef>& blocks, const string& name );
extern Niflib::NiNodeRef FindNodeByName(const vector<Niflib::NiNodeRef>& blocks, const wstring& name);
extern std::vector<Niflib::NiNodeRef> SelectNodesByName( const vector<Niflib::NiNodeRef>& blocks, LPCSTR match);
extern std::vector<Niflib::NiNodeRef> SelectNodesByName(const vector<Niflib::NiNodeRef>& blocks, LPCWSTR match);
extern int CountNodesByName( const vector<Niflib::NiNodeRef>& blocks, LPCSTR match );
extern int CountNodesByName(const vector<Niflib::NiNodeRef>& blocks, LPCWSTR match);
extern std::vector<std::string> GetNamesOfNodes( const vector<Niflib::NiNodeRef>& blocks );
extern std::vector<Niflib::NiNodeRef> SelectNodesByName( const vector<Niflib::NiNodeRef>& blocks, LPCSTR match);
extern std::vector<Niflib::NiNodeRef> SelectNodesByName(const vector<Niflib::NiNodeRef>& blocks, LPCWSTR match);
extern int CountNodesByType(const vector<Niflib::NiObjectRef>& blocks, Niflib::Type);

extern INode* FindINode(Interface *i, const string& name);
extern INode* FindINode(Interface *i, const wstring& name);
extern INode* FindINode(Interface *i, Niflib::NiObjectNETRef node);

struct NodeEquivalence
{
   bool operator()(const Niflib::NiNodeRef& lhs, const Niflib::NiNodeRef& rhs) const{
      return (!lhs || !rhs) ? (lhs < rhs) : (lhs->GetName() < rhs->GetName());
   }
   bool operator()(const Niflib::NiNodeRef& lhs, const std::string& rhs) const{
      return (lhs->GetName() < rhs);
   }
   bool operator()(const std::string& lhs, const Niflib::NiNodeRef& rhs) const{
      return (lhs < rhs->GetName());
   }
};

inline Niflib::NiNodeRef BinarySearch(vector<Niflib::NiNodeRef> &nodes, const string& name)
{
   typedef std::pair<vector<Niflib::NiNodeRef>::iterator, vector<Niflib::NiNodeRef>::iterator> NiNodePair;
   NiNodePair pair = std::equal_range(nodes.begin(), nodes.end(), name, NodeEquivalence());
   if (pair.first != pair.second) {
      return (*pair.first);
   }
   return Niflib::NiNodeRef();
}

// Simple conversion helpers
static inline float TODEG(float x) { return x * 180.0f / PI; }
static inline float TORAD(float x) { return x * PI / 180.0f; }

static inline Color TOCOLOR(const Niflib::Color3& c3) {
   return Color(c3.r, c3.g, c3.b);
}

static inline Niflib::Color3 TOCOLOR3(const Color& c3) {
   return Niflib::Color3(c3.r, c3.g, c3.b);
}

static inline Niflib::Color3 TOCOLOR3(const Point3& c3) {
   return Niflib::Color3(c3.x, c3.y, c3.z);
}

static inline Niflib::Color3 TOCOLOR3(const Niflib::Color4& c4) {
	return Niflib::Color3(c4.r, c4.g, c4.b);
}

static inline Color TOCOLOR(const Niflib::Color4& c3) {
	return Color(c3.r, c3.g, c3.b);
}

static inline Niflib::Color4 TOCOLOR4(const Color& c3) {
	return Niflib::Color4(c3.r, c3.g, c3.b);
}

static inline Niflib::Color4 TOCOLOR4(const Niflib::Color3& c3) {
	return Niflib::Color4(c3.r, c3.g, c3.b, 1.0f);
}

static inline Point3 TOPOINT3(const Niflib::Color3& c3){
   return Point3(c3.r, c3.g, c3.b);
}

static inline Point3 TOPOINT3(const Niflib::Vector3& v){
   return Point3(v.x, v.y, v.z);
}

static inline Point3 TOPOINT3(const Niflib::Vector4& v){
	return Point3(v.x, v.y, v.z);
}

static inline Niflib::Vector3 TOVECTOR3(const Point3& v){
   return Niflib::Vector3(v.x, v.y, v.z);
}

static inline Niflib::Vector3 TOVECTOR3(const Niflib::Vector4& v){
	return Niflib::Vector3(v.x, v.y, v.z);
}

static inline Niflib::Vector4 TOVECTOR4(const Point3& v, float w = 0.0){
	return Niflib::Vector4(v.x, v.y, v.z, w);
}


static inline Quat TOQUAT(const Niflib::Quaternion& q, bool inverse = false){
   Quat qt(q.x, q.y, q.z, q.w);
   return (inverse && q.w != FloatNegINF) ? qt.Inverse() : qt;
}

static inline Quat TOQUAT(const Niflib::QuaternionXYZW& q, bool inverse = false){
   Quat qt(q.x, q.y, q.z, q.w);
   return (inverse && q.w != FloatNegINF) ? qt.Inverse() : qt;
}

static inline Niflib::Quaternion TOQUAT(const Quat& q, bool inverse = false){
   return (inverse && q.w != FloatNegINF) ? TOQUAT(q.Inverse(), false) : Niflib::Quaternion(q.w, q.x, q.y, q.z);
}

static inline Niflib::QuaternionXYZW TOQUATXYZW(const Niflib::Quaternion& q){
   Niflib::QuaternionXYZW qt;
   qt.x = q.x; qt.y = q.y; qt.z = q.z; qt.w = q.w;
   return qt;
}

static inline Niflib::QuaternionXYZW TOQUATXYZW(const Quat& q){
   Niflib::QuaternionXYZW qt;
   qt.x = q.x; qt.y = q.y; qt.z = q.z; qt.w = q.w;
   return qt;
}

static inline AngAxis TOANGAXIS(const Niflib::Quaternion& q, bool inverse = false){
   Quat qt(q.x, q.y, q.z, q.w);
   if (inverse && q.w != FloatNegINF) qt.Invert();
   return AngAxis(q.x, q.y, q.z, q.w);
}

static inline Matrix3 TOMATRIX3(const Niflib::Matrix44 &tm, bool invert = false){
   Niflib::Vector3 pos; Niflib::Matrix33 rot; float scale;
   tm.Decompose(pos, rot, scale);
   Matrix3 m(rot.rows[0].data, rot.rows[1].data, rot.rows[2].data, Point3());
   if (invert) m.Invert();
   m.Scale(Point3(scale, scale, scale));
   m.SetTrans(Point3(pos.x, pos.y, pos.z));
   return m;
}

static inline Matrix3 TOMATRIX3INERTIA(const Niflib::InertiaMatrix &tm) {
	Matrix3 m(Point3(tm[0][0], tm[0][1], tm[0][2]), Point3(tm[1][0], tm[1][1], tm[1][2]), Point3(tm[2][0], tm[2][1], tm[2][2]), Point3());
	return m;
}

static inline Niflib::InertiaMatrix TOINERTIAMATRIX(const Matrix3 &tm) {
	Niflib::InertiaMatrix im(tm.GetRow(0)[0], tm.GetRow(0)[1], tm.GetRow(0)[2], 0,
		tm.GetRow(1)[0], tm.GetRow(1)[1], tm.GetRow(1)[2], 0,
		tm.GetRow(2)[0], tm.GetRow(2)[1], tm.GetRow(2)[2], 0);
	return im;
}

static inline Niflib::Matrix33 TOMATRIX33(const Matrix3 &tm, bool invert = false){
	Niflib::Matrix33 m3(tm.GetRow(0)[0], tm.GetRow(0)[1], tm.GetRow(0)[2],
		tm.GetRow(1)[0], tm.GetRow(1)[1], tm.GetRow(1)[2],
		tm.GetRow(2)[0], tm.GetRow(2)[1], tm.GetRow(2)[2]);
	return m3;
}

static inline Matrix3 TOMATRIX3(Niflib::Vector3& trans, Niflib::QuaternionXYZW quat, float scale){
	Matrix3 tm(true), qm;
	Quat q(quat.x, quat.y, quat.z, quat.w);
	q.MakeMatrix(qm);
	tm.SetTranslate(TOPOINT3(trans));
	tm *= qm;
	tm *= ScaleMatrix(Point3(scale, scale, scale));
	return tm;
}

static inline Matrix3 TOMATRIX3(Niflib::Vector3& trans, Niflib::Quaternion quat, float scale){
	Matrix3 tm, qm;
	Quat q(quat.x, quat.y, quat.z, quat.w);
	q.MakeMatrix(qm);
	tm.SetTranslate(TOPOINT3(trans));
	tm *= qm;
	tm *= ScaleMatrix(Point3(scale, scale, scale));
	return tm;
}

static inline Niflib::Matrix44 TOMATRIX4(const Matrix3 &tm, bool invert = false){
   Niflib::Matrix33 m3(tm.GetRow(0)[0], tm.GetRow(0)[1], tm.GetRow(0)[2],
                       tm.GetRow(1)[0], tm.GetRow(1)[1], tm.GetRow(1)[2],
                       tm.GetRow(2)[0], tm.GetRow(2)[1], tm.GetRow(2)[2]);
   Niflib::Matrix44 m4(TOVECTOR3(tm.GetTrans()), m3, 1.0f);
   return m4;
}

static inline Point3 GetScale(const Matrix3& mtx){
   return Point3( fabs(mtx.GetRow(0)[0]), fabs(mtx.GetRow(1)[1]), fabs(mtx.GetRow(2)[2]) );
}

static inline float Average(const Point3& val) {
   return (val[0] + val[1] + val[2]) / 3.0f;
}

static inline float Average(const Niflib::Vector3& val) {
   return (val.x + val.y + val.z) / 3.0f;
}

static inline Niflib::TexCoord TOTEXCOORD(const Niflib::HalfTexCoord& b) {
	return Niflib::TexCoord(Niflib::ConvertHFloatToFloat(b.u), Niflib::ConvertHFloatToFloat(b.v));
}

static inline Niflib::HalfTexCoord TOHTEXCOORD(const Niflib::TexCoord& b) {
	Niflib::HalfTexCoord a;
	a.u = Niflib::ConvertFloatToHFloat(b.u);
	a.v = Niflib::ConvertFloatToHFloat(b.v);
	return a;
}

static inline Niflib::Vector3 TOVECTOR3(const Niflib::HalfVector3& b) {
	return Niflib::Vector3(Niflib::ConvertHFloatToFloat(b.x), Niflib::ConvertHFloatToFloat(b.y), Niflib::ConvertHFloatToFloat(b.z));
}

static inline Niflib::HalfVector3 TOHVECTOR3(const Niflib::Vector3& b) {
	Niflib::HalfVector3 a;
	a.x = Niflib::ConvertFloatToHFloat(b.x);
	a.y = Niflib::ConvertFloatToHFloat(b.y);
	a.z = Niflib::ConvertFloatToHFloat(b.z);
	return a;
}

template <typename U, typename T>
inline Niflib::Ref<U> SelectFirstObjectOfType( vector<Niflib::Ref<T> > const & objs ) {
   for (vector<Niflib::Ref<T> >::const_iterator itr = objs.begin(), end = objs.end(); itr != end; ++itr) {
      Niflib::Ref<U> obj = DynamicCast<U>(*itr);
      if (obj) return obj;
   }
   return Niflib::Ref<U>();
}

template <typename U, typename T>
inline Niflib::Ref<U> SelectFirstObjectOfType( list<Niflib::Ref<T> > const & objs ) {
   for (list<Niflib::Ref<T> >::const_iterator itr = objs.begin(), end = objs.end(); itr != end; ++itr) {
      Niflib::Ref<U> obj = DynamicCast<U>(*itr);
      if (obj) return obj;
   }
   return Niflib::Ref<U>();
}

TSTR PrintMatrix3(Matrix3& m);
TSTR PrintMatrix44(Niflib::Matrix44& m);

extern Modifier *GetOrCreateSkin(INode *node);
extern Modifier *GetSkin(INode *node);
extern TriObject* GetTriObject(Object *o);

extern TSTR GetFileVersion(const char *fileName);
extern TSTR GetFileVersion(const wchar_t *fileName);

template<typename T>
inline Niflib::Ref<T> CreateNiObject() {
   return Niflib::StaticCast<T>(T::Create());
}

void CollapseGeomTransform(Niflib::NiTriBasedGeomRef shape);
void CollapseGeomTransforms(std::vector<Niflib::NiTriBasedGeomRef>& shapes);
void FixNormals(std::vector<Niflib::Triangle>& tris, std::vector<Niflib::Vector3>& verts, std::vector<Niflib::Vector3>& norms);

Modifier *GetbhkCollisionModifier(INode* node);
Modifier *CreatebhkCollisionModifier(INode* node, int type, int materialIndex, int layerIndex, byte filter);

void GetIniFileName(char *iniName);
void GetIniFileName(wchar_t *iniName);

Matrix3 GetLocalTM(INode *node);

// Morph related routines in nimorph.cpp
extern Modifier *GetMorpherModifier(INode* node);
extern Modifier *CreateMorpherModifier(INode* node);
extern void MorpherBuildFromNode(Modifier* mod, int index, INode *target);
extern void MorpherSetName(Modifier* mod, int index, TSTR& name);
extern void MorpherRebuild(Modifier* mod, int index);
extern TSTR MorpherGetName(Modifier* mod, int index);
extern bool MorpherIsActive(Modifier* mod, int index);
extern bool MorpherHasData(Modifier* mod, int index);
extern int MorpherNumProgMorphs(Modifier* mod, int index);
extern INode *MorpherGetProgMorph(Modifier* mod, int index, int morphIdx);
extern void MorpherGetMorphVerts(Modifier* mod, int index, vector<Niflib::Vector3>& verts);

#pragma region Enumeration support
// Enumeration support
typedef struct EnumLookupType {
   int value;
   const TCHAR *name;
} EnumLookupType;

extern TSTR EnumToString(int value, const EnumLookupType *table);
extern LPCTSTR EnumToStringRaw(int value, const EnumLookupType *table);
extern int StringToEnum(TSTR value, const EnumLookupType *table);
extern int EnumToIndex(int value, const EnumLookupType *table);

extern TSTR FlagsToString(int value, const EnumLookupType *table);
extern int StringToFlags(TSTR value, const EnumLookupType *table);
#pragma endregion

extern unsigned long Crc32Array(const void *data, size_t size);

extern bool GetTexFullName(Texmap *texMap, TSTR& fName);
extern bool GetTexFullName(Texmap *texMap, tstring& fName);

#endif // _NIUTILS_H_