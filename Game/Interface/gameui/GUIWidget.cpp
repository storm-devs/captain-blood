#include "GUIWidget.h"
#include "..\utils\InterfaceUtils.h"

#include "BaseGUICursor.h"

//#include "..\..\GMXService\gmx_util.h"

char ALIGN_CENTER[] = "Center";
char ALIGN_LEFT	 [] = "Left";
char ALIGN_RIGHT [] = "Right";
char ALIGN_TOP	 [] = "Top";
char ALIGN_BOTTOM[] = "Bottom";

char PLAY_CONTINIOUS		[] = "Continious";
char PLAY_ONCE_ON_ACTIVATION[] = "Once on activation";
char PLAY_ONCE_ON_SHOW		[] = "Once on show";
char PLAY_ONCE_BY_COMMAND	[] = "Once by command";

#include "..\..\..\System\XRender\AdvFont\AdvFont.h"
/*
const int icon_max_count = 16;

static IVBuffer *pVB = NULL;
static IIBuffer *pIB = NULL;

static int refCount = 0;

static IVariable	*Texture;
static IBaseTexture *Image;

const dword buffer_size = 1024;
char GUIWidget::buffer[buffer_size*2];

float GUIWidget::font_hk = 0.0f;
*/
GUIWidget *GUIWidget::_curText = null;		// текущая активная подсказка

ICoreStorageFloat *GUIWidget::_sub = null;	// признак показа субтитров
long GUIWidget::_subUsers = 0;

GUIWidget::GUIWidget(void)/* : table(_FL_),
							   hash (_FL_)*/
{
	m_stat = true;

	m_statCur = 0;
	m_statEnd = 0;

	m_statTimeMax = 0.02f;

	m_cursor = null;

	m_model = null;
	m_font	= null;

	m_restart = false;

	m_alpha = 1.0f;

	m_lastTime = 0.0f;
	m_curTime = 0.0f;

	m_shouldPlay = false;

//	drawLevel = ML_GUI4;
	drawLevel = ML_GUI2;

	m_fadeTime = 0.5f;

	m_effTime = 0.0f;

	effectTime = 0.5f;
	effectSize = 2.0f;

	m_useAnim = true;

	m_time = 0.0f;
	m_animSpeed = 3.0f;

	m_cutsTime = 0.0f;

	effectColor = (dword)-1;

	m_hk = 1.0f;

	m_parsed = false;

	m_needUpdate = false;

//	lastImage = null;
	m_version = -1;

	m_asp = 4.0f/3.0f;

	m_fontScale = 1.0f;

/*	CreateBuffers();

	if( font_hk == 0.0f )
	{
		IFileService *storage = (IFileService *)api->GetService("FileService");
		Assert(storage)

		IIniFile *ini = storage->SystemIni();

		if( ini )
		{
			font_hk = ini->GetFloat("Controls","IconMult",1.0f);
		}
		else
			font_hk = 1.0f;
	}*/
	m_text = null;

	//////////////////////

	Assert(_subUsers >= 0)

	if( _subUsers++ < 1 )
	{
		Assert(_sub == null)

		_sub = api->Storage().GetItemFloat("Options.Subtitles",_FL_);
		Assert(_sub)
	}
}

GUIWidget::~GUIWidget(void)
{
	if( m_text && m_cutsUse )
	{
		if( _curText == this )
			_curText = null;
	}

	if( m_model )
	{
		m_model->Release();
		m_model = null;
	}

	if( m_font )
	{
		m_font->Release();
		m_font = null;
	}

//	if( Mission().ValidatePointer(GetParent(),m_validateHash))
//		GetParent()->UnRegister(this);

/*	if( !--refCount )
	{
		if( pIB )
			pIB->Release(); pIB = null;

		if( pVB )
			pVB->Release(); pVB = null;

		if( Texture )
			Texture->Release();
	}*/
	if( m_text )
		m_text->Release();

	/////////////////////

	Assert(_subUsers > 0)

	if( !--_subUsers )
	{
		Assert(_sub)

		RELEASE(_sub)
	}
}
/*
float GUIWidget::SC2UIRC(float t)
{
	return m_params.useSafeFrame ? InterfaceUtils::ScreenCoord2UIRectCoord(t) : t;
}

float GUIWidget::SS2UIRS(float size)
{
	return m_params.useSafeFrame ? InterfaceUtils::ScreenSize2UIRectSize(size) : size;
}
*/
inline float GUIWidget::frac(float f)
{
	return f - floor(f);
}

void GUIWidget::RenderBackGround(const Rect &_r/*, float deltaTime*/)
{
	if( !EditMode_IsOn() && !m_params.texture[0] )
		return;

	float tu = frac(m_curFrame*m_params.frameWidth);
	float count = 1.0f/m_params.frameWidth;

	Rect r = _r;

	if( m_effTime > 0.0f )
	{
		float k = m_effTime/effectTime;

		float w = 0.5f*(r.r - r.l);
		float h = 0.5f*(r.b - r.t);

		r.l -= w*k; r.r += w*k;
		r.t -= h*k; r.b += h*k;
	}

	float width  = r.r - r.l;
	float height = r.b - r.t;

	float tv = m_params.frameHeight*floor(m_curFrame/count);

	float al = 1.0f;

	if( EditMode_IsOn() && !m_params.texture[0] )
		al = 0.5f;

	tu += m_params.frameStartU;
	tv += m_params.frameStartV;

	m_quadRender.DrawQuad(
		r.l,r.t,width,height,
	//	tu,tv,m_params.frameWidth,m_params.frameHeight,10.0f + m_alpha*al*GetAlpha());
		tu,tv,m_params.frameWidth,m_params.frameHeight,10.0f + m_alpha*al);

/*	if( !m_shouldPlay )
		return;

	if( m_curTime - m_lastTime >= 1.0f/m_params.fps )
	{
		m_curFrame++;

		if( m_curFrame >= m_params.firstFrame + m_params.framesCount )
		{
			if( m_params.playMethod != Params::OnceByCommand )
				m_curFrame = m_params.firstFrame;
			else
				m_curFrame--;

			if( m_params.playMethod != Params::Continious )
				m_shouldPlay = false;
		}

		m_lastTime = m_curTime;
	}*/
/*
//	m_curTime += api->GetDeltaTime(); // не стоит использовать движковый DeltaTime
	m_curTime += deltaTime;*/
}

void GUIWidget::RenderGeometry(const Rect &r)
{
	if( !enableRender )
		return;

	if( !m_model )
		return;

	Color c(drawColor);	c.a *= m_alpha;

	if( c.a < 0.001f )
		return;

//	Matrix saveView = Render().GetView();
	Matrix saveProj = Render().GetProjection();

//	Render().SetView	  (Matrix());
//	Render().SetProjection(Matrix());

	Matrix proj(true); proj.BuildOrtoProjection(2.0f,2.0f,1.0f,100.0f);

	Render().SetProjection(proj);

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	Vector pos = m_params.position;

	pos.x = 2*(pos.x*width /100.0f + r.l/100.0f - 0.5f);
	pos.y = 2*(pos.y*height/100.0f - r.t/100.0f + 0.5f - height/100.0f);

	pos.z = 5.0f;

	Matrix world;

	const RENDERVIEWPORT &vp = Render().GetViewport();

	float asp = vp.Width/(float)vp.Height;

	asp /= InterfaceUtils::AspectRatio(Render());

	const float aspect = 1.0f/asp;

	float xs = m_params.scale*aspect;
	float ys = m_params.scale;

	if( m_scale >= 0.0f )
	{
		xs *= m_scale;
		ys *= m_scale;
	}

	float zs = xs;

	world *= Matrix().BuildScale(xs,ys,zs);
	world *= Matrix(m_params.orient,pos);

	Matrix view = Render().GetView();
	view.Inverse();

	world *= view;

	m_model->SetTransform(world);

	// чистим z-буфер под геометрией
	Vector vMin(world.MulVertex(m_model->GetBound().vMin));
	Vector vMax(world.MulVertex(m_model->GetBound().vMax));

	RENDERRECT rect;

	rect.x1 = (long)((vMin.x + 1.0f)*0.5f*Render().GetViewport().Width);
	rect.x2 = (long)((vMax.x + 1.0f)*0.5f*Render().GetViewport().Width);

	rect.y1 = (long)((vMin.y + 1.0f)*0.5f*Render().GetViewport().Height);
	rect.y2 = (long)((vMax.y + 1.0f)*0.5f*Render().GetViewport().Height);

	Render().Clear(1,&rect,CLEAR_ZBUFFER,0,1.0f,0);

	FogParamsSave();

	m_model->SetUserColor(c);
	m_model->Draw();

	FogParamsRestore();

//	Render().SetView	  (saveView);
	Render().SetProjection(saveProj);
}

void GUIWidget::RenderText(const Rect &r)
{
	if( !m_font )
		return;

/*	float fh = m_font->GetHeight();

	m_font->SetHeight(fh*m_fontScale);

	dword cx = Render().GetViewport().Width;
	dword cy = Render().GetViewport().Height;

	float width	 = r.r - r.l;
	float height = r.b - r.t;

	float textX = cx*SC2UIRC((r.l)/100.0f);
	float textY = cy*SC2UIRC((r.t)/100.0f);

	float outputWidth  = cx*SS2UIRS(width /100.0f);
	float outputHeight = cy*SS2UIRS(height/100.0f);

	float textHeight = m_font->GetHeight()*m_linesCount;

	switch( m_params.vertAlign )
	{
		case Params::Center:
			textY += (outputHeight - textHeight)/2;
			break;

		case Params::Bottom:
			textY += (outputHeight - textHeight);
			break;
	}

	dword  strBegin  = 0;
	dword lineNumber = 0;

	Vertex *p = (Vertex *)pVB->Lock(0,0,LOCK_DISCARD);
	Vertex *q = p + icon_max_count/2*4;

	int n = 0;

	for( dword i = 0; i < m_params.defaultString.Len(); i++ )
	{
		if( m_params.defaultString[i] == '\n' )
		{
			m_params.defaultString[i] = 0;

			char *line = (char *)m_params.defaultString.GetBuffer() + strBegin;
			float lineWidth = m_font->GetLength(line);

			textX = cx*SC2UIRC(r.l/100.0f);

			switch( m_params.horizAlign )
			{
				case Params::Center:
					textX += (outputWidth - lineWidth)/2;
					break;

				case Params::Right:
					textX += (outputWidth - lineWidth);
					break;
			}

			Color c = m_params.color;
			
		//	c.a *= m_fadeAlpha*GetAlpha();
			c.a *= m_alpha;

			m_font->SetColor(c);

			if( m_effTime > 0.0f && m_parsed )
			{
				float y = textY + lineNumber*m_font->GetHeight();
				
				if( m_effBeg > 0 )
				{
					char d = line[m_effBeg];

					line[m_effBeg] = 0;

					m_font->Print(textX,y,line);

					line[m_effBeg] = d;
				}

				if( m_effBeg + m_effLen < strlen(line))
				{
					m_font->Print(textX + m_effWidth,y,line + m_effBeg + m_effLen);
				}
			}
			else
			{
				m_font->Print(textX,textY + lineNumber*m_font->GetHeight(),line);
			}

			m_strOffX = textX;
			m_strOffY = textY + m_font->GetHeight()/2;

			if( table )
			{
				for( int j = 0 ; j < hash[lineNumber] ; j++ )
				{
					if( n > icon_max_count/2 )
						break;

					Info &info = table[n++];

					float xx = (textX + info.x)/cx;
					float yy = (textY + info.y)/cy;

					xx = xx*2.0f - 1.0f;
					yy = 1.0f - yy*2.0f;

				//	float sy = info.h/cy*2*m_hk;
					float sy = info.h/cy*2;

				//	float sx = info.w/cx*2*m_hk;
					float sx = sy*info.k;

					float dx = 0.0f;
					float dy = 0.0f;

					float ak = 1.0f;

				//	if( m_useAnim )
					if( m_useAnim && info.anim )
				//	{
				//		float k = 1.0f + (1.0f - cosf(m_time))*0.5f*0.2f;

				//		float tx = sx*k;
				//		float ty = sy*k;

				//		xx -= (tx - sx)*0.5f;
				//		yy += (ty - sy)*0.5f;

				//		sx = tx;
				//		sy = ty;
				//	}
					{
						float k = (1.0f - cosf(m_time))*0.5f*0.1f;

					//	ak = 1.0f - k*10.0f;
						ak = 0.6f - k*10.0f*0.4f;

						float tx = sx*(1.0f + k);
						float ty = sy*(1.0f + k);

					//	xx -= (tx - sx)*0.5f + k*sx;
					//	yy += (ty - sy)*0.5f + k*sy;
						dx  = (tx - sx)*0.5f + k*sx;
						dy  = (ty - sy)*0.5f + k*sy;

						sx = tx;
						sy = ty;
					}

					ImagePlace &pl = info.p;

					float _l = pl.u; float _r = _l + pl.w;
					float _t = pl.v; float _b = _t + pl.h;

					q[0].p = Vector(xx	   ,yy - sy,0.0f);
					q[1].p = Vector(xx	   ,yy	   ,0.0f);
					q[2].p = Vector(xx + sx,yy	   ,0.0f);
					q[3].p = Vector(xx + sx,yy - sy,0.0f);

					q[0].tu = _l; q[0].tv = _b;
					q[1].tu = _l; q[1].tv = _t;
					q[2].tu = _r; q[2].tv = _t;
					q[3].tu = _r; q[3].tv = _b;

					q[0].al = c.a*ak;//m_fadeAlpha;
					q[1].al = c.a*ak;//m_fadeAlpha;
					q[2].al = c.a*ak;//m_fadeAlpha;
					q[3].al = c.a*ak;//m_fadeAlpha;

					q += 4;

					xx -= dx;
					yy += dy;

					p[0].p = Vector(xx	   ,yy - sy,0.0f);
					p[1].p = Vector(xx	   ,yy	   ,0.0f);
					p[2].p = Vector(xx + sx,yy	   ,0.0f);
					p[3].p = Vector(xx + sx,yy - sy,0.0f);

					p[0].tu = _l; p[0].tv = _b;
					p[1].tu = _l; p[1].tv = _t;
					p[2].tu = _r; p[2].tv = _t;
					p[3].tu = _r; p[3].tv = _b;

					p[0].al = c.a;//m_fadeAlpha;
					p[1].al = c.a;//m_fadeAlpha;
					p[2].al = c.a;//m_fadeAlpha;
					p[3].al = c.a;//m_fadeAlpha;

					p += 4;
				}
			}

			m_params.defaultString[i] = '\n';

			 strBegin = i + 1;
			lineNumber++;
		}
	}

	pVB->Unlock();

	if( n )
	{
		Render().SetStreamSource(0,pVB);
		Render().SetIndices(pIB,0);

		if( Image )
			Texture->SetTexture(Image);

		Render().DrawIndexedPrimitive(Circular_shadow_id, 
			PT_TRIANGLELIST,icon_max_count/2*4,n*4,icon_max_count/2*6,n*2);

		Render().DrawIndexedPrimitive(Circular_id,
			PT_TRIANGLELIST,0,n*4,0,n*2);
	}

	m_font->SetHeight(fh);*/
	if( m_text )
	{
		m_text->Update(m_time);
		m_text->UpdateCutsTime(m_cutsTime);

		m_text->SetColor(m_params.color);
		m_text->SetAlpha(m_alpha);

	//	IGUIText::Effect eff;
		IAdvFont::Effect eff;

		eff.Beg = m_effBeg;
		eff.Len = m_effLen; eff.Width = m_effWidth;

		m_text->SetScale(m_fontScale);
		m_text->SetFakeScale(m_fontScale);

		m_text->Draw(true,/*m_font,*/r.l,r.t,true,m_effTime > 0.0f && m_parsed ? &eff : null);

		m_text->GetPos(m_strOffX,m_strOffY);

		if( !EditMode_IsOn())
		{
			if( m_cutsUse )
			{
				if( !m_text->IsTextActive()) // истекло время отображения подсказки
				{
					if( _curText && _curText == this )
					{
						_curText = null;
					}

					Show(false);
				}
			}
		}
	}
}
/*
void GUIWidget::Work(float deltaTime, long)
{
//	if( m_useAnim )
		m_time += deltaTime*20.0f;//*m_animSpeed;

	if( EditMode_IsOn())
	{
		ValidateParent();
	}

//	if( Controls().GetControlsImage() != lastImage && m_needUpdate )
	if( Controls().GetImagesUpdated() != m_updated && m_needUpdate )
		UpdateText();

	m_alphaOriginal = GetAlpha();

	if( GetFadeState() == BaseGUIElement::FadingIn )
	{
		if( m_fadeAlpha == 0.0f )
			m_fadeAlpha  = m_alphaOriginal*0.5f;
		else
			m_fadeAlpha += (deltaTime/m_fadeTime)*m_alphaOriginal*1.5f;

		if( m_fadeAlpha > m_alphaOriginal )
		{
			m_fadeAlpha = m_alphaOriginal; SetFadeState(FadingFinished);
		}
	}
	else
	if( GetFadeState() == BaseGUIElement::FadingOut )
	{
	//	m_fadeAlpha -= (deltaTime/m_fadeTime)*m_alphaOriginal*0.7f;
		m_fadeAlpha -= (deltaTime/m_fadeTime)*m_alphaOriginal*1.2f;

		if( m_fadeAlpha < 0)
		{
			m_fadeAlpha = 0.0f; SetFadeState(FadingFinished);

		//	_Show(false);

			bool show = nativeShow; _Show(false); nativeShow = show;
		}
	}
	else
	{
		m_fadeAlpha = m_alphaOriginal;
	}

	Rect rect; GetRect(rect);

	rect.l += m_frameXOffset*m_aspect;
	rect.r += m_frameXOffset*m_aspect;

	rect.t += m_frameYOffset;
	rect.b += m_frameYOffset;

	Rect clip; GetParentClipRect(clip);

	if( !RectInRect(
			clip.l,
			clip.t,
			clip.r,
			clip.b,rect.l,rect.t,rect.r,rect.b))
		return;

	if( !PointInRect(rect.l,rect.t,clip) ||
		!PointInRect(rect.r,rect.b,clip))
		SetClipRect(&clip);

//	SetAlpha(m_fadeAlpha);
	m_alpha = m_fadeAlpha;

//	m_alpha = GetAlpha();

	// отрисовка картинки
	RenderBackGround(rect,deltaTime);

	// отрисовка геометрии
	RenderGeometry	(rect);

	// отрисовка текста
	RenderText		(rect);

	if( m_effTime > 0.0f )
	{
		float k = m_effTime/effectTime;

		Color c; c.LerpA(m_params.color,effectColor,k);

		m_font->SetHeight(m_params.size*(1.0f + effectSize*k));
		m_font->SetColor(c);

		char *s = m_params.defaultString.GetDataBuffer() + m_effBeg;
		char  d = s[m_effLen];

		float rx = 0.5f*m_font->GetLength(m_effLen,s);
		float ry = 0.5f*m_font->GetHeight();

		s[m_effLen] = 0;

		m_font->Print(m_strOffX + m_effOffX - rx,m_strOffY - ry,s);

		s[m_effLen] = d;

		m_font->SetHeight(m_params.size);

		m_effTime -= deltaTime;

		if( m_effTime < 0.0f )
			m_effTime = 0.0f;
	}

	m_frameXOffset = m_frameYOffset = 0.0f;

	SetClipRect(NULL);
}
*/
void GUIWidget::Restart()
{
	m_restart = true;

	ReCreate();
}

void GUIWidget::Work(float deltaTime, long)
{
	if( EditMode_IsOn())
	{
		if( !EditMode_IsVisible())
			return;

		ValidateParent();
	}

	Draw();
	Update(deltaTime);
}

void GUIWidget::Update(float deltaTime)
{
	if( m_stat && m_statCur != m_statEnd && m_text )
	{
		m_statTime -= deltaTime;

		if( m_statTime <= 0.0f )
		{
			int d = 1;

			if( m_statTimeMax < deltaTime )
			{
				d = int(deltaTime/m_statTimeMax + 0.5f);
			}

			if( m_statCur < m_statEnd )
			{
				m_statCur += d;

				if( m_statCur > m_statEnd )
					m_statCur = m_statEnd;
			}
			else
			{
				m_statCur -= d;

				if( m_statCur < m_statEnd )
					m_statCur = m_statEnd;
			}

			char buf[32];

			sprintf_s(buf,sizeof(buf),"%d",m_statCur);

		/////////////////////////////////

		//	m_params.defaultString = buf;
		//	m_params.defaultPtr = m_params.defaultString.c_str();

			m_initString = buf;
			m_initPtr = m_initString.c_str();

			m_params.defaultPtr = m_initPtr;

		/////////////////////////////////

			m_text->Prepare(true,GetWidth()*m_aspect*m_widthAspectKoef,GetHeight(),m_params.defaultPtr,
				m_params.horizAlign,m_params.vertAlign,m_params.useSafeFrame,false);

			m_statTime = m_statTimeMax;
		}
	}

//	if( m_useAnim )
	//	m_time += deltaTime*20.0f;//*m_animSpeed;
		m_time += deltaTime*m_animSpeed;

	m_cutsTime += deltaTime;

//	if( Controls().GetControlsImage() != lastImage && m_needUpdate )
//	if( Controls().GetImagesUpdated() != m_updated && m_needUpdate )
//		UpdateText(true);

//	m_alphaOriginal = GetAlpha();
//	m_alphaOriginal = 1.0f;

	if( GetFadeState() == BaseGUIElement::FadingIn )
	{
		float t = m_alphaOriginal*0.5f;

		if( m_fadeAlpha < t )
			m_fadeAlpha = t;
		else
			m_fadeAlpha += (deltaTime/m_fadeTime)*m_alphaOriginal*1.5f;

		if( m_fadeAlpha > m_alphaOriginal )
		{
			m_fadeAlpha = m_alphaOriginal; SetFadeState(FadingFinished);
		}

		SetAlpha(m_fadeAlpha);
	}
	else
	if( GetFadeState() == BaseGUIElement::FadingOut )
	{
	//	m_fadeAlpha -= (deltaTime/m_fadeTime)*m_alphaOriginal*0.7f;
		m_fadeAlpha -= (deltaTime/m_fadeTime)*m_alphaOriginal*1.2f;

		if( m_fadeAlpha < 0)
		{
			m_fadeAlpha = 0.0f; SetFadeState(FadingFinished);

		//	_Show(false);

			bool show = nativeShow; _Show(false); nativeShow = show;
		}

		SetAlpha(m_fadeAlpha);
	}
	else
	{
	//	m_fadeAlpha = m_alphaOriginal;
	//	m_fadeAlpha = GetAlpha();
	}

	if( m_effTime > 0.0f )
	{
		m_effTime -= deltaTime;

		if( m_effTime < 0.0f )
			m_effTime = 0.0f;
	}

	if( m_shouldPlay )
	{
		if( m_curTime - m_lastTime >= 1.0f/m_params.fps )
		{
			m_curFrame++;

			if( m_curFrame >= m_params.firstFrame + m_params.framesCount )
			{
				if( m_params.playMethod != Params::OnceByCommand )
					m_curFrame = m_params.firstFrame;
				else
					m_curFrame--;

				if( m_params.playMethod != Params::Continious )
					m_shouldPlay = false;
			}

			m_lastTime = m_curTime;
		}
	}

	m_curTime += deltaTime;

	bool click = Controls().GetControlStateType(m_click) == CST_ACTIVATED;
	bool dblcl = Controls().GetControlStateType(m_dblcl) == CST_ACTIVATED;

	if( !click && !dblcl )
		return;

/*	if( EditMode_IsOn())
	{
		if( !m_cursor || !ValidatePointer(m_cursor,m_cursorHash))
			 m_cursor = (BaseGUICursor *)FindObject(InterfaceUtils::GetCursorName());
	}*/

	if( !m_cursor || m_cursor->IsHided())
		return;

	float x,y; m_cursor->GetPosition(x,y);

	x *= 100.0f;
	y *= 100.0f;

	Rect r; GetParentClipRect(r);

	if( !PointInRect(x,y,r))
		return;

	GetRect(r);

	if( !PointInRect(x,y,r))
		return;

	if( click )
		NotifyParent(this,ChildClicked);

	if( dblcl )
		NotifyParent(this,ChildDblClicked);
}

void GUIWidget::Draw()
{
	if( InterfaceUtils::IsHide())
		return;

	if( m_needUpdate )
	{
		if( Controls().GetControlsImage() == null ) // render device is lost
		{
			if( m_text )
				m_text->ResetImage();
		}
		else
		{
			if( Controls().GetControlsImageVersion() != m_version )
				UpdateText(true);
		}
	}

	Rect rect; GetRect(rect);

	rect.l += m_frameXOffset*m_aspect;
	rect.r += m_frameXOffset*m_aspect;

	rect.t += m_frameYOffset;
	rect.b += m_frameYOffset;

	Rect clip; GetParentClipRect(clip);

	if( !RectInRect(
			clip.l,
			clip.t,
			clip.r,
			clip.b,rect.l,rect.t,rect.r,rect.b))
		return;

	if( !PointInRect(rect.l,rect.t,clip) ||
		!PointInRect(rect.r,rect.b,clip))
		SetClipRect(&clip);

//	SetAlpha(m_fadeAlpha);
//	m_alpha = m_fadeAlpha;
	m_alpha = GetAlpha();

	if( !IsShow())
		m_alpha = GetNativeAlpha();

//	m_alpha = GetAlpha();

	// отрисовка картинки
	RenderBackGround(rect/*,deltaTime*/);

	// отрисовка геометрии
	RenderGeometry	(rect);

	// отрисовка текста
	RenderText		(rect);

	if( m_effTime > 0.0f && m_parsed )
	{
		float k = m_effTime/effectTime;

		Color c; c.LerpA(m_params.color,effectColor,k);

		m_font->SetHeight(m_params.size*(1.0f + effectSize*k));
		m_font->SetColor(c);

		char *s = m_params.defaultString.GetDataBuffer() + m_effBeg;
		char  d = s[m_effLen];

		float rx = 0.5f*m_font->GetLength(m_effLen,s);
		float ry = 0.5f*m_font->GetHeight();

		s[m_effLen] = 0;

		m_font->Print(m_strOffX + m_effOffX - rx,m_strOffY - ry,s);

		s[m_effLen] = d;

		m_font->SetHeight(m_params.size);
	}

	m_frameXOffset = m_frameYOffset = 0.0f;

	SetClipRect(NULL);
}

void GUIWidget::Draw(float x, float y, float alpha, bool inner)
{
	if( InterfaceUtils::IsHide())
		return;

//	if( Controls().GetControlsImage() != lastImage && m_needUpdate )
	if( m_needUpdate )
	{
		if( Controls().GetControlsImage() == null ) // render device is lost
		{
			if( m_text )
				m_text->ResetImage();
		}
		else
		{
			if( Controls().GetControlsImageVersion() != m_version )
				UpdateText(true);
		}
	}

	Rect rect; GetRect(rect);

	float cx = rect.r - rect.l;
	float cy = rect.b - rect.t;

	if( inner )
	{
		Rect clip; GetParentClipRect(clip);

		if( !RectInRect(
				clip.l,
				clip.t,
				clip.r,
				clip.b,rect.l,rect.t,rect.r,rect.b))
			return;

		if( !PointInRect(rect.l,rect.t,clip) ||
			!PointInRect(rect.r,rect.b,clip))
			SetClipRect(&clip);
	}
	else
	{
		x = x*100.0f - cx*0.5f;
		y = y*100.0f - cy*0.5f;

		rect.l = x; rect.r = x + cx;
		rect.t = y; rect.b = y + cy;
	}

	if( EditMode_IsOn() || m_params.texture[0] )
	{
		m_quadRender.DrawQuad(rect.l,rect.t,cx,cy,0.0f,0.0f,
			m_params.frameWidth ,
			m_params.frameHeight,10.0f + alpha);
	}

	float a = m_alpha;
			  m_alpha = alpha;

	float f = m_fadeAlpha;
			  m_fadeAlpha = alpha;

	RenderGeometry(rect);
	RenderText	  (rect);

	m_alpha		= a;
	m_fadeAlpha = f;

	if( inner )
		SetClipRect(NULL);
}

bool GUIWidget::IsPlayingAnimation()
{
	return m_shouldPlay;
}

void GUIWidget::SkipMOPs(MOPReader &reader)
{
	if( m_text && m_cutsUse )
	{
		if( _curText == this )
			_curText = null;
	}

	reader.Bool();		// use safe frame

	reader.Float();		// width
	reader.Float();		// height

	reader.Long();		// draw priority

	reader.Bool();		// use fading
	reader.Bool();		// use effect

	reader.Float();		// effect time
	reader.Float();		// effect size

	reader.Colors();	// effect color

	m_params.stringID = reader.LocString();
	m_initPtr		  = reader.String().c_str();

	if( m_params.stringID[0] )
		m_initPtr = m_params.stringID;

//	m_params.defaultString = m_initPtr;
	m_params.defaultPtr	   = m_initPtr;

	reader.String().c_str();	// font name
	reader.Float();		// font size

	reader.Float();		// font kerning

	reader.Colors();	// font color

	reader.Enum();		// text hor align
	reader.Enum();		// text ver align

	//// statistic effect ////
	
	reader.Bool();		// numeric
	reader.Float();		// update time

	//////////////////////////

	reader.String().c_str();	// model name
	reader.String().c_str();	// anim  name

	reader.Position();	// model pos
	reader.Angles();	// model ang

	reader.Float();		// model scale

	//// render params ////

	reader.Bool();		// skip fog

	reader.Colors();	// draw color

	reader.Bool();		// smooth alpha
	reader.Bool();		// enable render
	reader.Bool();		// dynamic lighting

	///////////////////////

	reader.String().c_str();	// texture name

	reader.Bool();		// is black&white

	m_curFrame = m_params.firstFrame = reader.Long();

	reader.Long();		// frames count

	reader.Float();		// frame offset
	reader.Float();		//

	reader.Float();		// frame width
	reader.Float();		// frame height

	reader.Long();		// fps

	reader.Enum();		// play method

	/////////////////

	reader.Bool();		// cuts use

	reader.Float();		// cuts delay
	reader.Long();		// cuts priority

	reader.Bool();		// cuts debug

	/////////////////

	m_alphaOriginal = GetNativeAlpha();

	if( m_fading )
		SetAlpha(0.0f);

	SetFadeState(FadingFinished);

	reader.Float();		// anim speed

	nativeShow = reader.Bool();
	MissionObject::Show(false);

	m_fade = 0.0f;
	m_fadeAlpha = 0.0f;

	MissionObject::Activate(true);
}

void GUIWidget::ReadMOPs(MOPReader &reader)
{
	if( m_text && m_cutsUse )
	{
		if( _curText == this )
			_curText = null;
	}

//	m_cursor = (BaseGUICursor *)FindObject(InterfaceUtils::GetCursorName());
	m_cursor = null;

//	string enumStr;
	const char *enumStr;

	m_params.useSafeFrame = reader.Bool();

	float width	 = reader.Float();
	float height = reader.Float();

//	m_initPtr = null;

	if( !Restricted(preserveSize))
		SizeTo(width,height);

	m_natSize.w = width;
	m_natSize.h = height;

	m_params.drawPriority = reader.Long();

	drawShift = 0;

	m_fading = reader.Bool();
	m_effect = reader.Bool();

//	m_effBeg	   = reader.Long();
//	m_effLenNative = reader.Long();

	effectTime = reader.Float();
	effectSize = reader.Float();

	effectColor = reader.Colors();

	m_params.stringID = reader.LocString();
	m_initPtr		  = reader.String().c_str();

	if( m_params.stringID[0] )
		m_initPtr = m_params.stringID;

//	m_params.defaultString = m_initPtr;
	m_params.defaultPtr	   = m_initPtr;

	m_params.fontName = reader.String().c_str();
	m_params.size	  = reader.Float();

	m_params.kerning = reader.Float();

	m_hk = Render().GetFullScreenViewPort_2D().Height/1024.0f;

	m_params.size *= m_hk;

	m_params.color = reader.Colors();

//	effectColor	   = m_params.color*effectColor;
//	effectColor.a *= m_params.color.a;

	enumStr = reader.Enum().c_str();

	if( string::IsEqual(enumStr,ALIGN_CENTER))
		m_params.horizAlign = IAdvFont::Center;
	else
	if( string::IsEqual(enumStr,ALIGN_LEFT))
		m_params.horizAlign = IAdvFont::Left;
	else
	if( string::IsEqual(enumStr,ALIGN_RIGHT))
		m_params.horizAlign = IAdvFont::Right;

	enumStr = reader.Enum().c_str();

	if( string::IsEqual(enumStr,ALIGN_CENTER))
		m_params.vertAlign = IAdvFont::Center;
	else
	if( string::IsEqual(enumStr,ALIGN_TOP))
		m_params.vertAlign = IAdvFont::Top;
	else
	if( string::IsEqual(enumStr,ALIGN_BOTTOM))
		m_params.vertAlign = IAdvFont::Bottom;

	//// statistic effect ////

	m_stat = reader.Bool();
	m_statTimeMax = reader.Float();

	//////////////////////////

	m_params.model = reader.String().c_str();
	m_params.anim  = reader.String().c_str();

	m_params.position = reader.Position();
	m_params.orient	  = reader.Angles();

	m_params.scale = reader.Float();

	//// render params ////

	skipFog	  = reader.Bool();

	drawColor = reader.Colors();

	m_smoothAlpha = reader.Bool();

	enableRender = reader.Bool();

	dynLighting = reader.Bool();

	///////////////////////

	const char *t1 = reader.String().c_str();
//	const char *t2 = reader.String().c_str();

//	if( m_aspect > 1.0f && !m_widthAspect && t2[0] )
//		m_params.texture = t2;
//	else
		m_params.texture = t1;

	m_black = reader.Bool();

	m_curFrame = m_params.firstFrame = reader.Long();

	m_params.framesCount = reader.Long();

	m_params.frameStartU = reader.Float();
	m_params.frameStartV = reader.Float();

	m_params.frameWidth	 = reader.Float();
	m_params.frameHeight = reader.Float();

	m_params.fps = reader.Long();

	enumStr = reader.Enum().c_str();

	if( string::IsEqual(enumStr,PLAY_CONTINIOUS))
		m_params.playMethod = Params::Continious;
	else
	if( string::IsEqual(enumStr,PLAY_ONCE_ON_ACTIVATION))
		m_params.playMethod = Params::OnceOnActivate;
	else
	if( string::IsEqual(enumStr,PLAY_ONCE_ON_SHOW))
		m_params.playMethod = Params::OnceOnShow;
	else
	if( string::IsEqual(enumStr,PLAY_ONCE_BY_COMMAND))
		m_params.playMethod = Params::OnceByCommand;

	/////////////////

	m_cutsUse = reader.Bool();

	m_cutsDelay	   = reader.Float();
	m_cutsPriority = reader.Long();

	m_cutsDebug = reader.Bool();

	/////////////////

//	m_alphaOriginal = GetAlpha();
//	m_alphaOriginal = 1.0f;
	m_alphaOriginal = GetNativeAlpha();

	if( m_fading )
		SetAlpha(0.0f);

//	m_alphaOriginal = GetNativeAlpha();

	SetFadeState(FadingFinished);

//	m_useAnim = reader.Bool();
	m_useAnim = true;

	m_animSpeed = reader.Float()*2.0f*PI;
//	m_time = 0.0f;

	nativeShow = reader.Bool();
//	Show(reader.Bool());

	bool isChangeWidth = reader.Bool();
	m_widthAspectKoef = isChangeWidth ? 1.0f/m_aspect : 1.0f;

	m_fade = 0.0f;
	m_fadeAlpha = IsShow() ? 1.0f : 0.0f;

//	m_za = 0.0f;

	if( !Restricted(preserveSize))
		SizeTo(width,height);


	MissionObject::Activate(true);
}

void _cdecl GUIWidget::InitFunc(float, long)
{
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

		m_initComplete = true;
	}

	DelUpdate(&GUIWidget::InitFunc);

	UpdateText();

	Show(nativeShow);
}

void GUIWidget::OnParentNotify(Notification event)
{
	BaseGUIElement::OnParentNotify(event);
}

bool GUIWidget::Create(MOPReader &reader)
{
/*	Render().GetShaderId("Circular_shadow", Circular_shadow_id);
	Render().GetShaderId("Circular", Circular_id);
*/
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

bool GUIWidget::RestartObject(MOPReader &reader)
{
	BaseGUIElement::SkipMOPs(reader);
	SkipMOPs(reader);

	NotifyChildren(ParentChanged);

//	SetUpdate(&GUIWidget::InitFunc,EditMode_IsOn() ? ML_FIRST : ML_FIRST - 1);
	SetUpdate(&GUIWidget::InitFunc,ML_FIRST + 1);

	if( m_initComplete == false )
	{
		m_restart = false;
	}

	UpdateText();

/*	if( m_params.stringID[0] )
		m_params.defaultString = m_params.stringID;
	
	m_initString = m_params.defaultString;*/

	if( m_params.playMethod == Params::Continious )
		m_shouldPlay = true;

	m_frameXOffset = m_frameYOffset = 0.0f;

	return true;
}

bool GUIWidget::EditMode_Update(MOPReader &reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs(reader);

//	m_asp = m_aspect > 1.0f ? 4.0f/3.0f : 16.0f/9.0f;
	m_asp = m_aspect_native;

	NotifyChildren(ParentChanged);

//	SetUpdate(&GUIWidget::InitFunc,EditMode_IsOn() ? ML_FIRST : ML_FIRST - 1);
	SetUpdate(&GUIWidget::InitFunc,ML_FIRST + 1);

	m_initComplete = false;

	// Для корректной ининциализации RenderAdvFont в функции InitFunc():
	// ML_FIRST - 1 в режиме игры - чтобы сработать до триггеров
	// ML_FIRST     в режиме радактора - чтобы у рендера вьюпорт был установлен(?)

/*	if( m_params.stringID[0] )
		m_params.defaultString = m_params.stringID;*/

	m_quadRender.SetTexture(m_params.texture,m_black);

	IGMXScene *oldModel = m_model;

	m_model = Geometry().CreateScene(m_params.model,&Animation(),&Particles(),&Sound(),_FL_);

	if( m_model )
		m_model->SetAnimationFile(m_params.anim);

//	if( m_model )
//		m_model->SetFloatAlphaReference(m_smoothAlpha ? 0.003921f : 0.5f);
	if( m_model )
	{
		m_model->SetDynamicLightState(dynLighting);

		if( m_smoothAlpha )
			m_model->SetFloatAlphaReference(0.003921f);
	}

	if( oldModel )
	{
		oldModel->Release();
		oldModel = null;
	}

	IFont *oldFont = m_font;

	if( m_params.fontName[0] )
	{
		m_font = Render().CreateFont(m_params.fontName,m_params.size,-1,"dbgFontAlpha");

		if( m_font )
		{
			m_font->SetColor(m_params.color);

		//	float aspect = InterfaceUtils::AspectRatio(Render());

		//	m_font->SetHeight(Render().GetViewport().Height/768.0f*m_params.size*SS2UIRS(1.0f)*aspect);
		//	m_font->SetHeight(Render().GetViewport().Height/768.0f*m_params.size*SS2UIRS(1.0f));
		}

		if( oldFont )
		{
			oldFont->Release();
			oldFont = null;
		}

	//	float aspect = Render().GetViewport().Width/(float)Render().GetViewport().Height;

		if( m_font )
		//	m_font->SetHeight(Render().GetViewport().Height/768.0f*m_params.size*aspect);
		//	m_font->SetHeight(Render().GetViewport().Height/768.0f*m_params.size);
			m_font->SetHeight(m_params.size);

	//	m_initString = m_params.defaultString;

//		Parse	   (m_params.defaultString);
//		UpdateTable(m_params.defaultString);

		// форматирование строчки, word-wrap
//	/*	m_linesCount = InterfaceUtils::WordWrapString(
//			m_params.defaultString,m_font,
//			m_params.width/100.0f*Render().GetViewport().Width);*/
//		m_linesCount = InterfaceUtils::WordWrapString(
//			buffer				  ,m_font,
//			GetWidth()/100.0f*Render().GetViewport().Width*m_aspect/*/m_hk*/,buffer_size);

//		UpdateHash (m_params.defaultString);

//		m_needUpdate = (m_initString != m_params.defaultString);
	}
	///////////////////////////

/*	if( !m_text )
	{
		IGUITextManager *tm = (IGUITextManager *)Mission().CreateObject("GUITextManager","GUITextManager");
		Assert(tm)

		m_text = tm->CreateText();
	}*/
	RenderAdvFont *text = (RenderAdvFont *)Render().CreateAdvancedFont(
		m_params.fontName,m_params.size,m_params.color);

	if( m_text )
		m_text->Release();

	m_text = text;

	if( m_text )
	{
		m_text->SetCutsDelay(m_cutsUse,m_cutsDelay);
		m_text->DrawDebug(m_cutsDebug);
		m_text->SetPlayCuts(!EditMode_IsOn());
	//	m_text->SetProirity(m_cutsPriority);
	}

	///////////////////////////

	if( m_params.playMethod == Params::Continious )
		m_shouldPlay = true;

	m_frameXOffset = m_frameYOffset = 0.0f;

	UpdateText();

	return true;
}
/*
void GUIWidget::UpdateTable(string &s)
{
	if( !m_font )
		return;

	table.DelAll();

	char name[40];

	char *line = (char *)s.GetBuffer();
	char *p = line;

	IBaseTexture *image = Controls().GetControlsImage();
//	Assert(image)

	dword imageW = Image ? Image->GetWidth () : 0;
	dword imageH = Image ? Image->GetHeight() : 0;

	string t;

	while( 1 )
	{
		p = strchr(line,'[');

		if( !p )
			break;

		if( p > line )
		{
			*p = 0;

			t += line;

			*p = '[';

			line = p;
		}

		p++;

		for( int j = 0 ; *p && *p != ']' ; )
			name[j++] = *p++;

		if( !*p )
			break;

		name[j] = 0;

		Info &info = table[table.Add()];

		info.p = Controls().GetControlImagePlace(name);

		info.w = imageW*info.p.w;
		info.h = imageH*info.p.h;

		int n = (int)ceilf(info.w/m_font->GetLength("#"));

		while( n-- )
			t += '#';

		p++; line = p;
	}

	t += line;

	s = t;
}
*//*
void GUIWidget::UpdateTable(string &s)
{
	if( !m_font )
		return;

	float fh = m_font->GetHeight()*font_hk;

	table.DelAll();

	char name[40];

	char *line = (char *)s.GetBuffer();
	char *p = line;

//	IBaseTexture *image = Controls().GetControlsImage();
//	Assert(image)

//	dword imageW = image->GetWidth ();
//	dword imageH = image->GetHeight();

//	Assert(Image)
	Image = Controls().GetControlsImage();

//	lastImage = Image;
	m_updated = Controls().GetImagesUpdated();

	dword imageW = Image ? Image->GetWidth () : 0;
	dword imageH = Image ? Image->GetHeight() : 0;

	dword cx = Render().GetViewport().Width;
	dword cy = Render().GetViewport().Height;

//	string t;
	buffer[0] = 0;

	while( 1 )
	{
		p = strchr(line,'[');

		if( !p )
			break;

		if( p > line )
		{
			*p = 0;

		//	t += line;
			strcat_s(buffer,buffer_size,line);

			*p = '[';

			line = p;
		}

		p++;

		if( *p == '[' )
		{
			strcat_s(buffer,buffer_size,"[");

			line = p = p + 1;

			continue;
		}

		bool anim = false;

		if( *p == '#' )
		{
			anim = true; p++;
		}

		for( int j = 0 ; *p && *p != ']' ; )
			name[j++] = *p++;

		if( !*p )
			break;

		name[j] = 0;

		Info &info = table[table.Add()];

		info.p = Controls().GetControlImagePlace(name);

		if( info.p.h )
		{
			info.anim = anim;
			
			info.w = imageW*info.p.w;//*0.4f;
			info.h = imageH*info.p.h;//*0.4f;

			info.w *= fh/info.h;
			info.h  = fh;

			info.k = info.w/info.h/m_asp;

			int n;

			float width = m_font->GetLength("`");

			if( fabs(width) < 1e-30f ) // символ не найден
				n = info.w ? 1 : 0;
			else
			{
			//	n = info.w ? (int)ceilf(info.w*m_hk/width) : 0;

			//	float sx = info.h/cy*m_hk*info.k;
				float sx = info.h/cy*info.k;

				n = info.w ? (int)ceilf(sx*cx/width) : 0;
			}

			while( n-- > 0 )
			//	t += '@';
				strcat_s(buffer,buffer_size,"`");
		}
		else
		{
			info.w = 0.0f;
			info.h = 0.0f;

			info.k = 0.0f;

		//	strcat_s(buffer,buffer_size,"[");
			strcat_s(buffer,buffer_size,name);
		//	strcat_s(buffer,buffer_size,"]");
		}

		p++; line = p;
	}

//	t += line;
	strcat_s(buffer,buffer_size,line);

//	s = t;
//	s = buffer;
}

float round(float x)
{
	float f = floorf(x);

	if( x - f > 0.5f )
		return f + 1.0f;
	else
		return f;
}*/
/*
void GUIWidget::UpdateHash (string &s)
{
	if( !m_font )
		return;

	hash.DelAll();

	dword i = 0; dword j = 0;
	dword n = 0;

	dword t = 0;
	dword d = 0;

	string z;

	hash.Add();
	hash[d] = 0;

	while( 1 )
	{
		while( s[i] != '#' && s[i] != '\n' )
			z += s[i++], j++;

		if( s[i] == '\n' )
		{
			z += s[i], j++;

			if( i >= s.Len() - 1 )
				break;
			else
			{
				i++; n = j;	d++;

				hash.Add();
				hash[d] = 0; continue;
			}
		}

		hash[d]++;

		Info &info = table[t++];

		info.x = m_font->GetLength(j - n,z.c_str() + n);
		info.y = m_font->GetHeight()*d;

		info.y += (m_font->GetHeight() - info.h)*0.5f;

		while( s[i] == '#' )
			i++;

		int n = (int)round(info.w/m_font->GetLength(" "));

		while( n-- )
			z += ' ', j++;

		if( s[i] == '\n' )
		{
			z += s[i], j++;

			if( i >= s.Len() - 1 )
				break;
			else
			{
				i++; n = j; d++;

				hash.Add();
				hash[d] = 0; continue;
			}
		}
	}

	s = z;
}
*//*
void GUIWidget::UpdateHash (string &s)
{
	if( !m_font )
		return;

	dword cx = Render().GetViewport().Width;
	dword cy = Render().GetViewport().Height;

//	float aspect = InterfaceUtils::AspectRatio(Render());

	hash.DelAll();

	dword i = 0; // текущая позиция в исходной строке
	dword j = 0; // текущая позиция в строке результата

	dword n = 0; // индекс первого символа в строке

	dword t = 0; // индекс в таблице
	dword d = 0; // номер текущей строки

//	string z;
//	buffer[0] = 0;

	hash.Add();
	hash[d] = 0;

	char *dst = buffer + buffer_size;

	while( 1 )
	{
	//	while( s	 [i] != '`' && s	 [i] != '\n' )
		while( buffer[i] != '`' && buffer[i] != '\n' )
		//	dst[j++] = s	 [i++];
			dst[j++] = buffer[i++];

	//	if( s	  [i] == '\n' )
		if( buffer[i] == '\n' )
		{
		//	dst[j++] = s	 [i];
			dst[j++] = buffer[i];

		//	if( i >= s.Len() - 1 )
			if( !buffer[i + 1] )
				break;
			else
			{
				i++; n = j;	d++;

				hash.Add();
				hash[d] = 0; continue;
			}
		}

		hash[d]++;

		Info &info = table[t++];

		info.x = m_font->GetLength(j - n,dst + n);
		info.y = m_font->GetHeight()*d;

	//	info.y += (m_font->GetHeight() - info.h*m_hk)*0.5f;
		info.y += (m_font->GetHeight() - info.h)*0.5f;

	//	while( s	 [i] == '`' )
		while( buffer[i] == '`' )
			i++;

		int n;

		float width = m_font->GetLength(" ");

		if( width == 0.0f ) // символ не найден
		//	n = 1;
			n = 0;
		else
		{
		//	n = (int)round(info.w*m_hk/width);

		//	float sx = info.h/cy*m_hk*info.k;
			float sx = info.h/cy*info.k;

			n = (int)round(sx*cx/width);
		//	n = (int)ceilf(sx*cx/width);
		}

		while( n-- )
			dst[j++] = ' ';

	//	if( s	  [i] == '\n' )
		if( buffer[i] == '\n' )
		{
		//	dst[j++] = s	 [i];
			dst[j++] = buffer[i];

		//	if( i >= s.Len() - 1 )
			if( !buffer[i + 1] )
				break;
			else
			{
				i++; n = j; d++;

				hash.Add();
				hash[d] = 0; continue;
			}
		}
	}

	dst[j] = 0;

//	s = z;
	s = dst;
}*/

void GUIWidget::Parse(string &s)
{
	m_parsed = false;

	dword len = s.Len();

	if( len && m_font )
	{
		for( dword i = 0 ; i < len ; i++ )
			if( s[i] == '`' )
				break;

		if( i < len - 1 )
		{
			m_effBeg = i;

			s.Delete(i,1); len--;

			if( s[i] == '`' )
			{
				s.Delete(i,1); return;
			}

			i++;

			while( i < len && s[i] != '`' ) i++;

			if( i < len )
			{
				m_effLen = i   - m_effBeg;

				s.Delete(i,1);
			}
			else
				m_effLen = len - m_effBeg;
		}
		else
		{
			if( i < len )
				s.Delete(i,1);

			return;
		}

		const char *t = s.GetBuffer();

		float width = m_font->GetLength(m_effLen,t + m_effBeg);

		m_effOffX   = m_font->GetLength(m_effBeg,t)
			+ width*0.5f;
	//	m_effWidth	= m_font->GetLength(m_effBeg		   ,t) + width;
		m_effWidth	= m_font->GetLength(m_effBeg + m_effLen,t);

		m_parsed = true;
	}
}

void GUIWidget::Show(bool isShow)
{
/*	if( !EditMode_IsOn())
	{
		if( isShow && m_stat )
		{
			if( IsShow())
			{
				m_statCur = m_statEnd - 1;

				m_statTime = 0.0f;
			}
			else
			{
				m_statCur = 0;
				m_statEnd = atoi(m_initPtr);

				m_statTime = m_statTimeMax;

				if( m_statCur < m_statEnd )
				{
					m_params.defaultString = "0";
					m_params.defaultPtr = m_params.defaultString.c_str();

					m_text->Prepare(true,GetWidth()*m_aspect*m_widthAspectKoef,GetHeight(),m_params.defaultPtr,
						m_params.horizAlign,m_params.vertAlign,m_params.useSafeFrame,false);
				}
			}
		}
	}*/

 	if( !EditMode_IsOn())
	{
		if( isShow )
		{
			if( m_text && m_cutsUse )
			{
				isShow = _sub ? fabsf(_sub->Get(1.0f)) > 0.1f : true;

				if( isShow )
				{
					//// обработка приоритетов ////

					if( _curText )
					{
						if( _curText != this )
						{
							if( _curText->GetPriority() < m_cutsPriority )
							{
								_curText->Show(false);
								_curText = this;
							}
							else
								isShow = false;
						}
					}
					else
					{
						_curText = this;
					}

					///////////////////////////////

					if( isShow )
					{
						m_cutsTime = 0.0f;

						m_text->UpdateCutsTime(m_cutsTime);
					}
				}
			}
		}
		else
		{
			if( m_text && m_cutsUse )
			{
				if( _curText == this )
					_curText = null;
			}
		}
	}

	if( !m_fading || Restricted(-1))
	{
		SetFadeState(FadingFinished); m_fadeAlpha = GetAlpha();

		if( m_fadeAlpha == 0.0f )
		{
			SetAlpha(m_fadeAlpha = m_alphaOriginal);
		}

		_Show(isShow);

		return;
	}

	if( IsShow())
	{
		if( !isShow )
		{
			if( GetFadeState() == FadingFinished )
			{
				m_fadeAlpha = GetAlpha();
				m_alphaOriginal = GetNativeAlpha();
			}

			SetFadeState(FadingOut); nativeShow = isShow;

		//	m_alphaOriginal = GetNativeAlpha();

			if( GetNativeAlpha() == 0.0f )
			{
				bool show = nativeShow; _Show(false); nativeShow = show;
			}

			return;
		}
		else
		{
			SetFadeState(FadingIn);

			if( EditMode_IsOn())
				_Show(isShow,false);

			return;
		}
	}
	else
	{
		if( isShow )
		{
			SetFadeState(FadingIn);	m_fadeAlpha = 0.0f;

		//	SetAlpha(m_fadeAlpha = 0.01f);
		/*	if( GetNativeAlpha() == 0.0f )
				SetAlpha(m_fadeAlpha = 0.01f);*/

			float alpha = GetNativeAlpha();

			if( alpha != 0.0f )
				m_alphaOriginal = alpha;

			if( m_alphaOriginal == 0.0f )
			{
				SetFadeState(FadingFinished);
			}
			else
				SetAlpha(m_fadeAlpha = 0.01f);
		}
	}

	_Show(isShow);
}

void GUIWidget::_Show(bool isShow, bool animate)
{
	BaseGUIElement::Show(isShow);

	if( m_fading && !IsShow() && GetFadeState() == FadingIn )
	{
		SetAlpha(0.0f);
		SetFadeState(FadingFinished);
	}

	if( animate && IsShow() && m_params.playMethod == Params::OnceOnShow )
	{
		m_shouldPlay = true;
		LogicDebug("Animation started");
	}

	BaseGUIElement *parent = GetParent();

	long level = drawLevel;

/*	if( parent )
		level = parent->GetDrawLevel() + 100;*/

//	DelUpdate((MOF_UPDATE)&GUIWidget::Work);

	if( EditMode_IsOn())
		DelUpdate((MOF_UPDATE)&GUIWidget::Work);

	if( IsShow())
	{
		if( GetAlpha() > 0.0f || GetFadeState() == FadingIn )
		{
			SetUpdate((MOF_UPDATE)&GUIWidget::Work,level + m_params.drawPriority + drawShift);
		//	api->Trace("WIDGET %s on",GetObjectID().c_str());
		}
	}
	else
	{
		DelUpdate((MOF_UPDATE)&GUIWidget::Work);
	//	api->Trace("WIDGET %s off",GetObjectID().c_str());
	}
}

void GUIWidget::Activate(bool isActive)
{
	BaseGUIElement::Activate(isActive);

	if( m_stat && IsShow())
	{
		if( m_statCur != m_statEnd )
		{
			m_statCur  = m_statEnd - 1;

			m_statTime = 0.0f;
		}

		return;
	}

	if( isActive && m_params.playMethod == Params::OnceOnActivate )
	{
		m_shouldPlay = true;
		LogicDebug("Animation started");
	}
}

void GUIWidget::Command(const char *id, dword numParams, const char **params)
{
/*	if( string::IsEqual(id,"UseAnim_1"))
	{
		m_useAnim = true;
	}
	else
	if( string::IsEqual(id,"UseAnim_0"))
	{
		m_useAnim = false;
	}
	else*/
	if( string::IsEqual(id,"SetString"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command GUIWidget::<SetString> error. Too few arguments");

			return;
		}

	//	LogicDebug("Command <SetString>: \"%s\"",params[0] ? params[0] : "");

		const char *new_value = params[0];

		if( m_stat )
		{
			if( IsShow() && IsActive())
			{
				m_statCur = atoi(m_params.defaultPtr);
				m_statEnd = atoi(params[0]);

				m_statTime = m_statTimeMax;

				if( string::IsEmpty(m_params.defaultPtr))
				{
					new_value = "0";
				}
				else
					return;
			}
			else
			{
				m_statCur = m_statEnd = atoi(params[0]);
			}
		}

	//	if( m_params.defaultString != new_value )
		if(!string::IsEqual(m_initPtr,new_value))
		{
			m_initString = new_value;
			m_initPtr = m_initString.c_str();

			m_params.defaultPtr = m_initPtr;

			///////

			int version = Controls().GetControlsImageVersion();

			if( version != m_version )
			{
				if( m_text )
					m_text->ResetImage();

				m_version = version;
			}

			///////

		//	Parse(m_params.defaultString);

			if( strchr(m_initPtr,'`'))
			{
				m_params.defaultString = m_initPtr;

				Parse(m_params.defaultString);

				m_params.defaultPtr = m_params.defaultString.c_str();
			}
			else
			{
				m_params.defaultPtr = m_initPtr; m_parsed = false;
			}

		/*	UpdateTable(m_params.defaultString);

		//	m_linesCount = InterfaceUtils::WordWrapString(
		//		m_params.defaultString,m_font,
		//		m_params.width*Render().GetViewport().Width/100.0f);
			m_linesCount = InterfaceUtils::WordWrapString(
				buffer				  ,m_font,
			//	GetWidth()/100.0f*Render().GetViewport().Width*m_aspect/m_hk,buffer_size);
				GetWidth()/100.0f*Render().GetViewport().Width*m_aspect		,buffer_size);

			UpdateHash (m_params.defaultString);

			m_needUpdate = (m_initString != m_params.defaultString);*/
			m_needUpdate = m_text ?
				m_text->Prepare(true,/*m_font,*/GetWidth()*m_aspect*m_widthAspectKoef,GetHeight(),/*m_initString*/
					/*m_params.defaultString*/m_params.defaultPtr,
					m_params.horizAlign,m_params.vertAlign,m_params.useSafeFrame,m_parsed) :
				false;
		}
	}
	else
	if( string::IsEqual(id,"Play"))
	{
		if( IsShow() && m_params.playMethod == Params::OnceByCommand && !m_shouldPlay )
			m_shouldPlay = true;
	}
	else
	if( string::IsEqual(id,"Reset"))
	{
		m_curFrame = m_params.firstFrame;
	}
	else
	if( string::IsEqual(id,"PlayEffect"))
	{
	/*	if( m_effect && m_font )
		{
			dword len = m_params.defaultString.Len();

			if( m_effBeg < len )
			{
				m_effLen = m_effLenNative;

				if( m_effLen > len )
					m_effLen = len;

				if( m_effLen + m_effBeg > len )
					m_effLen = len - m_effBeg;

				if( m_effLen )
				{
					const char *s = m_params.defaultString.GetBuffer();

					float width = m_font->GetLength(m_effLen,s + m_effBeg);

					m_effOffX  = m_font->GetLength(m_effBeg,s)
						+ width*0.5f;

					m_effWidth = m_font->GetLength(m_effBeg,s) + width;

					m_parsed = true;
				}
			}
		}*/

		if( m_effect && effectTime > 0.0f /*&& m_parsed*/ )
		{
		//	if( m_effTime <= 0.0f )
				m_effTime  = effectTime;
		}
	}
	else
	if( string::IsEqual(id,"SetColor"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command GUIWidget::<SetColor> error. Too few arguments");
			return;
		}

		if( params[0] )
		{
			dword color;

			if( sscanf_s(params[0],"%x",&color) == 1 )
			{
				m_params.color = Color(color);
			}
			else
			{
				LogicDebugError("Command GUIWidget::<SetColor> error. Invalid color value");
			}
		}
	}
	else
	if( string::IsEqual(id,"Goto"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command GUIWidget::<Goto> error. Too few arguments");
			return;
		}

		if( params[0] )
		{
			if( m_model )
			{
				IAnimation *ani = m_model->GetAnimation();

				if( ani )
				{
					ani->Goto(params[0],0.0f);

					LogicDebug("Goto animation node [%s]",params[0]);
				}
				else
				{
					LogicDebugError("No active animation");
				}
			}
		}
	}
	else
	{
		BaseGUIElement::Command(id,numParams,params);
	}
}

void GUIWidget::UpdateText(bool resetImage)
{
//	if( !m_initPtr )
//		return;

	if( m_text /*&& resetImage*/ )
	{
		m_text->ResetImage();
	}
/*
//	m_params.defaultString = m_initString;
	m_params.defaultString = m_initPtr;

	Parse	   (m_params.defaultString);*/

	if( strchr(m_initPtr,'`'))
	{
		m_params.defaultString = m_initPtr;

		Parse(m_params.defaultString);

		m_params.defaultPtr = m_params.defaultString.c_str();
	}
	else
	{
		m_params.defaultPtr = m_initPtr; m_parsed = false;
	}

/*	UpdateTable(m_params.defaultString);

//	m_linesCount = InterfaceUtils::WordWrapString(
//		m_params.defaultString,m_font,
//		m_params.width*Render().GetViewport().Width/100.0f);
	m_linesCount = InterfaceUtils::WordWrapString(
		buffer				  ,m_font,
	//	GetWidth()/100.0f*Render().GetViewport().Width*m_aspect/m_hk,buffer_size);
		GetWidth()/100.0f*Render().GetViewport().Width*m_aspect		,buffer_size);

	UpdateHash (m_params.defaultString);

	m_needUpdate = (m_initString != m_params.defaultString);*/

	if (m_text)
	{
		m_text->SetKerning(m_params.kerning);
	}

	m_needUpdate = m_text ?
		m_text->Prepare(true,/*m_font,*/GetWidth()*m_aspect*m_widthAspectKoef,GetHeight(),/*m_initString*/
			/*m_params.defaultString*/m_params.defaultPtr,
			m_params.horizAlign,m_params.vertAlign,m_params.useSafeFrame,m_parsed) :
		false;

	m_version = Controls().GetControlsImageVersion();

	/////////////////////////////////////////////////

	if( m_stat )
	{
		m_statCur = m_statEnd = atoi(m_params.defaultPtr);
	}
}
/*
void GUIWidget::CreateBuffers()
{
	if( !refCount++ )
	{
		IRender	  *render	= (IRender	 *)api->GetService("DX9Render");
	//	IControls *controls = (IControls *)api->GetService("Controls");

		if( pVB )
			pVB->Release();

		if( pIB )
			pIB->Release();

		pVB = render->CreateVertexBuffer(
			sizeof(Vertex)*icon_max_count*4,
			sizeof(Vertex),
			_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
		Assert(pVB)

		pIB = render->CreateIndexBuffer(
			sizeof(WORD)  *icon_max_count*6,
			_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
		Assert(pIB)

		WORD *p = (WORD*)pIB->Lock();
		Assert(p)

		for( long i = 0 ; i < icon_max_count ; i++ )
		{
			p[i*6 + 0] = WORD(i*4 + 0);
			p[i*6 + 1] = WORD(i*4 + 1);
			p[i*6 + 2] = WORD(i*4 + 2);
			p[i*6 + 3] = WORD(i*4 + 0);
			p[i*6 + 4] = WORD(i*4 + 2);
			p[i*6 + 5] = WORD(i*4 + 3);
		}

		pIB->Unlock();

		Texture = render->GetTechniqueGlobalVariable("CircularTexture",_FL_);
	//	Image = controls->GetControlsImage();
	}
}*/

static char GUIDescription[] =

"GUI Graphical element.\n\n"
"    Use it to setup graphic representation of any GUI item\n"
"    NOTE: all sizes-positions are relative (not pixel sizes-positions)\n\n"
"Commands:\n\n"
"    SetString <string> - set new default string\n\n"
"    SetColor <color> - set text color\n\n"
"    Play - play animation\n\n"
"    Reset - reset animation\n\n"
"    Goto <node name> - go to animation node\n\n"
"    PlayEffect - play string effect";

MOP_BEGINLISTCG(GUIWidget, "GUI Widget", '1.00', 1000, GUIDescription, "Interface")

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

	MOP_FLOATEX("Alpha", 1.0f, 0.0f, 1.0f)

	MOP_STRING("Parent id", "")

	MOP_BOOL("Use safe frame", true)

	MOP_FLOAT("Width" , 25.0f)
	MOP_FLOAT("Height", 25.0f)
	
	MOP_LONG("Draw priority", 0)

	MOP_BOOLC("Use fading", true , "Плавное появление/исчезновение")
	MOP_BOOLC("Use effect", false, "Показывать спецэффект при изменении строки")

	MOP_GROUPBEG("Effect")

	//	MOP_LONGC("Effect start", 0, "Первый символ для спецэффекта")
	//	MOP_LONGC("Effect count",-1, "Количество символов")

		MOP_FLOAT("Effect time", 0.5f)
		MOP_FLOAT("Effect size", 2.0f)

		MOP_COLOR("Effect color", (dword)-1)

	MOP_GROUPEND()

	MOP_GROUPBEG("Text")

		MOP_LOCSTRING("String id")
		MOP_STRING("Default string", "")

		MOP_STRING("Font name", "DemoFont")
		MOP_FLOAT ("Font size", 40)
		MOP_FLOAT ("Font kerning", 0)

		MOP_COLOR("Font color", Color(1,1,1,1))

		MOP_ENUMBEG("HAlignment")

			MOP_ENUMELEMENT(ALIGN_CENTER)
			MOP_ENUMELEMENT(ALIGN_LEFT)
			MOP_ENUMELEMENT(ALIGN_RIGHT)

		MOP_ENUMEND

		MOP_ENUM("HAlignment", "Horizontal alignment")

		MOP_ENUMBEG("VAlignment")

			MOP_ENUMELEMENT(ALIGN_CENTER)
			MOP_ENUMELEMENT(ALIGN_TOP)
			MOP_ENUMELEMENT(ALIGN_BOTTOM)

		MOP_ENUMEND

		MOP_ENUM("VAlignment", "Vertical alignment")

		MOP_BOOLC("Numeric", false, "Play statistic effect")
		MOP_FLOATEX("Update time", 0.02f, 0.0f, 0.5f)

	MOP_GROUPEND()

	MOP_GROUPBEG("Geometry")

		MOP_STRING("Model", "")
		MOP_STRING("Animation", "")

		MOP_POSITION("Position", Vector(0,0,5));
		MOP_ANGLES  ("Angles"  , Vector(0.0f));

		MOP_FLOAT("Scale", 1.0f)

		MOP_GROUPBEG("Render params")

			MOP_BOOLC("Disable fog", false, "Fog don't affect to this geometry")
			MOP_COLOR("Color", Color(0.0f, 0.0f, 0.0f, 1.0f))
			MOP_BOOLC("Smooth alpha", false, "Disable alpha test (Make smooth alpha)")
			MOP_BOOLC("Render", true, "Enable primary render")
			MOP_BOOL("Dynamic lighting", false)

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_GROUPBEG("Picture")

		MOP_STRING("Texture"	   , "")
	//	MOP_STRING("Texture (16:9)", "")

		MOP_BOOL("Black",false)

		MOP_LONG("First anim frame", 0)

		MOP_LONGEX("Anim frames count", 1, 1, 65536)

		MOP_FLOATEX("Anim frame start x", 0.0f, 0.0f, 1.0f)
		MOP_FLOATEX("Anim frames start y", 0.0f, 0.0f, 1.0f)

		MOP_FLOATEX("Anim frame width", 1.0f, 0.0f, 1.0f)
		MOP_FLOATEX("Anim frames height", 1.0f, 0.0f, 1.0f)

		MOP_LONG("Anim speed (FPS)", 10)

		MOP_ENUMBEG("Play Method")

			MOP_ENUMELEMENT(PLAY_CONTINIOUS)
			MOP_ENUMELEMENT(PLAY_ONCE_ON_ACTIVATION)
			MOP_ENUMELEMENT(PLAY_ONCE_ON_SHOW)
			MOP_ENUMELEMENT(PLAY_ONCE_BY_COMMAND)

		MOP_ENUMEND

		MOP_ENUM("Play Method", "Play Method")

	MOP_GROUPEND()

	MOP_GROUPBEG("Subtitles")

		MOP_BOOL("Use", false)

		MOP_FLOAT("Total time", 0.0f)
		MOP_LONG("Priority", 0)

		MOP_BOOL("Debug", false)

	MOP_GROUPEND()

//	MOP_BOOLC("Anim", false, "Анимировать динамические подсказки")
	MOP_FLOATEXC("Anim speed", 3.0f, 0.0f, 10.0f, "Скорость анимации подсказок")

	MOP_BOOL("Visible", true)

	MOP_BOOL("Aspect width", false)

MOP_ENDLIST(GUIWidget)
