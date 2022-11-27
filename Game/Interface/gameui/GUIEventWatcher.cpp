#include "GUIEventWatcher.h"
#include "../utils/InterfaceUtils.h"

GUIEventWatcher:: GUIEventWatcher(void)
{
	m_target.Reset();

	m_showState	  = false;
	m_activeState = false;
	m_focusState  = false;

	m_fadeState = BaseGUIElement::FadingFinished;

	m_active = false;
}

GUIEventWatcher::~GUIEventWatcher(void)
{
}

void GUIEventWatcher::Restart()
{
	Show(true);
	Activate(m_active);
}

void GUIEventWatcher::ReadMOPs(MOPReader &reader)
{
	m_params.subject = reader.String();

	m_params.watchShowOn	 = reader.Bool();
	m_params.watchShowOff	 = reader.Bool();
	m_params.watchActivate	 = reader.Bool();
	m_params.watchDeactivate = reader.Bool();
	m_params.watchFocusOn	 = reader.Bool();
	m_params.watchFocusOff	 = reader.Bool();
	m_params.watchFadeOn	 = reader.Bool();
	m_params.watchFadeOff	 = reader.Bool();

	m_params.eventWorker.Init(reader);

	Activate(m_active = reader.Bool());
}

bool GUIEventWatcher::Create(MOPReader &reader)
{
	return EditMode_Update(reader);
}

bool GUIEventWatcher::EditMode_Update(MOPReader &reader)
{
	ReadMOPs(reader);

	if( FindObject(m_params.subject,m_target))
	{
		if( m_target.Ptr()->Is(InterfaceUtils::GetBaseId()))
		{
			BaseGUIElement *p = (BaseGUIElement *)m_target.Ptr();

			m_showState	  = p->IsShow();
			m_activeState = p->IsActive();
			m_focusState  = p->HaveFocus();
			m_fadeState	  = p->GetFadeState();
		}
		else
			m_target.Reset();
	}

	return true;
}

void GUIEventWatcher::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	DelUpdate(&GUIEventWatcher::Work);

	if( isActive )
		SetUpdate(&GUIEventWatcher::Work,ML_EXECUTE_END);
}

void GUIEventWatcher::Work(float deltaTime, long)
{
	if( EditMode_IsOn())
	{
		m_target.Validate();
	}

	BaseGUIElement *target = (BaseGUIElement *)m_target.Ptr();

	if( !target )
		return;

	if( m_params.watchShowOn && target->IsShow() && !m_showState)
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if( m_params.watchShowOff && !target->IsShow() && m_showState )
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if( m_params.watchActivate && target->IsActive() && !m_activeState )
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if( m_params.watchDeactivate && !target->IsActive() && m_activeState )
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if (m_params.watchFocusOn && target->HaveFocus() && !m_focusState)
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if( m_params.watchFocusOff && !target->HaveFocus() && m_focusState )
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if( m_params.watchFadeOn &&
		(target->GetFadeState() == BaseGUIElement::FadingIn || target->GetFadeState() == BaseGUIElement::FadingOut) &&
		m_fadeState == BaseGUIElement::FadingFinished )
	{
		m_params.eventWorker.Activate(Mission(),false);
	}
	if( m_params.watchFadeOff &&
		target->GetFadeState() == BaseGUIElement::FadingFinished &&
		(m_fadeState == BaseGUIElement::FadingIn || m_fadeState == BaseGUIElement::FadingOut))
	{
		m_params.eventWorker.Activate(Mission(),false);
	}

	m_showState	  = target->IsShow();
	m_activeState = target->IsActive();
	m_focusState  = target->HaveFocus();
	m_fadeState	  = target->GetFadeState();
}


static char GUIDescription[] =
"GUI Event watcher.\n\n"
"Monitors GUI element and triggers on state change.";

MOP_BEGINLISTCG(GUIEventWatcher, "GUI Event Watcher", '1.00', 10000, GUIDescription, "Interface")

	MOP_STRING("Object id", "")

	MOP_BOOL("Watch visible on"	, false)
	MOP_BOOL("Watch visible off", false)
	MOP_BOOL("Watch activate"	, false)
	MOP_BOOL("Watch deactivate"	, false)
	MOP_BOOL("Watch get focus"	, false)
	MOP_BOOL("Watch loose focus", false)
	MOP_BOOL("Watch begin fade"	, false)
	MOP_BOOL("Watch end fade"	, false)

	MOP_MISSIONTRIGGER("")

	MOP_BOOL("Active", true)

MOP_ENDLIST(GUIEventWatcher)
