#include "ObjectTip.h"

ObjectTip:: ObjectTip() : tips(_FL_)
{
}

ObjectTip::~ObjectTip()
{
}

void ObjectTip::Restart()
{
	Show(false);

	Activate(true);
	Show(mshow);
}

bool ObjectTip::Create		   (MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool ObjectTip::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void ObjectTip::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
	{
		SetUpdate(&ObjectTip::Draw,ML_GUI1);
	}
	else
	{
		DelUpdate(&ObjectTip::Draw);

		for( int i = 0 ; i < tips ; i++ )
		{
			TipInfo &info = tips[i];

			if( info.target.Ptr() && info.widget.Validate())
			{
				IGUIElement *widget = (IGUIElement *)info.widget.Ptr();

				widget->SetPosition(info.x,info.y);
				widget->SetSize	   (info.w,info.h);

				widget->SetFontScale(1.0f);

				widget->SetAlign(info.layout);
			}
		}
	}
}

void ObjectTip::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	//
}

void ObjectTip::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"restart"))
	{
		Restart();

		LogicDebug("Command <restart>. Restart complete.");
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

void _cdecl ObjectTip::Draw(float dltTime, long level)
{
	if( EditMode_IsOn())
	{
		if( debug == false )
			return;

		for( int i = 0 ; i < tips ; i++ )
		{
			TipInfo &info = tips[i];

			if( !info.target.Validate())
				 FindObject(info.targetName,info.target);

			if( !info.widget.Validate())
				 FindObject(info.widgetName,info.widget);

			IGUIElement *widget = (IGUIElement *)info.widget.Ptr();

			if( widget )
			{
				widget->GetNatPosition(info.x,info.y);
				widget->GetNatSize	  (info.w,info.h);

				info.layout = widget->GetNatAlign();
			}
		}
	}

	for( int i = 0 ; i < tips ; i++ )
	{
		TipInfo &info = tips[i];

		MissionObject *target =				   info.target.Ptr();
		IGUIElement	  *widget = (IGUIElement *)info.widget.Ptr();

		if( target && widget )
		{
			Matrix m; target->GetMatrix(m);

			Vector pos = Render().GetView().MulVertex(m.MulVertex(info.off));

			Vector4 p =	Render().GetProjection().Projection(pos,0.5f,0.5f);

			float x = p.v.x - 0.5f;
			float y = p.v.y;

			x /= widget->GetAspect();

			if( pos.z < 0.0f )
			{
				x = 10.0f;
				y = 10.0f;
			}

			widget->SetAlign(IGUIElement::OnCenter);
			widget->SetPosition(x,y);

			if( scale )
			{
				float k = p.w*5.0f;

				widget->SetSize(info.w*k,info.h*k);
				widget->SetFontScale(k);

				if( hmirr )
					widget->SetPosition(x,y - info.h*k);
			}
			else
			{
				if( hmirr )
					widget->SetPosition(x,y - info.h);
			}
		}
	}
}

void ObjectTip::PostCreate()
{
	for( int i = 0 ; i < tips ; i++ )
	{
		TipInfo &info = tips[i];

		FindObject(info.targetName,info.target);
		FindObject(info.widgetName,info.widget);

		IGUIElement *widget = (IGUIElement *)info.widget.Ptr();

		if( widget )
		{
			widget->GetNatPosition(info.x,info.y);
			widget->GetNatSize	  (info.w,info.h);

			info.layout = widget->GetNatAlign();
		}
	}
}

void ObjectTip::InitParams(MOPReader &reader)
{
	for( int i = 0 ; i < tips ; i++ )
	{
		TipInfo &info = tips[i];

		if( info.target.Ptr() && info.widget.Validate())
		{
			IGUIElement *widget = (IGUIElement *)info.widget.Ptr();

			widget->SetPosition(info.x,info.y);
			widget->SetSize	   (info.w,info.h);

			widget->SetFontScale(1.0f);

			widget->SetAlign(info.layout);
		}
	}

	tips.DelAll();

	long n = reader.Array();

	for( int i = 0 ; i < n ; i++ )
	{
		TipInfo &info = tips[tips.Add()];

		info.targetName = reader.String();
		info.widgetName = reader.String();

		info.off = reader.Position();

		info.target.Reset();
		info.widget.Reset();
	}

	hmirr = reader.Bool();

	scale = reader.Bool();
	debug = reader.Bool();

	Show(mshow = reader.Bool());
}

MOP_BEGINLISTCG(ObjectTip, "ObjectTips", '1.00', 100, "ObjectTips\n\n    Use to add GUI tips to mission objects", "Default")

	MOP_ARRAYBEG("Objects", 0, 100)

		MOP_STRING("Target", "")
		MOP_STRING("Widget", "")

		MOP_POSITION("Offset", 0.0f)

	MOP_ARRAYEND

	MOP_BOOL("HMirr", false)

	MOP_BOOL("Scale", false)
	MOP_BOOL("Debug", false)

	MOP_BOOL("Show", true)

MOP_ENDLIST(ObjectTip)
