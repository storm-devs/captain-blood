
#include "..\SndBase.h"

class ProjectSound;
class ProjectObject;
class SoundBaseParams;

class FormSndBaseParams : public GUIControl
{
	enum Consts
	{
		c_space = 10,
		c_slider_width = 30,
		c_slider_space = 50,
		c_title_h = 23,
		c_title_space = sgc_header_left_space,
		c_combo_w = sgc_header_combo_w,
		c_combo_h = sgc_header_controls_h,
		c_combo_list_w = c_combo_w,
		c_combo_list_h = sgc_header_combo_list_h,
		c_button_w = sgc_header_buttons_w,
		c_combo_step_y = 50,
		c_check_step_y = 25,
	};


public:
	FormSndBaseParams(GUIControl * parent, const GUIRectangle & rect, bool isLightVersion);
	~FormSndBaseParams();


public:
	//Установить текущий звук, в который прописывать выбранные параметры
	void SetCurrentSound(const UniqId & sndId);

private:
	//Рисование
	virtual void Draw();

	//События
	void _cdecl OnChangeVolume(GUIControl* sender);
	void _cdecl OnChangePriority(GUIControl* sender);
	void _cdecl OnChangeCount(GUIControl* sender);
	void _cdecl OnChangeSelect(GUIControl* sender);
	void _cdecl OnChangeLoop(GUIControl* sender);
	void _cdecl OnChangeFx(GUIControl* sender);
	void _cdecl OnChangePhonemes(GUIControl* sender);
	void _cdecl OnChangeSetup(GUIControl* sender);
	void _cdecl OnSetupAdd(GUIControl* sender);
	void _cdecl OnCheckSetupName(GUIControl* sender);
	void _cdecl OnAddSetupNew(GUIControl* sender);
	void _cdecl OnRestoreSelect(GUIControl* sender);
	void _cdecl OnSetupRename(GUIControl* sender);
	void _cdecl OnSetupRenameProcess(GUIControl* sender);
	void _cdecl OnSetupDelete(GUIControl* sender);
	void _cdecl OnSetupDeleteProcess(GUIControl* sender);
	void _cdecl OnPlaySound(GUIControl* sender);
	void _cdecl OnStopSound(GUIControl* sender);

private:
	//Получить текущую структуру для модификации и текущий объект для сохранения
	SoundBaseParams * GetCurrentObject(ProjectObject * & saveObject);
	//Заполнить список пресэтов
	void FillPresetsList();

public:
	GUIEventHandler onSoundPlay;
	GUIEventHandler onSoundStop;

private:
	NumberSlider * volumeSlider;
	NumberSlider * prioritySlider;
	NumberSlider * counterSlider;	
	FormComboBox * selectList;
	FormComboBox * fxList;
	FormCheckBox * loopCheckBox;
	FormCheckBox * phonemesCheckBox;
	FormComboBox * setupsList;
	FormButton * buttonAdd;
	FormButton * buttonRename;
	FormButton * buttonDelete;
	FormButton * buttonPlay;
	FormButton * buttonStop;
	ExtName title;
	ProjectSound * currentSound;
	float correctVolume;
	bool isInitState;
};

