#include "CircularBar.h"
#include "..\utils\InterfaceUtils.h"

const int segment_count = 32;

CircularBar:: CircularBar()
{
	buffer	= null;
	texture = null;

	m_drawPriority = 0;

	Circular_id = null;
	BarTexture	= null;
}

CircularBar::~CircularBar()
{
	RELEASE(buffer)

	RELEASE(texture)

	BarTexture = NULL;
}

void CircularBar::Restart()
{
	ReCreate();
}

bool CircularBar::Create(MOPReader &reader)
{
	if( !BarTexture )
		 BarTexture = Render().GetTechniqueGlobalVariable("CircularTexture",_FL_);

	if( !Circular_id )
		 Render().GetShaderId("Circular",Circular_id);

	if( !buffer )
		 CreateBuffer();

	InitParams(reader);

	Show(true);

	return true;
}

bool CircularBar::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void CircularBar::Activate(bool isActive)
{
	//
}

void CircularBar::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&CircularBar::Draw,ML_GUI1 + m_drawPriority);
	else
		DelUpdate(&CircularBar::Draw);

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}

void _cdecl CircularBar::Draw(float dltTime, long level)
{
	if( InterfaceUtils::IsHide())
		return;

	float m_w = this->m_w*m_aspect;

	float p = m_pos/m_max*segment_count;
	int	  n = ffloor(p);
	float u = p - n;

	Vertex *v = (Vertex *)buffer->Lock(0,0,LOCK_DISCARD);

	float b = 0.0f;

	for( int i = 0 ; i < n + 2 ; i++, b += 1.0f )
	{
		if( i > segment_count )
			break;

		float a = 2*PI*i/segment_count;

		float x = sinf(a);
		float y = cosf(a);

		if( i > n )
		{
			a = 2*PI*(i - 1)/segment_count;

			float d = sinf(a);
			float e = cosf(a);

			x = Lerp(d,x,u);
			y = Lerp(e,y,u);

		//	a = 2*PI*(i - 1 + u)/segment_count;

			b = b - 1.0f + u;
		}

		v->p = Vector(
			x*m_outer*m_w + (m_x + m_w*0.5f)*2.0f - 1.0f,
			y*m_outer*m_h + 1.0f - (m_y + m_h*0.5f)*2.0f,0.0f);

		v->tu = b;
		v->tv = 0.0f;

		v->al = 1.0f;

		v++;

		v->p = Vector(
			x*m_inner*m_w + (m_x + m_w*0.5f)*2.0f - 1.0f,
			y*m_inner*m_h + 1.0f - (m_y + m_h*0.5f)*2.0f,0.0f);

		v->tu = b;
		v->tv = 1.0f;

		v->al = 1.0f;

		v++;
	}

	buffer->Unlock();

	if( n > 0 )
	{
		Render().SetStreamSource(0,buffer);

		if( BarTexture && texture )
			BarTexture->SetTexture(texture);

		if( n <= segment_count && u > 0.0f )
			n++;

		Render().DrawPrimitive(Circular_id,
			PT_TRIANGLESTRIP,0,n*2);
	}
}

void CircularBar::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"set_max"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <set_max> error. Not enought parameters.");
			return;
		}

		m_max = (float)atof(params[0]);

		LogicDebug("Command <set_max>. Max pos = %.",m_max);
	}
	else
	if( string::IsEqual(id,"set_pos"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <set_pos> error. Not enought parameters.");
			return;
		}

		m_pos = (float)atof(params[0]);

		if( m_pos > m_max )
			m_pos = m_max;

		LogicDebug("Command <set_pos>. Cur pos = %.",m_pos);
	}
	else
	if( string::IsEqual(id,"restart"))
	{
		Restart();
		LogicDebug("Command <restart>. Bar was restarted.",m_pos);
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

void CircularBar::SetPos(float val)
{
	m_pos = val;

	if( m_pos > m_max )
		m_pos = m_max;
}

void CircularBar::CreateBuffer()
{
//	if( buffer )
//		buffer->Release();

	buffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*(segment_count + 1)*2,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(buffer)
}

void CircularBar::InitAspect()
{
	//	все элементы создаются из расчета этого аспекта
	const float def_aspect = 16.0f/9.0f;

	float cx;
	float cy;

	if( EditMode_IsOn())
	{
	//	cx = (float)Render().GetViewport().Width;
	//	cy = (float)Render().GetViewport().Height;
		cx = (float)Render().GetFullScreenViewPort_2D().Width;
		cy = (float)Render().GetFullScreenViewPort_2D().Height;
	}
	else
	{
		cx = (float)Render().GetFullScreenViewPort_2D().Width;
		cy = (float)Render().GetFullScreenViewPort_2D().Height;
	}

//	аспект разрешения экрана
	float scr_aspect = cx/cy;

//	api->Trace("");
//	api->Trace("    WINDOW: asp = %f res = %.0fx%.0f",scr_aspect,cx,cy);

//	аспект пикселя
	float dot_aspect = InterfaceUtils::AspectRatio(Render());

//	api->Trace("       DOT: asp = %f",dot_aspect);
//	api->Trace("");

//	реальный аспект
	float cur_aspect = scr_aspect/dot_aspect;

//	m_aspect		= def_aspect/cur_aspect;
//	m_aspect_native = cur_aspect;

	m_aspect = def_aspect/cur_aspect;
}

void CircularBar::InitParams(MOPReader &reader)
{
//	m_aspect = InterfaceUtils::AspectRatio(Render());
	InitAspect();

	const char *t = reader.String().c_str();

	if( !texture || !string::IsEqual(texture->GetName(),t))
	{
		if( texture )
			texture->Release();

	//	texture = Render().CreateTexture(_FL_,t);
		texture = Render().CreateTextureFullQuality(_FL_,t);
	}

	m_x = reader.Float()*0.01f;
	m_y = reader.Float()*0.01f;

	m_w = reader.Float()*0.01f;
	m_h = reader.Float()*0.01f;

	m_inner = reader.Float();
	m_outer = reader.Float();

	m_max = reader.Float();
	m_pos = reader.Float();

	if( m_pos > m_max )
		m_pos = m_max;

	m_drawPriority = reader.Long();
}

MOP_BEGINLISTCG(CircularBar, "CircularBar", '1.00', 100, "CircularBar\n\n    Circular progress bar\n\nAviable commands list:\n\n    set_max - set max value\n\n        param[0] - new value\n\n    set_pos - set position\n\n        param[0] - new value", "Interface")

	MOP_STRING("texture", "")

	MOP_FLOAT("x", 0.0f)
	MOP_FLOAT("y", 0.0f)

	MOP_FLOAT("w", 1.0f)
	MOP_FLOAT("h", 1.0f)

	MOP_FLOAT("inner", 0.7f)
	MOP_FLOAT("outer", 1.0f)

	MOP_FLOAT("max", 1.0f)
	MOP_FLOAT("pos", 0.0f)

	MOP_LONG("Draw priority", 0)

MOP_ENDLIST(CircularBar)
