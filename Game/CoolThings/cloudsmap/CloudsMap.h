#ifndef CLOUDS_MAP_MISSION_OBJECT
#define CLOUDS_MAP_MISSION_OBJECT

#include "..\..\..\Common_h\Mission.h"


class CloudsMap : public MissionObject
{	
	struct PreviewVertex
	{
		Vector vPos;
		float u;
		float v;
	};

	PreviewVertex Quad[4];

	IVariable * CloudsMapParams;
	IVariable * CloudsMapTexture;
	IVariable * CloudsMapOffsetX;
	IVariable * CloudsMapOffsetZ;

	IVariable * CloudsColor;
	

	bool bVis;
	bool bActive;


	Vector4 curOffset;

	IBaseTexture* pTex;
	float Width;
	float Depth;
	Vector vPos;

	float fSpeedX;
	float fSpeedZ;

	Color cloudShadowColor;

	void SetupVariables ();


	float multiplerX;
	float multiplerZ;

public:

	//Конструктор - деструктор
	CloudsMap();
	virtual ~CloudsMap();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);


	MO_IS_FUNCTION(LightingMap, MissionObject);

	void _cdecl Realize(float fDeltaTime, long level);
	void EditMode_GetSelectBox(Vector & min, Vector & max);

	void Activate(bool isActive);


};

#endif