#include "GUISlider.h"
#include "GUIWidget.h"
#include "GUIButton.h"

#include "..\Utils\InterfaceUtils.h"

#include "BaseGUICursor.h"

GUISlider:: GUISlider()
{
	drawLevel = ML_GUI2;

	m_restart = false;
}

GUISlider::~GUISlider()
{
	if( m_val )
		m_val->Release();
}

void GUISlider::Restart()
{
	if( HaveFocus())
		SetElementInFocus(null);

	m_restart = true;

	ReCreate();
}

void GUISlider::SkipMOPs(MOPReader &reader)
{
	m_moved = false;

	reader.Float();  // width
	reader.Float();  // height

	drawPriority = reader.Long();

	reader.String().c_str(); // backName
	reader.String().c_str(); // foreName

	reader.String().c_str(); // backNameSel
	reader.String().c_str(); // foreNameSel

	reader.String().c_str(); // backNameDis
	reader.String().c_str(); // foreNameDis

	reader.Float();  // btnWidth
	reader.Float();  // btnHeight

	reader.Float();  // priLo
	reader.Float();  // priHi

	reader.Float();  // secLo
	secHi = 0.0f;

	reader.Bool();	 // m_vertical

//	m_bx = priLo;
//	m_by = secLo;

	reader.String().c_str(); // m_val_name

	reader.Float();  // m_val_def

	reader.Float();  // m_val_min
	reader.Float();  // m_val_max

	reader.Float();  // m_val_step
	reader.Float();  // m_val_t

	m_time = 0.0f;
	reader.Float();  // m_timeMax

	m_repeat = false;

	onChange.Init(reader);
	onTry	.Init(reader);

	//// links ////

	dword count = reader.Array();

	for( dword i = 0 ; i < count ; i++ )
	{
		reader.String().c_str(); // control alias
		reader.String().c_str(); // oblect
	}

	///////////////

	reader.Bool();   // m_autofocus

	onGetFocus.Init(reader);
	onLosFocus.Init(reader);

	reader.Bool();   // m_params.initiallyFocused
	reader.Bool();   // m_focusByPress

	m_dt = 0.0f;

	Activate(reader.Bool());
	nativeShow = reader.Bool();
}

void GUISlider::ReadMOPs(MOPReader &reader)
{
	m_cursor = null; m_moved = false;

	float width  = reader.Float();
	float height = reader.Float();

	SizeTo(width,height);

	drawPriority = reader.Long();

	backName = reader.String().c_str();
	foreName = reader.String().c_str();

	backNameSel = reader.String().c_str();
	foreNameSel = reader.String().c_str();

	backNameDis = reader.String().c_str();
	foreNameDis = reader.String().c_str();

	btnWidth  = reader.Float()*m_aspect;
	btnHeight = reader.Float();

	priLo = reader.Float();
	priHi = reader.Float();

	secLo = reader.Float();
	secHi = 0.0f;

	m_vertical = reader.Bool();

//	if( !m_vertical )
	{
		priLo *= m_aspect;
		priHi *= m_aspect;
	}

	m_bx = priLo;
	m_by = secLo;

//	m_objectName = reader.String().c_str();
	m_object = null;

	m_val_name = reader.String().c_str();
	m_val = null;

	m_val_def = reader.Float();

	m_val_min = reader.Float();
	m_val_max = reader.Float();

	m_val_step = reader.Float();
	m_val_t	   = reader.Float();

	m_time	  = 0.0f;
	m_timeMax = reader.Float();

	m_timeMaxBeg = m_timeMax*4.0f;

	m_repeat = false;

//	m_min = 0.0f;
//	m_max = 1.0f;

//	m_step = m_val_step/(m_val_max - m_val_min);

	onChange.Init(reader);
	onTry	.Init(reader);

	m_params.links.DelAll();

	Params::NavigationLink link;

	dword count = reader.Array();

	for( dword i = 0 ; i < count ; i++ )
	{
		link.control = reader.String().c_str();
		link.object	 = reader.String().c_str();

		m_params.links.Add(link);
	}

	m_autofocus = reader.Bool();

	onGetFocus.Init(reader);
	onLosFocus.Init(reader);

	m_params.initiallyFocused = reader.Bool();
	m_focusByPress = reader.Bool();

	m_back = null;
	m_fore = null;

	m_back_sel = null;
	m_fore_sel = null;

	m_back_dis = null;
	m_fore_dis = null;

	m_dt = 0.0f;

	Activate(reader.Bool());
	nativeShow = reader.Bool();
}

bool GUISlider::Create		   (MOPReader &reader)
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

bool GUISlider::RestartObject(MOPReader &reader)
{
	BaseGUIElement::SkipMOPs(reader);
	SkipMOPs(reader);

//	SetUpdate(&GUISlider::InitFunc,ML_FIRST - 2);
	SetUpdate(&GUISlider::InitFunc,ML_FIRST);

	return true;
}

bool GUISlider::EditMode_Update(MOPReader &reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs					   (reader);

//	SetUpdate(&GUISlider::InitFunc,ML_FIRST - 2);
	SetUpdate(&GUISlider::InitFunc,ML_FIRST);

	return true;
}

void _cdecl GUISlider::InitFunc(float, long)
{
	DelUpdate(&GUISlider::InitFunc);

	if( m_restart )
	{
		m_restart = false;

		if( m_back )
		{
			m_back->Show(false); m_al_back = m_back->GetNativeAlpha();
		}
		if( m_fore )
		{
			m_fore->Show(false); m_al_fore = m_back->GetNativeAlpha();
		}

		if( m_back_sel )
		{
			m_back_sel->Show(false); m_al_back_sel = m_back_sel->GetNativeAlpha();
		}
		if( m_fore_sel )
		{
			m_fore_sel->Show(false); m_al_fore_sel = m_fore_sel->GetNativeAlpha();
		}

		if( m_back_dis )
		{
			m_back_dis->Show(false); m_al_back_dis = m_back_dis->GetNativeAlpha();
		}
		if( m_fore_dis )
		{
			m_fore_dis->Show(false); m_al_fore_dis = m_fore_dis->GetNativeAlpha();
		}

		Show(nativeShow);

		return;
	}

	BaseGUIElement *parent = GetParent();

	if( parent )
		parent->UnRegister(this);

	if( parent = FindParent())
	{
		parent->Register(this);
	}

	Controls().EnableControlGroup("GUI",true);

	GUIWidget *p;

	p = (GUIWidget *)FindObject(backName);	  if( p ) { p->SetFading(false); p->Show(false); } m_back = p;
	p = (GUIWidget *)FindObject(foreName);	  if( p ) { p->SetFading(false); p->Show(false); } m_fore = p;

	p = (GUIWidget *)FindObject(backNameSel); if( p ) { p->SetFading(false); p->Show(false); } m_back_sel = p;
	p = (GUIWidget *)FindObject(foreNameSel); if( p ) { p->SetFading(false); p->Show(false); } m_fore_sel = p;

	p = (GUIWidget *)FindObject(backNameDis); if( p ) { p->SetFading(false); p->Show(false); } m_back_dis = p;
	p = (GUIWidget *)FindObject(foreNameDis); if( p ) { p->SetFading(false); p->Show(false); } m_fore_dis = p;

	if( m_back ) m_al_back = m_back->GetNativeAlpha();
	if( m_fore ) m_al_fore = m_fore->GetNativeAlpha();

	if( m_back_sel ) m_al_back_sel = m_back_sel->GetNativeAlpha();
	if( m_fore_sel ) m_al_fore_sel = m_fore_sel->GetNativeAlpha();

	if( m_back_dis ) m_al_back_dis = m_back_dis->GetNativeAlpha();
	if( m_fore_dis ) m_al_fore_dis = m_fore_dis->GetNativeAlpha();

	Show(nativeShow);
}

void _cdecl GUISlider::Work(float deltaTime, long)
{
	if( EditMode_IsOn() && !EditMode_IsVisible())
		return;

	Update(deltaTime); m_dt = deltaTime;
	Draw();
}

void GUISlider::SetFocused()
{
	bool newFocus = (GetElementInFocus() != this);

	if( newFocus )
	{
		if( GetElementInFocus())
			GetElementInFocus()->OnLooseFocus();

		SetElementInFocus(this);

		if( m_cursor && !(m_cursor->IsMoved() || m_cursor->IsClicked()))
			SetCursor();

		LogicDebug("Slider getFocus trigger");
		onGetFocus.Activate(Mission(),false);
	}
}

void GUISlider::SetNormal()
{
	//
}

void GUISlider::OnAcquireFocus()
{
	for( dword i = 0 ; i < m_params.links.Size() ; i++ )
		m_params.links[i].activeTime = 0.0f;

	SetFocused();

	m_doNotCheckFocusChange = true;
	m_focusTime = 0.0f;
}

void GUISlider::OnLooseFocus()
{
	LogicDebug("Slider losFocus trigger");
	onLosFocus.Activate(Mission(),false);

	SetNormal();
}

bool GUISlider::GoToLink(const char *control)
{
	for( dword i = 0 ; i < m_params.links.Size() ; i++ )
	{
		if( string::IsEqual(m_params.links[i].control,control))
		{
			MissionObject *mo = FindObject(m_params.links[i].object);

			if( mo && mo->IsShow())
			{
				static const ConstString buttonId("GUIButton");
				static const ConstString sliderId("GUISlider");
				if( mo->Is(buttonId))
				{
					GUIButton *p = (GUIButton *)mo;

					if( p->IsActive() && p->ParentIsActive())
					{					
						p->OnAcquireFocus();

						return true;
					}
					else
					{
						return p->GoToLink(control);
					}
				}
				else
				if( mo->Is(sliderId))
				{
					GUISlider *p = (GUISlider *)mo;

					if( p->IsActive() && p->ParentIsActive())
					{					
						p->OnAcquireFocus();

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

bool GUISlider::ProcessKeyboard()
{
	bool wasKeyControl = false; // было ли управление клавишами

	if( EditMode_IsOn() || !IsActive() || !ParentIsActive())
		return wasKeyControl;

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
					static const ConstString buttonId("GUIButton");
					static const ConstString sliderId("GUISlider");
					if( mo->Is(buttonId))
					{
						GUIButton *p = (GUIButton *)mo;

						if( p->IsActive() && p->ParentIsActive())
						{					
							p->OnAcquireFocus();

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
					if( mo->Is(sliderId))
					{
						GUISlider *p = (GUISlider *)mo;

						if( p->IsActive() && p->ParentIsActive())
						{					
							p->OnAcquireFocus();

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

	return wasKeyControl;
}

void GUISlider::Update(float deltaTime)
{
	if( EditMode_IsOn() || !IsActive())
		return;

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

	ProcessKeyboard();

	if( !m_moved && HaveFocus())
	{
		if( Controls().GetControlStateType(m_left ) == CST_ACTIVATED )
		{
			StepDown();
			SetCursor(); m_time = m_timeMaxBeg; m_repeat = false;
			return;
		}
		if( Controls().GetControlStateType(m_right) == CST_ACTIVATED )
		{
			StepUp();
			SetCursor(); m_time = m_timeMaxBeg; m_repeat = false;
			return;
		}

		if( Controls().GetControlStateType(m_left ) == CST_ACTIVE )
		{
			if( m_time > 0.0f )
			{
				m_time -= deltaTime;

				if( m_time <= 0.0f )
				{
					StepDown();
					SetCursor();
					
					m_time = m_timeMax;
				}
			}
			return;
		}
		if( Controls().GetControlStateType(m_right) == CST_ACTIVE )
		{
			if( m_time > 0.0f )
			{
				m_time -= deltaTime;

				if( m_time <= 0.0f )
				{
					StepUp();
					SetCursor();

					m_time = m_timeMax;
				}
			}
			return;
		}
	}

	m_focusTime += deltaTime;

//	if( m_focusTime > 0.2f )
	if( m_focusTime > 0.0f )
		m_doNotCheckFocusChange = false;

	if( !m_cursor || m_cursor->IsHided())
		return;

	float x,y; m_cursor->GetPosition(x,y);

	x *= 100.0f;
	y *= 100.0f;

	Rect cr; GetParentClipRect(cr);
	Rect  r; GetRect(r);

	if( m_autofocus && m_cursor->IsMoved())
	{
		if( !HaveFocus())
		{
			Rect tr;

			tr.l = r.l + priLo; tr.r =  r.r - priHi;
			tr.t = r.t + secLo; tr.b = tr.t + btnHeight;

			if( PointInRect(x,y,cr) &&
				PointInRect(x,y,tr))
				SetFocused();
		}
	}

	ControlStateType st = Controls().GetControlStateType(m_click);

	if( m_moved && st != CST_ACTIVE )
		m_moved = false;

	if( m_moved )
	{
		m_onMin = false;
		m_onMax = false;

		cr.l += m_dx + priLo; cr.r += m_dx - btnWidth - priHi;
		cr.t += m_dy;		  cr.b += m_dy - btnHeight;

		 r.l += m_dx + priLo;  r.r += m_dx - btnWidth - priHi;
		 r.t += m_dy;		   r.b += m_dy - btnHeight;

		if( fabsf(x - (m_lx - m_dx + 0.5f*btnWidth )) > 1.5f*btnWidth ||
			fabsf(y - (m_ly - m_dy + 0.5f*btnHeight)) > 1.5f*btnHeight )
		{
			if( !m_reseted )
			{
				m_xx = m_bx;
				m_yy = m_by;

				m_bx = m_ox;
				m_by = m_oy;
				
				m_reseted = true;

				SetValue();
			}
		}
		else
		{
			if( m_reseted )
			{
				m_bx = m_xx;
				m_by = m_yy;

				m_reseted = false;
			}

			if( x < cr.l ) { x = cr.l; m_onMin = true; }
			if( x <  r.l ) { x =  r.l; m_onMin = true; }

			if( x > cr.r ) { x = cr.r; m_onMax = true; }
			if( x >  r.r ) { x =  r.r; m_onMax = true; }

			if( y < cr.t ) y = cr.t;
			if( y <  r.t ) y =  r.t;

			if( y > cr.b ) y = cr.b;
			if( y >  r.b ) y =  r.b;

			m_bx += x - m_px;
		//	m_by += y - m_py;

			if( x != m_px )
				m_lx = x;
		//	if( y != m_py )
		//		m_ly = y;

			m_px = x;
			m_py = y;

			SetValue();
		}
	}
	else
	if( st == CST_ACTIVATED )
	{
		Rect t = r;	// позиция ползунка

		t.l += m_bx; t.r = t.l + btnWidth;
		t.t += m_by; t.b = t.t + btnHeight;

		r.l += priLo;
		r.r -= priHi;

		r.t += m_by; r.b = r.t + btnHeight;

		if( PointInRect(x,y,cr) &&
			PointInRect(x,y, r))
		{
			if(	PointInRect(x,y, t))
			{
				m_dx = x - t.l;
				m_dy = y - t.t;

				m_lx = m_px = x;
				m_ly = m_py = y;
			}
			else
			{
				m_bx = priLo + x - r.l - btnWidth/2;

				if( m_bx < priLo )
				{
					m_bx = priLo;
					m_onMin = true;
				}

				if( m_bx > priLo + r.r - r.l - btnWidth )
				{
					m_bx = priLo + r.r - r.l - btnWidth;
					m_onMax = true;
				}

				m_dx = btnWidth/2;
				m_dy = y - r.t;

				x = r.l - priLo + m_bx + btnWidth/2;

				m_lx = m_px = x;
				m_ly = m_py = y;

				SetValue();
			}

			m_ox = m_bx;
			m_oy = m_by;

			m_moved = true; m_reseted = false;

			if( !HaveFocus() && m_focusByPress )
				SetFocused();
		}
	}
	else
	if( st == CST_INACTIVATED )
	{
		m_moved = false;
	}
}

static float round(float x, float t)
{
	float a = x - fmodf(x,t);
	float b = a + t;

	return x - a <= b - x ? a : b;
}

void GUISlider::SetValue()
{
/*	float val = (m_bx - priLo)/(GetWidth()*m_aspect - priLo - priHi - btnWidth);

	if( val != m_value )
	{
		m_value = val;

	//	if( m_object )
	//		m_object->SetAlpha(val);

		if( m_val )
			m_val->Set(Lerp(m_val_min,m_val_max,m_value));

		LogicDebug("Slider onChange trigger");
		onChange.Activate(Mission(),false);
	}*/
	float val = (m_bx - priLo)/(GetWidth()*m_aspect - priLo - priHi - btnWidth);

	val = m_val_min + m_val_max*Clampf(val);

	if( m_onMin ) val = m_val_min;
	if( m_onMax ) val = m_val_max;

	float d = fabsf(val - m_value);

	if( d > m_val_t || (m_onMin||m_onMax)&&(d > 0.00001f))
	{
		if( !m_onMin && !m_onMax )
		{
		/*	if( val > m_value )
			{
				val -= fmodf(val,m_val_t);
			}
			else
			{
				val -= fmodf(val,m_val_t);
				val += m_val_t;
			}*/
			val = round(val,m_val_t);
		}

		m_value = val;

		if( m_object )
			m_object->SetAlpha(val);

		if( m_val )
			m_val->Set(m_value);

		LogicDebug("Slider onChange trigger");
		onChange.Activate(Mission(),false);
	}
}

void GUISlider::GetValue()
{
/*	if( m_object )
	{
		float val = m_object->GetAlpha();

		m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);

		m_value = val;
	}*/

/*	float val = m_val ? m_val->Get(m_val_def) : m_val_def;

	val = Clampf((val - m_val_min)/(m_val_max - m_val_min));

	m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);

	m_value = val;*/
	m_value = m_val ? m_val->Get(m_val_def) : m_val_def;

	m_value = Clampf(m_value,m_val_min,m_val_max);

	if( fabsf(m_val_max - m_value) > m_val_t*0.5f )
	{
	//	m_value -= fmodf(m_value,m_val_t);
		m_value = round(m_value,m_val_t);
	}
	else
	{
		m_value = m_val_max;
	}

	float val = (m_value - m_val_min)/(m_val_max - m_val_min);

	m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);
}

void GUISlider::StepUp()
{
/*	float val = m_value + m_step;

	if( val > m_max )
		val = m_max;

	if( val != m_value )
	{
		m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);

		m_value = val;

	//	if( m_object )
	//		m_object->SetAlpha(val);

		if( m_val )
			m_val->Set(Lerp(m_val_min,m_val_max,m_value));

		LogicDebug("Slider onChange trigger");
		onChange.Activate(Mission(),false);
	}
	else
	{
		LogicDebug("Slider onTry trigger");
		onTry.Activate(Mission(),false);
	}*/
	float val = m_value + m_val_step;

	if( val > m_val_max )
		val = m_val_max;

	if( val != m_value )
	{
		m_value = val;

		if( fabsf(m_val_max - m_value) > m_val_t*0.5f )
		{
		//	m_value -= fmodf(m_value,m_val_t);
			m_value = round(m_value,m_val_t);
		}
		else
		{
			m_value = m_val_max;
		}

		val = Clampf((m_value - m_val_min)/(m_val_max - m_val_min));

		m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);

		if( m_object )
			m_object->SetAlpha(val);

		if( m_val )
			m_val->Set(m_value);

		LogicDebug("Slider onChange trigger");
		onChange.Activate(Mission(),false);
	}
	else
	{
		m_bx = GetWidth()*m_aspect - priHi - btnWidth;

		LogicDebug("Slider onTry trigger");
		onTry.Activate(Mission(),false);
	}
}

void GUISlider::StepDown()
{
/*	float val = m_value - m_step;

	if( val < m_min )
		val = m_min;

	if( val != m_value )
	{
		m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);

		m_value = val;

	//	if( m_object )
	//		m_object->SetAlpha(val);

		if( m_val )
			m_val->Set(Lerp(m_val_min,m_val_max,m_value));

		LogicDebug("Slider onChange trigger");
		onChange.Activate(Mission(),false);
	}
	else
	{
		LogicDebug("Slider onTry trigger");
		onTry.Activate(Mission(),false);
	}*/
	float val = m_value - m_val_step;

	if( val < m_val_min )
		val = m_val_min;

	if( val != m_value )
	{
		m_value = val;

		if( fabsf(m_val_max - m_value) > m_val_t*0.5f )
		{
		//	m_value -= fmodf(m_value,m_val_t);
			m_value = round(m_value,m_val_t);
		}
		else
		{
			m_value = m_val_max;
		}

		val = Clampf((m_value - m_val_min)/(m_val_max - m_val_min));

		m_bx = priLo + val*(GetWidth()*m_aspect - priLo - priHi - btnWidth);

		if( m_object )
			m_object->SetAlpha(val);

		if( m_val )
			m_val->Set(m_value);

		LogicDebug("Slider onChange trigger");
		onChange.Activate(Mission(),false);
	}
	else
	{
		m_bx = priLo;

		LogicDebug("Slider onTry trigger");
		onTry.Activate(Mission(),false);
	}
}

void GUISlider::Draw()
{
	if( InterfaceUtils::IsHide())
		return;

	Rect r; GetRect(r);

	Rect clip; GetParentClipRect(clip);

	if( !RectInRect(
			clip.l,
			clip.t,
			clip.r,
			clip.b,r.l,r.t,r.r,r.b))
		return;

	if( !PointInRect(r.l,r.t,clip) ||
		!PointInRect(r.r,r.b,clip))
		SetClipRect(&clip);

	float x = r.l;// float width  = r.r - r.l;
	float y = r.t;// float height = r.b - r.t;

	GUIWidget *back = m_back;
	GUIWidget *fore = m_fore;

	float al_b = 1.0f;
	float al_f = 1.0f;

	if( back ) al_b = m_al_back;
	if( fore ) al_f = m_al_fore;

	if( HaveFocus())
	{
		if( m_back_sel ) { back = m_back_sel; al_b = m_al_back_sel; }
		if( m_fore_sel ) { fore = m_fore_sel; al_f = m_al_fore_sel; }
	}
	else
	if( !IsActive())
	{
		if( m_back_dis ) { back = m_back_dis; al_b = m_al_back_dis; }
		if( m_fore_dis ) { fore = m_fore_dis; al_f = m_al_fore_dis; }
	}

	float al = GetAlpha();

	if( back )
	{
		back->Update(m_dt);
		back->SetAlpha(al*al_b);
		back->MoveTo(x/m_aspect,y);
		back->Draw();
	}

	if( fore )
	{
		fore->Update(m_dt);
		fore->SetAlpha(al*al_f);
		fore->MoveTo((x + m_bx)/m_aspect,(y + m_by));
		fore->Draw();
	}

	SetClipRect(null);
}

void GUISlider::SetCursor()
{
	if( m_baseCursor )
	{
		Rect r; GetRect(r);

		r.l += m_bx; r.r = r.l + btnWidth;
		r.t += m_by; r.b = r.t + btnHeight;

		m_baseCursor->SetPosition(
			0.5f*0.01f*(r.l + r.r),
			0.5f*0.01f*(r.t + r.b));
	}
}

void GUISlider::Show(bool isShow)
{
	BaseGUIElement::Show(isShow);

	if( EditMode_IsOn())
		DelUpdate(&GUISlider::Work);

	GetValue();

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
			SetUpdate(&GUISlider::Work,drawLevel + drawPriority);
		}
	}
	else
	{
		if( HaveFocus())
			SetElementInFocus(null);

		DelUpdate(&GUISlider::Work);
	}
}

void GUISlider::Activate(bool isActive)
{
	BaseGUIElement::Activate(isActive);

	if( isActive == false )
	{
		if( GetElementInFocus() == this )
		{
			OnLooseFocus();
			SetElementInFocus(null);
		}
	}
}

void GUISlider::Command(const char *id, dword numParams, const char **params)
{
	if( !IsActive())
		return;

	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"SetFocus"))
	{
		if( IsShow() && IsActive())
			SetFocused();

	//	if( IsShow() && IsActive())
	//		SetCursor();

		LogicDebug("Command GUISlider::<SetFocus>. Slider is now focused.");
	}
	else
	if( string::IsEqual(id,"up"))
	{
		StepUp();

		return;
	}
	else
	if( string::IsEqual(id,"down"))
	{
		StepDown();

		return;
	}
	else
	{
		BaseGUIElement::Command(id,numParams,params);
	}
}

MOP_BEGINLISTCG(GUISlider, "GUI Slider", '1.00', 100, "GUI Slider", "Interface")

	MOP_ENUMBEG("Layout")

		MOP_ENUMELEMENT("Left")
		MOP_ENUMELEMENT("Center")
		MOP_ENUMELEMENT("Right")

	MOP_ENUMEND

	MOP_ENUM("Layout", "Layout")

	MOP_FLOAT("X position", 0.0f)
	MOP_FLOAT("Y position", 0.0f)

	MOP_FLOATEX("Alpha", 1.0f, 0.0f, 1.0f)

	MOP_STRING("Parent id", "")

	MOP_FLOAT("Width" , 25.0f)
	MOP_FLOAT("Height", 25.0f)

	MOP_LONG("Draw priority", 0)

	MOP_STRING("Back widget", "")
	MOP_STRING("Fore widget", "")

	MOP_STRING("Back widget sel", "")
	MOP_STRING("Fore widget sel", "")

	MOP_STRING("Back widget dis", "")
	MOP_STRING("Fore widget dis", "")

	MOP_FLOAT("Button width" , 5.0f)
	MOP_FLOAT("Button height", 5.0f)

	MOP_FLOATC("Pri offset down", 0.0f, "Отступ ползунка от левой границы")
	MOP_FLOATC("Pri offset up"  , 0.0f, "Отступ ползунка от правой границы")

	MOP_FLOATC("Sec offset down", 0.0f, "Отступ ползунка от верхней границы")

	MOP_BOOLC("Vertical", false, "-- не работает --")

//	MOP_STRING("GUI object", "")
	MOP_STRING("Value name", "")

	MOP_FLOAT("Value default", 0.0f)

	MOP_FLOAT("Value min", 0.0f)
	MOP_FLOAT("Value max", 1.0f)

	MOP_FLOAT("Value step", 0.1f)
	MOP_FLOATC("Value threshold", 0.001f, "Порог срабатывания триггера")

	MOP_FLOATEX("Autorepeat time", 0.2f, 0.1f, 50.0f)

	MOP_MISSIONTRIGGERC("OnChange", "")
	MOP_MISSIONTRIGGERC("OnTry", "")

	MOP_ARRAYBEG("Navigation links", 0, 100)

		MOP_STRING("Control", "")
		MOP_STRING("Object to go to", "")

	MOP_ARRAYEND

	MOP_BOOLC("Autofocus", true, "Получать фокус при наведении курсора")

	MOP_MISSIONTRIGGERC("GetFocus", "")
	MOP_MISSIONTRIGGERC("LosFocus", "")

	MOP_BOOL("Initially focused", false)
	MOP_BOOL("Focus by press", true)

	MOP_BOOL("Active" , true)
	MOP_BOOL("Visible", true)

MOP_ENDLIST(GUISlider)
