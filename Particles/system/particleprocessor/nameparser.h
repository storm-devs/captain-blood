#ifndef SIMPLE_GEOM_NAME_PARSER
#define SIMPLE_GEOM_NAME_PARSER


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\templates.h"


#define MAX_STRING_SIZE 8192

class GeomNameParser
{

	array<char*> Tokens;
	char NamesString[MAX_STRING_SIZE];

public:
 
	GeomNameParser () : Tokens(_FL_) {};
	~GeomNameParser () {};
 
 
 __forceinline const char* GetRandomName (const char* names)
 {
	 Tokenize (names);
	 if (Tokens.Size() == 0) return NULL;

	 return Tokens[(rand () % Tokens.Size())];
 }


 __forceinline DWORD GetTokensCount ()
 {
	 return Tokens.Size();
 }

 __forceinline const char* GetTokenByIndex (DWORD Index)
 {
	 return Tokens[Index];
 }
 
 __forceinline void Tokenize (const char* names)
 {
	Tokens.DelAll();
	crt_strncpy (NamesString, MAX_STRING_SIZE, names, MAX_STRING_SIZE-1);
  int StrLen = strlen (NamesString);
  if (NamesString[StrLen-1] != ';') crt_strcat (NamesString, MAX_STRING_SIZE, ";");
  StrLen = strlen (NamesString);

  int FromChar = 0;
  for (int n = 0; n < StrLen; n++)
  {
    if (NamesString[n] == ';')
    {
      NamesString[n] = 0;

      char* ModelName = NamesString + FromChar;
      if (strlen (ModelName) > 0)
      {
				Tokens.Add(ModelName);
      }
      FromChar = n+1;
    }
  }
 }
  
};
	

#endif