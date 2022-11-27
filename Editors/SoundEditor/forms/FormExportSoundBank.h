//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormExportSoundBank
//============================================================================================


#include "..\SndOptions.h"


class SndExporter;
class FormLog;

class FormExportSoundBank : public GUIWindow
{
	enum Consts
	{
		c_width = 600,
		c_height = 600,
		c_logspace = 10,
		c_cancel_button_w = 150,
		c_cancel_button_h = 25,
		c_cancel_button_y = 30,
	};

public:
	FormExportSoundBank(GUIControl * parent, UniqId * banks, long count);
	virtual ~FormExportSoundBank();

private:
	//Рисование
	virtual void Draw();

	void _cdecl StopExport(GUIControl* sender);
	


private:
	FormLog * exportLog;
	FormButton * buttonCancel;
	SndExporter * sndExporter;
	dword currentExportIndex;
	array<ProjectSoundBank *> exportTable;
};