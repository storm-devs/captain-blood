#include "ComboPreview.h"

#include "..\..\..\Common_h\ICharacter.h"

#include "..\CircularBar\CircularBar.h"
#include "..\Progress\Progress.h"

#include "..\..\..\Common_h\AnimationNativeAccess.h"

#include "..\GameUI\BaseGUIElement.h"
#include "..\utils\InterfaceUtils.h"

ComboPreview:: ComboPreview() : combos(_FL_)
{
//	model = "";
//	combo = "";

	anime = NULL;

	n = 0; time = 0.0f; started = true;

	cirWidth = 8.0f;
	btnWidth = 8.0f;
	marWidth = 8.0f;

	info = NULL; itime = 0.0f;
	comn = NULL; ntime = 0.0f;

	curCombo = 0;
}

ComboPreview::~ComboPreview()
{
}

void ComboPreview::Restart()
{
	Reset();

	ReCreate();
}

void ComboPreview::Reset()
{
	GUIWidget *p;

	if( p = (GUIWidget *)FindObject("A"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("AA"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("AAA"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("AAAA"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("AAAAB"))
		p->Show(false);

	if( p = (GUIWidget *)FindObject("B"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("BB"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("BBB"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("BBBB"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("BBBBA"))
		p->Show(false);

	if( p = (GUIWidget *)FindObject("M"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("MM"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("MMM"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("MMMM"))
		p->Show(false);
	if( p = (GUIWidget *)FindObject("MMMMM"))
		p->Show(false);

	CircularBar *b;

	if( b = (CircularBar *)FindObject("C"))
		b->SetPos(0);
	if( b = (CircularBar *)FindObject("CC"))
		b->SetPos(0);
	if( b = (CircularBar *)FindObject("CCC"))
		b->SetPos(0);
	if( b = (CircularBar *)FindObject("CCCC"))
		b->SetPos(0);
	if( b = (CircularBar *)FindObject("CCCCC"))
		b->SetPos(0);

	if( anime )
		anime->ActivateLink("idle");

	n = 0; time = 0.0f; started = true;

	Progress *pb = (Progress *)FindObject("P");

	if( pb )
	{
		pb->SetPos(0.0f);

	//	pb->SizeTo(1.0f - 0.08f*aspect);
		pb->SizeTo(1.0f - 0.01f*aspect*btnWidth);

		pb->Show(false);
	}
}

bool ComboPreview::Create(MOPReader &reader)
{
	InitParams(reader);

	Show(true);

	if( !EditMode_IsOn() && combos )
	{
		long n = startCombo < combos ? startCombo : 0;

	//	Combo &combo = combos[4];
		Combo &combo = combos[n];

		if( combo.seq[0] && combo.nodes )
			this->combo = combo.seq;

	//	curCombo = 4;
		curCombo = n;
	}

	return true;
}

bool ComboPreview::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void ComboPreview::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
}

void ComboPreview::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&ComboPreview::Draw,ML_PARTICLES3);
	else
		DelUpdate(&ComboPreview::Draw);

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}


void ComboPreview::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"select"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <select> error. Not enought parameters.");
			return;
		}

		int n = atoi(params[0]);

		if( n < combos )
		{
			curCombo = n; combo = combos[curCombo].seq;

			Reset();

			LogicDebug("Command <select>. Combo index = %d.",curCombo);
		}
		else
			LogicDebugError("Command <select> error. Invalid combo index (%d).",n);
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

void _cdecl ComboPreview::Draw(float dltTime, long level)
{
	if( !IsActive())
		return;

	if( EditMode_IsOn())
		return;

	if( !anime )
	{
		ICharacter *p = (ICharacter *)FindObject(modelName);

		if( p )
		{
			anime = p->GetAnimation();

			InitTimeLine();
		}
	}

	bool x_pressed = Controls().GetControlStateType("ChrA1") == CST_ACTIVATED;
	bool y_pressed = Controls().GetControlStateType("ChrB1") == CST_ACTIVATED;

	if( !anime )
	{
		ICharacter *p = (ICharacter *)FindObject(modelName);

		if( p )
			anime = p->GetAnimation();
	}

	if( info )
	{
		if( itime > 1.0f )
		{
			info->Show(false);

			info = NULL; itime = 0.0f;
		}
		else
		{
			itime += dltTime;

			return;
		}
	}

	if( comn )
	{
		if( ntime > 1.0f )
		{
			comn->Show(false);

			comn = NULL; ntime = 0.0f;
		}
		else
			ntime += dltTime;
	}

	if( Controls().GetControlStateType("ChrBlock1") == CST_ACTIVATED )
	{
		if( curCombo )
		{
			curCombo--; combo = combos[curCombo].seq;

			comn = (GUIWidget *)FindObject("Name");

			if( comn )
			{
				comn->SetDefaultString(combo + "\n");
				comn->Show(true);

				ntime = 0.0f;
			}

			anime->Goto("idle",0.3f);

			Reset();
			
			return;
		}
	}

	if( Controls().GetControlStateType("ChrAccept1") == CST_ACTIVATED )
	{
		if( curCombo < combos - 1 )
		{
			curCombo++; combo = combos[curCombo].seq;

			comn = (GUIWidget *)FindObject("Name");

			if( comn )
			{
				comn->SetDefaultString(combo + "\n");
				comn->Show(true);

				ntime = 0.0f;
			}

			anime->Goto("idle",0.3f);

			Reset();
			
			return;
		}
	}

	if( combo.Len() && n < combo.Len())
	{
		GUIWidget *p; CircularBar *b;

		if( started )
		{
			DWORD len = combo.Len(); string name = ""; string cn = ""; string mn = "";

			array<Node> &nodes = combos[curCombo].nodes;

			tl = 0.0f; tlp = 0.0f;

			for( DWORD i = 0 ; i < len ; i++ )
			{
				if( i >= nodes.Size())
					break;

			//	float t = nodes[i].beg;
				tl += nodes[i].beg;
			}

			BaseGUIElement::Rect r = {0.0f,0.0f,100.0f,100.0f};
			BaseGUIElement *panel = (BaseGUIElement *)FindObject("ComboPanel");

			if( panel )
				panel->GetRect(r);

		/*	float tx = 0.0f;
			float tr = 8*aspect;
			float tw = 100 - tx - tr;*/

			float tx = r.l;
		//	float tr = 8*aspect;
			float tr = cirWidth*aspect;
			float tw = r.r - tx - tr;

			for( DWORD i = 0 ; i < len ; i++ )
			{
				if( i >= nodes.Size())
					break;

				if( i < len - 1 )
					name += combo[i];
				else
					name  = "", name += combo[i];

				if( p = (GUIWidget *)FindObject(name))
				{
					tx += nodes[i].beg/tl*tw;

				//	p->MoveTo(tx/aspect,88);
					p->MoveTo(tx/aspect + (cirWidth - btnWidth)*0.5f,
						r.b - cirHeight + (cirHeight - btnHeight)*0.5f);

					p->SetAnim(false);

					p->Activate(false);
					p->Show(true);

					if( b = (CircularBar *)FindObject(cn += 'C'))
					//	b->MoveTo(tx,88.1f);
						b->MoveTo(tx,r.b - /*11.9f*/cirHeight);

					p = (GUIWidget *)FindObject(mn += 'M');

					if( p )
					{
						p->Show(false);
						p->MoveTo(tx/aspect + (cirWidth - marWidth)*0.5f,
							r.b - cirHeight + (cirHeight - marHeight)*0.5f);
					}
				}
			}

			Progress *pb = (Progress *)FindObject("P");

			if( pb )
			{
				pb->SetPos(0.0);

			//	pb->SizeTo(1.0f - 0.08f*aspect);
			//	pb->SizeTo((r.r - r.l)*0.01f - 0.08f*aspect);
				pb->SizeTo((r.r - r.l)*0.01f - 0.01f*cirWidth*aspect);

			//	pb->MoveTo((r.l + 4)*0.01f,(r.b - 11)*0.01f);
				pb->MoveTo((r.l/aspect + cirWidth*0.5f)*0.01f,
					(r.b - /*11*/cirHeight + (cirHeight - pb->GetHeight())*0.5f)*0.01f);

				pb->Show(true);
			}

			started = false; tt = 0.0f; canMove = false;

			curNode = "idle"; moved = false;
		}

		string name = ""; name += combo[n];

		if( n == combo.Len() - 1 )
			name += "_sp1";

		if( n >= combos[curCombo].nodes.Size())
		{
			anime->Goto("idle",0.3f);

			info = (GUIWidget *)FindObject("Lose");

			if( info )
				info->Show(true);

			Reset();

			eventLose.Activate(Mission(),false);

			return;
		}

		Node &node = combos[curCombo].nodes[n];

		if( n > 0 )
		{
			Progress *pb = (Progress *)FindObject("P");

			if( tt > node.beg )
				tt = node.beg;

			if( pb )
				pb->SetPos(tlp + tt/tl);
		}

		if( (time > node.len) ||
			(combo[n] == 'A' ? y_pressed : x_pressed) ||
			(canMove && anime && !anime->IsCanActivateLink(name)))
		{
			anime->Goto("idle",0.3f);

			info = (GUIWidget *)FindObject("Lose");

			if( info )
				info->Show(true);

			Reset();

			eventLose.Activate(Mission(),false);

			return;
		}

		const char *cn;

		if( anime && curNode != (cn = anime->CurrentNode()))
		{
			curNode = cn; moved = true;
		}

		if( anime && anime->IsCanActivateLink(name) && tt >= node.beg )
		{
			if( !canMove )
			{
				curNode = anime->CurrentNode();
				moved = false;
			}

		//	canMove = true;

			tt = node.beg;

			DWORD len = combo.Len(); string bn = ""; /*string mn = "";*/ string cn = ""; string mn = "";

			for( DWORD i = 0 ; i <= n ; i++ )
			{
				bn += combo[i];
				cn += 'C';
				mn += 'M';
			}

			if( n == len - 1 )
				bn = "", bn += combo[len - 1];

		/*	for( DWORD i = 0 ; i <  n ; i++ )
			{
				mn += 'M';
			}

			if( p = (GUIWidget *)FindObject(mn))
				p->Show(true);*/

			if( p = (GUIWidget *)FindObject(bn))
				p->Activate(!p->IsActive());

			if( b = (CircularBar *)FindObject(cn))
				b->SetPos(time/node.len);

			if( p && !canMove )
				p->SetAnim(true);

			canMove = true;

			if( combo[n] == 'A' ? x_pressed : y_pressed )
			{
				anime->ActivateLink(name);

				n++; time = 0.0f; tlp += tt/tl; tt = 0.0f;

				if( b = (CircularBar *)FindObject(cn))
					b->SetPos(0.0f);

				if( p )
					p->SetAnim(false);

				if( p = (GUIWidget *)FindObject(mn))
					p->Show(true);

				canMove = false;
			}
			else
				time += dltTime;
		}
		else
			if( moved )
				tt += dltTime;
	}
	else
	if( !EditMode_IsOn() && anime && string::IsEqual(anime->CurrentNode(),"idle"))
	{
		anime->Goto("idle",0.5f);

		info = (GUIWidget *)FindObject("Win");

		if( info )
			info->Show(true);

		Reset();

		eventWin.Activate(Mission(),false);

		return;
	}
}

void ComboPreview::InitAspect()
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

	aspect = def_aspect/cur_aspect;
}

void ComboPreview::InitParams(MOPReader &reader)
{
//	aspect = InterfaceUtils::AspectRatio(Render());
	InitAspect();

	const char *t = reader.String().c_str();

	if( EditMode_IsOn())
	{
		if( model != t )
		{
			ICharacter *p = (ICharacter *)FindObject(t);

			if( p )
				anime = p->GetAnimation();

			model = t;
		}
	}
	else
	{
		ICharacter *p = (ICharacter *)FindObject(t);

		if( p )
			anime = p->GetAnimation();
	}

	modelName = t;

	combos.DelAll();

	long cn = reader.Array();

	for( int i = 0 ; i < cn ; i++ )
	{
		Combo &combo = combos[combos.Add()];

		combo.seq = reader.String().c_str();

	/*	long nn = reader.Array();

		for( int j = 0 ; j < nn ; j++ )
		{
			Node &node = combo.nodes[combo.nodes.Add()];

			int len = reader.Long();
			int fps = reader.Long();

			int ab = reader.Long();
			int ae = reader.Long(); if( !ae ) ae = len;

			int mb = reader.Long();
			int me = reader.Long(); if( !me ) me = len;
		}*/
	}

	startCombo = reader.Long();

	eventWin .Init(reader);
	eventLose.Init(reader);

	cirWidth = reader.Float(); cirHeight = reader.Float();
	btnWidth = reader.Float(); btnHeight = reader.Float();
	marWidth = reader.Float(); marHeight = reader.Float();

	Activate(reader.Bool());

	InitTimeLine();
}

void ComboPreview::InitTimeLine()
{
	long cn = combos;

	for( int i = 0 ; i < cn ; i++ )
	{
		Combo &combo = combos[i];

		long last = strlen(combo.seq) - 1;

	//	long n = combo.nodes;

	//	if( !n )
	//		continue;

		combo.nodes.DelAll();
		combo.nodes.Add();

		combo.nodes[0].beg = 0.0f;
		combo.nodes[0].len = 3.0f;

		int len; float fps;

		int ab,ae;
		int mb,me;

		char name[] = "  ";

		char link	[] = " ";
		char link_sp[] = " _sp1";

		bool r; const char *p;

		for( int j = 1 ; j <= last ; j++ )
		{
		//	if( j >= n )
		//		break;

			name[0] = combo.seq[0];
			name[1] = '0' + j;

			link   [0] = combo.seq[j];
			link_sp[0] = combo.seq[j];

			p = j < last ? link : link_sp;

			if( anime )
			{
				AGNA_NodeInfo ni(name);

				r = anime->GetNativeGraphInfo(ni);
			//	Assert(r)
				if( !r )
				{
					LogicDebugError("InitTimeLine ERROR: animation node %s not found",name);
					continue;
				}

				AGNA_ClipInfo ci(ni.index,0);

				r = anime->GetNativeGraphInfo(ci);
			//	Assert(r)
				if( !r )
				{
					LogicDebugError("InitTimeLine ERROR: animation clip %s[0] not found",name);
					continue;
				}

				len = ci.frames;
				fps = ci.fps;

				AGNA_LinkInfo li(ni.index,-1);

				for( dword k = 0 ; k < ni.numLinks ; k++ )
				{
					li.linkIndex = k;

					r = anime->GetNativeGraphInfo(li);
				//	Assert(r)
					if( !r )
					{
						LogicDebugError("InitTimeLine ERROR: animation link %s[%d] not found",name,k);
						continue;
					}

					if( string::IsEqual(li.command,p))
						break;
				}
			//	Assert(k < ni.numLinks)

				ab = li.arange[0];
				ae = li.arange[1]; if( ae > 99999 ) ae = len;

				mb = li.mrange[0];
				me = li.mrange[1]; if( me > 99999 ) me = len;
			}

			combo.nodes.Add();

			Node &node = combo.nodes[j];

			node.beg = ab/fps;
			node.len = ae/fps - node.beg;
		}
	}
}

MOP_BEGINLISTCG(ComboPreview, "Combo preview", '1.00', 100, "Combo preview\n\n    Use to demonstrate combo sequencies\n\nAviable commands list:\n\n    select - set current combo\n\n        param[0] - combo index", "Interface")

	MOP_STRING("Character", "Player")

	MOP_ARRAYBEG("Combos", 0, 100)

		MOP_STRING("sequence", "")

	/*	MOP_ARRAYBEG("Nodes", 0, 100)

			MOP_LONG("len", 0)
			MOP_LONG("fps", 0)

			MOP_LONG("ab", 0)
			MOP_LONG("ae", 0)

			MOP_LONG("mb", 0)
			MOP_LONG("me", 0)

		MOP_ARRAYEND*/

	MOP_ARRAYEND

	MOP_LONG("Start combo", 0)

	MOP_MISSIONTRIGGERC("OnWin" ,"Win trigger")
	MOP_MISSIONTRIGGERC("OnLose","Lose trigger")

	MOP_FLOAT("Cir width",   8.0f);
	MOP_FLOAT("Cir height", 12.0f);

	MOP_FLOAT("Btn width" ,  8.0f);
	MOP_FLOAT("Btn height", 12.5f);

	MOP_FLOAT("Mar width" , 10.0f);
	MOP_FLOAT("Mar height", 10.0f);

	MOP_BOOL("Active", false)

MOP_ENDLIST(ComboPreview)
