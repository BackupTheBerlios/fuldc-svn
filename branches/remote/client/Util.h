/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if !defined(UTIL_H)
#define UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if !(defined(_WIN32) || defined(_WIN64))
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#include "Text.h"

template<typename T, bool flag> struct ReferenceSelector {
	typedef T ResultType;
};
template<typename T> struct ReferenceSelector<T,true> {
	typedef const T& ResultType;
};

template<typename T> class IsOfClassType {
public:
	template<typename U> static char check(int U::*);
	template<typename U> static float check(...);
public:
	enum { Result = sizeof(check<T>(0)) };
};

template<typename T> struct TypeTraits {
	typedef IsOfClassType<T> ClassType;
	typedef ReferenceSelector<T, ((ClassType::Result == 1) || (sizeof(T) > sizeof(char*)) ) > Selector;
	typedef typename Selector::ResultType ParameterType;
};

#define GETSET(type, name, name2) \
private: type name; \
public: TypeTraits<type>::ParameterType get##name2() const { return name; } \
	void set##name2(TypeTraits<type>::ParameterType a##name2) { name = a##name2; }

#define LIT(x) x, (sizeof(x)-1)

/** Evaluates op(pair<T1, T2>.first, compareTo) */
template<class T1, class T2, class op = equal_to<T1> >
class CompareFirst {
public:
	CompareFirst(const T1& compareTo) : a(compareTo) { }
	bool operator()(const pair<T1, T2>& p) { return op()(p.first, a); }
private:
	CompareFirst& operator=(const CompareFirst&);
	const T1& a;
};

/** Evaluates op(pair<T1, T2>.second, compareTo) */
template<class T1, class T2, class op = equal_to<T2> >
class CompareSecond {
public:
	CompareSecond(const T2& compareTo) : a(compareTo) { }
	bool operator()(const pair<T1, T2>& p) { return op()(p.second, a); }
private:
	CompareSecond& operator=(const CompareSecond&);
	const T2& a;
};

template<class T>
class LessIgnoreCase {
public:
	bool operator() (const T& t1, const T& t2) const { return Util::stricmp(t1, t2) < 0; }
};

template<class T1, class T2, class op= less_equal<T1> >
class SortFirst {
public:
	bool operator() (const pair<T1, T2>&p1, const pair<T1, T2>&p2) const { return op()(p1.first, p2.first); };
};

template<class T1, class T2, class op= less_equal<T1> >
class SortSecond {
public:
	bool operator()(const pair<T1, T2>&p1, const pair<T1, T2>&p2) { return op()(p1.second, p2.second); };
};

template<class T>
struct PointerHash {
#if _MSC_VER >= 1300
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
#endif
	size_t operator()(const T* a) const { return ((size_t)a)/sizeof(T); }
	bool operator()(const T* a, const T* b) { return a < b; }
};
template<>
struct PointerHash<void> {
	size_t operator()(const void* a) const { return ((size_t)a)>>2; }
};

/**
 * Compares two values
 * @return -1 if v1 < v2, 0 if v1 == v2 and 1 if v1 > v2
 */
template<typename T1>
inline int compare(const T1& v1, const T1& v2) { return (v1 < v2) ? -1 : ((v1 == v2) ? 0 : 1); }

class Flags {
	public:
		typedef int MaskType;

		Flags() : flags(0) { }
		Flags(const Flags& rhs) : flags(rhs.flags) { }
		Flags(MaskType f) : flags(f) { }
		bool isSet(MaskType aFlag) const { return (flags & aFlag) == aFlag; }
		bool isAnySet(MaskType aFlag) const { return (flags & aFlag) != 0; }
		void setFlag(MaskType aFlag) { flags |= aFlag; }
		void unsetFlag(MaskType aFlag) { flags &= ~aFlag; }
		Flags& operator=(const Flags& rhs) { flags = rhs.flags; return *this; }
	private:
		MaskType flags;
};

template<typename T>
class AutoArray {
	typedef T* TPtr;
public:
	explicit AutoArray(TPtr t) : p(t) { }
	explicit AutoArray(size_t size) : p(new T[size]) { }
	~AutoArray() { delete[] p; }
	operator TPtr() { return p; }
	AutoArray& operator=(TPtr t) { delete[] p; p = t; return *this; }
private:
	AutoArray(const AutoArray&);
	AutoArray& operator=(const AutoArray&);

	TPtr p;
};

class Util
{
public:
	static tstring emptyStringT;
	static string emptyString;
	static wstring emptyStringW;
	static const string TEMP_EXT;
	static const string ANTI_FRAG_EXT;

	static void initialize();

	/** Path of temporary storage */
	static string getTempPath() {
#if defined(_WIN32) || defined(_WIN64)
		TCHAR buf[MAX_PATH + 1];
		DWORD x = GetTempPath(MAX_PATH, buf);
		return Text::wideToUtf8(wstring(buf, x));
#else
		return "/tmp/";
#endif
	}

	/** Path of configuration files */
	static const string& getConfigPath() { return configPath; }
	static const string& getDataPath() { return dataPath; }
	static const string& getSystemPath() { return systemPath; }

	/** Path of file lists */
	static string getListPath() { return getConfigPath() + "FileLists" PATH_SEPARATOR_STR; }
	/** Notepad filename */
	static string getNotepadFile() { return getConfigPath() + "Notepad.txt"; }

	static string translateError(int aError) {
#if defined(_WIN32) || defined(_WIN64)
		LPVOID lpMsgBuf;
		DWORD chars = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			aError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			);
		if(chars == 0) {
			return string();
		}
		string tmp = Text::fromT((LPCTSTR)lpMsgBuf);
		// Free the buffer.
		LocalFree( lpMsgBuf );
		string::size_type i = 0;

		while( (i = tmp.find_first_of("\r\n", i)) != string::npos) {
			tmp.erase(i, 1);
		}
		return tmp;
#else // _WIN32
		return strerror(aError);
#endif // _WIN32
	}

	static string getFilePath(const string& path) {
		string::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != string::npos) ? path.substr(0, i + 1) : path;
	}
	static string getFileName(const string& path) {
		string::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != string::npos) ? path.substr(i + 1) : path;
	}
	static string getFileExt(const string& path) {
		string::size_type i = path.rfind('.');
		return (i != string::npos) ? path.substr(i) : Util::emptyString;
	}
	static string getLastDir(const string& path) {
		string::size_type i = path.rfind(PATH_SEPARATOR);
		if(i == string::npos)
			return Util::emptyString;
		string::size_type j = path.rfind(PATH_SEPARATOR, i-1);
		return (j != string::npos) ? path.substr(j+1, i-j-1) : path;
	}

	static wstring getFilePath(const wstring& path) {
		wstring::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != wstring::npos) ? path.substr(0, i + 1) : path;
	}
	static wstring getFileName(const wstring& path) {
		wstring::size_type i = path.rfind(PATH_SEPARATOR);
		return (i != wstring::npos) ? path.substr(i + 1) : path;
	}
	static wstring getFileExt(const wstring& path) {
		wstring::size_type i = path.rfind('.');
		return (i != wstring::npos) ? path.substr(i) : Util::emptyStringW;
	}
	static wstring getLastDir(const wstring& path) {
		wstring::size_type i = path.rfind(PATH_SEPARATOR);
		if(i == wstring::npos)
			return Util::emptyStringW;
		wstring::size_type j = path.rfind(PATH_SEPARATOR, i-1);
		return (j != wstring::npos) ? path.substr(j+1, i-j-1) : path;
	}

	static void decodeUrl(const string& aUrl, string& aServer, uint16_t& aPort, string& aFile);
	static string validateFileName(string aFile);
	static wstring validateFileName(wstring aFile);
	static string cleanPathChars(string aNick);
	
	static string formatBytes(const string& aString) {
		return formatBytes(toInt64(aString));
	}
	
	static string formatMessage(const string& nick, const string& message);
	static string toDOS(const string& tmp);

	static wstring getShortTimeString(time_t t = time(NULL) );

	static tstring replace(const tstring& aString, const tstring& fStr, const tstring& rStr);
	static string replace(const string& aString, const string& fStr, const string& rStr);

	static string trim(string aString) {
		return trim(aString, " \t\r\n");
	}
	static string trim(string aString, string aChars);

	static string getTimeString() {
		char buf[64];
		time_t _tt;
		time(&_tt);
		tm* _tm = localtime(&_tt);
		if(_tm == NULL) {
			strcpy(buf, "xx:xx:xx");
		} else {
			strftime(buf, 64, "%X", _tm);
		}
		return buf;
	}

	static wstring getTimeStringW() {
		wchar_t buf[64];
		time_t _tt;
		time(&_tt);
		tm* _tm = localtime(&_tt);
		if(_tm == NULL) {
			wcscpy(buf, _T("xx:xx:xx"));
		} else {
			wcsftime(buf, 64, _T("%X"), _tm);
		}
		return buf;
	}

	static string toAdcFile(const string& file) {
		if(file == "files.xml.bz2" || file == "MyList.DcLst")
			return file;

		string ret;
		ret.reserve(file.length() + 1);
		ret += '/';
		ret += file;
		for(string::size_type i = 0; i < ret.length(); ++i) {
			if(ret[i] == '\\') {
				ret[i] = '/';
			}
		}
		return ret;
	}
	static string toNmdcFile(const string& file) {
		if(file.empty())
			return Util::emptyString;
		
		string ret(file.substr(1));
		for(string::size_type i = 0; i < ret.length(); ++i) {
			if(ret[i] == '/') {
				ret[i] = '\\';
			}
		}
		return ret;
	}
	
	static string formatBytes(int64_t aBytes);
	static wstring formatBytesW(int64_t aBytes);

	static wstring formatExactSize(int64_t aBytes);

	static string formatSeconds(int64_t aSec) {
		char buf[64];
		sprintf(buf, "%01lu:%02d:%02d", (unsigned long)(aSec / (60*60)), (int)((aSec / 60) % 60), (int)(aSec % 60));
		return buf;
	}

	static wstring formatSecondsW(int64_t aSec) {
		wchar_t buf[64];
		swprintf(buf, L"%01I64d:%02d:%02d", aSec / (60*60), (int)((aSec / 60) % 60), (int)(aSec % 60));
		return buf;
	}

	static string formatParams(const string& msg, StringMap& params, bool filter);
	static wstring formatTime(const wstring &msg, const time_t t);
	static string formatTime(const string &msg, const time_t t);
	static string formatTime(int64_t aSec, bool shortString = true) {
		char buf[128];
		if(shortString)
			sprintf(buf, "%01d:%02d:%02d:%02d", (int)(aSec /(60*60*24)), (int)((aSec / (60*60)) % 24), (int)((aSec / 60) % 60), (int)(aSec % 60));
		else
			sprintf(buf, "%01d days %01d hours %01d minutes %01d seconds", (int)(aSec /(60*60*24)), (int)((aSec / (60*60)) % 24), (int)((aSec / 60) % 60), (int)(aSec % 60));
		return buf;
	}
	static wstring formatTimeW(int64_t aSec, bool shortString = true) {
		wchar_t buf[128];
		if(shortString)
			swprintf(buf, L"%01d:%02d:%02d:%02d", (int)(aSec /(60*60*24)), (int)((aSec / (60*60)) % 24), (int)((aSec / 60) % 60), (int)(aSec % 60));
		else
			swprintf(buf, L"%01d days %01d hours %01d minutes %01d seconds", (int)(aSec /(60*60*24)), (int)((aSec / (60*60)) % 24), (int)((aSec / 60) % 60), (int)(aSec % 60));
		return buf;
	}

	static int64_t toInt64(const string& aString) {
#if defined(_WIN32) || defined(_WIN64)
		return _atoi64(aString.c_str());
#else
		return atoll(aString.c_str());
#endif
	}

	static int toInt(const string& aString) {
		return atoi(aString.c_str());
	}
	static uint32_t toUInt32(const string& str) {
		return toUInt32(str.c_str());
	}
	static uint32_t toUInt32(const char* c) {
		return (uint32_t)atoi(c);
	}

	static int toInt(const tstring& aString) {
		return _wtoi(aString.c_str());
	}

	static uint32_t toUInt32(const tstring& str) {
		return toUInt32(str.c_str());
	}

	static uint32_t toUInt32(const wchar_t* c) {
		return static_cast<uint32_t>(_wtoi(c));
	}

	static double toDouble(const string& aString) {
		// Work-around for atof and locales...
		lconv* lv = localeconv();
		string::size_type i = aString.find_last_of(".,");
		if(i != string::npos && aString[i] != lv->decimal_point[0]) {
			string tmp(aString);
			tmp[i] = lv->decimal_point[0];
			return atof(tmp.c_str());
		}
		return atof(aString.c_str());
	}

	static float toFloat(const string& aString) {
		return (float)toDouble(aString.c_str());
	}

	static string toString(short val) {
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", (int)val);
		return buf;
	}
	static string toString(unsigned short val) {
		char buf[8];
		snprintf(buf, sizeof(buf), "%u", (unsigned int)val);
		return buf;
	}
	static string toString(int val) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d", val);
		return buf;
	}
	static string toString(unsigned int val) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%u", val);
		return buf;
	}
	static string toString(long val) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%ld", val);
		return buf;
	}
	static string toString(unsigned long val) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%lu", val);
		return buf;
	}
	static string toString(long long val) {
		char buf[32];
		snprintf(buf, sizeof(buf), I64_FMT, val);
		return buf;
	}
	static string toString(unsigned long long val) {
		char buf[32];
		snprintf(buf, sizeof(buf), U64_FMT, val);
		return buf;
	}
	static string toString(double val) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%0.2f", val);
		return buf;
	}

	static string toString(const StringList& lst) {
		if(lst.size() == 1)
			return lst[0];
		string tmp("[");
		for(StringList::const_iterator i = lst.begin(); i != lst.end(); ++i) {
			tmp += *i + ',';
		}
		if(tmp.length() == 1)
			tmp.push_back(']');
		else
			tmp[tmp.length()-1] = ']';
		return tmp;
	}

	static wstring toStringW( long val ) {
		wchar_t buf[32];
		swprintf(buf, L"%ld", val);
		return buf;
	}

	static wstring toStringW( int64_t val ) {
		wchar_t buf[32];
		swprintf(buf, L"%ld", val);
		return buf;
	}

	static wstring toStringW( DWORD val ) {
		wchar_t buf[16];
		swprintf(buf, L"%d", val);
		return buf;
	}

	static wstring toStringW( int val ) {
		wchar_t buf[16];
		swprintf(buf, L"%d", val);
		return buf;
	}

	static wstring toStringW( size_t val ) {
		wchar_t buf[16];
		swprintf(buf, L"%d", val);
		return buf;
	}

	static wstring toStringW( double val ) {
		wchar_t buf[32];
		swprintf(buf, L"%0.2f", val);
		return buf;
	}

	static string toHexEscape(char val) {
		char buf[sizeof(int)*2+1+1];
		snprintf(buf, sizeof(buf), "%%%X", val&0x0FF);
		return buf;
	}
	static char fromHexEscape(const string aString) {
		unsigned int res = 0;
		sscanf(aString.c_str(), "%X", &res);
		return static_cast<char>(res);
	}

	template<typename T>
	static T& intersect(T& t1, const T& t2) {
		for(typename T::iterator i = t1.begin(); i != t1.end();) {
			if(find_if(t2.begin(), t2.end(), bind1st(equal_to<typename T::value_type>(), *i)) == t2.end())
				i = t1.erase(i);
			else
				++i;
		}
		return t1;
	}

	static string encodeURI(const string& /*aString*/, bool reverse = false);
	static string getLocalIp();
	static bool isPrivateIp(string const& ip);
	/**
	 * Case insensitive substring search.
	 * @return First position found or string::npos
	 */
	static string::size_type findSubString(const string& aString, const string& aSubString, string::size_type start = 0) throw();
	static wstring::size_type findSubString(const wstring& aString, const wstring& aSubString, wstring::size_type start = 0) throw();

	/* Utf-8 versions of strnicmp and stricmp, unicode char code order (!) */
	static int stricmp(const char* a, const char* b);
	static int strnicmp(const char* a, const char* b, size_t n);

	static int stricmp(const wchar_t* a, const wchar_t* b) {
		while(*a && Text::toLower(*a) == Text::toLower(*b))
			++a, ++b;
		return ((int)Text::toLower(*a)) - ((int)Text::toLower(*b));
	}
	static int strnicmp(const wchar_t* a, const wchar_t* b, size_t n) {
		while(n && *a && Text::toLower(*a) == Text::toLower(*b))
			--n, ++a, ++b;

		return n == 0 ? 0 : ((int)Text::toLower(*a)) - ((int)Text::toLower(*b));
	}

	static int stricmp(const string& a, const string& b) { return stricmp(a.c_str(), b.c_str()); }
	static int strnicmp(const string& a, const string& b, size_t n) { return strnicmp(a.c_str(), b.c_str(), n); }
	static int stricmp(const wstring& a, const wstring& b) { return stricmp(a.c_str(), b.c_str()); }
	static int strnicmp(const wstring& a, const wstring& b, size_t n) { return strnicmp(a.c_str(), b.c_str(), n); }

	static string getOsVersion();

	static string getIpCountry (string IP);

	static bool getAway() { return away; }
	static void setAway(bool aAway) {
		away = aAway;
		if (away)
			awayTime = time(NULL);
	}

	static bool getManualAway() { return manualAway; }
	static void setManualAway(bool aManualAway) { manualAway = aManualAway;	}

	static string getAwayMessage();
	static void setAwayMessage(const string& aMsg) { awayMsg = aMsg; }

	static uint32_t rand();
	static uint32_t rand(uint32_t high) { return rand() % high; }
	static uint32_t rand(uint32_t low, uint32_t high) { return rand(high-low) + low; }
	static double randd() { return ((double)rand()) / ((double)0xffffffff); }

private:
	/** Per-user configuration */
	static string configPath;
	/** Global configuration */
	static string systemPath;
	/** Various resources (help files etc) */
	static string dataPath;

	static bool away;
	static bool manualAway;
	static string awayMsg;
	static time_t awayTime;

	typedef map<uint32_t, uint16_t> CountryList;
	typedef CountryList::iterator CountryIter;

	static CountryList countries;

};

/** Case insensitive hash function for strings */
struct noCaseStringHash {
#if _MSC_VER < 1300
	enum {bucket_size = 4};
	enum {min_buckets = 8};
#else
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
#endif // _MSC_VER == 1200

	size_t operator()(const string* s) const {
		return operator()(*s);
	}

	size_t operator()(const string& s) const {
		size_t x = 0;
		const char* end = s.data() + s.size();
		for(const char* str = s.data(); str < end; ) {
			wchar_t c = 0;
			int n = Text::utf8ToWc(str, c);
			if(n < 0) {
				x = x*32 - x + '_';
				str += abs(n);
			} else {
				x = x*32 - x + (size_t)Text::toLower(c);
				str += n;
			}
		}
		return x;
	}

	size_t operator()(const wstring* s) const {
		return operator()(*s);
	}
	size_t operator()(const wstring& s) const {
		size_t x = 0;
		const wchar_t* y = s.data();
		wstring::size_type j = s.size();
		for(wstring::size_type i = 0; i < j; ++i) {
			x = x*31 + (size_t)Text::toLower(y[i]);
		}
		return x;
	}
};

/** Case insensitive string comparison */
struct noCaseStringEq {
	bool operator()(const string* a, const string* b) const {
		return a == b || Util::stricmp(*a, *b) == 0;
	}
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a, b) == 0;
	}
	bool operator()(const wstring* a, const wstring* b) const {
		return a == b || Util::stricmp(*a, *b) == 0;
	}
	bool operator()(const wstring& a, const wstring& b) const {
		return Util::stricmp(a, b) == 0;
	}
};

/** Case insensitive string ordering */
struct noCaseStringLess {
	bool operator()(const string* a, const string* b) const {
		return Util::stricmp(*a, *b) < 0;
	}
	bool operator()(const string& a, const string& b) const {
		return Util::stricmp(a, b) < 0;
	}
	bool operator()(const wstring* a, const wstring* b) const {
		return Util::stricmp(*a, *b) < 0;
	}
	bool operator()(const wstring& a, const wstring& b) const {
		return Util::stricmp(a, b) < 0;
	}
};

template<class Key, class Traits = noCaseStringLess>

struct noCaseStringHashComp {
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;

	size_t operator()(const string* s) const {
		return operator()(*s);
	}

	size_t operator()(const string& s) const {
		size_t x = 0;
		const char* end = s.data() + s.size();
		for(const char* str = s.data(); str < end; ) {
			wchar_t c = 0;
			int n = Text::utf8ToWc(str, c);
			if(n < 0) {
				x = x*32 - x + '_';
				str += abs(n);
			} else {
				x = x*32 - x + (size_t)Text::toLower(c);
				str += n;
			}
		}
		return x;
	}

	size_t operator()(const wstring* s) const {
		return operator()(*s);
	}
	size_t operator()(const wstring& s) const {
		size_t x = 0;
		const wchar_t* y = s.data();
		wstring::size_type j = s.size();
		for(wstring::size_type i = 0; i < j; ++i) {
			x = x*31 + (size_t)Text::toLower(y[i]);
		}
		return x;
	}

	bool operator() (const Key& a, const Key& b) const { return comp(a, b); }



private:
	Traits comp;
};

#endif // !defined(UTIL_H)