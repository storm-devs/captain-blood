#include "..\..\..\common_h/mission/Mission.h"
#include "csProgressBar.h"

#include "..\Utils\InterfaceUtils.h"

static const char TOP_LEFT	  [] = "Top-left";
static const char TOP_RIGHT	  [] = "Top-right";
static const char BOTTOM_LEFT [] = "Bottom-left";
static const char BOTTOM_RIGHT[] = "Bottom-right";
/*
const float MaxAddDecEffectTime = 1.0f;
const float MaxShowEffectTime	= 0.8f;
*/
csProgressBar::csProgressBar()
{
	m_restart = false;

	m_fValue = 0.0f;
	m_fPrevValue = 0.0f;
	m_fPercent	 = 0.5f;

	m_pHPObject.Reset();

	MaxAddDecEffectTime = 1.0f;
	MaxShowEffectTime	= 0.8f;

	m_fValue = 0.0f;
	m_showAddDecEffect = false;
	m_showEffectTime = 0.0f;

	pRS = (IRender*)api->GetService("DX9Render");
	Assert(pRS)

	barTexture = pRS->GetTechniqueGlobalVariable("interfaceTexture",_FL_);

	texture = NULL;

	drawLevel = ML_GUI2;
}

csProgressBar::~csProgressBar()
{	
	if( texture )
	{
		texture->Release();
		texture = null;
	}

	barTexture = null;
}

void csProgressBar::Restart()
{
	m_restart = true;

	ReCreate();
}

// Вызывается в режиме игры после создания всех объектов
void csProgressBar::PostCreate()
{
	if( !m_pHPObject.Ptr() && !m_params.HPObject.IsEmpty())
	{
		FindObject(m_params.HPObject,m_pHPObject);

		if( m_pHPObject.Ptr())
		{
			m_fPrevValue = m_fValue = m_pHPObject.Ptr()->GetHP();
		}
	}

	m_pHPObject.Validate();
}

//Создание объекта
bool csProgressBar::Create		   (MOPReader & reader)
{
	if( !m_restart )
	{
		Render().GetShaderId("interfaceQuad",GUI_Quad_id);
	}

	EditMode_Update(reader);

	return true;
}

//Обновление параметров
bool csProgressBar::EditMode_Update(MOPReader & reader)
{
////////////////////////////////////////////////
//	BaseGUIElement::EditMode_Update(reader);  //
////////////////////////////////////////////////

	if( !m_restart )
	{
	//	m_aspect = InterfaceUtils::AspectRatio(Render());
		InitAspect();
	}

	Layout layout;

	switch( reader.Enum().c_str()[0] )
	{
		case 'C':
			layout = OnCenter;
			break;

		case 'L':
			layout = OnLeft;
			break;

		case 'R':
			layout = OnRight;
			break;
	};

	if( !Restricted(preserveLayout))
		m_layout = layout;

	m_natLay = layout;

	back_t = reader.String().c_str();
	fron_t = reader.String().c_str();

	if( !m_restart )
	{
		if( texture )
			texture->Release();

	//	texture = (ITexture *)pRS->CreateTexture(_FL_,fron_t);
		texture = (ITexture *)pRS->CreateTextureFullQuality(_FL_,fron_t);

		 m_backRender.SetTexture(back_t);
		m_frontRender.SetTexture(fron_t);
	}

//	m_params.fX = reader.Float();
//	m_params.fY = reader.Float();
	float x = reader.Float();
	float y = reader.Float();

	if( !Restricted(preservePos))
		MoveTo(x,y);

	m_natPos.x = x;
	m_natPos.y = y;

//	m_params.fWidth	 = reader.Float();
//	m_params.fHeight = reader.Float();
	float w = reader.Float();
	float h = reader.Float();

	if( !Restricted(preserveSize))
		SizeTo(w,h);

	m_natSize.w = w;
	m_natSize.h = h;

	m_fValue = m_params.fMaxValue = (float)reader.Long();

	m_fPrevValue = 0.0f;

	m_params.HPObject = reader.String();

	fMinBorder = reader.Float() * 0.01f;
	fMaxBorder = reader.Float() * 0.01f;

	m_params.fStartBar = reader.Float() * 0.01f;
	m_params.fEndBar   = reader.Float() * 0.01f;

	m_params.drawPriority = reader.Long();

	m_res = reader.Enum().c_str();

	m_params.doShowEffect = reader.Bool();

//	Show(reader.Bool());
	nativeShow = reader.Bool();

	MaxAddDecEffectTime = reader.Float();
	MaxShowEffectTime	= reader.Float();

	circular = reader.Bool();
	reverse  = reader.Bool();
	vertical = reader.Bool();

	inverse = reader.Bool();

//	MoveTo(m_params.fX	  ,m_params.fY);
//	SizeTo(m_params.fWidth,m_params.fHeight);

	SetAlpha (reader.Float());
	SetParent(reader.String());

//	SetUpdate(&csProgressBar::InitFunc,ML_FIRST);
	SetUpdate(&csProgressBar::InitFunc,ML_FIRST + 2);

	return true;
}

void csProgressBar::OnParentNotify(Notification event)
{
	BaseGUIElement::OnParentNotify(event);

	if( event == ParentChanged )
		Update();
}

// извращенный расчет ректа
void csProgressBar::GetRect(Rect &rect, bool useOffset)// const
{
	float x = m_X;
	float y = m_Y;

	float w = GetWidth ()*m_aspect;
	float h = GetHeight();

	if( w >= 0.0f )
	{
		rect.l = x;
		rect.t = y;

		rect.r = x + w;
		rect.b = y + h;
	}
	else
	{
		rect.l = x + w;
		rect.t = y;

		rect.r = x;
		rect.b = y + h;
	}
}

void _cdecl csProgressBar::InitFunc(float, long)
{
	DelUpdate(&csProgressBar::InitFunc);

	if( m_restart )
	{
		m_restart = false;
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
	}

	Update();

	Show(nativeShow);
}
/*
void csProgressBar::Update()
{
	FindParent();

	Rect r; GetParentRect(r);
	
	parent_bottom = r.b;

	float x,y; GetNativePos (x,y);
	float w,h; GetNativeSize(w,h);

	x *= m_aspect;

	if( w < 0.0f )
		w = -w;

	if( string::IsEqual(m_res,TOP_LEFT))
	{
		m_X = x + r.l;
		m_Y = y + r.t;

		m_bottom = false;
	}

	if( string::IsEqual(m_res,BOTTOM_LEFT))
	{
		m_X = x + r.l;
		m_Y = r.b - h - y;

		m_bottom = true;
	}

	if( string::IsEqual(m_res,TOP_RIGHT))
	{
		m_X = r.r - x;
		m_Y = y + r.t;

		w = -w;

		m_bottom = false;
	}

	if( string::IsEqual(m_res,BOTTOM_RIGHT))
	{
		m_X = r.r  - x;
		m_Y = r.b - h - y;

		w = -w;

		m_bottom = true;
	}

	SizeTo(w,h);
}
*/
void csProgressBar::Update()
{
	FindParent();

	Rect r; GetParentRect(r);
	
	parent_bottom = r.b;

	float x,y; GetNativePos (x,y);
	float w,h; GetNativeSize(w,h);

	x *= m_aspect;

	if( w < 0.0f )
		w = -w;

	if( string::IsEqual(m_res,"Top"))
	{
		switch( m_layout )
		{
			case OnLeft:
				m_X = x + r.l;
				m_Y = y + r.t;
				break;

			case OnCenter:
				m_X = (r.l + r.r)*0.5f + x - w*m_aspect*0.5f;
				m_Y = y + r.t;
				break;

			case OnRight:
				m_X = r.r - x;
				m_Y = y + r.t; w = -w;
				break;
		}

		m_bottom = false;
	}
	else
	{
		switch( m_layout )
		{
			case OnLeft:
				m_X = x + r.l;
				m_Y = r.b - h - y;
				break;

			case OnCenter:
				m_X = (r.l + r.r)*0.5f + x - w*m_aspect*0.5f;
				m_Y = r.b - h - y;
				break;

			case OnRight:
				m_X = r.r  - x;
				m_Y = r.b - h - y; w = -w;
		}

		m_bottom = true;
	}

	SizeTo(w,h);
}

static const float R1 =  PI/4;
static const float R2 = -R1;
static const float R3 = -R1*3;
static const float R4 = -R1*5;

void csProgressBar::Calculate(float x, float y, float w, float h, Vertex &v, float ang, float a, float wk)
{
	float s = sin(ang);
	float c = cos(ang);

	float xk = c/fabsf(s);
	float yk = s/fabsf(c);

	if( ang > R1 || ang <= R4 )
	{
		v.x = x + w*xk;
		v.y = y - h;
		v.a = a;
		v.u = 0.5f + 0.5f*xk*wk;
		v.v = 0.0f;
	}
	else
	if( ang > R2 )
	{
		v.x = x + w;
		v.y = y - h*yk;
		v.a = a;
		v.u = wk < 0.0f ? 0.0f : 1.0f;
		v.v = 0.5f - 0.5f*yk;
	}
	else
	if( ang > R3 )
	{
		v.x = x + w*xk;
		v.y = y + h;
		v.a = a;
		v.u = 0.5f + 0.5f*xk*wk;
		v.v = 1.0f;
	}
	else
	if( ang > R4 )
	{
		v.x = x - w;
		v.y = y - h*yk;
		v.a = a;
		v.u = wk < 0.0f ? 1.0f : 0.0f;
		v.v = 0.5f - 0.5f*yk;
	}
}

void csProgressBar::DrawBar(float wk, float x, float y, float w, float h, float beg, float end, float alpha, float xs, float ys)
{
	if( reverse )
	{
		beg = 1.0f - beg;
		end = 1.0f - end;
	}

	if( end <= beg )
	//	return;
	{
		float t = end;

		end = beg;
		beg = t;
	}

	x -= (w*xs - w)*0.5f;
	y -= (h*ys - h)*0.5f;

	w *= xs;
	h *= ys;

	w *= 0.5*0.01f;
	h *= 0.5*0.01f;

	x *= 0.01f;
	y *= 0.01f;

	x += w;
	y += h;

	beg = PI*0.5f - 2*PI*beg;
	end = PI*0.5f - 2*PI*end;

	Vertex b[7]; long n = 0;

	b[n].x = x;
	b[n].y = y;
	b[n].a = alpha;
	b[n].u = 0.5f;
	b[n].v = 0.5f;

	n++;

	float pos = PI/4; float step = PI/2;

	while( beg < pos )
	{
	//	Calculate(x,y,w,h,b[n++],pos,alpha);
		pos -= step;
	}

	Calculate(x,y,w,h,b[n++],beg,alpha,wk);

	while( end < pos )
	{
		Calculate(x,y,w,h,b[n++],pos,alpha,wk);
		pos -= step;
	}

	Calculate(x,y,w,h,b[n++],end,alpha,wk);

	if( inverse )
	{
		for( int i = 0 ; i < n ; i++ )
		{
			b[i].x = b[i].x*2.0f - 1.0f;
			b[i].y = 1.0f - (y - (b[i].y - y))*2.0f;
			b[i].z = 1.0f;

			b[i].v = 1.0f - b[i].v;
		}
	}
	else
	{
		for( int i = 0 ; i < n ; i++ )
		{
			b[i].x = b[i].x*2.0f - 1.0f;
			b[i].y = 1.0f - b[i].y*2.0f;
			b[i].z = 1.0f;
		}
	}

	if( barTexture && texture )
		barTexture->SetTexture(texture);

	pRS->DrawPrimitiveUP(GUI_Quad_id, PT_TRIANGLEFAN,n - 2,b,6*sizeof(float));
}

void csProgressBar::Draw(float wScale, float hScale, float fMaximalVal, float globalAlpha)
{
	bool edit_mode = EditMode_IsOn();

	float y = m_Y;

	float w = GetWidth ();
	float h = GetHeight();

	if( !Restricted(preserveSize))
		w *= m_aspect;

	if( m_bottom )
		y = parent_bottom - h - GetNativeY();

	float wk;
	float xx;
	
	if( circular && reverse )
	{
		wk = -1.0f;
		xx =  GetWidth();
	}
	else
	{
		wk =  1.0f;
		xx =  0.0f;
	}

	if( edit_mode || back_t[0] )
	{
		float al = 1.0f;

		if( edit_mode && !back_t[0] )
			al = 0.5f;

		m_backRender.DrawQuad(
			m_X + xx,y,w*wk,h,0,0,1.0,1.0,10.0f + 1.0f*globalAlpha*al,wScale,hScale,vertical,true);
	}

	if( !edit_mode && !fron_t[0] )
		return;

	float al = 1.0f;

	if( edit_mode && !fron_t[0] )
		al = 0.5f;

	// рисуем спецэффект добавления/убавления жизней
	if( m_fPrevValue != m_fValue && !edit_mode )
	{
		m_showAddDecEffect = true;
		m_addDecEffectTime = MaxAddDecEffectTime;

		if( m_fPrevValue < m_fValue) m_effects |= AddHP;
		if( m_fPrevValue > m_fValue) m_effects |= DecHP;
				
		m_addDecEffectStartU = ScaleToBorder(m_fValue/fMaximalVal)*(m_params.fEndBar - m_params.fStartBar);
		m_addDecEffectEndU   = ScaleToBorder(m_fPrevValue/fMaximalVal)*(m_params.fEndBar - m_params.fStartBar);
	}

	float off = 0.0f;

	if( m_showAddDecEffect && m_effects&AddHP )
	{
		float k = m_addDecEffectTime/MaxAddDecEffectTime;

		off = k*(m_addDecEffectEndU - m_addDecEffectStartU);
	}

	if( circular )
	{
		DrawBar(wk,m_X,y,w,h,
			m_params.fStartBar,
		//	m_params.fStartBar + m_fPercent*(m_params.fEndBar - m_params.fStartBar),
			m_params.fStartBar + m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,
			10.0f + 1.0f*globalAlpha*al,wScale,hScale);
	}
	else
	{
		if( vertical )
		{
			if( reverse )
			{
				m_frontRender.DrawQuad(
				//	m_X,y + m_params.fStartBar*h,
					m_X,y + h - (1.0f - m_params.fEndBar)*h - (m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off)*h,
					w,(m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off)*h,

				//	m_params.fStartBar,0,
				//	m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,1.0f,
				//	0,m_params.fStartBar,
					0,1.0f - (1.0f - m_params.fEndBar + m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off),
					1.0f,m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,

					10.0f + 1.0f*globalAlpha*al,wScale,hScale,vertical,true);
			}
			else
			{
				m_frontRender.DrawQuad(
					m_X,y + m_params.fStartBar*h,
					w,(m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off)*h,

				//	m_params.fStartBar,0,
				//	m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,1.0f,
					0,m_params.fStartBar,
					1.0f,m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,

					10.0f + 1.0f*globalAlpha*al,wScale,hScale,vertical,true);
			}
		}
		else
		{
			if( reverse )
			{
				m_frontRender.DrawQuad(
				//	m_X + m_params.fStartBar*w,y,
					m_X + w - (1.0f - m_params.fEndBar)*w - (m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off)*w,y,
					  (m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off)*w,h,
				//	m_params.fStartBar,0,
					1.0f - (1.0f - m_params.fEndBar + m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off),0,
					m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,1.0f,
					10.0f + 1.0f*globalAlpha*al,wScale,hScale,vertical,true);
			}
			else
			{
				m_frontRender.DrawQuad(
					m_X + m_params.fStartBar*w,y,
					  (m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off)*w,h,
					m_params.fStartBar,0,
					m_fPercent*(m_params.fEndBar - m_params.fStartBar) + off,1.0f,
					10.0f + 1.0f*globalAlpha*al,wScale,hScale,vertical,true);
			}
		}
	}

	if( m_showAddDecEffect )
	{
		float Bright = 10.0f;

	//	if( m_effects&AddHP )
		if( m_effects&AddHP && circular )
		{
		//	Bright *= 2.0f;
		}

		if( m_addDecEffectTime <= 0.0f )
		{
			m_addDecEffectTime = 0.0f;
			m_showAddDecEffect = false;

			m_effects &= ~AddHP;
			m_effects &= ~DecHP;
		}

		if( circular )
		{
			float k = m_addDecEffectTime/MaxAddDecEffectTime;		

			if( m_effects&AddHP ) k = 1.0f - k;

			DrawBar(wk,m_X,y,w,h,
					m_params.fStartBar + m_addDecEffectStartU,
					m_params.fStartBar + m_addDecEffectStartU + m_addDecEffectTime*(m_addDecEffectEndU - m_addDecEffectStartU),
					Bright + globalAlpha*k*al,wScale,hScale);
		}
		else
		{
			float k = m_addDecEffectTime/MaxAddDecEffectTime;

			float delta = k*(m_addDecEffectEndU - m_addDecEffectStartU);

			if( m_effects&AddHP )
				k = 1.0f - k;

			if( vertical )
			{
				if( reverse )
				{
					m_frontRender.DrawQuad(
					//	m_X,y + (m_params.fStartBar + m_addDecEffectStartU)*h + (h*hScale - h)*0.5f*0.5f,
						m_X,y + h - (1.0f - m_params.fEndBar + m_addDecEffectStartU)*h - (h*hScale - h)*0.5f*0.5f - delta*h,
						w,delta*h,

					//	m_params.fStartBar + m_addDecEffectStartU,0,
					//	  delta,1.0f,
					//	0,m_params.fStartBar + m_addDecEffectStartU,
						0,1.0f - (1.0f - m_params.fEndBar + m_addDecEffectStartU + delta),
						  1.0f,delta,

						Bright + globalAlpha*k*al,wScale,hScale,vertical,true);
				}
				else
				{
					m_frontRender.DrawQuad(
						m_X,y + (m_params.fStartBar + m_addDecEffectStartU)*h + (h*hScale - h)*0.5f*0.5f,
						w,delta*h,

					//	m_params.fStartBar + m_addDecEffectStartU,0,
					//	  delta,1.0f,
						0,m_params.fStartBar + m_addDecEffectStartU,
						  1.0f,delta,

						Bright + globalAlpha*k*al,wScale,hScale,vertical,true);
				}
			}
			else
			{
				if( reverse )
				{
					m_frontRender.DrawQuad(
						m_X + w - (1.0f - m_params.fEndBar + m_addDecEffectStartU)*w - (w*wScale - w)*0.5f*0.5f - delta*w,y,
						delta*w,h,
					//	m_params.fStartBar + m_addDecEffectStartU,0,
						1.0f - (1.0f - m_params.fEndBar + m_addDecEffectStartU + delta),0,
						delta,1.0f,
						Bright + globalAlpha*k*al,wScale,hScale,vertical,true);
				}
				else
				{
					m_frontRender.DrawQuad(
						m_X + (m_params.fStartBar + m_addDecEffectStartU)*w + (w*wScale - w)*0.5f*0.5f,y,
						delta*w,h,
						m_params.fStartBar + m_addDecEffectStartU,0,
						delta,1.0f,
						Bright + globalAlpha*k*al,wScale,hScale,vertical,true);
				}
			}
		}
	}
}

void _cdecl csProgressBar::Work(float dltTime, long level)
{
	if( EditMode_IsOn() && !EditMode_IsVisible())
		return;

	Draw();
	Update(dltTime);
}

void csProgressBar::Update(float dltTime)
{
	if(!EditMode_IsOn())
	{
		if( !m_pHPObject.Validate())
		{
			FindObject(m_params.HPObject,m_pHPObject);

			if( m_pHPObject.Ptr())
			{
				m_fPrevValue = m_fValue = m_pHPObject.Ptr()->GetHP();
			}
		}
	}

	if( m_showAddDecEffect )
		m_addDecEffectTime -= dltTime;

	if( m_effects&ShowBar )
	{
		m_showEffectTime -= dltTime;

		if( m_showEffectTime <= 0.0f )
		{
			m_showEffectTime = 0.0f;

			m_effects &= ~ShowBar;
		}
	}

	m_fPrevValue = m_fValue;
}

void csProgressBar::Draw()
{
	if( InterfaceUtils::IsHide())
		return;

	float fMaximalVal = m_params.fMaxValue;

	if( m_pHPObject.Ptr())
	{		
		float maxHP = m_pHPObject.Ptr()->GetMaxHP();

		if( maxHP > 0.0f )
		{
			fMaximalVal = maxHP;
			m_fValue	= m_pHPObject.Ptr()->GetHP();
		}
	}

	m_fPercent = ScaleToBorder(m_fValue/fMaximalVal);	
	
	// FIX ME!!!!
	if( m_fPercent > 1.0f )
		m_fPercent = 1.0f;

//	Draw(1.0f,1.0f,fMaximalVal,1.0f);
	Draw(1.0f,1.0f,fMaximalVal,GetAlpha());

	if( m_effects&ShowBar )
	{
		if( m_showEffectTime > 0.0f )
		{
			float FinalScales[] = {0.1f,0.2f,0.4f};

			for( dword k = 0 ; k < sizeof(FinalScales)/sizeof(FinalScales[0]) ; k++ )
			{
				Draw(1.0f + FinalScales[k]*(1.0f - m_showEffectTime/MaxShowEffectTime),
					 1.0f + FinalScales[k]*(1.0f - m_showEffectTime/MaxShowEffectTime),
				//	 fMaximalVal,m_showEffectTime/MaxShowEffectTime);
					 fMaximalVal,m_showEffectTime/MaxShowEffectTime*GetAlpha());
			}
		}
	}
}

void csProgressBar::Show(bool isShow)
{
//	MissionObject ::Show(isShow);
	BaseGUIElement::Show(isShow);

	BaseGUIElement *parent = GetParent();

	long level = drawLevel;

/*	if( parent )
		level = parent->GetDrawLevel() + 100;*/

	DelUpdate(&csProgressBar::Work);

	if( IsShow())
	{
		if( GetAlpha() > 0.0f )
		{
			SetUpdate(&csProgressBar::Work,level + m_params.drawPriority);
		}

		if( m_params.doShowEffect && !EditMode_IsOn())
		{
			m_effects |= ShowBar;

			m_showEffectTime = MaxShowEffectTime;
		}
	}
}

void csProgressBar::SetMax(float val)
{
	if( m_params.fMaxValue == val )
		return;

	m_params.fMaxValue = val;
	//LogicDebug("Change progress bar max to %3.2f", m_params.fMaxValue);

	if( m_fValue > m_params.fMaxValue )
		m_fValue = m_params.fMaxValue;

	m_fPrevValue = m_fValue;
}

void csProgressBar::SetPos(float val)
{
	if( m_fValue == val )
		return;

	m_fValue = val;

	if( m_fValue < 0.0f )
		m_fValue = 0.0f;

	if( m_fValue > m_params.fMaxValue )
		m_fValue = m_params.fMaxValue;
}

//Обработчик команд для объекта
void csProgressBar::Command(const char *id, dword numParams, const char **params)
{
	m_fPrevValue = m_fValue;

	if( string::IsEmpty(id))
		return;
	
	if( string::IsEqual(id,"Set"))
	{
		if( numParams > 0 )
		{
			float fVal = (float)atoi(params[0]);

			SetPos(fVal);
		}

		return;
	}

	if( string::IsEqual(id,"Inc"))
	{
		if( numParams > 0 )
		{
			float fInc = (float)atoi(params[0]);

			m_fValue += fInc;
		}
		else
		{
			m_fValue += 1.0f;
		}

		if( m_fValue > m_params.fMaxValue )
			m_fValue = m_params.fMaxValue;

		return;
	}

	if( string::IsEqual(id,"Dec"))
	{
		if( numParams > 0 )
		{
			float fDec = (float)atoi(params[0]);

			m_fValue -= fDec;
		}
		else
		{
			m_fValue -= 1.0f;
		}

		if( m_fValue < 0.0f )
			m_fValue = 0.0f;

		return;
	}

	if( string::IsEqual(id,"Zero"))
	{
		LogicDebug("Zero progress bar");

		m_fValue = 0;

		return;
	}

	if( string::IsEqual(id,"Full"))
	{
		LogicDebug("Full progress bar");

		float fMaximalVal = m_params.fMaxValue;

		if( m_pHPObject.Ptr())
		{
			float maxHP = m_pHPObject.Ptr()->GetMaxHP();

			if( maxHP > 0.0f )
			{
				fMaximalVal = maxHP;
				m_fValue = m_pHPObject.Ptr()->GetHP();
			}
		}

		m_fValue = fMaximalVal;

		return;
	}

	if( string::IsEqual(id,"SetMax"))
	{
		if( numParams > 0 )
		{
			float fVal = (float)atoi(params[0]);
			
			SetMax(fVal);
		}

		return;
	}

	if( string::IsEqual(id,"SetHP"))
	{
		if( numParams > 0 )
		{
			m_fValue = (float)atoi(params[0]);
			//LogicDebug("Change progress bar HP to %3.2f", m_fValue);

			if( m_fValue > m_params.fMaxValue )
				m_fValue = m_params.fMaxValue;

			if( m_fValue < 0 )
				m_fValue = 0;
		}

		return;
	}

	if( string::IsEqual(id,"SetMO"))
	{
		if( numParams > 0 )
		{
			m_pHPObject.Reset();
			m_params.HPObjectStr = params[0];
			m_params.HPObject.Set(m_params.HPObjectStr.c_str());
		}

		return;
	}

	BaseGUIElement::Command(id,numParams,params);
}

float csProgressBar::ScaleToBorder(float hp)
{
	static float out_hp = 1.0f;
	
	if (hp<=fMinBorder)
	{
		out_hp = 0.0f;
	}
	else
	if (hp>=fMaxBorder)
	{
		out_hp = 1.0f;
	}
	else
	{
		out_hp = (hp - fMinBorder)/(fMaxBorder - fMinBorder);
	}

	return out_hp;
}

static const char *g_comment =

"Object for some events at some count\n"
" \n"
"Commands list:\n"
"----------------------------------------\n"
"  Increase progress value\n"
"----------------------------------------\n"
"    command: inc\n"
"    parm: number value (without param number = 1)\n"
" \n"
"----------------------------------------\n"
"  Decrease progress value\n"
"----------------------------------------\n"
"    command: dec\n"
"    parm: number value (without param number = 1)\n"
" \n"
"----------------------------------------\n"
"  Zero progress value\n"
"----------------------------------------\n"
"    command: zero\n"
" \n"
"----------------------------------------\n"
"  Full progress value\n"
"----------------------------------------\n"
"    command: full\n"
" \n"
"----------------------------------------\n"
"  Set progress maximum\n"
"----------------------------------------\n"
"    command: SetMax\n"
"    parm: number value\n"
" \n"
" ";

MOP_BEGINLISTCG(csProgressBar, "ProgressBar", '1.00', /*100*/50, g_comment, "Interface")

	MOP_ENUMBEG("Layout")

		MOP_ENUMELEMENT("Left")
		MOP_ENUMELEMENT("Center")
		MOP_ENUMELEMENT("Right")

	MOP_ENUMEND

	MOP_ENUM("Layout", "Layout")

	MOP_STRING("Background Texture", "")
	MOP_STRING("Progress Texture", "")

	MOP_FLOATEX("X", 0.0f, -50.0f, 100.0f);
	MOP_FLOATEX("Y", 0.0f, -50.0f, 100.0f);

	MOP_FLOATEX("Width" , 50.0f, 0.1f, 100.0f);
	MOP_FLOATEX("Height", 25.0f, 0.1f, 100.0f);

	MOP_LONGEX("Max value", 10000, 0, 10000000000);
	MOP_STRING("Show HP from", "")

	MOP_FLOATEX("Min HP Border", 0, 0, 100);
	MOP_FLOATEX("Max HP Border", 100, 0, 100);

	MOP_FLOATEX("Start Bar", 25.0f, 0.0f, 100.0f);
	MOP_FLOATEX("End Bar", 85.0f, 0.0f, 100.0f);

	MOP_LONG("Draw priority", 0)

	MOP_ENUMBEG("ControlAlign")

	/*	MOP_ENUMELEMENT(TOP_LEFT)
		MOP_ENUMELEMENT(BOTTOM_LEFT)
		MOP_ENUMELEMENT(TOP_RIGHT)
		MOP_ENUMELEMENT(BOTTOM_RIGHT)*/

		MOP_ENUMELEMENT("Top")
		MOP_ENUMELEMENT("Bottom")

	MOP_ENUMEND

	MOP_ENUM("ControlAlign", "Align")

	MOP_BOOL("Do show effect", true)
	MOP_BOOL("Show", true)

	MOP_FLOAT("AddDecEffectTime", 1.0f)
	MOP_FLOAT("ShowEffectTime"	, 0.8f)

	MOP_BOOL("Circular", false)
//	MOP_BOOLC("Reverse" , false, "инвертирует направление указателя в круговом режиме")
	MOP_BOOLC("Reverse" , false, "инвертирует направление индикатора")
	MOP_BOOL("Vertical", false)

	MOP_BOOLC("Inverse", false, "зеркалит по вертикали индикатор в круговом режиме")

	MOP_FLOAT ("Alpha", 1.0f)
	MOP_STRING("Parent id", "")

MOP_ENDLIST(csProgressBar)
