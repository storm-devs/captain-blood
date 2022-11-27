#include "BaseGUITable.h"
#include "GUIWidget.h"


BaseGUITable::Element::Element()
{
	widget.name = null;
	widget.p = null;
	number.name = null;
	number.p = null;
	title.name = null;
	title.p = null;
	delay = 0.0f;
	rel = false;
	play = false;
}



BaseGUITable:: BaseGUITable() : elems(_FL_)
{
	tickSound = null;
	sound = null; fadeOut = 0.0f;
}

BaseGUITable::~BaseGUITable()
{
	if( EditMode_IsOn())
		return;

	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

		RELEASE(e.number.p)
	}

	RELEASE(sound);
	RELEASE(tickSound);
}

void BaseGUITable::Restart()
{
	//
}

bool BaseGUITable::Create		  (MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool BaseGUITable::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void BaseGUITable::Show(bool isShow)
{
	if(!IsShow())
	{
		if( isShow )
		{
			const char *diff_name = "Profile.Global.Difficulty";

			ICoreStorageString *d = api->Storage().GetItemString(diff_name,_FL_);

			if( d && d->IsValidate())
			{
				const char *diff = d->Get();

				if( string::NotEmpty(diff))
				{
					SetTable(diff);

					RELEASE(d)

					SetUpdate(&BaseGUITable::Update,ML_GUI1);
				}
				else
				{
					LogicDebugError("Invalid game difficulty value.");

					RELEASE(d) return;
				}
			}
			else
			{
				LogicDebugError("Base variable \"%s\" not found.",diff_name);

				RELEASE(d) return;
			}
		}
	}
	else
	{
		if(!isShow )
			DelUpdate(&BaseGUITable::Update);
	}

	MissionObject::Show(isShow);
}

void BaseGUITable::SetTable(const char *diff)
{
	index = 0; time = 0.0f; wait = false; skip = false;

	ICoreStorageFloat *r;

	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

		RELEASE(e.number.p)

		char buf[128]; const char *name;

		if( string::NotEmpty(e.number.name))
		{
			if( e.rel )	// смотрим переменную в разделе текущей сложности
			{
				sprintf_s(buf,sizeof(buf),"Profile.%s.%s",diff,e.number.name);

				name = buf;
			}
			else		// абсолютный путь до переменной
			{
				name = e.number.name;
			}

			r = api->Storage().GetItemFloat(name,_FL_);

			if( r && r->IsValidate())
			{
				e.number.p = r;
			}
			else
			{
				LogicDebugError("Base variable \"%s\" not found.",name);

				e.number.p = null;

				RELEASE(r)
			}
		}
		else
		{
			LogicDebugError("Elements[%d].Number is empty.",i);

			e.number.p = null;
		}
	}
}

void _cdecl BaseGUITable::Update(float dltTime, long level)
{
	do
	{
		if( index < elems )
		{
			if( skip == false )
			{
				if( Controls().GetControlStateType(menu) == CST_ACTIVATED )
					skip = true;
			}

			const Element &e = elems[index];

			if( wait )
			{
			//	if( Controls().GetControlStateType(menu) == CST_ACTIVATED )
			//		e.widget.p->Activate(true);

				tickPeriodicTime -= dltTime;
				if (tickSoundName.NotEmpty() && tickPeriodic && tickPeriodicTime<=0.0f)
				{
					tickPeriodicTime += tickCooldown;
					Sound().Create(tickSoundName.c_str(), _FL_, true, true);
				}

				if( skip )
					e.widget.p->Activate(true);

				if( skip || e.widget.p->Complete())
				{
					if( sound )
						sound->FadeOut(fadeOut);

					if (tickSound && !tickPeriodic)
						tickSound->FadeOut(fadeOut);

					if (tickEndSoundName.NotEmpty())
						Sound().Create(tickEndSoundName.c_str(), _FL_, true, true);

					index++; time = 0.0f;

					wait = false;

					if( index >= elems )
					{
						complete.Activate(Mission(),false);
					}
				}
			}
			else
			{
				if( e.widget.p &&
					e.number.p )
				{
					if( skip == false )
						time += dltTime;

					if( skip || time >= e.delay )
					{
						if( e.title.p )
							e.title.p->Show(true);

						e.widget.p->Activate(e.play); // разрешаем/запрещаем анимацию набора значения

						int val = (int)e.number.p->Get();

						static char buf[32];
						sprintf_s(buf,sizeof(buf),"%d",val);

						const char *s = buf;

						e.widget.p->Command("SetString",1,&s);

						if( sound && e.play )
						{
							sound->Stop();
							sound->Play();
						}

						tickPeriodicTime = tickCooldown;

						if (tickSoundName.NotEmpty() && !tickPeriodic && e.play)
						{
							if (!tickSound)
								tickSound = Sound().Create(tickSoundName.c_str(), _FL_, false, false);

							if (tickSound && e.play)
							{
								tickSound->Stop();
								tickSound->Play();
							}
						}

						wait = true;
					}
				}
				else
				{
					index++;

					if( index >= elems )
					{
						complete.Activate(Mission(),false);
					}
				}
			}
		}
		else
		{
			Show(false);
		}
	}
	while( skip && IsShow());
}

void BaseGUITable::PostCreate()
{
	menu = Controls().FindControlByName(menu_control);

	MOSafePointer p; MissionObject *q;

	for( int i = 0 ; i < elems ; i++ )
	{
		Element &e = elems[i];

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

		if( string::NotEmpty(e.title.name))
		{
			FindObject(ConstString(e.title.name),p); q = p.Ptr();

			if( q )
			{
				e.title.p = q;
			}
			else
			{
				LogicDebugError("MissionObject \"%s\" not found.",e.title.name);

				e.title.p = null;
			}
		}
		else
			e.title.p = null;
	}

	Show(show);
}

void BaseGUITable::InitParams(MOPReader &reader)
{
	const char *name = reader.String().c_str();

	RELEASE(tickSound);

	if( !sound || !string::IsEqual(sound->GetName(),name))
	{
		RELEASE(sound)

		if( string::NotEmpty(name))
		{
			sound = Sound().Create(name,_FL_,false,false);
		}
	}

	fadeOut = reader.Float();

	tickSoundName = reader.String();
	tickPeriodic = reader.Bool();
	tickCooldown = reader.Float();
	tickEndSoundName = reader.String();

	long count = reader.Array();

	elems.DelAll();
	elems.Reserve(count);

	for( int i = 0 ; i < count ; i++ )
	{
		Element &e = elems[elems.Add()];

		e.widget.name = reader.String().c_str();

		e.number.name = reader.String().c_str();
		e.rel = reader.Bool();

		e.delay = reader.Float();

		e.title.name = reader.String().c_str();

		e.play = reader.Bool();
	}

	menu_control = reader.String().c_str();

	complete.Init(reader);

	show = reader.Bool();

	MissionObject::Show(false);

	tickPeriodicTime = tickCooldown;
}

void BaseGUITable::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( EditMode_IsOn())
		return;

	if( string::IsEqual(id,"Clear"))
	{
		const char *s = "";

		for( int i = 0 ; i < elems ; i++ )
		{
			const Element &e = elems[i];

			if( e.widget.p )
			{
				e.widget.p->Activate(false);
				e.widget.p->Command("SetString",1,&s);
			}

			if( e.title.p )
				e.title.p->Show(false);
		}

		LogicDebug("Command <Clear>: done");
	}
	else
	{
		LogicDebugError("Invalid command name: \"%s\"",id);
	}
}

static const char *_desc =

"GUI Table\n\n"
"    Use to display game statistics\n\n"
"Commands:\n\n"
"    Clear - clear all items (prepare for next show)";

MOP_BEGINLISTCG(BaseGUITable, "GUI Table", '1.00', 2000, _desc, "Interface")

	MOP_STRINGC("Sound", "", "Звук набора значения")
	MOP_FLOATEXC("Fade out", 0.0f, 0.0f, 2.0f, "Время затухания")

	MOP_GROUPBEG("Tick sound group")
		MOP_STRINGC("Tick sound", "", "Звук набора значения")
		MOP_BOOLC("Tick periodic", true, "Если true, то звук перезапускается каждые Tick time секунд")
		MOP_FLOATC("Tick time", 1.0f, "Время между стартом звука")
		MOP_STRINGC("Tick end sound", "", "Звук окончания набора значения")
	MOP_GROUPEND()

	MOP_ARRAYBEG("Elements", 0, 100)

		MOP_STRINGC("Widget", "", "Виджет для отображения значения переменной")

		MOP_STRINGC("Number", "", "Имя переменной из базы игры")
		MOP_BOOLC("Relative", true, "Добавлять к имени переменной префикс [Profile.<уровень сложности>.]")

		MOP_FLOATEXC("Delay", 0.5f, 0.0f, 5.0f, "Задержка перед активацией элемента")

		MOP_STRINGC("Title", "", "Объект, появляющийся при активации элемента")

		MOP_BOOLC("Play", true, "Отыгрывать анимацию набора значения")

	MOP_ARRAYEND

	MOP_STRINGC("Skip", "Menu_Select", "Имя контрола для скипания анимации таблицы")

	MOP_MISSIONTRIGGERC("Complete","")

	MOP_BOOLC("Show", false, "Запускать таблицу автоматически при старте миссии")

MOP_ENDLIST(BaseGUITable)
