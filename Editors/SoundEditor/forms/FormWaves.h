//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormWaves
//============================================================================================

#include "..\SndOptions.h"
#include "dev\WaveDecomposer.h"

class FormWaveSelector;
class FormWaveFiles;
class FormComboBox;
class FormLabel;
class NumberSlider;
class FormEdit;

class FormWaves : public GUIControl
{
	enum Consts
	{
		c_dividerwidth = 6,
		c_topBorder = 10,
		c_leftBorder = 10,
		c_y_stepSpace = 10,
		c_labelsWidth = 220,
		c_listsWidth = 300,
		c_listsHeightPC = 160,
		c_listsHeightXBOX = 355,
		c_lineHeight = 26,
		c_previewButtons_s = 24,
		c_checkBoxWidth = 240,


		//Идентификаторы форматов
		c_format_pcm =          0x10000000,
		c_format_xwma =         0x20000000,
		c_format_xma =          0x30000000,
		c_format_mask =         0xf0000000,
		c_format_quality_mask = 0xff,
		c_format_quality_shift = 20,
		c_format_rate_mask     = 0x000fffff,
	};

public:
	FormWaves(GUIWindow * parent, GUIRectangle & rect);
	virtual ~FormWaves();

	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Обновить фокус
	void UpdateFocus();
	//Рисование
	virtual void Draw();
		
	void _cdecl OnWavesSelectorAddFolder(GUIControl* sender);
	void _cdecl OnWaveAddNewFolder_Check(GUIControl* sender);
	void _cdecl OnWaveAddNewFolder_Ok(GUIControl* sender);
	void _cdecl OnWavesSelectorRename(GUIControl* sender);
	void _cdecl OnWavesSelectorRename_Check(GUIControl* sender);
	void _cdecl OnWavesSelectorRename_Ok(GUIControl* sender);
	void _cdecl OnWavesSelectorDelete(GUIControl* sender);
	void _cdecl OnWavesSelectorDelete_Process(GUIControl* sender);
	void _cdecl OnWavesSelectorChangeFocus(GUIControl* sender);	
	void _cdecl OnChangeWaveNotes(GUIControl* sender);
	void _cdecl OnUpdateWaveNotesHint(GUIControl* sender);
	void _cdecl OnChangeWaveExportParams(GUIControl* sender);
	bool ApplyChangeWaveExportParams(ProjectWave::WaveOptions & opts, dword formatValue, bool isCanXwma);
	void _cdecl OnPreviewPlayWavePC(GUIControl* sender);		
	void _cdecl OnPreviewPlayWaveXbox(GUIControl* sender);
	void _cdecl OnStop(GUIControl* sender);
	void _cdecl OnPreviewChangeWave(GUIControl* sender);
	void _cdecl OnChangeIgnorePA(GUIControl* sender);
	void _cdecl OnChangeIgnoreMA(GUIControl* sender);
	void _cdecl OnChangeIgnoreSln(GUIControl* sender);
	void _cdecl OnChangeIgnoreClk(GUIControl* sender);
	void _cdecl OnWavesSelectorDecomposeWave(GUIControl* sender);
	void _cdecl OnWavesSelectorHideDecompose(GUIControl* sender);
	

private:
	//Основные элементы формы
	FormWaveSelector * listOfWaves;
	FormWaveFiles * listOfFiles;
	FormButton * listOfWavesButtonAddFolder;
	FormButton * listOfWavesButtonRename;
	FormButton * listOfWavesButtonDelete;
	FormLabel * currentWaveLabel;	
	FormLabel * currentWave;
	FormLabel * currentWaveNotesLabel;
	FormEdit * currentWaveEdit;
	FormLabel * pcFormatLabel;
	FormComboBox * pcFormat;
	FormButton * pcPlay;
	FormLabel * xboxFormatLabel;
	FormComboBox * xboxFormat;
	FormButton * xboxPlay;
	FormButton * stop;
	NumberSlider * waveMirror;
	FormLabel * waveMirrorLabel;
	FormCheckBox * ignorePACheckBox;
	FormCheckBox * ignoreMACheckBox;
	FormCheckBox * ignoreSlnCheckBox;
	FormCheckBox * ignoreClkCheckBox;
	string exportedWaveSize;
	WaveDecomposer * waveDecomposer;
};









