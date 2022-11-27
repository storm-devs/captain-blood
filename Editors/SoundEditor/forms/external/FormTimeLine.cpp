#include "FormTimeLine.h"
#include "PreviewAnimation.h"

#include "..\FormButton.h"
#include "..\FormMessageBox.h"

#include "..\..\..\..\Common_h\Animation.h"
#include "..\..\..\..\Common_h\AnimationNativeAccess.h"

FormTimeLine::FormTimeLine(GUIControl *parent, const GUIRectangle &rect, PreviewAnimation *preview)
	: FormControlBase(parent,rect),nodes(_FL_)
{
	this->preview = preview;

	dragNode = null;

	nodesHead = -1;
	nodesTail = -1;

	nodesFree = -1;

	dragThumb = null;

	scr.page = 0; scr.pos = 0;

	offset = 0; dragOffset = 0;

	dragEvent = null; dragEventIndex = -1; curNode = null;

	dragMeter = null;

	selNode = null;

	selE = null; selEIndex = -1;

	cursor = -1000.0f;

	animeId.Reset();
	movieIndex = -1;

	//////////////////

	focusEvent = null;

	focusEventIndex	= -1;
	focusEventNode  = -1;

	focus = null;
	focusIndex = -1;

//	SetFocus(null,-1,false);
//	SelectEvent(null);

	//////////////////

	anime = null;

	loading = false;

	ani = null;

	blendTime = 0.2f;

	animNode = null;

	cursorNode = -1;
	cursorFrame = 0.0f;

	dragCursor = false;

	smallFont = NEW GUIFont("arialcyrsmall");
	Assert(smallFont)

	scr.scale = 0.2f;

	mustSetFrame = 0;

	animPause = false;

	animLocal = false;

	initAnim = false;

	initPlay = false;

	moveLeft = false;

	moveCursor = false;

	autoScroll = true;

	////

	scr.r.x = 0;
	scr.r.y = rect.h - c_scroll;
	scr.r.w = rect.w;
	scr.r.h = c_scroll;

	scr.page = scr.r.w/scr.scale;
	scr.size = 0;

	scr.update();

	////

	met.r.x = 0;
	met.r.y = scr.r.y - c_meter;
	met.r.w = scr.r.w;
	met.r.h = c_meter;

	////

	cur.r.x = 0;
	cur.r.y = met.r.y - c_cursor;
	cur.r.w = met.r.w;
	cur.r.h = c_cursor;

	////

	eve.r.x = 0;
	eve.r.y = c_node_h;
	eve.r.w = cur.r.w;
	eve.r.h = rect.h - c_node_h - c_scroll - c_meter - c_cursor;

	////

	preview->bBegin->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::Begin);
	preview->bEnd  ->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::End);

	preview->bNodeBegin->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::NodeBegin);
	preview->bNodeEnd  ->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::NodeEnd);

	preview->bPrev->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::Prev);
	preview->bNext->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::Next);

	preview->bPlayNode->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::PlayNode);
	preview->bPlay	  ->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::Play);

//	preview->bPause->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::Pause);
	preview->bStop ->onDown.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::Stop);

	//Max - через этот хандлер рисуюеться отладочная информация по звукам
	preview->onUpdateSounds.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::UpdateSounds);

	////

	editNodes  = true;
	editEvents = true;

	////

	ldown = false;
	rdown = false;

	ctl_c = false;

	////

	play_delay = 0;

	////

	lastID = UniqId::zeroId;
}

FormTimeLine::~FormTimeLine()
{
	DELETE(smallFont)
}

static bool event_update = false;

float FormTimeLine::DrawNode(const GUIRectangle &rect, const Node &node, bool sel)
{
	float scale = scr.scale;

	int node_x = round(scale*(node.t - scr.pos));
	int node_y = 0;

	int node_r = round(scale*(node.t - scr.pos + node.w));
	int node_z = round(scale*(node.t - scr.pos + node.width));

	if( node_z >= 0 && node_x < rect.w )
	{
		bool node_is_valid = node.index >= 0;

		node_x += rect.x;
		node_y += rect.y;

		node_r += rect.x;

		int node_w = node_r - node_x;

		const dword disabled = options->bkg;

		Color fcol(node.isEnable ? 0xff00c000 : disabled);

		const dword err = 0xffff00b0;
		const dword war = 0xffd0d000;

		if( node_is_valid == false )
			fcol = Color(err);
		else
		if( node.valid == false )
			fcol = Color(war);

		const dword red = 0xffe00000;

		if( &node == selNode )
		{
			fcol = red;
		}

		Color capt(fcol);

		capt.r *= 0.85f;
		capt.g *= 0.85f;
		capt.b *= 0.85f;

	/*	Color bcol(disabled); bcol.a *= 0.6f;

		bcol.r *= 0.75f;
		bcol.g *= 0.75f;
		bcol.b *= 0.75f;*/
		Color bcol(0x80404040);

		if( &node != focus )
		{
			const float gamma = 0.8f;

			fcol.r *= gamma;
			fcol.g *= gamma;
			fcol.b *= gamma;

			bcol.r *= gamma;
			bcol.g *= gamma;
			bcol.b *= gamma;

			capt.r *= gamma;
			capt.g *= gamma;
			capt.b *= gamma;
		}

		float width = node.w;

		int next_r = node_r;
		int next_x = node_x;

		if( node.next >= 0 )
		{
			const Node &next = nodes[node.next];

			next_x = round(scale*(next.t - scr.pos)) + rect.x;
		}

		//// Caption

		if( node.next >= 0 && nodes[node.next].t < node.t + node.w - 0.5f*node.frameLen )
		{
			const Node &next = nodes[node.next];

			GUIHelper::Draw2DRect(node_x,node_y,next_x - node_x,c_node_h,fcol);

			GUIPoint pt[3];

			pt[0] = GUIPoint(0, 0);
			pt[1] = GUIPoint(5, 5);
			pt[2] = GUIPoint(0,10);

			Color col(capt);

			col.r *= 0.65f;
			col.g *= 0.65f;
			col.b *= 0.65f;

			GUIHelper::DrawPolygon(
				pt,3,col,next_x - 14 + 1,node_y + 3);
			GUIHelper::DrawPolygon(
				pt,3,col,next_x -  9 + 1,node_y + 3);

			width = next.t - node.t;
		}
		else
			GUIHelper::Draw2DRect(node_x,node_y,node_w + 1,c_node_h,fcol);

		//// CheckBox

		if( node_is_valid )
		{
			GUIHelper::Draw2DRect(node_x + 2,node_y + 17,8,8,options->bkg2White[7]);

			if( node.isEnable )
			{
				GUIHelper::Draw2DRect(node_x + 3,node_y + 18,6,6,capt);
			}
		}

		////

		if( &node == focus )
		{
			GUIHelper::Draw2DRect(node_x,rect.y + cur.r.y,node_w + 1,cur.r.h,fcol);
		}
		else
		{
			GUIHelper::Draw2DRect(node_x + 1,rect.y + cur.r.y,node_w,cur.r.h,fcol);
		}

		const Node *prevElem = null;

		if( &node == dragNode )
		{
			prevElem = prevNode;
		}
		else
		if( node.prev >= 0 )
		{
			prevElem = &nodes[node.prev];
		}

		if( prevElem )
		{
			const Node &prev = *prevElem;

			if( node.t < prev.t + prev.w )
			{
				int frame = int((node.t - prev.t + prev.frameLen*0.5f)/prev.frameLen);

				char buf[16]; sprintf_s(buf,sizeof(buf),"%d",frame);

				int tw = smallFont->GetWidth(buf) + 2;

				GUIHelper::Draw2DRect(node_x,rect.y + cur.r.y,tw,cur.r.h,capt);

				GUIHelper::DrawVertLine(
					cur.r.y + rect.y,
					cur.r.h,
					node_x,capt);

				smallFont->Print(node_x,rect.y + cur.r.y - 1,options->colorTextHi,buf);
			}
		}

		if( dragEvent && curNode == &node )
		{
			int frame = dragEvent->frame;

			char buf[16]; sprintf_s(buf,sizeof(buf),"%d",frame);

			int tw = smallFont->GetWidth(buf) + 2;

			int x = round(scale*(node.t - scr.pos + frame*node.frameLen));

			GUIHelper::Draw2DRect(x,rect.y + cur.r.y,tw,cur.r.h,capt);

			smallFont->Print(x,rect.y + cur.r.y - 1,options->colorTextHi,buf);
		}

		int cext = 0;

		if( node.next >= 0)
		{
			const Node &next = nodes[node.next];

			if( node_x + node.name.w > next_x )
			{
				cext = node_x + node.name.w - next_x;
			}
		}

	/*	if( node.name.w > node_w + 1 )
		{
			GUIHelper::Draw2DRect(node_x + node_w + 1,node_y,node.name.w - node_w - 1 - cext,node.name.h,bcol);
		}*/

		GUIHelper::Draw2DRect(node_x,node_y,node.name.w - cext,node.name.h,capt);
		smallFont->Print(node_x,node_y,options->colorTextHi,node.name.str);

		if( node_is_valid )
		{
			smallFont->Print(node_x,node_y + 24,options->colorTextHi&0x80ffffff,
				"%d/%4.3ffps",node.frames,node.fps);
		}

		int back_w = node_w;

		if( node.next >= 0 )
		{
			if( back_w > next_x - node_x )
				back_w = next_x - node_x;
		}

		GUIHelper::Draw2DRect(node_x,node_y + eve.r.y,back_w,eve.r.h,bcol);

		GUIHelper::DrawVertLine(
			node_y,
			c_node_h + eve.r.h,
			node_x,capt);

		int drag_l = node_x + scr.r.w;
		int drag_r = node_x + scr.r.w;

		if( dragNode && dragNode != &node )
		{
			drag_l = node_x + round(scale*(dragNode->t - node.t));
			drag_r = node_x + round(scale*(dragNode->t - node.t + dragNode->w));
		}

		//// Grid

		if( node.frames > 1 )
		{
			Color col(fcol); col.a *= 0.75;

			float df = node.frameLen;

			for( int i = 0 ; i < node.frames ; i++ )
			{
				int t = rect.x + round(scale*(node.t + i*df - scr.pos));

				if( t > node_x + width*scale && t < next_r || t >= drag_l && t < drag_r )
					;
				else
					GUIHelper::DrawVertLine(
						node_y + eve.r.y,eve.r.h,
						t,col);
			}

			if( width == node.w || node.t + node.w > nodes[node.next].t + nodes[node.next].w )
			{
				if( node_r >= drag_l && node_r < drag_r )
					;
				else
					GUIHelper::DrawVertLine(
						node_y + eve.r.y,eve.r.h,
						node_r,col);
			}
		}

		//// Events

		if( node.index >= 0 )
		{
			float node_width = node.w;

			dword base = &node == selNode ? 0xffdf0000 : node.isEnable ? 0xff308030 : disabled;
			dword dele = 0xffdf0000;

			if( node.valid == false )
				base = 0xffb9b300;

			const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

			int eyp = node_y + eve.r.y + 3;
			int pos = 1;

			for( int j = 0 ; j < events ; j++ )
			{
				const ProjectAnimation::Event &e = events[j];

				if( !e.position )
				{
					anime->SetEventPosition(node.index,j,pos);

					event_update = true;
				}

				int ey = eyp + (e.position - 1)*c_evnt_h;

				bool is3D; const char *locName = anime->GetEventSoundParams(node.index,j,&is3D);

				bool invalidLocName =
					is3D && string::NotEmpty(locName) && preview->GetLocatorIndex(locName) < 0;

				float tmin,tmax;

				bool looped = anime->EventSoundIsLoop(node.index,j);

				if( looped || !anime->GetEventSoundTime(node.index,j,tmin,tmax))
				{
					tmin = tmax = 0.0f;
				}
				else
				{
					tmin *= 1000.0f;
					tmax *= 1000.0f;
				}

			//	if( tmax < c_evnt_h/0.2f )
			//		tmax = c_evnt_h/0.2f;

				int ex = rect.x + round(scale*(node.t + e.frame*node.frameLen - scr.pos));
				int er = rect.x + round(scale*(node.t + e.frame*node.frameLen - scr.pos + tmax));

				if( er - ex < 2 )
					er = ex + 2;

				float right = e.frame*node.frameLen + tmax;

				if( node_width < right )
					node_width = right;

				dword bh = c_evnt_h - 6;

			/*	GUIHelper::Draw2DRect(ex + 2,ey,er - ex - 2,bh,options->bkg2White[7]);

				Color back(&e == selE ? dele : &e == focusEvent ? 0xff388f2c : base);
				Color mark(&e == selE ? dele : base);

			//	back.a *= (&e == focusEvent ? 0.7f : 0.5f);
				back.a *= (&e == focusEvent ? 0.3f : 0.5f);

				if( &e == focusEvent )
					mark.a *= 0.65f;

				if( &node == focus )
				{
					mark.a *= 0.75f;
					back.a *= 0.75f;
				}*/

				bool in = e.frame < (dword)node.frames;

				Color back(invalidLocName || !in ? 0xffc0c000 : fcol);

				if( in == false || invalidLocName )
				{
					if( &node == focus )
					{
						back.r *= 1.1f;
						back.g *= 1.1f;
						back.b *= 1.1f;
					}
				}

				back.r *= 1.1f;
				back.g *= 1.1f;
				back.b *= 1.1f;

				if( &e == focusEvent )
				{
					back.r *= 1.2f;
					back.g *= 1.2f;
					back.b *= 1.2f;
				}
				else
				{
					back.a *= 0.8f;
				}

				if( &e == selE )
					back = red;

			/*	if( er > node_r )
				{
					GUIHelper::Draw2DRect(ex + 2,ey,node_r - ex + 1 - 2,bh,back);
					GUIHelper::Draw2DRect(node_r + 1,ey,er - node_r,bh,
						(&e == selE ? dele : &e == focusEvent ? options->black2Bkg[3] : options->black2Bkg[0])&0x40ffffff);
				}
				else*/
					GUIHelper::Draw2DRect(ex + 2,ey,er - ex - 2,bh,back);

				if( looped )
				{
					Color blue(0xff1010e0);

					if( &node == focus )
					{
						blue.r *= 1.2f;
						blue.g *= 1.2f;
						blue.b *= 1.2f;
					}

					if( &e == focusEvent )
					{
						blue.r *= 1.2f;
						blue.g *= 1.2f;
						blue.b *= 1.2f;
					}
					else
					{
						blue.a *= 0.6f;
					}

					if( &e == selE )
						blue = red;

					GUIHelper::Draw2DRect(ex + 2,ey - 1,60,bh + 2,blue);

					smallFont->Print(ex + c_evnt_h + 1,ey - 4,options->colorTextHi,"Looped");
				}

				bool isFoc = &e == focusEvent;

				if( e.eventId == lastID )
				{
					dword c = -1;

					GUIHelper::DrawHorizLine(ex - 2,3,ey - 5,c);
					GUIHelper::DrawHorizLine(ex - 2,3,ey + c_evnt_h - 2,c);

					GUIHelper::DrawHorizLine(ex + c_evnt_h - 2,3,ey - 5,c);
					GUIHelper::DrawHorizLine(ex + c_evnt_h - 2,3,ey + c_evnt_h - 2,c);

					GUIHelper::DrawVertLine(ey - 5,3,ex - 2,c);
					GUIHelper::DrawVertLine(ey - 5,3,ex + c_evnt_h + 1,c);

					GUIHelper::DrawVertLine(ey + c_evnt_h - 5,3,ex - 2,c);
					GUIHelper::DrawVertLine(ey + c_evnt_h - 5,3,ex + c_evnt_h + 1,c);
				}

			//	GUIHelper::Draw2DRect(ex,ey - 3,c_evnt_h,c_evnt_h,mark);
				GUIHelper::DrawSprite(ex,ey - 3,c_evnt_h,c_evnt_h,is3D ?
					isFoc ? &options->imageAniEvtLocSel : &options->imageAniEvtLoc :
					isFoc ? &options->imageAniEvtGlbSel : &options->imageAniEvtGlb);

				pos++;
			}

			return node_width;
		}
		else
			return -1.0f;
	}
	else
		return -1.0f;
}

//Рисование
void FormTimeLine::OnDraw(const GUIRectangle &rect)
{
	if( ani )
	{
		if( animNode || cursorNode >= 0 )
		{
			const Node &node = animNode ? *animNode : nodes[cursorNode];

			if( !string::IsEqual(ani->CurrentNode(),anime->GetAnxNodeName(node.index)))
			{
				Goto(node);

				if( !animNode )
					SetFrame(cursorFrame);
			}
		}
	}

	if( ani && mustSetFrame > 0 )
	{
		UpdateFrame();
	}

	if( offset && GetAsyncKeyState(VK_CONTROL) >= 0 )
	{
		UpdateOffset();
	}

	if( play_delay > 0 )
	{
		if( play_delay > 1 )
		{
		}
		else
		{
			if( play_local )
				PlayNode(null);
			else
				Play(null);
		}

		play_delay--;
	}
	else
	{
		if( animNode && !animPause )
		{
			UpdatePlay();
		}
	}

	bool lk = GetAsyncKeyState(VK_LEFT)  < 0;
	bool rk = GetAsyncKeyState(VK_RIGHT) < 0;

	if( editNodes && !animNode && !dragNode && focus )
	{
		if( lk && !ldown && !rk )
			StepNodeL();
		else
		if( rk && !rdown && !lk )
			StepNodeR();
	}

	ldown = lk;
	rdown = rk;

	///////////

	bool cc =
		GetAsyncKeyState(VK_CONTROL) < 0 &&
		GetAsyncKeyState('C')		 < 0;

	if( editEvents && !animNode && !dragNode && focus && focusEvent )
	{
		if( cc && !ctl_c )
		{
			if( lastID != focusEvent->eventId )
			{
				lastID  = focusEvent->eventId;
			}
			else
			{
				lastID = UniqId::zeroId;
			}
		}
	}

	ctl_c = cc;

	///////////////////////////////////////////////

	if( nodesHead >= 0 )
	{
		GUIHelper::Draw2DRect(rect.x,rect.y,rect.w,rect.h,options->black2Bkg[5]);
	}

/*	//// Cursor

	if( nodesHead >= 0 )
	{
		GUIHelper::Draw2DRect(
			rect.x + cur.r.x,
			rect.y + cur.r.y,cur.r.w,cur.r.h,options->bkg&0x7fffffff);
	}

	////*/

	event_update = false;

	if( nodesHead >= 0 )
	{
		int index = nodesHead;

		for( ;; )
		{
			const Node &node = nodes[index];

			float width = DrawNode(rect,node);

			if( width > 0.0f && width != node.width )
			{
				Node &node = nodes[index];

				node.width = width;

				const Node &tail = nodes[nodesTail];

				float tr = tail.t + tail.width;
				float nr = node.t + node.width;

				float size = coremax(tr,nr);

				if( size > scr.size )
					scr.setSize(size);
			}

			if( index == nodesTail )
				break;

			index = node.next;
		}
	}

	if( dragNode )
	{
		const Node &node = *dragNode;

		DrawNode(rect,node,true);
	}

	/////////////////////////

	if( event_update )
		anime->SaveChanges();

	/////////////////////////

	//// Meter

	if( nodesHead >= 0 )
	{
		GUIHelper::Draw2DRect(
			rect.x + met.r.x,
			rect.y + met.r.y,met.r.w,met.r.h,options->bkg2White[1]);

		float dp = 5.0f;
		float dt = dp/scr.scale;

		dt = powf(10.0f,ceilf(log10f(dt)));
		dp = dt*scr.scale;

		float t = scr.pos - fmodf(scr.pos,dt*10);
		float x = (t - scr.pos)*scr.scale;

		 t *= 0.001f;
		dt *= 0.001f;

		/////////////////////////////

		float rect_w = (float)rect.w;

		for( int i = 0 ; x < rect_w ; i++,x += dp,t += dt )
		{
			bool mark = i%10 == 0;

			int xx = round(x);

			GUIHelper::DrawVertLine(
				rect.y + met.r.y,
				mark ? 6 : 3,
				rect.x + met.r.x + xx,options->black2Bkg[4]);

			if( mark )
			{
				smallFont->Print(
					rect.x + met.r.x + 2 + xx,
					rect.y + met.r.y + 3,
					options->black2Bkg[4],"%3.2f",t);
			}
		}

		int boff = 0;

		if( scr.size < scr.page )
			boff = round(scr.scale*(scr.page - scr.size));

		const char *cap = "sec.";

		int cap_w = smallFont->GetWidth(cap) + 1;
		int cap_h = met.r.h - 6;

		int cap_x = rect.x + met.r.x + met.r.w - boff - cap_w - 3;
		int cap_y = rect.y + met.r.y + 6;

		GUIHelper::Draw2DRect(
			cap_x - 2,
			cap_y,
			cap_w + 3 + 2,
			cap_h,
			options->bkg2White[1]);

		smallFont->Print(cap_x,cap_y - 3,options->black2Bkg[4],cap);
	}

	//// Scroller

	GUIHelper::Draw2DRect  (rect.x + scr.r.x,rect.y + scr.r.y,scr.r.w    ,scr.r.h    ,options->black2Bkg[5]);
	GUIHelper::DrawWireRect(rect.x + scr.r.x,rect.y + scr.r.y,scr.r.w - 1,scr.r.h - 1,options->black2Bkg[0]);

	if( cursor >= 0.0f && scr.page < scr.size )
	{
		int x = rect.x + scr.r.x + round(scr.r.w*cursor/scr.size);
		int y = rect.y + scr.r.y;

		int h = scr.r.h;

		GUIHelper::Draw2DRect(x - 3,y,7,h,0x40000000);

		GUIHelper::DrawVertLine(y,h - 1,x,0xff000000);
	}

	GUIHelper::Draw2DRect  (rect.x + scr.t.x,rect.y + scr.t.y,scr.t.w    ,scr.t.h    ,
		scr.size <= scr.page ? options->bkg : options->bkg2White[1]);
	GUIHelper::DrawWireRect(rect.x + scr.t.x,rect.y + scr.t.y,scr.t.w - 1,scr.t.h - 1,
		options->black2Bkg[0]);

	//// Cursor

	if( cursor >= 0.0f )
	{
		int y = rect.y + eve.r.y; int w = eve.r.h + cur.r.h;

	/*	if( animLocal )
		{
			int z = rect.x + round(scr.scale*(cursorBeg - scr.pos));

			if( z + 5 >= 0 )
			{
				GUIHelper::Draw2DRect(z - 3,y,7,w,options->black2Bkg[0]&0x40ffffff);

				GUIHelper::DrawVertLine(
					y,
					w,
					z,options->black2Bkg[0]);
			}
		}*/

		int x = rect.x + round(scr.scale*(cursor - scr.pos));

		if( x + 5 >= 0 )
		{
			//float cursorBase = animLocal ? animNode->t : cursorBeg;
			//Max
			float cursorBase = (animLocal && animNode) ? animNode->t : cursorBeg;
			if( cursor > cursorBase )
			{
				int p = rect.x + round(scr.scale*(cursorBase - scr.pos));

			//	GUIHelper::Draw2DRect(p,y,x - p,w,0x200000ff);
				GUIHelper::Draw2DRect(p,y,x - p,w,0x406060ff);
			}

			GUIHelper::Draw2DRect(x - 3,y,7,w,options->black2Bkg[0]&0x40ffffff);

			GUIHelper::DrawVertLine(
				y,
				w,
				x,options->black2Bkg[0]);
		}
	}

	//// Border

	if( scr.size < scr.page )
	{
		int boff = round(scr.scale*(scr.page - scr.size)) - 1;

		GUIHelper::Draw2DRect(
			rect.x + rect.w - boff,rect.y,boff,rect.h - c_scroll,options->bkg);
	}

	if( dragNode || dragEvent )
	{
		const Node &node = dragNode ? *dragNode : *curNode;
		const Node &tail = nodesTail >= 0 ? nodes[nodesTail] : node;

		float r1 = tail.t + tail.width;
		float r2 = node.t + node.width;

		float rr = coremax(r1,r2);

		if( rr < scr.size && rr - scr.pos < scr.page )
		{
			int boff = round(scr.scale*(scr.size - rr)) - 1;

			int d = round(scr.scale*(scr.size - (scr.pos + scr.page)));

			if( d > 0 )
				boff -= d;

			if( scr.size < scr.page )
				boff += round(scr.scale*(scr.page - scr.size));

			GUIHelper::Draw2DRect(
				rect.x + rect.w - boff,
				rect.y,
				boff,
				rect.h - c_scroll,options->bkg&0x80ffffff);
		}
	}

	///////////////////////////////////////////////////////

	UpdateButtons();
}

//Начало перемещения, возвранить true, если начинать тащить. elementPivot графический центр элемента
bool FormTimeLine::OnDragBegin(const GUIPoint &mousePos, GUIPoint &elementPivot)
{
	options->kbFocus = this;
	options->gui_manager->SetKeyboardFocus(this);

	if( scr.size > scr.page )
	{
		if( scr.t.Inside(mousePos))
		{
			elementPivot = scr.t.pos;
			dragThumb = &scr;

			if( animNode )
				autoScroll = false;

			return true;
		}

		if( met.r.Inside(mousePos))
		{
			metBase = scr.pos; metPos = mousePos.x;

			elementPivot = 0;
			dragMeter = &met;

			return true;
		}
	}

	if( cur.r.Inside(mousePos))
	{
		if( !ani )
		{
		//	cursor = scr.pos + mousePos.x/scr.scale;

			return false;
		}

		if( nodesHead >= 0 )
		{
			float pos_x = mousePos.x/scr.scale + scr.pos;
			int   pos_y = mousePos.y;

			Place place;

			pos_y -= c_cursor + eve.r.h;

			if( FindElement(pos_x,pos_y,place))
			{
				if( place.in )
				{
					Node &node = nodes[place.index];

					if( node.index >= 0 && node.valid )
					{
					//	SetFocus(&node,place.index,false);

					//	ani->Pause(false);

					//	ani->Goto(node.name.str,0.0f);
					//		 Goto(node);

						if( !Goto(node))
						{
							return false;
						}

						ani_Pause(true);

						float frame = (pos_x - node.t)/node.frameLen;

					/*	AGNA_SetCurrentFrame scf(frame);

						bool r = ani->GetNativeGraphInfo(scf);
						Assert(r)
					//	mustSetFrame = 2;*/
						SetFrame(frame);

						UpdateEvents();

						CheckEvents(node,frame - 0.9f,frame);

					//	cursor = node.t + floorf(frame)*node.frameLen;
						cursor = pos_x;

						cursorNode	= place.index;
						cursorFrame = frame;

						cursorBeg = cursor;

						cursorBase = cursor;

						dragCursor = true;

						elementPivot = 0;

						animNode = null;

						animPause = false;

						return true;
					}

					return false;
				}
			}
		}

	/*	if( ani )
		{
			ani->Pause(false);

			ani->Goto("idle",0.3f);
		}

		cursorNode = -1;
		cursorFrame = 0.0f;

		cursor = -1000.0;

		animNode = null;

		animPause = false;*/

		return false;
	}

	if( nodesHead >= 0 )
	{
		float pos_x = mousePos.x/scr.scale + scr.pos;
		int   pos_y = mousePos.y;

		Place place;

		if( FindElement(pos_x,pos_y,place))
		{
			int index = place.index; Node &node = nodes[index];

			SetFocus(&node,index);

			if( place.in )
			{
				if( editNodes && !animNode )
				{
					int x = int((pos_x - node.t)*scr.scale);
					int y = pos_y;

					if( x >=  2 && x <  2 + 8 &&
						y >= 17 && y < 17 + 8 )
					{
						if( node.index >= 0 && node.valid )
						{
							node.isEnable = !node.isEnable;

							////////////////////////////////////

							ExportNodes(); anime->SaveChanges();

							////////////////////////////////////

						/*	if( !node.isEnable )
							{
								SelectEvent(null);
							}*/
						}

						return false;
					}

					nodeBase = node.t; mouseOff = mousePos.x;

					elementPivot = 0;
					dragNode = &node;

					dragNodeNext = node.next;

					if( dragNodeNext >= 0 )
					{
						const Node &next = nodes[dragNodeNext];

						dragNodeNextOffset = next.t - node.t;
					}
					else
						dragNodeNextOffset = -1.0f;

					prevNode = index != nodesHead ? &nodes[node.prev] : null;
					nextNode = index != nodesTail ? &nodes[node.next] : null;

					dragIndex = index;
					RemoveNode(node,index);

					scroll_l = false;
					scroll_r = false;

					if( cursorNode >= 0 )
					{
						moveCursor = true;

						cursorOffset = cursor - nodes[cursorNode].t;
					}

					return true;
				}
				else
					return false;
			}

			if( /*editEvents &&*/ !animNode )
			{
				const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

				if( place.e >= 0 )
				{
					const ProjectAnimation::Event &e = events[place.e];

					SelectEvent(&e,place.index,place.e);

					if( editEvents && GetAsyncKeyState(VK_SHIFT) < 0 )
					{
						if( lastID != e.eventId )
						{
							lastID  = e.eventId;
						}
						else
						{
							lastID  = UniqId::zeroId;
						}
					}

					if( editEvents && node.isEnable && node.valid )
					{
						evenBase = e.frame*node.frameLen;

						elementPivot.x = 0;
						elementPivot.y = pos_y;

						dragEvent = &e; dragEventIndex = place.e; curNode = &node;

						return true;
					}
					else
						return false;
				}

				if( editEvents && node.isEnable && node.valid )
				{
					pos_x -= node.t;
					pos_y -=  eve.r.y;

					float x = pos_x;
					int   y = pos_y;

					x = coremin(x,node.w);

					int j = y/c_evnt_h + 1;

					dword frame = dword(x/node.frameLen);

					dword ei = anime->AddEvent(node.index,frame,movieIndex,&lastID);

					//////////////

					for( ;; )
					{
						for( int i = ei - 1 ; i >= 0 ; i-- )
						{
							const ProjectAnimation::Event &e = events[i];

							if( e.position == j )
							{
								Assert(frame != e.frame)

								if( frame < e.frame )
								{
									anime->ExchangeEvents(node.index,i,ei);

									ei = i; continue;
								}
							}
						}

						break;
					}

					//////////////

					anime->SetEventPosition(node.index,ei,j);

					x = frame*node.frameLen;

					float tmin,tmax;

					bool looped = anime->EventSoundIsLoop(node.index,ei);

					if( looped || !anime->GetEventSoundTime(node.index,ei,tmin,tmax))
					{
						tmin = tmax = c_evnt_h/0.2f;
					}
					else
					{
						tmin *= 1000.0f;
						tmax *= 1000.0f;
					}

					if( tmax < c_evnt_h/0.2f )
						tmax = c_evnt_h/0.2f;

					if( node.width < x + tmax )
						node.width = x + tmax;

					const Node &tail = nodes[nodesTail];

					float tr = tail.t + tail.width;
					float nr = node.t + node.width;

					scr.setSize(coremax(tr,nr));

					SelectEvent(&events[ei],place.index,ei);

					/////////////////////

					anime->SaveChanges();

					/////////////////////
				}
			}

			return false;
		}
	}

	return false;
}

//Перемещение текущего элемента в данную точку
void FormTimeLine::OnDragMoveTo(const GUIPoint &elementPosition)
{
	Assert(dragNode || dragThumb || dragEvent || dragMeter || dragCursor)

	if( dragCursor )
	{
		Node &node = nodes[cursorNode];

		float prev = cursor;

		MoveCursor(cursorBase + elementPosition.x/scr.scale,node,cursorFrame,cursorNode);

	//	api->Trace("%f",cursor - prev);

		preview->Delta(fabsf(cursor - prev)*0.001f);

		return;
	}

	if( dragNode )
	{
		Node &node = *dragNode;

		float beft = node.t;

		node.t = nodeBase + elementPosition.x/scr.scale + dragOffset;

		if( node.t < 0.0f )
			node.t = 0.0f;

		UpdatePrevNext(node.t,node.t < beft);

		////

		if( prevNode )
		{
			const Node &prev = *prevNode;

			float df = prev.frameLen;

			float last = prev.t + df*prev.frames + df*0.5f;

			if( node.t < last )
			{
				node.t = prev.t + df*floorf((node.t - prev.t + df*0.5f)/df);
			}
		}

		////

		int pos = mouseOff + elementPosition.x;

		offset = 0.0f;

		if( node.t < beft )
			scroll_l = true;

		if( scroll_l )
		{
			if( pos < 100 )
			{
				offset = (pos - 100)/scr.scale;
			}
		}

		if( nodesTail >= 0 )
		{
			const Node &tail = nodes[nodesTail];

			if( node.t > tail.t + tail.w )
				node.t = tail.t + tail.w;
		}
		else
		{
			if( node.t > 0.0f )
				node.t = 0.0f;
		}

		const Node &tail = nodesTail >= 0 ? nodes[nodesTail] : node;

		float r1 = tail.t + tail.width;
		float r2 = node.t + node.width;

		if( r2 >= r1 )
			if( beft < node.t && scr.size < r2 )
				scr.setSize(coremax(r1,r2));

		if( node.t > beft )
			scroll_r = true;

		if( scroll_r )
		{
			if( pos + 100 > scr.r.w )
			{
				offset = (pos + 100 - scr.r.w)/scr.scale;
			}
		}

		if( moveCursor && dragIndex == cursorNode )
		{
			cursorBeg = cursor = node.t + cursorOffset;
		}
	}
	else
	if( dragThumb )
	{
		scr.t.x = elementPosition.x;

		scr.t.x = coremax(scr.t.x,scr.r.x);
		scr.t.x = coremin(scr.t.x,scr.r.x + scr.r.w - scr.t.w);

		scr.setPos((scr.size - scr.page)*(scr.t.x - scr.r.x)/(scr.r.w - scr.t.w));
	}
	else
	if( dragEvent )
	{
		float x = evenBase + elementPosition.x/scr.scale;

		if( x < 0.0f )
			x = 0.0f;

		float rf = 0.5f*curNode->frameLen;
		float df = rf + rf;

		x = df*floorf((x + rf)/df);

		float last = df*(curNode->frames - 1);

		if( x > last )
			x = last;

		bool update = false;

		dword frame = dword((x + rf)/df);

	/*	if( dragEvent->frame != frame )
		{
			anime->MoveEvent(curNode->index,dragEventIndex,frame);

			update = true;
		}

		const array<ProjectAnimation::Event> &events = anime->GetNode(curNode->index).events;

		int y = elementPosition.y;

		if( y >= eve.r.y &&
			y <  eve.r.y + (int)events.Size()*c_evnt_h )
		{
			y -= eve.r.y;

			int i = y/c_evnt_h;

			if( i != dragEventIndex )
			{
				anime->ExchangeEvents(curNode->index,dragEventIndex,i);

				dragEvent = &events[i]; dragEventIndex = i;

			//	focusEvent = dragEvent;
			//	focusEventIndex = i;
				SelectEvent(dragEvent,focusEventNode,i);

				update = true;
			}
		}*/

		int j = (elementPosition.y - eve.r.y)/c_evnt_h + 1;

		if( j <  1 ) j =  1;
		if( j > 10 ) j = 10;

		if( dragEvent->position == j &&
			dragEvent->frame == frame )
			return;

		const array<ProjectAnimation::Event> &events = anime->GetNode(curNode->index).events;

		int left  = -1; // первый элемент слева
		int right = -1; // первый элемент справа

		int ei = dragEventIndex;

		for( int i = 0 ; i < events ; i++ )
		{
			if( i == ei )
				continue;

			const ProjectAnimation::Event &e = events[i];

			if( e.position == j )
			{
				if(	e.frame == frame )
				{
					return; // не ставим два евента в одном кадре на одной позиции
				}

				if( i < ei )
				{
					left = i;
				}

				if( i > ei )
				{
					if( right < 0 )
						right = i;
				}
			}
		}

		anime->MoveEvent		(curNode->index,ei,frame);
		anime-> SetEventPosition(curNode->index,ei,j);

		if( left < 0 )
		{
			if( right < 0 )
			{
				return; // больше нет евентов на данной позиции
			}
			else
			{
				if( frame < events[right].frame )
					return;
			}
		}
		else
		{
			if( right < 0 )
			{
				if( frame > events[left].frame )
					return;
			}
			else
			{
				if( frame > events[left ].frame &&
					frame < events[right].frame )
					return;
			}
		}

		//////////////

		if( left >= 0 && frame < events[left].frame )
		{
			for( ;; )
			{
				for( int i = ei - 1 ; i >= 0 ; i-- )
				{
					const ProjectAnimation::Event &e = events[i];

					if( e.position == j )
					{
						Assert(frame != e.frame)

						if( frame < e.frame )
						{
							anime->ExchangeEvents(curNode->index,i,ei);

							ei = i; continue;
						}
					}
				}

				break;
			}
		}
		else
		{
			for( ;; )
			{
				for( int i = ei + 1 ; i < events ; i++ )
				{
					const ProjectAnimation::Event &e = events[i];

					if( e.position == j )
					{
						Assert(frame != e.frame)

						if( frame > e.frame )
						{
							anime->ExchangeEvents(curNode->index,ei,i);

							ei = i; continue;
						}
					}
				}

				break;
			}
		}

		dragEvent = &events[ei]; dragEventIndex = ei;

	//	focusEvent = dragEvent;
	//	focusEventIndex = i;
		SelectEvent(dragEvent,focusEventNode,ei);

		//////////////

		update = true;

		UpdateSize(*curNode);

		/////////////////////////

		if( update )
			anime->SaveChanges();

		/////////////////////////
	}
	else // dragMeter
	{
		int epos = elementPosition.x;

		scr.setPos(metBase - epos/scr.scale + dragOffset);

		offset = 0.0f;

		if( metPos + epos < met.r.x )
		{
			offset = float(metPos + epos - met.r.x);
		}

		if( metPos + epos > met.r.x + met.r.w )
		{
			offset = float(metPos + epos - met.r.x - met.r.w);
		}
	}
}

//Прекратить операцию перетаскивания
void FormTimeLine::OnDragEnd()
{
	Assert(dragNode || dragThumb || dragEvent || dragMeter || dragCursor)

	if( dragCursor )
	{
	//	cursorNode = -1;
	//	cursorFrame = 0.0f;
		dragCursor = false;

		return;
	}

	if( dragNode )
	{
		dragOffset = 0; offset = 0;

		InsertNode(*dragNode,dragIndex);

		if( dragNodeNext >= 0 )
		{
			float off = 0.0f;
			const Node &node = nodes[dragNodeNext];

			if( node.prev >= 0 )
			{
				if( node.prev != dragIndex )
				{
					const Node &prev = nodes[node.prev];

					if( node.prev == dragIndex )
					{
						off = prev.t + dragNodeNextOffset - node.t;
					}
					else
					{
						off = prev.t + prev.w - node.t;
					}
				}
			}
			else
			{
				off = -node.t;
			}

			if( off )
			{
				Shift(dragNodeNext,off);

				scr.setSize(scr.size + off);
			}
		}

		dragNode = null;

		prevNode = null;
		nextNode = null;

	//	CheckCursor();

		if( moveCursor )
		{
			moveCursor = false;

			cursorBeg = cursor = nodes[cursorNode].t + cursorOffset;
		}
	}
	else
	if( dragThumb )
	{
		dragThumb = null;
	}
	else
	if( dragEvent )
	{
		const Node &tail = nodes[nodesTail];
		const Node &node = *curNode;

		float tr = tail.t + tail.width;
		float nr = node.t + node.width;

		scr.setSize(coremax(tr,nr));

		dragEvent = null; curNode = null;
	}
	else // dragMeter
	{
		dragOffset = 0; offset = 0;

		dragMeter = null;
	}
}

//Щелчёк левой кнопкой мыши
void FormTimeLine::OnMouseLeftClick(const GUIPoint &mousePos)
{
	//
}

//Щелчёк правой кнопкой мыши
void FormTimeLine::OnMouseRightClick(const GUIPoint &mousePos)
{
	const GUIRectangle &rect = GetClientRect();	GUIPoint pt(mousePos);

	const int mb_height = 130;

	if( pt.y > rect.h - mb_height )
		pt.y = rect.h - mb_height;

	pt += rect.pos; ClientToScreen(pt);

/*	if( cur.r.Inside(mousePos))
	{
		if( ani )
		{
			ani->Pause(false);

			ani->Goto("idle",0.3f);
		}

		cursorNode = -1;
		cursorFrame = 0.0f;

		cursor = -1000.0;

		animNode = null;

		animPause = false;

		return;
	}*/

	if( nodesHead >= 0 )
	{
		float pos_x = mousePos.x/scr.scale + scr.pos;
		int   pos_y = mousePos.y;

		Place place;

		if( FindElement(pos_x,pos_y,place))
		{
			int index = place.index; Node &node = nodes[index];

			SetFocus(&node,index);

			if( place.in )
			{
				if( editNodes )
				{
					indexNode = index;
					indexE	  = -1;

					selNode = &node;

					FormMessageBox *p = NEW FormMessageBox(
						options->GetString(SndOptions::s_form_warning),
						options->GetString(SndOptions::s_timeline_del_node),
						FormMessageBox::m_okcancel ,null,
						FormMessageBox::pp_left_top,&pt);

					p->onOk    .SetHandler(this,(CONTROL_EVENT)&FormTimeLine::OnDeleteElement);
					p->onCancel.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::OnIgnoreElement);

					options->gui_manager->ShowModal(p);
				}
			}
			else
			{
				if( place.e >= 0 )
				{
					if( editEvents && node.isEnable )
					{
						const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

						indexNode = place.index;
						indexE	  = place.e;

						selE = &events[place.e];

						SelectEvent(selE,indexNode,indexE);

						FormMessageBox *p = NEW FormMessageBox(
							options->GetString(SndOptions::s_form_warning),
							options->GetString(SndOptions::s_timeline_del_event),
							FormMessageBox::m_okcancel ,null,
							FormMessageBox::pp_left_top,&pt);

						p->onOk    .SetHandler(this,(CONTROL_EVENT)&FormTimeLine::OnDeleteElement);
						p->onCancel.SetHandler(this,(CONTROL_EVENT)&FormTimeLine::OnIgnoreElement);

						options->gui_manager->ShowModal(p);
					}
				}
			}
		}
	}
}

//Дельта от колеса
void FormTimeLine::OnMouseWeel(const GUIPoint &mousePos, long delta)
{
	if( dragMeter ||
		dragThumb || dragCursor || dragNode || dragEvent )
		return;

	if( animNode )
		autoScroll = false;

//	if( scr.r.Inside(mousePos))
	if( mousePos.y >= cur.r.y )
	{
		scr.scroll(delta*40/scr.scale);
	}
	else
	{
		if( delta > 0 )
		{
			scr.scale *= 1.1f; scr.pos += mousePos.x*(0.1f/scr.scale);
		}
		else
		{
			scr.pos -= mousePos.x*(0.1f/scr.scale); scr.scale /= 1.1f;
		}

		scr.setPage(scr.r.w/scr.scale);
	}
}

///////////////////////////////////////

bool FormTimeLine::Goto(const Node &node, float blend)
{
	AGNA_GotoNodeClip snc(anime->GetAnxNodeName(node.index),anime->GetNode(node.index).clipIndex,blend);

	bool r = ani->GetNativeGraphInfo(snc);
//	Assert(r)
	if( !r )
	{
		api->Trace("\nTimeLine: can't goto clip [%s:%d].\n",
			anime->GetAnxNodeName(node.index),
			anime->GetNode(node.index).clipIndex);

		return false;
	}

	///////////////////

	preview->SetCharactersGraphInfo(snc);

	return true;
}

void FormTimeLine::GotoIdle(float blend)
{
	AGNA_GotoNodeClip snc(null,0,blend);

	bool r = ani->GetNativeGraphInfo(snc);
	Assert(r)

	///////////////////

	preview->SetCharactersGraphInfo(snc);
}

void FormTimeLine::SetFrame(float frame)
{
	AGNA_SetCurrentFrame scf(frame);

	bool r = ani->GetNativeGraphInfo(scf);
	Assert(r)

	preview->SetCharactersGraphInfo(scf);
}

void FormTimeLine::ani_Pause(bool enable)
{
	Assert(ani)
	Assert(preview)

//	ani	   ->Pause(enable);
//	preview->Pause(enable);

	AGNA_AnimationPause ap(enable);

	bool r = ani->GetNativeGraphInfo(ap);
	Assert(r)

	preview->SetCharactersGraphInfo(ap);

	preview->Pause(enable);
}

void FormTimeLine::CheckEvents(const Node &node, float prevFrame, float currFrame)
{
	const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

	for( int i = 0 ; i < events ; i++ )
	{
		const ProjectAnimation::Event &e = events[i];

		float eventFrame = (float)e.frame;

		if((prevFrame < eventFrame && currFrame >= eventFrame) ||
		   (prevFrame > eventFrame && currFrame <= eventFrame))
		{
			anime->EventSoundPlay(node.index,i);

		/*	////

			const char *name = anime->GetEventSoundParams(node.index,i);

			if( string::NotEmpty(name))
			{
				preview->SelectLocator(name);
			}*/
		}
	}
}

void FormTimeLine::UpdateEvents()
{
	for( int i = 0 ; i < nodesMap ; i++ )
	{
		int anxNode = nodesMap.GetKey(i);

		const array<ProjectAnimation::Event> &events = anime->GetNode(anxNode).events;

		for( int j = 0 ; j < events ; j++ )
		{
			const char *locName = anime->GetEventSoundParams(anxNode,j);

			if( string::NotEmpty(locName))
			{
				int index = preview->GetLocatorIndex(locName);

				if( index >= 0 )
				{
					const Matrix &locMatrix = preview->GetLocatorMatrix(index);

					anime->EventSoundPosition(anxNode,j,locMatrix.pos);
				}
			}
		}
	}
}

void FormTimeLine::SelectEvent(const ProjectAnimation::Event *e, int node, int index)
{
	const char *name = null;

	if( node >= 0 && index >= 0 )
	{
		name = anime->GetEventSoundParams(nodes[node].index,index);
	}

	preview->SelectLocator(name);

	if( e != focusEvent )
	{
		focusEvent = e;

		if( focusEventIndex != index ||	focusEventNode != node )
		{
			if( focusEventIndex >= 0 &&	focusEventNode >= 0 )
			{
				int anxNode = nodes[focusEventNode].index;
				int index = focusEventIndex;

				anime->EventSoundStop(anxNode,index);
			}

			focusEventIndex = index;
			focusEventNode	= node;
		}

		onSelectEvent.Execute(this);
	}
}

void FormTimeLine::UpdateSize(Node &node)
{
	const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

	node.width = node.w;

	for( int j = 0 ; j < events ; j++ )
	{
		const ProjectAnimation::Event &e = events[j];

		float x = e.frame*node.frameLen;

		float tmin,tmax;

		bool looped = anime->EventSoundIsLoop(node.index,j);

		if( looped || !anime->GetEventSoundTime(node.index,j,tmin,tmax))
		{
			tmin = tmax = c_evnt_h/0.2f;
		}
		else
		{
			tmin *= 1000.0f;
			tmax *= 1000.0f;
		}

		if( tmax < c_evnt_h/0.2f )
			tmax = c_evnt_h/0.2f;

		if( node.width < x + tmax )
			node.width = x + tmax;
	}

	const Node &tail = nodes[nodesTail];

	float tr = tail.t + tail.width;
	float nr = node.t + node.width;

	float size = coremax(tr,nr);

	if( size > scr.size )
		scr.setSize(size);
}

void FormTimeLine::ScrollToCursor()
{
	if( autoScroll == false )
	{
		if( !dragThumb )
		{
			if( cursor - scr.pos < scr.page && scr.pos <= cursor )
			{
				autoScroll = true;
			}
		}
	}

	if( autoScroll == false )
		return;

	if( cursor - scr.pos >= scr.page )
	{
		if( animLocal )
		{
		//	const Node &node = *focus;
			const Node &node = nodes[cursorNode];

			scr.scroll(scr.pos - (node.t + node.w - scr.page));
		}
		else
		{
			scr.scroll(scr.pos - cursor);
		}
	}
	else
	{
	//	const Node &node = animLocal ? *focus : nodes[cursorNode];
		const Node &node = nodes[cursorNode];

		if( scr.pos > cursor )
		{
			scr.scroll(scr.pos - node.t);
		}
	}
}

void FormTimeLine::SetFocus(const Node *node, int index, bool reset)
{
	if( focus && focus != node )
	{
		SelectEvent(null);
	}

	focus = node;
	focusIndex = index;

	reset = false;

	if( reset )
	{
		if( ani )
		{
			ani_Pause(false);

		//	ani->Goto("idle",0.3f);
		//	preview->SetIdle();
			GotoIdle(0.3f);

			animNode = null;

			animTime = 0.0f; animPrev = -0.001f;
			animStep = 0.0f;

			cursor = -1000.0f;

		//	preview->bPlay->image = &options->imagePlay;
		}

		cursor = -1000.0f;
	}
}

void FormTimeLine::MoveCursor(float t, const Node &node, float &cursFrame, int &cursNode, bool align)
{
	float frame = (t - node.t)/node.frameLen;

	if( frame < 0.0f )
	{
		if( node.prev >= 0 )
		{
			Node &prev = nodes[node.prev];

			if( prev.index >= 0 && prev.valid && Goto(prev))
			{
			//	SetFocus(&prev,node.prev,false);

				frame = (t - prev.t)/prev.frameLen;

				if( frame < 0.0f )
					frame = 0.0f;

			//	ani->Goto(prev.name.str,0.0f);
			//		 Goto(prev);

				if( align )
					frame = floorf(frame);

			/*	AGNA_SetCurrentFrame scf(frame);

				bool r = ani->GetNativeGraphInfo(scf);
				Assert(r)
			//	mustSetFrame = 2;*/
				SetFrame(frame);

			//	moveLeft = true;
				CheckEvents(node,-0.1f,0.9f);
				CheckEvents(prev,frame - 0.9f,frame);

				if( align )
					cursor = prev.t + frame*prev.frameLen;
				else
					cursor = t;

				cursorBeg = cursor;

				cursFrame = frame;

				cursNode = node.prev;

				return;
			}
		}
		
		frame = 0.0f;
	}

	float count = (float)node.frames;

	if( node.next >= 0 )
	{
		Node *next = &nodes[node.next]; float r = node.frameLen*0.5f;

		if( next->t < node.t + node.w - r )
		{
			count -= int((node.t + node.w - next->t + r)/node.frameLen);
		}
	}

	float dt = 1.0f/scr.scale; // размер пикселя

	if( frame >= count )
	{
		if( node.next >= 0 )
		{
			Node &next = nodes[node.next];

			if( next.index >= 0 && next.valid && Goto(next))
			{
			//	SetFocus(&next,node.next,false);

				frame = (t - next.t)/next.frameLen;

				count = (float)(next.frames - 1);

				if( frame >= count )
					frame  = count;

			//	ani->Goto(next.name.str,0.0f);
			//		 Goto(next);

				if( align )
					frame = floorf(frame);

			/*	AGNA_SetCurrentFrame scf(frame);

				bool r = ani->GetNativeGraphInfo(scf);
				Assert(r)
			//	mustSetFrame = 2;*/
				SetFrame(frame);

				CheckEvents(next,frame - 0.9f,frame);

				if( align )
					cursor = next.t + frame*next.frameLen;
				else
					cursor = t;

				cursorBeg = cursor;

				cursFrame = frame;

				cursNode = node.next;

				return;
			}
		}

		frame = count - dt/node.frameLen;
	}

	if( align )
		frame = floorf(frame);

	if( cursFrame != frame )
	{
	/*	AGNA_SetCurrentFrame scf(frame);

		bool r = ani->GetNativeGraphInfo(scf);
		Assert(r)*/
		SetFrame(frame);

		UpdateEvents();

		CheckEvents(node,cursFrame,frame);

		if( t < node.t )
			t = node.t;

		if( t > node.t + node.w - dt )
			t = node.t + node.w - dt;

		if( align )
			cursor = node.t + frame*node.frameLen;
		else
			cursor = t;

		cursorBeg = cursor;

		cursFrame = frame;
	}
}

void FormTimeLine::CheckCursor()
{
	if( cursor >= scr.size )
	{
		if( nodesHead >= 0 )
		{
			End(null);
		}
		else
		{
			if( ani )
			{
				ani_Pause(false);

			//	ani->Goto("idle",0.3f);
			//	preview->SetIdle();
				GotoIdle(0.3f);
			}

			cursorNode = -1;
			cursorFrame = 0.0f;

			cursor = -1000.0;

			animNode = null;

			animPause = false;
		}
	}
}

void FormTimeLine::Trim(const Node &node)
{
	if( node.next >= 0 )
	{
		const Node &next = nodes[node.next];

		float offset = -next.t;

		if( node.prev >= 0 )
		{
			const Node &prev = nodes[node.prev];

			offset = prev.t + prev.w - next.t;
		}

		if( offset )
		{
			int index = node.next;

			for( ;; )
			{
				Node &next = nodes[index];

				next.t += offset;

				if( index == cursorNode )
				{
					cursorBeg = cursor += offset;
				}

				if( index == nodesTail )
					break;

				index = next.next;
			}
		}
	}
}

void FormTimeLine::DeleteNode(int index)
{
	Node &node = nodes[index];

	if( node.index >= 0 && node.valid )
	{
		int mapIndex = nodesMap.Find(node.index);
		Assert(mapIndex >= 0)

		if( --nodesMap.GetVal(mapIndex) <= 0 )
			nodesMap.Del(mapIndex);
	}

	if( index == cursorNode )
	{
		if( node.next >= 0 && nodes[node.next].valid )
		{
			const Node &next = nodes[node.next];

			cursorNode = node.next;

			cursorFrame = 0.0f;

			cursor = cursorBeg = next.t;

			preview->LockEvents(lockEvents = true);

			Goto(next);

		/*	AGNA_SetCurrentFrame scf(cursorFrame);

			bool r = ani->GetNativeGraphInfo(scf);
			Assert(r)
		//	mustSetFrame = 2;*/
			SetFrame(cursorFrame);
		}
		else
		if( node.prev >= 0 && nodes[node.prev].valid )
		{
			const Node &prev = nodes[node.prev];

			cursorNode = node.prev;

			cursorFrame = prev.frames - 1.0f/scr.scale/node.frameLen;

			cursor = cursorBeg = prev.t + cursorFrame*prev.frameLen;

			preview->LockEvents(lockEvents = true);

			Goto(prev);

		/*	AGNA_SetCurrentFrame scf(cursorFrame);

			bool r = ani->GetNativeGraphInfo(scf);
			Assert(r)
		//	mustSetFrame = 2;*/
			SetFrame(cursorFrame);
		}
		else
		{
			// вызовется CheckCursor() в конце функции
		}
	}

	Trim(node);

	if( focus == &node )
	{
		if( node.next >= 0 )
		{
			SetFocus(&nodes[node.next],node.next);
		}
		else
		if( node.prev >= 0 )
		{
			SetFocus(&nodes[node.prev],node.prev);
		}
		else
		{
			SetFocus(null,-1);
		}
	}

	RemoveNode(node,index);

///////////////////////////

//	nodes.DelIndex(index);

	node.next = nodesFree;

	nodesFree = index;

///////////////////////////

	if( nodesTail < 0 )
	{
		scr.setSize(0);
	}
	else
	{
		Expand();
	}

	CheckCursor();

	////////////////////////////////////

	ExportNodes();

	if( node.index >= 0 && node.valid )
	{
		anime->DeleteEvents(node.index,movieIndex);
	}

	anime->SaveChanges();

	////////////////////////////////////
}

int FormTimeLine::GetFreeNode()
{
	if( nodesFree < 0 )
	{
		dword index = nodes.Add();

		if( focus )
			focus = &nodes[focusIndex];

		return index;
	}
	else
	{
		int index = nodesFree;

		nodesFree = nodes[index].next;

		return index;
	}
}

void FormTimeLine::Expand()
{
	float width = 0;

	for( int index = nodesHead ; index >= 0 ; )
	{
		const Node &node = nodes[index];

		if( width < node.t + node.width )
			width = node.t + node.width;

		index = node.next;
	}

	scr.setSize(width);
}

void FormTimeLine::Shift(int index, float off)
{
	for( ;; )
	{
		Node &node = nodes[index];

		node.t += off;

		if( index == cursorNode )
		{
			cursorBeg = cursor += off;
		}

		if( index == nodesTail )
			break;

		index = node.next;
	}
}

void FormTimeLine::StepNodeL()
{
	Node &node = nodes[focusIndex];

	if( node.prev >= 0 )
	{
		Node &prev = nodes[node.prev];
	
		float df = prev.frameLen;

		if( node.t > prev.t + df*1.5f )
		{
			float t = prev.t + df*floorf((node.t - prev.t - df*0.5f)/df);
			float d = t - node.t;

			Shift(focusIndex,d);

			scr.setSize(scr.size + d);

			////////////////////////////////////

			ExportNodes(); anime->SaveChanges();

			////////////////////////////////////
		}
	}
}

void FormTimeLine::StepNodeR()
{
	Node &node = nodes[focusIndex];

	if( node.prev >= 0 )
	{
		Node &prev = nodes[node.prev];
	
		float df = prev.frameLen;

		if( node.t < prev.t + df*prev.frames )
		{
			float t = prev.t + df*floorf((node.t - prev.t + df*1.5f)/df);
			float d = t - node.t;

			Shift(focusIndex,d);

			scr.setSize(scr.size + d);

			////////////////////////////////////

			ExportNodes(); anime->SaveChanges();

			////////////////////////////////////
		}
	}
}

void FormTimeLine::InsertNode(Node &node, int curr, int after)
{
	if( nodesHead < 0 )
	{
		nodesHead = curr;
		nodesTail = curr;

		node.prev = -1;
		node.next = -1;
	}
	else
	{
		Node &head = nodes[nodesHead];
		Node &tail = nodes[nodesTail];

		if( node.t < head.t )
		{
			node.prev = -1;
			node.next = nodesHead;

			head.prev = curr;

			nodesHead = curr;

			node.t = 0;
		}
		else
		if( node.t >= tail.t && (after < 0 || after == nodesTail))
		{
			node.prev = nodesTail;
			node.next = -1;

			tail.next = curr;

			nodesTail = curr;
		}
		else
		{
			int index = (after >= 0 ? after : nodesHead);

			for( ;; )
			{
				Node &prev = nodes[index];
				Node &next = nodes[prev.next];

				if( node.t <= next.t || after >= 0 )
				{
					node.prev = index;
					node.next = prev.next;

					prev.next = curr;
					next.prev = curr;

					if( node.t > prev.t + prev.w )
						node.t = prev.t + prev.w;

					float off = 0.0f;
					
					if( &node == dragNode && node.next == dragNodeNext )
					{
						off = node.t + dragNodeNextOffset - next.t;
					}
					else
					{
						off = node.t + node.w - next.t;
					}

					if( off )
					{
						Shift(node.next,off);
					}

					break;
				}

				if( prev.next == nodesTail )
					break;

				index = prev.next;
			}
		}
	}

	Expand();

	SetFocus(&node,curr);

/*	float beg = node.t;
	float end = node.t + node.w - scr.page;

	if( scr.pos < end )
		scr.pos = end;

	if( scr.pos > beg )
		scr.pos = beg;

	scr.update();*/

	if( loading )
		return;

	////////////////////////////////////

	ExportNodes(); anime->SaveChanges();

	////////////////////////////////////
}

void FormTimeLine::RemoveNode(Node &node, int index)
{
	if( nodesHead == index )
	{
		if( nodesTail == index )
		{
			nodesHead = -1;
			nodesTail = -1;
		}
		else
		{
			nodesHead = node.next;

			nodes[node.next].prev = -1;
		}
	}
	else
	if( nodesTail == index )
	{
		if( nodesHead == index )
		{
			nodesHead = -1;
			nodesTail = -1;
		}
		else
		{
			nodesTail = node.prev;

			nodes[node.prev].next = -1;
		}
	}
	else
	{
		nodes[node.prev].next = node.next;
		nodes[node.next].prev = node.prev;
	}

	node.prev = -1;
	node.next = -1;
}

void FormTimeLine::ClearNodes()
{
	nodesMap.DelAll();

	nodesHead = -1;
	nodesTail = -1;

	for( dword i = 0 ; i < nodes.Size() ; i++ )
	{
		Node &node = nodes[i];

		node.prev = -1;
		node.next = i < nodes.Last() ? i + 1 : -1;
	}

	nodesFree = nodes ? 0 : -1;

	SetFocus(null,-1);

	scr.setSize(0);

	SelectEvent(null);
}

bool FormTimeLine::FindElement(float t, int y, Place &place)
{
	place.index = -1; place.in = false;
	place.e		= -1;

	for( int index = nodesTail ; index >= 0 ; )
	{
		Node &node = nodes[index];

		bool in = t < node.t + node.w;

		if( t >= node.t && (in || index == nodesTail))
		{
			if( node.next >= 0 )
			{
				const Node &next = nodes[node.next];

				if( t >= next.t + next.w )
					return false;
			}

			place.index = index;

			if( y >= 0 && y < c_node_h )
			{
				if( in )
				{
					place.in = true;
					
					return true;
				}
				else
					return false;
			}

			if( y >= eve.r.y &&
				y <  eve.r.y + eve.r.h )
			{
				const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

				t -= node.t;
				y -=  eve.r.y;

				int j = y/c_evnt_h + 1;

				for( int i = events - 1 ; i >= 0 ; i-- )
				{
					const ProjectAnimation::Event &e = events[i];

					Assert(e.position)

					if( e.position == j )
					{
						float x = e.frame*node.frameLen;

						float tmin,tmax;

						bool looped = anime->EventSoundIsLoop(node.index,i);

						if( looped || !anime->GetEventSoundTime(node.index,i,tmin,tmax))
						{
							if( looped )
							{
								tmin = tmax = 60/scr.scale;
							}
							else
							{
							//	tmin = tmax = c_evnt_h/0.2f;
								tmin = tmax = node.frameLen;
							}
						}
						else
						{
							tmin *= 1000.0f;
							tmax *= 1000.0f;
						}

					//	float block = c_evnt_h/0.2f/scr.scale;
						float block = node.frameLen;

						float bl = c_evnt_h/scr.scale;

						if( block < bl )
							block = bl;

						if( tmax < block )
							tmax = block;

						if( t >= x && t < x + tmax )
						{
							place.e = i;

							return true;
						}
					}
				}

			//	return true;
				return in;
			}
		}

		index = node.prev;
	}

	return false;
}

void FormTimeLine::UpdatePrevNext(float t, bool left)
{
	if( nodesHead >= 0 )
	{
		if( left )
		{
			if( prevNode && t < prevNode->t )
			{
				for( ;; )
				{
					int pr = prevNode->prev;

					prevNode = pr >= 0 ? &nodes[pr] : null;

					if( !prevNode || prevNode->t <= t )
						break;
				}
			}

			if( prevNode )
			{
				int ne = prevNode->next;

				nextNode = ne >= 0 ? &nodes[ne] : null;
			}
			else
			{
				nextNode = &nodes[nodesHead];
			}
		}
		else
		{
			if( nextNode && t >= nextNode->t )
			{
				for( ;; )
				{
					int ne = nextNode->next;

					nextNode = ne >= 0 ? &nodes[ne] : null;

					if( !nextNode || nextNode->t > t )
						break;
				}
			}

			if( nextNode )
			{
				int pr = nextNode->prev;

				prevNode = pr >= 0 ? &nodes[pr] : null;
			}
			else
			{
				prevNode = &nodes[nodesTail];
			}
		}
	}
}

void FormTimeLine::ExportNodes()
{
	array<ProjectAnimation::Node> &data = anime->GetMovieNodes(movieIndex);

	data.DelAll();

	for( int index = nodesHead ; index >= 0 ; )
	{
		ProjectAnimation::Node &item = data[data.Add()];

		const Node &node = nodes[index];

		if( node.next >= 0 )
		{
			const Node &next = nodes[node.next];

			float df = node.frameLen;

			int count = int((next.t - node.t + df*0.5f)/df);

			if( count > node.frames )
				count = node.frames;

			item.framesCount = count;
		}
		else
		{
			item.framesCount = node.frames;
		}

		item.index	  = node.index;
		item.isEnable = node.isEnable != 0;

		index = node.next;
	}
}

/////////////////////////////////////////////////////////////

void _cdecl FormTimeLine::OnIgnoreElement(GUIControl *sender)
{
	selNode = null;
	selE	= null;
}

void _cdecl FormTimeLine::OnDeleteElement(GUIControl *sender)
{
	selNode = null;
	selE	= null;

	if( indexE >= 0 )
	{
		SelectEvent(null);

		Node &node = nodes[indexNode];

		anime->DeleteEvent(node.index,indexE);

		UpdateSize(node);

		/////////////////////

		anime->SaveChanges();

		/////////////////////
	}
	else
	{
		DeleteNode(indexNode);
	}
}

/////////////////////////////////////////////////////////////

void FormTimeLine::SetMovie(const UniqId & animationId, int indexMovie)
{
	if( !animationId.IsValidate() )
	{
		ClearNodes();

		animeId.Reset();
		movieIndex = -1;

		ani = preview->Init(null,null);

		return;
	}

	if( animeId == animationId &&
		movieIndex == indexMovie )
	{
		if( preview->GetMissionName() ==
			  anime->GetMissionName(indexMovie))
		{
			return;
		}
	}

	animeId = animationId;
	movieIndex = indexMovie;

	//////////////////

	focusEvent = null;

	focusEventIndex = -1;
	focusEventNode	= -1;

	focus = null;
	focusIndex = -1;

	play_delay = 0;

	//////////////////

	ClearNodes();

	scr.scale = 0.2f;

	scr.size = 0.0f;
	scr.pos	 = 0.0f;

	scr.page = scr.r.w/scr.scale;

	if( anime )
	{
		//

		anime = null;
	}

	lockEvents = false;

	anime = project->AnimationGet(animeId);
	Assert(anime)

	if( ani = anime->LoadAnimation())
	{
		ani = preview->Init(anime->GetMissionName(indexMovie),anime->GetAnimationName());
		anime->ReleaseAnimation();
		blendTime = preview->GetBlend();
	}

	loading = true;

	array<ProjectAnimation::Node> &data = anime->GetMovieNodes(movieIndex);

	for( int i = 0 ; i < data ; i++ )
	{
		const ProjectAnimation::Node &item = data[i];

		float offset = 0.0f;

		if( i > 0 )
		{
			int framesCount = data[i - 1].framesCount;

			const Node &prev = nodes[nodesTail];

			if( framesCount < prev.frames )
			{
				float width = framesCount*prev.frameLen;

				offset = width - prev.w;
			}
		}

		AddNode(item.index,item.isEnable,offset);
	}

	loading = false;

	////

	animNode = null;

	animTime = 0.0f; animPrev = -0.001f;
	animStep = 0.0f;

	cursor = -1000.0f;

	initAnim = false;

	mustSetFrame = 0;

	autoScroll = true;

	if( ani )
	{
		AGNA_BlockControl bc(true);

		bool r = ani->GetNativeGraphInfo(bc);
		Assert(r)

		preview->SetCharactersGraphInfo(bc);

		////////////////////

		if( nodesHead >= 0 && nodes[nodesHead].valid )
		{
			initAnim = true;

		//	mustSetFrame = 10;
			mustSetFrame =  2;

			cursor = 0.0f;

			cursorNode	= nodesHead;
			cursorFrame = 0.0f;

			cursorBeg = cursor;

			ani_Pause(false);
		}
		else
		{
			ani_Pause(false);

		//	ani->Goto("idle",0.3f);
		//	preview->SetIdle();
			GotoIdle(0.3f);
		}
	}

	////

	if( nodesHead >= 0 )
	{
		const Node &node = nodes[nodesHead];

		SetFocus(&node,nodesHead);

		float beg = node.t;
		float end = node.t + node.w - scr.page;

		if( scr.pos < end )
			scr.pos = end;

		if( scr.pos > beg )
			scr.pos = beg;

		scr.update();
	}
}

void FormTimeLine::AddNode(int indexNode, bool isEnable, float offset)
{
	if( anime )
	{
		int curr = GetFreeNode(); Node &node = nodes[curr];

		node.index = indexNode;

		if( node.index >= 0 )
		{
			const ProjectAnimation::PrjAnxNode &anxNode = anime->GetNode(indexNode);

			Assert(anxNode.fps > 0.0f)
			Assert(anxNode.clipFrames > 0)

			node.frames = anxNode.clipFrames - 1;

			node.fps = anxNode.fps;
			node.frameLen = 1.0f/node.fps*1000.0f;

			node.isEnable = isEnable;

			anime->GetNodeName(indexNode,node.name.data);
			node.name.Init(node.name.data.c_str());

			const char *buf = node.name.data.c_str();

			node.name.w = long(smallFont->GetWidth (buf) + 0.4999f) + 1;
			node.name.h = long(smallFont->GetHeight(buf) + 0.4999f) + 1;

			const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

			node.w = node.frames*node.frameLen;

			node.width = node.w;

			for( int j = 0 ; j < events ; j++ )
			{
				const ProjectAnimation::Event &e = events[j];

				float x = e.frame*node.frameLen;

				float tmin,tmax;

				bool looped = anime->EventSoundIsLoop(node.index,j);

				if( looped || !anime->GetEventSoundTime(node.index,j,tmin,tmax))
				{
					tmin = tmax = c_evnt_h/0.2f;
				}
				else
				{
					tmin *= 1000.0f;
					tmax *= 1000.0f;
				}

				if( tmax < c_evnt_h/0.2f )
					tmax = c_evnt_h/0.2f;

				if( node.width < x + tmax )
					node.width = x + tmax;
			}

			node.valid = anime->CheckNode(node.index);
		//	node.valid = curr%2 ? false : true;

			if( node.valid )
			{
				nodesMap[node.index]++;
			}
		}
		else
		{
			node.frames = 1;

			node.fps = 0.0f;
			node.frameLen = 500.0f;

			node.isEnable = FALSE;

			node.name.str = "[bad node]";

		/*	const char *buf = node.name.str;

			node.name.w = long(smallFont->GetWidth (buf) + 0.4999f) + 1;
			node.name.h = long(smallFont->GetHeight(buf) + 0.4999f) + 1;*/
			node.name.w = 0;
			node.name.h = 0;

			node.w = node.frames*node.frameLen;

			node.width = node.w;
		}

		node.t = offset;

		if( focus )
		{
			node.t += focus->t + focus->w;
		}
		else
		{
			node.t += scr.size;
		}

		InsertNode(node,curr,focusIndex);

		if( ani && loading == false )
		{
			if( nodesHead == nodesTail )
			{
				if( node.index >= 0 && node.valid )
				{
					Begin(null);
				}
			}
		}
	}
}

void FormTimeLine::PlayEvent()
{
	if( focusEvent )
	{
		int anxNode =  focus->index;
		int index = focusEventIndex;

		const char *locName = anime->GetEventSoundParams(anxNode,index);

		if( string::NotEmpty(locName))
		{
			int locIndex = preview->GetLocatorIndex(locName);

			if( locIndex >= 0 )
			{
				const Matrix &locMatrix = preview->GetLocatorMatrix(locIndex);

				anime->EventSoundPosition(anxNode,index,locMatrix.pos);
			}
		}

		anime->EventSoundPlay(anxNode,index);
	}
}

void FormTimeLine::StopEvent()
{
	if( focusEvent )
	{
		int anxNode =  focus->index;
		int index = focusEventIndex;

		anime->EventSoundStop(anxNode,index);
	}
}

///////////////////////////////////////////////////

void _cdecl FormTimeLine::Begin(GUIControl *sender)
{
	const Node &head = nodes[nodesHead];

	if( head.index < 0 || !head.valid )
		return;

	ani_Pause(false);

//	ani->Goto(head.name.str,0.0f);
//		 Goto(head);

	bool ok = Goto(head);

	ani_Pause(true);

	if( !ok )
		return;

/*	AGNA_SetCurrentFrame scf(0.0f);

	bool r = ani->GetNativeGraphInfo(scf);
	Assert(r)
//	mustSetFrame = 2;*/
	SetFrame(0.0f);

	cursor = 0.0f;

	SetFocus(&head,nodesHead,false);

	cursorBeg = cursor;

	cursorNode	= nodesHead;
	cursorFrame = 0.0f;

	ScrollToCursor();
}

void _cdecl FormTimeLine::End(GUIControl *sender)
{
	const Node &tail = nodes[nodesTail];

	if( tail.index < 0 || !tail.valid )
		return;

	ani_Pause(false);

//	ani->Goto(tail.name.str,0.0f);
//		 Goto(tail);

	bool ok = Goto(tail);

	ani_Pause(true);

	if( !ok )
		return;

	float frame = tail.frames - 1.0f/scr.scale/tail.frameLen;

/*	AGNA_SetCurrentFrame scf(frame);

	bool r = ani->GetNativeGraphInfo(scf);
	Assert(r)
//	mustSetFrame = 2;*/
	SetFrame(frame);

	cursor = tail.t + frame*tail.frameLen;

	SetFocus(&tail,nodesTail,false);

	cursorBeg = cursor;

	cursorNode	= nodesTail;
	cursorFrame = frame;

	ScrollToCursor();
}

void _cdecl FormTimeLine::NodeBegin(GUIControl *sender)
{
	const Node &node = nodes[cursorNode];

/*	AGNA_SetCurrentFrame scf(0.0f);

	bool r = ani->GetNativeGraphInfo(scf);
	Assert(r)*/
	SetFrame(0.0f);

	cursor = node.t;

	cursorBeg = cursor;

	cursorFrame = 0.0f;

	ScrollToCursor();
}

void _cdecl FormTimeLine::NodeEnd(GUIControl *sender)
{
	const Node &node = nodes[cursorNode];

	dword nodeFrames = GetNodeFrames(node);

	float frame = nodeFrames - 1.0f/scr.scale/node.frameLen;

/*	AGNA_SetCurrentFrame scf(frame);

	bool r = ani->GetNativeGraphInfo(scf);
	Assert(r)*/
	SetFrame(frame);

	cursor = node.t + frame*node.frameLen;

	cursorBeg = cursor;

	cursorFrame = frame;

	ScrollToCursor();
}

void _cdecl FormTimeLine::Prev(GUIControl *sender)
{
	const Node &node = nodes[cursorNode];

	float t = cursor - node.t;

	float r = node.frameLen*0.5f;

	if( cursorFrame < 1.0f && node.prev >= 0 )
	{
		const Node &prev = nodes[node.prev];

		r = prev.frameLen*0.5f;
	}

	float dt = fmodf(t,node.frameLen);

	if( dt > 0.01f )
		t -= dt*0.5f;
	else
		t -= r;

	MoveCursor(node.t + t,node,cursorFrame,cursorNode,true);

	if( cursor >= scr.pos + scr.page )
	{
		const Node &node = nodes[cursorNode];

		scr.scroll(scr.pos - (node.t + node.w - scr.page));
	}
	else
		ScrollToCursor();

	cursorBeg = cursor;
}

void _cdecl FormTimeLine::Next(GUIControl *sender)
{
	const Node &node = nodes[cursorNode];

	float t = cursor - node.t;

	float r = node.frameLen*0.5f;

	if( cursorFrame > float(node.frames - 1) && node.next >= 0 )
	{
		const Node &next = nodes[node.next];

		r = next.frameLen*0.5f;
	}

	float dt = node.frameLen - fmodf(t,node.frameLen);

	if( dt > 0.001f )
		t += dt + r;
	else
		t += node.frameLen + r;

	if( t >= node.w && node.next < 0 )
	{
		t = node.w - 1.0f/scr.scale;

		MoveCursor(node.t + t,node,cursorFrame,cursorNode,false);
	}
	else
	{
		MoveCursor(node.t + t,node,cursorFrame,cursorNode,true);
	}

	if( cursor >= scr.pos + scr.page )
	{
		const Node &node = nodes[cursorNode];

		scr.scroll(scr.pos - (node.t + node.w - scr.page));
	}
	else
		ScrollToCursor();

	cursorBeg = cursor;
}

void _cdecl FormTimeLine::PlayNode(GUIControl *sender)
{
	autoScroll = true;

	if( animNode )
	{
		play_local = true;

		Stop(null);

		play_delay = 2;	return;
	}

	if( animPause )
	{
		animPause = false; ani_Pause(false);

		return;
	}

	bool inPlay = false;

	if( animNode )
	{
		StopEvents(*animNode);

		inPlay = true;
	}

	StopEvent();

	animLocal = true;

//	const Node &node = *focus;
	const Node &node = nodes[cursorNode];

	float frame = 0.0f;

	cursor = node.t;

//	ani->Goto(node.name.str,0.0f);
		 Goto(node);

/*	AGNA_SetCurrentFrame scf(frame);

	bool r = ani->GetNativeGraphInfo(scf);
	Assert(r)
//	mustSetFrame = 2;*/
	SetFrame(frame);

	CheckEvents(node,frame - 0.9f,frame);

	ani_Pause(false);

	if( !inPlay )
	{
		dword prepareFrames = GetNodeFrames(node);

		anime->NodePrepareForSoundPlay(node.index,prepareFrames);
	}

	animNode = &node;

	animTime = 0.0f;
	animStep = 1.0f/node.fps;

	initPlay = true;

	if( inPlay )
	{
		preview->CharacterReset();
	}
}

void _cdecl FormTimeLine::Play(GUIControl *sender)
{
	options->sa->EditStopAllSounds(null);
	autoScroll = true;

	if( animNode )
	{
		play_local = false;

		Stop(null);

		play_delay = 2;	return;
	}

	if( animPause )
	{
		animPause = false; ani_Pause(false);

		return;
	}

	bool inPlay = false;

	if( animNode )
	{
		StopEvents(*animNode);

		inPlay = true;
	}

	StopEvent();

	if( focus )
	{
		if( inPlay )
		{
			preview->CharacterReset();
		}

	//	const Node &node = *focus;
		const Node &node = nodes[cursorNode];

	//	float frame = 0.0f;
		float frame = cursorFrame;

		cursor = cursorBeg;

		{
		//	ani->Goto(node.name.str,0.0f);
		//		 Goto(node);

		//	if( animNode && animNode != &node )
			{
				Goto(node);
			}

		/*	AGNA_SetCurrentFrame scf(frame);

			bool r = ani->GetNativeGraphInfo(scf);
			Assert(r)
		//	mustSetFrame = 2;*/
			SetFrame(frame);

			CheckEvents(node,frame - 0.9f,frame);

			ani_Pause(false);

		/*	for( int i = 0 ; i < nodesMap ; i++ )
			{
				int anxNode = nodesMap.GetKey(i);

				dword prepareFrames = anime->GetNode(anxNode).clipFrames;

				anime->NodePrepareForSoundPlay(anxNode,prepareFrames);
			}*/

			if( !inPlay )
			{
				PrepareForSoundPlay();

			//	preview->ActivateTriggers();
			}
		}

		animNode = &node;

	//	animTime = 0.0f;
		animStep = 1.0f/node.fps;

		animTime = animStep*frame;

		initPlay = true;
	}
}

void _cdecl FormTimeLine::Pause(GUIControl *sender)
{
	options->sa->EditStopAllSounds(null);
	const Node &node = *animNode;

	StopEvents(node);

	ani_Pause(true);

	animPause = true;
}

void _cdecl FormTimeLine::Stop(GUIControl *sender)
{
	autoScroll = true;

	Pause(null);

	animPause = false;

	animNode = null;

	preview->CharacterReset();

	if( cursorNode >= 0 )
	{
		const Node &node = nodes[cursorNode];

	//	SetFocus(&node,cursorNode,false);

		preview->LockEvents(lockEvents = true);

	//	ani->Goto(node.name.str,0.0f);
			 Goto(node);

	/*	AGNA_SetCurrentFrame scf(cursorFrame);

		bool r = ani->GetNativeGraphInfo(scf);
		Assert(r)
	//	mustSetFrame = 2;*/
		SetFrame(cursorFrame);

		preview->LockEvents(lockEvents = false);

		cursor = cursorBeg;

		if( !animLocal )
		{
			ScrollToCursor();
		}
	}
	else
	{
		ani_Pause(false);

	//	ani->Goto("idle",0.3f);
	//	preview->SetIdle();
		GotoIdle(0.3f);

		cursor = -1000.0f;
	}

	animLocal = false;

	options->sa->EditStopAllSounds(null);
}

void _cdecl FormTimeLine::UpdateSounds(GUIControl *sender)
{
	if(anime)
	{
		if(preview)
		{
			Vector pos, dir;
			preview->GetListenerParameters(pos, dir);
			anime->EventsSetListener(pos, pos - dir);
		}
		anime->EventSoundDraw();
	}
}


///////////////////////////////////////////////////

void FormTimeLine::PrepareForSoundPlay()
{
	for( int index = nodesHead ; index >= 0 ; )
	{
		const Node &node = nodes[index];

		dword prepareFrames = node.frames;

		if( node.next >= 0 )
		{
			const Node &next = nodes[node.next];

			float df = node.frameLen;

			int count = int((next.t - node.t + df*0.5f)/df);

			if( count > node.frames )
				count = node.frames;

			prepareFrames = count;
		}

		anime->NodePrepareForSoundPlay(node.index,prepareFrames);

		index = node.next;
	}
}

void FormTimeLine::UpdateButtons()
{
	if( nodesTail >= 0 && !dragNode )
	{
		bool en = ani && focus && focus->index >= 0 && focus->valid/* && play_delay < 1*/;

		const Node &tail = nodes[nodesTail];

		float tt = 1.5f/scr.scale;

		bool onBeg = cursor >= 0.0f && cursor <= tt;
		bool onEnd = cursor >= 0.0f && cursor >= tail.t + tail.w - tt;

		bool play = animNode != null || play_delay > 0;

		bool stepL = en && cursor >= 0.0f && !onBeg && !play;
		bool stepR = en && cursor >= 0.0f && !onEnd && !play;

		bool onNodeBeg = false;
		bool onNodeEnd = false;

		if( cursorNode >= 0 )
		{
			const Node &node = nodes[cursorNode];

			dword nodeFrames = GetNodeFrames(node);

			onNodeBeg = cursor <= node.t + tt;
			onNodeEnd = cursor >= node.t + nodeFrames*node.frameLen - tt;
		}

		preview->bBegin->Enabled = en && !onBeg && !play;
		preview->bEnd  ->Enabled = en && !onEnd && !play;

		preview->bPrev->Enabled = stepL;
		preview->bNext->Enabled = stepR;

		preview->bNodeBegin->Enabled = stepL && !onNodeBeg;
		preview->bNodeEnd  ->Enabled = stepR && !onNodeEnd;

		preview->bPlay->Enabled = en && cursor >= 0.0f && !onEnd && !animLocal/* && play_delay < 1*/;

	//	preview->bPause->Enabled = play && !animPause;
		preview->bStop ->Enabled = play;

		play = animNode != null;

	//	preview->bPlayNode->Enabled = en && (!play || animLocal);
		preview->bPlayNode->Enabled = en && (!play || animLocal) && cursor >= 0.0f/* && play_delay < 1*/;

		if( play_delay > 0 )
		{
			if( play_local )
			{
				preview->bPlay->Enabled = false;
			}
			else
			{
				preview->bPlayNode->Enabled = false;
			}
		}
	}
	else
	{
		preview->bBegin->Enabled = false;
		preview->bEnd  ->Enabled = false;

		preview->bNodeBegin->Enabled = false;
		preview->bNodeEnd  ->Enabled = false;

		preview->bPrev->Enabled = false;
		preview->bNext->Enabled = false;

		preview->bPlay	  ->Enabled = false;
		preview->bPlayNode->Enabled = false;

	//	preview->bPause->Enabled = false;
		preview->bStop ->Enabled = false;
	}
}

////////////////////////////////

void FormTimeLine::UpdateFrame()
{
	if( !--mustSetFrame )
	{
		if( initAnim )
		{
			preview->CharacterReset();

			const Node &node = nodes[nodesHead];

			ani_Pause(true);

			preview->LockEvents(lockEvents = true);

		//	ani->Goto(node.name.str,0.0f);
				 Goto(node);

		/*	AGNA_SetCurrentFrame scf(0.0f);

			bool r = ani->GetNativeGraphInfo(scf);
			Assert(r)
		//	mustSetFrame = 2;*/
			SetFrame(0.0f);

			preview->LockEvents(lockEvents = false);

			initAnim = false;
		}
		else
		{
			Assert(0)

			/////////////////////////////////////////////

			float frame = animLocal ? 0.0f : cursorFrame;

		/*	AGNA_SetCurrentFrame scf(frame);

			bool r = ani->GetNativeGraphInfo(scf);
			Assert(r)*/
			SetFrame(frame);

			if( animNode || dragCursor )
			{
				UpdateEvents();

				if( moveLeft )
				{
					moveLeft = false;

					CheckEvents(nodes[nodes[cursorNode].next],-0.1f,0.9f);
				}
				
				CheckEvents(nodes[cursorNode],frame - 0.9f,frame);
			}

			if( lockEvents )
			{
				preview->LockEvents(lockEvents = false);
			}

			////////////////////////////////////////////
		}
	}
}

void FormTimeLine::UpdateOffset()
{
	Assert(dragNode || dragMeter)

	if( dragNode )
	{
		float off = offset*api->GetDeltaTime()*10.0f;

		if( off < 0.0f )
		{
			off = scr.scroll(-off);

			dragNode->t += off;

			if( dragNode->t < 0.0f )
				dragNode->t = 0.0f;

			UpdatePrevNext(dragNode->t,true);

			dragOffset += off;
		}
		else
		{
			off = scr.scroll(-off);

			dragNode->t += off;

			if( dragNode->t < 0.0f )
				dragNode->t = 0.0f;

			if( nodesTail >= 0 )
			{
				const Node &tail = nodes[nodesTail];

				if( dragNode->t > tail.t + tail.w )
					dragNode->t = tail.t + tail.w;
			}
			else
			{
				if( dragNode->t > 0.0f )
					dragNode->t = 0.0f;
			}

			UpdatePrevNext(dragNode->t,false);

			dragOffset += off;

			const Node &node = *dragNode;
			const Node &tail = nodesTail >= 0 ? nodes[nodesTail] : node;

			float r1 = tail.t + tail.width;
			float r2 = node.t + node.width;

			scr.setSize(coremax(r1,r2));
		}
	}
	else // dragMeter
	{
		float off = offset*api->GetDeltaTime()*50.0f;

		off = scr.scroll(off);

		dragOffset += off;
	}
}

void FormTimeLine::UpdatePlay()
{
	const Node &node = *animNode;

	int frames = node.frames;

	const Node *next = null; float blend = blendTime;

	if( node.next >= 0 )
	{
	/*	next = &nodes[node.next];

		if( next->t < node.t + node.w - node.frameLen )
		{
			frames -= int((node.t + node.w - next->t + node.frameLen)/node.frameLen);

			blend = blendTime;
		}*/
		next = &nodes[node.next]; float r = node.frameLen*0.5f;

		if( next->t < node.t + node.w - r )
		{
			frames -= int((node.t + node.w - next->t + r)/node.frameLen);

			blend = blendTime;
		}
	}

	const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

	animPrev = animTime;

	AGNA_GetCurrentFrame gcf;

	bool r = ani->GetNativeGraphInfo(gcf);
	Assert(r)

	animTime = node.frameLen*0.001f*gcf.currentFrame;

	if( animTime < animStep*(frames))
	{
	/*	if( mustSetFrame <= 0 )
		{
			AGNA_GetCurrentFrame gcf;

			bool r = ani->GetNativeGraphInfo(gcf);
			Assert(r)

			float frame = gcf.currentFrame;

			cursor = node.t + node.frameLen*frame;
		}*/
		float t = animTime;

		if( t < 0.0f )
			t = 0.0f;

		cursor = node.t + t*1000.0f;

	//	animPrev = animTime;

		if( initPlay )
		{
			initPlay = false;

			animPrev -= 0.001f;
		}

		////////////////////////////////

	//	animTime += api->GetDeltaTime();
	/*	AGNA_GetCurrentFrame gcf;

		bool r = ani->GetNativeGraphInfo(gcf);
		Assert(r)

		animTime = node.frameLen*0.001f*gcf.currentFrame;*/

		////////////////////////////////

		float prevFrame = animPrev*node.fps;
		float currFrame = animTime*node.fps;

		CheckEvents(node,prevFrame,currFrame);

		ScrollToCursor();
	}
	else
	{
		for( int i = 0 ; i < events ; i++ )
		{
			anime->EventSoundEndNode(node.index,i);
		}

		if( next && next->index >= 0 && next->valid && !animLocal && Goto(*next,blend))
		{
		//	ani->Goto( next->name.str,blend);
		//		 Goto(*next,blend);

			animTime -=  animStep*(frames);
			animPrev  = -0.001f;

			////////////////////////////////

		//	animTime += api->GetDeltaTime();

			////////////////////////////////

			animStep = 1.0f/next->fps;

			animNode = next;

			float prevFrame = animPrev*node.fps;
			float currFrame = animTime*node.fps;

			CheckEvents(*next,prevFrame,currFrame);

		/*	AGNA_SetCurrentFrame scf(prevFrame);

			bool r = ani->GetNativeGraphInfo(scf);
			Assert(r)*/

			float t = animTime;

			if( t < 0.0f )
				t = 0.0f;

			cursor = Lerp(cursor,next->t + t*1000.0f,0.5f);

		//	SetFocus(next,node.next,false);
		}
		else
		{
			preview->CharacterReset();

		/*	ani->Goto("idle",0.3f);

			animNode = null;

			animTime = 0.0f; animPrev = -0.001f;
			animStep = 0.0f;

			cursor = -1000.f;

			preview->bPlay->image = &options->imagePlay;*/

			animNode = null;

			if( cursorNode >= 0 )
			{
				const Node &node = nodes[cursorNode];

			//	SetFocus(&node,cursorNode,false);

				ani_Pause(true);

				preview->LockEvents(lockEvents = true);

			//	ani->Goto(node.name.str,0.0f);
					 Goto(node);

			/*	AGNA_SetCurrentFrame scf(cursorFrame);

				bool r = ani->GetNativeGraphInfo(scf);
				Assert(r)
			//	mustSetFrame = 2;*/
				SetFrame(cursorFrame);

				preview->LockEvents(lockEvents = false);

				cursor = cursorBeg;

				if( !animLocal )
				{
					ScrollToCursor();
				}
			}
			else
			{
			//	ani->Goto("idle",0.3f);
			//	preview->SetIdle();
				GotoIdle(0.3f);

				cursor = -1000.0f;
			}

			animLocal = false;
		}
	}

	UpdateEvents();
}
