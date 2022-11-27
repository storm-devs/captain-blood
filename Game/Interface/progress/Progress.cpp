#include "Progress.h"
#include "..\utils\InterfaceUtils.h"

Progress:: Progress()
{
	buffer = null;

	m_drawPriority = 0;

	texture1 = null;
	texture2 = null;

	Circular_id = null;
	BarTexture	= null;
}

Progress::~Progress()
{
	RELEASE(buffer)

	RELEASE(texture1)
	RELEASE(texture2)

	BarTexture = NULL;
}

void Progress::Restart()
{
	ReCreate();
}

bool Progress::Create(MOPReader &reader)
{
	if( !BarTexture )
		 BarTexture = Render().GetTechniqueGlobalVariable("CircularTexture",_FL_);

	if( !Circular_id )
		 Render().GetShaderId("Circular",Circular_id);

	if( !buffer )
		 CreateBuffer();

	InitParams(reader);

	return true;
}

bool Progress::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void Progress::Activate(bool isActive)
{
	//
}

void Progress::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&Progress::Draw,ML_GUI1 - 1 + m_drawPriority);
	else
		DelUpdate(&Progress::Draw);

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}

void _cdecl Progress::Draw(float dltTime, long level)
{
	if( InterfaceUtils::IsHide())
		return;

	float m_x = this->m_x*m_aspect;

	float t = m_pos/m_max;
	float p = t*m_w;

	Vertex *v = (Vertex *)buffer->Lock(0,0,LOCK_DISCARD);

	v->p = Vector(
		(m_x + 0.0f)*2.0f - 1.0f,
		1.0f - (m_y + 0.0f)*2.0f,0.0f);

	v->tu = 0.0f;
	v->tv = 0.0f;

	v->al = 1.0f;

	v++;

	v->p = Vector(
		(m_x + 0.0f)*2.0f - 1.0f,
		1.0f - (m_y + m_h)*2.0f,0.0f);

	v->tu = 0.0f;
	v->tv = 1.0f;

	v->al = 1.0f;

	v++;

	v->p = Vector(
		(m_x + p)*2.0f - 1.0f,
		1.0f - (m_y + 0.0f)*2.0f,0.0f);

	v->tu = t;
	v->tv = 0.0f;

	v->al = 1.0f;

	v++;

	v->p = Vector(
		(m_x + p)*2.0f - 1.0f,
		1.0f - (m_y + m_h)*2.0f,0.0f);

	v->tu = t;
	v->tv = 1.0f;

	v->al = 1.0f;

	v++;

	v->p = Vector(
		(m_x + m_w)*2.0f - 1.0f,
		1.0f - (m_y + 0.0f)*2.0f,0.0f);

	v->tu = 1.0f;
	v->tv = 0.0f;

	v->al = 1.0f;

	v++;

	v->p = Vector(
		(m_x + m_w)*2.0f - 1.0f,
		1.0f - (m_y + m_h)*2.0f,0.0f);

	v->tu = 1.0f;
	v->tv = 1.0f;

	v->al = 1.0f;

	v++;

	buffer->Unlock();

	Render().SetStreamSource(0,buffer);

	if( texture1 )
	{
		if( BarTexture )
			BarTexture->SetTexture(texture1);

		Render().DrawPrimitive(Circular_id, 
			PT_TRIANGLESTRIP,0,1*2);
	}

	if( texture2 )
	{
		if( BarTexture )
			BarTexture->SetTexture(texture2);

		Render().DrawPrimitive(Circular_id,
			PT_TRIANGLESTRIP,2,1*2);
	}
}

void Progress::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"set_max"))
	{
		if( numParams < 2 )
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
		if( numParams < 2 )
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

void Progress::SetPos(float val)
{
	m_pos = val;

	if( m_pos > m_max )
		m_pos = m_max;
}

void Progress::CreateBuffer()
{
//	if( buffer )
//		buffer->Release();

	buffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*(3)*2,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(buffer)
}

void Progress::InitAspect()
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

void Progress::InitParams(MOPReader &reader)
{
	
//	m_aspect = InterfaceUtils::AspectRatio(Render());
	InitAspect();
	
	const char *t = reader.String().c_str();

	if( !texture1 || !string::IsEqual(texture1->GetName(),t))
	{
		if( texture1 )
			texture1->Release();

	//	texture1 = Render().CreateTexture(_FL_,t);
		texture1 = Render().CreateTextureFullQuality(_FL_,t);
	}

	t = reader.String().c_str();

	if( !texture2 || !string::IsEqual(texture2->GetName(),t))
	{
		if( texture2 )
			texture2->Release();

	//	texture2 = Render().CreateTexture(_FL_,t);
		texture2 = Render().CreateTextureFullQuality(_FL_,t);
	}

	m_x = reader.Float()*0.01f;
	m_y = reader.Float()*0.01f;

	m_w = reader.Float()*0.01f;
	m_h = reader.Float()*0.01f;

	m_max = reader.Float();
	m_pos = reader.Float();

	if( m_pos > m_max )
		m_pos = m_max;

	m_drawPriority = reader.Long();

	Show(reader.Bool());
}

MOP_BEGINLISTCG(Progress, "Progress", '1.00', 100, "", "Interface")

	MOP_STRING("texture1", "")
	MOP_STRING("texture2", "")

	MOP_FLOAT("x", 0.0f)
	MOP_FLOAT("y", 0.0f)

	MOP_FLOAT("w", 1.0f)
	MOP_FLOAT("h", 1.0f)

	MOP_FLOAT("max", 1.0f)
	MOP_FLOAT("pos", 0.0f)

	MOP_LONG("Draw priority", 0)

	MOP_BOOL("Show", true)

MOP_ENDLIST(Progress)
