#ifndef STOP_DEBUG
#ifndef _XBOX

#ifndef _APP_INTERNAL_DEBUGSYMBOLS_HEADER_
#define _APP_INTERNAL_DEBUGSYMBOLS_HEADER_

#include <windows.h>
#include <dbghelp.h>

class DebugSymbols
{

	//Определение базовых типов
	enum BasicType
	{
		//...
		btNoType = 0,

		//void
		btVoid = 1,

		//char (sym.length==1)
		//char_XXX
		btChar = 2,

		//wchar_t
		btWChar = 3,

		//signed char (sym.length==1)
		//short       (sym.length==2)
		//int         (sym.length==4)
		//__int_XXX
		btInt = 6,

		//unsigned char    (sym.length==1)
		//unsigned short   (sym.length==2)
		//unsigned int     (sym.length==4)
		//__uint_XXX
		btUInt = 7,


		//float      (sym.length==4)
		//double     (sym.length==8)
		//__float_XXX
		btFloat = 8,

		//BCD (binary-coded decimal)
		btBCD = 9,

		//bool   (sym.length==1)
		//__bool_XXX
		btBool = 10,

		//long int (sym.length==PTRSIZE)
		//__long_int_XXX
		btLong = 13,

		//unsigned long int (sym.length==PTRSIZE)
		//__unsigned_long_int_XXX
		btULong = 14,

		//CURRENCY
		btCurrency = 25,

		//DATE
		btDate = 26,

		//VARIANT
		btVariant = 27,

		//COMPLEX (complex number)
		btComplex = 28,

		//BIT
		btBit = 29,

		//BSTR
		btBSTR = 30,

		//HRESULT
		btHresult = 31
	};


	enum SymTag
	{ 
		SymTagNull,
		SymTagExe,
		SymTagCompiland,
		SymTagCompilandDetails,
		SymTagCompilandEnv,
		SymTagFunction,
		SymTagBlock,
		SymTagData,
		SymTagAnnotation,
		SymTagLabel,
		SymTagPublicSymbol,
		SymTagUDT,
		SymTagEnum,
		SymTagFunctionType,
		SymTagPointerType,
		SymTagArrayType, 
		SymTagBaseType, 
		SymTagTypedef, 
		SymTagBaseClass,
		SymTagFriend,
		SymTagFunctionArgType, 
		SymTagFuncDebugStart, 
		SymTagFuncDebugEnd,
		SymTagUsingNamespace, 
		SymTagVTableShape,
		SymTagVTable,
		SymTagCustom,
		SymTagThunk,
		SymTagCustomType,
		SymTagManagedType,
		SymTagDimension
	};


	HANDLE process;

	bool bInited;


public:

	static BOOL CALLBACK EnumerateLoadedModulesProc(PCSTR name, DWORD64 base, ULONG size, PVOID data);
	static BOOL CALLBACK EnumerateSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);

	static const char * basicTypeToString (DebugSymbols::BasicType type, ULONG64 len);
	static const char * TagToString (DebugSymbols::SymTag type);

	DWORD_PTR m_stackFrameAddrFrameOffset;

public:



	DebugSymbols();
	~DebugSymbols();

	DWORD_PTR getAddrFrameOffset();

	bool Init(const char * szPDBSearchPathsCanDivideBySemicolon = NULL);
	void Close();

	BOOL enumSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);

	const char * GetFunctionParams(DWORD64 adr);
	const char * GetSymbolInfo(DWORD64 adr, DWORD_PTR stackFrameAddrFrameOffset = NULL);
	int GetType (ULONG64 ModBase, ULONG TypeIndex, char * result, char * resultValue, int ptrCount, DWORD_PTR var_data);

	const char * GetValue(BasicType basicType, ULONG64 length, DWORD_PTR pAddress, int ptrCount);

	void PrintPDBSearchPath(FILE * f);
};



#endif

#endif
#endif