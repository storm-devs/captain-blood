

#ifndef _BonusesManagerImp_h_
#define _BonusesManagerImp_h_


#include "..\..\..\Common_h\bonuses.h"
#include "BonusesModelsManager.h"


class BonusesManagerImp;

class Greedy : public IGreedy
{
	Vector pos;
	MissionObject* owner;
	BonusesManagerImp* manager;

public:
	Greedy(BonusesManagerImp* pManager, MissionObject* pOwner);
	~Greedy();

	virtual void Release();

	virtual void SetPosition(const Vector& pos) {this->pos = pos;}
	virtual const Vector & GetPosition() {return pos;}
	virtual MissionObject* OwnerMO() {return owner;}

	Vector & Position() {return pos;}
	void ClearManager() {manager=null;}
};

//Менеджер бонусов
class BonusesManagerImp : public BonusesManager
{
	enum State
	{
		s_empty,
		s_immune,
		s_wait,
		s_fade,
		s_pickup_fly,
		s_pickup_start,
		s_pickup,
		s_pickup_fade,
	};

	struct BonusFx;

	struct BonusFxArchive
	{
		friend struct BonusFx;

		BonusFxArchive();

		bool IsDone();
		void ForceRelease();

	private:
		IParticleSystem * prticles;				//Партиклы для эффекта
		ISound3D * sound;						//Звук эффекта
	};

	struct BonusFx
	{		
		BonusFx();

		void Init(IMission & mis, const Matrix & mtx, const BonusBase::FX & fx);
		void Release();
		void Udpade(const Matrix & mtx);
		void Play(bool isOnes = true);
		bool IsActive();
		void PutToArchvie(BonusFxArchive & arc);

	private:
		IParticleSystem * prticles;				//Партиклы для эффекта
		ISound3D * sound;						//Звук эффекта
		bool isPlay;							//Запущен эффект ли
	};

	struct Bonus
	{
		Vector pos;								//Позиция бонуса
		Vector velocity;						//Скорость и направление полёта
		float pickupRadius;						//Радиус взятия бонуса
		float radius;							//Радиус бонуса
		State state;							//Текущее состояние бонуса
		float ay;								//Текущий угол поворота
		float phase;							//Текущая фаза высоты
		BonusesModelsManager::Index model;		//Модель бонуса
//		BonusFx drop;							//Эффект выподания
		BonusFx wait;							//Эффект сопровождающий бонус при ожидании
		BonusFx jump;							//Эффект сопровождающий бонус при отскоках
		BonusFx pickupfly;						//Эффект начала поднятия бонуса
		BonusFx pickup;							//Эффект поднятия бонуса
		BonusFx fade;							//Эффект исчезновения при таймауте
		MOSafePointerType<BonusBase> object;	//Объект описания бонуса
		float liveTime;							//Текущее время жизни бонуса
		float maxLiveTime;						//Общее время жизни бонуса
		float flyTime;							//Отросительное время полйта из точки velocity до точки playerPos
		float dropAlhpa;						//Альфа блокирующая мигание в самом начале дропа
		float alpha;							//Альфа для рендера
		long applyCount;						//Количество применений
		float applyTime;						//Время, прошедшее с последнего применения
		float applyDltTime;						//Промежуток времени между Apply
		float workTime;							//Время работы бонуса
		float dltTime;							//Остаток dltTime для физики
		bool isSleep;							//Перемещается бонус или остановился
		BonusBase::UserBonus* userBonus;		//Примочка для управления бонусом
		MissionObject* pickuper;				//Объект который забрал бонус
		long pickuperCacheId;					//ИД объекта который забрал бонус (для проверок на валидность)
		IGreedy* greedy;						//Объект пожиратель бонуса
		float friction;							//затухание скорости движения
	};

	struct LightDesc
	{
		Vector localPos;		//Позиция источника с системе камеры
		Color filter;			//lightColor = (directionColor + safe).Normalize()*filter + base;
		Color safe;
		Color base;
		float radius;			//Радиус источника
		ILight * light;			//Источник, созданный для миссии
	};

	class PickuperList
	{
		MGIterator* gi;
		struct Pickuper
		{
			MissionObject* obj;
			Matrix transform;
			bool bDeleteFlag;
		};
		array<Pickuper> list;

	public:
		PickuperList() : list(_FL_) {gi=null;}
		~PickuperList() {RELEASE(gi);}

		// обновить список подборщиков (вернуть истину если кто нить есть)
		bool UpdateList(IMission& mis);
		// выбрать ближайшего из тех кто может подобрать бонус
		MissionObject* FindNearestByBonus(Bonus* b);
	private:
		void UpdatePickuper(MissionObject* mo);
	};

	struct GlowVertex
	{
		Vector pos;
		dword col;
		float u,v;
	};

public:
	BonusesManagerImp();
	virtual ~BonusesManagerImp();


public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Создание бонуса
	virtual void CreateBonusInside(const Vector & pos, const ConstString & tableName, DropParams * crParams);
	//Удалить все бонусы
	virtual void ResetAllBonusesInside();
	//Кэшировать модельку
	virtual void CacheModelInside(const char * modelName);
	//Создать пожирателя
	virtual IGreedy* CreateGreedyInside(MissionObject* pOwnerMO);
	//Убрать пожирателя из списка
	void RemoveGreedy(IGreedy* pGreedy);
	//Создать бонус
	void CreateBonusInside(const Vector & pos, BonusBase * bonus, float dltAngle, DropParams * crParams);
	//Работа мэнеджера
	void _cdecl Work(float dltTime, long level);
	//Нарисовать модельки для тени
	void _cdecl ShadowDraw(const char * group, MissionObject * sender);
	//Нарисовать бонус
	bool WaitBonus(Bonus & bonus, float dltTime, bool & isLive);
	//Переместить бонус
	void MoveBonus(Bonus & bonus, float dltTime);
	//Нарисовать бонус
	bool DrawBonus(const BonusBase::CreationParams & params, Bonus & bonus, float dltTime);
	//Применить бонус
	bool ApplyBonus(const BonusBase::CreationParams & params, Bonus & bonus, float dltTime, bool isAlive);
	//Удалить бонус
	void Release(Bonus & bonus);
	//Удалить все бонусы
	void ReleaseAll();

	//Отметить бонус на растворение
	void MarkForFadeout();
	//Перенести эффект в архив
	void PutEffectToArchive(BonusFx & fx);

public:
	array<Bonus> bonuses;			//Набор бонусов
	array<BonusBase *> drop;		//Набор брошеных бонусов
	float dropAngle;				//Текущее направление броска
	BonusesModelsManager models;	//Созданные модели
	array<ILight *> createdLights;	//Созданные источники света
	Color litAmbientColor;			//Цвет амбиентного освещения
	Color litDirectionColor;		//Цвет направленого источника света
	Color litDirectionBColor;		//Цвет направленого источника света обратной стороны
	Vector litDirectionInCamSpace;	//Направленние освещение глобального света в системе камеры
	//LightDesc lights[2];			//Источники света для бонусов
	bool needExtendedDraw;			//Есть ли пользовательские рисовалки
	array<BonusFxArchive> activeFx;	//Оставшиеся с бонусов активные эффекты
	PickuperList pickupers;			//Список подборщиков бонусов

	array<Greedy*> aGreedy;			//

	// найти ближайшего пожирателя бонуса
	IGreedy* FindNearestGreedy(Bonus* b);

	//void TempGreedyFrame();

protected:
	void DrawGlow(const BonusBase::CreationParams & params, Bonus & bonus, float dltTime);
	void FlushGlow();

private:
	static const long nMaxGlowVertexQnt = 255;
	static Vector m_camPos;
	ShaderId m_idGlowShader;
	long m_nGlowQnt;
	IVariable* m_pGlowTextureVar;
	IBaseTexture* m_pGlowTexture;
	const char* m_pcGlowTextureName;
	long m_nGlowHorz;
	float m_fGlowTdU;
	float m_fGlowTdV;
	GlowVertex m_pGlowVertex[nMaxGlowVertexQnt*6];
};



#endif
