
#include "Chest.h"
#include "..\..\character\character.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"
#include "..\..\..\common_h\bonuses.h"

Chest::Chest()
{
	statIsActivated = false;
	statWasOpened = false;
}

bool Chest::EditMode_Update(MOPReader & reader)
{
	dropPosition = 0.0f;
	Release();

	player = (Character*)Mission().Player();
	SetPlayerAnimEvents();

	pattern_name = reader.String();
		
	init_pos = reader.Position();
	init_angels = reader.Angles();

	particle_radius = reader.Float();

	position = init_pos;
	Vector angels = init_angels;
	
	transform = Matrix(angels,position);
	player_point = position;
	
	bonusTable = reader.String();

	InitFromPattern();

	Activate(reader.Bool());
	Show(reader.Bool());
	if(reader.Bool())
	{
		SetUpdate(&Chest::DebugDraw,ML_ALPHA5);
	}

	triger_Accept.Init(reader);
	triger_Enter.Init(reader);
	triger_Exit.Init(reader);
	triger_Interrupt.Init(reader);
	triger_Success.Init(reader);		
	
	Registry(MG_QE);

	return true;
}

bool Chest::InitFromPattern()
{
	pattern = null;

	static const ConstString strTypeId("ChestPattern");
	if (!pattern_ptr.FindObject(&Mission(),pattern_name,strTypeId))
	{
		if (!EditMode_IsOn()) api->Trace("Pattern for %s not set or invalid",GetObjectID().c_str());
		return false;
	}
	
	pattern = pattern_ptr.Ptr();

	radius = pattern->radius_activation;	

	startNode = pattern->player_node;	

	crt_strcpy(buttonName, sizeof(buttonName), pattern->button_name);
	crt_strcpy(buttonName, sizeof(buttonName), "1");


	buttonCode = Controls().FindControlByName(buttonName);

	particle_name = pattern->particle_name;
	particle_loc_pos = pattern->particle_loc_pos;
	particle_loc_angels = pattern->particle_loc_angels;

	InitHintParticle();	

	colider.cld_loc_pos = pattern->colider.cld_loc_pos;
	colider.cld_loc_angels = pattern->colider.cld_loc_angels;
	colider.cld_size = pattern->colider.cld_size;	

	colider.colider_matrix = Matrix(colider.cld_loc_angels,colider.cld_loc_pos) * transform;

	if (!EditMode_IsOn())
	{
		colider.Init(&Physics(),this);
	}

	tip_name = pattern->button_widget_name;	

	if (!EditMode_IsOn())
	{
		ITipsManager* tip_manager = ITipsManager::GetManager(&Mission());

		if (tip_manager)
		{
			button_tip = tip_manager->CreateTip(tip_name,0.0f,this);			

			if (button_tip)
			{
				button_tip->Activate(true);			
			}
		}
	}	

	shd_cast = pattern->shadow_cast;
	shd_rcv = pattern->shadow_recive;

	
	
	player_point = -transform.vz;	
	player_point.y = 0.0f;
	player_point.Normalize();
	player_point = position + player_point * pattern->player_distance;
	

	IGMXScene* old_model = model;
	model = Geometry().CreateGMX( pattern->model_name , &Animation(), &Particles(), &Sound());	
	RELEASE(old_model);		
	

	if (model)
	{
		anim = Animation().Create(pattern->anim_name, _FL_);
		model->SetAnimation(anim);
		model->SetDynamicLightState(pattern->dyn_light);

		if (anim)
		{
			events.Init(&Sound(),&Particles(),&Mission());
			events.AddAnimation(anim);			
			events.SetScene(model,transform);
		}				
	}

	dropPosition = transform*pattern->loc_dropPosition;
	dropAngle = transform.vz.GetAY();
	transform = Matrix(pattern->loc_angles,pattern->loc_pos) * transform;

	if (button_tip)
	{
		button_tip->SetPos(transform.pos);
	}

	return true;
}

void _cdecl Chest::EditMode_Work(float dltTime, long level)
{		
	bool need_update = false;

	if (!pattern_ptr.Validate())
	{		
		static const ConstString strTypeId("ChestPattern");
		if (pattern_ptr.FindObject(&Mission(),pattern_name,strTypeId))
		{
			need_update = true;
			pattern = pattern_ptr.Ptr();
		}
		else
		{		
			pattern = null;
			pattern_ptr.Reset();
			need_update = true;
		}		
	}
	else
	{
		need_update = pattern->IsNeedUpdate();
	}

	if (need_update)
	{
		Release();

		position = init_pos;
		Vector angels = init_angels;

		transform = Matrix(angels,position);
		player_point = position;

		InitFromPattern();		
	}	
}


void _cdecl Chest::DebugDraw(float dltTime, long level)
{
	const dword conusColor = 0xffff00ff;
	Render().DrawSphere(dropPosition, 0.2f, conusColor);	
	Render().DrawSphere(player_point, 0.1f, 0xff0000ff);
	if(pattern)
	{
		//Сектор раскрытия в котором могут падать бонусы
		float minAy = dropAngle + pattern->dropMinAngle;
		float maxAy = dropAngle + pattern->dropMaxAngle;
		Render().DrawVector(dropPosition, dropPosition + Vector(0.0f, 0.0f, 1.0f).Rotate(minAy), conusColor);
		Render().DrawVector(dropPosition, dropPosition + Vector(0.0f, 0.0f, 1.0f).Rotate(maxAy), conusColor);		
		//Нарисуем параболы
		Vector v[4] = {Vector(0.0f, pattern->dropMinVy, pattern->dropMinVxz), 
						Vector(0.0f, pattern->dropMinVy, pattern->dropMaxVxz),
						Vector(0.0f, pattern->dropMaxVy, pattern->dropMinVxz),
						Vector(0.0f, pattern->dropMaxVy, pattern->dropMaxVxz)};
		Vector pos[4];
		Vector oldPos[4];
		for(dword j = 0; j < 4; j++)
		{
			oldPos[j] = pos[j] = dropPosition;
			v[j].Rotate((minAy + maxAy)*0.5f);
		}
		float timeStep = 0.03f;
		Render().FlushBufferedLines();
		for(dword i = 0; i < 300; i++)
		{
			for(dword j = 0; j < 4; j++)
			{
				if(pos[j].y < dropPosition.y - 1.0f) continue;
				v[j].FrictionXZ(pattern->friction*timeStep, 1.0f);
				v[j].y -= 9.8f*timeStep;
				oldPos[j] = pos[j];
				pos[j] += v[j]*timeStep;
				Render().DrawBufferedLine(oldPos[j], conusColor, pos[j], conusColor);
			}
		}
		Render().FlushBufferedLines();
	}
}

void Chest::InitData()
{
	Registry(CHEST_OBJECT_GROUP);

	pattern_name.Empty();
	pattern_ptr.Reset();
	pattern = null;

	allowRestart = true;
	wasCompleted = false;
	
	startNode.Set("break chest1");
	//float  dist_to_move;
	//Vector move_dir;

	bonusTable.Empty();	
	
	if (EditMode_IsOn()) SetUpdate(&Chest::EditMode_Work,ML_EXECUTE1);
}

void Chest::BeginQuickEvent()
{	
	player->physics->Orient(position);

	if (player->animation) player->animation->Goto("Idle",0.0f);
	player->physics->Activate(false);	

	player->logic->SetPairMode(true);

	QuickEvent::BeginQuickEvent();
}

void Chest::QuickEventUpdate(float dltTime)
{
	if (MovePlayer(dltTime))
	{		
		if (player->animation) player->animation->Goto(startNode.c_str(),0.2f);
		ActivateLink("open");
	}	
}

void Chest::Activate(bool isActive)
{
	if (isActive && !statIsActivated && !EditMode_IsOn())
	{
		if (player)
			player->arbiter->StatsTotalChestAdd();
		statIsActivated = true;
	}

	QuickEvent::Activate(isActive);
}

void Chest::Restart()
{	
	/*if (statWasOpened && player)
	{
		statWasOpened = false;
		player->arbiter->StatsOpenedChestAdd(-1.0f);
	}*/

	QuickEvent::Restart();
}

void Chest::Interupt(bool win_game)
{	
	if (!bQuickEventEnabled)
		return;

	player->logic->SetPairMode(false);

	if (win_game)
	{
		if (player) 
		{
			if (!statWasOpened)
			{
				statWasOpened = true;
				player->arbiter->StatsOpenedChestAdd(1.0f);
				//ACHIEVEMENT_REACHED(player, LOOTER);
			}
		}

		LogicDebug("Chest Success");
		triger_Success.Activate(Mission(),false);		
		
		Activate(false);

		BonusesManager::DropParams params;

		params.minVy = pattern->dropMinVy;
		params.maxVy = pattern->dropMaxVy;
		params.minVxz = pattern->dropMinVxz;
		params.maxVxz = pattern->dropMaxVxz;

		params.skipLifeTime = 0.0f;
		
		params.minAy = dropAngle + pattern->dropMinAngle;
		params.maxAy = dropAngle + pattern->dropMaxAngle;

		params.friction = pattern->friction;

		BonusesManager::CreateBonus(Mission(),dropPosition,bonusTable,&params);
		//BonusesManager::CreateBonus(Mission(),position,bonusTable);

		wasCompleted = true;
	}
	else
	{
		LogicDebug("Chest Interrupt Triger Activated");
		triger_Interrupt.Activate(Mission(),false);

		player->animation->ActivateLink("idle",true);
		ActivateLink("idle");
	}
		
	player->physics->Activate(true);
	//bQuickEventEnabled = false;

	QuickEvent::Interupt(win_game);	
}

void Chest::AnimEvent(const char * param)
{
	if (!bQuickEventEnabled) return;	

	if (param[0] == 'f')  //"finish"
	{
		Interupt(true);
	}
}

void Chest::SaveState()
{
	allowRestart = !wasCompleted;
}

bool Chest::AllowRestart()
{
	return allowRestart;
}

const char * Chest::comment = "Chest Object. Set Player Start Node.";

MOP_BEGINLISTG(Chest, "Chest Object", '1.00', 150, "Quick Events")
		
	//MOP_QE_STD
	//MOP_POSITION("Local Position", Vector(0.0f))
	//MOP_ANGLES("Local Angels", Vector(0.0f))
	//MOP_FLOATEX("Player distance", 1.5f,0.1f,1000000.0f)
	//MOP_STRING("Model", "")
	//MOP_STRING("Animation", "")
	//MOP_BOOL("Dynamic Lighting", true)
	//MOP_STRING("Bonus Table", "")

	MOP_STRING("Pattern", "")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angels", Vector(0.0f))
	MOP_FLOAT("Particle show radius", 5.0f)
	MOP_STRING("Bonus Table", "")
	MOP_BOOLC("Active", true, "Active QuickEvent in start mission time")
	MOP_BOOLC("Visible", true, "Show QuickEvent in start mission time")
	MOP_BOOLC("Debug draw", false, "Show debug info")
	MOP_MISSIONTRIGGERG("Accept trigger", "Accept")	
	MOP_MISSIONTRIGGERG("Enter trigger", "Enter")
	MOP_MISSIONTRIGGERG("Exit trigger", "Exit")
	MOP_MISSIONTRIGGERG("Interrupt trigger", "Interrupt")
	MOP_MISSIONTRIGGERG("Success trigger", "Success")

MOP_ENDLIST(Chest)


