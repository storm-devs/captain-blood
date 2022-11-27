

#pragma once


#include "..\SndBase.h"

class FormWaveSelector;
struct FormWaveSelectorOptions;
class ProjectWave;

class FormAddWaveToSound : public GUIWindow
{
	enum Consts
	{
		c_border = 5,
		c_button_width = 130,
		c_button_height = 25,		

	};

public:
	FormAddWaveToSound(FormWaveSelectorOptions & opt, GUIControl * parent, const GUIRectangle & rect);
	virtual ~FormAddWaveToSound();

	//Получить список выделенных звуков
	void GetWaves(array<UniqId> & chooseWaves);

private:
	virtual void Draw();
	void _cdecl OnOkAdd(GUIControl* sender);
	void _cdecl OnCancelAdd(GUIControl* sender);

public:
	GUIEventHandler onOk;
	GUIEventHandler onCancel;

private:
	FormButton * buttonOk;
	FormButton * buttonCancel;
	FormWaveSelector * list;
};