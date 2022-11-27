#include "AdvFont.h"
#include "..\Render.h"
/*
static char temp_buffer[temp_buffer_size * 2];

static float __aspect;
static float __asp;

static float font_hk;

static IVBuffer *_VB = null;
static IIBuffer *_IB = null;

static IVariable * Texture = null;
static IBaseTexture * Image;

static IRender * __render       = null;
static IControlsService *__controls = null;

static ShaderId Circular_shadow_id;
static ShaderId Circular_id;

static RenderAdvFont *_curText = null; // текущая активная подсказка
*/
RenderAdvFont::RenderAdvFont(CritSection *section, string *buffer1, string *buffer2, IVBuffer* _pVB, IIBuffer* _pIB, const char * pFontName, float fHeight, dword dwColor,
							 const char * fntShader, const char * circularShader, const char * circularShdwShader)
	: Resource(DX8TYPE_ADVANCED_FONT, 0),
	decs (_FL_),
	dech (_FL_),
	table(_FL_),
	hash (_FL_),
	cuts (_FL_), textBuffer(*buffer1),destBuffer(*buffer2)
{
//	sName = pFontName;
//	sName.Lower();

	critSection = section;

	m_font = NGRender::pRS->CreateFont(pFontName, fHeight, dwColor);

	if (fntShader == NULL)
	{
		m_font->SetTechnique("dbgFontAlpha");
	} else
	{
		m_font->SetTechnique(fntShader);
	}

	pVB = NULL;
	pIB = NULL;

	buttonsImage = NULL;
	ButtonsTexture = NULL;

	PostInit();


	m_time = 0.0f;

	m_alpha = 1.0f;

	m_fontScale = 1.0f;



	m_fakeScale = 1.0f;



	// все элементы создаются из расчета этого аспекта
	const float def_aspect = 16.0f / 9.0f;


	float cx = (float)NGRender::pRS->GetFullScreenViewPort_2D().Width;
	float cy = (float)NGRender::pRS->GetFullScreenViewPort_2D().Height;

	//      аспект разрешения экрана
	float scr_aspect = cx / cy;

	//      аспект пикселя
	float dot_aspect = NGRender::pRS->GetWideScreenAspectWidthMultipler();

	//      реальный аспект
	float cur_aspect = scr_aspect/dot_aspect;

	__aspect = def_aspect/cur_aspect;
	__asp    = cur_aspect;


	useCuts = false;
	totalDelay = 0.0f;

	drawDebug = false;

//	priority = 0;


	pVB = _pVB;
	pIB = _pIB;


	m_pointer = "";
	m_usePointer = true;


	printedChars = 0;

	Circular_shadow_id = NULL;
	Circular_id = NULL;

	////////////////////////////


	if (circularShader)
	{
		NGRender::pRS->GetShaderId(circularShader, Circular_id);
	} else
	{
		NGRender::pRS->GetShaderId("Circular", Circular_id);
	}


	if (circularShdwShader)
	{
		NGRender::pRS->GetShaderId(circularShdwShader, Circular_shadow_id);
	} else
	{
		NGRender::pRS->GetShaderId("Circular_shadow", Circular_shadow_id);
	}

	
	
	
}


RenderAdvFont::~RenderAdvFont()
{
	if (buttonsImage)
	{
	//	buttonsImage->Release();
		buttonsImage = NULL;
	}

	
	ButtonsTexture = NULL;
	
	// ВАНО добавил, иначе были фонт лики
	if (m_font)
	{
		m_font->Release();
		m_font = NULL;
	}
}



void RenderAdvFont::PostInit()
{
	srvControls = (IControlsService *)api->GetService("ControlsService");
	if (srvControls)
	{
		buttonsImage = srvControls->GetControlsImage();

		if (buttonsImage)
		{
		//	buttonsImage->AddRef();
		}
	} else
	{
		buttonsImage = NULL;
	}



	if (ButtonsTexture == NULL)
	{
		ButtonsTexture = NGRender::pRS->GetTechniqueGlobalVariable("CircularTexture",_FL_);
	}

/*
	if (Circular_shadow_id == NULL)
	{
		NGRender::pRS->GetShaderId("Circular_shadow", Circular_shadow_id);
	}

	if (Circular_id == NULL)
	{
		NGRender::pRS->GetShaderId("Circular", Circular_id);
	}
*/


	



}

void RenderAdvFont::ResetImage()
{
	if( srvControls )
	{
		buttonsImage = srvControls->GetControlsImage();

		if( buttonsImage )
		{
		//	buttonsImage->AddRef();
		}
	}
	else
	{
		buttonsImage = null;
	}
}

void RenderAdvFont::UpdateTable(string &s)
{
	if( !m_font )
	{
		return;
	}

	float fh = m_font->GetHeight();

	table.DelAll();

	decs.DelAll();
	cuts.DelAll();

//	static  char name[256];
//	string  name;
	string &name = destBuffer;

	char *buff = (char *)s.GetBuffer();

	char *line = buff;
	char *p = line;

//	IBaseTexture *image = Controls().GetControlsImage();
//	Assert(image)

//	dword imageW = image->GetWidth ();
//	dword imageH = image->GetHeight();

//	Assert(Image)
/*
//	lastImage = Image;
	m_updated = Controls().GetImagesUpdated();
*/
	dword imageW = buttonsImage ? buttonsImage->GetWidth () : 0;
	dword imageH = buttonsImage ? buttonsImage->GetHeight() : 0;

	dword cx = NGRender::pRS->GetViewport().Width;
	dword cy = NGRender::pRS->GetViewport().Height;

	textBuffer = "";

	bool def = true; // используется дефолтный цвет

	while( 1 )
	{
		p = strchr(line,'<');

		if( !p )
			break;

	//	if( p > buff && p[-1] == '>' )
	//		textBuffer += "\003"; // вставляем разрыв между идущими подряд тегами

		if( p > line )
		{
			*p = 0;

			textBuffer += line;

			*p = '<';

			line = p;
		}

		p++;

		if( *p == '<' )
		{
			textBuffer += "<";

			p++; line = p;
			continue;
		}

		while( *p && *p == ' ' ) p++;

		if( *p == '>' )
		{
			p++; line = p;
			continue;
		}

		name = "";

		for( int j = 0 ; *p && *p != '>' && *p != '=' && *p != ' ' /*&& j <= 128*/ ; )
		{
			name += *p++; j++;
		}

		if( *p == 0 )
			break;

	//	name[j] = 0;

		if( name == "nl" )
		{
			while( *p && *p != '>' ) p++;

			if( *p == 0 )
				break;

			textBuffer += "\004";

			p++; line = p;
			continue;
		}

		if( name == "/clr" )
		{
			while( *p && *p != '>' ) p++;

			if( *p == 0 )
				break;

			if( !def )
			{
				Dec &dec = decs[decs.Add()];

				dec.def = true;
				dec.col = -1;

				dec.i = -1;
				dec.j = -1;

				textBuffer += "\001";

				def = true;
			}

			p++; line = p;
			continue;
		}

		if( name == "clr" )
		{
			while( *p && *p == ' ' ) p++;

			if( *p != '=' )
			{
				while( *p && *p != '>' ) p++;

				if( *p == 0 )
					break;

				p++; line = p;
				continue;
			}

			p++;

			name = "";

			for( int j = 0 ; *p && *p != '>' /*&& j <= 128*/ ; )
			{
				name += *p++; j++;
			}

			if( *p == 0 )
				break;

		//	name[j] = 0;

			dword color = 0;

			if( sscanf_s(name.c_str(),"%x",&color) == 1 )
			{
				Dec &dec = decs[decs.Add()];

				dec.def = false;
				dec.col = color;

				dec.i = -1;
				dec.j = -1;

				textBuffer += "\001";

				def = false;
			}

			p++; line = p;
			continue;
		}

		if( name == "br" )
		{
			if( !useCuts )
			{
				while( *p && *p != '>' ) p++;

				if( *p == 0 )
					break;

				p++; line = p;
				continue;
			}

			while( *p && *p == ' ' ) p++;

			if( *p == '>' )
			{
				Cut &cut = cuts[cuts.Add()];

				cut.time = -1.0f;

				cut.i = -1;

				textBuffer += "\002";

				p++; line = p;
				continue;
			}

			if( *p != '=' )
			{
				while( *p && *p != '>' ) p++;

				if( *p == 0 )
					break;

				p++; line = p;
				continue;
			}

			p++;

			name = "";

			for( int j = 0 ; *p && *p != '>' /*&& j <= 128*/ ; )
			{
				name += *p++; j++;
			}

			if( *p == 0 )
				break;

		//	name[j] = 0;

			float time = 0;

			if( sscanf_s(name.c_str(),"%f",&time) == 1 )
			{
				Cut &cut = cuts[cuts.Add()];

				cut.time = time >= 0.0f ? time : 0.0f;

				cut.i = -1;

				textBuffer += "\002";

				def = false;
			}

			p++; line = p;
			continue;
		}

		bool img = false;
		bool anm = false;

		if( name == "img" )
		{
			img = true;
		}

		if( name == "anm" )
		{
			img = true; anm = true;
		}

		if( !img )
		{
			while( *p && *p != '>' ) p++;

			if( *p == 0 )
				break;

			p++; line = p;
			continue;
		}

		while( *p && *p == ' ' ) p++;

		if( *p != '=' )
		{
			while( *p && *p != '>' ) p++;

			if( *p == 0 )
				break;

			p++; line = p;
			continue;
		}

		p++;

		while( *p && *p == ' ' ) p++;

		name = "";

		for( int j = 0 ; *p && *p != '>' && *p != ' ' /*&& j <= 128*/ ; )
		{
			name += *p++; j++;
		}

		if( *p == 0 )
			break;

	//	name[j] = 0;

	//	Info &info = table[table.Add()];
		Info  info;

		if( srvControls )
		{
			info.p = srvControls->GetControlImagePlace(name.c_str());
		}
		else
		{
			info.p.h = 0.0f;
			info.p.u = 0.0f;
			info.p.v = 0.0f;
			info.p.w = 0.0f;
		}

		if( info.p.h )
		{
			info.anim = anm;

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
				textBuffer += "`";

			table.Add(info);
		}
		else
		{
		/*	info.w = 0.0f;
			info.h = 0.0f;

			info.k = 0.0f;*/

		//	textBuffer += "[";
			textBuffer += name;
		//	textBuffer += "]";
		}

		while( *p && *p != '>' ) p++;

		if( *p == 0 )
		{
			break;
		}

		p++; line = p;
		continue;
	}

	textBuffer += line;
}

//ВНИМАНИЕ: обязательно нужен buffer и buffer_size заполненый из UpdateTable
void RenderAdvFont::UpdateHash(string &s)
{
	if( !m_font )
		return;

	dword cx = NGRender::pRS->GetViewport().Width;
	dword cy = NGRender::pRS->GetViewport().Height;

//	float aspect = InterfaceUtils::AspectRatio(Render());

//	hash.DelAll();
//	dech.DelAll();

	dword i = 0; // текущая позиция в исходной строке
	dword j = 0; // текущая позиция в строке результата

	dword n = 0; // индекс первого символа в строке

	dword t = 0; // индекс в таблице
	dword d = 0; // номер текущей строки

	hash.Add();
	dech.Add();

	hash[d] = 0;
	dech[d] = 0;

	int cdi = 0; // текущий индекс цветового тега
	int cci = 0; // текущий индекс ката

	destBuffer = ""; const char *temp_buffer = textBuffer.c_str();

	while( 1 )
	{
		for( ;; )
		{
			while( temp_buffer[i] != '`' && temp_buffer[i] != '\n' && temp_buffer[i] != '\001' )
			{
				destBuffer += temp_buffer[i++]; j++;
			}

			if( temp_buffer[i] == '\001' )
			{
				Dec &dec = decs[cdi++];

				dec.i = d;
				dec.j = j - n;

				dech[d]++;

				i++; continue;
			}
			else
				break;
		}

		if( temp_buffer[i] == '\n' )
		{
			destBuffer += temp_buffer[i]; j++;

			if( !temp_buffer[i + 1] )
				break;
			else
			{
				i++; n = j; d++;

				hash.Add();
				dech.Add();

				hash[d] = 0;
				dech[d] = 0; continue;
			}
		}

		hash[d]++;

		Info &info = table[t++];

		info.x = m_font->GetLength(j - n,destBuffer.c_str() + n);
		info.y = m_font->GetHeight()*d;

	//	info.y += (m_font->GetHeight() - info.h*m_hk)*0.5f;
		info.y += (m_font->GetHeight() - info.h		)*0.5f;

		while( temp_buffer[i] == '`' )
			i++;

		int m;

		float width = m_font->GetLength(" ");

		if( width == 0.0f ) // символ не найден
		//	m = 1;
			m = 0;
		else
		{
		//	n = (int)round(info.w*m_hk/width);

		//	float sx = info.h/cy*m_hk*info.k;
			float sx = info.h/cy*info.k;

			m = (int)round(sx*cx/width);
		//	m = (int)ceilf(sx*cx/width);
		}

		while( m-- )
		{
			destBuffer += ' '; j++;
		}

		if( temp_buffer[i] == '\n' )
		{
			destBuffer += temp_buffer[i]; j++;

			if( !temp_buffer[i + 1] )
				break;
			else
			{
				i++; n = j; d++;

				hash.Add();
				dech.Add();

				hash[d] = 0;
				dech[d] = 0; continue;
			}
		}
	}

	s = destBuffer;
}

void RenderAdvFont::UpdateCuts()
{
	int str_len = m_params.defaultString.Len();
	int len = str_len - cuts;

	if( len < 1 )
		cuts.DelAll();

	if( cuts < 1 )
		return;

	float time = 0; // общее время показа фрагментов с заданной продолжительностью
	int count = 0;	// общее число символов в фрагментах с автоопределением времени показа

	int	prev_j = -1;

	for( int i = 0 ; i < cuts ; i++ )
	{
		Cut &cut = cuts[i];

		cut.n = (cut.j - prev_j) - 1;

		if( cut.n < 0 )
			cut.n = 0;

		if( cut.time < 0.0f ) // время показа фрагмента не задано
		{
			count += cut.n;
		}
		else
		{
			time += cut.time;
		}

		prev_j = cut.j;
	}

	int lastj = cuts.LastE().j;
	int extra = ((str_len - 1) - lastj) - 1;

	if( extra > 0 )
		count += extra;

	float auto_time = totalDelay - time;

	float defi_k; // коэффициент нормализации для фрагментов с заданным временем
	float auto_t; // время показа одного символа в фрагментах с автоопределением

	if( time > totalDelay )
	{
		defi_k = totalDelay/time;

		time = totalDelay;
		auto_time = 0.0f;

		auto_t = 0.0f;
	}
	else
	{
		defi_k = 1.0f;

		if( count )
			auto_t = auto_time/count;
		else
			;
		//	auto_t = ?
	}

	float beg = 0.0f; // время начала показа текущего фрагмента

//	api->Trace("");

	for( int i = 0 ; i < cuts ; i++ )
	{
		Cut &cut = cuts[i];

		if( cut.time < 0.0f ) // время показа фрагмента не задано
		{
			cut.time  = auto_t*cut.n;
		}
		else
		{
			cut.time *= defi_k;
		}

		cut.beg = beg;
		cut.end = beg + cut.time;

		beg = cut.end;

	//	api->Trace("%d %d %d %f %f %f",cut.i,cut.n,cut.j,cut.beg,cut.time,cut.end);
	}

//	api->Trace("");
}

bool RenderAdvFont::Prepare(bool coreThread, float w, float h, const char *text, Align hor, Align ver, bool useSafeFrame, bool copyText)
{
	if( coreThread )
		AssertCoreThread

	SyncroCode localSync(*critSection);

//	NGRender::pRS->getAdvancedFontsManager->Refresh(this);
//	PostInit();

	if( !m_font)
	{
		return false;
	}

	m_params.horizAlign = hor;
	m_params. vertAlign = ver;

	m_params.useSafeFrame = useSafeFrame;

	m_w = w;
	m_h = h;

//	учитываем внешний коэффициент (из SetScale())
	float fh = m_font->GetHeight();
	m_font->SetHeight(fh * m_fontScale);

	if( string::IsEmpty(text))
	{
		m_font->SetHeight(fh);

		m_usePointer = true;
		m_pointer = null;

		return false;
	}


	//JOKER: что бы на песи в редакторе не поломалось сделал так - а вообще разделять перенос строк (форматирование) и рендер мне кажется неправильным
	//ресайзящиеся окна с текстом внутри не сделать к примеру...
#ifndef _XBOX
	DWORD dwWidth = NGRender::pRS->GetViewport().Width;
#else
	DWORD dwWidth = NGRender::pRS->GetFullScreenViewPort_2D().Width;
#endif
	
	float outputWidth = w/100.0f*dwWidth/**__aspect*/;

	bool simple = !strchr(text,'<');
	bool single = !m_font || m_font->GetLength(text) < outputWidth;

	if( copyText )
	{
		if( simple && single )
		{
			m_font->SetHeight(fh);

			m_params.defaultString = text;

			m_usePointer = true;
			m_pointer = m_params.defaultString.c_str();

			m_linesCount = 1;

			return false;
		}
	}
	else
	{
		if( simple ) // не нужно парсить
		{
			if( single ) // одна строка
			{
				m_font->SetHeight(fh);

				m_usePointer = true;
				m_pointer = text;

				m_linesCount = 1;

				return false;
			}
		}
		else
			copyText = true;
	}

	/////////////////////////////
	/////////////////////////////

	hash.DelAll();
	dech.DelAll();

	if( copyText && !simple )
	{
		m_params.defaultString = text;

		UpdateTable(m_params.defaultString);
	}
	else
	{
		table.DelAll();

		decs.DelAll();
		cuts.DelAll();

		textBuffer = text;
	}

	m_linesCount = WordWrapString(textBuffer,m_font,outputWidth);

	if( copyText || m_linesCount > 1 )
	{
		if( !simple )
		{
		//	hash.DelAll();
		//	dech.DelAll();

			UpdateHash(m_params.defaultString);

			if( useCuts )
				UpdateCuts();
		}
		else
		{
			if( copyText )
			{
				if( m_linesCount > 1 )
				{
					m_params.defaultString = textBuffer;
				}
				else
				{
					m_font->SetHeight(fh);

					m_params.defaultString = text;

					m_usePointer = true;
					m_pointer = m_params.defaultString.c_str();

					return false;
				}
			}
			else
			{
				m_params.defaultString = textBuffer;
			}
		}

		copyText = true;
	}

	/////////////////////////////
	/////////////////////////////

	m_font->SetHeight(fh);

	//============================================================================

	m_usePointer = !copyText;

	if( m_usePointer )
		m_pointer = text;

	if( copyText )
		return m_params.defaultString != text;
	else
		return false;
}

void RenderAdvFont::SetFakeScale(float k)
{
	m_fakeScale = k;
}

void RenderAdvFont::Draw(bool coreThread, float x, float y, bool animate, Effect *eff)
{
	if( coreThread )
		AssertCoreThread

	SyncroCode localSync(*critSection);

	if( m_usePointer )
	{
		if( m_pointer == null )
			return;
	}

	float fh = m_font->GetHeight();

	m_font->SetHeight(fh * m_fontScale);

	dword cx = NGRender::pRS->GetViewport().Width;
	dword cy = NGRender::pRS->GetViewport().Height;

	float width = m_w /** __aspect*/;
	float height = m_h;

	width *= m_fakeScale;
	height *= m_fakeScale;

	float textX = cx * x / 100.0f;
	float textY = cy * y / 100.0f;

	float outputWidth  = cx * width / 100.0f;
	float outputHeight = cy * height / 100.0f;

	float textHeight = m_font->GetHeight() * m_linesCount;

	switch( m_params.vertAlign )
	{
	case IAdvFont::Center:
		textY += (outputHeight - textHeight)/2;
		break;

	case IAdvFont::Bottom:
		textY += (outputHeight - textHeight);
		break;
	}

	//// дефолтный цвет ////

	Color c = m_params.color;

//	c.a *= m_fadeAlpha*GetAlpha();
	c.a *= m_alpha;

	m_font->SetColor(c);

	////////////////////////

	if( m_usePointer )
	{
		float lineWidth = m_font->GetLength(m_pointer);

		switch( m_params.horizAlign )
		{
			case IAdvFont::Center:
				textX += (outputWidth - lineWidth)/2;
				break;

			case IAdvFont::Right:
				textX += (outputWidth - lineWidth);
				break;
		}

		if( eff )
		{
		//	char  line[256]; crt_strcpy(line,256,m_pointer);
			char *line = (char *)m_pointer; // указатель на string в GUIWidget

			if( eff->Beg > 0 )
			{
				char d = line[eff->Beg];

				line[eff->Beg] = 0;

				m_font->Print(textX,textY,line);

				line[eff->Beg] = d;
			}

			if( eff->Beg + eff->Len < strlen(line))
			{
				m_font->Print(textX + eff->Width,textY,line + eff->Beg + eff->Len);
			}
		}
		else
		{
			m_font->Print(textX,textY,m_pointer);
		}

		m_strOffX = textX;
		m_strOffY = textY + m_font->GetHeight()/2;

		m_font->SetHeight(fh);

		return;
	}

	dword  strBegin  = 0;
	dword lineNumber = 0;

	Vertex *p = (Vertex *)pVB->Lock(0, 0, LOCK_DISCARD);
	Vertex *q = p + icon_max_count / 2 * 4;

	int n = 0;
	int z = 0;

	for( dword i = 0 ; i < m_params.defaultString.Size() ; i++ )
	{
		if( m_params.defaultString[i] == '\n' )
		{
			m_params.defaultString[i] = 0;

			char *line = (char *)m_params.defaultString.GetBuffer() + strBegin;
			float lineWidth = m_font->GetLength(line);

			textX = cx * (x/100.0f);

			switch( m_params.horizAlign )
			{
			case IAdvFont::Center:
				textX += (outputWidth - lineWidth)/2;
				break;

			case IAdvFont::Right:
				textX += (outputWidth - lineWidth);
				break;
			}

			bool draw = false; float y_off = 0.0f;

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
				float font_h = m_font->GetHeight();

				if( useCuts && playCuts )
				{
					float time = m_cutsTime;

					int cci = -1; // текущий отображаемый фрагмент

					if( cuts )
					{
						const Cut &last = cuts.LastE();

						int line_i = (int)lineNumber;

						if( line_i >= last.i )
						{
							if( time >= last.end &&
								time <  totalDelay )
							{
								switch( m_params.vertAlign )
								{
									case IAdvFont::Top:
										y_off -= font_h*last.i;
										break;

									case IAdvFont::Center:
										y_off -= (outputHeight - textHeight)/2;
										y_off -= font_h*last.i;
										y_off += (outputHeight - font_h*(m_linesCount - last.i))/2;
										break;

									case IAdvFont::Bottom:
										;
										break;
								}

								draw = true; cci = cuts;
							}
						}
						else
						{
							for( int j = 0 ; j < cuts ; j++ )
							{
								int prev = j ? cuts[j - 1].i : 0;

								const Cut &cut = cuts[j];

								if( line_i >= prev &&
									line_i <  cut.i )
								{
									if( time >= cut.beg &&
										time <  cut.end )
									{
										switch( m_params.vertAlign )
										{
											case IAdvFont::Top:
												y_off -= font_h*prev;
												break;

											case IAdvFont::Center:
												y_off -= (outputHeight - textHeight)/2;
												y_off -= font_h*prev;
												y_off += (outputHeight - font_h*(cut.i - prev))/2;
												break;

											case IAdvFont::Bottom:
												y_off += font_h*(m_linesCount - cut.i);
												break;
										}

										draw = true; cci = j;
									}

									break;
								}
							}
						}
					}
					else
					{
						if( time < totalDelay )
							draw = true;
					}

				/*	//// обработка приоритетов ////

					if( draw )
					{
						if( _curText )
						{
							if( _curText != this )
							{
								if( _curText->GetPriority() < priority )
									_curText = this;

								draw = false;
							}
						}
						else
						{
							_curText = this;
						}
					}
					else
					{
						if( _curText && _curText == this )
						{
							_curText = null;
						}
					}

					///////////////////////////////*/

					if( draw && drawDebug )
					{
						const RENDERVIEWPORT &vp = NGRender::pRS->GetViewport();

						float _x = x*vp.Width /100.0f;
						float _y = y*vp.Height/100.0f;

						const dword nor = 0xb0ffffff;
						const dword sel = 0xffff0000;

						if( cuts )
						{
							if( cci >= 0 )
							{
								float e = totalDelay - cuts.LastE().end;

								for( int j = 0 ; j < cuts ; j++ )
								{
									float t = cuts[j].time;

									dword c = j == cci ? sel : nor;

									if( j <= cci )
									{
										NGRender::pRS->Print(_x,_y + 17*j,c,"%1.2f:%1.2f",
											t,Clampf(time - cuts[j].beg,0.0f,cuts[j].time));
									}
									else
									{
										NGRender::pRS->Print(_x,_y + 17*j,c,"%1.2f:",
											t);
									}
								}

								if( e > 0.0f )
								{
									dword c = cci < cuts ? nor : sel;

									if( cci < cuts )
									{
										NGRender::pRS->Print(_x,_y + 17*j,c,"%1.2f",
											e);
									}
									else
									{
										NGRender::pRS->Print(_x,_y + 17*j,c,"%1.2f:%1.2f",
											e,time - cuts.LastE().end);
									}
								}
							}
						}
						else
						{
							NGRender::pRS->Print(_x,_y,sel,"%1.2f:%1.2f",
								totalDelay,time);
						}
					}
				}
				else
					draw = true;

				if( draw )
				{
					int dn = dech ? dech[lineNumber] : 0;

					if( dn )
					{
						float x = textX;
						float y = textY + lineNumber*m_font->GetHeight() + y_off;

						int base = 0;

						for( int j = 0 ; j < dn ; j++ )
						{
							const Dec &dec = decs[z++];

							int pos = dec.j;

							if( pos - base > 0 )
							{
								char d = line[pos];

								line[pos] = 0;

								m_font->Print(x,y,line + base);

								x += m_font->GetLength(line + base);

								line[pos] = d;

								base = pos;
							}

							Color col(dec.col); col.a *= c.a;

							m_font->SetColor(dec.def ? c : col.GetDword());
						}

						if( line[base] )
						{
							m_font->Print(x,y,line + base);
						}
					}
					else
					{
						m_font->Print(textX,textY + lineNumber*m_font->GetHeight() + y_off,line);
					}
				}
				else
				{
					int dn = dech ? dech[lineNumber] : 0;

					if( dn )
					{
						for( int j = 0 ; j < dn ; j++ )
						{
							const Dec &dec = decs[z++];

							Color col(dec.col); col.a *= c.a;

							m_font->SetColor(dec.def ? c : col.GetDword());
						}
					}
				}
			}

			if( draw )
			{
				m_strOffX = textX;
				m_strOffY = textY + m_font->GetHeight()/2 + y_off;

				if( table )
				{
					for( int j = 0 ; j < hash[lineNumber] ; j++ )
					{
						if( n > icon_max_count / 2 )
						{
							break;
						}

						Info &info = table[n++];

						float xx = (textX + info.x) / cx;
						float yy = (textY + y_off + info.y) / cy;

						xx = xx * 2.0f - 1.0f;
						yy = 1.0f - yy * 2.0f;

						//      float sy = info.h/cy*2*m_hk;
						float sy = info.h / cy * 2;

						//      float sx = info.w/cx*2*m_hk;
						float sx = sy * info.k;

						float dx = 0.0f;
						float dy = 0.0f;

						float ak = 1.0f;

						//      if( animate )
						if( animate && info.anim )
							/*      {
							float k = 1.0f + (1.0f - cosf(m_time))*0.5f*0.2f;

							float tx = sx*k;
							float ty = sy*k;

							xx -= (tx - sx)*0.5f;
							yy += (ty - sy)*0.5f;

							sx = tx;
							sy = ty;
							}*/
						{
							float k = (1.0f - cosf(m_time)) * 0.5f * 0.1f;

							//      ak = 1.0f - k*10.0f;
							ak = 0.6f - k * 10.0f * 0.4f;

							float tx = sx * (1.0f + k);
							float ty = sy * (1.0f + k);

							//      xx -= (tx - sx)*0.5f + k*sx;
							//      yy += (ty - sy)*0.5f + k*sy;
							dx  = (tx - sx) * 0.5f + k * sx;
							dy  = (ty - sy) * 0.5f + k * sy;

							sx = tx;
							sy = ty;
						}

						ImagePlace & pl = info.p;

						float _l = pl.u; float _r = _l + pl.w;
						float _t = pl.v; float _b = _t + pl.h;

						q[0].p = Vector(xx, yy - sy, 0.0f);
						q[1].p = Vector(xx ,yy, 0.0f);
						q[2].p = Vector(xx + sx, yy, 0.0f);
						q[3].p = Vector(xx + sx, yy - sy, 0.0f);

						q[0].tu = _l; q[0].tv = _b;
						q[1].tu = _l; q[1].tv = _t;
						q[2].tu = _r; q[2].tv = _t;
						q[3].tu = _r; q[3].tv = _b;

						q[0].al = c.a * ak;//m_fadeAlpha;
						q[1].al = c.a * ak;//m_fadeAlpha;
						q[2].al = c.a * ak;//m_fadeAlpha;
						q[3].al = c.a * ak;//m_fadeAlpha;

						q += 4;
						xx -= dx;
						yy += dy;

						p[0].p = Vector(xx, yy - sy, 0.0f);
						p[1].p = Vector(xx, yy, 0.0f);
						p[2].p = Vector(xx + sx, yy, 0.0f);
						p[3].p = Vector(xx + sx, yy - sy, 0.0f);

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
			}

			m_params.defaultString[i] = '\n';

			strBegin = i + 1;
			lineNumber++;
		}
	}

	pVB->Unlock();


	m_font->SetHeight(fh);

	printedChars = n;


	if( printedChars > 0 )
	{
		NGRender::pRS->SetStreamSource(0, pVB);
		NGRender::pRS->SetIndices(pIB, 0);

		if( buttonsImage && ButtonsTexture )
		{
			ButtonsTexture->SetTexture(buttonsImage);
		}

		//////////////////////////////////////////////////////////////////

		//NGRender::pRS->GetShaderId("Circular_shadow", Circular_shadow_id);
		//NGRender::pRS->GetShaderId("Circular", Circular_id);

		//////////////////////////////////////////////////////////////////

		NGRender::pRS->DrawIndexedPrimitive(Circular_shadow_id, PT_TRIANGLELIST, icon_max_count / 2 * 4, printedChars * 4, icon_max_count / 2 * 6, printedChars * 2);

		NGRender::pRS->DrawIndexedPrimitive(Circular_id, PT_TRIANGLELIST, 0, printedChars * 4, 0, printedChars * 2);
	}
}

void RenderAdvFont::Print(bool coreThread, float relative_x, float relative_y, float relative_w, float relative_h, const char *text, Align hor, Align ver, bool useSafeFrame, bool animate, Effect * eff)
{
	Prepare(coreThread, relative_w, relative_h, text, hor, ver, useSafeFrame);
	Draw(coreThread, relative_x, relative_y, animate, eff);
}


void RenderAdvFont::Update(float time)
{
	m_time = time;
}

void RenderAdvFont::SetScale(float k)
{
	m_fontScale = k;
}

void RenderAdvFont::SetColor(const Color &c)
{
	m_params.color = c;
}

void RenderAdvFont::SetAlpha(float a)
{
	m_alpha = a;
}

void RenderAdvFont::GetPos(float &x, float &y)
{
	x = m_strOffX;
	y = m_strOffY;
}

dword RenderAdvFont::WordWrapString(string &buffer, IFont *font, float outputPixelWidth)
{
	if( !font )
	{
		return 0;
	}

	char *text = buffer.GetDataBuffer();

	dword linesCount = 1;

	float outputWidth = outputPixelWidth;

	dword potentialWrapPos = 0;     // возможное место переноса
	dword  prevRealWrapPos = 0;     // место где был сделан последний перенос

	dword strLength = strlen(text);

	bool just_br = true;

	int cci = 0; // текущий индекс ката

	for( dword i = 0 ; i < strLength ; i++ )
	{
		if( text[i] == '\004' )	// тег переноса строки
		{
		//	if( just_br )
		//		continue;

			text[i] = 0;

			float length = font->GetLength(text + prevRealWrapPos);

			text[i] = '\004';

			if( length >= outputWidth )
			{
				linesCount++;

			//	if( potentialWrapPos == 0 )
				if( potentialWrapPos == prevRealWrapPos )
				{
					potentialWrapPos = i;

					text[potentialWrapPos] = '\n';
				}
				else
				{
					if( i - potentialWrapPos > 1 )
					{
						text[potentialWrapPos] = '\n';

						linesCount++;

						potentialWrapPos = i;

						text[potentialWrapPos] = '\n';
					}
					else
					{
						potentialWrapPos = i;

						text[potentialWrapPos] = '\n';
					}
				}					

				prevRealWrapPos = potentialWrapPos;
			}
			else
			{
				prevRealWrapPos = potentialWrapPos = i;

				linesCount++;

				text[potentialWrapPos] = '\n';
			}

			just_br = true;
		}
		else
		if( text[i] == '\002' )	// тег разрыва строки
		{
			Cut &cut = cuts[cci++];

			cut.i = linesCount - 1;
			cut.j = i;

			if( just_br )
				continue;

			cut.i++;

			text[i] = 0;

			float length = font->GetLength(text + prevRealWrapPos);

			text[i] = '\002';

			if( length >= outputWidth )
			{
				linesCount++;

			//	if( potentialWrapPos == 0 )
				if( potentialWrapPos == prevRealWrapPos )
				{
					potentialWrapPos = i;

					text[potentialWrapPos] = '\n';
				}
				else
				{
					if( i - potentialWrapPos > 1 )
					{
						cut.i++;

						text[potentialWrapPos] = '\n';

						linesCount++;

						potentialWrapPos = i;

						text[potentialWrapPos] = '\n';
					}
					else
					{
						potentialWrapPos = i;

						text[potentialWrapPos] = '\n';
					}
				}					

				prevRealWrapPos = potentialWrapPos;
			}
			else
			{
				prevRealWrapPos = potentialWrapPos = i;

				linesCount++;

				text[potentialWrapPos] = '\n';
			}

			just_br = true;
		}
		else
		if( text[i] == ' ' )
		{
			if( just_br )
				continue;

			text[i] = 0;

			float length = font->GetLength(text + prevRealWrapPos);

			text[i] = ' ';

			if( length >= outputWidth )
			{
			//	if( potentialWrapPos == 0 )
				if( potentialWrapPos == prevRealWrapPos )
				{
					potentialWrapPos = i;
					continue;
				}

				linesCount++;

				text[potentialWrapPos] = '\n';

				prevRealWrapPos = potentialWrapPos;

			//	just_br = true;
			}

			potentialWrapPos = i;
		}
		else
			just_br = false;
	}

	if( text[i] != ' ' )
	{
		// обработка переноса последней строки
		float length = font->GetLength(text + prevRealWrapPos);

	//	if( length >= outputWidth )
		if( length >= outputWidth && (text[i] || potentialWrapPos))
		{
		/*	if( potentialWrapPos == 0 )
			{
				potentialWrapPos = i;
			}

			linesCount++;

			text[potentialWrapPos] = '\n';*/
			if( potentialWrapPos > prevRealWrapPos )
			{
				linesCount++;

				text[potentialWrapPos] = '\n';
			}
		}
	}

	buffer += "\n";

	return linesCount;
}









bool RenderAdvFont::Release()
{
	if (Resource.Release()) return ForceRelease();
	return false;
}

bool RenderAdvFont::ForceRelease()
{
	Resource.ForceRelease(); 
	NGRender::pRS->getAdvancedFontsManager()->Release(this);
	delete this;
	return true;
}

void  RenderAdvFont::SetKerning (float kerningValue)
{
	if (m_font)
	{
		m_font->SetKerning(kerningValue);
	}

}