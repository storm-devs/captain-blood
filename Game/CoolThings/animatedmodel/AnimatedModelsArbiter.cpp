#include "AnimatedModelsArbiter.h"
#include "AnimatedModel.h"

AnimatedModelsArbiter:: AnimatedModelsArbiter() : models(_FL_)
{
	debugDraw = false;
}

AnimatedModelsArbiter::~AnimatedModelsArbiter()
{
	#ifndef NO_CONSOLE

	Console().UnregisterCommand("AniModel");
				
	#endif
}

bool AnimatedModelsArbiter::Create(MOPReader &reader)
{
	#ifndef NO_CONSOLE		

	if(!EditMode_IsOn())
	{	
		Console().RegisterCommand("AniModel","show AnimatedModel item data",
			this,(CONSOLE_COMMAND)&AnimatedModelsArbiter::Console_AniModel);
	}

	#endif

	return true;
}

void _cdecl AnimatedModelsArbiter::Draw(float dltTime, long level)
{
//	float y =  0.0f;
	float y = 17.0f;

	for( int i = 0 ; i < models ; i++ )
	{
		y = models[i]->DrawDebug(y);
	}
}

#ifndef NO_CONSOLE

void _cdecl AnimatedModelsArbiter::Console_AniModel(const ConsoleStack &params)
{
	bool newFilter = false;

	if( params.GetSize())
	{
		const char *s = params.GetParam(0);

		if( s && s[0] && filter != s )
		{
			filter = s; newFilter = true;
		}
	}
	else
		filter = "*";

	if( newFilter )
	{
		debugDraw = true;
	}
	else
		debugDraw = !debugDraw;

	if( debugDraw )
	{
		SetUpdate((MOF_UPDATE)&AnimatedModelsArbiter::Draw,ML_DEBUG);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&AnimatedModelsArbiter::Draw);
	}

	for( int i = 0 ; i < models ; i++ )
	{
		AnimatedModel *p = models[i];

		bool is = string::IsFileMask(p->GetObjectID().c_str(),filter);

		p->SetDebug(is ? debugDraw : false);
	}
}

#endif

void AnimatedModelsArbiter::Activate(AnimatedModel *model, bool isActive)
{
	Assert(model)
	dword index = models.Find(model);

	if( isActive )
	{
		if( index == INVALID_ARRAY_INDEX )
		{
			models.Add(model);
		}
	}
	else
	{
		if( index != INVALID_ARRAY_INDEX )
		{
			models.DelIndex(index);
		}		
	}
}

MOP_BEGINLIST(AnimatedModelsArbiter, "", '1.00', 0)
MOP_ENDLIST(AnimatedModelsArbiter)
