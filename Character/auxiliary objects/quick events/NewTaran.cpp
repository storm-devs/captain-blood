#include "NewTaran.h"
#include "..\..\..\common_h\AnimationNativeAccess.h"
#include "..\..\character\character.h"
#include "..\..\character\components\characterlogic.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"
#include "..\..\player\playerController.h"

ConstString Taran2::chainSndName("m62_taran_roll");

void Taran2::InitData()
{
	penalty = false;

	debugTaran = false;

	started = 0;
	kickValue = 0.0f;
	kickDamage = 0.0f;
	lastKick = 0;
}

Taran2::Taran2()
{
	door_model = null;
	door_anim = null;
	model = null;

	taranBlender = null;

	anim = null;
	taranAnimLight = null;
	taranAnimStrong = null;

	playerBlender = null;
	playerAnimLight = null;
	playerAnimStrong = null;

	taranBlender = null;
	playerBlender = null;

	doorBlendStage = NULL;

	doorStdEvents = null;
}

Taran2::~Taran2()
{
	RemovePlayerAnimEvents();
	DELETE(quick_anim_events);
	DELETE(doorStdEvents);
	Release();
}

void Taran2::Release()
{
	QuickEvent::Release();	

	DELETE(taranBlender);
	DELETE(playerBlender);

	breakEvents.Release();

	DELETE(doorBlendStage)

	RELEASE(door_anim);	
	RELEASE(door_model);	

	RELEASE(taranAnimLight);
	RELEASE(taranAnimStrong);

	RELEASE(playerAnimLight);
	RELEASE(playerAnimStrong);
}

void Taran2::SetPlayerAnimEvents()
{
	if (playerAnimLight)
		playerAnimLight->SetEventHandler(quick_anim_events, (AniEvent)(&AnimListener::AnimEvent), "QuickEvent");		
}

void Taran2::RemovePlayerAnimEvents()
{
	if (playerAnimLight)
		playerAnimLight->DelEventHandler(quick_anim_events, (AniEvent)(&AnimListener::AnimEvent), "QuickEvent");		
}

bool Taran2::EditMode_Update(MOPReader & reader)
{	
	player = (Character*)Mission().Player();

	ReadStdParams(reader);	

	parralel_graph = reader.String().c_str();

	Vector loc_position = reader.Position();
	Vector angels = reader.Angles();

	player_point = -transform.vz;
	player_point.y = 0.0f;
	player_point.Normalize();
	player_point = position + player_point * reader.Float();

	IGMXScene* old_model = model;
	model = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Particles(), &Sound());	
	RELEASE(old_model);		
	
	orientpoint = transform.pos + transform.vz;	

	const char* anim_str = reader.String().c_str();	

	breakEvents.Release();
	DELETE(taranBlender);
	RELEASE(anim);
	RELEASE(taranAnimLight);
	RELEASE(taranAnimStrong);

	if (model && anim_str)
	{
		anim = Animation().Create(anim_str, _FL_);
		taranAnimLight = Animation().Create(anim_str, _FL_);
		taranAnimStrong = Animation().Create(anim_str, _FL_);

		model->SetAnimation(anim);
		model->SetDynamicLightState(true);

		if (anim && taranAnimLight && taranAnimStrong)
		{
			taranBlender = NEW TaranBlender(anim, taranAnimLight, taranAnimStrong);
			taranBlender->Restart();

			breakEvents.Init(this, taranAnimStrong);
		}
	}
	else
		taranBlender = null;

	useCharBonePosition = reader.Bool();
	const char* bone_str = reader.String().c_str();
	playerPosition = reader.Position();
	playerAngles = reader.Angles();
	playerTransform.Build(playerAngles, playerPosition);
	
	DELETE(doorBlendStage);
	DELETE(doorStdEvents);

	old_model = door_model;
	door_model = Geometry().CreateGMX( reader.String().c_str() , &Animation(), &Particles(), &Sound());	
	RELEASE(old_model);			

	anim_str = reader.String().c_str();
	if (door_model)
	{
		doorStdEvents = NEW AnimationStdEvents();
		doorStdEvents->Init(&Sound(), &Particles(), &Mission());

		door_model->SetAnimationFile(anim_str);
		door_model->SetDynamicLightState(true);

		door_anim = door_model->GetAnimation();

		if (door_anim)
		{
			doorBlendStage = NEW DoorBlendStage;
			doorBlendStage->Init(door_model,door_anim);
			door_anim->Start();
		}	
	}

	Vector door_position = reader.Position();
	Vector door_angels = reader.Angles();

	door_transform = Matrix(door_angels,door_position);// * transform;

	if (doorStdEvents && door_model)
		doorStdEvents->SetScene(door_model, door_transform);	

	max_life = reader.Long();
	num_damages = reader.Long();
	progressBar = reader.String();

	widget_left = reader.String();
	widget_right = reader.String();

	chainSndPosition = reader.Position();

	debugTaran = reader.Bool();
	debugPower = reader.Float();

	triger_Kick.Init(reader);

	accum_time = 0.0f;
	blend_time = 0.0f;
	
	transform = Matrix(angels,loc_position) * transform;


	if (anim)
	{
		bone_index = anim->FindBone(bone_str,true);		
	}
	else
	{
		bone_index = -1;
	}

	//transform = Matrix(angels,loc_position) * transform;



	blend_time = 0.0f;
	accum_time = 0.0f;
	pos_dir = false;
	life = max_life;

	kickValue = 0.0f;
	kickDamage = float(num_damages) / Max(1.0f, float(max_life) - 2.0f);
	lastKick = 0;

	FindObject(progressBar,bar);
	ShowObject(progressBar,false);	

	DELETE(quick_anim_events);
	quick_anim_events = NEW AnimListener();
	quick_anim_events->Init(this);

	if (player && taranBlender)
	{
		//RemovePlayerAnimEvents();

		RELEASE(playerAnimLight);
		RELEASE(playerAnimStrong);

		playerAnimLight = Animation().Create(parralel_graph, _FL_);
		playerAnimStrong = Animation().Create(parralel_graph, _FL_);

		SetPlayerAnimEvents();
	}

	//if (player) player->animation->AddAnimation("taran",parralel_graph);

	SetUpdate(&Taran2::TikTac, ML_DEBUG1);	
	SetUpdate(&Taran2::UpdateAnimations, ML_EXECUTE9);

	/*if (modelShadowCast || doorShadowCast)
		Registry(MG_SHADOWCAST, (MOF_EVENT)&Taran2::ShadowInfo, 0);
	else
		Unregistry(MG_SHADOWCAST);

	if (modelShadowRecv || doorShadowRecv)
		Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&Taran2::ShadowReceive, 0);
	else
		Unregistry(MG_SHADOWRECEIVE);*/

	return true;
}

void _cdecl Taran2::Draw(float dltTime, long level)
{
	if (door_model)
	{
		door_model->SetTransform(door_transform);
		door_model->Draw();
	}

	QuickEvent::Draw(dltTime, level);
}

//Нарисовать модельку для тени
void _cdecl Taran2::ShadowInfo(const char * group, MissionObject * sender)
{
	if (model)
	{
		model->SetTransform(transform);
		const Vector & vMin = model->GetBound().vMin;
		const Vector & vMax = model->GetBound().vMax;
		((MissionShadowCaster *)sender)->AddObject(this, &Taran2::ShadowCastModel, vMin, vMax);
	}

	if (door_model)
	{
		door_model->SetTransform(door_transform);
		const Vector & vMin = door_model->GetBound().vMin;
		const Vector & vMax = door_model->GetBound().vMax;
		((MissionShadowCaster *)sender)->AddObject(this, &Taran2::ShadowCastDoor, vMin, vMax);
	}
}

//Нарисовать модельку для тени
void _cdecl Taran2::ShadowCastModel(const char * group, MissionObject * sender)
{
	if (!model)
		return;

	model->SetTransform(transform);
	model->Draw();
}

//Нарисовать модельку для тени
void _cdecl Taran2::ShadowCastDoor(const char * group, MissionObject * sender)
{
	if (!door_model)
		return;

	door_model->SetTransform(door_transform);
	door_model->Draw();
}

//Нарисовать модельку для тени
void _cdecl Taran2::ShadowDraw(const char * group, MissionObject * sender)
{
	if (model)
	{
		model->SetTransform(transform);
		model->Draw();
	}

	if (door_model)
	{
		door_model->SetTransform(door_transform);
		door_model->Draw();
	}
}

//Получить бокс, описывающий model и door_model
void Taran2::GetBox(Vector & min, Vector & max)
{
	if (model)
	{
		model->SetTransform(transform);
		min = model->GetLocalBound().vMin;
		max = model->GetLocalBound().vMax;

		if (door_model)
		{
			door_model->SetTransform(door_transform);

			min.Min(door_model->GetLocalBound().vMin);
			max.Max(door_model->GetLocalBound().vMax);
		}
	}
	else
	{
		min = 0.01f;
		max = 0.01f;
	}
}

void Taran2::BeginQuickEvent()
{			
	//player->physics->Activate(false);		
	player->logic->SetAnimSlowDown(false);

	if (player->animation)
	{
		player->animation->Goto("idle",0.2f);
		//player->animation->EnableGraphBlend("taran");
	}		

	if (taranBlender)
		taranBlender->Restart();
	if (playerBlender)
		playerBlender->Restart();

	started = 0;
	{
		FindObject(progressBar,bar);

		if (bar.Validate())
		{
			const char* params[1];				

			char buffer[16];
			crt_snprintf(buffer,sizeof(buffer),"%f", time);			
			params[0] = buffer;

			bar.Ptr()->Command("SetHP",1,params);

			params[0] = "100";
			bar.Ptr()->Command("SetMax",1,params);

			bar.Ptr()->Show(true);
		}	
	}

	if (bQuickEventEnabled)
	{
		ShowObject(widget_left,pos_dir);
		ShowObject(widget_right,!pos_dir);
	}

	QuickEvent::BeginQuickEvent();
}

void _cdecl Taran2::UpdateAnimations(float dltTime, long level)
{
	if (taranBlender)
		taranBlender->ReadData();

	if (playerBlender)
		playerBlender->ReadData();
}

void _cdecl Taran2::TikTac(float dltTime, long level)
{
	float prev_blend_time = blend_time;

	accum_time += dltTime;
	Vector dir(0.0f);

	if (bQuickEventEnabled && started>1) dir.x = Controls().GetControlStateFloat(buttonCode);
	//dir.z = Controls().GetControlStateFloat("ChrForwBack1");

	if(bQuickEventEnabled && started>1)
	{
		if (Controls().GetControlStateType(cheatButton) == CST_ACTIVATED)
		{
			life = 0;
			
			ActivateLink("Break");				

			if (door_anim) door_anim->Goto("open", 0.0f);
			Interupt(true);			
		}
	}

	const Matrix & cam = Render().GetView();
	dir = cam.MulNormalByInverse(dir);

	dir = transform.MulNormalByInverse(dir);
	float cosine = dir.z * (pos_dir ? -1.0f : 1.0f);

	for(const float step = 0.05f; accum_time >= step; accum_time -= step)
	{		
		//Render().DrawVector(transform.pos, transform.pos + dir, 0xffff0000);
		//Render().DrawMatrix(transform);

		if (fabs(cosine) > 0.3f)
		{
			if(cosine > 0.0f)
			{
				blend_time += step * 0.25f;
			}
			else
			{
				if (penalty)
				{
					blend_time -= step * 0.8f;
				}
				else
				{
					float kAtt = 0.9f*powf(blend_time, 2.0f) + 0.1f;
					kAtt *= 0.9f;
					blend_time -= step * kAtt;
				}
			}			
		}
		else
		{			
			float kAtt = 0.9f*powf(blend_time, 2.0f) + 0.1f;
			kAtt *= 0.9f;
			blend_time -= step * kAtt;
		}
	}	

	if (bQuickEventEnabled)// && blend_time>0.2f)
	{
		ShowObject(widget_left,pos_dir);
		ShowObject(widget_right,!pos_dir);
	}

	//Render().Print(10,10,0xffffffff,"%4.3f",blend_time);
	//if (penalty) Render().Print(10,30,0xffffffff,"penalty");
	//Render().Print(10,50,0xff00ff00,"%4.3f",dir.z);
	//Render().Print(10,70,0xff00ff00,"%4.3f",pos_dir ? -1.0f : 1.0f);

	if (bQuickEventEnabled)
	{
		blend_time = Clampf(blend_time,0.00f,1.0f);
	}
	else
	{
		blend_time = Clampf(blend_time,0.00f,1.0f);
	}

	//blend_time = 0.6f;
		
	//if (player && bQuickEventEnabled)
	{
		//player->animation->SetGraphBlend(blend_time);
	}

#ifndef STOP_DEBUG
	blend_time = Clamp(blend_time * debugPower);
#endif

	if (taranBlender)
		taranBlender->SetBlend(blend_time);

	if (playerBlender)
		playerBlender->SetBlend(blend_time);
	
	if (useCharBonePosition && bone_index != -1)
	{
		Matrix mt = anim->GetBoneMatrix(bone_index);

		mt.SetIdentity3x3();
		mt.pos.y = 0.0f;

		mt = mt * transform;			
		player_point = mt.pos;

		orientpoint = mt.vz + mt.pos;
	}
	else
	{
		player_point = playerTransform.pos;
		orientpoint = player_point + playerTransform.vz;
	}

	//Sound().Create(ConstString("m62_taran_roll"), _FL_);
	//anim->Get
	//Render().Print(10.0f, 30.0f, 0xFF00FF00, "cosine = %.1f, blend_time = %.2f", cosine, blend_time);

#ifndef STOP_DEBUG
	if (bQuickEventEnabled && debugTaran && taranAnimLight && taranAnimStrong)
	{
		Render().Print(10.0f, 30.0f, 0xFF00FF00, "trn light  = %.2f", GetFrame(taranAnimLight));
		Render().Print(10.0f, 50.0f, 0xFF00FF00, "trn strong = %.2f", GetFrame(taranAnimStrong));
		Render().Print(10.0f, 70.0f, 0xFF00FF00, "trn light node = %s, trn strong node = %s", taranAnimLight->CurrentNode(), taranAnimStrong->CurrentNode());
		
		Render().Print(10.0f, 90.0f, 0xFF00FF00, "blend_time = %.2f", blend_time);

		Render().Print(10.0f, 120.0f, 0xFF00FF00, "plr light  = %.2f", GetFrame(playerAnimLight));
		Render().Print(10.0f, 140.0f, 0xFF00FF00, "plr strong = %.2f", GetFrame(playerAnimStrong));
		Render().Print(10.0f, 160.0f, 0xFF00FF00, "plr light node = %s, plr strong node = %s", playerAnimLight->CurrentNode(), playerAnimStrong->CurrentNode());
		Render().Print(10.0f, 180.0f, 0xFF00FF00, "plr original node = %s", player->animation->GetCurAnimation()->CurrentNode());
		Render().Print(10.0f, 210.0f, 0xFF00FF00, "trn mode : %s,  penalty: %s", (pos_dir) ? "+ positive" : "- negative", (penalty) ? "true" : "false");
		
	}
	/*if (bQuickEventEnabled && debugTaran && break_anim && anim && player->animation->GetCurAnimation() && player->animation->animData[0].animation)
	{
		IAnimation * plr_anim_idle = player->animation->GetCurAnimation();
		IAnimation * plr_anim_break = player->animation->animData[0].animation;

		Render().Print(10.0f, 30.0f, 0xFF00FF00, "trn idle  = %.2f, plr idle  = %.2f", GetFrame(anim), GetFrame(plr_anim_idle));
		Render().Print(10.0f, 50.0f, 0xFF00FF00, "trn break = %.2f, plr break = %.2f", GetFrame(break_anim), GetFrame(plr_anim_break));

		Render().Print(10.0f, 70.0f, 0xFF00FF00, "diff      = %.2f, diff      = %.2f", GetFrame(anim) - GetFrame(break_anim), GetFrame(plr_anim_idle) - GetFrame(plr_anim_break));

		Render().Print(350.0f, 30.0f, 0xFFFF7F7F, "delta = %.2f", fabsf(GetFrame(anim) - GetFrame(plr_anim_idle)));
		Render().Print(350.0f, 50.0f, 0xFFFF7F7F, "delta = %.2f", fabsf(GetFrame(break_anim) - GetFrame(plr_anim_break)));

		Render().Print(10.0f, 90.0f, 0xFF00FF00, "fps: trn idle = %.1f, trn break = %.1f", GetFPS(anim), GetFPS(break_anim));
		Render().Print(10.0f, 110.0f, 0xFF00FF00, "fps: plr idle = %.1f, plr break = %.1f", GetFPS(plr_anim_idle), GetFPS(plr_anim_break));

		Render().Print(10.0f, 140.0f, 0xFF00FF00, "trn idle node = %s, trn break node = %s", anim->CurrentNode(), break_anim->CurrentNode());
		Render().Print(10.0f, 160.0f, 0xFF00FF00, "plr idle node = %s, plr break node = %s", plr_anim_idle->CurrentNode(), plr_anim_break->CurrentNode());
		
		Render().Print(10.0f, 180.0f, 0xFF00FF00, "blend_time = %.2f", blend_time);
	}*/
#endif

	if (api->DebugKeyState('K'))
	{
		/*AGNA_SetCurrentFrame f0(0.0f), f1(0.0f), f2(0.0f) ,f3(0.0f);
		anim->GetNativeGraphInfo(f0);
		break_anim->GetNativeGraphInfo(f1);
		player->animation->GetCurAnimation()->GetNativeGraphInfo(f2);
		player->animation->animData[0].animation->GetNativeGraphInfo(f3);*/
		//Sleep(200);

		/*if (player->animation) player->animation->Goto(startNode.c_str(),0.0f);
		ActivateLink("Start");
		if (break_anim) break_anim->Goto("Start", 0.0f);
		if (anim) anim->Goto("Start", 0.0f);*/

		//ActivateLink("Kick");
		//if (player->animation) player->animation->ActivateLink("Kick");
	}
}

#ifndef STOP_DEBUG
float Taran2::GetFrame(IAnimation * _anim)
{
	AGNA_GetCurrentFrame anim_frame;
	_anim->GetNativeGraphInfo(anim_frame);

	return anim_frame.currentFrame;
}

float Taran2::GetFPS(IAnimation * _anim)
{
	AGNA_NodeInfo node(_anim->CurrentNode());	
	_anim->GetNativeGraphInfo(node);

	AGNA_ClipInfo fps(node.index, 0);		
	_anim->GetNativeGraphInfo(fps);

	return fps.fps;
}
#endif

void Taran2::BreakAnimEvent(const char * name, const char ** params, dword numParams)
{
	ConstString sndName;

	if (numParams && params[0][0])
		sndName.Set(params[0]);
	else
		sndName = chainSndName;

	ISound3D * snd = Sound().Create3D(sndName, chainSndPosition, _FL_);

	if (snd)
	{
		snd->SetVolume(Clamp(0.2f + blend_time * 0.8f));
		//snd->Play();
	}
}

void Taran2::QuickEventUpdate(float dltTime)
{		
	if (MovePlayer(dltTime))
	{		
		Start();
		//ActivateLink("Start");
		
		player->animation->DisableGraphBlend();
		if (taranAnimLight && taranAnimStrong)
		{
			DELETE(playerBlender);

			playerBlender = NEW TaranBlender(player->animation->GetCurAnimation(), playerAnimLight, playerAnimStrong);
			playerBlender->Restart();
		}

		bool b1 = playerAnimLight->Goto("light_Start", 0.2f);
		bool b2 = playerAnimStrong->Goto("strong_Start", 0.2f);

		bool b3 = player->animation->Goto(startNode.c_str(), 0.2f);

		started = 1;
	}
	
	if (started>=1)
	{
		player->physics->SetPos(player_point);
	}

	player->physics->Orient(orientpoint);		
	
	//player->animation->SetPlaySpeed( (1 - blend_time) * 0.35f + 0.5f );

	if (bar.Validate())
	{
		const char* params[1];

		char buffer[16];		
		crt_snprintf(buffer, sizeof(buffer),"%f", ((float)life/(float)max_life) * 100.0f );

		params[0] = buffer;
		bar.Ptr()->Command("SetHP",1,params);

		if (!bQuickEventEnabled)
		{
			bar.Ptr()->Show(false);
		}
	}
}

void Taran2::Interupt(bool win_game)
{		
	if (!bQuickEventEnabled)
		return;

	ShowObject(widget_left,false);
	ShowObject(widget_right,false);
	ShowObject(progressBar,false);

	if (player->animation) player->animation->DisableGraphBlend();
	DELETE(playerBlender);

	if (win_game)
	{
		colider.Activate(false);
		LogicDebug("Taran Success Triger Activated");
		triger_Success.Activate(Mission(),false);
		Activate(false);

		ACHIEVEMENT_REACHED(player, UNSTOPPABLE);
	}
	else
	{
		LogicDebug("Door Interrupt Triger Activated");
		triger_Interrupt.Activate(Mission(),false);
	}


	if (player->animation) player->animation->ActivateLink("idle");


	//bQuickEventEnabled = false;

	player->logic->SetAnimSlowDown(true);	

	ActivateLink("End");

	QuickEvent::Interupt(win_game);	
}

void Taran2::AnimEvent(const char * param)
{		
	if (!bQuickEventEnabled) return;

	if (param[0] == 'z')
	{
		penalty = true;
	}
	else
	if (param[0] == 'x')
	{
		penalty = false;
	}
	else
	if (param[0] == 'p')
	{
		pos_dir = true;	
	}
	else
	if (param[0] == 'n')
	{					
		pos_dir = false;
	}
	else
	if (param[0] == 's')
	{						
		started = 2;
	}
	else		
	if (param[0] == 'k')
	{						
		if (blend_time>0.75f)
		{
			life--;

			if (life<=0)
			{
				ActivateLink("Break");				

				if (door_anim) door_anim->Goto("break", 0.0f);
				Interupt(true);
			}
			else
			{
				ActivateLink("Kick");
				ActivatePlayerLink("Kick");
				//if (player->animation) player->animation->ActivateLink("Kick");

				LogicDebug("Taran Kick Triger Activated");
				triger_Kick.Activate(Mission(),false);

				char anim_str[256];

				kickValue += kickDamage;
				if (lastKick != int(kickValue))
					sprintf_s(anim_str, sizeof(anim_str), "kick%d", int(kickValue));
				else
					sprintf_s(anim_str, sizeof(anim_str), "shake%d", int(kickValue) + 1);
				if (door_anim) door_anim->Goto(anim_str, 0.0f);
				lastKick = int(kickValue);
			}
		}		
	}
}

void Taran2::HideHUD()
{
	ShowObject(widget_left,false);
	ShowObject(widget_right,false);
	
	if (bar.Validate())
	{			
		bar.Ptr()->Show(false);
	}

	QuickEvent::HideHUD();	
}

bool Taran2::AllowInterruptByButton()
{
	if (fabsf(Controls().GetControlStateFloat(buttonCode))>0.1f)
	{
		return false;
	}

	return true;
}

void Taran2::Command(const char * id, dword numParams, const char ** params)
{
	if ((string::IsEqual(id,"instance_open") || string::IsEqual(id,"open")))
	{
		if (string::IsEqual(id,"open"))
		{
			if (door_anim) door_anim->Goto("open", 0.0f);
		}
		else
		{
			if (door_anim) door_anim->Goto("open",0.1f);

		}

		Activate(false);			

		bQuickEventEnabled = false;
		ShowObject(progressBar,false);		

		//QuickEvent::Interupt(true);		

		PlayHintParticle(false);
		if (button_tip) button_tip->Activate(false);		
	}

	QuickEvent::Command(id,numParams,params);
}

static const char * comment;
const char * Taran2::comment = "Taran. Set Player Start Node.";

MOP_BEGINLISTG(Taran2, "Taran Object v2", '1.00', 150, "Quick Events")
		
	MOP_QE_STD	

	MOP_STRING("Parallel Graph", "")

	MOP_POSITION("Local Position", Vector(0.0f))
	MOP_ANGLES("Local Angels", Vector(0.0f))

	MOP_FLOATEX("Player distance", 0.0f,0.0f,10000.0f)

	MOP_STRING("Model", "")
	MOP_STRING("Taran Animation", "")

	MOP_BOOLC("Use Bone Position", true, "true - use Char Bone position, false - use Player Position/Angles")
	MOP_STRING("Char Bone position", "")
	MOP_POSITION("Player Position", Vector(0.0f))
	MOP_ANGLES("Player Angles", Vector(0.0f))

	MOP_STRING("Door Model", "")
	MOP_STRING("Door Animation", "")

	MOP_POSITION("Door Position", Vector(0.0f))
	MOP_ANGLES("Door Angels", Vector(0.0f))

	MOP_LONGEX("Life", 5, 1, 1000)
	MOP_LONGEXC("Damages", 0, 0, 1000, "Number of damage animations for door")
	MOP_STRING("Life Bar", "")

	MOP_STRING("Widget Right", "")
	MOP_STRING("Widget Left", "")

	MOP_POSITION("Chain Sound Position", Vector(0.0f))

	MOP_BOOL("Debug", false)

	MOP_FLOAT("Debug Taran Power", 1.0f)

	MOP_MISSIONTRIGGERG("Kick trigger", "Kick")	

MOP_ENDLIST(Taran2)