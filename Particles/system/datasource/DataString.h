#ifndef _PARTICLE_DATA_STRING_H_
#define _PARTICLE_DATA_STRING_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "..\..\icommon\memfile.h"



class FieldList;
class TextFile;

#ifndef _XBOX
class TiXmlElement;
#endif

class DataString
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;

	string Value;

public:

//конструктор/деструктор
	DataString (FieldList* pMaster);
	virtual ~DataString ();

//Получить значение
	const char* GetValue ();

//Установить значение
	void SetValue (const char* val);


	void Load (MemFile* File);


#ifndef _XBOX
	void LoadXML (TiXmlElement* root);

	void WriteXML (TextFile* xmlFile, dword level);
#endif

	void Write (MemFile* File);



	void SetName (const char* szName, const char* szEditorName);

	const char* GetEditorName ();
	const char* GetName ();

};


#endif