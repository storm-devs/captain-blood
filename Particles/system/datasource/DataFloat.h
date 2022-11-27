#ifndef _PARTICLE_DATA_FLOAT_H_
#define _PARTICLE_DATA_FLOAT_H_


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

class DataFloat
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;


	float Value;

public:

//конструктор/деструктор
	DataFloat (FieldList* pMaster);
	virtual ~DataFloat ();

	//Получить значение
	float GetValue ();

	//Установить значение
	void SetValue (float val);


	void Load (MemFile* File);

	void Write (MemFile* File);

#ifndef _XBOX
	void LoadXML (TiXmlElement* root);

	void WriteXML (TextFile* xmlFile, dword level);
#endif


	void SetName (const char* szName, const char* szEditorName);

	const char* GetEditorName ();
	const char* GetName ();

};


#endif