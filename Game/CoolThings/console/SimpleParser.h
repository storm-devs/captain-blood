#ifndef SIMPLE_CONSOLE_PARSER
#define SIMPLE_CONSOLE_PARSER


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "..\..\..\Common_h\core.h"


#define MAX_STRING_SIZE 8192

class ConsoleParser
{

	array<long> Tokens;
	array<char> NamesString;

public:
 
	ConsoleParser () : Tokens(_FL_), NamesString(_FL_, MAX_STRING_SIZE) {};
	~ConsoleParser () {};
 
 

 __forceinline DWORD GetTokensCount ()
 {
	 return Tokens.Size();
 }

 __forceinline const char* GetTokenByIndex (DWORD Index)
 {
	 return &NamesString[Tokens[Index]];
 }
 
	__forceinline void Tokenize (const char* names)
	{
		Tokens.DelAll();
		NamesString.DelAll();
		while(*names)
		{
			if(!*names) break;
			//Пропускаем пробелы
			for(; *names == ' '; names++);
			//Начинаем токен
			if(!*names) break;
			Tokens.Add(NamesString);
			if(*names == '"')
			{
				*names++;
				if(!*names) break;
				//Токен в кавычках
				while(*names)
				{
					if(names[0] == '"')
					{
						if(names[1] == '"')
						{
							NamesString.Add('"');
							names += 2;
							continue;
						}
						names++;
						break;
					}
					NamesString.Add(*names++);
				}
				NamesString.Add(0);
			}else{
				//Токен до пробела
				while(*names && *names != ' ')
				{
					if(names[0] == '"' && names[1] == '"')
					{
						NamesString.Add('"');
						names += 2;
						continue;
					}
					NamesString.Add(*names++);
				}
				NamesString.Add(0);
			}
		}
		NamesString.Add(0);
		if(NamesString[Tokens[Tokens - 1]] == 0)
		{
			Tokens.DelIndex(Tokens - 1);
		}
	}
  
};
	

#endif