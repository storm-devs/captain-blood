#ifndef Taran_H
#define Taran_H

#include "QuickEvent.h"
#include "DoorBlender.h"
#include "..\..\..\common_h\AnimationStdEvents.h"
#include "..\..\character\components\AnimationsBlender.h"

class Taran : public QuickEvent
{
public:
	
	DoorBlendStage* doorBlendStage;	

	Vector orientpoint;	
	const char* parralel_graph;
	
	IAnimation* break_anim;
	AnimationsBlender* anim_blender;

	IGMXScene*  door_model;
	IAnimation* door_anim;
	Matrix door_transform;
	
	float blend_time;
	bool  pos_dir;
	float accum_time;

	long  bone_index;
	int   started;	

	static ConstString chainSndName;
	Vector chainSndPosition;

	bool debugTaran;

	ConstString widget_left;
	ConstString widget_right;

	int				lastKick;
	float			kickValue, kickDamage;

	int				life;
	int				max_life, num_damages;
	ConstString		progressBar;
	MOSafePointer	bar;	

	MissionTrigger triger_Kick;

	Taran();
	virtual ~Taran();

	virtual void InitData();
	virtual bool EditMode_Update(MOPReader & reader);	

	virtual void Release();

	virtual void _cdecl Draw(float dltTime, long level);

	//Нарисовать модельку для тени
	virtual void _cdecl ShadowInfo(const char * group, MissionObject * sender);
	//Нарисовать модельку для тени
	void _cdecl ShadowCastModel(const char * group, MissionObject * sender);
	void _cdecl ShadowCastDoor(const char * group, MissionObject * sender);
	//Нарисовать модельку для тени
	virtual void _cdecl ShadowDraw(const char * group, MissionObject * sender);

	virtual void GetBox(Vector & min, Vector & max);	

	virtual bool InteruptOnHit() { return true; };
	virtual bool AllowInterruptByButton();

	virtual void _cdecl TikTac(float dltTime, long level);

	virtual void BeginQuickEvent();	
	virtual void QuickEventUpdate(float dltTime);	
	virtual void Interupt(bool win_game);
	virtual void AnimEvent(const char * param);	
	
	virtual void HideHUD();

	virtual const char* GetName() { return "Taran";};

	virtual void Command(const char * id, dword numParams, const char ** params);

	static const char * comment;

	bool penalty;

	inline virtual void  ActivateLink(const char* link)
	{
		if (break_anim) break_anim->ActivateLink(link,true);
		if (anim) anim->ActivateLink(link,true);
	};

	inline virtual void  Goto(const char* link)
	{
		if (break_anim) break_anim->Goto(link,0.0f);
		if (anim) anim->Goto(link,0.0f);
	};

	void BreakAnimEvent(const char * name, const char ** params, dword numParams);

private:

#ifndef STOP_DEBUG
	float GetFPS(IAnimation * _anim);
#endif

	class TaranEvents : public AnimationStdEvents
	{
	private:
		Taran * taran;
		IAnimation * anx;

		void _cdecl TaranEvent(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			taran->BreakAnimEvent(name, params, numParams);
		}

	public:
		TaranEvents()
		{
			anx = null;
			taran = null;
		}

		virtual ~TaranEvents()
		{
		}

		void Release()
		{
			if (anx)
			{
				DelHandlers(anx);
				anx->DelEventHandler(this, (AniEvent)&TaranEvents::TaranEvent, "taranevent");
				anx->Release();
			}

			anx = null;
			taran = null;
		}

		void Init(Taran * taran, IAnimation * anx)
		{
			Release();

			if (!anx || !taran) return;

			AddHandlers(anx);

			this->taran = taran;
			this->anx = anx;
			this->anx->Clone();
			anx->SetEventHandler(this, (AniEvent)&TaranEvents::TaranEvent, "taranevent");
		}
	};

	TaranEvents	breakEvents;
	AnimationStdEvents * doorStdEvents;
};

#endif