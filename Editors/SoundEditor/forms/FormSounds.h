//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormSounds
//============================================================================================

#include "..\SndOptions.h"

class FormListSounds;
class FormSoundWaves;
class FormComboBox;
class FormButton;
class NumberSlider;
class FormAttGraph;
class FormSndBaseParams;

class FormSounds : public GUIControl
{
	enum Consts
	{
		c_bank_space = 50,
		c_bank_cb_left = 20,
		c_bank_cb_top = 25,
		c_bank_cb_width = 200,
		c_bank_cb_height = 20,
		c_bank_btn_space = 10,
		c_bank_cbl_height = 200,
		c_soundslist_width_in_percents = 30,
		c_swlist_heigth_in_percents = 28,
		c_title_h = 23,
		c_controls_space = 40,
		c_graphpart_y = 34,
		c_graphpart_height = 28,
		c_graphpart_title_y = -2,		
		c_slider_width = 30,
		c_slider_space = 50,
		c_soundsbtn_width = 16,
		c_soundsbtn_height = 16,
		c_soundsbtn_y = 2,
		c_soundsbtn_border = 6,
		c_combo_w = 200,
		c_combo_h = 18,
		c_combo_list_h = 120,
	};

public:
	FormSounds(GUIWindow * parent, GUIRectangle & rect);
	virtual ~FormSounds();

private:
	//Рисование
	virtual void Draw();

	//Был изменён текущий звук
	void ChangeSound(const UniqId & newSoundId);
	//Получить текущий банк
	bool GetCurrentSoundBank(UniqId & bankId);

	void _cdecl OnChangeSound(GUIControl* sender);
	void _cdecl OnSoundAddNewSound(GUIControl* sender);
	void _cdecl OnSoundAddNewSound_Check(GUIControl* sender);
	void _cdecl OnAddNewSoundBank_Ok(GUIControl* sender);
	void _cdecl OnSoundAddNewFolder(GUIControl* sender);
	void _cdecl OnSoundAddNewFolder_Check(GUIControl* sender);
	void _cdecl OnSoundAddNewFolder_Ok(GUIControl* sender);
	void _cdecl OnSoundRenameSelectItem(GUIControl* sender);
	void _cdecl OnSoundDeleteSelectItem(GUIControl* sender);
	void _cdecl OnSoundDeleteSelectItem_Process(GUIControl* sender);
	void _cdecl OnSoundRenameSelectItem_Check(GUIControl* sender);
	void _cdecl OnSoundRenameSelectItem_Ok(GUIControl* sender);
	void _cdecl OnSoundChangeWave(GUIControl* sender);
	
	void _cdecl OnSoundSoundBankOptions(GUIControl* sender);
	void _cdecl OnSoundSoundBankExport(GUIControl* sender);

	void _cdecl OnSoundPlay(GUIControl* sender);
	void _cdecl OnSoundStop(GUIControl* sender);
	

private:
	//Основные элементы формы
	FormListSounds * listOfSounds;
	FormSoundWaves * listOfSoundWaves;
	FormSndBaseParams * sndBaseParams;
	FormAttGraph * attenuationGraph;
	FormSndBaseParams * soundBaseParams;	
	FormButton * buttonSoundBankExport;	
	FormButton * buttonSoundBankOptions;
	FormButton * buttonCreateSound;
	FormButton * buttonCreateFolder;
	FormButton * buttonRename;
	FormButton * buttonDelete;
	
	
	long divX;	
	UniqId currentSoundId;				//Индекс текущего выбранного звука
	bool isInitState;
	UniqId previewSoundId;
	ISoundScene * previewScene;
	ISound * previewSound;
};









