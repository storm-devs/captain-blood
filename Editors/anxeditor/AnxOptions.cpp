//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxOptions
//============================================================================================


#include "AnxOptions.h"
#include "AnxBase.h"

//============================================================================================

AnxOptions::AnxOptions() : eventForms(_FL_),
							constsForms(_FL_),
							linkCommands(_FL_)
{
	//Общии параметры
	screenBkg = 0xff808080;
	//Параметры нода
	node.defBkg = 0xff101080;
	node.defSelBkg = 0xff5050c0;
	node.hrcBkg = 0xff801010;
	node.hrcSelBkg = 0xffc05050;
	node.grpBkg = 0xff108010;
	node.grpSelBkg = 0xff50c050;
	node.inBkg = 0xff108080;
	node.inSelBkg = 0xff50c0c0;
	node.outBkg = 0xff801080;
	node.outSelBkg = 0xffc050c0;
	node.defFrm = 0xffffffff;
	node.w = 128;
	node.h = 48;
	node.fNameLabel = "AnxSmallFont";
	node.fontLabel = null;
	//
	link.defBkg = 0xff008000;
	link.defSelBkg = 0xff30ff30;
	link.fakeBkg = 0xff808000;
	link.fakeSelBkg = 0xffffff30;
	link.defFrm = 0xffffffff;
	link.defActFrm = 0xffff0000;
	//
	width = height = 0;
	project = null;
	gui_manager = null;
	//
	firstAnt = null;
	//Допустимые ошибки при компресии треков
	quaternionError = 0.00001f;
	positionError = 0.0001f;
	scaleError = 0.0001f;
	animationService = null;
	geometryService = null;
	fileService = null;
	render = null;
	controlService = null;
	controls = null;
	//
	fNameViewer = "AnxSmallFont";
	fontViewer = null;
	fNameBig = "SansSerif";
	fontBig = null;
}

AnxOptions::~AnxOptions()
{
	controls->Release();
	controls = null;
	if(node.fontLabel) delete node.fontLabel;
	if(fontViewer) delete fontViewer;
	if(fontBig) delete fontBig;
	for(long i = 0; i < eventForms; i++) delete eventForms[i];
	eventForms.Empty();
	for(i = 0; i < constsForms; i++) delete constsForms[i];
	constsForms.Empty();	
	for(i = 0; i < linkCommands; i++) delete linkCommands[i];
	linkCommands.Empty();
}

AnxOptions::EventForm::~EventForm()
{
	for(long i = 0; i < params; i++) delete params[i];
	params.Empty();
}

AnxOptions::ConstForm::~ConstForm()
{
	for(long i = 0; i < params; i++) delete params[i];
	params.Empty();
}

void AnxOptions::Init(IRender * render)
{
#ifndef NO_TOOLS
	this->render = render;
	//Параметры нода
	node.fontLabel = NEW GUIFont(node.fNameLabel);
	width = render->GetScreenInfo3D().dwWidth;
	height = render->GetScreenInfo3D().dwHeight;
	fontViewer = NEW GUIFont(fNameViewer);
	fontBig = NEW GUIFont(fNameBig);
	//Сервисы
	animationService = (IAnimationService *)api->GetService("AnimationService");
	geometryService = (IGMXService *)api->GetService("GMXService");
	controlService = (IControlsService *)api->GetService("ControlsService");
	controls = controlService->CreateInstance(_FL_);
	IAnxEditorServiceAccess * iesa = (IAnxEditorServiceAccess *)api->GetService("AnxEditorServiceAccess");
	Assert(iesa);
	iesa->DisableThreads();
	//Загружаем параметры из AnxEditor.ini
	fileService = (IFileService *)api->GetService("FileService");
	Assert(fileService);
	IIniFile * ini = fileService->OpenIniFile("AnxEditor.ini", _FL_);
	if(ini)
	{
		LoadEventForms(ini);
		LoadConstsForms(ini);
		LoadLinkCommands(ini);
		ini->Release(); ini = null;
	}
	LoadPath();
	compressor.Prepare(Compressor::cm_fast);
#endif
}

//Сохранить пути в ini
void AnxOptions::UpdatePath()
{
	IEditableIniFile * ini = fileService->OpenEditableIniFile("AnxEditorPathes.ini", file_open_always, _FL_);
	if(ini)
	{
		ini->SetString("AnxFilesPath", "project", (char *)path.project.GetBuffer());
		ini->SetString("AnxFilesPath", "ant", (char *)path.ant.GetBuffer());
		ini->SetString("AnxFilesPath", "anx", (char *)path.anx.GetBuffer());
		ini->SetString("AnxFilesPath", "gmx", (char *)path.gmx.GetBuffer());
		ini->SetString("AnxFilesPath", "prt", (char *)path.prt.GetBuffer());
		ini->SetString("AnxFilesPath", "events", (char *)path.events.GetBuffer());
		ini->Release(); ini = null;
	}
}

//Показывать ли расширеную информацию на графе
bool AnxOptions::IsShowInfo()
{
	return GetAsyncKeyState(VK_SPACE) < 0 && GetAsyncKeyState(VK_CONTROL) < 0;
}

//============================================================================================

//Загрузить шаблоны форм для событий
void AnxOptions::LoadEventForms(IIniFile * ini)
{
	bool isFirst = true;
	array<string> str(_FL_);
	while(true)
	{
		//Форма
		EventForm * form = NEW EventForm();
		//Ищим начало описание события
		while(true)
		{
			ReadStrings(ini, "AnxEvents", isFirst, str);
			if(!str)
			{
				delete form;
				return;
			}
			if(str[0] == "begin")
			{
				if(str != 2 || !str[1].Len())
				{
					api->Trace("Find invalidate event begin description, skip this event...");
				}else{
					break;
				}
			}
		}
		form->descr = str[1];
		str.DelAll();
		//Получаем имя
		ReadStrings(ini, "AnxEvents", isFirst, str);
		if(!str || str != 2 || !str[1].Len() || (str[0] != "name" && str[0] != "name_nobuffered"))
		{
			api->Trace("Find invalidate event name, description: %s", form->descr.GetBuffer());
			delete form;
			return;
		}
		form->name = str[1];
		form->noBuffered = (str[0] == "name_nobuffered");
		//Читаем параметры
		while(true)
		{
			ReadStrings(ini, "AnxEvents", isFirst, str);
			if(!str)
			{
				api->Trace("Event not ended. Description: %s", form->descr.GetBuffer());
				delete form;
				return;
			}
			if(str[0] == "param")
			{
				if(str < 2 || str > 3)
				{
					api->Trace("Find invalidate event. Description: %s", form->descr.GetBuffer());
					continue;
				}
				EventForm::Param * p = NEW EventForm::Param();
				p->name = str[1];
				if(str == 3) p->value = str[2];
				form->params.Add(p);
			}else
			if(str[0] == "enum")
			{
				if(str < 4)
				{
					api->Trace("Find invalidate event. Description: %s", form->descr.GetBuffer());
					continue;
				}
				EventForm::Param * p = NEW EventForm::Param();
				p->name = str[1];				
				for(long j = 2; j < str; j++)
				{
					p->enumValue[p->enumValue.Add()] = str[j];
				}
				form->params.Add(p);
			}else
			if(str[0] == "end")
			{
				break;
			}else{
				api->Trace("Find invalidate event. Description: %s", form->descr.GetBuffer());
				delete form;
				return;
			}
		}
		eventForms.Add(form);
	}
}

//Загрузить шаблоны форм для констант
void AnxOptions::LoadConstsForms(IIniFile * ini)
{
	bool isFirst = true;
	array<string> str(_FL_);
	while(true)
	{
		//Форма
		ConstForm * form = NEW ConstForm();
		//Ищим начало описание события
		while(true)
		{
			ReadStrings(ini, "AnxConsts", isFirst, str);
			if(!str)
			{
				delete form;
				return;
			}
			if(str[0] == "begin")
			{
				if(str != 3 || !str[1].Len() || !str[2].Len())
				{
					api->Trace("Find invalidate consts form begin description, skip this form...");
				}else{
					break;
				}
			}
		}
		form->type = str[1];
		form->descr = str[2];		
		str.DelAll();
		//Читаем параметры
		while(true)
		{
			ReadStrings(ini, "AnxConsts", isFirst, str);
			if(!str)
			{
				api->Trace("Consts form not ended. Description: %s", form->descr.GetBuffer());
				delete form;
				return;
			}
			if(str[0] == "string" || str[0] == "float" || str[0] == "blend")
			{
				if(str != 3)
				{
					api->Trace("Find invalidate consts form. Description: %s", form->descr.GetBuffer());
					continue;
				}
				ConstForm::Param * p = NEW ConstForm::Param();
				p->name = str[1];
				p->info = str[2];
				p->type = ConstForm::t_string;
				if(str[0] == "float") p->type = ConstForm::t_float;
				if(str[0] == "blend") p->type = ConstForm::t_blend;
				form->params.Add(p);
			}else
			if(str[0] == "enumstring" || str[0] == "enumfloat" || str[0] == "enumblend")
			{
				if(str < 5)
				{
					api->Trace("Find invalidate consts form. Description: %s", form->descr.GetBuffer());
					continue;
				}
				ConstForm::Param * p = NEW ConstForm::Param();
				p->name = str[1];
				p->info = str[2];
				p->type = ConstForm::t_string;
				if(str[0] == "enumfloat") p->type = ConstForm::t_float;
				if(str[0] == "enumblend") p->type = ConstForm::t_blend;
				for(long j = 3; j < str; j++)
				{
					p->venum.Add(str[j]);
				}
				form->params.Add(p);
			}else
			if(str[0] == "end")
			{
				break;
			}else{
				api->Trace("Find invalidate consts form. Description: %s", form->descr.GetBuffer());
				delete form;
				return;
			}
		}
		constsForms.Add(form);
	}
}

//Загрузить имена для команд линков
void AnxOptions::LoadLinkCommands(IIniFile * ini)
{
	if(!ini) return;
	ini->GetStrings("AnxLinkCommands", "string", linkCommands);
}

//Загрузить пути до файлов
void AnxOptions::LoadPath()
{
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * ini = fs->OpenIniFile("AnxEditorPathes.ini", _FL_);
	if(!ini) return;
	//Путь по проектов
	path.project = ini->GetString("AnxFilesPath", "project", path.project.c_str());
	//Путь до ant
	path.ant = ini->GetString("AnxFilesPath", "ant", path.ant.c_str());
	//Путь до anx
	path.anx = ini->GetString("AnxFilesPath", "anx", path.anx.c_str());
	//Путь до gmx
	path.gmx = ini->GetString("AnxFilesPath", "gmx", path.gmx.c_str());
	//Путь до particles/*.prj
	path.prt = ini->GetString("AnxFilesPath", "prt", path.prt.c_str());
	//Путь до импорта эвентов
	path.events = ini->GetString("AnxFilesPath", "prt", path.events.c_str());
	ini->Release(); ini = null;
}

//Прочитать строки из ini
void AnxOptions::ReadStrings(IIniFile * ini, const char * section, bool & isFirst, array<string> & str)
{
	static long index = 0;
	str.DelAll();
	if(!ini) return;
	const char * buf = null;
	if(isFirst)
	{
		index = 0;
		isFirst = false;
		buf = ini->GetString(section, "string", null, index++);
		if(!buf) return;
	}else{
		buf = ini->GetString(section, "string", null, index++);
		if(!buf) return;
	}
	dword idx = str.Add();
	for(dword i = 0; buf[i] != 0; i++)
	{
		if(buf[i] == '$')
		{
			idx = str.Add();
			continue;
		}
		str[idx] += buf[i];
	}
}

