#include "GUISelector.h"

#include "GUIWidget.h"
#include "GUIButton.h"

#include "..\Utils\InterfaceUtils.h"
#include "..\ProgressBar\csProgressBar.h"

#include "BaseGUICursor.h"

GUISelector::GUISelector(void) : lines(_FL_),
	m_scales(_FL_),
	m_alphas(_FL_),
	m_drawps(_FL_)
{
	m_restart = false;

	drawLevel = ML_GUI2;

	curLine = -1;
	curItem = -1;

	befLine = -1;
	befItem = -1; maxItem = 0;

	marker.Reset();

	scroll_x = 0.0f;
	scroll_y = 0.0f;

	maxWidth = 0;

	m_state = Normal;

	m_dx = 0.0f;
	m_dy = 0.0f;

	nativeShow = true;
}

GUISelector::~GUISelector(void)
{
}

void GUISelector::Restart()
{
	Show(false);

	m_restart = true;

	ReCreate();
}

void GUISelector::SkipMOPs(MOPReader & reader)
{
	reader.Float();	 // m_clipWidth
	reader.Float();	 // m_clipHeight

	reader.Bool();	 // m_useClipping

	reader.String().c_str(); // m_texture
	reader.Float();	 // m_texAlpha

	drawPriority = reader.Long();

	m_quadRender.SetTexture(m_texture);

	int ln = reader.Array();

	for( int i = 0 ; i < ln ; i++ )
	{
		Line &line = lines[i];

		int in = reader.Array();

		for( int j = 0 ; j < in ; j++ )
		{
			Item &item = line.items[j];

			reader.String().c_str(); // item.normal.name
			reader.String().c_str(); // item.closed.name

			item.open = reader.Bool();

			BaseGUIElement *normal = (BaseGUIElement *)item.normal.p.Ptr();
			BaseGUIElement *closed = (BaseGUIElement *)item.closed.p.Ptr();

			item.onFoc.Init(reader);
			item.onLea.Init(reader);

			item.onSel.Init(reader);
			item.onTry.Init(reader);

			item.k = 1.0f;
			item.a = 0.6f;

			item.draw = 0;

			item.q = item.open ? normal : closed;

			if( normal )
				normal->Show( item.open);
			if( closed )
				closed->Show(!item.open);
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

	reader.Long();	 // startLine
	reader.Long();	 // startItem

	befLine = curLine;
	befItem = curItem; maxItem = curItem;

	reader.String().c_str(); // markerName
	reader.Float();	 // markerScale

	reader.Float();	 // itemWidth
	reader.Float();	 // itemHeight

	reader.Float();	 // horSpace
	reader.Float();	 // verSpace

	int sn = reader.Array();

	for( int i = 0 ; i < sn ; i++ )
	{
		reader.Float(); // scale
		reader.Float(); // alpha

		reader.Long();  // draw priority
	}

	reader.Float(); // m_scaleSpeed
	reader.Float(); // m_scrolSpeed

	reader.Float(); // m_delTime

	reader.Float(); // m_begTime
	reader.Float(); // m_endTime

	reader.Float(); // m_accTime

	reader.Bool();  // centered

	reader.Bool();  // m_isLooped

	reader.Enum();  // align (center/bottom)

	if( curLine >= 0 && curItem >= 0 && centered )
	{
		Rect r; GetRect(r,false);

		ScrollTo(
			0.5f*(r.r - r.l - itemWidth),
			0.5f*(r.b - r.t - itemHeight));
	}

	reader.String().c_str(); // m_up
	reader.String().c_str(); // m_down
	reader.String().c_str(); // m_left
	reader.String().c_str(); // m_right
	reader.String().c_str(); // m_sel

	BaseGUIElement::Activate(reader.Bool());

//	Show		(reader.Bool());
	nativeShow = reader.Bool();

	reader.Bool(); // m_silentActivate

//	MissionObject::Show(nativeShow);

	m_alphaOriginal = GetNativeAlpha();
	m_fadeAlpha = 1.0f;

	m_clipX = GetX();
	m_clipY = GetY();

	m_state = Normal;

//	ResetItems();
	SetUpdate(&GUISelector::Pre_Func,ML_FIRST);
}

void GUISelector::ReadMOPs(MOPReader & reader)
{
	m_isLooped = true;

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

	/*
	Max: prog.name = ""; поэтому результате не найдёться ниодного объекта

	for( int i = 0 ; i < 2 ; i++ )
	{
		Progress &prog = progs[i];

	//	prog.name = reader.String().c_str();
		prog.name = "";

		if( FindObject(prog.name,prog.p))
		{
			if( !prog.p.Ptr()->Is(InterfaceUtils::GetPbarId()))
				 prog.p.Reset();
		}
	}
	*/

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

			item.normal.name = reader.String();
			item.closed.name = reader.String();

			item.open = reader.Bool();

			if( FindObject(item.normal.name,item.normal.p))
			{
				if( !item.normal.p.Ptr()->Is(InterfaceUtils::GetBaseId()))
					 item.normal.p.Reset();
			}

			BaseGUIElement *normal = (BaseGUIElement *)item.normal.p.Ptr();

			if( normal )
			{
				normal->data_a = i;
				normal->data_b = j;
			}

			if( FindObject(item.closed.name,item.closed.p))
			{
				if( !item.closed.p.Ptr()->Is(InterfaceUtils::GetBaseId()))
					 item.closed.p.Reset();
			}

			BaseGUIElement *closed = (BaseGUIElement *)item.closed.p.Ptr();

			if( closed )
			{
				closed->data_a = i;
				closed->data_b = j;
			}

			item.onFoc.Init(reader);
			item.onLea.Init(reader);

			item.onSel.Init(reader);
			item.onTry.Init(reader);

			item.k = 1.0f;
			item.a = 0.6f;

			item.draw = 0;

			item.q = item.open ? normal : closed;

			if( normal )
				normal->Show( item.open);
			if( closed )
				closed->Show(!item.open);
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

	markerName = reader.String();

	FindObject(markerName,marker);

	markerScale = reader.Float();

	itemWidth  = reader.Float()*m_aspect;
	itemHeight = reader.Float();

	horSpace = reader.Float()*m_aspect;
	verSpace = reader.Float();

	m_scales.DelAll();
	m_alphas.DelAll();
	m_drawps.DelAll();

	int sn = reader.Array();

	for( int i = 0 ; i < sn ; i++ )
	{
		m_scales.Add(reader.Float());
		m_alphas.Add(reader.Float());

		m_drawps.Add(reader.Long());
	}

	m_scaleSpeed = reader.Float();
	m_scrolSpeed = reader.Float();

	m_delTime = reader.Float();

	m_begTime = reader.Float();
	m_endTime = reader.Float();

	m_accTime = reader.Float();

	centered = reader.Bool();

	m_isLooped = reader.Bool();

	switch( reader.Enum().c_str()[0] )
	{
		case 'C':
			m_bottom = false;
			break;

		case 'B':
			m_bottom = true;
			break;
	};

	if( curLine >= 0 && curItem >= 0 && centered )
	{
		Rect r; GetRect(r,false);

		ScrollTo(
			0.5f*(r.r - r.l - itemWidth),
			0.5f*(r.b - r.t - itemHeight));
	}

/*	for( int i = 0 ; i < 4 ; i++ )
	{
		Button &butt = butts[i];

		butt.name = reader.String().c_str();

		butt.p = (GUIButton *)FindObject(butt.name);
		butt.hash = -1;

		if( butt.p )
		{
			if( butt.p->Is(InterfaceUtils::GetButtonId()))
				ValidatePointer(butt.p,butt.hash);
			else
				butt.p = null;
		}
	}*/
	for( int i = 0 ; i < 4 ; i++ )
	{
		Button &butt = butts[i];

		butt.name.Empty();

		butt.p.Reset();
	}

	const char *name;

	name = reader.String().c_str(); m_up	= Controls().FindControlByName(name);
	name = reader.String().c_str(); m_down	= Controls().FindControlByName(name);
	name = reader.String().c_str(); m_left	= Controls().FindControlByName(name);
	name = reader.String().c_str(); m_right	= Controls().FindControlByName(name);
	name = reader.String().c_str(); m_sel	= Controls().FindControlByName(name);

//	btnWidth  = reader.Float()*m_aspect;
//	btnHeight = reader.Float();
	btnWidth  = 20.0f;
	btnHeight = 20.0f;

	BaseGUIElement::Activate(reader.Bool());

//	Show		(reader.Bool());
	nativeShow = reader.Bool();

	m_silentActivate = reader.Bool();

//	MissionObject::Show(nativeShow);

	m_alphaOriginal = GetNativeAlpha();
	m_fadeAlpha = 1.0f;

	m_clipX = GetX();
	m_clipY = GetY();

	m_state = Normal;

	m_cursor.Reset();

	ResetItems();
}

bool GUISelector::Create		 (MOPReader &reader)
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

void GUISelector::PostCreate()
{
	FindObject(InterfaceUtils::GetCursorName(),m_cursor);

/*	if( IsShow() == false )
	{
		if( marker )
			marker->Notify(ParentHided);

		for( int i = 0 ; i < lines ; i++ )
		{
			for( int j = 0 ; j < lines[i].items ; j++ )
			{
				const Item &item = lines[i].items[j];

				if( item.q )
					item.q->Notify(ParentHided);
			}
		}
	}*/

	BaseGUIElement::Show(nativeShow);
}

void _cdecl GUISelector::Pre_Func(float, long)
{
	DelUpdate(&GUISelector::Pre_Func);

	ResetItems();
}

void _cdecl GUISelector::InitFunc(float, long)
{
	DelUpdate(&GUISelector::InitFunc);

	if( m_restart )
	{
		m_restart = false;

		BaseGUIElement::Show(nativeShow);

		if( IsShow())
		{
			SetUpdate(&GUISelector::Work,drawLevel + drawPriority);
		//	SetUpdate(&GUISelector::Draw,drawLevel + drawPriority + 2000);
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

				if( curLine < lines && curItem < lines[curLine].items )
				{
					Item &item = lines[curLine].items[curItem];

				/*	if( item.normal.p && item.normal.p->Is(InterfaceUtils::GetButtonId()))
						item.normal.p->SetFocus();
					else
					{
						if( startLine >= 0 && startItem >= 0 )
						{
							LogicDebug("Item [%d,%d]: getFocus trigger",curLine,curItem);
							item.onFoc.Activate(Mission(),false);
						}

						item.k = 1.0f;
					}*/

					item.k = 1.0f;
				}

			//	befLine = 0;
			//	befItem = 0;

				UpdatePos(false,false,true,startLine >= 0 && startItem >= 0 && IsShow() && IsActive());

				if( IsShow())
				{
					UpdateElements(0.0f,true);
				}
			}
		}

		return;
	}

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

//	Show(IsShow());

//	BaseGUIElement::Show(nativeShow);
//	Show(nativeShow);

	if( EditMode_IsOn())
		DelUpdate(&GUISelector::Work);

	BaseGUIElement::Show(nativeShow);

	if( IsShow())
	{
		SetUpdate(&GUISelector::Work,drawLevel + drawPriority);
	//	SetUpdate(&GUISelector::Draw,drawLevel + drawPriority + 2000);
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

			if( curLine < lines && curItem < lines[curLine].items )
			{
				Item &item = lines[curLine].items[curItem];

			/*	if( item.normal.p && item.normal.p->Is(InterfaceUtils::GetButtonId()))
					item.normal.p->SetFocus();
				else
				{
					if( startLine >= 0 && startItem >= 0 )
					{
						LogicDebug("Item [%d,%d]: getFocus trigger",curLine,curItem);
						item.onFoc.Activate(Mission(),false);
					}

					item.k = 1.0f;
				}*/

				item.k = 1.0f;
			}

		//	befLine = 0;
		//	befItem = 0;

			UpdatePos(false,false,true,startLine >= 0 && startItem >= 0 && IsShow() && IsActive());

			if( IsShow())
			{
				UpdateElements(0.0f,true);
			}
		}
	}
}

bool GUISelector::RestartObject(MOPReader &reader)
{
	BaseGUIElement::SkipMOPs(reader);
	SkipMOPs(reader);

	NotifyChildren(ParentChanged);

	SetUpdate(&GUISelector::InitFunc,ML_GUI5 - 1);

	if( m_initComplete == false )
	{
		m_restart = false;
	}

	return true;
}

bool GUISelector::EditMode_Update(MOPReader &reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs					   (reader);

	NotifyChildren(ParentChanged);

	SetUpdate(&GUISelector::InitFunc,ML_GUI5 - 1);

	m_initComplete = false;

	return true;
}

inline float cubic_curve(float t)
{
	float t2 = t*t; return 3*t2 - 2*t2*t;
}

void _cdecl GUISelector::Work(float deltaTime, long level)
{
	if( EditMode_IsOn() && !EditMode_IsVisible())
		return;

	Draw();
	Update(deltaTime);

	Draw(deltaTime,level);
}

void GUISelector::Update(float deltaTime)
{
	bool edit_mode = EditMode_IsOn();

	if( edit_mode )
	{
		bool reset = false;

		for( int i = 0 ; i < lines ; i++ )
		{
			Line &line = lines[i];

			for( int j = 0 ; j < line.items ; j++ )
			{
				Item &item = line.items[j];

				if(!item.normal.p.Validate())
				{
					if( FindObject(item.normal.name,item.normal.p))
					{
						reset = true;
					}
				}
				else
				{
					BaseGUIElement *normal = (BaseGUIElement *)item.normal.p.Ptr();

					if( item.normal.name != normal->GetObjectID() )
					{
						normal->Restrict(preserveNone);
						normal->ReCreate();

						FindObject(item.normal.name,item.normal.p);
					}
				}

				BaseGUIElement *normal = (BaseGUIElement *)item.normal.p.Ptr();

				if( normal )
				{
					normal->data_a = i;
					normal->data_b = j;
				}

				////

				if(!item.closed.p.Validate())
				{
					if( FindObject(item.closed.name,item.closed.p))
					{
						reset = true;
					}
				}
				else
				{
					BaseGUIElement *closed = (BaseGUIElement *)item.closed.p.Ptr();

					if( item.closed.name != closed->GetObjectID() )
					{
						closed->Restrict(preserveNone);
						closed->ReCreate();

						FindObject(item.closed.name,item.closed.p);
					}
				}

				BaseGUIElement *closed = (BaseGUIElement *)item.closed.p.Ptr();

				if( closed )
				{
					closed->data_a = i;
					closed->data_b = j;
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
					reset = true;
				}
			}
			else
			{
				csProgressBar *p = (csProgressBar *)prog.p.Ptr();

				if( prog.name != p->GetObjectID() )
				{
					p->Restrict(preserveNone);
					p->ReCreate();

					FindObject(prog.name,prog.p);
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
					reset = true;
				}
			}
			else
			{
				GUIButton *p = (GUIButton *)butt.p.Ptr();

				if( butt.name != p->GetObjectID() )
				{
					p->Restrict(preserveNone);
					p->ReCreate();

					FindObject(butt.name,butt.p);
				}
			}
		}

		if( reset )
			ResetItems();

		if(!marker.Validate())
		{
			FindObject(markerName,marker);
		}
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
/*
//	const int kn = 3;
	int kn = m_scales;

//	static float _sk[kn] = {1.3f,1.15f,1.0f};
//	static float _ak[kn] = {1.0f,0.80f,0.6f};

//	const float upk = 0.02f;
	const float upk = 2.50f;

//	const float rek = 0.02f;

	if( curLine >= 0 && curItem >= 0 )
	{
		float px = GetOffsetX();
		float py = GetOffsetY();

		float et = m_state == Normal ? 0.0f : sqrtf((m_to.x - px)*(m_to.x - px) + (m_to.y - py)*(m_to.y - py));

		et *= 0.2f;

		float kk = deltaTime*upk*m_scaleSpeed;

		float dd = 1.0f/(0.5f + et*et);

		//////////////////////////////////////////////
		// Поддерживаем пока только одномерный лист //
		//////////////////////////////////////////////

		int n = lines[0].items;

		bool hor = n > 1;

		if( !hor )
			n = lines;

		int li = 0;
		int ii = 0;

		for( int i = 0 ; i < n ; i++ )
		{
			int k = 0;

			if( hor )
			{
				k = abs(ii - curItem);

				if( m_isLooped )
				{
					int t = curItem > ii ? n - curItem + ii : n - ii + curItem;

					if( k > t )
						k = t;
				}
			}
			else
			{
				k = abs(li - curLine);

				if( m_isLooped )
				{
					int t = curLine > li ? n - curLine + li : n - li + curLine;

					if( k > t )
						k = t;
				}
			}

			float sk = 1.0f;
			float ak = 1.0f;

			long draw = 0;

			if( kn > 0 )
			{
				if( k > kn - 1 )
					k = kn - 1;

				sk = m_scales[k];
				ak = m_alphas[k]; draw = m_drawps[k];
			}

		//	float sk = _sk[k];
		//	float ak = _ak[k];

			if( !lines[li].items )
				continue;

			Item &item = lines[li].items[ii];

			if( item.q )
			{
				if( item.k != sk || item.a != ak )
				{
					if( edit_mode )
					{
						item.k = sk;
					}
					else
					{
						float dk = (sk - item.k)*dd;

						dk = sign(dk)*sqrtf(fabsf(dk));

						dk *= kk;

						if( fabsf(dk) > fabsf(sk - item.k))
							dk = sk - item.k;

						item.k += dk;
					}

					float iw = itemWidth/m_aspect;
					float ih = itemHeight;

					float w = iw*item.k;
					float h = ih*item.k;

					item.q->SizeTo(w,h);

					float x = ii*(iw + horSpace/m_aspect);
					float y = li*(ih + verSpace);

					item.q->MoveTo(x - 0.5f*(w - iw),y - 0.5f*(h - ih));

					if( edit_mode )
					{
						item.a = ak;
					}
					else
					{
						float da = (ak - item.a)*kk;

						da = sign(da)*sqrtf(fabsf(da));

						if( fabsf(da) > fabsf(ak - item.a))
							da = ak - item.a;

						item.a += da;
					}

					item.q->SetAlpha(item.a);
				}

				if( item.draw != draw )
				{
					item.q->ResetDrawUpdate(item.draw = draw);
				}
			}

			if( hor )
				ii++;
			else
				li++;
		}

		if( curLine < lines && curItem < lines[curLine].items )
		{
			const Item &cur = lines[curLine].items[curItem];

			if( marker && cur.q )
			{
				float x,y;

				cur.q->GetNativePos(x,y);

				float w = markerScale*cur.q->GetWidth();
				float h = markerScale*cur.q->GetHeight();

				x -= 0.5f*(w - cur.q->GetWidth());
				y -= 0.5f*(h - cur.q->GetHeight());

				marker->MoveTo(x,y);
				marker->SizeTo(w,h);
			}
		}
	}
*/	UpdateElements(deltaTime,edit_mode);

//	SetAlpha(m_fadeAlpha);

	if( edit_mode )
		return;

	if( IsActive())
	{
	//	const float rpTime = 0.65f;
		float rpTime = m_delTime;

	//	const float ctTime = 0.35f;
	//	const float mnTime = 0.15f;
		float ctTime = m_begTime;
		float mnTime = m_endTime;

	//	const float reTime = 4.00f;
		float reTime = m_accTime;

	//	const float k = 0.55f;

		ControlStateType stU = Controls().GetControlStateType(m_up);
		ControlStateType stD = Controls().GetControlStateType(m_down);
		ControlStateType stL = Controls().GetControlStateType(m_left);
		ControlStateType stR = Controls().GetControlStateType(m_right);

		int dx = 0;
		int dy = 0;

		switch( stU )
		{
			case CST_ACTIVATED:
				btU.start(rpTime); dy--;
				break;

			case CST_INACTIVATED:
				btU.en = false;
				break;

			case CST_ACTIVE:
				if( btU.en )
				{
					btU.time += deltaTime; btU.t += deltaTime;

					if( btU.time >= btU.timeMax )
					{
						btU.time = 0.0f; dy--;

						if( btU.timeMax >= rpTime )
						{
							btU.timeMax  = ctTime; btU.t = 0.0f;
						}
						else
							btU.timeMax = Lerp(ctTime,mnTime,coremin(1.0f,btU.t/reTime));
					}
				}
				break;
		}

		switch( stD )
		{
			case CST_ACTIVATED:
				btD.start(rpTime); dy++;
				break;

			case CST_INACTIVATED:
				btD.en = false;
				break;

			case CST_ACTIVE:
				if( btD.en )
				{
					btD.time += deltaTime; btD.t += deltaTime;

					if( btD.time >= btD.timeMax )
					{
						btD.time = 0.0f; dy++;

						if( btD.timeMax >= rpTime )
						{
							btD.timeMax  = ctTime; btD.t = 0.0f;
						}
						else
							btD.timeMax = Lerp(ctTime,mnTime,coremin(1.0f,btD.t/reTime));
					}
				}
				break;
		}

		switch( stL )
		{
			case CST_ACTIVATED:
				btL.start(rpTime); dx--;
				break;

			case CST_INACTIVATED:
				btL.en = false;
				break;

			case CST_ACTIVE:
				if( btL.en )
				{
					btL.time += deltaTime; btL.t += deltaTime;

					if( btL.time >= btL.timeMax )
					{
						btL.time = 0.0f; dx--;

						if( btL.timeMax >= rpTime )
						{
							btL.timeMax  = ctTime; btL.t = 0.0f;
						}
						else
							btL.timeMax = Lerp(ctTime,mnTime,coremin(1.0f,btL.t/reTime));
					}
				}
				break;
		}

		switch( stR )
		{
			case CST_ACTIVATED:
				btR.start(rpTime); dx++;
				break;

			case CST_INACTIVATED:
				btR.en = false;
				break;

			case CST_ACTIVE:
				if( btR.en )
				{
					btR.time += deltaTime; btR.t += deltaTime;

					if( btR.time >= btR.timeMax )
					{
						btR.time = 0.0f; dx++;

						if( btR.timeMax >= rpTime )
						{
							btR.timeMax  = ctTime; btR.t = 0.0f;
						}
						else
							btR.timeMax = Lerp(ctTime,mnTime,coremin(1.0f,btR.t/reTime));
					}
				}
				break;
		}

		if( dy )
		{
			if( dy < 0 )
				MoveUp();
			else
				MoveDown();
		}
		if( dx )
		{
			if( dx < 0 )
				MoveLeft();
			else
				MoveRight();
		}
	}

	const float AccelerationFactor = 1.0f;

	if( m_state != Normal )
	{
	/*	m_curScrollTime += deltaTime;

		if( m_curScrollTime > m_scrollTime )
		{
			m_curScrollTime = m_scrollTime;
			m_state = Normal;
		}

		float k = m_curScrollTime/m_scrollTime;

		float x = Lerp(m_from.x,m_to.x,k);
		float y = Lerp(m_from.y,m_to.y,k);

		ScrollTo(x,y);*/

		float px = GetOffsetX();
		float py = GetOffsetY();

		float scr = sqrtf(m_scrolSpeed);

	//	float k = deltaTime*20.0f*m_scrolSpeed;
		float k = deltaTime*20.0f*scr;

		float dx = (m_to.x - px);
		float dy = (m_to.y - py);

		dx = sign(dx)*sqrtf(fabsf(dx));
		dy = sign(dy)*sqrtf(fabsf(dy));

		dx *= k;
		dy *= k;

		if( fabsf(dx) >= fabsf(px - m_to.x))
		{
			dx = m_to.x - px;
		}
		if( fabsf(dy) >= fabsf(py - m_to.y))
		{
			dy = m_to.y - py;
		}

		px += dx;
		py += dy;

	//	if( px == GetOffsetX() && py == GetOffsetY())
		if( px == m_to.x && py == m_to.y )
			m_state = Normal;

		ScrollTo(px,py);

		m_dx = m_to.x - px;
		m_dy = m_to.y - py;

	/*	m_tt += deltaTime;

		if( m_tt >= m_tmax )
			m_tt  = m_tmax;

		float px = GetOffsetX();
		float py = GetOffsetY();

		float t  = m_tt;
		float t2 = t*t;

		float dx = m_xv*t + 0.5f*m_xa*t2;
		float dy = m_yv*t + 0.5f*m_ya*t2;

	//	if( fabsf(dx) >= fabsf(px - m_to.x))
	//	{
	//		dx = m_to.x - px;
	//	}
	//	if( fabsf(dy) >= fabsf(py - m_to.y))
	//	{
	//		dy = m_to.y - py;
	//	}

		px = m_from.x + dx;
		py = m_from.y + dy;

	//	if( px == m_to.x && py == m_to.y )
	//	if( fabsf(px - m_to.x) <= 1.0f && fabsf(py - m_to.y) <= 1.0f )
		if( m_tt >= m_tmax )
			m_state = Normal;

		ScrollTo(px,py);

		m_dx = m_to.x - px;
		m_dy = m_to.y - py;*/
	}
}

void GUISelector::UpdateElements(float deltaTime, bool instant)
{
//	const int kn = 3;
	int kn = m_scales;

//	static float _sk[kn] = {1.3f,1.15f,1.0f};
//	static float _ak[kn] = {1.0f,0.80f,0.6f};

//	const float upk = 0.02f;
	const float upk = 2.50f;

//	const float rek = 0.02f;

	if( curLine >= 0 && curItem >= 0 )
	{
		float px = GetOffsetX();
		float py = GetOffsetY();

		float et = m_state == Normal ? 0.0f : sqrtf((m_to.x - px)*(m_to.x - px) + (m_to.y - py)*(m_to.y - py));

		et *= 0.2f;

		float kk = deltaTime*upk*m_scaleSpeed;

		float dd = 1.0f/(0.5f + et*et);

		//////////////////////////////////////////////
		// Поддерживаем пока только одномерный лист //
		//////////////////////////////////////////////

		int n = lines[0].items;

		bool hor = n > 1;

		if( !hor )
			n = lines;

		int li = 0;
		int ii = 0;

		for( int i = 0 ; i < n ; i++ )
		{
			int k = 0;

			if( hor )
			{
				k = abs(ii - curItem);

				if( m_isLooped )
				{
					int t = curItem > ii ? n - curItem + ii : n - ii + curItem;

					if( k > t )
						k = t;
				}
			}
			else
			{
				k = abs(li - curLine);

				if( m_isLooped )
				{
					int t = curLine > li ? n - curLine + li : n - li + curLine;

					if( k > t )
						k = t;
				}
			}

			float sk = 1.0f;
			float ak = 1.0f;

			float ck = 1.0f;

			long draw = 0;

			if( kn > 0 )
			{
				if( k > kn - 1 )
					k = kn - 1;

				sk = m_scales[k];
				ak = m_alphas[k]; draw = m_drawps[k];

				ck = m_scales[0];
			}

		//	float sk = _sk[k];
		//	float ak = _ak[k];

			if( !lines[li].items )
				continue;

			Item &item = lines[li].items[ii];

			if( item.q )
			{
				if( item.k != sk || item.a != ak )
				{
					if( /*edit_mode*/instant )
					{
						item.k = sk;
					}
					else
					{
						float dk = (sk - item.k)*dd;

						dk = sign(dk)*sqrtf(fabsf(dk));

						dk *= kk;

						if( fabsf(dk) > fabsf(sk - item.k))
							dk = sk - item.k;

						item.k += dk;
					}

					float iw = itemWidth/m_aspect;
					float ih = itemHeight;

					float w = iw*item.k;
					float h = ih*item.k;

					item.q->SizeTo(w,h);

					item.q->SetFontScale(item.k);

					float x = ii*(iw + horSpace/m_aspect);
					float y = li*(ih + verSpace);

					if( m_bottom && hor )
						y += 0.5f*(ih*(ck - item.k) + (GetHeight() - ih*ck));

					item.q->MoveTo(x - 0.5f*(w - iw),y - 0.5f*(h - ih));

					if( /*edit_mode*/instant )
					{
						item.a = ak;
					}
					else
					{
						float da = (ak - item.a)*kk;

						da = sign(da)*sqrtf(fabsf(da));

						if( fabsf(da) > fabsf(ak - item.a))
							da = ak - item.a;

						item.a += da;
					}

					item.q->SetAlpha(item.a);
				}

				if( item.draw != draw )
				{
					item.q->ResetDrawUpdate(item.draw = draw);
				}
			}

			if( hor )
				ii++;
			else
				li++;
		}

		if( curLine < lines && curItem < lines[curLine].items )
		{
			const Item &cur = lines[curLine].items[curItem];

			GUIWidget *mark = (GUIWidget *)marker.Ptr();

			if( mark && cur.q )
			{
				float x,y;

				cur.q->GetNativePos(x,y);

				float w = markerScale*cur.q->GetWidth();
				float h = markerScale*cur.q->GetHeight();

				x -= 0.5f*(w - cur.q->GetWidth());
				y -= 0.5f*(h - cur.q->GetHeight());

				mark->MoveTo(x,y);
				mark->SizeTo(w,h);
			}
		}
	}
}

void GUISelector::Draw()
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

	if( EditMode_IsOn() /*&& m_useClipping*/ && !m_texture[0] )
	{
		m_helperQuad.DrawQuad(x,y,width,height,0.0f,0.0f,
			0.0f,0.0f,10.3f);
	}
}

void _cdecl GUISelector::Draw(float deltaTime, long)
{
	if( EditMode_IsOn())
		return;

/*	if( curLine >= 0 &&	curItem >= 0 )
	{
		const Item &item = lines[curLine].items[curItem];

		if( item.p )
		{
			bool b = item.p->Is(InterfaceUtils::GetButtonId());

			if( m_state != Normal )
			{
				float a = m_curScrollTime/m_scrollTime;

				if( !b )
					item.p->SetAlpha(0.6f + a*0.4f);

			//	if( befLine >= 0 && befItem >= 0 )
			//	{
			//		const Item &prev = lines[befLine].items[befItem];

			//		if( prev.p && !prev.p->Is(InterfaceUtils::GetButtonId()))
			//			prev.p->SetAlpha(1.0f - a*0.4f);
			//	}
			}
			else
			{
				if( !b )
					item.p->SetAlpha(1.0f);
			}
		}

		if( m_state != Normal )
		{
			float a = m_curScrollTime/m_scrollTime;

			if( befLine >= 0 && befItem >= 0 )
			{
				const Item &prev = lines[befLine].items[befItem];

				if( prev.p && !prev.p->Is(InterfaceUtils::GetButtonId()))
					prev.p->SetAlpha(1.0f - a*0.4f);
			}
		}
	}*/

	if( IsActive())
	{
		if( Controls().GetControlStateType(m_sel) == CST_ACTIVATED )
		{
			if( curLine >= 0 && curItem >= 0 )
			{
				Item &item = lines[curLine].items[curItem];

				if( item.q && item.q->Is(InterfaceUtils::GetButtonId()))
					;
				else
				{
					if( item.open )
					{
						LogicDebug("Item [%d,%d]: select trigger",curLine,curItem);
						item.onSel.Activate(Mission(),false);
					}
					else
					{
						LogicDebug("Item [%d,%d]: try trigger",curLine,curItem);
						item.onTry.Activate(Mission(),false);
					}
				}
			}
		}
	}

/*	//// обработка мыши ////

	if( EditMode_IsOn())
		return;

	if( !IsActive())
		return;

	if( m_state != Normal )
		return;

	if( !m_cursor )
		 m_cursor = (BaseGUICursor *)FindObject(InterfaceUtils::GetCursorName());

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

	float _X = GetX(false);
	float _Y = GetY(false);

	x -= _X;
	y -= _Y;

	float sx = 0.0f;
	float sy = 0.0f;

//	Rect r; GetRect(r);

//	float width	 = r.r - r.l;
//	float height = r.b - r.t;

//	if( centered )
//	{
//	//	sx = -0.5f*(width  - itemWidth );
//	//	sy = -0.5f*(height - itemHeight);
//	}

	int i = int((y - GetOffsetY() - sy)/itemHeight);
	int j = int((x - GetOffsetX() - sx)/itemWidth );

	if( i >= 0 && i < lines )
	{
		Line &line = lines[i];

		if( line.items > 0 )
		{
			bool lsh = false;
			bool rsh = false;

			if( m_isLooped )
			{
				int last = line.items - 1;

				if( j < 0 )
				{
					j = last; lsh = true;
				}
				else
				if( j > last )
				{
					j = 0;	  rsh = true;
				}
			}

			if( j >= 0 && j < line.items )
			{
				bool en = true; BaseGUIElement *q = lines[i].items[j].q;

				if( q && q->Is(InterfaceUtils::GetButtonId()))
				{
					Rect r; q->GetRect(r);

					if( PointInRect(_x*100.0f,_y*100.0f,r))
						en = false;
				}

				if( en )
				{
					if( i != curLine || j != curItem || scroll_x != 0.0f || scroll_y != 0.0f )
					{
						if( Controls().GetControlStateType("LeftMouseButton") == CST_ACTIVATED )
						{
							float lb = _X + GetOffsetX() + curItem*itemWidth;
							float tb = _Y + GetOffsetY() + curLine*itemHeight;

							befLine = curLine;
									  curLine = i;

							befItem = curItem;
									  curItem = j;

							maxItem = curItem;

							if( lsh )
							{
								ScrollTo(
									GetOffsetX() - GetFullW(),
									GetOffsetY());
							}
							else
							if( rsh )
							{
								ScrollTo(
									GetOffsetX() + GetFullW(),
									GetOffsetY());
							}

							UpdatePos(_x*100.0f < lb,_y*100.0f < tb);
						}
					}

					if( Controls().GetControlStateType("LeftMouseDbl") == CST_ACTIVATED )
					{
						Item &item = lines[i].items[j];

						if( item.q->Is(InterfaceUtils::GetButtonId()))
							;
						else
						{
							LogicDebug("Item [%d,%d]: select trigger",i,j);
							item.onSel.Activate(Mission(),false);
						}
					}
				}
			}
		}
	}*/
}

void GUISelector::ReleaseItems()
{
	for( int i = 0 ; i < lines ; i++ )
	{
		Line &line = lines[i];

		for( int j = 0 ; j < line.items ; j++ )
		{
			Item &item = line.items[j];

			BaseGUIElement *normal = (BaseGUIElement *)item.normal.p.Ptr();

			if( normal )
			{
				normal->Restrict(preserveNone);
				normal->ReCreate();
			}

			BaseGUIElement *closed = (BaseGUIElement *)item.closed.p.Ptr();

			if( closed )
			{
				closed->Restrict(preserveNone);
				closed->ReCreate();
			}

			item.q = null;
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

	GUIWidget *mark = (GUIWidget *)marker.Ptr();

	if( mark )
	{
		mark->Restrict(preserveNone);
		mark->ReCreate();
	}
}

void GUISelector::ResetItems()
{
	UnRegisterAll();

	float y = 0.0f;

	float cscal = m_scales ? m_scales[0] : 1.0f;

	float scale = m_scales ? m_scales.LastE() : 1.0f;
	float alpha = m_alphas ? m_alphas.LastE() : 1.0f;

	long draw = m_drawps ? m_drawps.LastE() : 0;

	float iw = itemWidth/m_aspect;
	float ih = itemHeight;

	float sw = scale*iw;
	float sh = scale*ih;

	float dx = 0.5f*(iw - sw);
	float dy = 0.5f*(ih - sh);

	bool hor = lines && lines[0].items > 1;

	if( m_bottom && hor )
		dy += 0.5f*(ih*(cscal - scale) + (GetHeight() - ih*cscal));

	for( int i = 0 ; i < lines ; i++ )
	{
		Line &line = lines[i];

		float x = 0.0f;

		for( int j = 0 ; j < line.items ; j++ )
		{
			Item &item = line.items[j];

			BaseGUIElement *normal = (BaseGUIElement *)item.normal.p.Ptr();

			if( normal )
			{
				normal->SetParent(GetObjectID());

				if( normal->Is(InterfaceUtils::GetButtonId()))
				{
					normal->MoveTo(
						x + (itemWidth  - normal->GetWidth ())*0.5f,
						y + (itemHeight - normal->GetHeight())*0.5f);
				}
				else
				{
					normal->MoveTo(x + dx,y + dy);
					normal->SizeTo(sw,sh);

					normal->SetAlpha(alpha);

					if( normal->Is(InterfaceUtils::GetWidgetId()))
					{
						((GUIWidget *)normal)->SetAlphaOriginal(alpha);
					}

				//	item.a = alpha;
				//	item.k = scale;
					item.a = -1.0f;
					item.k = -1.0f;

					item.draw = draw;
				}

				normal->SetShiftAspect(false);
				normal->SetWidthAspect(false);

				normal->SetLayout(OnLeft);

				normal->Restrict(preservePos|preserveSize|preserveParent|preserveLayout);

				normal->Show( item.open);

				Register(normal);
			}

			BaseGUIElement *closed = (BaseGUIElement *)item.closed.p.Ptr();

			if( closed )
			{
				closed->SetParent(GetObjectID());

				if( closed->Is(InterfaceUtils::GetButtonId()))
				{
					closed->MoveTo(
						x + (itemWidth  - closed->GetWidth ())*0.5f,
						y + (itemHeight - closed->GetHeight())*0.5f);
				}
				else
				{
					closed->MoveTo(x + dx,y + dy);
					closed->SizeTo(sw,sh);

					closed->SetAlpha(alpha);

					if( closed->Is(InterfaceUtils::GetWidgetId()))
					{
						((GUIWidget *)closed)->SetAlphaOriginal(alpha);
					}

				//	item.a = alpha;
				//	item.k = scale;
					item.a = -1.0f;
					item.k = -1.0f;

					item.draw = draw;
				}

				closed->SetShiftAspect(false);
				closed->SetWidthAspect(false);

				closed->SetLayout(OnLeft);

				closed->Restrict(preservePos|preserveSize|preserveParent|preserveLayout);

				closed->Show(!item.open);

				Register(closed);
			}

			item.q = item.open ? normal : closed;

			x += (itemWidth + horSpace)/m_aspect;
		}

		y += itemHeight + verSpace;
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

			switch( i )
			{
				case 0:
				{
					float off = p->GetNativeX()*m_aspect;

					p->SizeTo(
						w - (bw + off)*2,
						p->GetHeight());
					p->MoveTo(
						(x + bw + off)/m_aspect,
						y + h + (bh - p->GetHeight())*0.5f);

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
							p->GetWidth()*m_aspect,
							h - (bh + p->GetNativeY())*2);
						p->MoveTo(
							(x + w + (bw - p->GetWidth())*0.5f)/m_aspect,
							y + bh + p->GetNativeY());
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

	GUIWidget *mark = (GUIWidget *)marker.Ptr();

	if( mark )
	{
		mark->SetParent(GetObjectID());

		mark->SetLayout(OnLeft);
	//	mark->SizeTo(itemWidth/m_aspect,itemHeight);

		mark->Restrict(preservePos/*|preserveSize*/|preserveParent);
	}
}

void GUISelector::Show(bool isShow)
{
	bool sh = IsShow();

	BaseGUIElement::Show(isShow);

	DelUpdate(&GUISelector::Work);
//	DelUpdate(&GUISelector::Draw);

	if( IsShow())
	{
	//	SetUpdate(&GUISelector::Work,drawLevel);
	//	SetUpdate(&GUISelector::Draw,drawLevel + 2000);

	//	BaseGUIElement *parent = GetParent();

		long level = drawLevel;

	/*	if( parent )
			level = parent->GetDrawLevel() + 100;*/

		if( GetAlpha() > 0.0f )
		{
			SetUpdate(&GUISelector::Work,level + drawPriority);
		//	SetUpdate(&GUISelector::Draw,level + drawPriority + 2000);
		}
	}

	if( !IsActive())
		return;

	if( isShow != sh )
	{
		if( IsShow())
		{
			UpdateElements(0.0f,true);
		}

		if( lines > 0 && curLine >= 0 && curLine < lines )
		{
			if( lines[curLine].items > 0 && curItem >= 0 && curItem < lines[curLine].items )
			{
				if( isShow )
				{
					LogicDebug("Item [%d,%d]: getFocus trigger",curLine,curItem);
					lines[curLine].items[curItem].onFoc.Activate(Mission(),false);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trigger",curLine,curItem);
					lines[curLine].items[curItem].onLea.Activate(Mission(),false);
				}
			}			
		}		
	}
}

void GUISelector::Activate(bool isActive)
{
	bool act = IsActive();

	BaseGUIElement::Activate(isActive);

	if( !IsShow())
		return;

	if( isActive != act && !m_silentActivate )
	{
		if( lines > 0 && curLine >= 0 && curLine < lines )
		{
			if( lines[curLine].items > 0 && curItem >= 0 && curItem < lines[curLine].items )
			{
				if( isActive )
				{
					LogicDebug("Item [%d,%d]: getFocus trigger",curLine,curItem);
					lines[curLine].items[curItem].onFoc.Activate(Mission(),false);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trigger",curLine,curItem);
					lines[curLine].items[curItem].onLea.Activate(Mission(),false);
				}
			}			
		}		
	}
}

void GUISelector::ScrollHor(float dx)
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

/*	m_rect = GetFullRect();*/ Rect r; GetRect(r,false);
	m_rect.l = 0.0f;
	m_rect.t = 0.0f;

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	float max_w = maxWidth*(itemWidth + horSpace);
	float max_h = lines.Size()*(itemHeight + verSpace);

	if( maxWidth > 0 )
		max_w -= horSpace;

	if( lines.Size() > 0 )
		max_h -= verSpace;

	if( max_w < width )
		max_w = width;
	if( max_h < height )
		max_h = height;

	m_rect.r = m_rect.l + max_w;
	m_rect.b = m_rect.t + max_h;

	if( centered )
	{
		float sx = 0.5f*(width	- itemWidth);
		float sy = 0.5f*(height - itemHeight);

		m_rect.l -= sx;
		m_rect.r += sx;

		m_rect.t -= sy;
		m_rect.b += sy;
	}

	if( m_isLooped == false )
	{
		if( m_to.x > -m_rect.l )
			m_to.x = -m_rect.l;

		if( m_to.x < -m_rect.r + width )
			m_to.x = -m_rect.r + width;
	}

//	float R = m_to.x - m_from.x;

/*	R = sign(R)*sqrtf(fabsf(R));

	float K = 10.0f;

	m_xv =  K*R;
	m_xa = -0.5f*K*K;*/
/*	m_xa = -R*0.5f;
	m_xv = R;*/

/*	float R = m_to.x - m_from.x;

	m_xv = R*3.0f*m_scrolSpeed;

	float T = 2.0f*R/m_xv;

	m_xa = 2.0f*(R - m_xv*T)/(T*T);
//	m_xa = -(m_xv*m_xv)/(2.0f*R);

	m_tt =  0.0f; m_tmax = T;*/

	csProgressBar *p = (csProgressBar *)progs[0].p.Ptr();

	if( p )
	{
		p->SetMax((float)(lines[curLine].items.Size() - 1));
		p->SetPos((float)(curItem));
	}
}

void GUISelector::ScrollVer(float dy)
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

/*	m_rect = GetFullRect();*/ Rect r; GetRect(r,false);
	m_rect.l = 0.0f;
	m_rect.t = 0.0f;

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	float max_w = maxWidth*(itemWidth + horSpace);
	float max_h = lines.Size()*(itemHeight + verSpace);

	if( maxWidth > 0 )
		max_w -= horSpace;

	if( lines.Size() > 0 )
		max_h -= verSpace;

	if( max_w < width )
		max_w = width;
	if( max_h < height )
		max_h = height;

	m_rect.r = m_rect.l + max_w;
	m_rect.b = m_rect.t + max_h;

	if( centered )
	{
		float sx = 0.5f*(width	- itemWidth);
		float sy = 0.5f*(height - itemHeight);

		m_rect.l -= sx;
		m_rect.r += sx;

		m_rect.t -= sy;
		m_rect.b += sy;
	}

	if( m_isLooped == false )
	{
		if( m_to.y > -m_rect.t )
			m_to.y = -m_rect.t;

		if( m_to.y < -m_rect.b + height )
			m_to.y = -m_rect.b + height;
	}

//	float R = m_to.y - m_from.y;

/*	R = sign(R)*sqrtf(fabsf(R));

	float K = 10.0f;

	m_yv =  K*R;
	m_ya = -0.5f*K*K;*/
/*	m_ya = -R*0.5f;
	m_yv = R;*/

/*	float R = m_to.y - m_from.y;

	m_yv = R*3.0f*m_scrolSpeed;

	float T = 2.0f*R/m_yv;

	m_ya = 2.0f*(R - m_yv*T)/(T*T);
//	m_ya = -(m_yv*m_yv)/(2.0f*R);

	m_tt =  0.0f; m_tmax = T;*/

	csProgressBar *p = (csProgressBar *)progs[1].p.Ptr();

	if( p )
	{
		p->SetMax((float)(lines.Size() - 1));
		p->SetPos((float)(curLine));
	}
}

void GUISelector::UpdatePos(bool left, bool up, bool init, bool onFocActivate)
{
	Rect r; GetRect(r);

	float px = r.l + curItem*(itemWidth  + horSpace);
	float py = r.t + curLine*(itemHeight + verSpace);

	float rx = GetX(false);
	float ry = GetY(false);

	float dx = 0.0f;
	float dy = 0.0f;

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	if( centered )
	{
		int x_off = befItem - curItem;
		int y_off = befLine - curLine;

		if( x_off )
		{
			if( left )
			{
				if( x_off < 0 )
					x_off =  (maxWidth + x_off);
			}
			else
			{
				if( x_off > 0 )
					x_off = -(maxWidth - x_off);
			}
		}

		if( y_off )
		{
			if( up )
			{
				if( y_off < 0 )
					y_off =  (lines + y_off);
			}
			else
			{
				if( y_off > 0 )
					y_off = -(lines - y_off);
			}
		}

		dx = (x_off)*(itemWidth  + horSpace);
		dy = (y_off)*(itemHeight + verSpace);

		if( m_state != Normal )
		{
		//	dx += m_to.x - GetOffsetX();
		//	dy += m_to.y - GetOffsetY();
			dx += m_dx;
			dy += m_dy;
		}

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

		if( curLine != befLine ||
			curItem != befItem )
		{
			if( befLine >= 0 && befItem >= 0 )
			{
				Item &prev = lines[befLine].items[befItem];

				if( prev.q && prev.q->Is(InterfaceUtils::GetButtonId()))
				{
				//	if( GetElementInFocus() == prev.p )
				//		SetElementInFocus(this);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trigger",befLine,befItem);
					prev.onLea.Activate(Mission(),false);
				}
			}
		}

		if( curLine < lines && curItem < lines[curLine].items )
		{
			Item &item = lines[curLine].items[curItem];

			if( item.q && item.q->Is(InterfaceUtils::GetButtonId()))
				item.q->SetFocus();
			else
			{
				if( onFocActivate )
				{
					LogicDebug("Item [%d,%d]: getFocus trigger",curLine,curItem);
					item.onFoc.Activate(Mission(),false);
				}
			}
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

				if( prev.q && prev.q->Is(InterfaceUtils::GetButtonId()))
				{
				//	if( GetElementInFocus() == prev.p )
				//		SetElementInFocus(this);
				}
				else
				{
					LogicDebug("Item [%d,%d]: losFocus trigger",befLine,befItem);
					prev.onLea.Activate(Mission(),false);
				}
			}

			Item &item = lines[curLine].items[curItem];

			if( item.q && item.q->Is(InterfaceUtils::GetButtonId()))
				item.q->SetFocus();
			else
			{
				if( onFocActivate )
				{
					LogicDebug("Item [%d,%d]: getFocus trigger",curLine,curItem);
					item.onFoc.Activate(Mission(),false);
				}
			}
		}
	}

	if( curLine >= 0 && curItem >= 0 )
	{
		GUIWidget *mark = (GUIWidget *)marker.Ptr();

		if( mark && curLine < lines && curItem < lines[curLine].items )
		{
			if( lines[curLine].items[curItem].q )
			{
				const Item &cur = lines[curLine].items[curItem];

				float x,y;

				cur.q->GetPosition(x,y);

				float w = cur.k*itemWidth/m_aspect;
				float h = cur.k*itemHeight;

				mark->SizeTo(w*markerScale,h*markerScale);

				float dx = 0.5f*(mark->GetWidth () - w);
				float dy = 0.5f*(mark->GetHeight() - h);

				mark->MoveTo(x*100.0f - dx,y*100.0f - dy);
			}
		}
	}
}

void GUISelector::OnChildNotify(BaseGUIElement *p, ChildNotification event)
{
	Assert(p)

	if( IsActive() == false )
		return;

/*	if( event == ChildSelected )
	{
		int i = p->data_a;
		int j = p->data_b;

		if( i == curLine && j == curItem )
			return;

		if( i >= 0 && i < lines && j >= 0 && j < lines[i].items )
		{
			scroll_x -= (j - curItem)*(itemWidth  + horSpace);
			scroll_y -= (i - curLine)*(itemHeight + verSpace);

			befLine = curLine;
			befItem = curItem;

			curLine = i;
			curItem = j;

			Item &prev = lines[befLine].items[befItem];

			if( prev.q && prev.q->Is(InterfaceUtils::GetButtonId()))
			{
			//	if( GetElementInFocus() == prev.p )
			//		SetElementInFocus(this);
			}
			else
			{
				LogicDebug("Item [%d,%d]: losFocus trigger",befLine,befItem);
				prev.onLea.Activate(Mission(),false);
			}

			m_state = HScrolling;

			m_from.x = m_to.x = GetOffsetX();
			m_from.y = m_to.y = GetOffsetY();

			m_scrollTime = 0.2f; m_curScrollTime = 0.0f;

		//	UpdatePos();
		}
	}
	else*/
	if( event == ChildClicked )
	{
		int i = p->data_a;
		int j = p->data_b;

		if( i == curLine && j == curItem )
			return;

		if( i >= 0 && i < lines && j >= 0 && j < lines[i].items )
		{
			BaseGUICursor *cursor = (BaseGUICursor *)m_cursor.Ptr();

			if( cursor )
			{
				float x,y; cursor->GetPosition(x,y);

				x *= 100.0f;
				y *= 100.0f;

				const Item &cur = lines[i].items[j];

				for( int m = 0 ; m < lines ; m++ )
				{
					for( int n = 0 ; n < lines[m].items ; n++ )
					{
						const Item &item = lines[m].items[n];

						if( item.draw > cur.draw )
						{
							if( item.q )
							{
								Rect r; item.q->GetRect(r);

								if( PointInRect(x,y,r))
									return;
							}
						}
					}
				}
			}

			befLine = curLine;
			befItem = curItem;

			curLine = i;
			curItem = j;

			bool le = true;
			bool up = true;

			BaseGUIElement *bef = lines[befLine].items[befItem].q;
			BaseGUIElement *cur = lines[curLine].items[curItem].q;

			if( bef && cur )
			{
				Rect br; bef->GetRect(br);
				Rect cr; cur->GetRect(cr);

				if( cr.l > br.l )
					le = false;

				if( cr.t > br.t )
					up = false;
			}

			if(  up && curLine > befLine )
				ScrollTo(
					GetOffsetX(),
					GetOffsetY() - GetFullH());
			else
			if( !up && curLine < befLine )
				ScrollTo(
					GetOffsetX(),
					GetOffsetY() + GetFullH());
			else
			if(  le && curItem > befItem )
				ScrollTo(
					GetOffsetX() - GetFullW(),
					GetOffsetY());
			else
			if( !le && curItem < befItem )
				ScrollTo(
					GetOffsetX() + GetFullW(),
					GetOffsetY());

			UpdatePos(le,up);
		}
	}
	else
	if( event == ChildDblClicked )
	{
		if( m_state != Normal )
			return;

		int i = p->data_a;
		int j = p->data_b;

		if( i >= 0 && i < lines && j >= 0 && j < lines[i].items )
		{
			BaseGUICursor *cursor = (BaseGUICursor *)m_cursor.Ptr();

			if( cursor )
			{
				float x,y; cursor->GetPosition(x,y);

				x *= 100.0f;
				y *= 100.0f;

				const Item &cur = lines[i].items[j];

				for( int m = 0 ; m < lines ; m++ )
				{
					for( int n = 0 ; n < lines[m].items ; n++ )
					{
						const Item &item = lines[m].items[n];

						if( item.draw > cur.draw )
						{
							if( item.q )
							{
								Rect r; item.q->GetRect(r);

								if( PointInRect(x,y,r))
									return;
							}
						}
					}
				}
			}

			Item &item = lines[i].items[j];

			if( item.q && item.q->Is(InterfaceUtils::GetButtonId()))
				;
			else
			{
				if( item.open )
				{
					LogicDebug("Item [%d,%d]: select trigger",i,j);
					item.onSel.Activate(Mission(),false);
				}
				else
				{
					LogicDebug("Item [%d,%d]: try trigger",i,j);
					item.onTry.Activate(Mission(),false);
				}
			}
		}
	}
}

void GUISelector::MoveFocus  (int i, int j, bool trigger)
{
	bool up = i < curLine;
	bool le = j < curItem;

	befLine = curLine;
			  curLine = i;

	befItem = curItem;
			  curItem = j;

	maxItem = curItem;

	///////////////////////

	if( m_state != Normal )
	{
		ScrollTo(m_to.x,m_to.y); m_state = Normal;
	}

	UpdateElements(0.0f,true);

	///////////////////////

	UpdatePos(le,up,true,trigger);
}

void GUISelector::OpenElement(int i, int j, bool open)
{
	Item &item = lines[i].items[j];

	if( item.open == open )
		return;

	item.open = open;

	BaseGUIElement *show;
	BaseGUIElement *hide;

	if( open )
	{
		show = (BaseGUIElement *)item.normal.p.Ptr();
		hide = (BaseGUIElement *)item.closed.p.Ptr();
	}
	else
	{
		show = (BaseGUIElement *)item.closed.p.Ptr();
		hide = (BaseGUIElement *)item.normal.p.Ptr();
	}

	if( item.closed.p.Ptr() == null ) // не задан виджет для закрытого состояния
	{
		if( item.q && item.q->Is(InterfaceUtils::GetWidgetId()))
		{
			((GUIWidget *)item.q)->SetBlack(open == false);
		}

		return;
	}

	if( hide )
	{
		if( show )
		{
			show->Show(true);

			float x,y; hide->GetNativePos(x,y);

			show->MoveTo(x,y);
			show->SizeTo(
				((BaseGUIElement *)item.normal.p.Ptr())->GetWidth(),
				((BaseGUIElement *)item.normal.p.Ptr())->GetHeight());

			show->SetAlpha(hide->GetNativeAlpha());
		}

		hide->Show(false);
	}

	item.q = show;

	item.k = 1.0f;
	item.a = 0.6f;

	item.draw -= 1000;
}

void GUISelector::MoveUp()
{
/*	if( m_state != Normal )
	{
		if( curLine >= 0 && curItem >= 0 )
		{
			BaseGUIElement *p = lines[curLine].items[curItem].p;

			if( p->Is(InterfaceUtils::GetButtonId()))
				p->SetFocus();
		}
		return;
	}*/

	if( curLine >= 0 && lines > 1 )
	{
		if( curLine )
		{
			if( lines[curLine - 1].items < 1 )
				return;

			befItem = curItem;

			befLine = curLine;
					  curLine--;

			curItem = maxItem;

			if( curItem > lines[curLine].items - 1 )
				curItem = lines[curLine].items - 1;

			UpdatePos(true,true);
		}
		else if( m_isLooped )
		{
			int last = lines.Size() - 1;

			if( lines[last].items < 1 )
				return;

			befItem = curItem;

			befLine = curLine;
					  curLine = last;

			curItem = maxItem;

			if( curItem > lines[curLine].items - 1 )
				curItem = lines[curLine].items - 1;

			ScrollTo(
				GetOffsetX(),
				GetOffsetY() - GetFullH());

			UpdatePos(true,true);
		}
		else if( curItem >= 0 )
		{
			BaseGUIElement *q = lines[0].items[curItem].q;

			if( q->Is(InterfaceUtils::GetButtonId()))
				q->SetFocus();
		}
	}
}

void GUISelector::MoveDown()
{
/*	if( m_state != Normal )
	{
		if( curLine >= 0 && curItem >= 0 )
		{
			BaseGUIElement *p = lines[curLine].items[curItem].p;

			if( p->Is(InterfaceUtils::GetButtonId()))
				p->SetFocus();
		}
		return;
	}*/

	if( curLine >= 0 && lines > 1 )
	{
		if( curLine < lines - 1 )
		{
			if( lines[curLine + 1].items < 1 )
				return;

			befItem = curItem;

			befLine = curLine;
					  curLine++;

			curItem = maxItem;

			if( curItem > lines[curLine].items - 1 )
				curItem = lines[curLine].items - 1;

			UpdatePos(true,false);
		}
		else if( m_isLooped )
		{
			if( lines[0].items < 1 )
				return;

			befItem = curItem;

			befLine = curLine;
					  curLine = 0;

			curItem = maxItem;

			if( curItem > lines[curLine].items - 1 )
				curItem = lines[curLine].items - 1;

			ScrollTo(
				GetOffsetX(),
				GetOffsetY() + GetFullH());

			UpdatePos(true,false);
		}
		else if( curItem >= 0 )
		{
			BaseGUIElement *q = lines.LastE().items[curItem].q;

			if( q->Is(InterfaceUtils::GetButtonId()))
				q->SetFocus();
		}
	}
}

void GUISelector::MoveLeft()
{
/*	if( m_state != Normal )
	{
		if( curLine >= 0 && curItem >= 0 )
		{
			BaseGUIElement *p = lines[curLine].items[curItem].p;

			if( p->Is(InterfaceUtils::GetButtonId()))
				p->SetFocus();
		}
		return;
	}*/

	if( curLine >= 0 )
	{
		if( curItem >= 0 )
			maxItem = curItem;

		if( curItem >= 0 && lines[curLine].items > 1 )
		{
			if( curItem )
			{
				befLine = curLine;

				befItem = curItem;
						  curItem--;

				maxItem = curItem;

				UpdatePos(true,true);
			}
			else
			if( m_isLooped )
			{
				befLine = curLine;

				befItem = curItem;
						  curItem = lines[curLine].items - 1;

				maxItem = curItem;

				ScrollTo(
					GetOffsetX() - GetFullW(),
					GetOffsetY());

				UpdatePos(true,true);
			}
			else
			{
				BaseGUIElement *q = lines[curLine].items[curItem].q;

				if( q && q->Is(InterfaceUtils::GetButtonId()))
					q->SetFocus();
			}
		}
	}
}

void GUISelector::MoveRight()
{
/*	if( m_state != Normal )
	{
		if( curLine >= 0 && curItem >= 0 )
		{
			BaseGUIElement *p = lines[curLine].items[curItem].p;

			if( p->Is(InterfaceUtils::GetButtonId()))
				p->SetFocus();
		}
		return;
	}*/

	if( curLine >= 0 )
	{
		if( curItem >= 0 )
			maxItem = curItem;

		if( curItem >= 0 && lines[curLine].items > 1 )
		{
			if( curItem < lines[curLine].items - 1 )
			{
				befLine = curLine;

				befItem = curItem;
						  curItem++;

				maxItem = curItem;

				UpdatePos(false,true);
			}
			else
			if( m_isLooped )
			{
				befLine = curLine;

				befItem = curItem;
						  curItem = 0;

				maxItem = curItem;

				ScrollTo(
					GetOffsetX() + GetFullW(),
					GetOffsetY());

				UpdatePos(false,true);
			}
			else
			{
				BaseGUIElement *q = lines[curLine].items[curItem].q;

				if( q && q->Is(InterfaceUtils::GetButtonId()))
					q->SetFocus();
			}
		}
	}
}

void GUISelector::Command(const char *id, dword numParams, const char **params)
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
	if( string::IsEqual(id,"fadeout"))
	{
		if( !IsShow())
			return;
	
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
		MoveUp();
	}
	else
	if( string::IsEqual(id,"down"))
	{
		MoveDown();
	}
	else
	if( string::IsEqual(id,"left"))
	{
		MoveLeft();
	}
	else
	if( string::IsEqual(id,"right"))
	{
		MoveRight();
	}
	else
	if( string::IsEqual(id,"focus"))
	{
		if( numParams < 2 )
		{
			LogicDebugError("Command <focus> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <focus> error. Index not specified.");
			return;
		}

		long index = atol(params[0]);

		//////////////////////////////////////////////
		// Поддерживаем пока только одномерный лист //
		//////////////////////////////////////////////

		if( !lines )
		{
			LogicDebugError("Command <focus> error. List is empty.");
			return;
		}

		int n = lines[0].items;

		bool hor = n > 1;

		if( !hor )
			n = lines;

		if( index < n )
		{
			bool newState = false;

			if( params[1] )
			{
				switch( params[1][0] )
				{
					case '1':
					case 't':
					case 'T':
						newState = true;
						break;
				}
			}

			if( hor )
				MoveFocus(0,index,newState);
			else
				MoveFocus(index,0,newState);

			LogicDebug(
				"Command <focus>. Element %i is now focused and %striggered.",index,newState ? "" : "not ");
		}
		else
		{
			LogicDebugError(
				"Command <focus> error. Index value %i not in range [0, %i].",
				index,n);
			return;
		}
	}
	else
	if( string::IsEqual(id,"open"))
	{
		if( numParams < 2 )
		{
			LogicDebugError("Command <open> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <open> error. Index not specified.");
			return;
		}

		long index = atol(params[0]);

		//////////////////////////////////////////////
		// Поддерживаем пока только одномерный лист //
		//////////////////////////////////////////////

		if( !lines )
		{
			LogicDebugError("Command <open> error. List is empty.");
			return;
		}

		int n = lines[0].items;

		bool hor = n > 1;

		if( !hor )
			n = lines;

		if( index < n )
		{
			bool newState = false;

			if( params[1] )
			{
				switch( params[1][0] )
				{
					case '1':
					case 't':
					case 'T':
						newState = true;
						break;
				}
			}

			if( hor )
				OpenElement(0,index,newState);
			else
				OpenElement(index,0,newState);

			LogicDebug(
				"Command <open>. Element %i is now %s.",index,newState ? "opend" : "closed");
		}
		else
		{
			LogicDebugError(
				"Command <open> error. Index value %i not in range [0, %i].",
				index,n);
			return;
		}
	}
	else
	{
		BaseGUIElement::Command(id,numParams,params);
	}
}

static char GUIDescription[] =

"GUI Selector.\n\n"
"    Use it to make list of GUI items\n"
"    NOTE: all sizes-positions are relative (not pixel sizes-positions)\n\n"
"Commands:\n\n"
"    left, right - scrolls list horizontally\n"
"    up, down - scrolls list vertically\n\n"
"    focus [index] [1/0] - set focus to element and trigger or not (1/0)\n"
"    open [index] [1/0] - open/close element\n\n"
"    fadein  [time] - fades list in\n"
"    fadeout [time] - fades list out";

MOP_BEGINLISTCG(GUISelector, "GUI Selector", '1.00', 2000, GUIDescription, "Interface")

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

//	MOP_STRING("Hor bar", "")
//	MOP_STRING("Ver bar", "")

	MOP_ARRAYBEG("Lines", 0, 100)

		MOP_ARRAYBEG("Items", 0, 100)

			MOP_STRING("Normal", "")
			MOP_STRING("Closed", "")

			MOP_BOOL("Open", true)

			MOP_MISSIONTRIGGER("OnFocus" )
			MOP_MISSIONTRIGGER("OnLeave" )

			MOP_MISSIONTRIGGER("OnSelect")
			MOP_MISSIONTRIGGER("OnTry")

		MOP_ARRAYEND

	MOP_ARRAYEND

	MOP_LONG("Start line", -1)
	MOP_LONG("Start item", -1)

	MOP_STRING("Marker", "")
	MOP_FLOAT ("Marker scale", 1.0f)

	MOP_FLOAT("Item width" , 25.0f)
	MOP_FLOAT("Item height",  5.0f)

	MOP_FLOAT("Hor space", 5.0f)
	MOP_FLOAT("Ver space", 5.0f)

	MOP_ARRAYBEG("Scales", 0, 10)

		MOP_FLOAT("Scale", 1.0f)
		MOP_FLOAT("Alpha", 1.0f)

		MOP_LONG("Draw", 0)

	MOP_ARRAYEND

	MOP_GROUPBEG("Values")

		MOP_FLOATEX( "Scale speed", 1.0f, 1.0f, 10.0f)
		MOP_FLOATEX("Scroll speed", 1.0f, 1.0f, 10.0f)

		MOP_FLOATEXC("Delay time", 0.65f, 0.4f, 1.0f, "Задержка перед автоповтором")

		MOP_FLOATEXC("Beg time", 0.35f, 0.15f, 0.35f, "Начальное время перехода между элементами")
		MOP_FLOATEXC("End time", 0.15f, 0.15f, 0.35f, "Конечное время перехода между элементами")

		MOP_FLOATEXC("Accel time", 4.0f, 1.0f, 10.0f, "Время ускорения")

	MOP_GROUPEND()

	MOP_BOOL("Centered", false)
	MOP_BOOL("Looped", false)

	MOP_ENUMBEG("Align")

		MOP_ENUMELEMENT("Center")
		MOP_ENUMELEMENT("Bottom")

	MOP_ENUMEND

	MOP_ENUM("Align", "Align")

	MOP_GROUPBEG("Controls")

		MOP_STRING("Up"	  , "")
		MOP_STRING("Down" , "")

		MOP_STRING("Left" , "")
		MOP_STRING("Right", "")

		MOP_STRING("Sel"  , "")

	MOP_GROUPEND()

//	MOP_FLOAT("Button width" , 4.1f)
//	MOP_FLOAT("Button height", 5.0f)

	MOP_BOOL("Active" , true)
	MOP_BOOL("Visible", true)

	MOP_BOOL("Silent activate", false)

MOP_ENDLIST(GUISelector)
