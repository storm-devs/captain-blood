#ifndef _PARTICLE_DATA_BOOLEAN_H_
#define _PARTICLE_DATA_BOOLEAN_H_

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

class DataBool
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;
  

	bool Value;

public:

//конструктор/деструктор
	DataBool (FieldList* pMaster);
	virtual ~DataBool ();

//Получить значение
	bool GetValue ();

//Установить значение
	void SetValue (bool val);


//Сохранить/загрузить...
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