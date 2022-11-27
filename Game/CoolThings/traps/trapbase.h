#ifndef TRAPBASE_H
#define TRAPBASE_H

#include "..\..\..\Common_h\Mission.h"


class TrapBase;

class TrapPatternBase : public MissionObject
{
public:
	struct CharReactionData
	{
		const char* pcHitReaction;
		const char* pcBlockReaction;
		const char* pcDieReaction;
	};
	struct GeoRenderParams
	{
		Color color;
		bool dynamicLighting;
		bool shadowCast;
		bool shadowReceive;
		bool seaReflection;
	};
	struct BaseParams
	{
		bool isLocalParticles;
	};

	TrapPatternBase();

	void ReadCommonData(MOPReader & reader);
	const CharReactionData& GetReactions() {return m_reactions;}
	const GeoRenderParams& GetRenderParams() {return m_renderparams;}
	const BaseParams& GetBaseParams() {return m_baseparams;}
	void UpdateTraps();
	void RegistryTrap(TrapBase* pTrap);
	void UnregistryTrap(TrapBase* pTrap);

protected:
	CharReactionData m_reactions;
	GeoRenderParams m_renderparams;
	BaseParams m_baseparams;

	array<TrapBase*> m_aTraps;
};

#define MOP_REACTIONDATA \
	MOP_GROUPBEG("Hit reactions") \
		MOP_STRINGC("HitReaction", "", "Reaction node by hit character") \
		MOP_STRINGC("BlockReaction", "", "Reaction node by hit character while his blocked") \
		MOP_STRINGC("DieReaction", "", "Reaction node for die character from trap") \
	MOP_GROUPEND()
#define MOP_RENDERGEOMETRYPARAMS \
	MOP_GROUPBEG("Render params") \
		MOP_COLOR("Color", Color(0.0f, 0.0f, 0.0f, 1.0f)) \
		MOP_BOOL("Dynamic lighting", false) \
		MOP_BOOLC("Shadow cast", false, "Geometry can is shadow cast by some objects") \
		MOP_BOOLC("Shadow receive", false, "Geometry can is shadow receive from casting objects") \
		MOP_BOOLC("Sea reflection", false, "Geometry can reflect in sea") \
	MOP_GROUPEND()
#define MOP_BASEPARAMS \
	MOP_GROUPBEG("Base params") \
		MOP_BOOLC("Local particles", true, "Particles systems moved with geometry (if false then move only particles emitter)") \
	MOP_GROUPEND()
#define MOP_TRAPCOMMONPARAMS MOP_REACTIONDATA MOP_RENDERGEOMETRYPARAMS //MOP_BASEPARAMS



class TrapBase : public MissionObject
{
public:
	TrapBase();
	virtual ~TrapBase();

	virtual void Show(bool isShow);

	MO_IS_FUNCTION(TrapBase, MissionObject);

	void _cdecl Work(float fDeltaTime, long level);
	void _cdecl EditorWork(float fDeltaTime, long level);

	//Нарисовать модельку для тени
	void _cdecl ShadowCast(const char * group, MissionObject * sender);
	//Нарисовать модельку для тени
	void _cdecl ShadowDraw(const char * group, MissionObject * sender);

	// обработка на кадре
	virtual void Frame(float fDeltaTime) {}
	// отрисовка инфы для редактора
	virtual void EditorDraw() {}
	// обновление паттерна
	virtual void UpdatePattern() {}
	// реализация дамага от конкретной ловушки
	virtual void InflictDamage(DamageReceiver* pObj, TrapPatternBase::CharReactionData& reactions) {}
	// добавление объектов которым не наносится дамаг
	void AddExcludeObj(DamageReceiver* pObj,float fTime);
	// удаление объектов которым не наносится дамаг
	void DelExcludeObj(DamageReceiver* pObj);
	// очистить список исключений
	void ClearExcludeList() {m_aExcludeList.DelAll();}
	// установить звук
	void SetSound(ISound3D* pSound);
	// установить партикл
	void SetParticle(dword n, IParticleSystem* pParticle, const Vector& pos, bool bLocal);
	// двигать партиклы
	void TransformParticles(const Matrix & mtx);
	// двигать партикл локально
	void MoveParticle(dword n, const Vector & pos);
	// зачитать стандартные параметры
	void SetStandartParams(TrapPatternBase* pPattern);

	//--------------------------------------------------------------
	// set/get parameters
	//--------------------------------------------------------------
	void SetModel(IGMXScene* pModel);

	void SetTransform(const Matrix& mtx) {m_mtxTransform = mtx;}
	const Matrix& GetTransform() {return m_mtxTransform;}

	void SetDamageBox(const Vector& vs, const Vector& vc) {m_vDamageBoxSize = vs; m_vDamageBoxCenter= vc;}
	void SetDamageActive(bool bActive);

private:
	struct ExcludeObject
	{
		DamageReceiver* obj;
		float time;
	};
	struct DelParticle
	{
		IParticleSystem* particle;
		float time;
	};
	struct ParticleElem
	{
		IParticleSystem* p;
		Vector pos;
		bool islocal;

		ParticleElem() {p=NULL;}
		~ParticleElem() {RELEASE(p);}
	};

	static const int maxparticles = 4;

	IGMXScene* m_pModel;
	Matrix m_mtxTransform;
	Vector m_vSpeed;
	TrapPatternBase::CharReactionData m_reactions;
	TrapPatternBase::GeoRenderParams m_renderparams;
	TrapPatternBase::BaseParams m_baseparams;
	Vector m_vDamageBoxSize;
	Vector m_vDamageBoxCenter;
	bool m_bDamageActive;
	array<ExcludeObject> m_aExcludeList;
	array<DelParticle> m_aDelParticles;
	ParticleElem m_pParticle[maxparticles];
	ISound3D* m_pSound;

	void SetParticleToDeleteList(IParticleSystem* pParticle);
	void ReleaseParticles();
	bool IsExclude(DamageReceiver* pObj);
};

#endif
