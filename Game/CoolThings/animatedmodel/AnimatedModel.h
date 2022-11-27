#ifndef _AnimatedModel_h_
#define _AnimatedModel_h_

#include "Track.h"
#include "Shell.h"

#include "..\..\..\Common_h\AnimationStdEvents.h"

class AnimatedModelsArbiter;

class AnimatedModel : public MissionObject
{
	class AnimationEvents : public AnimationStdEvents
	{
	public:
		AnimationEvents()
		{
			this->animatedModel = null;
			this->model = null;
		}

		void InitModels(AnimatedModel * animatedModel, IGMXScene * model)
		{
			this->animatedModel = animatedModel;
			this->model = model;
		}

		virtual void AddHandlers(IAnimation *anx)
		{
			if( !anx )
				return;

			AnimationStdEvents::AddHandlers(anx);

			anx->SetEventHandler(this,(AniEvent)&AnimationEvents::Vibrate, "StartVibration");
		}

	public:
		void _cdecl Vibrate(IAnimation *ani, const char *name, const char **params, dword numParams)
		{
			if( numParams < 1)
				return;

			IForce *force = mission->Controls().CreateForce(params[0]);

			if( force )
				force->Play();
		}
	private:
		AnimatedModel * animatedModel;
		IGMXScene  * model;
	};

	enum EditMode
	{
		edit_path,
		edit_rotation_x,
		edit_rotation_y,
		edit_rotation_z,
		edit_scale
	};

	struct Link
	{
		const char * id;

		long srcTrack; long srcItem;
		long dstTrack; long dstItem;

		bool active;
	};

	enum FadingType
	{
		fade_none,
		fade_in,
		fade_out
	};

public:

	AnimatedModel();
	virtual ~AnimatedModel();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void PostCreate();

//	bool EditMode_Export();

	void Activate(bool isActive);
	void Show	 (bool isShow);

	void GetBox(Vector &min, Vector &max);

	void Restart();

	Matrix &GetMatrix(Matrix &m);

	MO_IS_FUNCTION(AnimatedModel, MissionObject);

	void Command(const char *id, dword numParams, const char **params);

public:

	void _cdecl ShadowInfo(const char *group, MissionObject *sender);
	void _cdecl ShadowDraw(const char *group, MissionObject *sender);

	void _cdecl SeaReflection(const char *group, MissionObject *sender);

	void _cdecl Draw(float dltTime, long level);

private:

	void DrawModel(bool primary = true);

	void InitParams		(MOPReader &reader);
	void CreateTrackData(MOPReader &reader);

	void CheckLinks();

	void GotoNode(dword ti, dword ni, float estTime = 0.0f);

	void DrawHandle(Vector &pt, Vector &tg, dword color);

	void GetPosition(
		Track &track, dword ni, float t, Vector &lookVec, Vector &upVec, float &za, float &ya, float &xa, float &sc, Vector &pos, LookMode lookMode);

	void Reset();

public:

	void SetDebug(bool active)
	{
		debugDraw = active;
	}

	float DrawDebug(float y);

private:

	AnimationEvents events;

	string curModel;
//	string curAnimation;

	Matrix matModel;

	Matrix matWorld;
	Vector ang;

	Matrix matAngle;

	float  modelScale;
	float  trackScale;

	IGMXScene  *model;
	IAnimation *animation;

	bool model_loadFailed;

	EditMode editMode;

	array<Link> links;

	array<Track> tracks_native;
	Shell<Track> tracks;

	dword curTrack;
	dword curNode;

	dword startTrack;

	dword editTrack;
	bool  editing;

	float trackTime;
	bool  trackDraw;

	bool showRotation;
	bool showScale;

	bool showThrough;
	float capacity;

	float curPos;
	float curTime;

	string animationNode;

	Vector lastLook;
	float  lastScale;

	//// render parameters ////

	long drawLevel;

	Color color;

	bool enableRender;

	bool dynamicLighting;

	bool shadowCast;
	bool shadowReceive;

	bool seaReflection;
	bool seaRefraction;

	bool hideInEditor;

	///////////////////////////

	ConstString masterName;

	MOSafePointer master;

//	bool firstTime;

	bool started;

	ConstString curTracks;

	float delay;
	float lastArrive;

	float restartDelay;

	float  curVelocity;
	float lastVelocity;

	float prevVel;

	bool nodeStarted;

	FadingType fade;

	float fadeAlpha;
	float fadeAlphaMin;

	float fadeTime;

	bool noSwing;

	//// arbiter ////

	AnimatedModelsArbiter *arbiter;

	bool debugDraw;

	/////////////////

	bool saved;	// пытались перейти на новый нод в неактивном состоянии

	dword saveTrack;
	dword saveNode;

	float saveTime;

};

#endif
