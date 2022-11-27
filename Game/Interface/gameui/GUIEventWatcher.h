#pragma once

#include "../../..\common_h/Mission.h"

#include "BaseGUIElement.h"

class GUIEventWatcher :	public MissionObject
{
public:

	struct Params
	{
		ConstString subject;

		bool watchShowOn;		// мониторить включение видимости
		bool watchShowOff;		// мониторить выключение видимости
		bool watchActivate;		// мониторить активацию
		bool watchDeactivate;	// мониторить деактивацию
		bool watchFocusOn;		// мониторить получение фокуса
		bool watchFocusOff;		// мониторить потерю фокуса
		bool watchFadeOn;		// мониторить начало фэйда
		bool watchFadeOff;		// мониторить конец фэйда
		
		MissionTrigger eventWorker;	// реализация обработки событий
	};

private:

	Params						m_params;

	MOSafePointer				m_target;		// за кем наблюдаем

	bool						m_showState;	// состояние видимости в прошлом кадре
	bool						m_activeState;	// состояние активности в прошлом кадре
	bool						m_focusState;	// состояние фокуса в прошлом кадре
	BaseGUIElement::FadeState	m_fadeState;	// состояние фэйда в прошлом кадре

	bool						m_active;

	void ReadMOPs(MOPReader &reader);

	void _cdecl Work(float deltaTime, long level);

public:

	GUIEventWatcher(void);
	virtual ~GUIEventWatcher(void);

	//////////////////////////////////////////////////////////////////////////

	virtual bool Create			(MOPReader &reader);
	virtual bool EditMode_Update(MOPReader &reader);

	virtual void Restart();

	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(GUIEventWatcher, MissionObject);

};
