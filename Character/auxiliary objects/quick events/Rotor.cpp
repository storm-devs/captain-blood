
#include "Rotor.h"
#include "..\..\character\character.h"
#include "..\..\character\components\characterlogic.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"

bool Rotor::EditMode_Update(MOPReader & reader)
{	
	player = (Character*)Mission().Player();
	SetPlayerAnimEvents();

	ReadStdParams(reader);	

	transform.pos += reader.Position();
	pos = transform.pos;
	
	IGMXScene* old_model = model;
	model = Geometry().CreateGMX( reader.String() , &Animation(), &Particles(), &Sound());	
	RELEASE(old_model);		
	
	bool isDynamicLight = reader.Bool();

	if (model)
	{		
		model->SetDynamicLightState(isDynamicLight);		

		const Vector & vMin = model->GetBound().vMin;
		const Vector & vMax = model->GetBound().vMax;

		Vector size = (vMax - vMin) * 0.5f;
		center_matrix.pos = - vMin - size;
		center_matrix = center_matrix;	
	}
	
	transform = center_matrix;
	transform.pos += pos;

	fTimeToSpin = reader.Float();
	fDamageRadius = reader.Float();

	angle = 0.0f;

	return true;
}

void Rotor::BeginQuickEvent()
{
	fCurTimeToSpin = fTimeToSpin;
	
	angle_speed = 0.0f;

	begin_spin = false;

	player->physics->Orient(pos);
	if (player->animation) player->animation->Goto(startNode,0.0f);

	QuickEvent::BeginQuickEvent();
}

void Rotor::QuickEventUpdate(float dltTime)
{
	if (!begin_spin)
	{
		player->physics->Orient(pos);

		return;
	}

	fCurTimeToSpin -= dltTime;	

	if (fCurTimeToSpin<0.0f)
	{
		Interupt(true);
	}

	if (fCurTimeToSpin>fTimeToSpin-0.75f)
	{
		angle_speed = (fTimeToSpin - fCurTimeToSpin)/0.75f * 10.0f;
	}
	else
	if (fCurTimeToSpin>fTimeToSpin-3.0f)
	{
		angle_speed = 10.0f;
	}
	else
	{
		angle_speed = fCurTimeToSpin/(fTimeToSpin-3.0f) * 10.0f;
	}

	angle += dltTime * angle_speed;	

	angle = angle - (int)(angle /PI * 0.5f) * PI * 2.0f;

	if (angle_speed>1.0f)
	{
		player->arbiter->SplashDamage(player,pos,fDamageRadius,30,false);
	}

	transform = center_matrix * Matrix().RotateY(angle);
	transform.pos += pos;
}

void Rotor::Interupt(bool win_game)
{	
	if (!bQuickEventEnabled)
		return;

	LogicDebug("Rotor Success Triger Activated");
	triger_Success.Activate(Mission(),false);						
		
	//bQuickEventEnabled = false;

	QuickEvent::Interupt(win_game);	
}

void Rotor::AnimEvent(const char * param)
{
	if (!bQuickEventEnabled) return;	
	
	if (param[0] == 'b')  //"finish"
	{
		begin_spin = true;		
	}

	QuickEvent::Interupt(false);
}

static const char * comment;
const char * Rotor::comment = "Rotor Object. Set Player Start Node.";

MOP_BEGINLISTG(Rotor, "Rotor Object", '1.00', 150, "Quick Events")
		
	MOP_QE_STD	

	MOP_POSITION("Local Position", Vector(0.0f))	

	MOP_STRING("Model", "")	
	MOP_BOOL("Dynamic Lighting", true)		

	MOP_FLOAT("Time to spin",7.0f)
	MOP_FLOAT("Damage Radius",1.0f)
	

MOP_ENDLIST(Rotor)