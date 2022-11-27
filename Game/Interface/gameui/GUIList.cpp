#include "GUIList.h"

#include "GUIWidget.h"
#include "GUIButton.h"

#include "..\Utils\InterfaceUtils.h"
#include "..\ProgressBar\csProgressBar.h"

#include "BaseGUICursor.h"

GUIList::GUIList(void) : lines(_FL_)
{
	m_restart = false;

	drawLevel = ML_GUI2;

	curLine = -1;
	curItem = -1;

	befLine = -1;
	befItem = -1; maxItem = 0;

	scroll_x = 0.0f;
	scroll_y = 0.0f;

	maxWidth = 0;

	nativeShow = true;
}

GUIList::~GUIList(void)
{
}

void GUIList::Restart()
{
	Show(false);

	m_restart = true;

	ReCreate();
}

void GUIList::SkipMOPs(MOPReader & reader)
{
	reader.Float();	 // m_clipWidth
	reader.Float();	 // m_clipHeight

	reader.Bool();	 // m_useClipping

	reader.String().c_str(); // m_texture
	reader.Float();	 // m_texAlpha

	drawPriority = reader.Long();

	for( int i = 0 ; i < 2 ; i++ )
	{
		reader.String().c_str(); // prog name
	}

	int ln = reader.Array();

	for( int i = 0 ; i < ln ; i++ )
	{
		Line &line = lines[i];

		int in = reader.Array();

		for( int j = 0 ; j < in ; j++ )
		{
			Item &item = line.items[j];

			reader.String().c_str(); // item name

			item.onFoc.Init(reader);
			item.onLea.Init(reader);
			item.onSel.Init(reader);
		}
	}

	if( ln )
	{
		curLine = 0;

		if( lines[curLine].items )
		{
			curItem = 0;

		//	lines[curLine].items[curItem].onFoc.Activate(Mission(),false);
		}
	}
	else
	{
		curLine = -1;
		curItem = -1;
	}

	reader.Long();  // startLine
	reader.Long();  // startItem

	befLine = curLine;
	befItem = curItem; maxItem = curItem;

	reader.Float(); // itemWidth
	reader.Float(); // itemHeight

	reader.Bool();  // centered

	if( curLine >= 0 && curItem >= 0 && centered )
	{
		Rect r; GetRect(r,false);

		ScrollTo(
			0.5f*(r.r - r.l - itemWidth),
			0.5f*(r.b - r.t - itemHeight));
	}

	for( int i = 0 ; i < 4 ; i++ )
	{
		reader.String().c_str(); // butt name
	}

	reader.Float(); // btnWidth
	reader.Float(); // btnHeight

	BaseGUIElement::Activate(reader.Bool());

//	Show		(reader.Bool());
	nativeShow = reader.Bool();

//	MissionObject::Show(nativeShow);

	m_alphaOriginal = GetNativeAlpha();
	m_fadeAlpha = 1.0f;

	m_clipX = GetX();
	m_clipY = GetY();

	m_state = Normal;

//	ResetItems();
}

void GUIList::ReadMOPs(MOPReader & reader)
{
	m_clipWidth	 = reader.Float();
	m_clipHeight = reader.Float();

	SizeTo(m_clipWidth,m_clipHeight);

	m_useClipping = reader.Bool();

	SetClipping(m_useClipping);

	m_texture  = reader.String().c_str();
	m_texAlpha = reader.Float();

	drawPriority = reader.Long();

	m_quadRender.SetTexture(m_texture);

	ReleaseItems();

	for( int i = 0 ; i < 2 ; i++ )
	{
		Progress &prog = progs[i];

		prog.name = reader.String();

		if( FindObject(prog.name,prog.p))
		{
			if( !prog.p.Ptr()->Is(InterfaceUtils::GetPbarId()))
				 prog.p.Reset();
		}
	}

	lines.DelAll();

	maxWidth = 0;

	int ln = reader.Array();

	for( int i = 0 ; i < ln ; i++ )
	{
		Line &line = lines[lines.Add()];

		int in = reader.Array();

		for( int j = 0 ; j < in ; j++ )
		{
			Item &item = line.items[line.items.Add()];

			item.name = reader.String();

			if( FindObject(item.name,item.p))
			{
				if( !item.p.Ptr()->Is(InterfaceUtils::GetBaseId()))
					 item.p.Reset();
			}

			BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

			if( p )
			{
				p->data_a = i;
				p->data_b = j;
			}

			item.onFoc.Init(reader);
			item.onLea.Init(reader);
			item.onSel.Init(reader);
		}

		if( maxWidth < in )
			maxWidth = in;
	}

	if( ln )
	{
		curLine = 0;

		if( lines[curLine].items )
		{
			curItem = 0;

		//	lines[curLine].items[curItem].onFoc.Activate(Mission(),false);
		}
	}
	else
	{
		curLine = -1;
		curItem = -1;
	}

	startLine = reader.Long();
	startItem = reader.Long();

	befLine = curLine;
	befItem = curItem; maxItem = curItem;

	itemWidth  = reader.Float()*m_aspect;
	itemHeight = reader.Float();

	centered = reader.Bool();

	if( curLine >= 0 && curItem >= 0 && centered )
	{
		Rect r; GetRect(r,false);

		ScrollTo(
			0.5f*(r.r - r.l - itemWidth),
			0.5f*(r.b - r.t - itemHeight));
	}

	for( int i = 0 ; i < 4 ; i++ )
	{
		Button &butt = butts[i];

		butt.name = reader.String();

		if( FindObject(butt.name,butt.p))
		{
			if( !butt.p.Ptr()->Is(InterfaceUtils::GetButtonId()))
				 butt.p.Reset();
		}
	}

	btnWidth  = reader.Float()*m_aspect;
	btnHeight = reader.Float();

	BaseGUIElement::Activate(reader.Bool());

//	Show		(reader.Bool());
	nativeShow = reader.Bool();

//	MissionObject::Show(nativeShow);

	m_alphaOriginal = GetNativeAlpha();
	m_fadeAlpha = 1.0f;

	m_clipX = GetX();
	m_clipY = GetY();

	m_state = Normal;

	m_cursor = null;

	ResetItems();
}

bool GUIList::Create		 (MOPReader &reader)
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

void _cdecl GUIList::InitFunc(float, long)
{
	if( m_restart )
	{
		m_restart = false;
	//	;
		ResetItems();
	}
	else
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

	//	BaseGUIElement::Show(nativeShow);
	//	Show(nativeShow);

		if( EditMode_IsOn())
			DelUpdate(&GUIList::Work);
	}

	BaseGUIElement::Show(nativeShow);

	if( IsShow())
	{
		SetUpdate(&GUIList::Work,drawLevel + drawPriority);
	//	SetUpdate(&GUIList::Draw,drawLevel + drawPriority + 2000);
	}

//	if( IsActive() && IsShow())
	{
		if( curLine >= 0 &&	curItem >= 0 )
		{
			if( curLine <= 0 )
			{
				if( startLine >= 0 && startLine < lines )
					curLine = startLine;

				befLine = 0;
			}
			else
				befLine = curLine;

			if( curItem <= 0 )
			{
				if( startItem >= 0 && startItem < lines[curLine].items )
				{
					maxItem = curItem = startItem;
				}

				befItem = 0;
			}
			else
				befItem = curItem;

			Item &item = lines[curLine].items[curItem];

			BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

			if( p && p->Is(InterfaceUtils::GetButtonId()))
				p->SetFocus();
			else
			{
				LogicDebug("Item [%d,%d]: getFocus trugger",curLine,curItem);
				item.onFoc.Activate(Mission(),false);
			}

		//	befLine = 0;
		//	befItem = 0;

			UpdatePos(true);
		}
	}

	DelUpdate(&GUIList::InitFunc);
}

bool GUIList::RestartObject(MOPReader &reader)
{
	BaseGUIElement::SkipMOPs(reader);
	SkipMOPs(reader);

	NotifyChildren(ParentChanged);

	SetUpdate(&GUIList::InitFunc,ML_GUI5 - 1);

	return true;
}

bool GUIList::EditMode_Update(MOPReader &reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs					   (reader);

	NotifyChildren(ParentChanged);

	SetUpdate(&GUIList::InitFunc,ML_GUI5 - 1);

	return true;
}

inline float cubic_curve(float t)
{
	float t2 = t*t; return 3*t2 - 2*t2*t;
}
#ifdef not_def
void _cdecl GUIList::Work(float deltaTime, long)
{
	if( EditMode_IsOn())
	{
		bool reset = false;

		for( int i = 0 ; i < lines ; i++ )
		{
			Line &line = lines[i];

			for( int j = 0 ; j < line.items ; j++ )
			{
				Item &item = line.items[j];

				if(!item.p || !ValidatePointer(item.p,item.hash))
				{
					item.p = (BaseGUIElement *)FindObject(item.name);

					if( item.p )
					{
						ValidatePointer(item.p,item.hash);
						reset = true;
					}
				}
				else
				{
					if( strcmp(item.name,item.p->GetObjectID().c_str()))
					{
						item.p->Restrict(preserveNone);
						item.p->ReCreate();

						item.p = (BaseGUIElement *)FindObject(item.name);
					}
				}

				if( item.p )
				{
					item.p->data_a = i;
					item.p->data_b = j;
				}
			}
		}

	//	if( reset )
	//		ResetItems();

		for( int i = 0 ; i < 2 ; i++ )
		{
			Progress &prog = progs[i];

			if(!prog.p || !ValidatePointer(prog.p,prog.hash))
			{
				prog.p = (csProgressBar *)FindObject(prog.name);

				if( prog.p )
				{
					ValidatePointer(prog.p,prog.hash);
					reset = true;
				}
			}
			else
			{
				if( strcmp(prog.name,prog.p->GetObjectID().c_str()))
				{
					prog.p->Restrict(preserveNone);
					prog.p->ReCreate();

					prog.p = (csProgressBar *)FindObject(prog.name);
				}
			}
		}

		for( int i = 0 ; i < 4 ; i++ )
		{
			Button &butt = butts[i];

			if(!butt.p || !ValidatePointer(butt.p,butt.hash))
			{
				butt.p = (GUIButton *)FindObject(butt.name);

				if( butt.p )
				{
					ValidatePointer(butt.p,butt.hash);
					reset = true;
				}
			}
			else
			{
				if( strcmp(butt.name,butt.p->GetObjectID().c_str()))
				{
					butt.p->Restrict(preserveNone);
					butt.p->ReCreate();

					butt.p = (GUIButton *)FindObject(butt.name);
				}
			}
		}

		if( reset )
			ResetItems();
	}

	Rect r; GetRect(r,false);

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

	if( m_state != Normal )
	{
		m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;

		float x = Lerp(m_from.x,m_to.x,k);
		float y = Lerp(m_from.y,m_to.y,k);

		ScrollTo(x,y);
	}
}
#endif
void _cdecl GUIList::Work(float deltaTime, long level)
{
	if( EditMode_IsOn() && !EditMode_IsVisible())
		return;

	Draw();
	Update(deltaTime);

	Draw(deltaTime,level);
}

void GUIList::Update(float deltaTime)
{
	if( EditMode_IsOn())
	{
		bool reset = false;

		for( int i = 0 ; i < lines ; i++ )
		{
			Line &line = lines[i];

			for( int j = 0 ; j < line.items ; j++ )
			{
				Item &item = line.items[j];

				if(!item.p.Validate())
				{
					if( FindObject(item.name,item.p))
					{
						if( item.p.Ptr()->Is(InterfaceUtils::GetBaseId()))
						{
							reset = true;
						}
						else
							item.p.Reset();
					}
				}
				else
				{
					BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

					if( item.name != p->GetObjectID() )
					{
						p->Restrict(preserveNone);
						p->ReCreate();

						if( FindObject(item.name,item.p))
						{
							if( !item.p.Ptr()->Is(InterfaceUtils::GetBaseId()))
								 item.p.Reset();
						}
					}
				}

				BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

				if( p )
				{
					p->data_a = i;
					p->data_b = j;
				}
			}
		}

	//	if( reset )
	//		ResetItems();

		for( int i = 0 ; i < 2 ; i++ )
		{
			Progress &prog = progs[i];

			if(!prog.p.Validate())
			{
				if( FindObject(prog.name,prog.p))
				{
					if( prog.p.Ptr()->Is(InterfaceUtils::GetPbarId()))
					{
						reset = true;
					}
					else
						prog.p.Reset();
				}
			}
			else
			{
				csProgressBar *p = (csProgressBar *)prog.p.Ptr();

				if( prog.name != p->GetObjectID() )
				{
					p->Restrict(preserveNone);
					p->ReCreate();

					if( FindObject(prog.name,prog.p))
					{
						if( !prog.p.Ptr()->Is(InterfaceUtils::GetPbarId()))
							 prog.p.Reset();
					}
				}
			}
		}

		for( int i = 0 ; i < 4 ; i++ )
		{
			Button &butt = butts[i];

			if(!butt.p.Validate())
			{
				if( FindObject(butt.name,butt.p))
				{
					if( butt.p.Ptr()->Is(InterfaceUtils::GetButtonId()))
						reset = true;
					else
						butt.p.Reset();
				}
			}
			else
			{
				GUIButton *p = (GUIButton *)butt.p.Ptr();

				if( butt.name != p->GetObjectID() )
				{
					p->Restrict(preserveNone);
					p->ReCreate();

					if( FindObject(butt.name,butt.p))
					{
						if( !butt.p.Ptr()->Is(InterfaceUtils::GetButtonId()))
							 butt.p.Reset();
					}
				}
			}
		}

		if( reset )
			ResetItems();
	}

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

	if( m_state != Normal )
	{
		m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;

		float x = Lerp(m_from.x,m_to.x,k);
		float y = Lerp(m_from.y,m_to.y,k);

		ScrollTo(x,y);
	}
}

void GUIList::Draw()
{
	if( InterfaceUtils::IsHide())
		return;

	Rect r; GetRect(r,false);

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
}

void _cdecl GUIList::Draw(float deltaTime, long)
{
	if( EditMode_IsOn())
		return;

	if( curLine >= 0 &&	curItem >= 0 )
	{
		const Item &item = lines[curLine].items[curItem];

		BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

		if( p )
		{
			bool b = p->Is(InterfaceUtils::GetButtonId());

			if( m_state != Normal )
			{
				float a = m_curScrollTime/m_scrollTime;

				if( !b )
					p->SetAlpha(0.6f + a*0.4f);

			/*	if( befLine >= 0 && befItem >= 0 )
				{
					const Item &prev = lines[befLine].items[befItem];

					if( prev.p && !prev.p->Is(InterfaceUtils::GetButtonId()))
						prev.p->SetAlpha(1.0f - a*0.4f);
				}*/
			}
			else
			{
				if( !b )
					p->SetAlpha(1.0f);
			}
		}

		if( m_state != Normal )
		{
			float a = m_curScrollTime/m_scrollTime;

			if( befLine >= 0 && befItem >= 0 )
			{
				const Item &prev = lines[befLine].items[befItem];

				BaseGUIElement *p = (BaseGUIElement *)prev.p.Ptr();

				if( p && !p->Is(InterfaceUtils::GetButtonId()))
					p->SetAlpha(1.0f - a*0.4f);
			}
		}
	}

	if( Controls().GetControlStateType("Menu_Select") == CST_ACTIVATED )
	{
		if( curLine >= 0 && curItem >= 0 )
		{
			Item &item = lines[curLine].items[curItem];

			BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

			if( p && p->Is(InterfaceUtils::GetButtonId()))
				;
			else
			{
				LogicDebug("Item [%d,%d]: select trugger",curLine,curItem);
				item.onSel.Activate(Mission(),false);
			}
		}
	}

	//// обработка мыши ////

	if( EditMode_IsOn())
		return;

	if( !IsActive())
		return;

	if( m_state != Normal )
		return;

	if( !m_cursor )
	{
		MOSafePointer sp;

		FindObject(InterfaceUtils::GetCursorName(),sp);

		m_cursor = (BaseGUICursor *)sp.Ptr();
	}

	if( !m_cursor || m_cursor->IsHided())
		return;

	float x,y; m_cursor->GetPosition(x,y);

	float _x = x;
	float _y = y;

	Rect rect; GetClipRect(rect);

	x *= 100;
	y *= 100;

	if( !PointInRect(x,y,rect))
		return;

	x -= GetX(false);
	y -= GetY(false);

	float sx = 0.0f;
	float sy = 0.0f;

/*	Rect r; GetRect(r);

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	if( centered )
	{
	//	sx = -0.5f*(width  - itemWidth );
	//	sy = -0.5f*(height - itemHeight);
	}*/

	int i = int((y - GetOffsetY() - sy)/itemHeight);
	int j = int((x - GetOffsetX() - sx)/itemWidth );

	if( i >= 0 && i < lines )
	{
		Line &line = lines[i];

		if( j >= 0 && j < line.items )
		{
			bool en = true; BaseGUIElement *p = (BaseGUIElement *)lines[i].items[j].p.Ptr();

			if( p && p->Is(InterfaceUtils::GetButtonId()))
			{
				Rect r; p->GetRect(r);

				if( PointInRect(_x*100.0f,_y*100.0f,r))
					en = false;
			}

			if( en )
			{
				if( i != curLine || j != curItem || scroll_x != 0.0f || scroll_y != 0.0f )
				{
					if( Controls().GetControlStateType("LeftMouseButton") == CST_ACTIVATED )
					{
						befLine = curLine;
								  curLine = i;

						befItem = curItem;
								  curItem = j;

						maxItem = curItem;

						UpdatePos();
					}
				}

				if( Controls().GetControlStateType("LeftMouseDbl") == CST_ACTIVATED )
				{
					Item &item = lines[i].items[j];

					if( item.p.Ptr()->Is(InterfaceUtils::GetButtonId()))
						;
					else
					{
						LogicDebug("Item [%d,%d]: select trugger",i,j);
						item.onSel.Activate(Mission(),false);
					}
				}
			}
		}
	}
}

void GUIList::ReleaseItems()
{
	for( int i = 0 ; i < lines ; i++ )
	{
		Line &line = lines[i];

		for( int j = 0 ; j < line.items ; j++ )
		{
			Item &item = line.items[j];

			BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

			if( p )
			{
				p->Restrict(preserveNone);
				p->ReCreate();
			}
		}
	}

	UnRegisterAll();

	for( int i = 0 ; i < 2 ; i++ )
	{
		Progress &prog = progs[i];

		csProgressBar *p = (csProgressBar *)prog.p.Ptr();

		if( p )
		{
			p->Restrict(preserveNone);
			p->ReCreate();
		}
	}

	for( int i = 0 ; i < 4 ; i++ )
	{
		Button &butt = butts[i];

		GUIButton *p = (GUIButton *)butt.p.Ptr();

		if( p )
		{
			p->Restrict(preserveNone);
			p->ReCreate();
		}
	}
}

void GUIList::ResetItems()
{
	UnRegisterAll();

	float y = 0.0f;

	for( int i = 0 ; i < lines ; i++ )
	{
		Line &line = lines[i];

		float x = 0.0f;

		for( int j = 0 ; j < line.items ; j++ )
		{
			Item &item = line.items[j];

			BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

			if( p )
			{
				p->SetParent(GetObjectID());

				if( p->Is(InterfaceUtils::GetButtonId()))
				{
					p->MoveTo(
						x + (itemWidth  - p->GetWidth ())*0.5f,
						y + (itemHeight - p->GetHeight())*0.5f);
				}
				else
				{
					p->MoveTo(x,y);
					p->SizeTo(itemWidth/m_aspect,itemHeight);

					p->SetAlpha(0.6f);

					if( p->Is(InterfaceUtils::GetWidgetId()))
					{
						((GUIWidget *)p)->SetAlphaOriginal(0.6f);
					}
				}

				p->SetShiftAspect(false);
				p->SetWidthAspect(false);

				p->SetLayout(OnLeft);

				p->Restrict(preservePos|preserveSize|preserveParent|preserveLayout);

				Register(p);
			}

			x += itemWidth/m_aspect;
		}

		y += itemHeight;
	}

	Rect r; GetRect(r,false);

	float x = r.l;
		  y = r.t;

	float w = r.r - r.l;
	float h = r.b - r.t;

	float bw = btnWidth;
	float bh = btnHeight;

	for( int i = 0 ; i < 2 ; i++ )
	{
		Progress &prog = progs[i];

		csProgressBar *p = (csProgressBar *)prog.p.Ptr();

		if( p )
		{
			p->SetParent(ConstString());

			float px,py,pw,ph; p->GetNatRect(px,py,pw,ph);

			switch( i )
			{
				case 0:
				{
					float off = px*m_aspect;

					p->SizeTo(
						w - (bw + off)*2,
						ph);
					p->MoveTo(
						(x + bw + off)/m_aspect,
						y + h + (bh - ph)*0.5f);

					p->SetMax((float)0);
					p->SetPos((float)0);

				} break;

				case 1:
				{
					if( lines.Size() < 2 )
					{
						p->SizeTo(  0.0f,  0.0f);
						p->MoveTo(100.0f,100.0f);
					}
					else
					{
						p->SizeTo(
							pw*m_aspect,
							h - (bh + py)*2);
						p->MoveTo(
							(x + w + (bw - pw)*0.5f)/m_aspect,
							y + bh + py);
					}

					p->SetMax((float)(lines.Size() - 1));
					p->SetPos((float)(curLine));

				} break;
			}

			p->SetLayout(OnLeft);

			p->Restrict(preservePos|preserveSize|preserveParent);
		}
	}

	for( int i = 0 ; i < 4 ; i++ )
	{
		Button &butt = butts[i];

		GUIButton *p = (GUIButton *)butt.p.Ptr();

		if( p )
		{
			p->SetParent(ConstString());

			switch( i )
			{
				case 0:
					if( lines.Size() < 2 )
						p->MoveTo(100.0f,100.0f);
					else
						p->MoveTo((x + w)/m_aspect,y);
					break;

				case 1:
					if( lines.Size() < 2 )
						p->MoveTo(100.0f,100.0f);
					else
						p->MoveTo((x + w)/m_aspect,y + h - bh);
					break;

				case 2:
					if( lines.Size() < 2 && !progs[0].p.Ptr())
						p->MoveTo(
							(x - bw)/m_aspect,
							y + (h - bh)*0.5f);
					else
						p->MoveTo(x/m_aspect,y + h);
					break;

				case 3:
					if( lines.Size() < 2 && !progs[0].p.Ptr())
						p->MoveTo(
							(x + w)/m_aspect,
							y + (h - bh)*0.5f);
					else
						p->MoveTo((x + w - bw)/m_aspect,y + h);
					break;
			}

			p->SizeTo(btnWidth/m_aspect,btnHeight);

			p->SetLayout(OnLeft);

			p->Restrict(preservePos|preserveSize|preserveParent);
		}
	}
}

void GUIList::Show(bool isShow)
{
	bool sh = IsShow();

	BaseGUIElement::Show(isShow);

	DelUpdate(&GUIList::Work);
//	DelUpdate(&GUIList::Draw);

	if( IsShow())
	{
	//	SetUpdate(&GUIList::Work,drawLevel);
	//	SetUpdate(&GUIList::Draw,drawLevel + 2000);

	//	BaseGUIElement *parent = GetParent();

		long level = drawLevel;

	/*	if( parent )
			level = parent->GetDrawLevel() + 100;*/

		if( GetAlpha() > 0.0f )
		{
			SetUpdate(&GUIList::Work,level + drawPriority);
		//	SetUpdate(&GUIList::Draw,level + drawPriority + 2000);
		}
	}

	if( !IsActive())
		return;

	if( isShow != sh )
	{
		if( lines > 0 && curLine >= 0 && curLine < lines )
		{
			if( lines[curLine].items > 0 && curItem >= 0 && curItem < lines[curLine].items )
			{
				if( isShow )
				{
					LogicDebug("Item [%d,%d]: getFocus trugger",curLine,curItem);
					lines[curLine].items[curItem].onFoc.Activate(Mission(),false);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trugger",curLine,curItem);
					lines[curLine].items[curItem].onLea.Activate(Mission(),false);
				}
			}			
		}		
	}
}

void GUIList::Activate(bool isActive)
{
	bool act = IsActive();

	BaseGUIElement::Activate(isActive);

	if( !IsShow())
		return;

	if( isActive != act )
	{
		if( lines > 0 && curLine >= 0 && curLine < lines )
		{
			if( lines[curLine].items > 0 && curItem >= 0 && curItem < lines[curLine].items )
			{
				if( isActive )
				{
					LogicDebug("Item [%d,%d]: getFocus trugger",curLine,curItem);
					lines[curLine].items[curItem].onFoc.Activate(Mission(),false);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trugger",curLine,curItem);
					lines[curLine].items[curItem].onLea.Activate(Mission(),false);
				}
			}			
		}		
	}
}

void GUIList::ScrollHor(float dx)
{
	m_from.x = GetOffsetX();
	m_from.y = GetOffsetY();

	m_state = HScrolling;

	float k = fabsf(dx)/itemWidth;

	if( k == 0.0f )
		k  = 0.5f;

	m_scrollLength = dx;
	m_scrollTime   = 0.3f*k;

	m_curScrollTime = 0.0f;

	m_to = m_from;

	m_to.x += m_scrollLength;

	m_rect = GetFullRect(); Rect r; GetRect(r,false);

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	if( centered )
	{
		float sx = 0.5f*(width	- itemWidth);
		float sy = 0.5f*(height - itemHeight);

		m_rect.l -= sx;
		m_rect.r += sx;

		m_rect.t -= sy;
		m_rect.b += sy;
	}

	if( m_to.x > -m_rect.l )
		m_to.x = -m_rect.l;

	if( m_to.x < -m_rect.r + width )
		m_to.x = -m_rect.r + width;

	csProgressBar *p = (csProgressBar *)progs[0].p.Ptr();

	if( p )
	{
		p->SetMax((float)(lines[curLine].items.Size() - 1));
		p->SetPos((float)(curItem));
	}
}

void GUIList::ScrollVer(float dy)
{
	if( dy == 0.0f )
	{
		csProgressBar *p = (csProgressBar *)progs[1].p.Ptr();

		if( p )
		{
			p->SetMax((float)(lines.Size() - 1));
			p->SetPos((float)(curLine));
		}
		return;
	}

	m_from.x = GetOffsetX();
	m_from.y = GetOffsetY();

	m_state = VScrolling;

	float k = fabsf(dy)/itemHeight;

	if( k == 0.0f )
		k  = 0.5f;

	m_scrollLength = dy;

	float time = 0.3f*k;

	if( m_scrollTime < time )
		m_scrollTime = time;

	m_to.y += m_scrollLength;

	m_rect = GetFullRect();

//	;
	m_rect.b = lines.Size()*itemHeight;

	Rect r; GetRect(r,false);

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	if( centered )
	{
		float sx = 0.5f*(width	- itemWidth);
		float sy = 0.5f*(height - itemHeight);

		m_rect.l -= sx;
		m_rect.r += sx;

		m_rect.t -= sy;
		m_rect.b += sy;
	}

	if( m_to.y > -m_rect.t )
		m_to.y = -m_rect.t;

	if( m_to.y < -m_rect.b + height )
		m_to.y = -m_rect.b + height;

	csProgressBar *p = (csProgressBar *)progs[1].p.Ptr();

	if( p )
	{
		p->SetMax((float)(lines.Size() - 1));
		p->SetPos((float)(curLine));
	}
}

void GUIList::UpdatePos(bool init)
{
	Rect r; GetRect(r);

	float px = r.l + curItem*itemWidth;
	float py = r.t + curLine*itemHeight;

	float rx = GetX(false);
	float ry = GetY(false);

	float dx = 0.0f;
	float dy = 0.0f;

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	if( centered )
	{
		dx = (befItem - curItem)*itemWidth;
		dy = (befLine - curLine)*itemHeight;

		dx += scroll_x;
		dy += scroll_y;

		scroll_x = 0.0f;
		scroll_y = 0.0f;
	}
	else
	{
		if( px < rx )
		{
			dx = rx - px;
		}
		else
		if( px + itemWidth > rx + width)
		{
			dx = rx + width - (px + itemWidth);
		}

		if( py < ry )
		{
			dy = ry - py;
		}
		else
		if( py + itemHeight > ry + height)
		{
			dy = ry + height - (py + itemHeight);
		}
	}

	if( init )
	{
		Scroll(dx,dy);

		csProgressBar *p0 = (csProgressBar *)progs[0].p.Ptr();

		if( p0 )
		{
			p0->SetMax((float)(lines[curLine].items.Size() - 1));
			p0->SetPos((float)(curItem));
		}

		csProgressBar *p1 = (csProgressBar *)progs[1].p.Ptr();

		if( p1 )
		{
			p1->SetMax((float)(lines.Size() - 1));
			p1->SetPos((float)(curLine));
		}
	}
	else
	{
		ScrollHor(dx);
		ScrollVer(dy);
	
		if( curLine != befLine ||
			curItem != befItem )
		{
			if( befLine >= 0 && befItem >= 0 )
			{
				Item &prev = lines[befLine].items[befItem];

			//	;
				SetElementInFocus(this);

				BaseGUIElement *p = (BaseGUIElement *)prev.p.Ptr();

				if( p && p->Is(InterfaceUtils::GetButtonId()))
				{
				//	if( GetElementInFocus() == prev.p )
				//		SetElementInFocus(this);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trugger",befLine,befItem);
					prev.onLea.Activate(Mission(),false);
				}
			}

			Item &item = lines[curLine].items[curItem];

			BaseGUIElement *p = (BaseGUIElement *)item.p.Ptr();

			if( p && p->Is(InterfaceUtils::GetButtonId()))
				p->SetFocus();
			else
			{
				LogicDebug("Item [%d,%d]: getFocus trugger",curLine,curItem);
				item.onFoc.Activate(Mission(),false);
			}
		}
	}
}

void GUIList::OnChildNotify(BaseGUIElement *p, ChildNotification event)
{
	Assert(p)

	if( event == ChildSelected )
	{
		int i = p->data_a;
		int j = p->data_b;

		if( i == curLine && j == curItem )
			return;

		if( i >= 0 && i < lines && j >= 0 && j < lines[i].items )
		{
			scroll_x -= (j - curItem)*itemWidth;
			scroll_y -= (i - curLine)*itemHeight;

			befLine = curLine;
			befItem = curItem;

			curLine = i;
			curItem = j;

			Item &prev = lines[befLine].items[befItem];

			BaseGUIElement *p = (BaseGUIElement *)prev.p.Ptr();

			if( p && p->Is(InterfaceUtils::GetButtonId()))
			{
			//	if( GetElementInFocus() == prev.p )
			//		SetElementInFocus(this);
			}
			else
			{
				LogicDebug("Item [%d,%d]: losFocus trugger",befLine,befItem);
				prev.onLea.Activate(Mission(),false);
			}

			m_state = HScrolling;

			m_from.x = m_to.x = GetOffsetX();
			m_from.y = m_to.y = GetOffsetY();

			m_scrollTime = 0.2f; m_curScrollTime = 0.0f;

		//	UpdatePos();
		}
	}
}

void GUIList::Command(const char *id, dword numParams, const char **params)
{
	if( !IsActive())
		return;

	if( string::IsEmpty(id))
		return;

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
	{
		if( !IsShow())
			return;

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
		if( string::IsEqual(id,"up"))
		{
			if( m_state != Normal )
			{
				if( curLine >= 0 && curItem >= 0 )
				{
					BaseGUIElement *p = (BaseGUIElement *)lines[curLine].items[curItem].p.Ptr();

					if( p->Is(InterfaceUtils::GetButtonId()))
						p->SetFocus();
				}
				return;
			}

			if( curLine >= 0 )
			{
				if( curLine )
				{
					befItem = curItem;

					befLine = curLine;
							  curLine--;

					curItem = maxItem;

					if( curItem > lines[curLine].items - 1 )
						curItem = lines[curLine].items - 1;

					UpdatePos();
				}
				else if( curItem >= 0 )
				{
					BaseGUIElement *p = (BaseGUIElement *)lines[0].items[curItem].p.Ptr();

					if( p->Is(InterfaceUtils::GetButtonId()))
						p->SetFocus();
				}
			}
		}
		else
		if( string::IsEqual(id,"down"))
		{
			if( m_state != Normal )
			{
				if( curLine >= 0 && curItem >= 0 )
				{
					BaseGUIElement *p = (BaseGUIElement *)lines[curLine].items[curItem].p.Ptr();

					if( p->Is(InterfaceUtils::GetButtonId()))
						p->SetFocus();
				}
				return;
			}

			if( curLine >= 0 )
			{
				if( curLine < lines - 1 )
				{
					befItem = curItem;

					befLine = curLine;
							  curLine++;

					curItem = maxItem;

					if( curItem > lines[curLine].items - 1 )
						curItem = lines[curLine].items - 1;

					UpdatePos();
				}
				else if( curItem >= 0 )
				{
					BaseGUIElement *p = (BaseGUIElement *)lines.LastE().items[curItem].p.Ptr();

					if( p->Is(InterfaceUtils::GetButtonId()))
						p->SetFocus();
				}
			}
		}
		else
		if( string::IsEqual(id,"left"))
		{
			if( m_state != Normal )
			{
				if( curLine >= 0 && curItem >= 0 )
				{
					BaseGUIElement *p = (BaseGUIElement *)lines[curLine].items[curItem].p.Ptr();

					if( p->Is(InterfaceUtils::GetButtonId()))
						p->SetFocus();
				}
				return;
			}

			if( curLine >= 0 )
			{
				if( curItem >= 0 )
					maxItem = curItem;

				if( curItem >= 0 )
				{
					if( curItem )
					{
						befLine = curLine;

						befItem = curItem;
								  curItem--;

						maxItem = curItem;

						UpdatePos();
					}
					else
					{
						BaseGUIElement *p = (BaseGUIElement *)lines[curLine].items[curItem].p.Ptr();

						if( p->Is(InterfaceUtils::GetButtonId()))
							p->SetFocus();
					}
				}
			}
		}
		else
		if( string::IsEqual(id,"right"))
		{
			if( m_state != Normal )
			{
				if( curLine >= 0 && curItem >= 0 )
				{
					BaseGUIElement *p = (BaseGUIElement *)lines[curLine].items[curItem].p.Ptr();

					if( p->Is(InterfaceUtils::GetButtonId()))
						p->SetFocus();
				}
				return;
			}

			if( curLine >= 0 )
			{
				if( curItem >= 0 )
					maxItem = curItem;

				if( curItem >= 0 )
				{
					if( curItem < lines[curLine].items - 1 )
					{
						befLine = curLine;

						befItem = curItem;
								  curItem++;

						maxItem = curItem;

						UpdatePos();
					}
					else
					{
						BaseGUIElement *p = (BaseGUIElement *)lines[curLine].items[curItem].p.Ptr();

						if( p->Is(InterfaceUtils::GetButtonId()))
							p->SetFocus();
					}
				}
			}
		}
		else
		{
			BaseGUIElement::Command(id,numParams,params);
		}
	}
}

static char GUIDescription[] =

"GUI List.\n\n"
"    Use it to make list of GUI items\n"
"    NOTE: all sizes-positions are relative (not pixel sizes-positions)\n\n"
"Commands:\n\n"
"    left, right - scrolls list horizontally\n"
"    up, down - scrolls list vertically\n\n"
"    fadein  [time] - fades list in\n"
"    fadeout [time] - fades list out";

MOP_BEGINLISTCG(GUIList, "GUI List", '1.00', 2000, GUIDescription, "Interface")

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

	MOP_FLOAT("Clip rect width" , 50.0f)
	MOP_FLOAT("Clip rect height", 50.0f)

	MOP_BOOL("Use clip rect", false)

	MOP_STRING("Back texture", "")
	MOP_FLOAT ("Back alpha", 1.0f)

	MOP_LONG("Draw priority", 0)

	MOP_STRING("Hor bar", "")
	MOP_STRING("Ver bar", "")

	MOP_ARRAYBEG("Lines", 0, 100)

		MOP_ARRAYBEG("Items", 0, 100)

			MOP_STRING("Name", "")

			MOP_MISSIONTRIGGER("OnFocus" )
			MOP_MISSIONTRIGGER("OnLeave" )
			MOP_MISSIONTRIGGER("OnSelect")

		MOP_ARRAYEND

	MOP_ARRAYEND

	MOP_LONG("Start line", -1)
	MOP_LONG("Start item", -1)

	MOP_FLOAT("Item width" , 25.0f)
	MOP_FLOAT("Item height",  5.0f)

	MOP_BOOL("Centered", false)

	MOP_GROUPBEG("Controls")

		MOP_STRING("Up"	  , "")
		MOP_STRING("Down" , "")

		MOP_STRING("Left" , "")
		MOP_STRING("Right", "")

	MOP_GROUPEND()

	MOP_FLOAT("Button width" , 4.1f)
	MOP_FLOAT("Button height", 5.0f)

	MOP_BOOL("Active" , true)
	MOP_BOOL("Visible", true)

MOP_ENDLIST(GUIList)
