#ifndef _PARTICLE_DATA_POSITION_H_
#define _PARTICLE_DATA_POSITION_H_


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "..\..\icommon\memfile.h"

#include "..\..\..\common_h\math3d.h"


class FieldList;
class TextFile;

#ifndef _XBOX
class TiXmlElement;
#endif

class DataPosition
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;


	Vector Value;

public:

//конструктор/деструктор
	DataPosition (FieldList* pMaster);
	virtual ~DataPosition ();

//Получить значение (Текущее время, Коэфицент рандома[0..1])
	const Vector& GetValue ();

//Установить значение
	void SetValue (const Vector& val);


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