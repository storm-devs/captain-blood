
#pragma once

#include "..\SndBase.h"

//-----------------------------------------------------------------------------------------------------
//FormWaves
//-----------------------------------------------------------------------------------------------------

struct FormWaveSelectorOptions
{
	FormWaveSelectorOptions();

	UniqId currentFolder;
	string setCursorOnItem;
};

struct FormWaveFilesOptions
{
	FormWaveFilesOptions();
	void ResetPath();		//Инициализировать путь

	string currentFolder;
	string setCursorOnItem;
};

struct FormWavesOptions
{
	FormWavesOptions();

	FormWaveSelectorOptions leftPanel;
	FormWaveFilesOptions rightPanel;
	bool isFocusLeftPanel;
};

//-----------------------------------------------------------------------------------------------------
//FormSounds
//-----------------------------------------------------------------------------------------------------


struct FormListSoundsOptions
{
	FormListSoundsOptions();

	UniqId currentFolder;
	string setCursorOnItem;
};

struct FormSoundWavesOptions
{
	FormSoundWavesOptions();

	UniqId currentSound;			//Индекс звука, с который работаем
	long currentWaveElement;		//Индекс волны, в звуке
};



struct FormSoundsOptions
{
	
	FormListSoundsOptions sounds;
	FormSoundWavesOptions soundWaves;
	FormWaveSelectorOptions addWave;
};


//-----------------------------------------------------------------------------------------------------
//FormAnimations
//-----------------------------------------------------------------------------------------------------

struct FormListAnimationsOptions
{
	FormListAnimationsOptions();

	UniqId currentAnimation;	//Идентификатор анимации на которой курсор
	string setCursorOnItem;
	string animationForNodes;
	string lastNode;
};



struct FormAnimationsOptions
{
	FormListAnimationsOptions aniList;
};



//-----------------------------------------------------------------------------------------------------

struct FormOptions
{
	FormWavesOptions formWaves;
	FormSoundsOptions formSounds;
	FormAnimationsOptions formAnimations;
};


