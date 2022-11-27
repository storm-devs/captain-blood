

#include "GamerProfile.h"


SaveGamerProfile::SaveGamerProfile()
{
	block = false;
}

//Инициализировать объект
bool SaveGamerProfile::Create(MOPReader & reader)
{
	successful.Init(reader);
	failed.Init(reader);
	return true;
}

//Активировать/деактивировать объект
void SaveGamerProfile::Activate(bool isActive)
{
	if(!isActive)
	{
		LogicDebug("Deactivate: ignore action");
		return;
	}
	if(block)
	{
		LogicDebugError("Recursive activate don't is possible, try reactivate this object through delay");
		return;
	}
	block = true;
	if(LiveService().Data_Save())
	{
		// Nikita: для отложенного сейва может быть ошибка позже - будем ждать завершения.
		if( LiveService().Data_SaveLoadIsProcessing() )
		{
			SetUpdate(&SaveGamerProfile::Update,ML_TRIGGERS+100);
			return;
		}

		LogicDebug("Activate: save successful");
		successful.Activate(Mission(), false);
	}else{
		LogicDebug("Activate: save failed");
		failed.Activate(Mission(), false);
	}
	block = false;
}

// ждать завершения отложенной записи
void _cdecl SaveGamerProfile::Update(float dltTime, long level)
{
	// еще не закончилась запись
	if( LiveService().Data_SaveLoadIsProcessing() )
		return;

	// проверяем на успех записи
	if(LiveService().Data_SaveLoadIsSuccessful())
	{
		LogicDebug("Activate: save successful");
		successful.Activate(Mission(), false);
	}else{
		LogicDebug("Activate: save failed");
		failed.Activate(Mission(), false);
	}

	// снимаем блокировку и ожидание на каждом кадре
	block = false;
	DelUpdate(&SaveGamerProfile::Update);
}


MOP_BEGINLISTG(SaveGamerProfile, "Gamer profile - Save", '1.00', 0, "Managment")
	MOP_MISSIONTRIGGERG("Successful", "s.")
	MOP_MISSIONTRIGGERG("Failed", "f.")	
MOP_ENDLIST(SaveGamerProfile)


SaveOptions::SaveOptions()
{
	block = false;
}

//Инициализировать объект
bool SaveOptions::Create(MOPReader & reader)
{
	successful.Init(reader);
	failed.Init(reader);
	return true;
}

//Активировать/деактивировать объект
void SaveOptions::Activate(bool isActive)
{
	if(!isActive)
	{
		LogicDebug("Deactivate: ignore action");
		return;
	}
	if(block)
	{
		LogicDebugError("Recursive activate don't is possible, try reactivate this object through delay");
		return;
	}
	block = true;
	if(LiveService().Option_Save())
	{
		// Nikita: для отложенного сейва может быть ошибка позже - будем ждать завершения.
		if( LiveService().Data_SaveLoadIsProcessing() )
		{
			SetUpdate(&SaveOptions::Update,ML_TRIGGERS+100);
			return;
		}

		LogicDebug("Activate: save successful");
		successful.Activate(Mission(), false);
	}else{
		LogicDebug("Activate: save failed");
		failed.Activate(Mission(), false);
	}
	block = false;
}

// ждать завершения отложенной записи
void _cdecl SaveOptions::Update(float dltTime, long level)
{
	// еще не закончилась запись
	if( LiveService().Data_SaveLoadIsProcessing() )
		return;

	// проверяем на успех записи
	if(LiveService().Data_SaveLoadIsSuccessful())
	{
		LogicDebug("Activate: save successful");
		successful.Activate(Mission(), false);
	}else{
		LogicDebug("Activate: save failed");
		failed.Activate(Mission(), false);
	}

	// снимаем блокировку и ожидание на каждом кадре
	block = false;
	DelUpdate(&SaveGamerProfile::Update);
}

MOP_BEGINLISTG(SaveOptions, "Save options", '1.00', 0, "Managment")
	MOP_MISSIONTRIGGERG("Successful", "s.")
	MOP_MISSIONTRIGGERG("Failed", "f.")	
MOP_ENDLIST(SaveOptions)

LoadGamerProfile::LoadGamerProfile()
{
	block = false;
}

//Инициализировать объект
bool LoadGamerProfile::Create(MOPReader & reader)
{
	successful.Init(reader);
	failed.Init(reader);
	return true;
}

//Активировать/деактивировать объект
void LoadGamerProfile::Activate(bool isActive)
{
	if(!isActive)
	{
		LogicDebug("Deactivate: ignore action");
		return;
	}
	if(block)
	{
		LogicDebugError("Recursive activate don't is possible, try reactivate this object through delay");
		return;
	}
	block = true;
	if(LiveService().Data_Load())
	{
		LogicDebug("Activate: load successful");
		successful.Activate(Mission(), false);
	}else{
		LogicDebug("Activate: load failed");
		failed.Activate(Mission(), false);
	}
	block = false;
}

MOP_BEGINLISTG(LoadGamerProfile, "Gamer profile - Load", '1.00', 0, "Managment")
	MOP_MISSIONTRIGGERG("Successful", "s.")
	MOP_MISSIONTRIGGERG("Failed", "f.")	
MOP_ENDLIST(LoadGamerProfile)

