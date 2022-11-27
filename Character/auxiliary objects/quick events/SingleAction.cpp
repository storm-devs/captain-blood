
#include "SingleAction.h"
#include "..\..\character\character.h"
#include "..\..\character\components\characterlogic.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"

bool SingleAction::EditMode_Update(MOPReader & reader)
{	
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
	
	if (anim)
	{
		events.DelAnimation(anim);
		anim->Release();
		anim = null;
	}

	if (model)
	{
		model->SetAnimationFile(anim_str);
		model->SetDynamicLightState(true);
		anim = model->GetAnimation();	

		if (anim)
		{
			events.AddAnimation(anim);
		}
	}

	only_once = reader.Bool();
	
	orientpoint = transform.vz;

	transform = Matrix(angels,loc_position) * transform;	

	return true;
}

void SingleAction::BeginQuickEvent()
{			
	player->physics->Activate(false);	
	if (player->animation) player->animation->Goto("idle",0.2f);	

	need_act = false;

	QuickEvent::BeginQuickEvent();
}

void SingleAction::QuickEventUpdate(float dltTime)
{
	if (MovePlayer(dltTime))
	{	
		LogicDebug("%s Accept Triger Activated", GetObjectType());
		triger_Accept.Activate(Mission(), false);

		SetPlayerAnimEvents();

		if (player->animation) player->animation->Goto(startNode.c_str(),0.0f);			
		ActivateLink("Action");				
	}

	player->physics->Orient(player->physics->GetPos() + orientpoint);	
}

void SingleAction::Interupt(bool win_game)
{	
	if (!bQuickEventEnabled)
		return;

	if (win_game)
	{
		LogicDebug("Single Action Success Triger Activated");
		triger_Success.Activate(Mission(),false);
		if (only_once) wasCompleted = true;
	}
		
	if (wasCompleted)
	{
		Activate(false);
	}

	//bQuickEventEnabled = false;

	//Vector pos = player->physics->GetPos();
	Vector pos;
	//player->GetPosFromRootBone(pos);
	//player->physics->SetPos(pos);
	player->physics->Activate(true);

	QuickEvent::Interupt(win_game);	
}

void SingleAction::AnimEvent(const char * param)
{
	if (!bQuickEventEnabled) return;	
	
	if (param[0] == 'f')  //"finish"
	{
		Interupt(true);
	}
}

static const char * comment;
const char * SingleAction::comment = "Single Action. Set Player Start Node.";

MOP_BEGINLISTG(SingleAction, "Single Action Object", '1.00', 150, "Quick Events")
		
	MOP_QE_STD	

	MOP_POSITION("Local Position", Vector(0.0f))
	MOP_ANGLES("Local Angels", Vector(0.0f))

	MOP_FLOATEX("Player distance", 0.0f,0.0f,10000.0f)

	MOP_STRING("Model", "")
	MOP_STRING("Animation", "")

	MOP_BOOL("One Time Action", false)

MOP_ENDLIST(SingleAction)