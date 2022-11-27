//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormEffects
//============================================================================================

#include "..\SndOptions.h"

class PreviewAnimation;
class FormTimeLine;
class FormListAnimations;
class FormListNodes;
class FormComboBox;
class FormCheckBox;
class BaseSlider;


class FormAnimation : public GUIControl
{
	enum Consts
	{
		c_timelineHeightInPercents = 30,
		c_editfield_width = 300,
		c_editfield_height = 200,
		c_editfield_border = 10,
		c_editfield_slider = 10,
		c_editfield_ed_height = 20,
		c_editfield_edl_height = 300,
		c_editfield_button_w = 32,
		c_editfield_button_h = 32,
	};

public:
	FormAnimation(GUIWindow * parent, GUIRectangle & rect);
	virtual ~FormAnimation();

	//Рисование
	virtual void Draw();

private:
	//Установить событие
	void _cdecl DoSetMovie(GUIControl* sender);
	//Добавить нод
	void _cdecl DoAddNode(GUIControl* sender);
	//Изменён выделеный эвент
	void _cdecl DoChangeSelectEvent(GUIControl* sender);
	//Спрятать параметры эвента
	void HideEventParams();
	//Изменён выделеный эвент
	bool IsCanEditEvent();
	//Выбрать звук для события
	void _cdecl DoSelectSound(GUIControl* sender);
	//Выбрать локатор для события
	void _cdecl DoSelectLocator(GUIControl* sender);	
	//Время фэйда
	void _cdecl DoChangeFadetime(GUIControl* sender);
	//Флажки эвента
	void _cdecl DoChangeLocalGlobal(GUIControl* sender);
	void _cdecl DoChangeBind(GUIControl* sender);
	void _cdecl DoChangeVoice(GUIControl* sender);
	void _cdecl DoPlaySelectEvent(GUIControl* sender);
	void _cdecl DoStopPlaySelectEvent(GUIControl* sender);

private:
	FormListAnimations * listOfAnimations;	
	PreviewAnimation * preview;
	FormTimeLine * timeLine;
	//Параметры для режима дизайнеров
	FormListNodes * listOfNodes;
	//Параметры для режима звуковиков
	FormComboBox * soundSelect;
	FormComboBox * locatorSelect;
	long selectAnxNodeForEvent;
	UniqId selectEventId;
	FormCheckBox * checkLocalGlobal;
	FormCheckBox * checkBind;
	FormCheckBox * checkVoice;
	BaseSlider * fadeTime;
	FormButton * eventPlay;
	FormButton * eventStop;

};









