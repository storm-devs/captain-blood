#ifndef _PARTICLE_DATA_UV_H_
#define _PARTICLE_DATA_UV_H_


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "..\..\icommon\memfile.h"

#include "..\..\..\common_h\d_types.h"
#include "..\..\..\common_h\math3d.h"

class FieldList;
class TextFile;

#ifndef _XBOX
class TiXmlElement;
#endif


class DataUV
{
	FieldList* Master;

  const char* szName;
	const char* szEditorName;

	array<Vector4> Frames;

public:

//конструктор/деструктор
	DataUV (FieldList* pMaster);
	virtual ~DataUV ();

	//Получить значение   [ x,y = UV1; z,w = UV2 ]
	const Vector4 &GetValue (DWORD FrameNum);

	//Установить значения
	void SetValues (const Vector4* _Frames, DWORD FramesCount);

	//Получить кол-во кадров
	DWORD GetFrameCount ();


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