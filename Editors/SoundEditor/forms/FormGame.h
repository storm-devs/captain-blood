//============================================================================================
// FormGame
//============================================================================================

#include "..\SndOptions.h"

class FormWaveSelector;
class FormWaveFiles;
class FormComboBox;
class FormLabel;
class NumberSlider;

class FormSoundWaves;
class FormGameList;

class MissionObject;

class FormAttGraph;
class FormSndBaseParams;

class FormGame : public GUIControl
{
	enum Consts
	{
		c_gamesizew = 70,
		c_gamesizeh = 70,

		c_bparams_h = 209,
	//	c_bparams_w = 580
	};

	struct Info
	{
		string name;
		string file;
	};

public:

	FormGame(GUIWindow * parent, GUIRectangle & rect);
	virtual ~FormGame();

	//Рисование
	virtual void Draw();

private:

	void Load();

private:

	void _cdecl OnComboSelect(GUIControl *sender);
	void _cdecl OnSoundSelect(GUIControl *sender);

	void _cdecl OnSwitch	 (GUIControl *sender);

private:

	class IPreviewService *service;

	int misIndex;
	bool misOk; // миссия загружена

	ExtName errText;

	class IMission *mission;

	bool pause;
	bool pauseDown;

	ExtName pauseText;

	MissionObject *camera;

	POINT cursor;

	FormComboBox *combo; array<Info> names;

	FormButton *button;

	FormGameList *list;

	FormSoundWaves *waves;

	FormAttGraph *graph;

	FormSndBaseParams *params;

	GUIRectangle wavesRect;

	IEditableIniFile * settingsIni;	

};
