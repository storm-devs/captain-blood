#ifndef STOP_DEBUG
#ifndef _XBOX

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Symbols.h"


#pragma comment(linker, "/defaultlib:dbghelp.lib")




DebugSymbols::DebugSymbols()
{
	m_stackFrameAddrFrameOffset = NULL;
	process = NULL;
	bInited = false;
}


DebugSymbols::~DebugSymbols()
{
	Close();
}


void DebugSymbols::Close()
{
	if (bInited)
	{
		SymCleanup(process);
		bInited = NULL;
 	}
}

void DebugSymbols::PrintPDBSearchPath(FILE * f)
{
	char searchPathCurrent[MAX_PATH];
	SymGetSearchPath(process, searchPathCurrent, MAX_PATH);

	if (f == NULL)
	{
		return;
	}

	fprintf(f, "PDB Search path = '%s'\n", searchPathCurrent);
}


bool DebugSymbols::Init(const char * szPDBSearchPathsCanDivideBySemicolon)
{
	if (bInited)
	{
		return false;
	}

	if (process == NULL)
	{
		process = GetCurrentProcess();
	}
	

	DWORD options = SYMOPT_DEFERRED_LOADS; //SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME;
	SymSetOptions(options);

	OutputDebugStringA("pdb search path = '");
	OutputDebugStringA(szPDBSearchPathsCanDivideBySemicolon);
	OutputDebugStringA("'\n");

	BOOL bInv = options & SYMOPT_DEFERRED_LOADS;
	if (SymInitialize(process, (char*)szPDBSearchPathsCanDivideBySemicolon,  bInv) == false)
	{
/*
		DWORD dwLastError = GetLastError();
		static char szBuffer[512] = { 0 };
		FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandleA( "NTDLL.DLL"), dwLastError, 0, szBuffer, sizeof( szBuffer ), 0);
*/

		return false;
	}

	// if we are not deferred load, walk our loaded modules and load debug info
	if ((options & SYMOPT_DEFERRED_LOADS) == 0)
	{
		// load debugging information
		EnumerateLoadedModules64(process, &EnumerateLoadedModulesProc, NULL);
	}

	bInited = true;
	return true;
}

const char * DebugSymbols::GetSymbolInfo(DWORD64 adr, DWORD_PTR stackFrameAddrFrameOffset)
{
	m_stackFrameAddrFrameOffset = stackFrameAddrFrameOffset;

	static char module[MAX_PATH];
	ZeroMemory(&module, sizeof(module));
	static char extension[MAX_PATH];
	ZeroMemory(&extension, sizeof(extension));

	static char symbol[MAX_SYM_NAME+16];
	ZeroMemory(&symbol, sizeof(symbol));

	static char filename[MAX_PATH];
	ZeroMemory(&filename, sizeof(filename));

	DWORD line = 0xFFFFFFFF;

	// resulting line is worst case of all components
	static char result[sizeof(module) + sizeof(symbol) + sizeof(filename) + 64];
	ZeroMemory(&result, sizeof(result));


	//
	// Start by finding the module the address is in
	//
	IMAGEHLP_MODULE64 moduleInfo;
	ZeroMemory(&moduleInfo, sizeof(IMAGEHLP_MODULE64));
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	if (SymGetModuleInfo64(process, adr, &moduleInfo))
	{
		// success, copy the module info
		_splitpath(moduleInfo.ImageName, NULL, NULL, module, extension);
		strcat(module, extension);

		//
		// Now find symbol information
		//

		// displacement of the symbol
		DWORD64 disp;

		// okay, the name runs off the end of the structure, so setup a buffer and cast it
		ULONG64 buffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME*sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
		PSYMBOL_INFO symbolInfo = (PSYMBOL_INFO)buffer;
		symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbolInfo->MaxNameLen = MAX_SYM_NAME;

		if (SymFromAddr(process, adr, &disp, symbolInfo) != 0 )
		{
			// success, copy the symbol info
			sprintf(symbol, "%s", symbolInfo->Name);

			const char * funcParams = GetFunctionParams(adr);

			//
			// Now find source line information
			//

			DWORD d;
			IMAGEHLP_LINE64 l;
			ZeroMemory(&l,sizeof(l));
			l.SizeOfStruct = sizeof(l);
			if ( SymGetLineFromAddr64(process, adr, &d, &l) !=0 )
			{
				// success, copy the source file name
				strcpy(filename, l.FileName);
				static char ext[MAX_PATH];
				static char file[MAX_PATH];
				_splitpath(filename, NULL, NULL, file, ext);

				sprintf(result, "%s!%s(%s) + 0x%x : %s%s(%d)", module, symbol, funcParams, (DWORD)disp, file, ext, l.LineNumber);
				return result;
			}

			sprintf(result, "%s!%s(%s) + 0x%x", module, symbol, funcParams, (DWORD)disp);
			return result;
		}

		sprintf(result, "%s", module);
		return result;
	} else
	{
		DWORD err = GetLastError();
		static char szErrorBuffer[512] = { 0 };
		FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandleA( "NTDLL.DLL"), err, 0, szErrorBuffer, sizeof( szErrorBuffer ), 0);
		return "Unknown";
	}
}


static char funcParamsTxt[MAX_SYM_NAME * 5];

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  EnumerateLoadedModulesProc()
//
//  written by: Geoff Evans
//
//  Callback loads symbol data from loaded dll into DbgHelp system, dumping error info
//
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DebugSymbols::EnumerateLoadedModulesProc(PCSTR name, DWORD64 base, ULONG size, PVOID data)
{
	static char tmpBuffer[8192];

	if (SymLoadModule64(GetCurrentProcess(), 0, name, 0, base, size))
	{
		_snprintf(tmpBuffer, sizeof(tmpBuffer), "Debug loaded Symbols for module: %s (0x%08I64X, %u)\n", name, base, size);
		OutputDebugString(tmpBuffer);
	}else
	{
		_snprintf(tmpBuffer, sizeof(tmpBuffer), "Debug failed to load Symbols for module: %s (%i)\n", name, GetLastError());
		OutputDebugString(tmpBuffer);
	}

	return TRUE;
}



BOOL CALLBACK DebugSymbols::EnumerateSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
	DebugSymbols * ptr = (DebugSymbols*)UserContext;
	return ptr->enumSymbolsCallback(pSymInfo, SymbolSize, ptr);
}


BOOL DebugSymbols::enumSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
	DebugSymbols * ptr = (DebugSymbols*)UserContext;

	//stack_frame->AddrFrame.Offset;
	DWORD_PTR addrFrameOffset = ptr->getAddrFrameOffset();

	static char resultText[MAX_SYM_NAME + 16 + 2048];
	ZeroMemory(&resultText, sizeof(resultText));

	static char resultTextValue[16384];
	ZeroMemory(&resultTextValue, sizeof(resultTextValue));


	DWORD_PTR var_data = 0;
	if (pSymInfo->Flags & SYMFLAG_REGREL && addrFrameOffset > 0)
	{
		var_data = (DWORD_PTR)addrFrameOffset;
		var_data += (DWORD_PTR)pSymInfo->Address;
	}


	// Indicate if the variable is a local or parameter
	if ( pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER)
	{
		strcat(resultText, "-----Param: ");
	} else
	{
		return TRUE;

		/*
		if ( pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL )
		{
		strcat(resultText, "-----Local: ");
		}
		*/
	}

	if ( pSymInfo->Tag == SymTagFunction)
	{
		return TRUE;
	}


	resultText[0] = 0;
	
	int ptrCount = GetType(pSymInfo->ModBase, pSymInfo->TypeIndex, resultText, resultTextValue, 0, var_data);

	if (funcParamsTxt[0] != 0)
	{
		strcat (funcParamsTxt, ", ");
	}

	strcat (funcParamsTxt, resultText);
	for (int q = 0; q < ptrCount; q++)
	{
		strcat (funcParamsTxt, "*");
	}

	strcat (funcParamsTxt, " ");
	strcat (funcParamsTxt, pSymInfo->Name);
	strcat (funcParamsTxt, resultTextValue);


	DWORD dwMemberOffset;
	SymGetTypeInfo(process, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_OFFSET, &dwMemberOffset );
	DWORD_PTR dwFinalOffset = (DWORD_PTR)pSymInfo->Address + dwMemberOffset;

	

	return TRUE;
}

const char * DebugSymbols::GetFunctionParams(DWORD64 adr)
{
	// Use SymSetContext to get just the locals/params for this frame
	IMAGEHLP_STACK_FRAME imagehlpStackFrame;
	imagehlpStackFrame.InstructionOffset = adr;
	SymSetContext( process, &imagehlpStackFrame, 0 );

	// Enumerate the locals/parameters
	funcParamsTxt[0] = 0;
	SymEnumSymbols( process, 0, 0, EnumerateSymbolsCallback, this );

	return funcParamsTxt;
}

DWORD_PTR DebugSymbols::getAddrFrameOffset()
{
	//STACKFRAME64 stack_frame;

/*
AddrFrame

An ADDRESS64 structure that specifies the frame pointer. 
x86:  The frame pointer is EBP.
	  Intel IPF:  There is no frame pointer, but AddrBStore is used.
x64:  The frame pointer is RBP or RDI. This value is not always used.

*/

//stack_frame->AddrFrame.Offset;

	return m_stackFrameAddrFrameOffset;
}

const char * DebugSymbols::GetValue(BasicType basicType, ULONG64 length, DWORD_PTR pAddress, int ptrCount)
{
	static char tmpStrValue[2048];
	ZeroMemory(&tmpStrValue, sizeof(tmpStrValue));


	if (pAddress == NULL)
	{
		return (" = ?");
	}

	if (ptrCount > 0)
	{
		sprintf(tmpStrValue, " = 0x%08X", *(DWORD *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btFloat && length == 4)
	{
		sprintf(tmpStrValue, " = %f", *(float *)pAddress);
		return tmpStrValue;
	}


	if ( basicType == btFloat && length == 8)
	{
		sprintf(tmpStrValue, " = %f", *(double *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btInt && length == 1)
	{
		sprintf(tmpStrValue, " = %i", *(char *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btInt && length == 2)
	{
		sprintf(tmpStrValue, " = %i", *(short *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btInt && length == 4)
	{
		sprintf(tmpStrValue, " = %i", *(int *)pAddress);
		return tmpStrValue;
	}


	if ( basicType == btUInt && length == 1)
	{
		sprintf(tmpStrValue, " = %u", *(unsigned char *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btUInt && length == 2)
	{
		sprintf(tmpStrValue, " = %u", *(unsigned short *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btUInt && length == 4)
	{
		sprintf(tmpStrValue, " = %u", *(unsigned int *)pAddress);
		return tmpStrValue;
	}


	if ( basicType == btBool && length == 1)
	{
		sprintf(tmpStrValue, " = %d", *(bool *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btLong && length == sizeof(void*))
	{
		sprintf(tmpStrValue, " = %d", *(long int *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btULong && length == sizeof(void*))
	{
		sprintf(tmpStrValue, " = %u", *(unsigned long int *)pAddress);
		return tmpStrValue;
	}


	if ( basicType == btChar && length == 1)
	{
		sprintf(tmpStrValue, " = '%c'", *(char *)pAddress);
		return tmpStrValue;
	}

	if ( basicType == btWChar)
	{
		sprintf(tmpStrValue, " = '%d'", *(wchar_t *)pAddress);
		return tmpStrValue;
	}

	
	

	return (" = ?");
}


int DebugSymbols::GetType (ULONG64 ModBase, ULONG TypeIndex, char * result, char * resultValue, int ptrCount, DWORD_PTR var_data)
{
	static char tmpStr[MAX_SYM_NAME + 16 + 2048];
	ZeroMemory(&tmpStr, sizeof(tmpStr));

	SymTag _tag = SymTagNull;

	BOOL bSymRes = SymGetTypeInfo(process, ModBase, TypeIndex, TI_GET_SYMTAG, &_tag);
	if (!bSymRes)
	{
		strcat(result, "fail!!");
		return ptrCount;

	}
		//printf ("tag : '%s'\n", TagToString(_tag));

	switch (_tag)
	{
	case SymTagPointerType:
		{
			DWORD _TypeIndex = 0;
			SymGetTypeInfo( process, ModBase, TypeIndex, TI_GET_TYPEID, &_TypeIndex );

			//Надо рекурсивно запрашивать теперь опять getType, это может быть UDT, basic type и т.д.
			ptrCount = GetType(ModBase, _TypeIndex, result, resultValue, (ptrCount+1), var_data);

			/*
			SymTag __tag = SymTagNull;
			SymGetTypeInfo(curProcess, ModBase, _TypeIndex, TI_GET_SYMTAG, &__tag);

			BasicType __test = btNoType;
			SymGetTypeInfo( GetCurrentProcess(), pSymInfo->ModBase, TypeIndex, TI_GET_BASETYPE, &__test);


			WCHAR * szwTypeName = NULL ;
			SymGetTypeInfo ( GetCurrentProcess(), pSymInfo->ModBase, TypeIndex, TI_GET_SYMNAME, &szwTypeName);


			int a = 0;
			*/

			break;
		}
	case SymTagUDT:
		{
			WCHAR * szwTypeName = NULL ;
			SymGetTypeInfo ( process, ModBase, TypeIndex, TI_GET_SYMNAME, &szwTypeName);

			sprintf(tmpStr, "%ls", szwTypeName);
			strcat(result, tmpStr);

			if (var_data && resultValue && ptrCount > 0)
			{
				const char * resVal = GetValue(btNoType, 1, var_data, ptrCount);
				strcat(resultValue, resVal);
			}


			//printf ("User type : '%ls'\n", szwTypeName);
			LocalFree(szwTypeName);
			break;
		}
	case SymTagBaseType:
		{
			BasicType basicType = btNoType;
			SymGetTypeInfo(process, ModBase, TypeIndex, TI_GET_BASETYPE, &basicType);

			ULONG64 length;
			SymGetTypeInfo(process, ModBase, TypeIndex, TI_GET_LENGTH, &length);

			strcat(result, basicTypeToString(basicType, length));

			//printf ("Base type : '%s'\n", basicTypeToString(basicType));


			if (var_data > 0 && resultValue)
			{
				const char * resVal = GetValue(basicType, length, var_data, ptrCount);
				strcat(resultValue, resVal);
			}


			break;
		}
	case SymTagEnum:
		{
			WCHAR * szwTypeName = NULL ;
			SymGetTypeInfo ( process, ModBase, TypeIndex, TI_GET_SYMNAME, &szwTypeName);

			sprintf(tmpStr, "%ls", szwTypeName);
			strcat(result, tmpStr);

			//printf ("Enum type : '%ls'\n", szwTypeName);
			LocalFree(szwTypeName);

			break;
		}
	case SymTagFunctionType:
		{
			printf ("Func type\n");
			break;
		}
	case SymTagArrayType:
		{
			printf ("Array type\n");
			break;
		}
	}

	//sprintf(value_str, "%d", *(DWORD *)value_addr);


	return ptrCount;

}



const char * DebugSymbols::basicTypeToString (DebugSymbols::BasicType type, ULONG64 len)
{
	switch ( type )
	{
	case btNoType :
		return ("...");
	case btVoid :
		return("void");
	case btChar :
		{
			if (len == 1)
			{
				return("char");
			}

			return("_char");
		}
	case btWChar :
		{
			if (len == 1)
			{
				return("wchar_t");
			}

			return("_wchar_t");
		}
	case btInt :
		{
			if (len == 1)
			{
				return("signed char");
			}

			if (len == 2)
			{
				return("short");
			}

			if (len == 4)
			{
				return("int");
			}

			return("_int");
		}
	case btUInt :
		{
			if (len == 1)
			{
				return("unsigned char");
			}

			if (len == 2)
			{
				return("unsigned short");
			}

			if (len == 4)
			{
				return("unsigned int");
			}

			return("_uint");
		}
	case btFloat :
		{
			if (len == 4)
			{
				return("float");
			}

			if (len == 8)
			{
				return("double");
			}

			return("_float");
		}
	case btBCD :
		return("BCD") ;
	case btBool :
		{
			if (len == 1)
			{
				return("bool");
			}

			return("_bool");
		}
	case btLong :
		{
			if (len == sizeof(void*))
			{
				return("long int");
			}

			return("_long int");
		}
	case btULong :
		{
			if (len == 4)
			{
				return("unsigned long int");
			}

			return("_unsigned long int");
		}
	case btCurrency :
		return("CURRENCY") ;
	case btDate :
		return("DATE") ;
	case btVariant :
		return("VARIANT") ;
	case btComplex :
		return("COMPLEX" ) ;
	case btBit :
		return("bit" ) ;
	case btBSTR :
		return("BSTR" ) ;
	case btHresult :
		return("HRESULT" ) ;
	default :
		return("**UNKNOWN BasicType**" );
	}

	return("**UNKNOWN BasicType**" );
}



const char * DebugSymbols::TagToString (DebugSymbols::SymTag type)
{
	switch(type)
	{
	case SymTagNull:
		return "SymTagNull";
	case SymTagExe:
		return "SymTagExe";
	case SymTagCompiland:
		return "SymTagCompiland";
	case SymTagCompilandDetails:
		return "SymTagCompilandDetails";
	case SymTagCompilandEnv:
		return "SymTagCompilandEnv";
	case SymTagFunction:
		return "SymTagFunction";
	case SymTagBlock:
		return "SymTagBlock";
	case SymTagData:
		return "SymTagData";
	case SymTagAnnotation:
		return "SymTagAnnotation";
	case SymTagLabel:
		return "SymTagLabel";
	case SymTagPublicSymbol:
		return "SymTagPublicSymbol";
	case SymTagUDT:
		return "SymTagUDT";
	case SymTagEnum:
		return "SymTagEnum";
	case SymTagFunctionType:
		return "SymTagFunctionType";
	case SymTagPointerType:
		return "SymTagPointerType";
	case SymTagArrayType:
		return "SymTagArrayType";
	case SymTagBaseType:
		return "SymTagBaseType";
	case SymTagTypedef:
		return "SymTagTypedef";
	case SymTagBaseClass:
		return "SymTagBaseClass";
	case SymTagFriend:
		return "SymTagFriend";
	case SymTagFunctionArgType:
		return "SymTagFunctionArgType";
	case SymTagFuncDebugStart:
		return "SymTagFuncDebugStart";
	case SymTagFuncDebugEnd:
		return "SymTagFuncDebugEnd";
	case SymTagUsingNamespace:
		return "SymTagUsingNamespace";
	case SymTagVTableShape:
		return "SymTagVTableShape";
	case SymTagVTable:
		return "SymTagVTable";
	case SymTagCustom:
		return "SymTagCustom";
	case SymTagThunk:
		return "SymTagThunk";
	case SymTagCustomType:
		return "SymTagCustomType";
	case SymTagManagedType:
		return "SymTagManagedType";
	case SymTagDimension:
		return "SymTagDimension";
	}

	return "Undefined";
}

#undef _CRT_SECURE_NO_WARNINGS

#endif
#endif