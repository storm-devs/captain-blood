
#include "Door.h"
#include "..\..\character\character.h"
#include "..\..\character\components\characterlogic.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"
#include "..\..\..\common_h\ILiveService.h"

Door::~Door()
{
	Release();
}

void Door::InitData()
{
	brokeStarted = false;

	button_time = 0.0f;
	num_pressed = 0;

	fPushTime = 0.0f;	

	triger_Broke.Reset();
	triger_Kick.Reset();

	doorBlendStage = null;
	bar.Reset();

	bIstance = false;

	kick_Widget.Empty();

	num_coliders_open = 0;

	isOpened = false;

	cur_timing = 0;	
	num_timing = 1;

	post_finish = false;

	anim_speed = 1.0;

	anim_addspeed = 0.25f;
	anim_speedlimit = 3.0f;
	anim_downspeed = 1.0f;

#ifdef CHARACTER_STARFORCE
	SpikeValue = null;
#endif
}

void Door::Release()
{
#ifdef CHARACTER_STARFORCE
	DELETE(SpikeValue);
#endif

	if (doorBlendStage)
	{				
		delete doorBlendStage;
		doorBlendStage = null;		
	}	

	QuickEvent::Release();	
}

bool Door::EditMode_Update(MOPReader & reader)
{
	Release();

#ifdef CHARACTER_STARFORCE
	SpikeValue = NEW float;
	*SpikeValue = 0.0f;
#endif

	player = (Character*)Mission().Player();
	SetPlayerAnimEvents();	

	ReadStdParams(reader);	

	Vector loc_position = reader.Position();
	Vector angels = reader.Angles();
		
	player_point = -transform.vz;
	player_point.y = 0.0f;
	player_point.Normalize();
	player_point = position + player_point * reader.Float();

	IGMXScene* old_model = model;
	model = Geometry().CreateGMX( reader.String().c_str() , &Animation(), &Particles(), &Sound());	
	RELEASE(old_model);		

	const char* anim_str = reader.String().c_str();
	bool isDynamicLight = reader.Bool();

	if (anim)
	{
		events.DelAnimation(anim);
		anim->Release();
		anim = null;
	}

	if (model)
	{
		model->SetAnimationFile(anim_str);
		model->SetDynamicLightState(isDynamicLight);

		anim = model->GetAnimation();		

		if (anim)
		{
			doorBlendStage = NEW DoorBlendStage;
			
			doorBlendStage->Init(model,anim);						

			anim->Start();

			events.Init(&Sound(),&Particles(),&Mission());
			events.AddAnimation(anim);			
			events.SetScene(model,transform);
		}				
	}	

	num_timing = reader.Array();

	for (int i=0;i<num_timing;i++)
	{
		timings[i].difficulty = reader.String().c_str();
		timings[i].num_presses = reader.Long();
	}	

	anim_addspeed = reader.Float();
	anim_speedlimit = reader.Float();
	anim_downspeed = reader.Float();

	progressBar = reader.String();
	kick_Widget = reader.String();

	num_coliders_open = reader.Array();

	for (int i=0;i<num_coliders_open;i++)
	{
		coliders_open[i].cld_loc_pos = reader.Position();
		coliders_open[i].cld_loc_angels = reader.Angles();
		coliders_open[i].cld_size.x = reader.Float() * 0.5f;
		coliders_open[i].cld_size.y = reader.Float() * 0.5f;
		coliders_open[i].cld_size.z = reader.Float() * 0.5f;

		coliders_open[i].colider_matrix = Matrix(coliders_open[i].cld_loc_angels,coliders_open[i].cld_loc_pos) * transform;

		if (!EditMode_IsOn())
		{
			coliders_open[i].Init(&Physics(),this);
			coliders_open[i].Activate(false);
		}
	}				

	bIstance = reader.Bool();
			
	triger_Broke.Init(reader);	
	triger_Kick.Init(reader);

	FindObject(progressBar,bar);
	ShowObject(progressBar,false);		

	transform = Matrix(angels,loc_position) * transform;

	return true;
}

void _cdecl Door::EditMode_Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	for (int i=0;i<num_coliders_open;i++)
	{	
		Render().DrawSolidBox(-coliders_open[i].cld_size, coliders_open[i].cld_size, coliders_open[i].colider_matrix, 0xaa0000ff);
	}

	QuickEvent::EditMode_Draw( dltTime, level);
}

void Door::BeginQuickEvent()
{	
	DetermineDifficulty();

	brokeStarted = false;

	button_time = 0.0f;
	num_pressed = 0;	

	anim_speed = 1.0f;
	player->physics->Orient(position);

#ifdef CHARACTER_STARFORCE
	checkSpike = 1e-20f;
	*SpikeValue = 0.0f;
#endif

	if (!bIstance)
	{		
		FindObject(progressBar,bar);

		if (bar.Ptr())
		{
			const char* params[1];				

			/*char buffer[16];
			crt_snprintf(buffer,sizeof(buffer),"%f", time);			
			params[0] = buffer;

			bar.Ptr()->Command("SetHP",1,params);*/
			BarSetHP(1.0f);

			params[0] = "100";
			bar.Ptr()->Command("SetMax",1,params);

			bar.Ptr()->Show(true);
		}	

		if (player->animation) player->animation->Goto(startNode.c_str(),0.0f);
	}
	else
	{
		if (player->animation) player->animation->Goto("Idle",0.0f);
		player->physics->Activate(false);		

		player->logic->SetPairMode(true);
	}

	fPushTime = 1.5f;

	ShowObject(kick_Widget,true);

	QuickEvent::BeginQuickEvent();
}

void Door::QuickEventUpdate(float dltTime)
{
#ifdef CHARACTER_STARFORCE
	if (checkSpike > 127.0f)
	{
		checkSpike -= 127.0f;

		if (player && player->arbiter->GuardingValueDamage<short>(3) - 73 != 0)
		{
			float time = player->arbiter->GetTime().GetTimeFloat();
			*SpikeValue += (0.49f + float(RRnd(0.0f, Min(4.0f, time / 63.0f))));
		}
	}
#endif

	if (bIstance)
	{	
		if (MovePlayer(dltTime))
		{		
			if (player->animation) player->animation->Goto(startNode.c_str(),0.2f);				
			ActivateLink("break door");				
			isOpened = true;
			ActivateColision(true);
		}

		return;
	}	

	if (num_pressed > timings[cur_timing].GetPresses(this)) return;

	if (!player) return;

	fPushTime -= dltTime;

	if (fPushTime>0)
	{
		player->PushCharcters(dltTime,2.0f,true,false);
	}
	else
	{
		fPushTime = 0.0f;
	}

	MovePlayer(dltTime);

	if (num_pressed <= timings[cur_timing].GetPresses(this) - 2)
	{
		player->physics->Orient(position);
	}
	
	//if (player->animation->TestActivateLink("Break Door")&&
	//player->animation->TestActivateLink("Kick Door"))
	{
		if(Controls().GetControlStateType(buttonCode) == CST_ACTIVATED)
		{	
			if (num_pressed >= timings[cur_timing].GetPresses(this) - 1)
			{				
				brokeStarted = true;
				anim_speed = 1.0f;

				player->animation->ActivateLink("Break Door",true);
				isOpened = true;
				ActivateColision(true);
			}
			else
			{
				anim_speed += anim_addspeed;

				if (anim_speed>anim_speedlimit)
				{
					anim_speed = anim_speedlimit;
				}

				player->animation->ActivateLink("Kick Door",true);
			}
		}
	}	

	if(Controls().GetControlStateType(cheatButton) == CST_ACTIVATED)
	{
		anim_speed = 0;
		num_pressed=timings[cur_timing].GetPresses(this) + 1;
		player->animation->ActivateLink("Break Door",true);
		isOpened = true;
		ActivateColision(true);

#ifdef CHARACTER_STARFORCE
		if (player)
			checkSpike += RRnd(96.0f, 128.0f + 64.0f) * Min(1.0f, float(int(player->arbiter->gameTicks->Get(0.0f) / 1877.0)));
#endif
	}

	anim_speed -= dltTime * anim_downspeed;

	if (anim_speed <1.0f)
	{
		anim_speed = 1.0f;
	}

	player->animation->SetPlaySpeed(anim_speed);
	if (anim) anim->SetPlaySpeed(anim_speed);

#ifdef CHARACTER_STARFORCE
	if (rand()%200 == 101)
	{
		float value = *SpikeValue;
		DELETE(SpikeValue);
		SpikeValue = NEW float;
		*SpikeValue = value;
	}
#endif

	if (bar.Ptr())
	{
		/*const char* params[1];

		char buffer[16];		
		crt_snprintf(buffer, sizeof(buffer),"%f", (1.0f - (float)num_pressed/(float)timings[cur_timing].GetPresses(this)) * 100.0f );

		params[0] = buffer;
		bar.Ptr()->Command("SetHP",1,params);*/

		if (!bQuickEventEnabled)
		{
			bar.Ptr()->Show(false);
		}
	}
}

void Door::BarSetHP(float hp)
{
	if (!bar.Ptr())
		return;

	const char* params[1];

	char buffer[16];		
	crt_snprintf(buffer, sizeof(buffer),"%f", hp * 100.0f);//(1.0f - (float)num_pressed/(float)timings[cur_timing].GetPresses(this)) * 100.0f );

	params[0] = buffer;
	bar.Ptr()->Command("SetHP",1,params);
}

bool Door::AllowInterruptByButton()
{
	if (bIstance) return false;

	// Vano: поменял тут >= на > и добавил || brokeStarted, чтобы можно было выйти из последнего хп 
	// двери, и нельзя было если пошла финальная анимация разлома
	if (num_pressed > timings[cur_timing].GetPresses(this) - 1 || brokeStarted)
	{
		return false;
	}

	if(Controls().GetControlStateFloat(buttonCode)>0.1f)
	{
		return false;
	}

	return true;
}

bool Door::InteruptOnHit()
{ 
	if (num_pressed >= timings[cur_timing].GetPresses(this))
		return false;

	if (brokeStarted)
		return false;

	return !bIstance; 
}

void Door::Interupt(bool win_game)
{		
	if (!bQuickEventEnabled)
		return;

	if (win_game) 
		ACHIEVEMENT_REACHED(player, BURGLAR);

	if (win_game)
		wasCompleted = true;

	if (win_game&&!post_finish)
	{
		LogicDebug("Door Success Triger Activated");
		triger_Success.Activate(Mission(),false);		

		Activate(false);
		wasCompleted = true;
	}	

	if (!win_game)
	{
		LogicDebug("Door Interrupt Triger Activated");
		triger_Interrupt.Activate(Mission(),false);

		player->animation->ActivateLink("idle",true);
		ActivateLink("idle");
	}
		
	ShowObject(kick_Widget,false);

	//bQuickEventEnabled = false;
	ShowObject(progressBar,false);

	player->physics->Activate(true);	
	player->logic->SetPairMode(false);	

	player->animation->SetPlaySpeed(1.0f);
	if (anim) anim->SetPlaySpeed(1.0f);

	QuickEvent::Interupt(win_game);	
}

void Door::AnimEvent(const char * param)
{
	if (!bQuickEventEnabled) return;	

	if (param[0] == 'k')  //"kick"
	{
#ifdef CHARACTER_STARFORCE
		if (player)
			checkSpike += RRnd(32.0f, 64.0f + 32.0f) * Min(1.0f, float(int(player->arbiter->gameTicks->Get(0.0f) / 1877.0)));
#endif
		num_pressed++;		
		BarSetHP(1.0f - num_pressed / (float)timings[cur_timing].GetPresses(this));
		LogicDebug("Door Kick Triger Activated");
		triger_Kick.Activate(Mission(),false);
		ActivateLink("kick door");

#ifdef CHARACTER_STARFORCE
		if (player && player->arbiter->timeInSecFromStart->Get(0.0f) > 1877.0)
			checkSpike += RRnd(32.0f, 64.0f + 32.0f);
#endif
	}
	else
	if (param[0] == 'b')  //"break"
	{
		brokeStarted = true;
		BarSetHP(0.0f);
		LogicDebug("Door Broke Triger Activated");
		triger_Broke.Activate(Mission(),false);
		ActivateLink("break door");		
	}
	else
	if (param[0] == 'f')  //"finish"
	{
		LogicDebug("Door Success Triger Activated");
		triger_Success.Activate(Mission(),false);		
	}
	else
	if (param[0] == 'p')  //"post finish"
	{		
		Activate(false);

		post_finish = true;
		Interupt(true);
		post_finish = false;
	}
}

void Door::HideHUD()
{
	ShowObject(kick_Widget,false);
	
	if (bar.Ptr())
	{			
		bar.Ptr()->Show(false);
	}

	QuickEvent::HideHUD();	
}

void Door::Command(const char * id, dword numParams, const char ** params)
{
	if (!id) return;

	if ((string::IsEqual(id,"instance_open") || string::IsEqual(id,"open")) && !bQuickEventEnabled)
	{
		if (string::IsEqual(id,"open"))
		{
			ActivateLink("open");
		}
		else
		{
			GotoNode("open");

		}

		Activate(false);			

		bQuickEventEnabled = false;
		ShowObject(progressBar,false);		

		//QuickEvent::Interupt(true);

		isOpened = true;
		wasCompleted = true;

		ActivateColision(true);

		PlayHintParticle(false);
		if (button_tip) button_tip->Activate(false);		
	}
	else
	if (string::IsEqual(id,"close"))
	{
		ActivateLink("close");


		Activate(true);			

		bQuickEventEnabled = false;
		ShowObject(progressBar,false);

		//QuickEvent::Interupt(true);

		isOpened = false;

		ActivateColision(true);
	}
	else
	if (string::IsEqual(id,"kick"))
	{
		ActivateLink("kick door");		
	}

	QuickEvent::Command(id,numParams,params);
}

void Door::ActivateColision(bool isAct)
{
	colider.Activate(false);

	for (int i=0;i<num_coliders_open;i++)
	{
		coliders_open[i].Activate(false);
	}

	if (!isOpened)
	{
		colider.Activate(isAct);		
	}
	else
	{
		for (int i=0;i<num_coliders_open;i++)
		{
			coliders_open[i].Activate(isAct);
		}
	}
}

void Door::DetermineDifficulty()
{
	ICoreStorageString* var = api->Storage().GetItemString("Profile.Global.Difficulty",_FL_);

	if (var)
	{
		if (var->IsValidate())
		{
			for (int i=0;i<num_timing;i++)
			if (string::IsEqual(var->Get("Normal"),timings[i].difficulty))
			{
				cur_timing = i;
				var->Release();
				return;
			}
		}

		var->Release();
	}
	

	cur_timing = 0;
}

static const char * comment;
const char * Door::comment = "Door Object. Set Player Start Node.";

MOP_BEGINLISTG(Door, "Door Object", '1.00', 150, "Quick Events")
		
	MOP_QE_STD	

	MOP_POSITION("Local Position", Vector(0.0f))
	MOP_ANGLES("Local Angels", Vector(0.0f))

	MOP_FLOATEX("Player distance", 1.0f,0.1f,1000000.0f)
	
	MOP_STRING("Model", "")
	MOP_STRING("Animation", "")

	MOP_BOOL("Dynamic Lighting", true)

	MOP_ARRAYBEGC("Difficulty Settings", 1, 7, "Difficulty Settings")

		MOP_STRING("Difficulty", "")		
		MOP_LONGEX("Button Times", 5, 2, 100)		

	MOP_ARRAYEND

	MOP_FLOATEX("Anim speed add", 0.25f,0.01f,1000.0f)
	MOP_FLOATEX("Anim speed limit", 1.75f,1.01f,1000.0f)
	MOP_FLOATEX("Anim speed down", 1.0f,0.1f,1000.0f)

	MOP_STRING("Progress Bar", "")

	MOP_STRING("Kick Button Widget", "")


	MOP_ARRAYBEGC("Open Door Coliders", 0, 5, "Open Door Coliders")

		MOP_POSITION("Colider locPos", Vector(0.0f))
		MOP_ANGLES("Colider locAng", Vector(0.0f))
		MOP_FLOAT("Colider Width", 1.0f)
		MOP_FLOAT("Colider Height", 1.0f)
		MOP_FLOAT("Colider Lenght", 1.0f)
		
	MOP_ARRAYEND

	MOP_BOOL("Is istance", false)
		
	MOP_MISSIONTRIGGERG("Broke trigger", "Broke")	
	MOP_MISSIONTRIGGERG("Kick trigger", "Kick")	

MOP_ENDLIST(Door)