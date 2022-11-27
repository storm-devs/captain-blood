#include "BaseGUIElement.h"
#include "BaseGUICursor.h"

MOP_BEGINLIST(GUIFocusHolder, "", '1.00', 0)
MOP_ENDLIST(GUIFocusHolder)

#include "../utils/InterfaceUtils.h"

BaseGUIElement:: BaseGUIElement(void) : m_children(_FL_)
{
	m_parent.Reset();

	m_focusHolder.Reset();

	nativeShow = false; drawLevel = ML_GUI2;
	m_clipping = false;

	m_scale = -1.0f; // по умолчанию скейл не используется

	m_offsetX = 0.0f;
	m_offsetY = 0.0f;

	m_isLooped = false;

	//// дефолтные значения, должны переопределяться ////

	m_aspect		= 1.0f;
	m_aspect_native = 1.0f;

	m_natPos.x = 0.0f;
	m_natPos.y = 0.0f;

	m_natSize.w = 0.0f;
	m_natSize.h = 0.0f;

	m_natLay = OnLeft;

	/////////////////////////////////////////////////////

	m_shiftAspect = true;
	m_widthAspect = true;
	m_widthAspectKoef = 1.0f;

	m_layout = OnCenter;

	m_restricts = preserveNone;

	m_fadeState = FadingFinished;

	/////

	memset(&m_params, 0, sizeof(m_params));
	m_params.parent.Empty();

	m_baseCursor = null;

	data_a = -1;
	data_b = -1;
}

BaseGUIElement::~BaseGUIElement(void)
{
	NotifyChildren(ParentDeleted);

	if( m_focusHolder.Validate() && ((GUIFocusHolder *)m_focusHolder.Ptr())->GetFocused() == this )
		SetElementInFocus(null);

	if( BaseGUIElement *p = GetParent())
		p->UnRegister(this);
}

void BaseGUIElement::InitAspect(void)
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

	m_aspect		= def_aspect/cur_aspect;
	m_aspect_native = cur_aspect;
}

void BaseGUIElement::SkipMOPs(MOPReader &reader)
{
	reader.Enum();		// layout

	reader.Float();		// x
	reader.Float();		// y

	m_params.alpha = reader.Float();

	reader.String().c_str();	// parent name
}

void BaseGUIElement::ReadMOPs(MOPReader &reader)
{
	m_parent.Reset();

	InitAspect();

//	bool shift = reader.Bool();
	bool shift = true;

//	bool width = reader.Bool();
	bool width = true;

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
	{
		m_shiftAspect = shift;
		m_widthAspect = width;

		m_layout = layout;
	}

	m_natLay = layout;

	float x = reader.Float();
	float y = reader.Float();

	if( !Restricted(preservePos))
	{
		m_params.x = x;
		m_params.y = y;
	}

	m_natPos.x = x;
	m_natPos.y = y;

	m_params.alpha = reader.Float();

	
	ConstString s = reader.String();

	if( !Restricted(preserveParent))
		m_params.parent = s;

	m_offsetX = 0.0f;
	m_offsetY = 0.0f;

	data_a = -1;
	data_b = -1;
}

bool BaseGUIElement::Create			(MOPReader &reader)
{
	ReadMOPs(reader);

	return true;
}

bool BaseGUIElement::EditMode_Update(MOPReader &reader)
{
	ReadMOPs(reader);

	return true;
}

void BaseGUIElement::PostCreate()
{
	MOSafePointer sp;

	FindObject(InterfaceUtils::GetCursorName(),sp);

	m_baseCursor = (BaseGUICursor *)sp.Ptr();
}

BaseGUIElement *BaseGUIElement::FindParent()
{
	if( FindObject(m_params.parent,m_parent))
	{
		if( !m_parent.Ptr()->Is(InterfaceUtils::GetBaseId()))
			 m_parent.Reset();
	}

	return (BaseGUIElement *)m_parent.Ptr();
}
/*
void BaseGUIElement::GetRect(Rect &rect, bool useOffset) const
{
	float parent_w = 100.0f;

	if( m_parent )
	{
		m_parent->GetRect(rect,useOffset); parent_w = m_parent->GetWidth();
	}
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}

	float width = rect.r - rect.l;

	float x = m_params.x;
	float y = m_params.y;

	if( useOffset )
	{
		x += GetOffsetX();
		y += GetOffsetY();
	}

	float w = m_params.w;
	float h = m_params.h;

	if( m_widthAspect )
	{
		w /= m_aspect;

		switch( m_layout )
		{
			case OnCenter:
				x = x + 0.5f*(m_params.w - w);
				x += rect.l;
				break;

			case OnLeft:
				if( m_shiftAspect )
					x /= m_aspect;
				x += rect.l;
				break;

			case OnRight:
				x = parent_w - (x + m_params.w);
				if( m_shiftAspect )
					x = rect.r - x/m_aspect;
				else
					x = rect.r - x;
				x -= w;
				break;
		}
	}
	else
	{
		if( m_shiftAspect )
		{
			switch( m_layout )
			{
				case OnCenter:
					x /= m_aspect;
				//	w = parent_w - (parent_w - w)/m_aspect;
					w = width - (parent_w - w)/m_aspect;
					x += rect.l;
					break;

				case OnLeft:
					w = x + w - x/m_aspect;
					x /= m_aspect;
					x += rect.l;
					break;

				case OnRight:
					w = parent_w - (parent_w - (x + w))/m_aspect - x;
					x = rect.r - (parent_w - (x + m_params.w))/m_aspect - w;
					break;
			}
		}
		else
		{
			x += rect.l;
		}
	}

	rect.l  = x;
	rect.t += y;

	rect.r = rect.l + w;
	rect.b = rect.t + h;
}
*/
void BaseGUIElement::GetRect(Rect &rect, bool useOffset)// const
{
	if( EditMode_IsOn())
		ValidateParent();

	float px = 0.0f;
	float py = 0.0f;

	BaseGUIElement *parent = (BaseGUIElement *)m_parent.Ptr();

	if( parent )
	{
		parent->GetRect(rect,useOffset);
	//	parent->GetRect(rect);

		px = rect.l;
		py = rect.t;
	}
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}

/*	float x = m_params.x*m_aspect;
	float y = m_params.y;*/

	float x;
	float y;

	GetPosition(x,y);

	x *= 100.0f*m_aspect*m_widthAspectKoef;
	y *= 100.0f;

	if( useOffset )
	{
		x += GetOffsetX();
		y += GetOffsetY();
	}

	float w = m_params.w*m_aspect*m_widthAspectKoef;
	float h = m_params.h;

	if( parent )
	{
		float scale = parent->m_scale;

	//	if( scale != 1.0f )
		if( scale >= 0.0f )
		{
			x *= scale; w *= scale;
			y *= scale; h *= scale;

			SetFontScale(scale);
		}
	}

	float center = (rect.l + rect.r)*0.5f;

	switch( m_layout )
	{
		case OnLeft:
			x = rect.l + x;
			break;

		case OnCenter:
			x = center + x - w*0.5f;
			break;

		case OnRight:
			x = rect.r - x - w;
			break;
	}

	rect.l  = x;
	rect.t += y;

	rect.r = rect.l + w;
	rect.b = rect.t + h;

	if( parent && parent->m_isLooped )
	{
		float ox = x - px;
		float oy = y;

		Rect rt; parent->GetPanelRect(rt);

		float cntx = 0.5f*(rt.l + rt.r);
		float cnty = 0.5f*(rt.t + rt.b);

		if( rect.l > cntx )
		{
			float l = rect.r - parent->GetFullW();

			if( rect.l - cntx > cntx - l )
			{
				rect.r = l; rect.l = rect.r - w;
			}
		}
		else
		if( rect.r < cntx )
		{
			float r = rect.l + parent->GetFullW();

			if( cntx - rect.r > r - cntx )
			{
				rect.l = r; rect.r = rect.l + w;
			}
		}

		if( rect.t > cnty )
		{
			float t = rect.b - parent->GetFullH();

			if( rect.t - cnty > cnty - t )
			{
				rect.b = t; rect.t = rect.b - h;
			}
		}
		else
		if( rect.b < cnty )
		{
			float b = rect.t + parent->GetFullH();

			if( cnty - rect.b > b - cnty )
			{
				rect.t = b; rect.b = rect.t + h;
			}
		}
	}
}

void BaseGUIElement::GetParentRect(Rect &rect, bool useOffset)// const
{
	BaseGUIElement *p = (BaseGUIElement *)m_parent.Ptr();

	if( p )
		p->GetRect(rect,useOffset);
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}
}

void BaseGUIElement::GetPanelRect(Rect &rect)
{
	if( EditMode_IsOn())
		ValidateParent();

	BaseGUIElement *p = (BaseGUIElement *)m_parent.Ptr();

	if( p )
	{
	//	p->GetRect(rect,useOffset);
		p->GetRect(rect);
	}
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}

	float x = m_params.x*m_aspect;
	float y = m_params.y;

/*	if( useOffset )
	{
		x += GetOffsetX();
		y += GetOffsetY();
	}*/

	float w = m_params.w*m_aspect;
	float h = m_params.h;

	float center = (rect.l + rect.r)*0.5f;

	switch( m_layout )
	{
		case OnLeft:
			x = rect.l + x;
			break;

		case OnCenter:
			x = center + x - w*0.5f;
			break;

		case OnRight:
			x = rect.r - x - w;
			break;
	}

	rect.l  = x;
	rect.t += y;

	rect.r = rect.l + w;
	rect.b = rect.t + h;
}

void BaseGUIElement::OnParentNotify(Notification event)
{
	switch( event )
	{
		case ParentShowed:
			Show(nativeShow);
			break;

		case ParentHided:
		{
			bool show = nativeShow;

			Show(false);

			nativeShow = show;

		} break;

		case ParentDeleted:
			if( BaseGUIElement *p = GetParent())
			{
				p->UnRegister(this);

				m_parent.Reset();
			}
			break;

		case SilentShow:
		//	if( GetNativeAlpha() > 0.0f )
			if( IsShow())
			{
				SetDrawUpdate();
				NotifyChildren(SilentShow);
			}
			break;

		case SilentHide:
		//	if( GetNativeAlpha() > 0.0f )
			if( IsShow())
			{
				DelDrawUpdate();
				NotifyChildren(SilentHide);
			}
			break;
	}
}

void BaseGUIElement::NotifyChildren(Notification event)
{
	for( dword i = 0; i < m_children.Size() ; i++ )
	{
		if( m_children[i].object.Validate())
			((BaseGUIElement *)m_children[i].object.Ptr())->OnParentNotify(event);
	}
}

void BaseGUIElement::NotifyParent(BaseGUIElement *p, ChildNotification event)
{
	BaseGUIElement *parent = (BaseGUIElement *)m_parent.Ptr();

	if( parent )
		parent->OnChildNotify(p,event);
}

void BaseGUIElement::Show(bool isShow)
{
	bool prevShow = IsShow();

	if( isShow )
	{
		LogicDebug("Show");
	}
	else
	{
		LogicDebug("Hide");
	}

	nativeShow = isShow;

	if( nativeShow )
	{
		const BaseGUIElement *parent = FindParent();
		bool  parentVisible = true;

		if( parent )
			parentVisible = parent->IsShow();

		isShow = isShow && parentVisible;

		if( isShow == false )
		{
			LogicDebug("Stay invisible: parent hided");
		}
	}

	MissionObject::Show(isShow);

//	for( dword i = 0 ; i < m_children.Size() ; i++ )
//		m_children[i].object->Show(isShow);

//	NotifyChildren(isShow ? ParentShowed : ParentHided);
	if( isShow )
	{
		if( !prevShow )
			NotifyChildren(ParentShowed);
	}
	else
	{
		if( prevShow )
			NotifyChildren(ParentHided);
	}
}

void BaseGUIElement::Activate(bool isActive)
{
	if( isActive )
	{
		LogicDebug("Activate");
	}
	else
	{
		LogicDebug("Deactivate");
	}

	MissionObject::Activate(isActive);

	for( int i = 0 ; i < m_children ; i++ )
	{
		((BaseGUIElement *)m_children[i].object.Ptr())->Activate(isActive);
	}
}

float BaseGUIElement::GetAlpha()
{
	float alpha = m_params.alpha;

	if( BaseGUIElement *p = GetParent())
		alpha *= p->GetAlpha();

	return alpha;
}

bool BaseGUIElement::ParentIsActive()
{
	if( BaseGUIElement *p = GetParent())
	{
		if( !p->IsActive())
			return false;
		else
			return p->ParentIsActive();
	}

	return true;
}

void BaseGUIElement::SetElementInFocus(BaseGUIElement *p)
{
	GetFocusHolder()->SetFocused(p);
}

BaseGUIElement *BaseGUIElement::GetElementInFocus()
{
	return GetFocusHolder()->GetFocused();
}

GUIFocusHolder *BaseGUIElement::GetFocusHolder()
{
	GUIFocusHolder *holder = (GUIFocusHolder *)m_focusHolder.Ptr();

	if( !holder )
	{
		static const ConstString objName("GUIFocusHolder");
		Mission().CreateObject(m_focusHolder,"GUIFocusHolder",objName);

		holder = (GUIFocusHolder *)m_focusHolder.Ptr();
		Assert(holder)
	}

	return holder;
}

void BaseGUIElement::  Register(BaseGUIElement* object)
{
	Assert(object)

	if( object != this )
	{
		for( int i = 0 ; i < m_children ; i++ )
			if( m_children[i].object.Ptr() == object )
				break;

		if( i >= m_children )
		{
			m_children.Add(object);
		}
	}
}

void BaseGUIElement::UnRegister(BaseGUIElement* object)
{
	Assert(object)

//	m_children.Del(object);
	for( int i = 0 ; i < m_children ; i++ )
	{
		if(((BaseGUIElement *)m_children[i].object.Ptr()) == object )
		{
			m_children.DelIndex(i); break;
		}
	}
}

void BaseGUIElement::SetParent(const ConstString & parent)
{
	BaseGUIElement *oldParent = GetParent();

	m_params.parent = parent;

	BaseGUIElement *newParent = FindParent();

	if( newParent != oldParent )
	{
		if( oldParent )
			oldParent->UnRegister(this);

		if( newParent )
			newParent->  Register(this);
	}
}

float BaseGUIElement::GetX(bool useOffset)// const
{
	Rect r; GetRect(r,useOffset);

	return r.l;
}

float BaseGUIElement::GetY(bool useOffset)// const
{
	Rect r; GetRect(r,useOffset);

	return r.t;
}

static LONG round(float x)
{
	float f = floorf(x);

	if( x - f > 0.5f )
		return LONG(f + 1.0f);
	else
		return LONG(f);
}
/*
void BaseGUIElement::SetClipRect(Rect *r)
{
	if( !r )
	{
		Render().SetStateHack(SH_SCISSORRECT,0);
		return;
	}

	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	const RENDERVIEWPORT vp = render->GetViewport();

	RECT rect;

	rect.left	= vp.X + round(r->l*0.01f*vp.Width );
	rect.top	= vp.Y + round(r->t*0.01f*vp.Height);
	rect.right	= vp.X + round(r->r*0.01f*vp.Width );
	rect.bottom = vp.Y + round(r->b*0.01f*vp.Height);

	Render().SetStateHack(SH_SCISSORRECT,1);
	Render().SetScissorRect(&rect);
}
*/
void BaseGUIElement::SetClipRect(Rect *r)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	const RENDERVIEWPORT vp = render->GetViewport();

	RECT rect;

	if( !r )
	{
		rect.left	= vp.X;
		rect.top	= vp.Y;
		rect.right	= vp.X + vp.Width;
		rect.bottom = vp.Y + vp.Height;

		Render().SetScissorRect(&rect);

	//	Render().SetStateHack(SH_SCISSORRECT, 0);
		Render().SetStateHack(SH_SCISSORRECT,-1);

		return;
	}

	rect.left	= vp.X + round(r->l*0.01f*vp.Width );
	rect.top	= vp.Y + round(r->t*0.01f*vp.Height);
	rect.right	= vp.X + round(r->r*0.01f*vp.Width );
	rect.bottom = vp.Y + round(r->b*0.01f*vp.Height);

	Render().SetStateHack(SH_SCISSORRECT,1);
	Render().SetScissorRect(&rect);
}
/*
void BaseGUIElement::GetClipRect(Rect &rect)
{
	if( m_parent && ValidatePointer(m_parent,m_parentHash))
		m_parent->GetClipRect(rect);
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}

	if( m_clipping )
	{
		float pw = m_params.w;
		float ph = m_params.h;

		float px = m_params.x;
		float py = m_params.y;

		px = px + 0.5f*pw*(1.0f - 1.0f/m_aspect);

		float x = px;
		float y = py;

		float w = pw/m_aspect;
		float h = ph;

		if( x > 0 )
			rect.l += x;

		if( y > 0 )
			rect.t += y;

		float dx = rect.r - rect.l - w;
		float dy = rect.b - rect.t - h;

		if( dx > 0 )
			rect.r -= dx;

		if( dy > 0 )
			rect.b -= dy;
	}
}
*/
void BaseGUIElement::GetClipRect(Rect &rect)
{
	if( BaseGUIElement *p = GetParent())
		p->GetClipRect(rect);
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}

	if( m_clipping )
	{
		Rect r; GetRect(r,false);

		if( rect.l < r.l )
			rect.l = r.l;

		if( rect.t < r.t )
			rect.t = r.t;

	/*	float dx = rect.r - rect.l - (r.r - r.l);
		float dy = rect.b - rect.t - (r.b - r.t);

		if( dx > 0 )
			rect.r -= dx;

		if( dy > 0 )
			rect.b -= dy;*/

		if( rect.r > r.r )
			rect.r = r.r;

		if( rect.b > r.b )
			rect.b = r.b;

		if( rect.r < rect.l )
			rect.r = rect.l;

		if( rect.b < rect.t )
			rect.b = rect.t;
	}
}

void BaseGUIElement::GetParentClipRect(Rect &rect)
{
	BaseGUIElement *parent = (BaseGUIElement *)m_parent.Ptr();

	if( parent )
		parent->GetClipRect(rect);
	else
	{
		rect.l = 0.0f; rect.r = 100.0f;
		rect.t = 0.0f; rect.b = 100.0f;
	}
}

bool BaseGUIElement::PointInRect(float x, float y, const Rect &r)
{
	if( x < r.l	) return false;
	if( x > r.r ) return false;
	
	if( y < r.t	) return false;
	if( y > r.b ) return false;

	return true;
}

bool BaseGUIElement::RectInRect(
	float l1, float t1, float r1, float b1,
	float l2, float t2, float r2, float b2)
{
	float rx1 = (r1 - l1)*0.5f;
	float ry1 = (b1 - t1)*0.5f;

	float rx2 = (r2 - l2)*0.5f;
	float ry2 = (b2 - t2)*0.5f;

	float dx = fabsf(l1 + rx1 - (l2 + rx2));
	float dy = fabsf(t1 + ry1 - (t2 + ry2));

	return dx <= rx1 + rx2 &&
		   dy <= ry1 + ry2;
}

BaseGUIElement::Rect BaseGUIElement::GetFullRect()
{
	Rect rect; GetRect(rect,false);

	float x = rect.l;
	float y = rect.t;

	rect.r -= rect.l; rect.l = 0.0f;
	rect.b -= rect.t; rect.t = 0.0f;

	for( int i = 0; i < m_children ; i++ )
	{
		if( m_children[i].object.Validate())
		{
			Rect r; ((BaseGUIElement *)m_children[i].object.Ptr())->GetRect(r,false);

			r.l -= x; r.r -= x;
			r.t -= y; r.b -= y;

			if( rect.l > r.l )
				rect.l = r.l;

			if( rect.t > r.t )
				rect.t = r.t;

			if( rect.r < r.r )
				rect.r = r.r;

			if( rect.b < r.b )
				rect.b = r.b;
		}
	}

	return rect;
}

void BaseGUIElement::ValidateParent()
{
	if( !m_parent.Validate())
	{
		 FindObject(m_params.parent,m_parent);
	}
}

void BaseGUIElement::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"setfxa"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command BaseGUIElement::[setfxa] error. No alpha specified.");
			return;
		}

		bool visible = GetAlpha() > 0.0f;

		float alpha = (float)atof(params[0]);

		SetAlpha(alpha);

		if( alpha > 0.0f )
		{
			if( IsShow() && !visible )
			{
				SetDrawUpdate();
				NotifyChildren(SilentShow);
			}
		}
		else
		{
			if( IsShow() &&  visible )
			{
				DelDrawUpdate();
				NotifyChildren(SilentHide);
			}
		}
	}
	else
	if( string::IsEqual(id,"setcursor"))
	{
	/*	if( m_baseCursor )
		{
			Rect r; GetRect(r);

			m_baseCursor->SetPosition(
				0.5f*0.01f*(r.l + r.r),
				0.5f*0.01f*(r.t + r.b));
		}*/
		SetCursor();
	}
	else
	if( string::IsEqual(id,"restart"))
	{
		Restart();
	}
}

void BaseGUIElement::SetCursor()
{
	if( m_baseCursor )
	{
		Rect r; GetRect(r);

		m_baseCursor->SetPosition(
			0.5f*0.01f*(r.l + r.r),
			0.5f*0.01f*(r.t + r.b));
	}
}
