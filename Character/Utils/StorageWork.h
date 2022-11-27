
#ifndef _StorageWork_h_
#define _StorageWork_h_

#include "..\..\Common_h\mission.h"

bool GetStateFolder(const char* group, ICoreStorageFolder* &var);

bool  GetStateFloatVar(const char* state, ICoreStorageFloat* &var);

void GetStatVar(ICoreStorageFloat * & var, const char* state);

float ReadGameState(const char* id,float def);
void  WriteGameState(const char* id, float value);

float ReadGameState(ICoreStorageFloat* var,float def);
void  WriteGameState(ICoreStorageFloat* var, float value);	

bool  GetStateStringVar(const char* state, ICoreStorageString* &var);

const char* ReadGameState(const char* id,const char* def);
void  WriteGameState(const char* id, const char* value);

const char* ReadGameState(ICoreStorageString* var,const char* def);
void  WriteGameState(ICoreStorageString* var, const char* value);	

#endif


