#include "BaseGUICursor.h"
#include "..\..\..\common_h\InputSrvCmds.h"

const float max_time = 0.1f;

BaseGUICursor:: BaseGUICursor()
{
	cursor.Reset();

//	m_hided = true;
	m_hided = false;

	m_time = 0.0f;

	m_px = m_x = 0.5f;
	m_py = m_y = 0.5f;

	m_moved	  = false;
	m_clicked = false;

	m_active = null;

	m_show = false;
}

BaseGUICursor::~BaseGUICursor()
{
	if( m_active )
		m_active->Release();
}

void BaseGUICursor::Restart()
{
//	m_px = m_x = 0.5f;
//	m_py = m_y = 0.5f;

	m_moved	  = false;
	m_clicked = false;

	m_cnt = 0;

//	m_init = true; m_nomove = true;
	m_hide = true;

	Show(m_show);

//	SetUpdate(&BaseGUICursor::InitFn,ML_FIRST);
	SetUpdate(&BaseGUICursor::InitFn,ML_FIRST + 2);
}

void _cdecl BaseGUICursor::InitFn(float dltTime, long level)
{
	BaseGUIElement *cur = (BaseGUIElement *)cursor.Ptr();

	if( cur )
	{
		if( m_hide )
		{
			cur->Show(false);

			m_hide = false;
		}
	}

	DelUpdate(&BaseGUICursor::InitFn);
}

bool BaseGUICursor::Create		   (MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool BaseGUICursor::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void BaseGUICursor::Show(bool isShow)
{
	MissionObject::Show(isShow);

	#ifndef _XBOX

	if( isShow )
	{
	//	SetUpdate(&BaseGUICursor::Update,ML_FIRST);
		SetUpdate(&BaseGUICursor::Update,ML_FIRST + 2);

		SetUpdate(&BaseGUICursor::Redraw,ML_LAST);
	}
	else
	{
		DelUpdate(&BaseGUICursor::Update);
		DelUpdate(&BaseGUICursor::Redraw);
	}

	#endif
}

void _cdecl BaseGUICursor::Update(float dltTime, long level)
{
	if( EditMode_IsOn())
		return;

	long active = m_active ? m_active->Get(1) : 1;

	if( !active )
	{
		m_nomove = true;
		m_nodraw = 2;

		return;
	}

	BaseGUIElement *cur = (BaseGUIElement *)cursor.Ptr();

	if( cur )
	{
		Controls().FreeMouse();

		bool clicked = (
			Controls().GetControlStateType(m_lb) != CST_INACTIVE ||
			Controls().GetControlStateType(m_rb) != CST_INACTIVE);

		if( clicked )
		{
			m_clicked = true; m_cnt = 5;
		}
		else
		{
			if( m_clicked )
			{
				if( --m_cnt > 0 )
					m_clicked = true;
				else
					m_clicked = false;
			}
		}

		if( dltTime <= 0.0f )
		{
			if( m_drawOnPause )
				cur->Draw();

			return;
		}

		float x = Controls().GetControlStateFloat(m_mh)/Render().GetScreenInfo2D().dwWidth;
		float y = Controls().GetControlStateFloat(m_mv)/Render().GetScreenInfo2D().dwHeight;

		if( m_nomove )
		{
			m_nomove = false;

			x = 0.5f;
			y = 0.5f;
		}

	/*	m_x += x - 0.5f;
		m_y += y - 0.5f;

		if( m_x < 0.0f )
			m_x = 0.0f;
		if( m_x > 1.0f )
			m_x = 1.0f;

		if( m_y < 0.0f )
			m_y = 0.0f;
		if( m_y > 1.0f )
			m_y = 1.0f;*/
		m_x = x;
		m_y = y;

		if( m_x < 0.0f ) m_x = -1.0f;
		if( m_x > 1.0f ) m_x =  2.0f;

		if( m_y < 0.0f ) m_y = -1.0f;
		if( m_y > 1.0f ) m_y =  2.0f;

		//// прячем курсор при активации клавиатуры ////

		const char *s = Controls().GetPressedControl(null);

	//	if( s && strncmp(s,"Mouse",5))
	//		m_hided = true;

		////////////////////////////////////////////////

		if( m_hide )
		{
			cur->Show(false);

			m_hide = false;
		}

		if( m_init )
		{
			m_init = false;

			m_px = m_x = 0.5f;
			m_py = m_y = 0.5f;
		}

		if( m_px == m_x &&
			m_py == m_y )
		{
			m_moved = false;
		}
		else
		{
		/*	if( m_init )
			{
				m_init = false;
			}
			else
			{
				m_hided = false;
			}*/
			m_hided = false;

			m_moved = true;
		}

		m_px = m_x;
		m_py = m_y;

		if( m_hided )
		{
			m_time -= dltTime;

			if( m_time < 0.0f )
				m_time = 0.0f;
		}
		else
		{
			m_time += dltTime;

			if( m_time > max_time )
				m_time = max_time;
		}
	}
}

void _cdecl BaseGUICursor::Redraw(float dltTime, long level)
{
	if( EditMode_IsOn())
		return;

	if( m_nodraw > 0 )
	{
		m_nodraw--;

		return;
	}

	BaseGUIElement *cur = (BaseGUIElement *)cursor.Ptr();

	if( cur )
	{
	/*	cur->Draw(
			m_x + m_dx,
			m_y + m_dy,m_time/max_time*cursor->GetAlpha());*/

		float w,h; cur->GetSize(w,h);

		float asp = cur->GetAspect();

		cur->SetAlign(IGUIElement::OnLeft);
		cur->SetPosition(
			(m_x + m_dx)/asp - w*0.5f - w*m_off_x,
			(m_y + m_dy)	 - h*0.5f - h*m_off_y);
		cur->SetAlpha(m_time/max_time);

		cur->Draw();
		cur->Update(dltTime);
	}
}
/*
void BaseGUICursor::PostCreate()
{
	if( !cursor )
	{
		cursor = (BaseGUIElement *)FindObject(cursorName);

		if( cursor )
		{
			BaseGUIElement::Rect r; cursor->GetRect(r);

			m_dx = (r.r - r.l)*0.5f*0.01f;
			m_dy = (r.b - r.t)*0.5f*0.01f;
		}
		else
		{
			m_dx = 0.0f;
			m_dy = 0.0f;
		}

		if( cursor && !EditMode_IsOn())
			cursor->Show(false);
	}
}
*/
void BaseGUICursor::InitParams(MOPReader &reader)
{
	m_px = m_x = 0.5f;
	m_py = m_y = 0.5f;

	m_moved	  = false;
	m_clicked = false;

	m_cnt = 0;

	m_lb = Controls().FindControlByName( "LeftMouseButton");
	m_rb = Controls().FindControlByName("RightMouseButton");

	m_mh = Controls().FindControlByName("MouseH");
	m_mv = Controls().FindControlByName("MouseV");

	ConstString t = reader.String();

	if( !cursor.Validate() || t != cursor.Ptr()->GetObjectID() )
	{
		FindObject(t,cursor);

		BaseGUIElement *cur = (BaseGUIElement *)cursor.Ptr();

		if( cur )
		{
			BaseGUIElement::Rect r; cur->GetRect(r);

			m_dx = (r.r - r.l)*0.5f*0.01f;
			m_dy = (r.b - r.t)*0.5f*0.01f;
		}
		else
		{
			m_dx = 0.0f;
			m_dy = 0.0f;
		}

		if( cur && !EditMode_IsOn())
			cur->Show(false);
	}

//	cursorName = t;

	m_px = m_x = 0.5f;
	m_py = m_y = 0.5f;

	m_init = true; m_nomove = true;
	m_hide = true;

	m_nodraw = 0;

	m_off_x = reader.Float();
	m_off_y = reader.Float();

	m_drawOnPause = reader.Bool();

	Show(m_show = reader.Bool());
}

MOP_BEGINLISTCG(BaseGUICursor, "GUI Cursor", '1.00', 2000/*0*/, "GUI Cursor\n\n    Use to manage screen cursor", "Interface")

	MOP_STRING("Face widget", "")

	MOP_FLOATEX("Face hor offset",0.0f,0.0f,1.0f)
	MOP_FLOATEX("Face ver offset",0.0f,0.0f,1.0f)

	MOP_BOOL("Draw on pause", true)

	MOP_BOOL("Show", false)

MOP_ENDLIST(BaseGUICursor)
