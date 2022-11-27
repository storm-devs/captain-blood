
#include "StorageWork.h"

bool GetStateFolder(const char* group, ICoreStorageFolder* &var)
{
	RELEASE(var);
	var = api->Storage().GetItemFolder(group,_FL_);	
	return (var!=null);
}

void GetStatVar(ICoreStorageFloat * & var, const char* state)
{
	RELEASE(var);
	char name[256];
	crt_strcpy(name, sizeof(name), "Runtime.Statistic.");
	crt_strcat(name, sizeof(name), state);
	var = api->Storage().GetItemFloat(name, _FL_);
	var->Set(0.0f);
}

bool GetStateFloatVar(const char* state, ICoreStorageFloat* &var)
{	
	RELEASE(var);
	var = api->Storage().GetItemFloat(state,_FL_);
	return (var!=null);
}


float ReadGameState(const char* id,float def)
{
	return api->Storage().GetFloat(id,def);	
}

void WriteGameState(const char* id, float value)
{	
	api->Storage().SetFloat(id, value);
}

float ReadGameState(ICoreStorageFloat* var,float def)
{
	if (var)
	{
		if (var->IsValidate())		
		{
			return var->Get(def);
		}		
	}

	return def;
}

void WriteGameState(ICoreStorageFloat* var, float value)
{	
	if (var)
	{
		var->Set(value);
	}
}

bool GetStateStringVar(const char* state, ICoreStorageString* &var)
{
	RELEASE(var);
	var = api->Storage().GetItemString(state,_FL_);
	return (var!=null);
}

const char* ReadGameState(const char* id,const char* def)
{	
	return api->Storage().GetString(id,def);	
}

void WriteGameState(const char* id, const char* value)
{
	api->Storage().SetString(id, value);
}

const char* ReadGameState(ICoreStorageString* var,const char* def)
{
	if (var)
	{
		if (var->IsValidate())		
		{
			return var->Get(def);
		}		
	}

	return def;
}

void  WriteGameState(ICoreStorageString* var, const char* value)
{
	if (var)
	{
		var->Set(value);
	}
}