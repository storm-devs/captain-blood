#include "BaseGUIText.h"
#include "../utils/InterfaceUtils.h"

const int icon_max_count = 16;

const dword buffer_size = 1024;
static char buffer[buffer_size*2];

static float __aspect;
static float __asp;

static float font_hk;

static IVBuffer *_VB = null;
static IIBuffer *_IB = null;

static IVariable	*Texture = null;
static IBaseTexture *Image;

static IRender			*__render	= null;
static IControlsService *__controls = null;

static ShaderId Circular_shadow_id;
static ShaderId Circular_id;

static int _refCount = 0;

struct Vertex
{
	Vector p;

	float tu;
	float tv;

	float al;
};

/////////////////////////////////

class GUIText : public IGUIText
{
	struct Params
	{
		string defaultString;

		Align horizAlign;
		Align  vertAlign;

		bool useSafeFrame;

		Color color;

		Params()
		{
			horizAlign = Left;
			 vertAlign = Top;

			useSafeFrame = true;

			color = (dword)-1;
		}
	};

	Params m_params;

	struct Info
	{
		ImagePlace p;

		float x; float w;
		float y; float h;

		float k;

		bool anim;
	};

	array<Info> table;
	array<long> hash;

private:

	float SC2UIRC(float t)
	{
		return m_params.useSafeFrame ? InterfaceUtils::ScreenCoord2UIRectCoord(t) : t;
	}

	float SS2UIRS(float size)
	{
		return m_params.useSafeFrame ? InterfaceUtils::ScreenSize2UIRectSize(size) : size;
	}

	void UpdateTable(string &s);
	void UpdateHash (string &s);

public:

	GUIText() : table(_FL_),
				hash (_FL_)
	{
		m_font = null;
		m_time = 0.0f;

		m_alpha = 1.0f;

		m_fontScale = 1.0f;
	}

	virtual bool Prepare(IFont *font, float w, float h, const char *text,
		Align hor,
		Align ver, bool useSafeFrame);

	virtual void Draw	(IFont *font, float x, float y,
		bool animate,
		Effect  *eff);

	virtual void Update(float time)
	{
		m_time = time;
	}

	virtual void SetScale(float k)
	{
		m_fontScale = k;
	}

	virtual void SetColor(const Color &c)
	{
		m_params.color = c;
	}

	virtual void SetAlpha(float a)
	{
		m_alpha = a;
	}

	virtual void GetPos(float &x, float &y)
	{
		x = m_strOffX;
		y = m_strOffY;
	}

	virtual void Release()
	{
		Assert(_refCount > 0)

		if( !--_refCount )
		{
			RELEASE(_VB)
			RELEASE(_IB)

			Texture = NULL;
		}

		delete this;
	}

private:

	IFont *m_font;

	dword m_linesCount;

	float m_fontScale;

	float m_w;
	float m_h;

	float m_strOffX;
	float m_strOffY;

	float m_time;

	float m_alpha;

};

void GUIText::Draw(IFont *font, float x, float y, bool animate, Effect *eff)
{
	m_font = font;

	if( !m_font )
		return;

	float fh = m_font->GetHeight();

	m_font->SetHeight(fh*m_fontScale);

	dword cx = __render->GetViewport().Width;
	dword cy = __render->GetViewport().Height;

	float width	 = m_w*__aspect;
	float height = m_h;

	float textX = cx*SC2UIRC(x/100.0f);
	float textY = cy*SC2UIRC(y/100.0f);

	float outputWidth  = cx*SS2UIRS(width /100.0f);
	float outputHeight = cy*SS2UIRS(height/100.0f);

	float textHeight = m_font->GetHeight()*m_linesCount;

	switch( m_params.vertAlign )
	{
		case Center:
			textY += (outputHeight - textHeight)/2;
			break;

		case Bottom:
			textY += (outputHeight - textHeight);
			break;
	}

	dword  strBegin  = 0;
	dword lineNumber = 0;

	Vertex *p = (Vertex *)_VB->Lock(0,0,LOCK_DISCARD);
	Vertex *q = p + icon_max_count/2*4;

	int n = 0;

	for( dword i = 0; i < m_params.defaultString.Len(); i++ )
	{
		if( m_params.defaultString[i] == '\n' )
		{
			m_params.defaultString[i] = 0;

			char *line = (char *)m_params.defaultString.GetBuffer() + strBegin;
			float lineWidth = m_font->GetLength(line);

			textX = cx*SC2UIRC(x/100.0f);

			switch( m_params.horizAlign )
			{
				case Center:
					textX += (outputWidth - lineWidth)/2;
					break;

				case Right:
					textX += (outputWidth - lineWidth);
					break;
			}
			
			Color c = m_params.color;
			
		//	c.a *= m_fadeAlpha*GetAlpha();
			c.a *= m_alpha;

			m_font->SetColor(c);

			if( eff )
			{
				float y = textY + lineNumber*m_font->GetHeight();
				
				if( eff->Beg > 0 )
				{
					char d = line[eff->Beg];

					line[eff->Beg] = 0;

					m_font->Print(textX,y,line);

					line[eff->Beg] = d;
				}

				if( eff->Beg + eff->Len < strlen(line))
				{
					m_font->Print(textX + eff->Width,y,line + eff->Beg + eff->Len);
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

				//	if( animate )
					if( animate && info.anim )
				/*	{
						float k = 1.0f + (1.0f - cosf(m_time))*0.5f*0.2f;

						float tx = sx*k;
						float ty = sy*k;

						xx -= (tx - sx)*0.5f;
						yy += (ty - sy)*0.5f;

						sx = tx;
						sy = ty;
					}*/
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

	_VB->Unlock();

	if( n )
	{
		__render->SetStreamSource(0,_VB);
		__render->SetIndices(_IB,0);

		if( Image )
			Texture->SetTexture(Image);

		__render->DrawIndexedPrimitive(Circular_shadow_id, 
			PT_TRIANGLELIST,icon_max_count/2*4,n*4,icon_max_count/2*6,n*2);

		__render->DrawIndexedPrimitive(Circular_id,
			PT_TRIANGLELIST,0,n*4,0,n*2);
	}

	m_font->SetHeight(fh);
}

bool GUIText::Prepare(IFont *font, float w, float h, const char *text, Align hor, Align ver, bool useSafeFrame)
{
	if( !font )
		return false;

	m_params.defaultString = text;

	m_params.horizAlign = hor;
	m_params. vertAlign = ver;

	m_params.useSafeFrame = useSafeFrame;

	m_w = w;
	m_h = h;

	m_font = font;

	UpdateTable(m_params.defaultString);

	m_linesCount = InterfaceUtils::WordWrapString(
		buffer,font,w/100.0f*__render->GetViewport().Width*__aspect,buffer_size);

	UpdateHash (m_params.defaultString);

	return m_params.defaultString != text;
}

void GUIText::UpdateTable(string &s)
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
	Image = __controls->GetControlsImage();
/*
//	lastImage = Image;
	m_updated = Controls().GetImagesUpdated();
*/
	dword imageW = Image ? Image->GetWidth () : 0;
	dword imageH = Image ? Image->GetHeight() : 0;

	dword cx = __render->GetViewport().Width;
	dword cy = __render->GetViewport().Height;

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

		info.p = __controls->GetControlImagePlace(name);

		if( info.p.h )
		{
			info.anim = anim;
			
			info.w = imageW*info.p.w;//*0.4f;
			info.h = imageH*info.p.h;//*0.4f;

			info.w *= fh/info.h;
			info.h  = fh;

			info.k = info.w/info.h/__asp;

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

static float round(float x)
{
	float f = floorf(x);

	if( x - f > 0.5f )
		return f + 1.0f;
	else
		return f;
}

void GUIText::UpdateHash(string &s)
{
	if( !m_font )
		return;

	dword cx = __render->GetViewport().Width;
	dword cy = __render->GetViewport().Height;

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
}

/////////////////////////////////

GUITextManager:: GUITextManager()
{
	//
}

GUITextManager::~GUITextManager()
{
//	Assert(!_refCount)
}

IGUIText *GUITextManager::CreateText()
{
	if( !_refCount++ )
	{
		InitParams();
		CreateBuffers();
	}

	return NEW GUIText();
}

bool GUITextManager::Create(MOPReader &reader)
{
//	CreateBuffers();

	return true;
}

void GUITextManager::Activate(bool isActive)
{
	//
}

void GUITextManager::Show(bool isShow)
{
	//
}

void GUITextManager::Command(const char *id, dword numParams, const char **params)
{
	//
}

void GUITextManager::InitParams()
{
	if( __render )
		return;

	__render   = (IRender *)		 api->GetService("DX9Render");
	__controls = (IControlsService *)api->GetService("ControlsService");

//	все элементы создаются из расчета этого аспекта
	const float def_aspect = 16.0f/9.0f;

	float cx;
	float cy;

	if( EditMode_IsOn())
	{
		cx = (float)__render->GetFullScreenViewPort_2D().Width;
		cy = (float)__render->GetFullScreenViewPort_2D().Height;
	}
	else
	{
		cx = (float)__render->GetFullScreenViewPort_2D().Width;
		cy = (float)__render->GetFullScreenViewPort_2D().Height;
	}

//	аспект разрешения экрана
	float scr_aspect = cx/cy;

//	аспект пикселя
	float dot_aspect = InterfaceUtils::AspectRatio(*__render);

//	реальный аспект
	float cur_aspect = scr_aspect/dot_aspect;

	__aspect = def_aspect/cur_aspect;
	__asp	 = cur_aspect;

	/////////////////////////////////

	IFileService *storage = (IFileService *)api->GetService("FileService");
	Assert(storage)

	IIniFile *ini = storage->SystemIni();

	if( ini )
	{
		font_hk = ini->GetFloat("Controls","IconMult",1.0f);
	}
	else
		font_hk = 1.0f;

	//////////////////////////////////

	__render->GetShaderId("Circular_shadow", Circular_shadow_id);
	__render->GetShaderId("Circular", Circular_id);
}

void GUITextManager::CreateBuffers()
{
	_VB = __render->CreateVertexBuffer(
		sizeof(Vertex)*icon_max_count*4,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(_VB)

	_IB = __render->CreateIndexBuffer(
		sizeof(WORD)  *icon_max_count*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(_IB)

	WORD *p = (WORD *)_IB->Lock();
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

	_IB->Unlock();

	Texture = __render->GetTechniqueGlobalVariable("CircularTexture",_FL_);
}

MOP_BEGINLIST(GUITextManager, "", '1.00', 100)
MOP_ENDLIST(GUITextManager)
