#include "PreviewAnimation.h"

#include "..\FormButton.h"

#include "..\..\..\..\Character\Character\Character.h"
#include "..\..\..\..\common_h\Mission.h"
#include "..\..\..\..\Common_h\InputSrvCmds.h"

#include "..\..\..\..\MissionEditor\MissionEditorExport.h"

#include "..\..\..\..\Common_h\AnimationNativeAccess.h"





PreviewAnimation::Transform::Transform(IGMXScene * s, GMXHANDLE hdl)
{
	Assert(s);
	Assert(hdl.isValid());
	scene = s;
	handle = hdl;
	p = null;
	scene->AddRef();
}

PreviewAnimation::Transform::Transform(MissionObject * mo)
{
	scene = null;
	handle.reset();
	Assert(mo);
	p = mo;
}

PreviewAnimation::Transform::~Transform()
{
	if(scene)
	{
		scene->Release();
		scene = null;
	}
	handle.reset();
	p = null;
}


Matrix PreviewAnimation::Transform::GetWorldTransform()
{
	Matrix m;
	if(scene)
	{
		if(handle.isValid())
		{
			scene->GetNodeWorldTransform(handle, m);
		}else{
			m.SetIdentity();
		}				
	}else{
		if(p)
		{
			p->GetMatrix(m);
		}else{
			m.SetIdentity();
		}
	}			
	return m;
}

const char * PreviewAnimation::Transform::GetName()
{
	if(scene)
	{
		return scene->GetNodeName(handle);
	}
	return "[root]";
}


class IPreviewService : public Service
{
public:

	virtual void SetCharacter(Character *p) = 0;

	virtual IAnimation *SetAnimation(const char *name) = 0;

	virtual IGMXScene *GetModel() = 0;

	virtual void Begin() = 0;

	virtual void Rotate(float ay) = 0;

	virtual void Reset() = 0;

	virtual void LockEvents(bool lock) = 0;

	virtual bool AddName(const char *xmlName, const char *xmlPath, const char *verPath) = 0;

};

struct ICharacterTable : public MissionObject
{
	struct Item
	{
		ConstString name;	// имя миссионного объекта
		bool		play;	// синхронно играть граф
	};

	virtual const array<Item> &GetTable() = 0;
	virtual const array<Item> &GetLocators() = 0;

	virtual float GetBlend() = 0;

	virtual bool GetRot() = 0;
};

PreviewAnimation::PreviewAnimation(GUIControl *parent, const GUIRectangle &rect) : GUIControl(parent),
	locators   (_FL_),
	locatorHash(_FL_),
	info	   (_FL_)
{
	SetClientRect(rect);
	SetDrawRect(rect);

	mission = null;

	misOk = false;
	aniOk = false;

	missionName = "[empty]";

	ani = null;

	misErr.Init(  "Mission not found.");
	aniErr.Init("Character not found.");

	initService = false;

	const int bw = 30;
	const int bh = 30;

	const int bi = 3;

	const int cn = 9;

	bar.r.w = 7 + (bw + bi)*cn + 7 - bi + 1 + (bw + bi) + 7 - bi + 1;
	bar.r.h = bh + 6;

	bar.r.x = 0;
	bar.r.y = rect.h - 3 - bh;

	const int image_w = 30;
	const int image_h = 30;

	GUIRectangle r(bar.r.x + 7,bar.r.y + 2,bw,bh);

	bBegin = NEW FormButton(this,r); r.x += bw + bi;
	bBegin->image = &options->imageMoveToStart;
	bBegin->imageW = image_w;
	bBegin->imageH = image_h;
	bBegin->Hint = options->GetString(SndOptions::s_hint_ani_tm_start);

	bNodeBegin = NEW FormButton(this,r); r.x += bw + bi;
	bNodeBegin->image = &options->imageMoveToStartNode;
	bNodeBegin->imageW = image_w;
	bNodeBegin->imageH = image_h;
	bNodeBegin->Hint = options->GetString(SndOptions::s_hint_ani_tm_start_node);

	bPrev = NEW FormButton(this,r); r.x += bw + bi;
	bPrev->image = &options->imageMoveToPrev;
	bPrev->imageW = image_w;
	bPrev->imageH = image_h;
	bPrev->Hint = options->GetString(SndOptions::s_hint_ani_tm_prev);

/*	bPause = NEW FormButton(this,r); r.x += bw + bi;
	bPause->image = &options->imagePause;*/

//	r.y -= 33;

	bPlay = NEW FormButton(this,r);
	bPlay->image = &options->imagePlay;
	bPlay->imageW = image_w;
	bPlay->imageH = image_h;
	bPlay->Hint = options->GetString(SndOptions::s_hint_ani_tm_play);

//	r.y += 33;
	r.x += bw + bi;

	bPlayNode = NEW FormButton(this,r); r.x += bw + bi;
	bPlayNode->image = &options->imagePlayNode;
	bPlayNode->imageW = image_w;
	bPlayNode->imageH = image_h;
	bPlayNode->Hint = options->GetString(SndOptions::s_hint_ani_tm_play_node);

	bStop = NEW FormButton(this,r); r.x += bw + bi;
	bStop->image = &options->imageStop;
	bStop->imageW = image_w;
	bStop->imageH = image_h;
	bStop->Hint = options->GetString(SndOptions::s_hint_ani_tm_stop);

	bNext = NEW FormButton(this,r); r.x += bw + bi;
	bNext->image = &options->imageMoveToNext;
	bNext->imageW = image_w;
	bNext->imageH = image_h;
	bNext->Hint = options->GetString(SndOptions::s_hint_ani_tm_next);

	bNodeEnd = NEW FormButton(this,r); r.x += bw + bi;
	bNodeEnd->image = &options->imageMoveToEndNode;
	bNodeEnd->imageW = image_w;
	bNodeEnd->imageH = image_h;
	bNodeEnd->Hint = options->GetString(SndOptions::s_hint_ani_tm_end_node);

	bEnd = NEW FormButton(this,r);  r.x += bw + bi;
	bEnd->image = &options->imageMoveToEnd;
	bEnd->imageW = image_w;
	bEnd->imageH = image_h;
	bEnd->Hint = options->GetString(SndOptions::s_hint_ani_tm_end);

	r.x += 7 - bi + 1;

	bSound = NEW FormButton(this,r);
	bSound->image = &options->imageDelete;
	bSound->imageW = 24;
	bSound->imageH = 24;
	bSound->Hint = "Don't draw sound info";

	bSound->onDown.SetHandler(this,(CONTROL_EVENT)&PreviewAnimation::Sound);

	locator = -1;

	leftDown = false;

	m_time = 0.0f;

	pause = false;

	service = (IPreviewService *)api->GetService("PreviewService");
	Assert(service)

	blend = 0.2f;

	deltaTime = 0.0f;

	drawSound = true;
	modelRot  = true;
}

PreviewAnimation::~PreviewAnimation()
{
//	api->DelObjectExecutionFunc(&viewer,(ObjectExecution)&Viewer::Beg);
//	api->DelObjectExecutionFunc(&viewer,(ObjectExecution)&Viewer::End);

	if( mission )
		mission->DeleteMission();

	mission = null;

	locators.DelAllWithPointers();
}

IAnimation *PreviewAnimation::Init(const char *misName, const char *aniName)
{
	if( missionName == misName )
	{
		if( aniOk ) 
		{
			Assert(ani)

			return ani;
		}
		else
			return null;
	}

	locators.DelAllWithPointers();

	missionName = misName;

	if(mission)
	{
		mission->DeleteMission();
		mission = null;
	}

	initService = false;

	locator = -1;

//	api->DelObjectExecution(&viewer,&Viewer::Beg);
//	api->DelObjectExecution(&viewer,&Viewer::End);

	viewer.Reset();

	misOk = false;
	aniOk = false;

	deltaTime = 0.0f;



	locatorHash.DelAll();
	info.DelAll();

	GetLocatorName(0,true);

	if( string::NotEmpty(misName))
	{
		mission = (IMission *)api->CreateObject("Mission");
		Assert(mission)

		mission->Controls().ExecuteCommand(InputSrvLockMouse(false));

		//// export mission ////

		bool done = false;
		bool skip = false;

		api->Trace("\n==========================================");
		api->Trace("Export mission: %s",misName);
		api->Trace("==========================================\n");

		string misFile("resource\\missions\\");

		misFile += misName;
		misFile += "\\data\\";
		misFile += misName;
		misFile += ".mis";

		bool misExist = options->fileService->IsExist(misFile);

		if( service->AddName(misName,
				options->pathProjectXmlz,
				options->pathVersionFiles) ||	// если миссия еще не экспортилась
			misExist == false )					// или бинарный файл миссии удален
		{
			string xmlName = options->pathProjectXmlz + misName + ".xmlz";

			if( misExist )
			{
				bool ok = options->fileService->Delete(misFile);

				api->Trace("Delete file %s: %s\n",misFile.c_str(),ok ? "done" : "failed");
			}

			if( options->fileService->IsExist(xmlName))
			{
				MissionEditorExport *editor = (MissionEditorExport *)api->CreateObject("MissionEditor");				

				if( editor )
				{
					editor->ConvertXML2MIS(xmlName);

					delete editor;

					done = true;
				}
				else
				{
				//	api->Trace("Sound editor error: mission %s can't be export, object MissionEditor not found",misName,xmlName.c_str());
					api->Trace("Mission can't be export, object MissionEditor not found");
				}

				options->SetInputOptions();
			}
			else
			{
			//	api->Trace("Sound editor error: mission %s can't be export, file %s not found",misName,xmlName.c_str());
				api->Trace("Mission can't be export, file %s not found",xmlName.c_str());
			}
		}
		else
		{
		//	api->Trace("Sound editor message: mission %s already exported, skip export action",misName);
			api->Trace("Mission already exported, skip export");

			skip = true;
		}

		api->Trace("\n==========================================");
		api->Trace("Export mission: %s: %s",misName,done ? "done" : skip ? "skipped" : "failed");
		api->Trace("==========================================");

		////////////////////////
		float progressCount = 0.0f;
		misOk = mission->CreateMission(misName, 100.0f, progressCount);
	}

	if( misOk )
	{
		ani = service->SetAnimation(aniName);

		aniOk = (ani != null);

		if( aniOk )
		{
			////

			IGMXScene * model = service->GetModel();
			ReadTable(model);

			GUIRectangle r = GetDrawRect();
			ClientToScreen(r);

			RENDERVIEWPORT vp;

			vp.X = r.x; vp.Width  = r.w;
			vp.Y = r.y; vp.Height = r.h;

			vp.MinZ = 0.1f;
			vp.MaxZ = 1.0f;

			viewer.Init(&mission->Render(),vp,this);

		//	api->SetObjectExecution(&viewer,"gui",0x100 + 1,&Viewer::Beg);
		//	api->SetObjectExecution(&viewer,null,0,&Viewer::End);

			initService = true;

			api->DelObjectExecutions(mission);
		}
		else
		{
			mission->DeleteMission();

			mission = null;
		}
	}
	else
	{
		ani = null;
	}

	bool en = ani != null;

	bBegin->Visible = en;
	bEnd  ->Visible = en;

	bNodeBegin->Visible = en;
	bNodeEnd  ->Visible = en;

	bPrev->Visible = en;
	bNext->Visible = en;

	bPlayNode->Visible = en;
	bPlay	 ->Visible = en;

//	bPause->Visible = en;
	bStop ->Visible = en;

	for( dword i = 0 ; i < locators.Size() ; i++ )
	{
		const char * locatorName = GetLocatorName(i);
		if(string::NotEmpty(locatorName))
		{
			bool isOk = locatorHash.AddObj(locatorName,i);
			Assert(isOk);
		}else{
//			Assert(false);
			GetLocatorName(i);
		}
	}	

	return ani;
}

void PreviewAnimation::ReadTable(IGMXScene * globalModel)
{
	blend	 = 0.2f;
	modelRot = true;

	array<GMXHANDLE> locs(_FL_, 256);

	if(globalModel)
	{
		locs.DelAll();
		globalModel->GetAllLocators(locs);
		for(dword j = 0 ; j < locs.Size(); j++ )
		{
			locators.Add(NEW Transform(globalModel, locs[j]));
		}
	}

	MGIterator &it = mission->GroupIterator(GroupId('C','r','T','b'),_FL_);

	for(ICharacterTable *p ; p = (ICharacterTable *)it.Get(); it.Next())
	{
		if( p->IsActive())
		{
			blend	 = p->GetBlend();	// сейчас только одна таблица, просто
			modelRot = p->GetRot();		// перебиваем значения

			MOSafePointer ptr;

			const array<ICharacterTable::Item> &table = p->GetTable();

			for( int i = 0 ; i < table ; i++ )
			{
				if( mission->FindObject(table[i].name,ptr))
				{
					MissionObject *obj = ptr.Ptr();

					MO_IS_IF(id_Character, "Character", obj)
					{
						Character *chr = (Character *)obj;

						locs.DelAll();
						IGMXScene * scene = chr->GetModel();
						scene->GetAllLocators(locs);

						LocInfo &item = info[info.Add()];

						item.tag = table[i].name.c_str();

						item.beg = locators;
						item.end = locators + locs;

						item.ani = chr->GetAnimation();

						item.run = table[i].play;

						for(dword j = 0 ; j < locs.Size(); j++ )
						{
							locators.Add(NEW Transform(scene, locs[j]));
						}
					}
					else
					{
						obj->LogicDebugError("Object must be Character.");
					}
				}
			}

			const array<ICharacterTable::Item> &locas = p->GetLocators();

			for( int i = 0 ; i < locas ; i++ )
			{
				if( locas[i].play ) // использовать локатор
				{
					if( mission->FindObject(locas[i].name,ptr))
					{
						MissionObject *obj = ptr.Ptr();

						MO_IS_IF(id_MissionLocator, "MissionLocator", obj)
						{
							LocInfo &item = info[info.Add()];

							item.tag = locas[i].name.c_str();

							item.beg = locators;
							item.end = locators + 1;

							item.ani = null;

							item.run = false;

							locators.Add(NEW Transform(obj));
						}
						else
						{
							obj->LogicDebugError("Object must be MissionLocator.");
						}
					}
				}
			}
		}
	}

	it.Release();

}

const char *PreviewAnimation::GetLocatorName(dword index, bool reset)
{
	static int lastIndex = -1;
	static int lastGroup = -1;

	static char buf[128]; static const char *name = null;

	if( reset )
	{
		lastIndex = -1;
		lastGroup = -1; name = null; return null;
	}

	if( !info || index < info[0].beg )
	{
		return locators[index]->GetName();
	}

	if( index == lastIndex )
	{
		return name;
	}

	if( lastGroup >= 0 )
	{
		const LocInfo &item = info[lastGroup];

		if( index >= item.beg &&
			index <  item.end )
		{
			lastIndex = index;

			sprintf_s(buf,sizeof(buf),"#%s:%s",item.tag,locators[index]->GetName());

			return name = buf;
		}
	}

	for( int i = 0 ; i < info ; i++ )
	{
		const LocInfo &item = info[i];

		if( index < item.end )
		{
			lastIndex = index; lastGroup = i;

			sprintf_s(buf,sizeof(buf),"#%s:%s",item.tag,locators[index]->GetName());

			return name = buf;
		}
	}

	Assert(0)

	return null;
}

////////////////////////////////////////////////////////////////

void PreviewAnimation::SetCharactersGraphInfo(IAnimation::GraphNativeAccessor &gna)
{
	for( int i = 0 ; i < info ; i++ )
	{
		const LocInfo &item = info[i];

		if( item.ani && item.run )
		{
			bool r = item.ani->GetNativeGraphInfo(gna);
		//	Assert(r)
			if( r == false )
			{
				if( gna.GetType() == agna_goto_nodeclip )
				{
				//	AGNA_GotoNodeClip &gnc = (AGNA_GotoNodeClip &)gna;

				//	api->Trace("PreviewAnimation: character %s: clip [%s:%d] not found.",item.tag,gnc.node,gnc.clipIndex);

					item.ani->Start();
				}
			}
		}
	}
}
/*
void PreviewAnimation::SetIdle()
{
	for( int i = 0 ; i < info ; i++ )
	{
		const LocInfo &item = info[i];

		if( item.ani && item.run )
		{
			item.ani->Goto("idle",0.3f);
		}
	}
}
*/
////////////////////////////////////////////////////////////////

void PreviewAnimation::MouseDown(int button, const GUIPoint &pt)
{
	if( modelRot )
	{
		if( button == GUIMSG_LMB_DOWN )
		{
			GUIRectangle r = GetClientRect();
			ClientToScreen(r);

			if( r.Inside(pt))
			{
				if( pt.y < bar.r.y )
				{
					leftDown = true;

					point = pt;
				}
			}
		}
	}

	GUIControl::MouseDown(button,pt);
}

void PreviewAnimation::MouseUp(int button, const GUIPoint &pt)
{
	if( button == GUIMSG_LMB_UP )
	{
		if( leftDown )
			leftDown = false;
	}

	GUIControl::MouseUp(button,pt);
}

void PreviewAnimation::MouseMove(int button, const GUIPoint &pt)
{
	if( leftDown )
	{
		if( pt.x != point.x )
		{
			float ay = 0.02f*(pt.x - point.x);

			CharacterRotate(ay);
		}

		point = pt;
	}

	GUIControl::MouseMove(button,pt);
}

//Рисование
void PreviewAnimation::OnDraw(const GUIRectangle & screenRect)
{
	if( !Visible )
		return;

	GUIRectangle r = screenRect;

	if( misOk == false )
	{
		int x = r.x + (r.w - misErr.w)/2;
		int y = r.y + (r.h - misErr.h)/2;

		options->render->Print((float)x,(float)y,options->colorTextLo,misErr.str);
	}
	else
	if( aniOk == false )
	{
		int x = r.x + (r.w - aniErr.w)/2;
		int y = r.y + (r.h - aniErr.h)/2;

		options->render->Print((float)x,(float)y,options->colorTextLo,aniErr.str);
	}
	else
	{
		float dt = api->GetDeltaTime();

		viewer.Beg(0.0f);

		#ifndef NO_TOOLS

		if(mission)
		{
			mission->EditorDraw(pause ? deltaTime : dt); 
			deltaTime = 0.0f;
		}

		if( drawSound )
			onUpdateSounds.Execute(this);

		#endif

		m_time += dt*5.0f;

		if( m_time > 2.0f*PI )
			m_time = fmodf(m_time,2.0f*PI);

		viewer.End(0.0f);

	/*	//// ToolBar

		int bx = r.x + bar.r.x;
		int by = r.y + bar.r.y;

		GUIHelper::Draw2DRect(bx,by,bar.r.w,bar.r.h,options->bkg2White[1]);

		GUIHelper::DrawHorizLine(
			bx,bar.r.w + 1,
			by - 1,
			options->black2Bkg[0]);

		GUIHelper::DrawVertLine(
			by,bar.r.h,
			bx + bar.r.w,
			options->black2Bkg[0]);*/
	}

//	GUIControl::Draw();
}

void _cdecl PreviewAnimation::Sound(GUIControl *sender)
{
	drawSound = !drawSound;

	if( drawSound )
	{
		bSound->image = &options->imageDelete;
		bSound->Hint = "Don't draw sound info";
	}
	else
	{
		bSound->image = &options->imageAddSound;
		bSound->Hint = "Draw sound info";
	}
}

void PreviewAnimation::GetListenerParameters(Vector & pos, Vector & dir)
{
	Matrix mtx;
	if(mission)
	{		
		mission->Sound().GetListenerMatrix(mtx);
	}
	pos = mtx.pos;
	dir = mtx.vz;
}

void PreviewAnimation::DrawBar()
{
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);

	int bx = r.x + bar.r.x;
	int by = r.y + bar.r.y;

	GUIHelper::Draw2DRect(bx,by,bar.r.w,bar.r.h,options->bkg2White[1]);

/*	GUIHelper::DrawHorizLine(
		bx,bar.r.w + 1,
		by - 1,
		options->black2Bkg[0]);

	GUIHelper::DrawVertLine(
		by,bar.r.h,
		bx + bar.r.w,
		options->black2Bkg[0]);*/

	GUIHelper::DrawWireRect(
		bx - 1,
		by - 1,bar.r.w + 1,bar.r.h,options->black2Bkg[0]);

/*	GUIHelper::DrawWireRect(
		bx + 71 - 1 + 33,
		by - 33 - 1,36,33,options->black2Bkg[0]);

	GUIHelper::Draw2DRect(bx + 71 + 33,by - 33,35,35,options->bkg2White[1]);*/

	GUIControl::Draw();

/*	GUIHelper::Draw2DRect(bx +  41,by + 3,29,29,0x30008000);
	GUIHelper::Draw2DRect(bx + 140,by + 3,29,29,0x30008000);
	GUIHelper::Draw2DRect(bx + 239,by + 3,29,29,0x30008000);*/

/*	GUIHelper::DrawWireRect(bx +  40,by + 2,30,30,0xff008000);
	GUIHelper::DrawWireRect(bx + 139,by + 2,30,30,0xff008000);
	GUIHelper::DrawWireRect(bx + 238,by + 2,30,30,0xff008000);*/

	if( initService )
	{
		service->Begin(); ActivateTriggers();

		initService = false;
	}
}

void PreviewAnimation::ActivateTriggers()
{
	if( mission )
	{
		MGIterator &it = mission->GroupIterator(GroupId('S','d','T','r'),_FL_);

		it.ExecuteEvent();
		it.Release();
	}
}

void PreviewAnimation::CharacterReset(bool isActivateTrigger)
{
	if(isActivateTrigger)
	{
		ActivateTriggers();
	}

	if( mission )
	{
		service->Reset();
	}
}

void PreviewAnimation::CharacterRotate(float ay)
{
	if( mission )
	{
		service->Rotate(ay);
	}
}

void PreviewAnimation::LockEvents(bool lock)
{
	if( mission )
	{
		service->LockEvents(lock);
	}
}

//////////////////////////////////////////

void PreviewAnimation::Viewer::Beg(float)
{
	if( render )
	{
		vp2 = render->GetViewport();

		render->SetViewport(vp1);
	}
}

void PreviewAnimation::Viewer::End(float)
{
	if( render )
	{
		int i = preview->GetLocator();
		if( i >= 0 )
		{
			const array<Transform *> &locators = preview->GetLocators();
			float dr = sinf(preview->m_time*4.0f)*0.005f;
			Vector r(0.08f + dr);
			Transform * p = locators[i];
			Matrix m = p->GetWorldTransform();
			render->DrawMatrix(m,0.3f + dr*2.0f,"LocatorLineNoZ");
			render->DrawSphereGizmo(m.pos,0.08f + dr,-1,-1,"LocatorLineNoZ");
			render->DrawMatrix(m,0.3f + dr*2.0f,"LocatorLine");
			render->DrawSphereGizmo(m.pos,0.08f + dr,-1,-1,"LocatorLine");
			Matrix mm = p->scene->GetTransform();
//			render->Print(50.0f, 50.0f, 0xffffffff, "Index: %i, Scene: %.8x, handle = %i, mobj: %.8x, name = %s, pos = (%f, %f, %f)", i, p->scene, p->handle.index, p->p, p->GetName(), m.pos.x, m.pos.y, m.pos.z);
//			render->Print(50.0f, 70.0f, 0xffffffff, "wp = (%f, %f, %f)", mm.pos.x, mm.pos.y, mm.pos.z);
		}
		render->SetViewport(vp2);
		if( preview )
		{
			preview->DrawBar();
		}
		options->gui_manager->GetCursor()->Draw();
	}
}
