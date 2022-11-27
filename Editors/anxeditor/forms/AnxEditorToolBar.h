//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxEditorToolBar	
//============================================================================================

#ifndef _AnxEditorToolBar_h_
#define _AnxEditorToolBar_h_

#include "..\AnxBase.h"

class AnxToolButton;
class AnxToolEdit;
class MasterSkeleton;

class AnxEditorToolBar : public GUIWindow
{
//--------------------------------------------------------------------------------------------
public:
	AnxEditorToolBar(AnxOptions & options);
	virtual ~AnxEditorToolBar();

//--------------------------------------------------------------------------------------------
public:
	//Рисование
	virtual void Draw();
	//Сообщения
	bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	void _cdecl UpdateHNodeName(GUIControl * sender);
	//Создать новый проект
	void _cdecl NewProject(GUIControl * sender);
	void _cdecl NewProjectCreateYes(GUIControl * sender);
	void _cdecl NewProjectSaveOk(GUIControl * sender);
	void _cdecl NewProjectSaveCancel(GUIControl * sender);
	//Загрузить проект
	void _cdecl OpenProject(GUIControl * sender);
	void _cdecl OpenProjectSaveCurrent(GUIControl * sender);
	void _cdecl OpenProjectProcessDialog(GUIControl * sender);
	void _cdecl OpenProjectProcess(GUIControl * sender);
	void _cdecl OpenProjectCancel(GUIControl * sender);
	//Сохранить проект
	void _cdecl SaveProject(GUIControl * sender);
	void _cdecl SaveProjectAs(GUIControl * sender);
	void _cdecl SaveProjectOk(GUIControl * sender);
	void _cdecl SaveProjectCancel(GUIControl * sender);
	void SaveProject(const char * name);
	//Выгрузить проект в anx
	void _cdecl ExportProject(GUIControl * sender);
	void _cdecl ExportProjectOk(GUIControl * sender);
	void _cdecl ExportProjectCancel(GUIControl * sender);
	//Импортировать другой проект
	void _cdecl ImportProject(GUIControl * sender);
	void _cdecl ImportProjectOk(GUIControl * sender);
	void _cdecl ImportProjectCancel(GUIControl * sender);
	//Импортировать звуковые события
	void _cdecl ImportEvents(GUIControl * sender);
	void _cdecl ImportEventsOk(GUIControl * sender);
	void _cdecl ImportEventsCancel(GUIControl * sender);
	//Импортировать ролик
	void _cdecl ImportMovie(GUIControl * sender);
	void _cdecl ImportMovieOk(GUIControl * sender);
	void _cdecl ImportMovieCancel(GUIControl * sender);
	//Перезагрузить анимационные клипы
	void _cdecl ReloadAnts(GUIControl * sender);
	void _cdecl ReloadAntsIsRecursive(GUIControl * sender);
	void _cdecl ReloadAntsIsNotRecursive(GUIControl * sender);
	void _cdecl ReloadAntsSelPath(GUIControl * sender);
	void _cdecl ReloadAntsOk(GUIControl * sender);
	void _cdecl ReloadAntsCancel(GUIControl * sender);
	//Установить мастер-скелет
	void _cdecl SetMasterSkeleton(GUIControl * sender);
	void _cdecl LoadMasterSkeletonOk(GUIControl * sender);
	void _cdecl LoadMasterSkeletonCancel(GUIControl * sender);
	//Установить модельку для предпросмотра
	void _cdecl SetViewerModel(GUIControl * sender);
	//Установить партиклы
	void _cdecl SetParticles(GUIControl * sender);
	void _cdecl SetParticlesOk(GUIControl * sender);
	void _cdecl SetParticlesCancel(GUIControl * sender);
	//Показать меню нодов
	void _cdecl ShowCreateNodesMenu(GUIControl * sender);
	//Показать список нодов
	void _cdecl ShowGotoNodesList(GUIControl * sender);
	//Перейти на нод
	void _cdecl SelectGotoNode(GUIControl * sender);
	//Выйти из редактора
	void _cdecl EvtExitFromEditor(GUIControl * sender);
	void _cdecl EvtExitFromEditorOk(GUIControl * sender);

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	GUILabel * label;
	AnxToolButton * buttonCreateNodes;
	AnxToolButton * buttonGotoNode;
	MasterSkeleton * skeleton;
	GUIFileOpen * dfo;
	GUIFileSave * dfs;
	float saveViewTime;
	AnxToolEdit * editHNodeName;
	bool isRecursiveAntsReload;
};

#endif

