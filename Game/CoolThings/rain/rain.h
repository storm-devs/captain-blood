#ifndef RAIN_WEATHER
#define RAIN_WEATHER

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\Common_h\QSort.h"



struct rainDropSFX
{
	union
	{
		struct
		{
			//Выравниваем
			dword __align[8];
		};

		struct
		{
			//Надо или нет рождать в этой точке капли
			dword dwEnabled;

			//Время сколько не рождалась капля
			float fTime;

			//Тип капли
			dword dwType;

			//Позиция для рождения
			Vector vPos;
		};
	};

	//Assign
	mathinline rainDropSFX & mathcall operator=(const rainDropSFX& rd)
	{
		dwEnabled = rd.dwEnabled;
		fTime     = rd.fTime;
		dwType    = rd.dwType;
		vPos      = rd.vPos;
		return *this;
	}
};


class RainSFXManager : public MissionObject
{
#ifndef _XBOX
	void RecursiveDeleteFolder(const char* szPath);
#endif

	public:

		RainSFXManager();
		~RainSFXManager();

		
		virtual bool Create(MOPReader & reader);
		virtual bool EditMode_Create(MOPReader & reader);

#ifndef _XBOX
		void BuildPack();

		virtual bool EditMode_Export()
		{ 
			BuildPack();
			return true;
		};
#endif

};





class RainSFX : public MissionObject
{
	ILoadBuffer * loadedData;

	long needUpdateCache;

	const byte* inplaceDropsBuffer;
	dword * inplaceDropsDataCount;
	rainDropSFX * inplaceDropsData;


	dword GetInplaceBufferSize (dword dwNumDrops);
	void CreateInplaceBuffer (dword dwNumDrops);
	void GenerateInplaceBuffer ();


	void RenderInplaceBuffer(float fDeltaTime);


	dword dropsCount;

	bool bShow;
	bool bActive;

	bool bDebugInfo;

	float fSpeed;


	struct RECT_VERTEX
	{
		Vector vGlobalPos;
		short	tu1;
		short	tv1;
	};


	struct BillBoard
	{
		Vector vPos;
		float fSizeX;
		float fSizeY;
	};

	//GMXBoundSphere SphereBound;

	static IVBuffer* pVBufferStatic;
	static IIBuffer* pIBufferStatic;

	IRender* pRS;
	IVariable * RainColor;
	IVariable * RainTexture;
	IVariable * RainCameraPos;


	array<BillBoard> Billboards;

	Vector4 vColor;

	bool bAllRainVisible;
	long splashesCount;
	ConstString groundParticlesSystem;
	ConstString waterParticlesSystem;
	float fMinSplashInterval;
	float fMaxSplashInterval;


	Matrix mWorld;
	Vector vSize;

	float dropX_mul;
	float dropY_mul;

	bool bIgnoreMaterials;

	ShaderId RainWeather_id;


	IBaseTexture* pTextureFrame1;
	string sTextureName1;

	void ChangeStatus ();
	void CreateBuffers ();
	void _cdecl Realize(float fDeltaTime, long level);
	void LoadTexture(const char* szTex1);


	void GenerateRandom();
	void Update(float fDeltaTime);


	

public:

	//Конструктор - деструктор
	RainSFX();
	virtual ~RainSFX();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);


	virtual void Show(bool isShow);

	virtual void Activate(bool isActive);



	MO_IS_FUNCTION(RainSFX, MissionObject);

	void EditMode_GetSelectBox(Vector & min, Vector & max);
	void GetBox(Vector & min, Vector & max);
	Matrix & GetMatrix(Matrix & mtx);

	bool SaveToFile();
	bool LoadFromFile();



};

#endif