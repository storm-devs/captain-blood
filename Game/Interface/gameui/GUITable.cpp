#include "GUITable.h"
#include "GUIWidget.h"

GUITable:: GUITable() :
	elems(_FL_),
	items(_FL_)
{
}

GUITable::~GUITable()
{
	if( EditMode_IsOn())
		return;

	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

		RELEASE(e.number.p);
		RELEASE(e.activity.p);
	}
}

void GUITable::Restart()
{
	//
}

bool GUITable::Create		  (MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool GUITable::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void GUITable::Show(bool isShow)
{
	if( elems && isShow != IsShow())
	{
		if( isShow )
		{
			SetUpdate(&GUITable::Update,ML_GUI1);
		}
		else
		{
			DelUpdate(&GUITable::Update);
		}
	}

	MissionObject::Show(isShow);
}

void _cdecl GUITable::Update(float dltTime, long level)
{
	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

		bool update = false;

		if( e.widget.p && e.number.p && e.widget.p->IsShow())
		{
			int val = (int)e.number.p->Get();

			if( val != e.val )
			{
				static char buf[32];
				sprintf_s(buf,sizeof(buf),"%d",val);

				const char *s = buf;

				e.widget.p->Command("SetString",1,&s);

				e.val = val;

				update = true;
			}
		}

		if( update )
		{
			if( e.time <= 0.0f )
			{
				e.from_x = items[e.item].x;
				e.from_y = items[e.item].y;
			}

			for( int j = e.item + 1 ; j < items ; j++ )
			{
				Item &t = items[j]; Element &next = elems[t.elem];

				if( e.val < next.val )
				{
					Item &c = items[j - 1];

					c.elem = t.elem; next.item = j - 1;

					if( next.time <= 0.0f )
					{
						next.from_x = t.x;
						next.from_y = t.y;
					}

					next.time = delay;

					next.to_x = c.x;
					next.to_y = c.y;

					if( next.panel.p )
						next.panel.p->ResetDrawUpdate(0);

					e.item = j; t.elem = i;

					e.time = delay;

					e.to_x = t.x;
					e.to_y = t.y;

					if( e.panel.p )
						e.panel.p->ResetDrawUpdate(1);
				}
				else
					break;
			}

			for( int j = e.item - 1 ; j >= 0 ; j-- )
			{
				Item &t = items[j]; Element &prev = elems[t.elem];

				if( e.val > prev.val )
				{
					Item &c = items[j + 1];

					c.elem = t.elem; prev.item = j + 1;

					if( prev.time <= 0.0f )
					{
						prev.from_x = t.x;
						prev.from_y = t.y;
					}

					prev.time = delay;

					prev.to_x = c.x;
					prev.to_y = c.y;

					if( prev.panel.p )
						prev.panel.p->ResetDrawUpdate(0);

					e.item = j; t.elem = i;

					e.time = delay;

					e.to_x = t.x;
					e.to_y = t.y;

					if( e.panel.p )
						e.panel.p->ResetDrawUpdate(1);
				}
				else
					break;
			}
		}

		if( e.time > 0.0f )
		{
			e.time -= dltTime;

			float k = e.time/0.5f;

			if( k < 0.0f )
				k = 0.0f;

			if( e.panel.p )
			{
				e.panel.p->MoveTo(
					Lerp(e.to_x,e.from_x,k),
					Lerp(e.to_y,e.from_y,k));
			}
		}
		else
		{
			if( e.panel.p )
			{
				const Item &item = items[e.item];

				e.panel.p->MoveTo(item.x,item.y);
			}
		}
	}
}

void GUITable::PostCreate()
{
	MOSafePointer p; MissionObject *q;

	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

		if( string::NotEmpty(e.panel.name))
		{
			FindObject(ConstString(e.panel.name),p); q = p.Ptr();

			if( q && q->Is(InterfaceUtils::GetBaseId()))
			{
				e.panel.p = (BaseGUIElement *)q;
			}
			else
			{
				LogicDebugError("GUIElement \"%s\" not found.",e.panel.name);

				e.panel.p = null;
			}
		}
		else
		{
			LogicDebugError("Elements[%d].Panel is empty.",i);

			e.panel.p = null;
		}

		if( string::NotEmpty(e.widget.name))
		{
			FindObject(ConstString(e.widget.name),p); q = p.Ptr();

			if( q && q->Is(InterfaceUtils::GetWidgetId()))
			{
				e.widget.p = (GUIWidget *)q;
			}
			else
			{
				LogicDebugError("GUIWidget \"%s\" not found.",e.widget.name);

				e.widget.p = null;
			}
		}
		else
		{
			LogicDebugError("Elements[%d].Widget is empty.",i);

			e.widget.p = null;
		}
	}

	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

		if( string::NotEmpty(e.activity.name))
		{
			LogicDebug("Elements[%d].Activity set for key '%s'", i, e.activity.name);

			e.activity.p = api->Storage().GetItemFloat(e.activity.name, _FL_);

			if (e.activity.p == null)
			{
				LogicDebugError("--Elements[%d].Activity. key in database not found!", i);
			}

		} else
		{
			LogicDebug("Elements[%d].Activity not set (don't change visibility state)", i);
			e.activity.p = null;
		}



		if( string::NotEmpty(e.number.name))
		{
			e.number.p = api->Storage().GetItemFloat(e.number.name,_FL_);
		}
		else
		{
			LogicDebugError("Elements[%d].Number is empty.",i);

			e.number.p = null;
		}
	}

	UpdateActivity();

	Show(show);
}

void GUITable::UpdateActivity()
{

	for( dword i = 0 ; i < elems.Size(); i++ )
	{
		Element &e = elems[i];

		if (e.activity.p && e.widget.p)
		{
			float fActivity = e.activity.p->Get();
			if (fActivity <= 0.000001f)
			{
				LogicDebug("Elements[%d] Hide widget, because key in database says is not active", i);
				e.widget.p->Show(false);

				//что бы при сортировке был внизу всегда, невидимый...
				e.val = -10000;
			} else
			{
				LogicDebug("Elements[%d] Show widget, because key in database says is active", i);
				e.widget.p->Show(true);
			}
		}
			 
	}


}

void GUITable::InitParams(MOPReader &reader)
{
	delay = reader.Float();

	long count = reader.Array();

	elems.DelAll();
	elems.Reserve(count);

	items.DelAll();
	items.Reserve(count);

	for( int i = 0 ; i < count ; i++ )
	{
		Element &e = elems[elems.Add()];
		Item	&t = items[items.Add()];

		e.panel.name = reader.String().c_str();

		t.x = reader.Float();
		t.y = reader.Float();

		t.elem = i;

		e.number.name = reader.String().c_str();
		e.activity.name = reader.String().c_str();

		e.val = 0;

		e.item = i;

		e.time = -1.0f;

		e.widget.name = reader.String().c_str();
	}

	show = reader.Bool();

	MissionObject::Show(false);
}

void GUITable::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( EditMode_IsOn())
		return;

	if( false )
	{
	}
	else
	{
		LogicDebugError("Invalid command name: \"%s\"",id);
	}
}

static const char *_desc =

"GUI Table\n\n"
"    Use to display game scores";

MOP_BEGINLISTCG(GUITable, "ScoreTable", '1.00', 2000, _desc, "Interface")

	MOP_FLOATEXC("Anim time", 0.5f, 0.0f, 1.0f, "Время анимации смены позиции элемента")

	MOP_ARRAYBEG("Elements", 0, 100)

		MOP_STRINGC("Panel", "", "Контейнер для элемента")

		MOP_FLOAT("X", 0.0f)
		MOP_FLOAT("Y", 0.0f)

		MOP_STRINGC("Number", "", "Имя переменной из базы")
		MOP_STRINGC("Activity", "", "Имя переменной из базы, если переменная установлена и меньше или равна нулю, не показывать данный пункт")

		MOP_STRINGC("Widget", "", "Виджет для отображения значения переменной")

	MOP_ARRAYEND

	MOP_BOOLC("Show", false, "Показать таблицу")

MOP_ENDLIST(GUITable)
