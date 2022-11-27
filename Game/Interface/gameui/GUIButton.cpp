#include "GUIButton.h"
#include "GUIWidget.h"
#include "GUISlider.h"

#include "../utils/InterfaceUtils.h"
#include "../../..\common_h/gui/controls/gui_helper.h"

#include "BaseGUICursor.h"

const float  press_delay = 0.10f;
const float repeat_delay = 0.01f;

static inline float SC2UIRC(float t)
{
	return InterfaceUtils::ScreenCoord2UIRectCoord(t);
}

static inline float SS2UIRS(float t)
{
	return InterfaceUtils::ScreenSize2UIRectSize(t);
}

GUIButton::GUIButton(void)
{
	m_restart = false;

	m_staticWidget	 = null;
	m_selectedWidget = null;
	m_normalWidget	 = null;
	m_focusedWidget	 = null;
	m_pressedWidget	 = null;
	m_disabledWidget = null;
	m_tooltipWidget	 = null;

	m_wasPressed  = false;
	m_wasReleased = true;

	m_activeWidget = null;

	m_doNotCheckFocusChange = false;
	m_focusTime = 0.0f;

	m_validateHash = -1;

//	drawLevel = ML_GUI2;
	drawLevel = ML_GUI1;

	drawPriority = 0;

	m_mouseOver = false;

	m_wasPressed  = false;
	m_wasReleased = false;

	m_repeat = false;

	m_pressDelay = 0.0f;
}

GUIButton::~GUIButton(void)
{
	if( EditMode_IsOn())
	{
		static ConstString empty("");
		if( m_staticWidget ) m_staticWidget->SetParent(empty);
		if( m_selectedWidget ) m_selectedWidget->SetParent(empty);
		if( m_normalWidget ) m_normalWidget->SetParent(empty);
		if( m_focusedWidget ) m_focusedWidget->SetParent(empty);
		if( m_pressedWidget ) m_pressedWidget->SetParent(empty);
		if( m_disabledWidget ) m_disabledWidget->SetParent(empty);
	}

	if( BaseGUIElement *parent = GetParent())
		parent->UnRegister(this);
}

void GUIButton::Restart()
{
	if( HaveFocus())
		SetElementInFocus(null);

	m_restart = true;

	ReCreate();
}

void GUIButton::SkipMOPs(MOPReader& reader)
{
	reader.Float();		// width
	reader.Float();		// height

	m_params.description = reader.LocString();

	reader.String().c_str();	// desc object

	reader.String().c_str();	// static	widget
	reader.String().c_str();	// selected widget
	reader.String().c_str();	// normal	widget
	reader.String().c_str();	// focused	widget
	reader.String().c_str();	// pressed	widget
	reader.String().c_str();	// disabled widget

	reader.Float();		// press x offset
	reader.Float();		// press y offset

	////  links  ////

	dword count = reader.Array();

	for( dword i = 0 ; i < count ; i++ )
	{
		reader.String().c_str(); // control alias
		reader.String().c_str(); // object
	}

	//// hotkeys ////

	count = reader.Array();

	for( dword i = 0 ; i < count ; i++ )
	{
		reader.String().c_str(); // hotkey alias
		reader.Bool();	 // on focus only
	}

	////////////////

	reader.Bool();	// action by release

	m_params.buttonEvent.Init(reader);
//	m_params.inactiEvent.Init(reader);

	reader.Bool();	// autorepeat
	reader.Bool();	// autofocus

	onGetFocus.Init(reader);
	onLosFocus.Init(reader);

	reader.Bool();	// initially focused

	m_state = stNormal;

	reader.Bool();	// focus by press

	drawPriority = reader.Long();

	Activate(reader.Bool());

	nativeShow = reader.Bool();

	m_activeWidget = null;

	SetState(stNormal);
	m_pressed = false;

	m_wasPressed  = false;
	m_wasReleased = false;

	m_repeat = false;

	m_pressDelay = 0.0f;
}

void GUIButton::ReadMOPs(MOPReader& reader)
{
	float width	 = reader.Float();
	float height = reader.Float();

	if( !Restricted(preserveSize))
		SizeTo(width,height);

	m_natSize.w = width;
	m_natSize.h = height;

	m_params.description = reader.LocString();
	m_params.descObject = reader.String();

	m_params.staticWidget = reader.String();
	m_params.selectedButtonWidget = reader.String();
	m_params.normalButtonWidget = reader.String();
	m_params.focusButtonWidget = reader.String();
	m_params.pressedButtonWidget = reader.String();
	m_params.disabledButtonWidget = reader.String();

	m_staticWidget	 = null;
	m_pressedWidget  = null;
	m_focusedWidget  = null;
	m_disabledWidget = null;
	m_selectedWidget = null;
	m_normalWidget	 = null;

	m_params.pressXoffset = reader.Float();
	m_params.pressYoffset = reader.Float();

	m_params.links.DelAll();

	dword count = reader.Array();

	m_params.links.AddElements(count);
	for( dword i = 0 ; i < count ; i++ )
	{
		Params::NavigationLink & link = m_params.links[i];
		link.control = reader.String().c_str();
		link.object	 = reader.String();		
	}

	m_params.hotKeys.DelAll();

	count = reader.Array();

	Params::HotKey hotkey;

	for( dword i = 0 ; i < count ; i++ )
	{
		hotkey.hotKey	   = reader.String().c_str();
		hotkey.onlyInFocus = reader.Bool();

		m_params.hotKeys.Add(hotkey);
	}

	m_release = reader.Bool();

	m_params.buttonEvent.Init(reader);
//	m_params.inactiEvent.Init(reader);

	m_autorepeat = reader.Bool();
	m_autofocus	 = reader.Bool();

	if( m_release )					// в режиме срабатывания по отжатию
	{
		if( m_autorepeat )
		{
			LogicDebugError("Режим срабатывания по отжатию, автоповтор невозможен");

			m_autorepeat = false;	// автоповтор невозможен
		}
	}

	onGetFocus.Init(reader);
	onLosFocus.Init(reader);

	m_params.initiallyFocused = reader.Bool();

	m_state = stNormal;

/*	if( m_staticWidget ) m_staticWidget->Show(false);
	if( m_pressedWidget ) m_pressedWidget->Show(false);
	if( m_focusedWidget ) m_focusedWidget->Show(false);
	if( m_disabledWidget ) m_disabledWidget->Show(false);
	if( m_selectedWidget ) m_selectedWidget->Show(false);
	if( m_normalWidget ) m_normalWidget->Show(false);*/

	m_activeWidget = m_normalWidget;

	m_focusByPress = reader.Bool();

	drawPriority = reader.Long();

	Activate(reader.Bool());

//	Show		(reader.Bool());
	nativeShow = reader.Bool();

	m_activeWidget = null;

	SetState(stNormal);
	m_pressed = false;

	m_cursor = null;
	m_cursorHash = -1;

	m_wasPressed  = false;
	m_wasReleased = false;

	m_repeat = false;

	m_pressDelay = 0.0f;

/*	////

	if( !m_cursor )
		 m_cursor = (BaseGUICursor *)FindObject(InterfaceUtils::GetCursorName());

	m_baseCursor = m_cursor;*/
}

void GUIButton::SetDisabled()
{
/*	GUIWidget *p = m_disabledWidget;

	if( m_staticWidget && m_staticWidget != p ) m_staticWidget->Show(false);
	if( m_normalWidget && m_normalWidget != p ) m_normalWidget->Show(false);
	if( m_pressedWidget && m_pressedWidget != p ) m_pressedWidget->Show(false);
	if( m_focusedWidget && m_focusedWidget != p ) m_focusedWidget->Show(false);
	if( m_disabledWidget ) m_disabledWidget->Show(true);*/

	SetState(stDisabled);

	GUIWidget *p = m_disabledWidget;

//	if( m_activeWidget != p )
	{
		if( m_activeWidget && m_activeWidget != p )
			m_activeWidget->Show(false);

		if( p && (p != m_activeWidget || !p->IsShow()))
		{
			m_activeWidget = p; p->Show(true);
		}
	}
}

void GUIButton::SetFocused()
{
//	if( GetElementInFocus() == this )
//		return;

	bool newFocus = (GetElementInFocus() != this);

	if( newFocus )
	{
		if( GetElementInFocus())
			GetElementInFocus()->OnLooseFocus();

		SetElementInFocus(this);
	}

/*	GUIWidget *p = m_focusedWidget;

	if( m_staticWidget && m_staticWidget != p ) m_staticWidget->Show(false);
	if( m_normalWidget && m_normalWidget != p ) m_normalWidget->Show(false);
	if( m_pressedWidget && m_pressedWidget != p ) m_pressedWidget->Show(false);
	if( m_disabledWidget && m_disabledWidget != p ) m_disabledWidget->Show(false);
	if( m_focusedWidget ) m_focusedWidget->Show(true);*/

	if( m_state != stFocused && newFocus )
	{
	//	api->Trace("ON GET FOCUS: %s",GetObjectID().c_str());
		LogicDebug("Button getFocus trigger");
		onGetFocus.Activate(Mission(),false);

		SetState(stFocused);

		if( m_tooltipWidget )
		{
			const char *param[1];

			param[0] = m_params.description;

			m_tooltipWidget->Command("SetString",1,param);
			m_tooltipWidget->Show(true);
		}

	/*	GUIWidget *p = m_focusedWidget;

	//	api->Trace("\n    %u %u\n",m_activeWidget,p);

		if( m_activeWidget != p )
		{
			if( m_activeWidget )
				m_activeWidget->Show(false);

			if( p )
			{
				m_activeWidget = p; p->Show(true);
			}
		}*/

		NotifyParent(this,ChildSelected);

		if( m_cursor && !(m_cursor->IsMoved() || m_cursor->IsClicked()))
			SetCursor();
	}

	GUIWidget *p = m_focusedWidget;

//	api->Trace("\n    %u %u\n",m_activeWidget,p);

//	if( m_activeWidget != p )
	{
		if( m_activeWidget && m_activeWidget != p )
			m_activeWidget->Show(false);

		if( p && (p != m_activeWidget || !p->IsShow()))
		{
			m_activeWidget = p; p->Show(true);
		}
	}
}

void GUIButton::SetNormal(bool reset)
{
	if( !IsActive())
	{
		SetDisabled();
		return;
	}

	GUIWidget *p = m_normalWidget;

	//// !!!УБРАТЬ ЭТО ЗЛО К Ч МАТЕРИ!!! ////

	if( m_staticWidget && m_staticWidget != p && m_staticWidget->IsShow()) m_staticWidget->Show(false);
	if( m_pressedWidget && m_pressedWidget != p && m_pressedWidget->IsShow()) m_pressedWidget->Show(false);
	if( m_focusedWidget && m_focusedWidget != p && m_focusedWidget->IsShow()) m_focusedWidget->Show(false);
	if( m_disabledWidget && m_disabledWidget != p && m_disabledWidget->IsShow()) m_disabledWidget->Show(false);
	if( m_selectedWidget && m_selectedWidget != p && m_selectedWidget->IsShow()) m_selectedWidget->Show(false);

	/////////////////////////////////////////

	if( reset )
		m_activeWidget = null;

	SetState(stNormal);

//	if( m_activeWidget != p )
	{
		if( m_activeWidget && m_activeWidget != p && m_activeWidget->IsShow())
			m_activeWidget->Show(false);

		if( p && (p != m_activeWidget || !p->IsShow()))
		{
			m_activeWidget = p; p->Show(true);
		}
	}
}

void GUIButton::SetSelected()
{
	SetState(stNormal);

	GUIWidget *p = m_selectedWidget;

//	if( m_activeWidget != p )
	{
		if( m_activeWidget && m_activeWidget != p )
			m_activeWidget->Show(false);

		if( p && (p != m_activeWidget || !p->IsShow()))
		{
			m_activeWidget = p; p->Show(true);
		}
	}
}

void GUIButton::SetPressed()
{
/*	GUIWidget *p = m_pressedWidget;

	if( m_staticWidget && m_staticWidget != p ) m_staticWidget->Show(false);
	if( m_normalWidget && m_normalWidget != p ) m_normalWidget->Show(false);
	if( m_focusedWidget && m_focusedWidget != p ) m_focusedWidget->Show(false);
	if( m_disabledWidget && m_disabledWidget != p ) m_disabledWidget->Show(false);
	if( m_pressedWidget )
	{
		m_pressedWidget->Show(true);
		m_pressedWidget->SetFrameOffset(m_params.pressXoffset, m_params.pressYoffset);
	}*/

	SetState(stPressed);

	GUIWidget *p = m_pressedWidget;

//	if( m_activeWidget != p )
	{
		if( m_activeWidget && m_activeWidget != p )
			m_activeWidget->Show(false);

		if( p && (p != m_activeWidget || !p->IsShow()))
		{
			m_activeWidget = p; p->Show(true);
		}
	}

	m_pressTime = press_delay;
}

void GUIButton::UnRegister(BaseGUIElement* object)
{
	BaseGUIElement::UnRegister(object);

	if( object == m_staticWidget ) m_staticWidget = null;
	if( object == m_selectedWidget ) m_selectedWidget = null;
	if( object == m_normalWidget ) m_normalWidget = null;
	if( object == m_focusedWidget ) m_focusedWidget = null;
	if( object == m_pressedWidget ) m_pressedWidget = null;
	if( object == m_disabledWidget ) m_disabledWidget = null;
	if( object == m_tooltipWidget ) m_tooltipWidget = null;
}

void GUIButton::OnHotKey()
{
	SetFocused();
	SetPressed(); m_wasPressed = true;
}

bool GUIButton::GoToLink(const char *control)
{
	for( dword i = 0 ; i < m_params.links.Size() ; i++ )
	{
		if( string::IsEqual(m_params.links[i].control,control))
		{
			MissionObject *mo = FindObject(m_params.links[i].object);

			if( mo && mo->IsShow())
			{
				if( mo->Is(InterfaceUtils::GetButtonId()))
				{
					GUIButton *p = (GUIButton *)mo;

					if( p->IsActive() && p->ParentIsActive())
					{					
						p->OnAcquireFocus();

						for( unsigned int k = 0 ; k < m_params.hotKeys.Size() ; ++k )
							if( string::IsEqual(m_params.links[i].control,m_params.hotKeys[k].hotKey))
								OnHotKey(),k = m_params.hotKeys.Size();

						return true;
					}
					else
					{
						return p->GoToLink(control);
					}
				}
				else
				if( mo->Is(InterfaceUtils::GetSliderId()))
				{
					GUISlider *p = (GUISlider *)mo;

					if( p->IsActive() && p->ParentIsActive())
					{					
						p->OnAcquireFocus();

						for( unsigned int k = 0 ; k < m_params.hotKeys.Size() ; ++k )
							if( string::IsEqual(m_params.links[i].control,m_params.hotKeys[k].hotKey))
								OnHotKey(),k = m_params.hotKeys.Size();

						return true;
					}
					else
					{
						return p->GoToLink(control);
					}
				}
			}
		}			
	}

	return false;
}

bool GUIButton::ProcessKeyboard()
{
	bool wasKeyControl = false; // было ли управление клавишами

	if( EditMode_IsOn() || !IsActive() || !ParentIsActive())
		return wasKeyControl;

	ControlStateType state;

	// проверка хоткеев
	if( !m_doNotCheckFocusChange )
	{
		for( dword i = 0 ; i < m_params.hotKeys.Size() ; i++ )
		{
			state = Controls().GetControlStateType(m_params.hotKeys[i].hotKey);

			if( state == CST_ACTIVATED &&
				((m_params.hotKeys[i].onlyInFocus && HaveFocus()) || !m_params.hotKeys[i].onlyInFocus))
			{
			//	if( IsActive())
			//		OnHotKey();

				if( IsActive())
				{
					if( m_autofocus || m_focusByPress )
						SetFocused();

					SetPressed(); m_wasPressed = true;
				}

				wasKeyControl = true; m_pressed = true;

				break;
			}

			if( state == CST_INACTIVATED )
				m_pressed = false;

			if( state == CST_ACTIVE && HaveFocus() && m_pressed )
			{
				if( IsActive())
					SetPressed();

				wasKeyControl = true;

				if( m_autorepeat )
					m_pressDelay += api->GetDeltaTime();
			}
		//	else
		//		m_pressDelay = 0.0f;
		}
	}

	// если в фокусе и фокус получен не в этом кадре - то проверка перехода фокуса на другой элемент
	if( HaveFocus() && !m_doNotCheckFocusChange )
	{
		for( dword i = 0 ; i < m_params.links.Size() ; i++ )
		{
			if( Controls().GetControlStateType(m_params.links[i].control) == CST_ACTIVATED ||
				m_params.links[i].activeTime > 0.2f )
			{
				MissionObject *mo = FindObject(m_params.links[i].object);

			//	if( mo && mo->Is(InterfaceUtils::GetBaseId()) && mo->IsShow())
				if( mo && mo->IsShow())
				{
				/*	static_cast<BaseGUIElement*>(mo)->OnAcquireFocus();

					for( unsigned int k = 0 ; k < m_params.hotKeys.Size() ; ++k )
						if( string::IsEqual(m_params.links[i].control,m_params.hotKeys[k].hotKey))
							OnHotKey(),k = m_params.hotKeys.Size();

					wasKeyControl = true;
					return wasKeyControl;*/

					if( mo->Is(InterfaceUtils::GetButtonId()))
					{
						GUIButton *p = (GUIButton *)mo;

						if( p->IsActive() && p->ParentIsActive())
						{					
							p->OnAcquireFocus();

							for( unsigned int k = 0 ; k < m_params.hotKeys.Size() ; ++k )
								if( string::IsEqual(m_params.links[i].control,m_params.hotKeys[k].hotKey))
									OnHotKey(),k = m_params.hotKeys.Size();

							wasKeyControl = true;
							return wasKeyControl;
						}
						else
						{
							wasKeyControl = p->GoToLink(m_params.links[i].control);
							return wasKeyControl;
						}
					}
					else
					if( mo->Is(InterfaceUtils::GetSliderId()))
					{
						GUISlider *p = (GUISlider *)mo;

						if( p->IsActive() && p->ParentIsActive())
						{					
							p->OnAcquireFocus();

							for( unsigned int k = 0 ; k < m_params.hotKeys.Size() ; ++k )
								if( string::IsEqual(m_params.links[i].control,m_params.hotKeys[k].hotKey))
									OnHotKey(),k = m_params.hotKeys.Size();

							wasKeyControl = true;
							return wasKeyControl;
						}
						else
						{
							wasKeyControl = p->GoToLink(m_params.links[i].control);
							return wasKeyControl;
						}
					}
				}
			}
		}
	}

	m_wasPressed = wasKeyControl;
	return wasKeyControl;
}

void GUIButton::ProcessMouse()
{
	if( Controls().Locked())
		return;

	if( EditMode_IsOn() || !IsActive() || !ParentIsActive())
		return;

//	if( !m_cursor )
//		 m_cursor = (BaseGUICursor *)FindObject(InterfaceUtils::GetCursorName());

	if( !m_cursor || m_cursor->IsHided())
		return;

	float x,y; m_cursor->GetPosition(x,y);

	Rect rect; GetParentClipRect(rect);

	if( !PointInRect(x*100,y*100,rect))
		return;

	x *= Render().GetScreenInfo2D().dwWidth;
	y *= Render().GetScreenInfo2D().dwHeight;

	// проверка мыши
	long mouseX, mouseY;
//	mouseX = (long)Controls().GetControlStateFloat("MouseH");
//	mouseY = (long)Controls().GetControlStateFloat("MouseV");
	mouseX = (long)x;
	mouseY = (long)y;
//	bool lbPressed = Controls().GetControlStateBool("LeftMouseButton");
	ControlStateType state = Controls().GetControlStateType("LeftMouseButton");

	Rect r; GetRect(r);

	float left	 = SC2UIRC(GetX()/100.0f)*Render().GetViewport().Width  + Render().GetViewport().X;
	float right	 = left + SS2UIRS((r.r - r.l)/100.0f)*Render().GetViewport().Width;
	float top	 = SC2UIRC(GetY()/100.0f)*Render().GetViewport().Height + Render().GetViewport().Y;
	float bottom = top  + SS2UIRS((r.b - r.t)/100.0f)*Render().GetViewport().Height;

	if( mouseX >= left && mouseX <= right &&
		mouseY >= top  && mouseY <= bottom )
	{
	//	if( IsShow() && IsActive())
	//		api->Trace("MOUSE OVER: %s",GetObjectID().c_str());

		m_mouseOver = true;

		if( state == CST_ACTIVATED )
		{
			m_pressed = true;

			if( m_autofocus || m_focusByPress )
				SetFocused();
		}

		if( state == CST_INACTIVATED )
			m_pressed = false;

	//	if( lbPressed )
		if( state == CST_ACTIVE && m_pressed )
		{
			if( IsActive())
				SetPressed();

			m_wasPressed = true;

			if( m_autorepeat )
				m_pressDelay += api->GetDeltaTime();
		}
		else
		{
			if( m_autofocus && m_cursor && (m_cursor->IsMoved() || m_state != stFocused && m_cursor->IsClicked()))
				SetFocused();

			if( !m_cursor->IsMoved())
				 m_mouseOver = false;

			m_wasPressed = false;
		}

		if( m_state == stNormal )
			SetSelected();
	}
	else
	{
		m_mouseOver = false;

		if( IsActive())
		{
			if( !HaveFocus())
			{
				SetNormal();
				m_wasPressed = false;
			}
			else
			{
				SetFocused();
				m_wasPressed = false;
			}
		}
		else
		{
			SetDisabled();
			m_wasPressed = false;
		}

		m_pressed = false;
	}
/*
//	if( m_state == stNormal )
	{
		if( m_mouseOver )
			SetSelected();
		else
			SetNormal();
	}*/
}
#ifdef not_def
void _cdecl GUIButton::Work(float deltaTime, long level)
{
/*	if( !IsActive())
	{
		SetDisabled();
		return;
	}*/

	ControlStateType state;

	for( dword i = 0 ; i < m_params.links.Size() ; i++ )
	{
		state = Controls().GetControlStateType(m_params.links[i].control);

		if( state == CST_ACTIVE ||
			state == CST_ACTIVATED )
			m_params.links[i].activeTime += deltaTime;
		else
			m_params.links[i].activeTime = 0.0f;
	}

	if( !ProcessKeyboard())
		 ProcessMouse();

//	if( !m_wasReleased && !m_wasPressed )
//		 m_pressed = false;

	if( !EditMode_IsOn() && !m_doNotCheckFocusChange )
	{
		if( m_wasPressed && m_wasReleased )
		{
			if( IsActive())
			{
				m_params.buttonEvent.Activate(Mission(),false);

				m_pressDelay = 0.0f;

				m_repeat = false;
			}
			else
			{
			//	m_params.inactiEvent.Activate(Mission(),false);
			}
		}
		else
		{
			if( m_autorepeat )
			{
				if( m_repeat )
				{
					if( m_pressDelay > repeat_delay )
					{
						m_params.buttonEvent.Activate(Mission(),false);

						m_pressDelay = 0.0f;
					}
				}
				else
				{
					if( m_pressDelay > repeat_delay*100 )
					{
						m_params.buttonEvent.Activate(Mission(),false);

						m_pressDelay = 0.0f;

						m_repeat = true;
					}
				}
			}
		}
	}

	m_focusTime += deltaTime;

//	if( m_focusTime > 0.2f )
	if( m_focusTime > 0.0f )
		m_doNotCheckFocusChange = false;

	m_wasReleased = !m_wasPressed;

	Rect rect; GetRect(rect);

	float width	 = rect.r - rect.l;
	float height = rect.b - rect.t;

	float x = GetX();
	float y = GetY();

	if( EditMode_IsOn())
		m_helperQuad.DrawQuad(x,y,width,height,0,0,0,0,10.5f);

	float k = m_pressTime/press_delay;

	float x_off = Lerp(0.0f,m_params.pressXoffset,k);
	float y_off = Lerp(0.0f,m_params.pressYoffset,k);

	x += x_off*m_aspect;
	y += y_off;

	float r = x + width;
	float b = y + height;

	Rect clip; GetParentClipRect(clip);

	if( !RectInRect(
			clip.l,
			clip.t,
			clip.r,
			clip.b,x,y,r,b))
		return;

	if( !PointInRect(x,y,clip) ||
		!PointInRect(r,b,clip))
		SetClipRect(&clip);

	x += width *0.5f;
	y += height*0.5f;

	x *= 0.01f;
	y *= 0.01f;

	switch( m_state )
	{
	/*	case stStatic:
			if( m_staticWidget )
				m_staticWidget->Draw(x,y,GetAlpha()*m_staticWidget->GetNativeAlpha());
			break;*/

		case stNormal:
			if( IsActive())
			{
				if( m_mouseOver && m_selectedWidget )
				{
					if( m_selectedWidget )
						m_selectedWidget->Draw(x,y,GetAlpha()*m_selectedWidget->GetNativeAlpha());
				}
				else
				{
					if( m_normalWidget )
						m_normalWidget->Draw(x,y,GetAlpha()*m_normalWidget->GetNativeAlpha());
				}
			}
			else
			{
				if( m_disabledWidget )
					m_disabledWidget->Draw(x,y,GetAlpha()*m_disabledWidget->GetNativeAlpha());
			}
			break;

		case stFocused:
			if( IsActive())
			{
				if( m_focusedWidget )
					m_focusedWidget->Draw(x,y,GetAlpha()*m_focusedWidget->GetNativeAlpha());
			}
			else
			{
				if( m_staticWidget )
					m_staticWidget->Draw(x,y,GetAlpha()*m_staticWidget->GetNativeAlpha());
			}
			break;

		case stPressed:
			if( m_pressedWidget )
				m_pressedWidget->Draw(x,y,GetAlpha()*m_pressedWidget->GetNativeAlpha());
			break;

	/*	case stDisabled:
			if( m_disabledWidget )
				m_disabledWidget->Draw(x,y,GetAlpha()*m_disabledWidget->GetNativeAlpha());
			break;*/
	}

	m_pressTime -= deltaTime;

	if( m_pressTime < 0.0f )
	{
		m_pressTime = 0.0f;

		if( !HaveFocus())
			SetNormal();
	}

	SetClipRect(NULL);
}
#endif
void _cdecl GUIButton::Work(float deltaTime, long level)
{
	if( EditMode_IsOn() && !EditMode_IsVisible())
		return;

	Draw();
	Update(deltaTime);
}

void GUIButton::Update(float deltaTime)
{
/*	if( !IsActive())
	{
		SetDisabled();
		return;
	}*/

	ControlStateType state;

	for( dword i = 0 ; i < m_params.links.Size() ; i++ )
	{
		state = Controls().GetControlStateType(m_params.links[i].control);

		if( state == CST_ACTIVE ||
			state == CST_ACTIVATED )
			m_params.links[i].activeTime += deltaTime;
		else
			m_params.links[i].activeTime = 0.0f;
	}

	if( !ProcessKeyboard())
		 ProcessMouse();

//	if( !m_wasReleased && !m_wasPressed )
//		 m_pressed = false;

	if( !EditMode_IsOn() && !m_doNotCheckFocusChange )
	{
		bool action = m_release ?
			!m_wasPressed && !m_wasReleased :	// по отжатию
			 m_wasPressed &&  m_wasReleased;	// по нажатию

	//	if( m_wasPressed && m_wasReleased )
		if( action )
		{
			if( IsActive())
			{
				LogicDebug("Button press trigger");
				m_params.buttonEvent.Activate(Mission(),false);

				m_pressDelay = 0.0f;

				m_repeat = false;
			}
			else
			{
			//	m_params.inactiEvent.Activate(Mission(),false);
			}
		}
		else
		{
			if( m_autorepeat )
			{
				if( m_repeat )
				{
					if( m_pressDelay > repeat_delay )
					{
						LogicDebug("Button press trigger");
						m_params.buttonEvent.Activate(Mission(),false);

						m_pressDelay = 0.0f;
					}
				}
				else
				{
					if( m_pressDelay > repeat_delay*100 )
					{
						LogicDebug("Button press trigger");
						m_params.buttonEvent.Activate(Mission(),false);

						m_pressDelay = 0.0f;

						m_repeat = true;
					}
				}
			}
		}
	}

	m_focusTime += deltaTime;

//	if( m_focusTime > 0.2f )
	if( m_focusTime > 0.0f )
		m_doNotCheckFocusChange = false;

	m_wasReleased = !m_wasPressed;

	if( m_release && m_wasPressed )
		;
	else
		m_pressTime -= deltaTime;

	if( m_pressTime < 0.0f )
	{
		m_pressTime = 0.0f;

		if( !HaveFocus())
		{
			if( m_mouseOver )
				SetSelected();
			else
				SetNormal();
		}
	}

	float k = m_pressTime/press_delay;

	float x_off = Lerp(0.0f,m_params.pressXoffset,k)*m_aspect;
	float y_off = Lerp(0.0f,m_params.pressYoffset,k);

	if( m_activeWidget )
		m_activeWidget->SetFrameOffset(x_off,y_off);
}

void GUIButton::Draw()
{
	if( InterfaceUtils::IsHide())
		return;

	Rect rect; GetRect(rect);

	float width	 = rect.r - rect.l;
	float height = rect.b - rect.t;

	float x = GetX();
	float y = GetY();

	if( EditMode_IsOn())
		m_helperQuad.DrawQuad(x,y,width,height,0,0,0,0,10.5f);

/*	float k = m_pressTime/press_delay;

	float x_off = Lerp(0.0f,m_params.pressXoffset,k);
	float y_off = Lerp(0.0f,m_params.pressYoffset,k);

//	;
	x_off *= m_aspect;*/

/*	x += x_off*m_aspect;
	y += y_off;

	float r = x + width;
	float b = y + height;

	Rect clip; GetParentClipRect(clip);

	if( !RectInRect(
			clip.l,
			clip.t,
			clip.r,
			clip.b,x,y,r,b))
		return;

	if( !PointInRect(x,y,clip) ||
		!PointInRect(r,b,clip))
		SetClipRect(&clip);

	x += width *0.5f;
	y += height*0.5f;

	x *= 0.01f;
	y *= 0.01f;

	switch( m_state )
	{
	//	case stStatic:
	//		if( m_staticWidget )
	//			m_staticWidget->Draw(x,y,GetAlpha()*m_staticWidget->GetNativeAlpha());
	//		break;

		case stNormal:
			if( IsActive())
			{
				if( m_mouseOver && m_selectedWidget )
				{
					if( m_selectedWidget )
						m_selectedWidget->Draw(x,y,GetAlpha()*m_selectedWidget->GetNativeAlpha());
				}
				else
				{
					if( m_normalWidget )
						m_normalWidget->Draw(x,y,GetAlpha()*m_normalWidget->GetNativeAlpha());
				}
			}
			else
			{
				if( m_disabledWidget )
					m_disabledWidget->Draw(x,y,GetAlpha()*m_disabledWidget->GetNativeAlpha());
			}
			break;

		case stFocused:
			if( IsActive())
			{
				if( m_focusedWidget )
					m_focusedWidget->Draw(x,y,GetAlpha()*m_focusedWidget->GetNativeAlpha());
			}
			else
			{
				if( m_staticWidget )
					m_staticWidget->Draw(x,y,GetAlpha()*m_staticWidget->GetNativeAlpha());
			}
			break;

		case stPressed:
			if( m_pressedWidget )
				m_pressedWidget->Draw(x,y,GetAlpha()*m_pressedWidget->GetNativeAlpha());
			break;

	//	case stDisabled:
	//		if( m_disabledWidget )
	//			m_disabledWidget->Draw(x,y,GetAlpha()*m_disabledWidget->GetNativeAlpha());
	//		break;
	}

	SetClipRect(NULL);*/
}

void GUIButton::OnAcquireFocus()
{
	for( dword i = 0 ; i < m_params.links.Size() ; i++ )
		m_params.links[i].activeTime = 0.0f;

	SetFocused();

	m_doNotCheckFocusChange = true;
	m_focusTime = 0.0f;

//	SetCursor();
}

void GUIButton::OnLooseFocus()
{
	if( m_state == stFocused ||
		m_state == stPressed )
	{
	//	api->Trace("ON LOS FOCUS: %s",GetObjectID().c_str());
		LogicDebug("Button losFocus trigger");
		onLosFocus.Activate(Mission(),false);
	}

//	if( IsShow())
		SetNormal();

	if( m_tooltipWidget )
	{
		m_tooltipWidget->Show(false);
	}
}

void GUIButton::OnParentNotify(BaseGUIElement::Notification event)
{
	BaseGUIElement::OnParentNotify(event);
}

bool GUIButton::Create(MOPReader &reader)
{
	if( m_restart )
	{
		RestartObject(reader);
	}
	else
	{
		EditMode_Update(reader);
	}

	return true;
}

bool GUIButton::RestartObject(MOPReader &reader)
{
	BaseGUIElement::SkipMOPs(reader);
	SkipMOPs(reader);

//	SetUpdate(&GUIButton::InitFunc,ML_FIRST);
	SetUpdate(&GUIButton::InitFunc,ML_FIRST + 2);

	if( m_initComplete == false )
	{
		m_restart = false;
	}

	return true;
}

void GUIButton::InitFunc(float, long)
{
	DelUpdate(&GUIButton::InitFunc);

	////////////////////////////////

	if( m_restart )
	{
		m_restart = false;

		if( m_staticWidget	 ) m_staticWidget  ->Show(false);
		if( m_selectedWidget ) m_selectedWidget->Show(false);
		if( m_normalWidget	 ) m_normalWidget  ->Show(false);
		if( m_focusedWidget	 ) m_focusedWidget ->Show(false);
		if( m_pressedWidget	 ) m_pressedWidget ->Show(false);
		if( m_disabledWidget ) m_disabledWidget->Show(false);

		if( m_tooltipWidget )
			m_tooltipWidget->Show(false);

		Controls().EnableControlGroup("GUI",true);

		Show(nativeShow);

		return;
	}

	////////////////////////////////

	m_initComplete = true;

	BaseGUIElement *parent = GetParent();

	if( BaseGUIElement *newParent = FindParent())
	{
		newParent->Register(this);
	}
	else
	{
		if( parent )
			parent->UnRegister(this);
	}

	const ConstString & myName = GetObjectID();

	GUIWidget *mo = null;

	mo = (GUIWidget*)FindObject(m_params.staticWidget);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_staticWidget = (GUIWidget*)mo;
		m_staticWidget->Show(false);

		m_staticWidget->SetShiftAspect(m_shiftAspect);
		m_staticWidget->SetWidthAspect(m_widthAspect);

		m_staticWidget->SetLayout(m_layout);

		m_staticWidget->SetParent(myName);
		m_staticWidget->SetFading(false);
	}

	mo = (GUIWidget*)FindObject(m_params.selectedButtonWidget);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_selectedWidget = (GUIWidget*)mo;
		m_selectedWidget->Show(false);

		m_selectedWidget->SetShiftAspect(m_shiftAspect);
		m_selectedWidget->SetWidthAspect(m_widthAspect);

		m_selectedWidget->SetLayout(m_layout);

		m_selectedWidget->SetParent(myName);
		m_selectedWidget->SetFading(false);
	}

	mo = (GUIWidget*)FindObject(m_params.normalButtonWidget);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_normalWidget = (GUIWidget*)mo;
		m_normalWidget->Show(false);

		m_normalWidget->SetShiftAspect(m_shiftAspect);
		m_normalWidget->SetWidthAspect(m_widthAspect);

		m_normalWidget->SetLayout(m_layout);

		m_normalWidget->SetParent(myName);
		m_normalWidget->SetFading(false);
	}

	mo = (GUIWidget*)FindObject(m_params.focusButtonWidget);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_focusedWidget = (GUIWidget*)mo;
		m_focusedWidget->Show(false);

		m_focusedWidget->SetShiftAspect(m_shiftAspect);
		m_focusedWidget->SetWidthAspect(m_widthAspect);

		m_focusedWidget->SetLayout(m_layout);

		m_focusedWidget->SetParent(myName);
		m_focusedWidget->SetFading(false);
	}

	mo = (GUIWidget*)FindObject(m_params.pressedButtonWidget);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_pressedWidget = (GUIWidget*)mo;
		m_pressedWidget->Show(false);

		m_pressedWidget->SetShiftAspect(m_shiftAspect);
		m_pressedWidget->SetWidthAspect(m_widthAspect);

		m_pressedWidget->SetLayout(m_layout);

		m_pressedWidget->SetParent(myName);
		m_pressedWidget->SetFading(false);
	}

	mo = (GUIWidget*)FindObject(m_params.disabledButtonWidget);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_disabledWidget = (GUIWidget*)mo;
		m_disabledWidget->Show(false);

		m_disabledWidget->SetShiftAspect(m_shiftAspect);
		m_disabledWidget->SetWidthAspect(m_widthAspect);

		m_disabledWidget->SetLayout(m_layout);

		m_disabledWidget->SetParent(myName);
		m_disabledWidget->SetFading(false);
	}

	mo = (GUIWidget*)FindObject(m_params.descObject);

	if( mo && mo->Is(InterfaceUtils::GetWidgetId()))
	{
		m_tooltipWidget = (GUIWidget*)mo;
		m_tooltipWidget->Show(false);
	}
/*
//	;
	m_activeWidget = m_normalWidget;*/

	Controls().EnableControlGroup("GUI",true);

	if( !m_cursor )
		 m_cursor = (BaseGUICursor *)FindObject(InterfaceUtils::GetCursorName());

	m_baseCursor = m_cursor;

//	Show(IsShow());
	Show(nativeShow);
}

bool GUIButton::EditMode_Update(MOPReader &reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs					   (reader);

//	SetUpdate(&GUIButton::InitFunc,ML_FIRST);
	SetUpdate(&GUIButton::InitFunc,ML_FIRST + 2);

	m_initComplete = false;

	return true;
}

void GUIButton::Show(bool isShow)
{
	BaseGUIElement::Show(isShow);

	if( EditMode_IsOn())
		DelUpdate(&GUIButton::Work);

	if( IsShow())
	{
		if( m_params.initiallyFocused || HaveFocus())
		{
			SetFocused();

			if( m_cursor && !m_cursor->IsClicked())
				SetCursor();
		}
		else
			SetNormal();

		m_doNotCheckFocusChange = true;
		m_focusTime = 0.0f;

		if( GetAlpha() > 0.0f )
		{
		//	SetUpdate(&GUIButton::Work,drawLevel + drawPriority);
			SetUpdate(&GUIButton::Work,drawLevel);
		}
	}
	else
	{
		if( m_activeWidget )
			m_activeWidget->Show(false);

		m_activeWidget = null;

		SetState(stNormal);

		if( HaveFocus())
			SetElementInFocus(null);

		DelUpdate(&GUIButton::Work);
	}

	m_pressed = false;

	m_pressTime = 0.0f;
}

void GUIButton::Activate(bool isActive)
{
	BaseGUIElement::Activate(isActive);

	if( !IsActive() && isActive )
	{
		SetState(stNormal);
	}

	if( isActive == false )
	{
		if( GetElementInFocus() == this )
		{
			OnLooseFocus();
			SetElementInFocus(null);
		}

		SetDisabled();
	}
}

void GUIButton::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"SetFocus"))
	{
		if( IsShow() && IsActive())
			SetFocused();

	//	if( IsShow() && IsActive())
	//		SetCursor();

		LogicDebug("Command GUIButton::<SetFocus>. Button is now focused.");
	}
	else
	if( string::IsEqual(id,"Set"))
	{
		if( numParams < 2 )
		{
			LogicDebugError("Command GUIButton::<Set> error. Too few arguments.");
			return;
		}

		const char *type = params[0];
		const char *name = params[1];

		if( name && type[0] )
		{
			MissionObject *p	  = FindObject(ConstString(name));
			GUIWidget	  *widget = NULL;

			if( p && p->Is(InterfaceUtils::GetWidgetId()))
			{
				widget = (GUIWidget *)p;
				widget->Show(false);
			}

			if( widget )
			{
				if( type && type[0] )
				{
					if( string::IsEqual(type,"Static"))
						m_staticWidget = widget;
					else
					if( string::IsEqual(type,"Selected"))
						m_selectedWidget = widget;
					else
					if( string::IsEqual(type,"Normal"))
						m_normalWidget = widget;
					else
					if( string::IsEqual(type,"Focused"))
						m_focusedWidget = widget;
					else
					if( string::IsEqual(type,"Pressed"))
						m_pressedWidget = widget;
					else
					if( string::IsEqual(type,"Disabled"))
						m_disabledWidget = widget;
					else
					{
						LogicDebugError("Command GUIButton::<Set> error. Invalid type name (%s).",type);
						return;
					}

					LogicDebug("Command GUIButton::<Set>. %s widget now is %s.",type,name);
				}
				else
				{
					LogicDebugError("Command GUIButton::<Set> error. Invalid type parameter.");
				}
			}
			else
			{
				LogicDebugError("Command GUIButton::<Set> error. Widget %s not found.",name);
			}
		}
		else
		{
			LogicDebugError("Command GUIButton::<Set> error. Invalid name parameter.");
		}
	}
	else
	{
		BaseGUIElement::Command(id,numParams,params);
	}
}

static char GUIDescription[] =
"GUI Button.\n\n"
"    NOTE: all sizes-positions are relative (not pixel sizes-positions)\n\n"
"Commands:\n\n"
"    Set <type> <name> - set new widget to use\n\n"
"        type - widget type (static, normal, focused, pressed, disabled)\n"
"        name - widget object name\n\n"
"    SetFocus - set button to focus state";

MOP_BEGINLISTCG(GUIButton, "GUI Button", '1.00', 200, GUIDescription, "Interface")

//	MOP_GROUPBEG("Wide screen layout")

	//	MOP_BOOL("Use shift aspect", false)
	//	MOP_BOOL("Use width aspect", true)

		MOP_ENUMBEG("Layout")

			MOP_ENUMELEMENT("Left")
			MOP_ENUMELEMENT("Center")
			MOP_ENUMELEMENT("Right")

		MOP_ENUMEND

		MOP_ENUM("Layout", "Layout")

//	MOP_GROUPEND()

	MOP_FLOAT("X position", 0.0f)
	MOP_FLOAT("Y position", 0.0f)

	MOP_FLOAT("Alpha", 1.0f)

	MOP_STRING("Parent id", "")

	MOP_FLOAT("Width" , 25.0f)
	MOP_FLOAT("Height", 25.0f)

	MOP_LOCSTRING("Tooltip message")
	MOP_STRING("Tooltip widget", "")

	MOP_STRING("Static widget", "")
	MOP_STRING("Selected widget", "")
	MOP_STRING("Normal state widget", "")
	MOP_STRING("Focused state widget", "")
	MOP_STRING("Pressed state widget", "")
	MOP_STRING("Disabled (inactive) state widget", "")

	MOP_FLOAT("X offset on press", 1.0f)
	MOP_FLOAT("Y offset on press", 1.0f)

	MOP_ARRAYBEG("Navigation links", 0, 100)

		MOP_STRING("Control", "")
		MOP_STRING("Object to go to", "")

	MOP_ARRAYEND

	MOP_ARRAYBEG("Hot keys", 0, 10)

		MOP_STRING("Control", "")
		MOP_BOOL("Only in focus", true)

	MOP_ARRAYEND

	MOP_BOOL("Action by release", false)

	MOP_GROUPBEG("Events")

		MOP_MISSIONTRIGGER("")

	MOP_GROUPEND()

//	MOP_MISSIONTRIGGERC("Inactive","")

	MOP_BOOL("Autorepeat", false)
	MOP_BOOL("Autofocus" , true)

	MOP_MISSIONTRIGGERC("GetFocus","")
	MOP_MISSIONTRIGGERC("LosFocus","")

	MOP_BOOL("Initially focused", false)
	MOP_BOOL("Focus by press", true)

	MOP_LONG("Draw priority", 0)

	MOP_BOOL("Active" , true)
	MOP_BOOL("Visible", true)
	
MOP_ENDLIST(GUIButton)
