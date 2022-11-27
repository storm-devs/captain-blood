#ifndef SUN_GLOW_OBJECT
#define SUN_GLOW_OBJECT

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\common_h\QSort.h"



class SunGlow : public MissionObject
{
	bool bShow;
	bool bActive;


	struct RECT_VERTEX
	{
		Vector	vRelativePos;
		dword	dwColor;
		float	tu1, tv1;
		float angle;
		Vector	vParticlePos;
		float fDisolve;
	};


	struct BillBoard
	{
		Vector vPos;
		float fSizeX;
		float fSizeY;

		float fSpeed;
	};

	//GMXBoundSphere SphereBound;

	IVBuffer* pVBuffer;
	IIBuffer* pIBuffer;

	IRender* pRS;
	//IVariable * vBBCameraPos;
	IVariable * SunTexture;


	array<BillBoard> Billboards;

	float fArea;
	dword dwColor;

	IBaseTexture* pTextureFrame1;
	string sTextureName1;

	void ChangeStatus ();
	void CreateBuffers ();
	void _cdecl Realize(float fDeltaTime, long level);
	void LoadTexture(const char* szTex1);



	Color glowColor;

	float powScale;
	float fSize;
	Vector vPosition;

	float m_newPowDelta;
	float m_newPowTime;

	ShaderId techId;

public:

	//Конструктор - деструктор
	SunGlow();
	virtual ~SunGlow();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);


	void Activate(bool isActive);

	virtual void Command(const char * id, dword numParams, const char ** params);

	MO_IS_FUNCTION(SunGlow, MissionObject);

};

#endif