
#include "QuickEvent.h"
#include "..\..\character\components\characterlogic.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"
#include "..\..\player\playerController.h"

QuickEvent::QuickEvent()
{
	quick_anim_events = NEW AnimListener();
	quick_anim_events->Init(this);

	inZone = false;
	inParticleZone = false;
	bQuickEventEnabled = false;

	position = 0.0f;
	radius = 3.0f;
	player_point = 0.0f;

	buttonName[0] = 0;
	buttonCode = -1;

	init_pos = 0.0f;
	init_angels = 0.0f;
		
	particle = null;
	particle_loc_pos = 0.0f;
	particle_loc_angels = 0.0f;
	particle_radius = 5.0f;		
	
	player = null;

	model = null;
	anim = null;

	shd_cast = false;
	shd_rcv = false;

	show_model = true;

	dist_to_move = 0.0f;
	move_dir = 0.0f;

	wasCompleted = false;

	button_tip = null;

	pl_has_sec_wp = false;

	need_act = true;
}

QuickEvent::~QuickEvent()
{
	DELETE(quick_anim_events);
	Release();
}

void QuickEvent::Release()
{
	if (anim)
		events.DelAnimation(anim);

	RELEASE(anim);
	RELEASE(model);
	RELEASE(particle);
	RELEASE(button_tip);

	show_model = true;
}

void QuickEvent::Activate(bool isActive)
{
	if (wasCompleted && isActive) return;

	if (!EditMode_IsOn())
	{
		if (button_tip) button_tip->Activate(isActive);

		if (isActive)
		{
			SetUpdate(&QuickEvent::Work, ML_EXECUTE1);
			if (button_tip) button_tip->SetAlpha(1.0f);
		}
		else
		{
			DelUpdate(&QuickEvent::Work);
			Unregistry(MG_ACTIVEACCEPTOR);
			if (button_tip) button_tip->SetAlpha(0.0f);
		}
	}

	if (!isActive)
	{		
		HideHUD();
	}

	MissionObject::Activate(isActive);
}

void QuickEvent::Show(bool isShow)
{
	AcceptorObject::Show(isShow);

	if (anim) anim->Pause(!isShow);

	if (isShow)
	{
		SetUpdate(&QuickEvent::Draw, ML_GEOMETRY5);

		if (EditMode_IsOn())
		{	
			SetUpdate(&QuickEvent::EditMode_Draw, ML_ALPHA3);
		}

		if (shd_cast)
		{
			Registry(MG_SHADOWCAST, &QuickEvent::ShadowInfo, ML_GEOMETRY5);
		}
		else
		{
			Unregistry(MG_SHADOWCAST);
		}
		
		if (shd_rcv)
		{
			Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&QuickEvent::ShadowDraw, ML_GEOMETRY5);
		}
		else
		{
			Unregistry(MG_SHADOWRECEIVE);
		}

		//PlayHintParticle(true);
		/*if (button_tip && IsActive())
		{
			button_tip->Activate(true);
		}
		else
		{
			if (button_tip) button_tip->Activate(false);
		}*/
	}
	else
	{
		DelUpdate(&QuickEvent::Draw);
		DelUpdate(&QuickEvent::EditMode_Draw);

		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);
		
		HideHUD();
	}

	ActivateColision(isShow);
}

bool QuickEvent::ReadStdParams(MOPReader & reader)
{	
	position = reader.Position();
	Vector angels = reader.Angles();
	radius = reader.Float();	

	transform = Matrix(angels,position);

	player_point = position;		

	startNode = reader.String();	

	crt_strcpy(buttonName, sizeof(buttonName), reader.String().c_str());
	crt_strcat(buttonName, sizeof(buttonName), "1");

	buttonCode = Controls().FindControlByName(buttonName);

	cheatButton = Controls().FindControlByName("ChrGotoHell1");

	colider.cld_loc_pos = reader.Position();
	colider.cld_loc_angels = reader.Angles();
	colider.cld_size.x = reader.Float() * 0.5f;
	colider.cld_size.y = reader.Float() * 0.5f;
	colider.cld_size.z = reader.Float() * 0.5f;
	
	colider.colider_matrix = Matrix(colider.cld_loc_angels,colider.cld_loc_pos) * transform;

	if (!EditMode_IsOn())
	{
		colider.Init(&Physics(),this);
	}

	particle_name = reader.String();	
	particle_loc_pos = reader.Position();
	particle_loc_angels = reader.Angles();
	particle_radius = reader.Float();
		
	InitHintParticle();

	tip_name = reader.String();

	if (!EditMode_IsOn())
	{
		ITipsManager* tip_manager = ITipsManager::GetManager(&Mission());

		if (tip_manager)
		{
			button_tip = tip_manager->CreateTip(tip_name,0.0f,this);			

			if (button_tip)
			{
				button_tip->Activate(false);			
				button_tip->SetPos(transform.pos);
			}
		}	
	}

	shd_cast = reader.Bool();
	shd_rcv = reader.Bool();

	Activate(reader.Bool());
	Show(reader.Bool());
	
	triger_Accept.Init(reader);	
	triger_Enter.Init(reader);
	triger_Exit.Init(reader);
	triger_Interrupt.Init(reader);
	triger_Success.Init(reader);

	Registry(MG_QE);

	return true;
}

void QuickEvent::SetPlayerAnimEvents()
{
	if (player && quick_anim_events)
	{		
		player->animation->SetEventHandler(quick_anim_events, (AniEvent)(&AnimListener::AnimEvent), "QuickEvent");		
	}
}

void QuickEvent::RemovePlayerAnimEvents()
{
	if (player && quick_anim_events)
	{		
		player->animation->DelEventHandler(quick_anim_events, (AniEvent)(&AnimListener::AnimEvent), "QuickEvent");		
	}
}

void QuickEvent::ShowObject(const ConstString & object_name,bool show)
{	
	MOSafePointer object;
	FindObject(object_name,object);
	
	if (object.Ptr())
	{
		if (object.Ptr()->IsShow() == show) return;

		object.Ptr()->Show(show);

		if (show)
		{
			LogicDebug("Quck Event - %s was show %s",GetObjectID().c_str(),object_name.c_str());
		}
		else
		{
			LogicDebug("Quck Event - %s was hide %s",GetObjectID().c_str(),object_name.c_str());
		}
	}
}

void QuickEvent::InitData()
{

}

void QuickEvent::Restart()
{	
	wasCompleted = false;

	DelUpdate(&QuickEvent::Work);
	DelUpdate(&QuickEvent::Draw);
	Unregistry(MG_ACTIVEACCEPTOR);

	colider.DeleteColider();

	player = NULL;

	inZone = false;
	inParticleZone = false;

	bQuickEventEnabled = false;	

	DELETE(quick_anim_events);
	quick_anim_events = NEW AnimListener();
	quick_anim_events->Init(this);

	triger_Accept.Reset();
	triger_Enter.Reset();
	triger_Exit.Reset();
	triger_Interrupt.Reset();		
	triger_Success.Reset();	

	InitData();
	ReCreate();
}

bool QuickEvent::Create(MOPReader & reader)
{
	InitData();	
	EditMode_Update (reader);	

	return true;
}

void QuickEvent::AnimEvent(const char * param)
{	
}

void QuickEvent::HideHUD()
{
	PlayHintParticle(false);		
	if (button_tip) button_tip->Activate(false);
}

void QuickEvent::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min =-radius;// + position;
	max = radius;// + position;
}

void _cdecl QuickEvent::EditMode_Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	
	Render().DrawSphere(player_point,0.25f,0xffffaaff);
	Render().DrawSphere(particle_matrix.pos,0.25f,0xffaa0000);
	Render().DrawSolidBox(-colider.cld_size, colider.cld_size, colider.colider_matrix, 0xaa00ff00);
	Render().DrawSphere(position,radius,0xffaaccff);		
}

void _cdecl QuickEvent::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;

	if (model && show_model)
	{
		model->SetTransform(transform);
		model->Draw();
	}
}

//Получить бокс, описывающий объект
void QuickEvent::GetBox(Vector & min, Vector & max)
{
	if(model)
	{
		model->SetTransform(transform);
		min = model->GetLocalBound().vMin;
		max = model->GetLocalBound().vMax;
	}
	else
	{
		min = 0.01f;
		max = 0.01f;
	}
}

Matrix & QuickEvent::GetMatrix(Matrix & mtx)
{	
	mtx = transform;		
	return mtx;
}

//Нарисовать модельку персонажа для тени
void _cdecl QuickEvent::ShadowInfo(const char * group, MissionObject * sender)
{

	if(sender && model)
	{
		model->SetTransform(transform);
		const Vector & vMin = model->GetBound().vMin;
		const Vector & vMax = model->GetBound().vMax;
		((MissionShadowCaster *)sender)->AddObject(this, &QuickEvent::ShadowDraw, vMin, vMax);
	}
}

//Нарисовать модельку персонажа для тени
void _cdecl QuickEvent::ShadowDraw(const char * group, MissionObject * sender)
{	
	if (model)
	{
		model->SetTransform(transform);
		model->Draw();
	}
}

void _cdecl QuickEvent::Work(float dltTime, long level)
{
	events.Update(transform, dltTime);

	QuickEventCheck();

	if (bQuickEventEnabled)
	{
		QuickEventUpdate(dltTime);
		return;
	}

	if (!player)
	{
		player = (Character*)Mission().Player();

		if (!player) return;
	}
	
	float pl_dist = (player->GetMatrix(Matrix()).pos - position).GetLength();

	if (pl_dist<radius && player->IsActive())
	{
		if (!inZone)
		{
			Registry(MG_ACTIVEACCEPTOR);

			LogicDebug("%s Enter Triger Activated", GetObjectType());
			triger_Enter.Activate(Mission(),false);
			inZone = true;
			PlayHintParticle(true);			
		}

		if (button_tip&&player)
		{
			if (((PlayerController*)player->controller)->AllowAccpet()) button_tip->SetState(ITip::active);
			if (button_tip) button_tip->Activate(true);
		}
	}
	else
	{
		if (inZone)
		{
			Unregistry(MG_ACTIVEACCEPTOR);

			LogicDebug("%s Exit Triger Activated", GetObjectType());
			triger_Exit.Activate(Mission(),false);
			inZone = false;

			PlayHintParticle(false);
			if (button_tip) button_tip->Activate(false);
		}

		if (button_tip&&player)
		{			
			if (button_tip) button_tip->Activate(false);
		}
	}



	if (pl_dist<particle_radius && player->IsActive())
	{
		if (!inParticleZone)
		{			
			inParticleZone = true;
			PlayHintParticle(true);			
		}
	}
	else
	{
		if (inParticleZone)
		{			
			inParticleZone = false;
			PlayHintParticle(false);			
		}
	}
}

bool QuickEvent::Accept(MissionObject * obj)
{
	if (!player) return false;
	if(obj != player) return false;
	if (bQuickEventEnabled) return false;

	((PlayerController*)player->controller)->qevent = this;
	((PlayerController*)player->controller)->HideWidgets();
	
	if (((PlayerController*)player->controller)->itemTaken)
	{
		player->animation->SetAnimation("base",0.0f);
	}

	BeginQuickEvent();

	if (need_act)
	{
		LogicDebug("%s Accept Triger Activated", GetObjectType());
		triger_Accept.Activate(Mission(), false);
	}

	SetPlayerAnimEvents();	

	bQuickEventEnabled = true;	

	return true;	
}

void QuickEvent::ActivateColision(bool isAct)
{
	colider.Activate(isAct);
}

void QuickEvent::InitHintParticle()
{
	particle_matrix = Matrix(particle_loc_angels,particle_loc_pos) * transform;

	particle = Particles().CreateParticleSystem(particle_name.c_str());

	if (particle)
	{		
		particle->Teleport(particle_matrix);
		particle->AutoDelete(false);
		particle->PauseEmission(true);
	}
}

void QuickEvent::PlayHintParticle(bool start)
{
	if (!particle) return;

	if (start)
	{
		if (inParticleZone)
		{
			particle->PauseEmission(false);			
		}
	}
	else
	{
		particle->PauseEmission(true);		
	}
}

void QuickEvent::BeginQuickEvent()
{
	pl_has_sec_wp = ((PlayerController*)player->controller)->itemTaken;

	move_dir = player->physics->GetPos() - player_point;
	dist_to_move = move_dir.Normalize();

	PlayHintParticle(false);
	if (button_tip) button_tip->Activate(false);
}

bool QuickEvent::MovePlayer(float dltTime)
{	
	if (dist_to_move>0)
	{		
		dist_to_move -= dltTime * 4.5f;

		if (dist_to_move<0)
		{
			dist_to_move = 0.0f;			
		}		

		player->physics->SetPos(player_point + move_dir * dist_to_move);
		player->physics->Orient(position);

		if (dist_to_move<=0.0f)
		{
			return true;
		}
	}	

	return false;

}

void QuickEvent::QuickEventUpdate(float dltTime)
{
	
}

bool QuickEvent::AllowInterruptByButton()
{
	return false;
}

void QuickEvent::Interupt(bool win_game)
{
	if (!bQuickEventEnabled)
		return;

	if (player)
	{	
		if (((PlayerController*)player->controller)->qevent)
		{
			((PlayerController*)player->controller)->qevent = null;
		}

		if (pl_has_sec_wp) ((PlayerController*)player->controller)->RestoreSecodWeapon();			
	}

	if (!win_game)
	{
		PlayHintParticle(true);
		if (button_tip) button_tip->Activate(true);
	}	

	RemovePlayerAnimEvents();

	bQuickEventEnabled = false;
}

void QuickEvent::Command(const char * id, dword numParams, const char ** params)
{
	if (!id) return;

	if (numParams<1) return;

	if (string::IsEqual(id,"show_model"))
	{		
		show_model = false;

		if (params[0][0] == 'y' || params[0][0] == 'Y')
		{
			show_model = true;
		}
	}
}