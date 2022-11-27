

#include "FormOptions.h"
#include "..\SndOptions.h"

FormWaveSelectorOptions::FormWaveSelectorOptions()
{
}

FormWaveFilesOptions::FormWaveFilesOptions()
{
	ResetPath();
}

//Инициализировать путь
void FormWaveFilesOptions::ResetPath()
{
	char buffer[MAX_PATH + 128];
	::GetCurrentDirectory(MAX_PATH + 127, buffer);
	buffer[MAX_PATH + 127] = 0;
	currentFolder = buffer;
	setCursorOnItem.Empty();
}

FormWavesOptions::FormWavesOptions()
{
	isFocusLeftPanel = true;
}


FormListSoundsOptions::FormListSoundsOptions()
{
}


FormSoundWavesOptions::FormSoundWavesOptions()
{
	currentWaveElement = -1;
}


FormListAnimationsOptions::FormListAnimationsOptions()
{
}