// string class
#ifndef STRING_HPP
#define STRING_HPP

#include "tcommon.h"
#include "..\StringUtils\string_hash.h"

//#define ENABLE_STRING_PROFILE
//#define ENABLE_STRING_TRACE

#define STRING_DEFAULT_ADD		16

#ifdef ENABLE_STRING_TRACE
inline void string_trace(const char * ptr)
{
	char buffer[4096];
	if (ptr == 0) return;

	HANDLE file_h = CreateFile("strings_log.txt", GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	SetFilePointer(file_h, 0, 0, FILE_END);

	crt_strcpy(buffer, sizeof(buffer), ptr);
	strcat_s(buffer, sizeof(buffer), "\x0d\x0a");
	DWORD bytes;
	WriteFile(file_h, buffer, strlen(buffer), &bytes,0);
	CloseHandle(file_h);
	_flushall();
}
#endif

class string 
{
private:
	char	* pBuffer;
	dword	dwSize;

	dword	dwCurSize, dwAddSize;

	void Init(dword _dwAdd)
	{
		pBuffer = null;
		dwSize = 0;
		dwCurSize = 0;
		dwAddSize = (_dwAdd==0) ? STRING_DEFAULT_ADD : _dwAdd;
	}

	inline void	SetZero()	
	{ 
		if (pBuffer) pBuffer[Len()] = 0; 
	};

public:

	// functions area
	void Reserve(dword dwReserveSize)
	{
		dword dwNewSize = dwAddSize * (1 + dwReserveSize / dwAddSize);
		if(dwNewSize != 0 && dwNewSize < dwAddSize) dwNewSize = dwAddSize;
		if (dwCurSize >= dwNewSize) return;
		dwCurSize = dwNewSize;
		pBuffer = (char*)RESIZE(pBuffer, dwCurSize);
		SetZero();
	}

	inline bool			IsEmpty()  	const { return (dwSize == 0); }
	static inline bool	IsEmpty(const char * c) { return !c || !c[0]; }
	inline bool			NotEmpty()  	const { return (dwSize > 0); }
	static inline bool	NotEmpty(const char * c) { return (c != null) && (c[0] != 0); }
	inline const char *	GetBuffer()	const { return pBuffer ? pBuffer : ""; }
	inline const char *	c_str()		const { return pBuffer ? pBuffer : ""; }
	inline dword		Len()  		const { return dwSize; }	
	static inline dword	Len(const char * str){ return Size(str); }	
	inline dword		Size()  	const { return Len(); }	
	static inline dword Size(const char * str)
	{
		if(!str) return 0;
		const char * s;
		for(s = str; *s; s++);
		return (dword)(s - str);
	}

	// return pointer for direct operations with string buffer
	inline char *		GetDataBuffer()	{ Assert(pBuffer); return pBuffer; };
	// return string buffer size for direct operations with string buffer
	inline dword		GetDataSize()	{ return dwCurSize; };
	// set new string size
	inline void			SetDataSize(dword dwNewSize)	{ Assert(dwNewSize <= dwCurSize); dwSize = dwNewSize; };

	// Operator return string length
	inline dword operator () (void) const { return Size(); }

	// operators area

	// string compare
	inline bool operator == (const char * pString) const { return (pString) ? IsEqual(GetBuffer(), pString) : false; }
	inline bool operator == (const string & s1) const { if(Len() == s1.Len()) return *this == s1.GetBuffer(); else return false; }

	inline bool operator != (const char * pString) const { return !(*this == pString); }
	inline bool operator != (const string & s1) const { return !(*this == s1); }

	inline bool operator > (const char * pString) const { return (pString) ? Compare(GetBuffer(), pString) > 0 : false; }
	inline bool operator > (const string & s1) const { return (*this > s1.GetBuffer()); }

	inline bool operator < (const char * pString) const { return !(*this >= pString); }
	inline bool operator < (const string & s1) const { return !(*this >= s1.GetBuffer()); }

	inline bool operator >= (const char * pString) const { return (pString) ? Compare(GetBuffer(), pString) >= 0 : false; }
	inline bool operator >= (const string & s1) const { return (*this >= s1.GetBuffer()); }

	inline bool operator <= (const char * pString) const { return !(*this > pString); }
	inline bool operator <= (const string & s1) const { return !(*this > s1.GetBuffer()); }

	inline bool CompareLen(const string & s, dword dwLen)
	{
		if (Len() < dwLen || s.Len() < dwLen) return false;
		return _strnicmp(GetBuffer(), s.GetBuffer(), dwLen) == 0;
	}

	//Сравнить знаково строки игнорируя регистр
	static inline long Compare(const char * s1, const char * s2)
	{ 
		if(!s1) s1 = "";
		if(!s2) s2 = "";
		return _stricmp(s1, s2);
	}

	//Сравнить игнорируя регистр
	static inline bool IsEqual(const char * s1, const char * s2)
	{ 		
		if(!s1) s1 = "";
		if(!s2) s2 = "";
		while(true)
		{
			char c1 = *s1;
			char c2 = *s2;
			if(c1 != c2)
			{
				if((*s1 | *s2) & 0x80)
				{
					return _stricmp(s1, s2) == 0;
				}
				if(c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
				if(c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
				if(c1 != c2) return false;
			}
			if(c1 == 0) return true;
			s1++;
			s2++;
		}
		return false;
	}
	//Сравнить игнорируя регистр
	static inline bool NotEqual(const char * s1, const char * s2)
	{ 
		return !IsEqual(s1, s2);
	}

	//Сравнить строки  с учётом регистра и длинны
	static inline bool IsEqual(const char * str1, long len1, const char * str2, long len2)
	{
		if(len1 != len2)
		{
			return false;
		}
		for(; len1 > 0; len1--, str1++, str2++)
		{
			if(*str1 != *str2)
			{
				return false;
			}
		}
		return true;
	}

	//Сравнить строки  с учётом регистра и длинны
	static inline bool NotEqual(const char * str1, long len1, const char * str2, long len2)
	{
		return !IsEqual(str1, len1, str2, len2);
	}


	// type conversion, to (const char *)
	inline operator const char * () const { return GetBuffer(); }

	string & operator = (const char * pText) 
	{ 
		if (!pText || !pText[0]) { Empty(); return *this;}
		dwSize = (dword)strlen(pText);

		Reserve(dwSize);
		memcpy(pBuffer,pText,dwSize); SetZero();

		return (*this);
	}

	string & operator = (const string & sText) 
	{ 
		dwSize = sText.Len();
		if (sText.IsEmpty()) { Empty(); return *this; }

		Reserve(dwSize);
		memcpy(pBuffer,sText,dwSize); SetZero();

		return *this;
	}

	string & operator += (const string & sText) 
	{
		if (sText.IsEmpty()) return (*this);
		dword dwNewSize = Len() + sText.Len();
		Reserve(dwNewSize);
		memcpy(&pBuffer[dwSize],sText,sText.Len()); 
		dwSize = dwNewSize;
		SetZero();
		return *this;
	}


	string & operator += (float fValue) 
	{
		string tmp;
		tmp.Format("%f", fValue);
		*this += tmp;
		return *this;
	}

	string & operator += (long lValue) 
	{
		string tmp;
		tmp.Format("%d", lValue);

		*this += tmp;
		return *this;
	}

	string & operator += (dword dwValue) 
	{
		string tmp;
		tmp.Format("%d", dwValue);

		*this += tmp;
		return *this;
	}


	string & operator += (char cSym) 
	{
		Reserve(dwSize + 1);
		pBuffer[dwSize] = cSym;
		dwSize++;
		SetZero();
		return *this;
	}

	string & operator += (const char * pText) 
	{
		if (!pText || !pText[0]) return *this;
		dword dwSLen = (dword)strlen(pText);
		dword dwNewSize = Len() + dwSLen;
		Reserve(dwNewSize);
		memcpy(&pBuffer[dwSize],pText,dwSLen); 
		dwSize = dwNewSize;
		SetZero();
		return *this;
	}


	friend string operator + (const string & s1, dword dwValue) 
	{
		string sRes = s1; sRes += dwValue; return sRes;
	}

	friend string operator + (const string & s1, float fValue) 
	{
		string sRes = s1; sRes += fValue; return sRes;
	}

	friend string operator + (const string & s1, long lValue) 
	{
		string sRes = s1; sRes += (long)lValue; return sRes;
	}


	friend string operator + (const string s1, const string & s2) 
	{		
		string sRes = s1; sRes += s2; return sRes;
	}

	friend string operator - (const string s1, const string & s2) 
	{		
		string sRes = s1; 
		if (EqualPostfix(sRes.c_str(), s2.c_str()))
		{
			sRes.dwSize -= s2.Len();
			sRes.SetZero();
		}
		return sRes;
	}

	string & _cdecl Format(const char *cFormat, ... ) 
	{
		char	str[1024];
		va_list args;
		va_start(args, cFormat);
		crt_vsnprintf(str, 1000, cFormat, args);
		va_end(args);

		*this = str;

		return *this;
	}

	string & Insert(dword dwIndex, const char * pInsString)
	{
		if (!pInsString) return *this;
		string sTmp = pInsString;
		return Insert(dwIndex, sTmp);
	}

	string & Delete(dword dwIndex, dword dwLen)
	{
		if (dwLen == 0 || dwIndex >= Len()) return *this;
		if (dwLen > Len() - dwIndex)
		{
			dwSize = dwIndex;
			SetZero();
			return *this;
		}
		memmove(&pBuffer[dwIndex], &pBuffer[dwIndex + dwLen], Len() - dwIndex - dwLen);
		dwSize = Len() - dwLen;
		SetZero();
		return *this;
	}

	string & Insert(dword dwIndex, const string & sInsString)
	{
		if (sInsString.IsEmpty()) return *this;
		if (dwIndex > Len()) dwIndex = Len();
		Reserve(Len() + sInsString.Len());
		dword dwShiftSize = Len() - dwIndex;
		if(dwShiftSize) memmove(&pBuffer[dwIndex + sInsString.Len()], &pBuffer[dwIndex], dwShiftSize);
		memcpy(&pBuffer[dwIndex], sInsString.GetBuffer(), sInsString.Len());
		dwSize += sInsString.Len();
		SetZero();
		return *this;
	}

	string & CheckPath()
	{
		dword dst, src;
		for(dst = 0, src = 0; src < Len(); src++)
		{
			char c = pBuffer[src];
			if(c == '/') c = '\\';
			if(c == '\\')
			{
				if(dst && pBuffer[dst - 1] == '\\')
				{
					continue;
				}
			}			
			pBuffer[dst++] = c;
		}
		dwSize = dst;
		SetZero();
		return *this;
	}

	string & TrimLeft()
	{
		if (!pBuffer || !dwSize) return *this;;

		char *pTemp = pBuffer;
		while ((*pTemp == ' ' || *pTemp == '\t') && (*pTemp != '\0')) pTemp++;

		// do nothing if string is already trimed
		if (pTemp == pBuffer) return *this;

		if (dword(pTemp - pBuffer) == Len()) dwSize = 0;
		else
		{
			dwSize = Len() - (pTemp - pBuffer);
			memcpy(pBuffer, pTemp, dwSize);
			SetZero();
		}
		return *this;
	}

	string & TrimRight()
	{
		if (!pBuffer || !dwSize) return *this;

		char *pTemp = pBuffer + Len() - 1;
		while ( (*pTemp == ' ' || *pTemp == '\t') && (pTemp >= pBuffer) ) pTemp--;

		// do nothing if string is already trimed
		if (pTemp == pBuffer + Len() - 1) return *this;

		if (pTemp < pBuffer) dwSize = 0;
		else 
		{ 
			dwSize = dword(pTemp - pBuffer) + 1; 
			SetZero(); 
		}
		return *this;
	}

	string & Trim()
	{
		TrimRight();
		TrimLeft();

		return *this;
	}

	// Convert a string to lowercase.
	string & Lower()	{ if (pBuffer) _strlwr_s(pBuffer, Len() + 1); return *this; }
	static void Lower(char * str, dword len){ if(str) _strlwr_s(str, len); }
	static void Lower(char * str){ if(str) _strlwr_s(str, Len(str) + 1); }
	

	// Convert a string to uppercase.
	string & Upper()	{ if (pBuffer) _strupr_s(pBuffer, Len() + 1);	return *this; }
	static void Upper(char * str, dword len){ if(str) _strupr_s(str, len + 1); }
	static void Upper(char * str){ if(str) _strupr_s(str, Len(str) + 1); }

	bool GetToken(char * pDest, dword & dwShift, char * pSkip)
	{
		if (!pSkip || dwShift >= Len()-1) return false;
		for (dword i=dwShift;i<Len();i++) if (GetBuffer()[i] == pSkip[0])
		{
			crt_strncpy(pDest, i - dwShift,  &GetBuffer()[dwShift], i - dwShift - 1);
			pDest[i - dwShift] = 0;
			dwShift = i + 1;
			return true;
		}
		return true;
	}

	// constructors area
	string(dword _dwAdd = STRING_DEFAULT_ADD)
	{
		Init(_dwAdd);
	}

	// char * constructor
	string(const char * pText, dword _dwAdd = STRING_DEFAULT_ADD) 
	{
		Init(_dwAdd);
		if (!pText || !pText[0]) return;
		dwSize = (dword)strlen(pText);

		Reserve(dwSize);
		memcpy(pBuffer,pText,dwSize); SetZero();
	}

	// string constructor
	string(const string & sText, dword _dwAdd = STRING_DEFAULT_ADD) 
	{
		Init(_dwAdd);
		if (sText.IsEmpty()) return;
		(*this) = sText;
	}

	#ifdef ENABLE_STRING_PROFILE

	~string() {}

	#else

	~string()
	{
#ifdef ENABLE_STRING_TRACE
		string_trace(pBuffer);
#endif
		DelAll();
	}

	#endif

	void Empty()
	{
		dwSize = 0;
		SetZero();
	}

	void DelAll()
	{
		DELETE(pBuffer);
		dwSize = 0;
		dwCurSize = 0;
	}

	static inline unsigned long Hash(const char * str)
	{
		return StringHash(str);
	}

	static inline unsigned long Hash(const char * str, dword & len)
	{
		return StringHash(str, (unsigned long &)len);
	}


	static inline unsigned long HashNoCase(const char * str)
	{
		return StringHashNoCase(str);
	}

	static inline unsigned long HashNoCase(const char * str, dword & len)
	{
		return StringHashNoCase(str, (unsigned long &)len);
	}

	inline char Last()
	{
		return (pBuffer && Len()) ? pBuffer[Len() - 1] : 0;
	}

	template<class _T> inline char & operator [] (_T Index)
	{
		Assert(Index < (_T)Len());
		return pBuffer[Index];
	}

	// find substring
	long FindSubStr(const char * pStr) const
	{
		if (!pBuffer || !pStr || !pStr[0]) return -1;
		char * pFindSubStr = strstr(pBuffer, pStr);
		return (pFindSubStr) ? dword(pFindSubStr - pBuffer) : -1;
	}
	// if last symbol == cSymbol, delete it
	inline string & DeleteLastSymbol(char cSymbol = '\\')
	{
		if (!Len() || pBuffer[Len() - 1] != cSymbol) return *this;
		dwSize--;
		SetZero();
		return *this;
	}
	// add extension to path if is't exist
	inline string & AddExtention(const char * ext)
	{
		if(!ext || !ext[0]) return *this;
		dword esize = (dword)strlen(ext);
		if(Len() < esize)
		{
			*this += ext;
			return *this;
		}
		const char * pStr = GetBuffer() + Len() - 1;
		const char * pExt = ext + esize - 1;
		dword c = 0;
		for(c = 0; c < esize; c++)
		{
			char cs = *pStr--;
			char ce = *pExt--;
			if(cs >= 'A' && cs <= 'Z') cs += 'a' - 'A';
			if(ce >= 'A' && ce <= 'Z') ce += 'a' - 'A';
			if(cs != ce) break;
		}
		if(c == esize) return *this;
		*this += ext;
		return *this;		
	}
	// 
	inline long LastChar(char c) const 
	{
		const char * pStr = GetBuffer();
		long i;
		for(i = Len() - 1; i >= 0; i--) if(pBuffer[i] == c) break;
		return i;
	}
	// return relative path = this - relativePath, if can't return this
	inline string & GetRelativePath(const string & relativePath)
	{
		if (Len() < relativePath.Len()) return *this;

		for (dword i=0; i<relativePath.Len(); i++)
		{
			char cs = GetBuffer()[i];
			char ce = relativePath[i];
			if(cs >= 'A' && cs <= 'Z') cs += 'a' - 'A';
			if(ce >= 'A' && ce <= 'Z') ce += 'a' - 'A';
			if (cs != ce) return *this;
		}

		memcpy(pBuffer, &pBuffer[relativePath.Len()], Len() - relativePath.Len());
		dwSize = Len() - relativePath.Len();
		SetZero();
		return *this;
	}
	//
	inline string & GetFilePath(const string & fullPath)
	{
		Empty();
		long i = fullPath.LastChar('\\');
		if(i >= 0)
		{
			Reserve(i + 1);
			for(long j = 0; j <= i; j++) *this += fullPath.pBuffer[j];
		}
		return *this;
	}
	//
	inline string & GetFileName(const string & fullPath)
	{
		Empty();
		long i = fullPath.LastChar('\\');
		if(i >= 0)
		{
			long size = fullPath.dwSize;
			Reserve(size - i);
			for(i++; i < size; i++) *this += fullPath.pBuffer[i];
			SetZero();
		}else{
			*this = fullPath;
		}
		return *this;
	}
	//
	static inline const char * GetFileName(const char * fullPath)
	{
		if(!fullPath) return null;
		const char * name = fullPath;		
		while(true)
		{
			char c = *fullPath;
			if(!c) return name;
			fullPath++;
			if(c == '\\' || c == '/')
			{
				name = fullPath;
			}
		}
		return null;
	}
	//
	inline string & GetFileTitle(const string & fullPath)
	{
		GetFileName(fullPath);
		long i = LastChar('.');
		if(i >= 0)
		{
			Delete(i, dwSize - i + 1);
		}
		return *this;
	}
	//
	inline string & GetFileExt(const string & fullPath)
	{
		Empty();
		long i = fullPath.LastChar('.');
		if(i >= 0)
		{
			long size = fullPath.dwSize;
			Reserve(size - i);
			for(i++; i < size; i++) *this += fullPath.pBuffer[i];
		}
		return *this;
	}
	// return true if Filename - directory(last symbol must be '\')
	inline bool IsDir() const 
	{
		return (Len() && pBuffer[Len() - 1] == '\\');
	}

	// return true if Filename equal with sMask
	inline bool IsFileMask(const string & sMask) const
	{
		return IsFileMask(GetBuffer(), sMask.GetBuffer());
	}

	inline static bool IsFileMask(const char * file, const char * mask)
	{
		if(!file) file = "";
		if(!mask) mask = "";
		const char * fileEnd = file + Size(file);
		const char * maskEnd = mask + Size(mask);
		const char * maskExt;
		for(maskExt = maskEnd; maskExt >= mask && *maskExt != '.'; maskExt--);
		if(maskExt >= mask)
		{
			const char * maskE = maskExt;
			maskExt++;
			const char * fileExt;
			for(fileExt = fileEnd; fileExt >= file && *fileExt != '.'; fileExt--);
			const char * fileE = fileExt;
			if(fileExt >= file)
			{
				fileExt++;
			}else{
				fileExt = "";
			}
			if(!CheckMask(fileExt, fileEnd, maskExt, maskEnd))
			{
				return false;
			}
			return CheckMask(file, fileE, mask, maskE);
		}
		return CheckMask(file, fileEnd, mask, maskEnd);
	}

	inline static bool IsMask(const char * str, const char * mask)
	{
		if(!str) str = "";
		if(!mask) mask = "";
		return CheckMask(str, str + Size(str), mask, mask + Size(mask));
	}

	//return true if str ended by postfix string
	inline static bool EqualPostfix(const char * str, const char * postfix)
	{
		if(!str || !postfix) return false;
		if(!str[0] || !postfix[0]) return false;
		long slen = (long)Len(str);
		long plen = (long)Len(postfix);
		if(slen < plen) return false;
		if(IsEqual(str + slen - plen, postfix)) return true;
		return false;
	}

	inline static bool EqualPrefix(const char * str, const char * prefix)
	{
		if(!str || !prefix) return false;
		while(*prefix)
		{
			char c1 = *str++;
			char c2 = *prefix++;
			if(c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
			if(c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
			if(c1 != c2) return false;
		}
		return true;
	}

private:

	inline static bool CheckMask(const char * str, const char * strEnd, const char * mask, const char * maskEnd)
	{
		while(true)
		{
			if(mask >= maskEnd)
			{
				return (str >= strEnd);
			}
			//Анализируем звёздочку в маске
			if(*mask == '*')
			{
				mask++;
				if(mask >= maskEnd)
				{
					return true;
				}
				while(true)
				{
					if(CheckMask(str++, strEnd, mask, maskEnd))
					{
						return true;
					}
					if(str >= strEnd)
					{
						return false;
					}
				}
			}
			if(str >= strEnd)
			{
				return false;
			}
			if(*mask == '?')
			{
				str++;
				mask++;				
				continue;
			}
			char cm = *mask++;
			char cs = *str++;			
			if(cm != cs)
			{
				if(cm <= 'Z' && cm >= 'A') cm += 'a' - 'A';
				if(cs <= 'Z' && cs >= 'A') cs += 'a' - 'A';
				if(cm != cs)
				{
					return false;
				}				
			}			
		}
	}
};

class ConstString
{
public:
	__forceinline ConstString()
	{
		Empty();
	}

	__forceinline explicit ConstString(const char * s)
	{
		Set(s);
	}

	__forceinline ConstString(const char * s, dword h, dword l)
	{
		Set(s, h, l);
	}

	__forceinline ConstString & Set(const char * s)
	{
		str = s;
		len = 0;
		hash = string::HashNoCase(s, len);
		return *this;
	}

	__forceinline ConstString & Set(const char * s, dword h, dword l)
	{
		str = s;
		hash = h;
		len = l;
		return *this;
	}

	__forceinline void Empty()
	{
		str = null;
		len = 0;
		hash = 0;
	}

	__forceinline ConstString & operator = (const ConstString & constString)
	{
		str = constString.str;
		len = constString.len;
		hash = constString.hash;
		return *this;
	}

	__forceinline bool operator == (const ConstString & constString) const
	{
		if(hash == constString.hash && len == constString.len)
		{
			if(string::IsEqual(str, constString.str))
			{
				return true;
			}
		}
		return false;
	}
	
	__forceinline bool operator != (const ConstString & constString) const
	{
		return !(*this == constString);
	}

	__forceinline const char * c_str() const
	{
		return str;
	}

	__forceinline dword Hash() const
	{
		return hash;
	}

	__forceinline dword Len() const
	{
		return len;
	}

	__forceinline bool IsEmpty() const
	{
		return string::IsEmpty(str);
	}

	__forceinline bool NotEmpty() const
	{
		return string::NotEmpty(str);
	}

	__forceinline static const ConstString & EmptyObject()
	{
		static const ConstString emptyObject;
		return emptyObject;
	}

private:
	__forceinline operator const char * () const 
	{
		Assert(false);
		return null;
	}

	__forceinline ConstString & operator = (const char *)
	{
		Assert(false);
		return *this;
	}

private:
	const char * str;
	dword len;
	dword hash;
};

#endif

