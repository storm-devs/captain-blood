//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// AnxAnimationViewer
//============================================================================================

#ifndef _AnxAnimationViewer_h_
#define _AnxAnimationViewer_h_

#include "..\AnxBase.h"
#include "..\..\..\Common_h\AnimationStdEvents.h"

class AnxToolButton;
class AnxToolEdit;

class AnxAnimationViewer : public GUIControl
{
//--------------------------------------------------------------------------------------------
public:
	AnxAnimationViewer(AnxOptions & options, GUIControl * parent, long x, long y, long w, long h);
	~AnxAnimationViewer();

	//Создать панель управления
	void ShowControlPanel();

//--------------------------------------------------------------------------------------------
public:
	//Нарисовать
	virtual void Draw();
	//Сообщения
	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);
	//Установить анимацию
	bool SetAnimation(void * data, dword size);
	//Установить новую скорость проигрывания анимации
	void SetNewFPS(float fps);
	//Получить позицию кадра на временной полосе в экранных координатах
	long GetFramePosition(long frame, long & y, long & h, bool isClamp = false);

//--------------------------------------------------------------------------------------------
//Кнопки проигрывания
//--------------------------------------------------------------------------------------------
private:
	void _cdecl GoToStart(GUIControl * sender);
	void _cdecl GoToPrevFrame(GUIControl * sender);
	void _cdecl PlayStop(GUIControl * sender);
	void _cdecl GoToNextFrame(GUIControl * sender);
	void _cdecl GoToEnd(GUIControl * sender);

	GUIImage imagePlay;
	GUIImage imageStop;
	bool isPlay;
	AnxToolButton * playButton;

//--------------------------------------------------------------------------------------------
//Разрешить-запретить перемещения персонажа
//--------------------------------------------------------------------------------------------
private:
	void _cdecl EnableMovement(GUIControl * sender);
	GUIImage movementEnable;
	GUIImage movementDisable;
	bool isMovement;
	AnxToolButton * movementButton;

//--------------------------------------------------------------------------------------------
//Разрешить-запретить слежение за персонажем
//--------------------------------------------------------------------------------------------
private:
	void _cdecl EnableTargeting(GUIControl * sender);
	GUIImage targetingEnable;
	GUIImage targetingDisable;
	bool isTargeting;
	AnxToolButton * targetingButton;

//--------------------------------------------------------------------------------------------
//Рисовать модельку или скилет
//--------------------------------------------------------------------------------------------
private:
	void _cdecl SwitchModel(GUIControl * sender);
	GUIImage showModel;
	GUIImage showSkeleton;
	bool isModel;
	AnxToolButton * modelButton;

//--------------------------------------------------------------------------------------------
//Включить-выключить события
//--------------------------------------------------------------------------------------------
private:
	void _cdecl EnableEvents(GUIControl * sender);
	GUIImage eventsEnable;
	GUIImage eventsDisable;
	bool isEvents;
	AnxToolButton * eventsButton;

//--------------------------------------------------------------------------------------------
//Полоса с кадрами
//--------------------------------------------------------------------------------------------
private:	
	void DrawTimeLine();
	void _cdecl SetNewFrame(GUIControl * sender);
	bool CheckTimeLineClick(const GUIPoint & point);
	AnxToolEdit * eFrame;
	GUIRectangle timeLineRect;

//--------------------------------------------------------------------------------------------
//Доступ к анимации
//--------------------------------------------------------------------------------------------
private:
	//Установить текущий кадр анимации
	void AnimationSetCurrentFrame(float frame);
	//Получить текущий кадр анимации
	float AnimationGetCurrentFrame();
	//Получить количество кадров в анимации
	dword AnimationGetNumberOfFrames();


//--------------------------------------------------------------------------------------------
//Разное
//--------------------------------------------------------------------------------------------
private:
	void _cdecl ResetCamPosition(GUIControl * sender);
	void _cdecl ResetManPosition(GUIControl * sender);

//--------------------------------------------------------------------------------------------
public:
	dword clipSize;
	float clipSRate;
	dword clipFrames;
	dword asignedBones;
	dword totalBones;
	dword globalPosBones;

//--------------------------------------------------------------------------------------------
private:	
	IAnxEditorServiceAccess * iesa;	//Сервис для создания анимации
	IParticleManager * particles;	//Менеджер партиклов для редактора
	ISoundScene * sounds;			//Звуковой контекст для редактора
	IAnimationScene * aniScene;		//Анимационная сцена с которой работаем
	AnxOptions & opt;
	string curAniFileName;			//Имя временного файла анимации, с которым работаем
	IGMXScene * model;				//Модель для просмотра	
	IAnimation * loadAnimation;		//Анимация для просмотра если нет модели
	void * animationData;			//Анимационные данные
	//GUIPoint oldPosition;			//Позиция курсора при переходи в режим свободного полёта
	bool isCameraView;				//Режим свободного полёта
	Vector position;				//Позиция скелета
	float angle;					//Угол скелета
	AnimationStdEvents events;		//Стандартный обработчик событий
	static dword createCounter;		//Счётчик созданный файлов анимации
};

#endif

