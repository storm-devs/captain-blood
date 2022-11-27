#ifndef SKY_HPP
#define SKY_HPP

#include "..\..\..\Common_h\Mission.h"

class Sky : public MissionObject
{
	IGMXService::RenderModeShaderId SkyFog_id;
	IGMXService::RenderModeShaderId SkyFog2_id;

public:
	Sky();
	virtual ~Sky();

	MO_IS_FUNCTION(Sky, MissionObject);

	bool Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	// Edit mode object create / update
	virtual bool EditMode_Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max);

	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);


	void _cdecl		Realize(float fDeltaTime, long level);
	void _cdecl		SeaReflection();
	// Используется при генерации sphere map'a в Ocean'e, сбрасывает угол текущего поворота в 0.0f, 
	// в основном фикс для редактора миссий
	void _cdecl		SphereMapGen(GroupId,long);

//	void _cdecl Draw(const char *group, MissionObject *sender);
	virtual void Command(const char *id, dword numParams, const char **params);

private:

	ICoreStorageLong * oceanMode;

	bool bNoSwing;
	bool bReflection;
	bool bReflDublicate;
	bool m_postDraw;
	dword dwSkyColor;
	float fSkySpeedRotate, fAngleY;
	Color	cColor;

	// вторая геометрия
	IGMXScene	* pSkyGeo2;
	Vector		vAngle2;
	float		fSkySpeedRotate2, fAngleY2;
	Color		cColor2;

	IVariable	* pFogColor;
	IVariable	* pFogCoeff;

	//string sModel;
	IGMXScene * pSkyGeo;
	Vector vScale, vPosition, vAngle;
	float startAngleY;
	IVariable * pVarSkyRotate;

	IVariable * varSkyColor;
	//bool bShowBackGeometry;

	/*long		iFogNumVerts;
	long		iFogNumTrgs;

	IVBuffer	* pFogVB;
	IIBuffer	* pFogIB;*/

	Color colFog;
	float fFogHeight;
	float fFogMultiply;

	//С кого считывать углы поворота по Y
	MOSafePointer connectToPtr;
	ConstString connectToName;

	// sky section
	void	Release();
	/*void	CreateFogSphere(float fFogHeight);
	float	CalcFogAlpha(const Vector & vPos, float fFogHeight);
	void	DrawFogSphere(const Matrix & mWorld);*/
};

#endif