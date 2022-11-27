//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxOptions
//============================================================================================

#ifndef _AnxOptions_h_
#define _AnxOptions_h_

#include "..\..\Common_h\math3d.h"
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\Common_h\render.h"
#include "..\..\Common_h\gmx.h"
#include "..\..\Common_h\Sound.h"
#include "..\..\Common_h\particles.h"
#include "..\..\Common_h\controls.h"
#include "..\..\Common_h\animation.h"
#include "..\..\Common_h\Compression\Compressor.h"
#include "..\..\System\AnimationService\AnxAnimation\AnxFileFormat.h"
#include "..\..\System\AnimationService\IAnxEditorServiceAccess.h"
#include "AnxStreams.h"

class AnxProject;
class IGUIManager;
class AntFile;
class GUIFont;

class AnxOptions
{
public:

	struct NodeOptions
	{
		dword defBkg;			//Цвет фона по умолчанию
		dword defSelBkg;		//Цвет фона по умолчанию для выделенного нода
		dword hrcBkg;			//Цвет фона для иерархического нода
		dword hrcSelBkg;		//Цвет фона для выделенного для иерархического нода
		dword grpBkg;			//Цвет фона для групирующего нода
		dword grpSelBkg;		//Цвет фона для выделенного для групирующего нода
		dword inBkg;			//Цвет фона для входного нода
		dword inSelBkg;			//Цвет фона для выделенного для входного нода
		dword outBkg;			//Цвет фона для выходного нода
		dword outSelBkg;		//Цвет фона для выделенного для выходного нода
		dword defFrm;			//Цвет рамки по умолчанию
		dword w;				//Ширина нода в нормальном состоянии
		dword h;				//Высота нода в нормальном состоянии
		string fNameLabel;		//Имя фонта для надписей на ноде
		GUIFont * fontLabel;	//Надпись на ноде
	};

	struct LinkOptions
	{
		dword defBkg;			//Цвет фона по умолчанию
		dword defSelBkg;		//Цвет фона по умолчанию для выделенного линка
		dword fakeBkg;			//Цвет фона для фэйкового линка
		dword fakeSelBkg;		//Цвет фона для выделенного фэйкового линка
		dword defFrm;			//Цвет рамки по умолчанию		
		dword defActFrm;		//Цвет рамки по умолчанию для выделенного исходного нода
	};
	
	struct EventForm
	{
		inline EventForm() : params(_FL_){};
		~EventForm();

		struct Param
		{
			inline Param() : enumValue(_FL_){};
			string name;		//Имя параметра
			string value;		//Значение параметра, если он константный
			array<string> enumValue;
		};

		string descr;			//Описание формы
		string name;			//Название события
		array<Param *> params;	//Параметры события
		bool noBuffered;		//Не буферезированный эвент
	};

	struct ConstForm
	{
		inline ConstForm() : params(_FL_){};
		~ConstForm();

		enum Type
		{
			t_string,
			t_float,
			t_blend,
		};

		struct Param
		{
			inline Param() : venum(_FL_){};
			string name;			//Имя константы
			string info;			//Описание параметра
			Type type;				//Тип параметра
			array<string> venum;	//Возможные значения при перечислении
		};

		string descr;			//Описание формы
		string type;			//Название формы
		array<Param *> params;	//Параметры события
	};


	struct FilesPath
	{
		string project;
		string ant;
		string anx;
		string gmx;
		string prt;
		string events;
	};

//--------------------------------------------------------------------------------------------
public:
	AnxOptions();
	virtual ~AnxOptions();
	void Init(IRender * render);

//--------------------------------------------------------------------------------------------
public:
	//Общии параметры
	dword screenBkg;
	//Параметры нодов
	NodeOptions node;
	//Параметры линков
	LinkOptions link;
	//Текущиц проект
	AnxProject * project;
	//Менеджер интерфейсов
	IGUIManager * gui_manager;
	//Первый в списке файлов
	AntFile * firstAnt;
	//Текущие параметры
	long width, height;			//Размеры экрана
	
	//Допустимые ошибки при компресии треков
	float quaternionError;
	float positionError;
	float scaleError;

	//Строка для различных манипуляций
	string stmp;

	//Фонт для просмоторщика
	string fNameViewer;
	GUIFont * fontViewer;
	//Большой фонт
	string fNameBig;
	GUIFont * fontBig;

	//Формы для событий
	array<EventForm *> eventForms;
	//Формы для констант
	array<ConstForm *> constsForms;

	//Библиотечные имена команд для линка
	array<string> linkCommands;

	//Пути до файлов в диалогах открытия и сохранения
	FilesPath path;
	void UpdatePath();	//Сохранить пути в ini

	Compressor compressor;

	//Показывать ли расширеную информацию на графе
	bool IsShowInfo();

//--------------------------------------------------------------------------------------------
public:
	IFileService * fileService;				//Файловый сервис
	IAnimationService * animationService;	//Сервис анимации
	IGMXService * geometryService;			//Сервис геометрии	
	IRender * render;						//Рисовалка
	IControlsService * controlService;		//Сервис ввода
	IControls * controls;

//--------------------------------------------------------------------------------------------
private:
	//Загрузить шаблоны форм для событий
	void LoadEventForms(IIniFile * ini);
	//Загрузить шаблоны форм для констант
	void LoadConstsForms(IIniFile * ini);
	//Загрузить имена для команд линков
	void LoadLinkCommands(IIniFile * ini);
	//Загрузить пути до файлов
	void LoadPath();
	//Прочитать строки из ini
	void ReadStrings(IIniFile * ini, const char * section, bool & isFirst, array<string> & str);
};

#endif

