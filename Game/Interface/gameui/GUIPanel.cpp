#include "GUIPanel.h"

#include "..\Utils\InterfaceUtils.h"
#include "..\ProgressBar\csProgressBar.h"

GUIPanel::GUIPanel(void)
{
	preTime	   = 0.0f;
	preTimeMax = 1.0f;

	m_hor = null;
	m_ver = null;

	drawLevel = ML_GUI2;

	m_show = false;
}

GUIPanel::~GUIPanel(void)
{
}

void GUIPanel::ReadMOPs(MOPReader & reader)
{
	m_clipWidth	 = reader.Float();
	m_clipHeight = reader.Float();

	m_natSize.w = m_clipWidth;
	m_natSize.h = m_clipHeight;

	m_useClipping = reader.Bool();

	SetClipping(m_useClipping);

	m_texture  = reader.String().c_str();
	m_texAlpha = reader.Float();

	drawPriority = reader.Long();

	m_quadRender.SetTexture(m_texture);

	ConstString h = reader.String();
	ConstString v = reader.String();

	MOSafePointer sp;

	if( h.NotEmpty() )
	{
		FindObject(h,sp);

		m_hor = (csProgressBar *)sp.Ptr();
	}

	if( v.NotEmpty() )
	{
		FindObject(v,sp);

		m_ver = (csProgressBar *)sp.Ptr();
	}

	Activate(reader.Bool());
	
//	Show		(reader.Bool());
	nativeShow = reader.Bool();

	m_show = nativeShow;

	m_alphaOriginal = GetNativeAlpha();
	m_fadeAlpha = 1.0f;

	m_clipX = GetX(false);
	m_clipY = GetY(false);

	SizeTo(m_clipWidth,m_clipHeight);

	m_state = Normal;
}

bool GUIPanel::Create		  (MOPReader &reader)
{
	EditMode_Update(reader);
	return true;
}

void _cdecl GUIPanel::InitFunc(float, long)
{
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

//	Show(IsShow());
	Show(nativeShow);

	DelUpdate(&GUIPanel::InitFunc);
}

bool GUIPanel::EditMode_Update(MOPReader & reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs					   (reader);

	NotifyChildren(ParentChanged);

	m_rect = GetFullRect();

	Rect r; GetRect(r,false);

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	if( m_hor )
	{
		m_hor->SetMax( m_rect.r - m_rect.l - width);
		m_hor->SetPos(-m_rect.l);
	}

	if( m_ver )
	{
		m_ver->SetMax( m_rect.b - m_rect.t - height);
		m_ver->SetPos(-m_rect.t);
	}

//	SetUpdate(&GUIPanel::InitFunc,ML_GUI5);
	SetUpdate(&GUIPanel::InitFunc,ML_GUI1);

	return true;
}

void GUIPanel::Restart()
{
	Show(m_show);

	ScrollTo(0.0f,0.0f);
}

inline float cubic_curve(float t)
{
	float t2 = t*t; return 3*t2 - 2*t2*t;
}
#ifdef not_def
void _cdecl GUIPanel::Work(float deltaTime, long)
{
//	Rect r; GetRect(r,false);
	Rect r; GetPanelRect(r);

/*	Rect t; GetParentRect(t);
	Rect c; GetParentClipRect(c);

	r.l += t.l - c.l; r.r += t.l - c.l;
	r.t += t.t - c.t; r.b += t.t - c.t;*/

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

	float x = r.l; float width	= r.r - r.l;
	float y = r.t; float height = r.b - r.t;

	if( m_texture[0] )
	{
		m_quadRender.DrawQuad(x,y,width,height,0.0f,0.0f,
			1.0f,1.0f,10.0f + m_alphaOriginal*m_texAlpha);
	}

	if( EditMode_IsOn() && m_useClipping && !m_texture[0] )
	{
		m_helperQuad.DrawQuad(x,y,width,height,0.0f,0.0f,
			0.0f,0.0f,10.3f);
	}

	SetClipRect(NULL);

	if( GetFadeState() == BaseGUIElement::FadingIn )
	{
		m_fadeAlpha += (deltaTime/m_fadeTime)*m_alphaOriginal;

		if( m_fadeAlpha > m_alphaOriginal )
		{
			m_fadeAlpha = m_alphaOriginal;
			SetFadeState(FadingFinished);
		}
	}
	else
	if( GetFadeState() == BaseGUIElement::FadingOut )
	{
		m_fadeAlpha -= (deltaTime/m_fadeTime)*m_alphaOriginal;

		if( m_fadeAlpha < 0 )
		{
			m_fadeAlpha = 0.0f;
			SetFadeState(FadingFinished);
			Show(false);
		}
	}

	SetAlpha(m_fadeAlpha);

	const float AccelerationFactor = 1.0f;

	if( m_state == HScrolling )
	{
		m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;
		float x = Lerp(m_from.x,m_to.x,k);

		ScrollTo(x,m_to.y);
	}
	else
	if( m_state == VScrolling )
	{
		m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;
		float y = Lerp(m_from.y,m_to.y,k);

		ScrollTo(m_to.x,y);
	}
}
#endif
void _cdecl GUIPanel::Work(float deltaTime, long)
{
	if( EditMode_IsOn() && !EditMode_IsVisible())
		return;

	Draw();
	Update(deltaTime);
}

void GUIPanel::Update(float deltaTime)
{
	if( GetFadeState() == BaseGUIElement::FadingIn )
	{
		m_fade -= deltaTime;

		if( m_fade < 0.0f )
		{
			SetFadeState(FadingFinished);

			m_fade = 0.0f;
		}

		m_fadeAlpha = 1.0f - m_fade/m_fadeTime;

		SetAlpha(m_fadeAlpha*m_alphaOriginal);
	}
	else
	if( GetFadeState() == BaseGUIElement::FadingOut )
	{
		m_fade -= deltaTime;

		if( m_fade < 0.0f )
		{
			SetFadeState(FadingFinished); Show(false);

			m_fade = 0.0f;
		}

		m_fadeAlpha = m_fade/m_fadeTime;

		SetAlpha(m_fadeAlpha*m_alphaOriginal);
	}

//	SetAlpha(m_fadeAlpha);

	const float AccelerationFactor = 1.0f;

	if( m_state == HScrolling )
	{
		m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;
		float x = Lerp(m_from.x,m_to.x,k);

		ScrollTo(x,m_to.y);
	}
	else
	if( m_state == VScrolling )
	{
		m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;
		float y = Lerp(m_from.y,m_to.y,k);

		ScrollTo(m_to.x,y);
	}

	if( preTime > 0.0f )
	{
		preTime -= deltaTime;

		if( preTime < 0.0f )
			preTime = 0.0f;

		float k = preTime/preTimeMax;

		float dx = preTo.x - preFrom.x;
		float dy = preTo.y - preFrom.y;

		float a = sinf(PI*Lerp(3.0f,-0.5f,k))*k*k;

		float x = preTo.x + a*dx;
		float y = preTo.y + a*dy;

		MoveTo(x,y);
	}
}

void GUIPanel::Draw()
{
	if( InterfaceUtils::IsHide())
		return;

//	Rect r; GetRect(r,false);
	Rect r; GetPanelRect(r);

/*	Rect t; GetParentRect(t);
	Rect c; GetParentClipRect(c);

	r.l += t.l - c.l; r.r += t.l - c.l;
	r.t += t.t - c.t; r.b += t.t - c.t;*/

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

	float x = r.l; float width	= r.r - r.l;
	float y = r.t; float height = r.b - r.t;

	if( m_texture[0] )
	{
		m_quadRender.DrawQuad(x,y,width,height,0.0f,0.0f,
			1.0f,1.0f,10.0f + GetAlpha()*m_texAlpha);
	}

	if( EditMode_IsOn() && m_useClipping && !m_texture[0] )
	{
		m_helperQuad.DrawQuad(x,y,width,height,0.0f,0.0f,
			0.0f,0.0f,10.3f);
	}

	SetClipRect(NULL);
}

void GUIPanel::Show(bool isShow)
{
	BaseGUIElement::Show(isShow);

	if( EditMode_IsOn())
		DelUpdate(&GUIPanel::Work);

	if( IsShow())
	{
	//	SetUpdate(&GUIPanel::Work,drawLevel);

		BaseGUIElement *parent = GetParent();

		long level = drawLevel;

	/*	if( parent )
			level = parent->GetDrawLevel() + 100;*/

		if( GetAlpha() > 0.0f )
		{
			SetUpdate(&GUIPanel::Work,level + drawPriority);
		}
	}
	else
	{
		DelUpdate(&GUIPanel::Work);
	}
}

void GUIPanel::Activate(bool isActive)
{
	BaseGUIElement::Activate(isActive);
}

void GUIPanel::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"hscroll"))
	{
		if( m_state != Normal )
			return;

		m_from.x = GetOffsetX();
		m_from.y = GetOffsetY();

		m_state = HScrolling;

		m_scrollLength = 5.0f;
		m_scrollTime   = 1.0f;

		m_curScrollTime = 0.0f;

		if( numParams > 0 )
			m_scrollLength = (float)atof(params[0]);

		if( numParams > 1 )
			m_scrollTime   = (float)atof(params[1]);

		m_to = m_from;

		m_to.x += m_scrollLength*m_aspect;

		m_rect = GetFullRect();

		if( m_to.x > -m_rect.l )
			m_to.x = -m_rect.l;

		Rect r; GetRect(r);

		float width = r.r - r.l;

		if( m_to.x < -m_rect.r + width )
			m_to.x = -m_rect.r + width;

		m_scrollTime *= fabsf((m_to.x - m_from.x)/(m_scrollLength*m_aspect));

		if( m_scrollTime == 0.0f )
			m_state = Normal;

		if( m_hor )
		{
			m_hor->SetMax( m_rect.r - m_rect.l - width);
			m_hor->SetPos(-m_rect.l - m_to.x);
		}

		return;
	}
	else
	if( string::IsEqual(id,"vscroll"))
	{
		if( m_state != Normal )
			return;

		m_from.x = GetOffsetX();
		m_from.y = GetOffsetY();

		m_state = VScrolling;

		m_scrollLength = 5.0f;
		m_scrollTime   = 1.0f;

		m_curScrollTime = 0.0f;

		if( numParams > 0 )
			m_scrollLength = (float)atof(params[0]);

		if( numParams > 1 )
			m_scrollTime   = (float)atof(params[1]);

		m_to = m_from;

		m_to.y += m_scrollLength;

		m_rect = GetFullRect();

		if( m_to.y > -m_rect.t )
			m_to.y = -m_rect.t;

		Rect r; GetRect(r);

		float height = r.b - r.t;

		if( m_to.y < -m_rect.b + height )
			m_to.y = -m_rect.b + height;

		if( m_ver )
		{
			m_ver->SetMax( m_rect.b - m_rect.t - height);
			m_ver->SetPos(-m_rect.t - m_to.y);
		}

		return;
	}
	else
	if( string::IsEqual(id,"fadein"))
	{
		Show(true);

		SetFadeState(FadingIn);

		m_fadeTime = 2.0f;

		if( numParams > 0 )
			m_fadeTime = (float)atof(params[0]);

		m_fade = m_fadeTime;

		return;
	}
	else
	if( string::IsEqual(id,"fadeout"))
	{
		SetFadeState(FadingOut);

		m_fadeTime = 2.0f;

		if( numParams > 0 )
			m_fadeTime = (float)atof(params[0]);

		m_fade = m_fadeTime;

		m_alphaOriginal = GetNativeAlpha();

		return;
	}
	else
	if( string::IsEqual(id,"move"))
	{
		float x = 0;
		float y = 0;

		if( numParams > 0 ) x = (float)atof(params[0]);
		if( numParams > 1 )	y = (float)atof(params[1]);

		MoveTo(x,y);
	}
	else
	if( string::IsEqual(id,"present"))
	{
		float dx = GetWidth();
		float dy = 0;

		if( numParams > 0 ) dx = (float)atof(params[0]);
		if( numParams > 1 )	dy = (float)atof(params[1]);

		float dt = 1.0f;

		if( numParams > 2 )	dt = (float)atof(params[2]);

		if( dt <= 0.0 )
			dt  = 1.0f;

		preTimeMax = dt;
		preTime	   = dt;

		GetNativePos(preFrom.x,preFrom.y);

		preTo.x = preFrom.x + dx;
		preTo.y = preFrom.y + dy;
	}
	else
	{
		BaseGUIElement::Command(id,numParams,params);
	}
}

static char GUIDescription[] =

"GUI Panel.\n\n"
"    Use it to group GUI items\n"
"    NOTE: all sizes-positions are relative (not pixel sizes-positions)\n\n"
"Commands:\n\n"
"    hscroll [value] [time] - scrolls panel horizontally by value during time (default: 5,1)\n"
"    vscroll [value] [time] - scrolls panel vertically by value during time   (default: 5,1)\n\n"
"    fadein  [time]      - fades panel and it's children in\n"
"    fadeout [time]      - fades panel and it's children out\n"
"    move  [x] [y]       - move panel to new position (0,0 by default)\n"
"    present [x] [y] [time] - move panel with speceffect (<width>,0,1 by default)";

MOP_BEGINLISTCG(GUIPanel, "GUI Panel", '1.00', 100, GUIDescription, "Interface")

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

	MOP_FLOAT("X position", 25.0f)
	MOP_FLOAT("Y position", 25.0f)

//	MOP_FLOAT("Alpha", 1.0f)
	MOP_FLOATEX("Alpha", 1.0f, 0.0f, 1.0f)

	MOP_STRING("Parent id", "")

	MOP_FLOAT("Clip rect width", 50.0f)
	MOP_FLOAT("Clip rect height", 50.0f)

	MOP_BOOL("Use clip rect", false)

	MOP_STRING("Back texture", "")
	MOP_FLOAT ("Back alpha", 1.0f)

	MOP_LONG("Draw priority", 0)

	MOP_STRING("Hor bar", "")
	MOP_STRING("Ver bar", "")

	MOP_BOOL("Active" , true)
	MOP_BOOL("Visible", true)

MOP_ENDLIST(GUIPanel)
