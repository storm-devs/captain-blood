#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\AnimationStdEvents.h"

#include "..\..\..\Common_h\ITips.h"
#include "..\..\..\Common_h\Bonuses.h"

class ActiveObject : public DamageReceiver
{
	struct CollisionBox
	{
		Matrix mtx;
		Vector size05;

		IPhysBox *box;
	};

	struct Effect
	{
		struct Sound
		{
			ConstString name; Vector p;
		}
		snd;

		struct Particles
		{
			const char *name; Matrix m;
		}
		sfx;

		void Init(MOPReader &reader)
		{
			snd.name = reader.String();
			snd.p	 = reader.Position();

			sfx.name = reader.String().c_str();

			Vector pos = reader.Position();
			Vector ang = reader.Angles();

			sfx.m.Build(ang,pos);
		}
	};

	class AnimationEvents : public AnimationStdEvents
	{
		IMission *mis;

	public:

		virtual void AddHandlers(IAnimation *anx)
		{
			if( !anx )
				return;

			AnimationStdEvents::AddHandlers(anx);

			anx->SetEventHandler(this,(AniEvent)&AnimationEvents::MissionCommand,"MOCmd");
		}

	public:

		void _cdecl MissionCommand(IAnimation *ani, const char *name, const char **params, dword numParams)
		{
			MOSafePointer sp;

			if( mis->FindObject(ConstString(params[0]),sp))
			{	
				sp.Ptr()->Command(params[1],numParams - 2,params + 2);
			}
		}

	public:

		void SetMission(IMission *mis)
		{
			this->mis = mis;
		}

	};

	struct FogParams
	{
		float h_density;
		float h_min;
		float h_max;

		float d_density;
		float d_min;
		float d_max;

		Color color;
	};

public:

	 ActiveObject();
	~ActiveObject();

public:

	virtual bool Create(MOPReader &reader);

	virtual void PostCreate();

	virtual bool EditMode_Create(MOPReader &reader);
	virtual bool EditMode_Update(MOPReader &reader);

	void Reset();

	void Activate(bool isActive);
	void Show	 (bool isShow);

	void Restart();

	Matrix &GetMatrix(Matrix &m);

	void Command(const char *id, dword numParams, const char **params);

	MO_IS_FUNCTION(ActiveObject, DamageReceiver);

private:

	void InitParams(MOPReader &reader);
	void InitBoxes (MOPReader &reader, const Matrix &m, array<CollisionBox> &boxes);

	void _cdecl Draw(float dltTime, long level);

	void ApplyDamage(float hp);

	void EnableLiveColls(bool en)
	{
		for( int i = 0 ; i < liveBoxes ; i++ )
		{
			liveBoxes[i].box->EnableCollision(en);
			liveBoxes[i].box->Activate(en);
		}
	}

	void EnableDeadColls(bool en)
	{
		for( int i = 0 ; i < deadBoxes ; i++ )
		{
			deadBoxes[i].box->EnableCollision(en);
			deadBoxes[i].box->Activate(en);
		}
	}

	void DeleteColls()
	{
		for( int i = 0 ; i < liveBoxes ; i++ )
			liveBoxes[i].box->Release();

		for( int i = 0 ; i < deadBoxes ; i++ )
			deadBoxes[i].box->Release();
	}

	void ActivateColliders(bool en);

	void UpdateColliders();

private:

	void ResetUpdate(bool alpha)
	{
		if( drawLevel < ML_ALPHA1 )
		{
			DelUpdate(&ActiveObject::Draw);
			SetUpdate(&ActiveObject::Draw,alpha ? drawLevel + (ML_ALPHA1 - ML_GEOMETRY1) : drawLevel);
		}
	}

	void FogParamsSave()
	{
		if( skipFog )
		{
			Render().getFogParams(fog.h_density,fog.h_min,fog.h_max,fog.d_density,fog.d_min,fog.d_max,fog.color);
			Render().setFogParams(		   0.0f,fog.h_min,fog.h_max,		 0.0f,fog.d_min,fog.d_max,fog.color);
		}
	}

	void FogParamsRestore()
	{
		if( skipFog )
		{
			Render().setFogParams(fog.h_density,fog.h_min,fog.h_max,fog.d_density,fog.d_min,fog.d_max,fog.color);
		}
	}

	void DrawScene();

private:

	void _cdecl ShadowCast(const char *group, MissionObject *sender) { DrawScene(); }
	void _cdecl ShadowInfo(const char *group, MissionObject *sender);

	void _cdecl ShadowReceive(const char *group, MissionObject *sender)
	{
		if(!EditMode_IsVisible()||!IsShow()) return;

		DrawScene();
	}

	void _cdecl SeaReflection(const char *group, MissionObject *sender) { DrawScene(); }
	void _cdecl SeaRefraction(const char *group, MissionObject *sender) { DrawScene(); }

public:

	virtual bool Attack(MissionObject *obj, dword src, float hp, const Vector &center, float rad);
	virtual bool Attack(MissionObject *obj, dword src, float hp, const Vector &from, const Vector &to);
	virtual bool Attack(MissionObject *obj, dword src, float hp, const Vector vert[4]);

private:

	AnimationEvents events;

	string modelName;
	string brokeName;

	const char *nodeOnHit;
	const char *nodeBroke;

	float onHitBlend;
	float brokeBlend;

	Matrix matrix;

	array<CollisionBox> liveBoxes;
	array<CollisionBox> deadBoxes;

	IGMXScene  *model;
	IAnimation *anime;

	IGMXScene  *broke;
	IAnimation *brokeAnime;

	IGMXScene  *scene;	// текущая моделька для отрисовки

	Effect effOnHit;
	Effect effBroke;

	Effect effLeast;

	float time;
	float timeMax;

	IMissionQTObject *collider;

	float health;
	float healthMax;

	Color color;

	ConstString bonusTable;

	ITip *tip;
	Vector tipPos;

	bool debugDraw;
	bool debugDrawBroke;

	ConstString masterName;

	MOSafePointer master;

	IParticleSystem *sfxLeast;
	ISound3D *sndLeast;

	float fadeAlpha;

	bool prevAlpha;		// на прошлом кадре были прозрачными

	//// Render params ////

	FogParams fog;

	bool skipFog;		// не учитывать туман при отрисовке модели

	long  drawLevel;
	Color drawColor;

	bool enableRender;	// разрешить рендер

	bool shadowCast;
	bool shadowReceive;

	bool seaReflection;
	bool seaRefraction;

	bool hideInEditor;	// полностью прятать в редакторе при невидимости

	bool noSwing;

	float distanceMin2;	// дистанция   лодирования
	float distanceK2;	// коэффициент лодирования

	bool checkDistance;

	///////////////////////

	bool m_active;
	bool m_show;

};
