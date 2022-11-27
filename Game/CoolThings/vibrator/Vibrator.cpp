#include "Vibrator.h"

Vibrator:: Vibrator()
{
	force = NULL;

	started = false; deviceIndex = 0;
}

Vibrator::~Vibrator()
{
	if( force )
		force->Release();
}

void Vibrator::Restart()
{
	Activate(false);
	Activate(m_active);
}

bool Vibrator::Create		  (MOPReader &reader)
{
	InitParams(reader);

	Show(true);

	return true;
}

bool Vibrator::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void Vibrator::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&Vibrator::Draw,ML_ALPHA5);
	else
		DelUpdate(&Vibrator::Draw);
}

void Vibrator::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if( force )
	{
		if( isActive )
		{
		/*	if( started )
				started = false;
			else
				force->Play();*/
		}
		else
		{
			force->Stop();
			started = false;
		}
	}
}

void Vibrator::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"play"))
	{
		if( force && IsActive())
		{
			force->Play();
			started = true;
		}

		LogicDebug("Command <play>. Playing started.");
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

void _cdecl Vibrator::Draw(float dltTime, long level)
{
	if( IsActive() && looped && force && started )
	{
		if( force->GetPosition() < 0.0f )
			force->Play();
	}
}

void Vibrator::InitParams(MOPReader &reader)
{
	const char *t = reader.String().c_str();

	looped = reader.Bool();

	Activate(m_active = reader.Bool());

	long index = reader.Long();

	if( profile != t || index != deviceIndex )
	{
		if( force )
			force->Release();

		force = Controls().CreateForce(t,false,index);

	//	if( force && IsActive())
	//		force->Play();

		profile = t;
	}

	deviceIndex = index;

	started = false;
}

MOP_BEGINLISTCG(Vibrator, "Vibrator", '1.00', 100, "Vibrator\n\n    Use to play force feedback effect\n\nAviable commands list:\n\n    play - play effect\n\n        no parameters", "Default")

	MOP_STRING("Profile", "")
	MOP_BOOL  ("Looped" , false)
	MOP_BOOL  ("Active" , true)
	MOP_LONG  ("Device" , 0)

MOP_ENDLIST(Vibrator)
