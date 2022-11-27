#include "GUICharacter.h"
#include "../utils/InterfaceUtils.h"

static char ALIGN_CENTER[] = "Center";
static char ALIGN_LEFT	[] = "Left";
static char ALIGN_RIGHT [] = "Right";
static char ALIGN_TOP	[] = "Top";
static char ALIGN_BOTTOM[] = "Bottom";

static char PLAY_CONTINIOUS		   [] = "Continious";
static char PLAY_ONCE_ON_ACTIVATION[] = "Once on activation";
static char PLAY_ONCE_ON_SHOW	   [] = "Once on show";
static char PLAY_ONCE_BY_COMMAND   [] = "Once by command";

GUICharacter::GUICharacter(void)
{
	m_asp = 4.0f/3.0f;

	m_restart = false;
}

GUICharacter::~GUICharacter(void)
{
}

float GUICharacter::SC2UIRC(float t)
{
	return m_params.useSafeFrame ? InterfaceUtils::ScreenCoord2UIRectCoord(t) : t;
}

float GUICharacter::SS2UIRS(float size)
{
	return m_params.useSafeFrame ? InterfaceUtils::ScreenSize2UIRectSize(size) : size;
}

inline float frac(float f)
{
	return f - floor(f);
}

static float round(float x)
{
	float f = floorf(x);

	if( x - f > 0.5f )
		return f + 1.0f;
	else
		return f;
}

void GUICharacter::Begin(float deltaTime, long)
{
	if( EditMode_IsOn())
	{
		if( !EditMode_IsVisible())
			return;

		ValidateParent();
	}

	Rect r; GetRect(r);

	float cx = r.r - r.l;
	float cy = r.b - r.t;

	if( m_debug || EditMode_IsSelect())
		m_quad.DrawQuad(r.l,r.t,cx,cy,0.0f,0.0f,1.0f,1.0f,10.0f + 0.5f);

	m_view = Render().GetView();
	m_proj = Render().GetProjection();	

	m_vp = Render().GetViewport();

	dword sw = m_vp.Width;
	dword sh = m_vp.Height;

	r.l *= 0.01f; r.r *= 0.01f;
	r.t *= 0.01f; r.b *= 0.01f;

	cx *= 0.01f;
	cy *= 0.01f;

	float ww = cx*sw;
	float wh = cy*sh;

	RENDERVIEWPORT vp = {
		m_vp.X + (dword)round(r.l*sw),
		m_vp.Y + (dword)round(r.t*sh),(dword)round(ww) + 1,(dword)round(wh) + 1,0.0f,1.0f};

	Render().SetViewport(vp);

	Matrix view(m_ang,m_pos);
	Matrix proj;

	Render().SetView(view);

	if( m_persp )
	{
		float k = InterfaceUtils::AspectRatio(Render());

		proj.BuildProjection(m_fov,ww/k,wh,0.1f,10.0f);
	}

//	api->Trace("%f %f",m_aspect,m_aspect_native);

	Render().SetProjection(proj);
}

void GUICharacter::End  (float deltaTime, long)
{
	Render().SetView	  (m_view);
	Render().SetProjection(m_proj);

	Render().SetViewport(m_vp);
}

void GUICharacter::ReadMOPs(MOPReader &reader)
{
	string enumStr;

	m_params.useSafeFrame = reader.Bool();

	float width	 = reader.Float();
	float height = reader.Float();

	if( !Restricted(preserveSize))
		SizeTo(width,height);

//	m_persp = reader.Bool();
	m_persp = true;

	m_debug = reader.Bool();

	m_fov = reader.Float()/180.0f*PI;

	m_ang = reader.Angles();
	m_pos = reader.Position();

	nativeShow = reader.Bool();
//	Show(reader.Bool());

	MissionObject::Activate(true);
}

void _cdecl GUICharacter::InitFunc(float, long)
{
	DelUpdate(&GUICharacter::InitFunc);

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

	Show(nativeShow);
}

void GUICharacter::OnParentNotify(Notification event)
{
	BaseGUIElement::OnParentNotify(event);
}

void GUICharacter::Restart()
{
	m_restart = true;

	ReCreate();
}

bool GUICharacter::Create		   (MOPReader &reader)
{
	if( !m_restart )
	{
		Render().GetShaderId("Circular_shadow", Circular_shadow_id);
		Render().GetShaderId("Circular", Circular_id);
	}

	EditMode_Update(reader);

	return true;
}

bool GUICharacter::EditMode_Update(MOPReader &reader)
{
	BaseGUIElement::EditMode_Update(reader);
	ReadMOPs(reader);

//	m_asp = m_aspect > 1.0f ? 4.0f/3.0f : 16.0f/9.0f;
	m_asp = m_aspect_native;

	NotifyChildren(ParentChanged);

//	SetUpdate(&GUICharacter::InitFunc,ML_FIRST);
	SetUpdate(&GUICharacter::InitFunc,ML_FIRST + 2);

	return true;
}

void GUICharacter::Show(bool isShow)
{
	BaseGUIElement::Show(isShow);

	if( EditMode_IsOn())
	{
		DelDrawUpdate();
	}

	if( IsShow())
	{
		SetDrawUpdate();
	}
	else
	{
		DelDrawUpdate();
	}
}

void GUICharacter::Activate(bool isActive)
{
	BaseGUIElement::Activate(isActive);
}

static char GUIDescription[] =

"GUI Character wrapper\n\n"
"    Use to place character into GUI layout\n"
"    NOTE: all sizes-positions are relative (not pixel sizes-positions)\n\n"
"Commands:\n\n"
"    none";

MOP_BEGINLISTCG(GUICharacter, "GUI Character", '1.00', 1000, GUIDescription, "Interface")

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

//	MOP_BOOL("Perspective", true)
	MOP_BOOL("Debug draw", false)

	MOP_FLOATEX("FOV", 45.0f, 20.0f, 70.0f)

	MOP_ANGLES("Ang", Vector(0.0f,0.0f,0.0f))
	MOP_POSITION("Pos", Vector(0.0f,0.0f,0.5f))

	MOP_BOOL("Visible", true)

MOP_ENDLIST(GUICharacter)
