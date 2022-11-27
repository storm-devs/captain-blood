/* устарело - теперь есть шейдер gmx_foam
#ifndef WATER_FOAM
#define WATER_FOAM


#include "..\..\..\Common_h\Mission.h"


class WaterFoam : public MissionObject
{
	ShaderId SeaTrack_id;

	struct FoamVertex
	{
		Vector pos;
		dword color;
		float u;
		float v;
	};

	struct FoamTrack
	{
		FoamVertex v1;
		FoamVertex v2;
	};

	array<FoamTrack> foam;

	float fScale;

	IVariable * trackTex1;
	IVariable * trackTex2;
	IVariable * trackTexBlend;


	float fRunSpeed;
	float fRunPosition;


	float fAnimSpeed;
	float frame;

	float alpha;

	bool bShowHelper;

	IBaseTexture* pFoamTextures[4];


	Matrix foamMatrix;

	Vector vMin;
	Vector vMax;


	bool noSwing;

	const char *masterName;

	MOSafePointer master;


public:

	WaterFoam();
	virtual ~WaterFoam();


	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	void _cdecl Work(float fDeltaTime, long level);

	void Activate(bool bIsActive);


	void PostCreate();


	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx)
	{
		mtx = foamMatrix;

		if( master.Ptr())
		{
			Matrix mat; master.Ptr()->GetMatrix(mat);

			mtx *= mat;
		}

		return mtx;
	};

	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max)
	{ 
		min = vMin;
		max = vMax;
	};

	virtual void EditMode_GetSelectBox(Vector & min, Vector & max)
	{
	  min = vMin;
		max = vMax;
	};



};



#endif

*/