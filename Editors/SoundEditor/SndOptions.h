//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditorMainWindow
//============================================================================================

#ifndef _SndOptions_h_
#define _SndOptions_h_

#include "Project\SndProject.h"
#include "Forms\\FormOptions.h"

//Код сообщения лежит lparam, сообщение GUIMSG_FORCE_DWORD
enum SndGUIMessage
{
	sndmsg_switchfocus,
	sndmsg_stopmodal,
	sndmsg_select_sound,
};


class SndOptions
{
	struct ErrorCode
	{
		string str;
		long id;
		bool isDone;		
	};


public:
	SndOptions();
	~SndOptions();

//---------------------------------------------------------------------------------------------------
//Системная информация
//---------------------------------------------------------------------------------------------------
	string pathProject;
	string pathProjectXmlz;
	string pathTempWorkFolder;
	string pathTempSaveFolder;
	string pathExportEvents;
	string pathTempLogs;
	string pathStageLog;
	string pathPhonemesLog;
	string pathLeakWaves;
	string pathGameMissionsIni;
	string pathVersionFiles;
	string pathGlobalSettings;
	string pathLocalSettings;
	string pathProjectPolygonOptions;

	dword screenWidth;
	dword screenHeight;

//---------------------------------------------------------------------------------------------------
//Цвета
//---------------------------------------------------------------------------------------------------
public:
	dword bkg;							//Базовый цвет фона
	dword black2Bkg[8];					//Тона от чёрного до цвета фона - 1
	dword bkg2White[8];					//Тона от цвета фона - 1 до белого
	dword colorTextHi;					//Цвет текста для тёмных тонов
	dword colorTextLo;					//Цвет текста для сетлых тонов
	dword colorSelect;					//Цвет выделения					

//---------------------------------------------------------------------------------------------------
//Стандартные картинки
//---------------------------------------------------------------------------------------------------
public:
	GUIImage imageCloseEditor;			//Картинка закрытия редактора
	GUIImage imageWave;					//Волновые данные
	GUIImage imageWaveRootFolder;		//Волновые данные корневая папка
	GUIImage imageWaveOpenFolder;		//Волновые данные открытая папка
	GUIImage imageSound;				//Звук
	GUIImage imageSoundRootFolder;		//Звуковые данные корневая папка
	GUIImage imageSoundOpenFolder;		//Звуковые данные открытая папка
	GUIImage imageCloseFolder;			//Закрытая папка
	GUIImage imageOpenFolder;			//Открытая папка
	GUIImage imageUp;					//Подняться вверх по иерархии
	GUIImage imageComputer;				//Компьютер
	GUIImage imageXbox;					//Xbox
	GUIImage imagePCM;					//Формат PCM
	GUIImage imageXWma;					//Формат XWma (ноты)
	GUIImage imageXma;					//Формат Xma
	GUIImage imageWaitToSave;			//Ждёт записи
	GUIImage imageAddNewBank;			//Добавить новый звуковой банк
	GUIImage imageSoundBanks;			//Звуковые банки
	GUIImage imageSoundBank;			//Звуковой банк
	GUIImage imageSoundBankExp;			//Экспортировать звуковой банк
	GUIImage imageSoundBankOpts;		//Опции звукового банка
	GUIImage imageEar;					//Значёк уха
	GUIImage imageAttenuations;			//Значёк гравиков затухания от дистанции
	GUIImage imageSoundSetup;			//Значёк настроек звука
	GUIImage imageChanalTime;			//Значёк секундомера (время занимающее канал)
	GUIImage imageAniEvtGlb;			//Картинка для анимационного эвента глобального звука
	GUIImage imageAniEvtGlbSel;			//Картинка для анимационного эвента глобального звука с выделением
	GUIImage imageAniEvtLoc;			//Картинка для анимационного эвента звука в локаторе
	GUIImage imageAniEvtLocSel;			//Картинка для анимационного эвента звука в локаторе с выделением
	GUIImage imageAnimations;			//Картинка для списка анимаций
	GUIImage imageAnimation;			//Картинка анимации
	GUIImage imageAniMovie;				//Картинка ролика
	GUIImage imageLocator;				//Картинка для списка локаторов
	GUIImage imagePlay;					//Проиграть
	GUIImage imagePlayNode;				//Проиграть текущий нод
	GUIImage imagePlayVol;				//Проиграть с применением громкости
	GUIImage imageStop;					//Остановить проигрывание
	GUIImage imageMoveToStart;			//На начало
	GUIImage imageMoveToStartNode;		//На начало нода
	GUIImage imageMoveToEnd;			//В конец
	GUIImage imageMoveToEndNode;		//В конец нода
	GUIImage imageMoveToPrev;			//В предыдущую позицию
	GUIImage imageMoveToNext;			//В следующую позицию
	GUIImage imageAddSound;				//Добавить звук
	GUIImage imageAddWave;				//Добавить волну
	GUIImage imageAddSilence;			//Добавить слот тишины
	GUIImage imageImportWave;			//Импортировать волны
	GUIImage imageAddFolder;			//Добавить папку
	GUIImage imageRename;				//Переименовать
	GUIImage imageDelete;				//Удалить	
	GUIImage imageAddSetup;				//Добавить настройки звука
	GUIImage imageAddAtts;				//Добавить график затухания
	GUIImage imageStageMovie;			//Состояние редактирования клипа
	GUIImage imageStageEvents;			//Состояние расстановки событий
	GUIImage imageStageApproval;		//Состояние приёмки
	GUIImage imageStageDone;			//Законченный клип
	GUIImage imageStageCheck;			//Были изменения, требуеться проверка
	GUIImage imageStageDamage;			//Были разрушены данные в файле, требуеться детальное рассмотрение
	GUIImage imageExportError;			//Входное сообщение об ошибке
	GUIImage imageLineArrow;			//Стрелка вправо
	GUIImage imageModel;				//Моделька
	GUIImage imageCamera;				//Картинка камеры


//---------------------------------------------------------------------------------------------------
//Шрифты
//---------------------------------------------------------------------------------------------------
public:
	IFont * uiFont;						//Стандартный шрифт


//---------------------------------------------------------------------------------------------------
//Службы, проект
//---------------------------------------------------------------------------------------------------
public:
	GUIControl * kbFocus;				//Окно принимающие ввод с клавиатуры
	GUIControl * dndControl;			//Контрок, активировавшее драг
	IGUIManager * gui_manager;			//Менеджер интерфейсов	
	IRender * render;					//Рисовалка	
	IFileService * fileService;			//Файловый сервис
	ISoundService * soundService;		//Звуковой сервис
	ISoundEditorAccessor * sa;			//Интерфейс звукового сервиса для редактора
	IAnimationScene * aniScene;			//Анимационная сцена, в которой создаём анимации для окна эвентов
	//Сервис ввода
	IControlsService * controlService;
	IControls * controls;
	//Формы
	FormOptions formOptions;			//Настройки форм
	//Права
	dword policy;
	bool isDeveloperMode;
	bool isEnableSwitchPolicy;
	long leakWavesMode;

//---------------------------------------------------------------------------------------------------
//Строки для UI
//---------------------------------------------------------------------------------------------------
public:	
	const char * GetString(dword id);	//Получить строку редактора

private:
	const char * strings[1024];	
public:
#include "strings.inl"	


public:
	//Сгенерировать законченную ошибку, если id пустой, то завести новую ошибку
	ErrorId _cdecl ErrorOut(ErrorId * id, bool isReady, const char * format, ...);
	//Отменить/удалить ошибку
	void ErrorCancel(ErrorId id);
	//Получить ошибку
	void ErrorGet(ErrorId id, string & error, bool isDelete);
	//Получить лог ошибок
	void ErrorGetLog(array<string> & errors);
	//Очистить лог ошибок
	void ErrorClear();


private:
	CritSection errorCodeAccessor;	
	array<ErrorCode> errorCodes;
	long errorCounter;

//---------------------------------------------------------------------------------------------------
//Утилитные
//---------------------------------------------------------------------------------------------------
public:
	//Надо ли подсветить контрол под курсором мыши
	bool IsHighlight(GUIControl * ctrl);
	//Идут ли прослушивание волны
	bool WaveIsPreview();
	//Остановить прослушивание
	void WavePreviewStop();
	//Установить в движке настройки ввода
	void SetInputOptions();	

private:
	//Предзагрузочные утилиты
	void PreloadUtilites();
	//Сдублировать звуковой банк
	bool DuplicateSoundBank(IIniFile * ini);
	//Установить режим выбора волн в звуках с 1 волной
	void SetSelectModeForSound();

//---------------------------------------------------------------------------------------------------
//Буфера
//---------------------------------------------------------------------------------------------------
public:
	string buildExportPathTmp;
	string getExportDataTmp;
	string buildFileNameTmp;
	string waveImportTmp;
	string saveWaveTmp;
	string formExportSoundBankDrawTmp;
	string formWaveFilesDoInitListTmp;
	string formWaveFilesDoUpByHerarchyTmp;
	string formImportWavesDrawTmp;
	string tmp_projectObject_Init;
	string tmp_projectObject_SaveProcess;
	string tmp_projectObject_LoadProcess;
	string tmp_Project_LoadProcess;
	string tmp_projectObject_Rename_Cur;
	string tmp_projectObject_Rename_New;
	string tmp_projectObject_IsCanRename;
	string tmp_projectObject_Delete;
	array<byte> saveBuffer;

	struct Collection
	{
		void * ptr;
		long index;
	};

	array<Collection> collection;
};

extern SndOptions * options;
extern SndProject * project;


#endif
