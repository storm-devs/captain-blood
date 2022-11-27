//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Mission objects
//============================================================================================
// MissionGeometry
//============================================================================================

#ifndef _MissionGeometry_h_
#define _MissionGeometry_h_


#include "..\Mission.h"
#include "..\..\..\Common_h\AnimationStdEvents.h"


class MissionGeometry : public GeometryObject
{
	enum FadeDir
	{
		fd_none,
		fd_fadein,
		fd_fadeout,		
		fd_hide,
		fd_forcedword = 0x7fffffff
	};

	enum Consts
	{
		c_fadelevel = ML_PARTICLES5 + 1,
	};

public:
	
	struct PGroup
	{
		ConstString name;
		PhysicsCollisionGroup group;
	};



//--------------------------------------------------------------------------------------------
public:
	MissionGeometry();
	~MissionGeometry();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Пересоздать объект
	//virtual void Restart();
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max);
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Получить сцену
	virtual IGMXScene * GetScene();

protected:
	//Загрузить модельку
	bool LoadModel(IGMXScene * & m, const ConstString & fileName);
	//Удалить модульку
	void UnloadModel();
	//Остановить - продолжить проигрывание анимации
	void PauseAnimation(bool isPause);
	//Нарисовать модельку
	virtual void _cdecl Draw(float dltTime, long level);
	//Нарисовать модельку
	virtual void _cdecl DrawTransparency(float dltTime, long level);
	//Нарисовать модельку для тени
	void _cdecl ShadowInfo(const char * group, MissionObject * sender);
	//Нарисовать модельку для тени
	void _cdecl ShadowCast(const char * group, MissionObject * sender);
	//Нарисовать модельку для тени
	void _cdecl ShadowRecive(const char * group, MissionObject * sender);
	//Нарисовать отражёную модельку
	void _cdecl SeaReflection(const char * group, MissionObject * sender);
	//Нарисовать модельку в преломление
	void _cdecl SeaRefraction(const char * group, MissionObject * sender);
	//Сгенерировать матрицу отрисовки
	void BuildViewMatrix(Matrix & m);
	//Серёга добавил функции, я немного упорядочил код
	void CalculateGluedMatrix(const Matrix & matrixIn, Matrix & matrixOut);
	void FogParamsSave();
	void FogParamsRestore();


protected:
	IGMXScene * model;				//Модель
	IGMXScene * shadow;				//Модель для отбрасывания теней
	AnimationStdEvents * events;	//Обработчик событий
	MOSafePointer connectToPtr;		//К кому прицепить геометрию
	ConstString connectToName;		//К кому прицепить геометрию
	Matrix modelMatrix;				//Матрица текущего положения
	long level;						//Уровень отрисовки
	long fadelevel;					//Уровень отрисовки при фэйде
	float fadeAlpha;				//Прозрачность для фейда
	float alpha;					//Текущая прозрачность
	FadeDir fade;					//Направление фейда
	float fadeTime;					//Время перед началом фейда
	float lodDistanceMin2;			//Дистанция лодирования
	float lodDistanceK2;			//Дистанция лодирования
	bool isCheckLod;				//Проверять ли лод
	bool isPrevLodAlpha;			//Предыдущий кадр альфа была включена
	bool enableRender;				//Разрешить рендер
	bool dynamicLighting;			//Динамическое освещение
	bool shadowCast;				//Отбрасывает тень
	bool shadowReceive;				//Принимает тень
	bool seaReflection;				//Отражается в море	
	bool seaRefraction;				//Преломляется в море
	bool bUseRootBone;				//Возвращать манрицу мобифицированую рутовой костью анимации
	bool hideInEditor;				//Полностью прятать в редакторе если снят флажёк видимости

	bool bUseDistanceHack;

	bool bNotUseFog; //Не использовать туман на этой геометрии
	Color color;					//Применяемый цвет
	IPhysRigidBody * rigidBody;		//Физическая модель
	bool noSwing;					//Не качать модельку


	float h_density;
	float h_min;
	float h_max;
	float d_density;
	float d_min;
	float d_max;
	Color fog_color;

public:
	static const PGroup pgroups[];
	static const char * comment;
};

#endif

