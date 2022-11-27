//============================================================================================
// Spirenkov Maxim, 2009
//============================================================================================
// Mission objects
//============================================================================================
// SetLanguage
//============================================================================================


#include "SetLanguage.h"
#include "..\MissionsManager.h"


SetLanguage::SetLanguage()
{
	langName = null;
}

SetLanguage::~SetLanguage()
{
}

//Инициализировать объект
bool SetLanguage::Create(MOPReader & reader)
{
	MissionObject::Activate(false);
	langName = reader.String().c_str();
	return true;
}

//Активировать
void SetLanguage::Activate(bool isActive)
{
	if(EditMode_IsOn())
	{
		return;
	}
	//На деактивацию ничего не делает
	if(!isActive)
	{
		LogicDebug("Deactivate (do nothing)");
		return;
	}
	//Ставим новый язык
	LogicDebug("Activate, set for wait language %s", langName);
	SetLanguageBlockService::SetNewLanguage(langName);
}

//Пересоздать объект
void SetLanguage::Restart()
{	
}

MOP_BEGINLISTG(SetLanguage, "Set language", '1.00', 0, "Managment")
	MOP_STRINGC("Language name", "", "Set language name or language id")
MOP_ENDLIST(SetLanguage)




CREATE_SERVICE(SetLanguageBlockService, 1000)


SetLanguageBlockService * SetLanguageBlockService::ptr = null;


SetLanguageBlockService::SetLanguageBlockService()
{
	Assert(!ptr);
	ptr = this;
}

SetLanguageBlockService::~SetLanguageBlockService()
{
	Assert(ptr == this);
	ptr = null;
}

void SetLanguageBlockService::SetNewLanguage(const char * name)
{
	if(string::IsEmpty(name))
	{
		return;
	}
	Assert(ptr);
	ptr->languageName = name;
}

bool SetLanguageBlockService::Init()
{
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 100);
	return true;
}

void SetLanguageBlockService::EndFrame(float dltTime)
{
	if(!languageName.IsEmpty())
	{
		const char * currentLang = api->Storage().GetString(ILocStrings_StorageLocalizationPath, "");
		if(languageName == currentLang)
		{
			languageName.Empty();
			api->Trace("Skip language process, because names is identical (\"%s\" and \"%s\")", languageName.c_str(), currentLang);
			return;
		}
		api->Trace("Set new language \"%s\" process...", languageName.c_str());
		//Перегружаем строки
#ifdef _XBOX
		IRender * render = (IRender *)api->GetService("DX9Render");
		Assert(render);
		render->X360_DeviceSuspend();
#endif
		api->Storage().SetString(ILocStrings_StorageLocalizationPath, languageName.c_str());
		ILocStrings * locStrings = (ILocStrings *)api->GetService("LocStrings");
		Assert(locStrings);
		locStrings->LoadAll();
		//Делаем рестарт всех миссий
		if(MissionsManager::Ptr())
		{
			MissionsManager::Ptr()->RestartAll();
		}else{
			api->Trace("Error: Can't set new language in editor");
		}
		languageName.Empty();
#ifdef _XBOX
		render->X360_DeviceResume();
#endif
	}
}


