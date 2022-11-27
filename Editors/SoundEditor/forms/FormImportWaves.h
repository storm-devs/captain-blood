//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormImportWaves
//============================================================================================

#include "..\SndOptions.h"


class FormImportWaves : public GUIWindow
{
	enum Consts
	{
		c_width = 600,
		c_height = 140,

		c_max_string_len = 74,

		c_textleft = 10,
		c_texttop = 10,
		c_button_width = 130,
		c_button_height = 25,
		c_button_space = 8,

		//Индексы наборов кнопок
		c_bt_importStop = 0,
		c_bt_waitSkip,
		c_bt_waitReplace,
		c_bt_waitRename,
		c_bt_waitStop,		
		c_bt_errorContinue,
		c_bt_errorStop,		
		c_bt_count,
		c_bt_importBeg = c_bt_importStop,
		c_bt_importEnd = c_bt_importStop,
		c_bt_waitBeg = c_bt_waitSkip,
		c_bt_waitEnd = c_bt_waitStop,
		c_bt_errorBeg = c_bt_errorContinue,
		c_bt_errorEnd = c_bt_errorStop,
	};

	enum Stage
	{
		stage_select_file,
		stage_wait_file,
		stage_import_file,
		stage_error_file,
		stage_no_files,
	};


public:
	FormImportWaves(GUIControl * parent);
	virtual ~FormImportWaves();

private:
	//Разместить набор кнопок
	void CreateButtons(long from, long to);
	//Показать группу кнопок
	void ShowButtons(long from, long to);
	//Рисование
	virtual void Draw();


private:
	void _cdecl OnImportStop(GUIControl* sender);
	void _cdecl OnWaitStop(GUIControl* sender);
	void _cdecl OnWaitReplace(GUIControl* sender);
	void _cdecl OnWaitRename(GUIControl* sender);
	void _cdecl OnWaitRename_Check(GUIControl* sender);
	void _cdecl OnWaitRename_Ok(GUIControl* sender);
	void _cdecl OnWaitSkip(GUIControl* sender);
	void _cdecl OnErrorStop(GUIControl* sender);
	void _cdecl OnErrorContinue(GUIControl* sender);



public:
	array<string> files;
	long importIndex;
	Stage stage;
	string currentPath;
	string currentName;
	string currentDrawPath;
	UniqId currentFolder;
	bool isReplaceMode;
	FormButton * buttons[c_bt_count];
	string errorCode;	
};









