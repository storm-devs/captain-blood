/*
#include "Milagrosa Minigame.h"


//Конструктор
MilagrosaMinigame::MilagrosaMinigame():buttonMG(NULL)
{
	animListener.pMilagrosaMinigame = this;

	IsTargetAttack = false;
	timetoattack = 0;

	NumPlayerHits = 0;
	NumBossHits = 0;	

	deltatime = 0.0f;

	button_times = 10;

	bFightMiniGame = false;
}

//Деструктор
MilagrosaMinigame::~MilagrosaMinigame()
{		
}

void MilagrosaMinigame::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
//		Registry(MG_DOF);
		Registry(MG_SHADOWCAST, &MilagrosaMinigame::ShadowInfo, ML_ALPHA1);
		Registry(MG_SHADOWRECEIVE, &MilagrosaMinigame::ShadowDraw, ML_ALPHA1);

		if (boss.animation)
		{
			boss.animation->Start();
		}

		if (player.animation)
		{
			player.animation->Start();
		}
	}
	else
	{
//		Unregistry(MG_DOF);
		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);
	}
		
}

void MilagrosaMinigame::UpdateBar(TCharacter* chr)
{
	MOSafePointer mo;
	FindObject(ConstString(chr->progress_bar),mo);

	if (mo.Ptr())
	{
		const char* params[2];

		char str[64];
		sprintf_s(str,sizeof(str),"%i", chr->MaxHP);

		params[0] = str;

		mo.Ptr()->Command("SetMax",1,params);
		mo.Ptr()->Command("SetHP",1,params);				
	}
}

bool MilagrosaMinigame::IfCanStartFight()
{
	charstate player_state = GetState(&player);
	charstate boss_state = GetState(&boss);

	if (((player_state == chr_upattack && boss_state == chr_upattack) || 
		(player_state == chr_downattack && boss_state == chr_downattack)) && Rnd()>0.35f)
	{		
		return true;
	}

	return false;
}

void MilagrosaMinigame::Restart()
{
	ReCreate();
}

void MilagrosaMinigame::LoadModel(TCharacter* chr,char* model,char* animation,char* weapon)
{
	IGMXScene* old_model = chr->model;	
	chr->model = Geometry().CreateGMX( model , &Animation(), &Particles(), &Sound());
	RELEASE(old_model);

	if (chr->model)
	{
		chr->model->SetDynamicLightState(true);

		chr->animation = Animation().Create(animation,_FL_);

		if (chr->animation)
		{
			chr->model->SetAnimation(chr->animation);
		}
		else
		{
			api->Trace("Can't Load '%s' animation ", animation);
		}

		chr->item = Geometry().CreateGMX( weapon , &Animation(), &Particles(), &Sound());

		if (!chr->item)
		{
			api->Trace("Can't Load '%s' model ",weapon);
		}
		else
		{
			chr->item->SetDynamicLightState(true);
		}
	}
	else
	{
		api->Trace("Can't Load '%s' model ",model);
	}
}

//Создание объекта
bool MilagrosaMinigame::Create(MOPReader & reader)
{
	widgetFight.Reset();
	widgetUp.Reset();
	widgetDown.Reset();	

	bFightMiniGame = false;

	timeto_show_Wgt = 0.0f;

	buttonMG.Init();

	EditMode_Update (reader);
	
	LoadModel(&player,"jpitt.gmx","pittmast.anx","Weapons//Blade_Pitt.gmx");
	LoadModel(&boss,"matilda.gmx","milagrossacaptain.anx","Matilda_weapon.gmx");	

	if (player.model && player.animation && boss.model && boss.animation)
	{			
		if (EditMode_IsOn())
		{
			SetUpdate((MOF_UPDATE)&MilagrosaMinigame::EditMode_Draw, ML_ALPHA1);
		}
		else
		{
			SetUpdate((MOF_UPDATE)&MilagrosaMinigame::Realize, ML_ALPHA1);

			if (player.animation)
			{			
				player.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::AnimEvent), "AI Event");
				player.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::BeginMiniGame), "BeginMiniGame");
				player.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::ChooseAnimLink), "ChooseAnimLink");
			}
	
			if (boss.animation)
			{
				boss.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::AnimEvent), "AI Event");		
			}
			
			UpdateBar(&player);
			UpdateBar(&boss);
		}
	}	


	return true;
}


//Обновление параметров
bool MilagrosaMinigame::EditMode_Update(MOPReader & reader)
{	
	button_duration = reader.Float();
	button_times = reader.Long();

	player.pos = reader.Position();
	player.ay = reader.Angles().y;		
	player.MaxHP = reader.Long();
	player.HP = player.MaxHP;
	player.progress_bar = reader.String().c_str();

	boss.pos = reader.Position();
	boss.ay = reader.Angles().y;
	boss.MaxHP = reader.Long();
	boss.HP = boss.MaxHP;
	boss.progress_bar = reader.String().c_str();

	left_border = reader.Position();
	right_border = reader.Position();

	player.win_triger.Init(reader);
	boss.win_triger.Init(reader);

	widgetFightID = reader.String().c_str();

	widgetUpID = reader.String().c_str();
	widgetDownID = reader.String().c_str();

	maxtimeto_show_Wgt = reader.Float();

	Activate(reader.Bool());

	return true;
}

float MilagrosaMinigame::CalcDistance()
{
	Vector pos = boss.pos - player.pos;

	pos.y = 0.0f;

	pos.Rotate(player.ay);

	return fabs(pos.z);

}

void MilagrosaMinigame::CheckBorder(TCharacter* chr,TCharacter* enemy_chr)
{
	float dist  = (chr->pos-left_border).GetLength();
	float dist2 = (chr->pos-right_border).GetLength();

	if (dist<2.0f || dist2<2.0f)
	{
		chr->HP = 0;

		chr->animation->ActivateLink("Death");
		
		enemy_chr->win_triger.Activate(Mission(),true);
	}
}

void MilagrosaMinigame::MoveCharacter(TCharacter* chr,TCharacter* enemy_chr)
{
	Vector dltPos = 0.0f;
	float dltAng = 0.0f;

	chr->animation->GetMovement(dltPos);	

	dltPos.x=dltPos.y=0.0f;

	dltPos = (dltPos.GetXZ()).Rotate(chr->ay);	

	float dist = (chr->pos+dltPos-enemy_chr->pos).GetLength();

	if (dist < 1.15f && dltPos.GetLength()>0.0f)
	{
		float dist2 = (chr->pos-enemy_chr->pos).GetLength();

		dltPos *= (dist-dist2)* 1.0f / dltPos.GetLength();
	}

	chr->pos += dltPos;	
}

void MilagrosaMinigame::BossThinking(float fDeltaTime)
{	
	if (player.HP<=0)// || buttonMG.IsEnabled())
	{
		return;
	}

	timetoattack -= fDeltaTime;

	if (timetoattack<0)
	{
		timetoattack = 0.0f;
	}	

	if ((boss.pos-player.pos).GetLengthXZ()<=3.5f&&
		GetState(&player) != chr_fall)
	{
		charstate st = GetState(&player);

		float k = Rnd();

		if (st == chr_upattack || st == chr_downattack)
		{
			if (!IsTargetAttack)
			{
				IsTargetAttack = true;

				if (k<0.3f)
				{
					boss.animation->ActivateLink("C");
				}
				else
				if (k<0.6f)
				{	
					boss.animation->ActivateLink("toBlock");
				}				
			}
		}			
		else
		{
			IsTargetAttack = false;

			if (timetoattack<=0.0f)
			{
				if (k<0.4f)
				{
					boss.animation->ActivateLink("A");
				}
				else
				if (k<0.8f)
				{	
					boss.animation->ActivateLink("B");
				}

				timetoattack = 1.5f+Rnd(1.5f);
			}
		}
	}
	else
	{
		boss.animation->ActivateLink("strafe forward");
	}
}

void MilagrosaMinigame::MovePlayer()
{
	Vector dir = 0.0f;	

	//Получим состояние контролов
	dir.x = Controls().GetControlStateFloat("ChrLeftRight1");
	dir.z = Controls().GetControlStateFloat("ChrForwBack1");	


	if(dir.z > 0.5f)
	{
		player.animation->ActivateLink("jump",true);					
	}
	else
	if(dir.z < -0.5f)
	{
		player.animation->ActivateLink("sit",true);
	}
	else
	if(dir.x > 0.5f)
	{
		player.animation->ActivateLink("strafe forward",true);
	}
	else
	if(dir.x < -0.5f)
	{
		player.animation->ActivateLink("strafe back",true);
	}
	else
	{
		player.animation->ActivateLink("from block",true);
	}
}

void MilagrosaMinigame::StartFightMiniGame()
{
	fight_phase = 2;
	num_pressed = (int)((float)button_times * 0.5f);
	
	button_time = button_duration;

	player.animation->ActivateLink("fight");
	boss.animation->ActivateLink("fight");

	bFightMiniGame = true;	

	float dist = CalcDistance();
	float delta = (dist - 0.65f) * 0.5f;	

	boss.delta = delta;
	player.delta = delta;

	if (!widgetFight.Validate())
	{	
		FindObject(ConstString(widgetFightID),widgetFight);
	}

	if (widgetFight.Ptr())
	{
		widgetFight.Ptr()->Show(true);
	}
}

void MilagrosaMinigame::FightMiniGame(float fDeltaTime)
{
	button_time -= fDeltaTime;

	//while (button_time<0)
	if (button_time<0)
	{
		button_time=button_duration;

		num_pressed--;

		if (num_pressed<0)
		{
			num_pressed = button_times;

			fight_phase--;

			if (fight_phase<0)
			{
				fight_phase = 0;
				num_pressed = 0;

				player.animation->ActivateLink("hit");
				boss.animation->ActivateLink("idle");				

				if (widgetFight.Ptr())
				{
					widgetFight.Ptr()->Show(false);
				}

				bFightMiniGame = false;
				HitCharater(&player, &boss);
				
				if (player.HP<=0)
				{
					return;
				}
			}
			else
			{			
				player.animation->ActivateLink("left");
				boss.animation->ActivateLink("left");
			}
		}
	}

	if (Controls().GetControlStateType("ChrA1") == CST_ACTIVATED||
		Controls().GetControlStateType("ChrB1") == CST_ACTIVATED||
		Controls().GetControlStateType("ChrC1") == CST_ACTIVATED||
		Controls().GetControlStateType("ChrBlock1") == CST_ACTIVATED)
	{
		num_pressed++;

		if (num_pressed>button_times)
		{
			num_pressed = 0;

			fight_phase++;

			if (fight_phase>4)
			{
				fight_phase = 4;
				num_pressed = button_times;

				player.animation->ActivateLink("idle");
				boss.animation->ActivateLink("pain");

				bFightMiniGame = false;

				if (widgetFight.Ptr())
				{
					widgetFight.Ptr()->Show(false);
				}

				HitCharater(&boss, &player);

				if (boss.HP<=0)
				{
					return;
				}		
			}
			else
			{			
				player.animation->ActivateLink("right");
				boss.animation->ActivateLink("right");
			}
		}
	}
}

void _cdecl MilagrosaMinigame::Realize(float fDeltaTime, long level)
{	
	if (!IsActive())
	{
		return;
	}	

	UpdateHelpWidgets(fDeltaTime);

	if (bFightMiniGame)
	{
		FightMiniGame(fDeltaTime);

		deltatime+=fDeltaTime * 3.0f;
		
		if (deltatime>1.0f)
		{
			deltatime = 1.0f;
		}

		CheckBorder(&player,&boss);
		CheckBorder(&boss,&player);
	}
	else
	{
		deltatime-=fDeltaTime * 3.0f;
		
		if (deltatime<0.0f)
		{
			deltatime = 0.0f;
		}

		if (player.HP>0 && boss.HP>0)
		{
			MovePlayer();
		
			buttonMG.Update(fDeltaTime,NULL,NULL);			

			if(Controls().GetControlStateType("ChrA1") == CST_ACTIVATED)
			{
				player.animation->ActivateLink("A");
			}
	
			if(Controls().GetControlStateType("ChrB1") == CST_ACTIVATED)
			{
				player.animation->ActivateLink("B");
			}
	
			BossThinking(fDeltaTime);
		}

		MoveCharacter(&player,&boss);
		MoveCharacter(&boss,&player);

		if (player.HP>0 && boss.HP>0)
		{
			CheckBorder(&player,&boss);
			CheckBorder(&boss,&player);
		}
	}	

	DrawCharacter(&player);
	DrawItem(&player,"righthand");

	DrawCharacter(&boss);
	DrawItem(&boss,"saber");
}

void _cdecl MilagrosaMinigame::EditMode_Draw(float fDeltaTime, long level)
{
	if(!EditMode_IsVisible()) return;

	DrawCharacter(&player);
	DrawItem(&player,"righthand");

	DrawCharacter(&boss);
	DrawItem(&boss,"saber");

	Render().DrawSphere(left_border,1.0f,0xffff00ff);
	Render().DrawSphere(right_border,1.0f,0xffff00ff);	
}

void MilagrosaMinigame::DrawCharacter(TCharacter* chr)
{
	if (!chr->model) return;

	Matrix mat;
	mat.Build(Vector(0.0f, chr->ay, 0.0f), chr->pos + Vector(0,0,chr->delta * deltatime).Rotate(chr->ay));

	chr->model->SetTransform(mat);
	chr->model->Draw();	
}

//Нарисовать итемы
void MilagrosaMinigame::DrawItem(TCharacter* chr, const char* locator)
{
	if (!chr->item) return;

	IGMXEntity * loc = chr->model->FindEntity(GMXET_LOCATOR, locator);

	if (loc)
	{					
		Matrix mtx = loc->GetLocalTransform();

		Matrix mat;		
		mat.Build(Vector(0.0f, chr->ay, 0.0f), chr->pos + Vector(0,0,chr->delta * deltatime).Rotate(chr->ay));		

		chr->item->SetTransform(loc->GetLocalTransform()*mat);
		chr->item->Draw();
	}	
}

Matrix & MilagrosaMinigame::GetMatrix(Matrix & mtx)
{
	mtx.Build(Vector(0.0f, player.ay, 0.0f), player.pos);

	return mtx;
}

void MilagrosaMinigame::HitCharater(TCharacter* chr, TCharacter* enemy_chr)
{
	chr->HP--;

	MOSafePointer mo;
	FindObject(ConstString(chr->progress_bar),mo);

	if (mo.Ptr())
	{		
		mo.Ptr()->Command("dec",0,NULL);
	}

	FindObject(ConstString("camera controller"),mo);	

	if (mo.Ptr())
	{		
		mo.Ptr()->Command("shockPolar1",0,NULL);
	}

	if (chr->HP<=0)
	{
		chr->animation->ActivateLink("Death");

		enemy_chr->win_triger.Activate(Mission(),true);

		return;
	}		
}

void MilagrosaMinigame::PlayerHit()
{
	if (player.HP<=0) return;

	float dist = (player.pos-boss.pos).GetLengthXZ2();

	if (dist < 4)
	{
		if (IfCanStartFight()) StartFightMiniGame();

		charstate player_state = GetState(&player);
		charstate boss_state = GetState(&boss);
		
		HitCharater(&boss, &player);

		if (boss.HP<0) return;

		NumBossHits++;

		if (NumBossHits>2)
		{
			NumBossHits = 0;

			boss.animation->ActivateLink("Trance");
		}
		else
		{
			if (boss_state==chr_weak)
			{
				NumBossHits = 0;

				boss.animation->ActivateLink("Strong Pain");
			}
			else
			{
				if (Rnd()>0.5f)
				{		
					boss.animation->ActivateLink("Pain");
				}
				else
				{
					boss.animation->ActivateLink("Hit");
				}
			}
		}
	}
}

void MilagrosaMinigame::BossHit()
{	
	if (boss.HP<=0) return;

	float dist = (player.pos-boss.pos).GetLengthXZ2();

	if (dist < 4)
	{
		if (IfCanStartFight()) StartFightMiniGame();

		charstate player_state = GetState(&player);
		charstate boss_state = GetState(&boss);

		if ((player_state == chr_duck && boss_state == chr_upattack) || 
			(player_state == chr_jump && boss_state == chr_downattack)) return;
		
		HitCharater(&player,&boss);

		if (player.HP<0) return;

		NumPlayerHits++;

		if (NumPlayerHits>2)
		{
			NumPlayerHits = 0;

			player.animation->ActivateLink("Fall");			
		}
		else
		{
			player.animation->ActivateLink("Hit");
		}
	}
}

//Доступиться до константы типа string
const char * MilagrosaMinigame::GetConstString(TCharacter* chr,const char * constName)
{
	if(chr->animation)
	{
		return chr->animation->GetConstString(constName);
	}	

	return null;
}

//Получить состояние персонажа из анимации
MilagrosaMinigame::charstate MilagrosaMinigame::GetState(TCharacter* chr)
{
	const char * st = GetConstString(chr,"Milagrosa state");
	if(!st) return chr_idle;
	switch(st[0])
	{				
		case 'w':
			if(st[1] == 'e') return chr_weak;
			return chr_walk;
		case 'i':
			return chr_idle;
		case 'u':		
			return chr_upattack;	
		case 'd':
			if(st[1] == 'o') return chr_downattack;
			return chr_duck;
		case 'h':
			return chr_hit;
		case 'j':		
			return chr_jump;
		case 'p':
			return chr_parish;	
		case 'f':
			if(st[1] == 'a') return chr_fall;
			return chr_fight;				
		default:
			return chr_idle;
	}
}


//Нарисовать модельку персонажа для тени
void _cdecl MilagrosaMinigame::ShadowInfo(const char * group, MissionObject * sender)
{
	if(sender)
	{		
		const Vector & vMin = Vector(coremin(player.pos.x,boss.pos.x)-1.5f,
									 coremin(player.pos.y,boss.pos.y)-1.5f,
									 coremin(player.pos.z,boss.pos.z)-1.5f);
		
		const Vector & vMax = Vector(coremax(player.pos.x,boss.pos.x)+1.5f,
									 coremax(player.pos.y,boss.pos.y)+1.5f,
									 coremax(player.pos.z,boss.pos.z)+1.5f);

		((MissionShadowCaster *)sender)->AddObject(this, &MilagrosaMinigame::ShadowDraw, vMin, vMax);
	}
}

//Нарисовать модельку персонажа для тени
void _cdecl MilagrosaMinigame::ShadowDraw(const char * group, MissionObject * sender)
{
	DrawCharacter(&player);
	DrawItem(&player,"righthand");

	DrawCharacter(&boss);
	DrawItem(&boss,"saber");
}

void MilagrosaMinigame::UpdateHelpWidgets(float fDeltaTime)
{
	if (timeto_show_Wgt>0.0f)
	{
		timeto_show_Wgt-=fDeltaTime;

		if (timeto_show_Wgt<0)
		{
			timeto_show_Wgt = 0;

			ShowHelpWidgets(false);
		}
	}
}

void MilagrosaMinigame::ShowHelpWidgets(bool show)
{
	if (show)
	{
		if (GetState(&boss)==chr_upattack)
		{
			if (!widgetUp.Validate())
			{
				FindObject(ConstString(widgetUpID),widgetUp);
			}

			if (widgetUp.Ptr())
			{
				widgetUp.Ptr()->Show(show);
			}			
		}
		else
		if (GetState(&boss)==chr_downattack)
		{
			if (!widgetDown.Validate())
			{
				FindObject(ConstString(widgetDownID),widgetDown);
			}

			if (widgetDown.Ptr())
			{
				widgetDown.Ptr()->Show(true);
			}			
		}

		timeto_show_Wgt = maxtimeto_show_Wgt;
	}
	else
	{
		if (widgetUp.Ptr())
		{
			widgetUp.Ptr()->Show(false);
		}

		if (widgetDown.Ptr())
		{
			widgetDown.Ptr()->Show(false);
		}
	}

}

const char * MilagrosaMinigame::comment = "";


MOP_BEGINLISTCG(MilagrosaMinigame, "Milagrosa Minigame", '1.00', 200, MilagrosaMinigame::comment, "Character objects")
	MOP_FLOATEX("Button duration", 0.3f,0.1f,2.0f)
	MOP_LONGEX("Button Times", 10, 2, 100)
	MOP_POSITION("PlayerPosition", Vector(0.0f))
	MOP_ANGLESEX("PlayerAngle", Vector(0.0f), Vector(0.0f, -4.0f*PI, 0.0f), Vector(0.0f, 4.0f*PI, 0.0f))
	MOP_LONGEX("Player HP", 10, 1, 10000)
	MOP_STRING("Player ProgressBar", "")
	MOP_POSITION("BossPosition", Vector(0.0f))
	MOP_ANGLESEX("BossAngle", Vector(0.0f), Vector(0.0f, -4.0f*PI, 0.0f), Vector(0.0f, 4.0f*PI, 0.0f))
	MOP_LONGEX("Boss HP", 10, 1, 10000)
	MOP_STRING("Boss ProgressBar", "")
	MOP_POSITION("Left Border", Vector(0.0f))
	MOP_POSITION("Right Border", Vector(0.0f))
	MOP_MISSIONTRIGGER("Player Win")
	MOP_MISSIONTRIGGER("Boss Win")
	MOP_STRING("Fight Widget", "")
	MOP_STRING("Fight Up", "")
	MOP_STRING("Fight Down", "")
	MOP_FLOATEX("Time To show help widget", 1.5f,0.1f,20.0f)
	MOP_BOOL("Active",true)
MOP_ENDLIST(MilagrosaMinigame)



*/