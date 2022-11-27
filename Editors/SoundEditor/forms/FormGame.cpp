//============================================================================================
// FormGame
//============================================================================================

#include "FormGame.h"
#include "Lists\FormWaveSelector.h"
#include "Lists\FormWaveFiles.h"
#include "FormEditName.h"
#include "FormMessageBox.h"
#include "Combobox\FormCombobox.h"
#include "FormLabel.h"
#include "Sliders\NumberSlider.h"

#include "..\..\..\common_h\Mission.h"
#include "..\..\..\Common_h\InputSrvCmds.h"

#include "Lists\FormSoundWaves.h"
#include "Lists\FormGameList.h"

#include "..\..\..\Character\Character\Character.h"
#include "..\..\..\MissionEditor\MissionEditorExport.h"

#include "FormAttGraph.h"
#include "FormSndBaseParams.h"

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

FormGame::FormGame(GUIWindow *parent, GUIRectangle &rect) : GUIControl(parent),names(_FL_,32)
{
#ifndef NO_TOOLS
	options->sa->EditEnablePreview(false);
#endif
	options->soundService->ReleaseSoundBank(SoundService_GlobalSoundBank);
	options->soundService->LoadSoundBank(SoundService_GlobalSoundBank);

	string curSettingsPath = options->pathLocalSettings;
	curSettingsPath += "FormGame.ini";
	settingsIni = options->fileService->OpenEditableIniFile(curSettingsPath.c_str(), file_open_always, _FL_);


	SetClientRect(rect);
	SetDrawRect(rect);

	service = (IPreviewService *)api->GetService("PreviewService");
	Assert(service)

//	int cx = (rect.w*c_gamesizew)/100;
//	int cy = (rect.h*c_gamesizeh)/100;
	int cx = rect.w - 300;
//	int cy = cx*3/4;
	int cy = cx*8/11;

	mission = null;

	misIndex = -1;
	misOk = false;

	pause = true;
	pauseDown = false;

	pauseText.Init("Paused");

	errText.Init("Mission not found");

	GUIRectangle re,rl;

	re.x =  10 + cx;
	re.y =  25;
	re.w = 250;
	re.h =  20;

	rl.x = re.x;
	rl.y = re.y + re.h;
	rl.w = re.w;
	rl.h = 200;

	combo = NEW FormComboBox(this,re,rl,false);

	combo->SetTitle(null,"Select mission");
//	combo->Hint = "Current mission";

	combo->onChange.SetHandler(this,(CONTROL_EVENT)&FormGame::OnComboSelect);

	/////////////////////

	GUIRectangle r;

	r.x = re.x + re.w + 10;
	r.y = re.y;
	r.w = 20;
	r.h = 20;

	button = NEW FormButton(this,r);
	button->image = &options->imageCamera;
	button->imageW = 16;
	button->imageH = 16;
	button->Hint = "Switch camera - mission/free";

	button->onDown.SetHandler(this,(CONTROL_EVENT)&FormGame::OnSwitch);
	button->Enabled = false;

	camera = null;

	/////////////////////

	combo->Enabled = false;

	IIniFile *info = options->fileService->OpenIniFile(options->pathGameMissionsIni,_FL_);

	if( info )
	{
		array<string> list(_FL_);

		info->GetSections(list);

		bool err = false;

		for( int i = 0 ; i < list ; i++ )
		{
			const char *sect = list[i].c_str();

			const char *name = info->GetString(sect,"name",null);

			if( string::NotEmpty(name))
			{
				Info &item = names[names.Add()];

				item.name = name;

				const char *file = info->GetString(sect,"xmlz",null);

				item.file = string::NotEmpty(file) ? file : name;

				item.file.AddExtention(".xmlz");

				combo->AddItem(null,item.name);
			}
			else
			{
				if( !err )
					api->Trace("");

				api->Trace("FormGame: missions list: empty section [%s]",sect);

				err = true;
			}
		}

		if( err )
			api->Trace("");

		RELEASE(info)

	//	if( count > 0 )
		if( names )
		{			
			combo->Enabled = true;
			const char * selItemName = null;
			if(settingsIni)
			{			
				selItemName = settingsIni->GetString(null, "current_mission", null);
			}
			if(selItemName)
			{
				combo->SelectItemByText(selItemName);
			}else{
				combo->SelectItem(0);
			}

		//	Load();
		}
	}

	/////////////////////

	r.x =  cx + 1;
	r.y =  72;
	r.w = 300 - 1;
	r.h =  cy - 72 - c_bparams_h;

	list = NEW FormGameList(this,r);

	list->onChangeFocus.SetHandler(this,(CONTROL_EVENT)&FormGame::OnSoundSelect);


	/////////////////////

	r.x =  cx + 1;
	r.y =  cy - c_bparams_h;
	r.w = 300 - 1;
	r.h = 200;

	params = NEW FormSndBaseParams(this,r,true);

	/////////////////////

	r.x = 0;
	r.y = cy;
	r.w = rect.w/2;
	r.h = rect.h - r.y;

	waves = NEW FormSoundWaves(options->formOptions.formSounds.soundWaves,this,r,true);

	wavesRect = r;

	/////////////////////

	r.x = r.w;
	r.y = cy;
	r.w = rect.w - r.w;
	r.h = rect.h - r.y;

	graph = NEW FormAttGraph(this,r,true);

	/////////////////////

	list->InitList();
//	list->SetFocus((long)0);

	/////////////////////

}

FormGame::~FormGame()
{
	if(settingsIni)
	{
		settingsIni->Release();
		settingsIni = null;
	}

	if( mission && misOk && pause == false )
	{
		mission->Controls().ExecuteCommand(InputSrvLockMouse(false));

		GetMainControl()->Application->ShowCursor(true);

		SetCursorPos(cursor.x,cursor.y);
	}

	if( mission )
		mission->DeleteMission();

	mission = null;

	camera	= null;
	combo	= null;
	button	= null;
	list	= null;
	waves	= null;
	graph	= null;
	params	= null;
}

//Рисование
void FormGame::Draw()
{

	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);

	float dt = api->GetDeltaTime();

//	int cx = (r.w*c_gamesizew)/100;
//	int cy = (r.h*c_gamesizeh)/100;
	int cx = r.w - 300;
//	int cy = cx*3/4;
	int cy = cx*8/11;

	if( misOk == false )
	{
		int x = r.x + (cx - errText.w)/2;
		int y = r.y + (cy - errText.h)/2;

		options->render->Print((float)x,(float)y,options->colorTextLo,errText.str);
	}
	else
	{
		if( GetAsyncKeyState(VK_PAUSE) != 0 )
		{
			if( pauseDown == false )
			{
				if( pause )
				{
					GetCursorPos(&cursor);
					SetCursorPos(0,0);
				}

				mission->Controls().ExecuteCommand(InputSrvLockMouse(pause));
			//	mission->EditorSetSleep(pause);

				options->controls->EnableControlGroup("mission.FreeCamera",pause);

				pause = !pause;

				if( pause )
				{
					SetCursorPos(cursor.x,cursor.y);
				}else{
					options->kbFocus = this;
					options->dndControl = null;
					options->gui_manager->SetKeyboardFocus(this);
					options->gui_manager->SetFocus(this);
				}

				GetMainControl()->Application->ShowCursor(pause);

			//	waves->Visible = pause;
			}

			pauseDown = true;
		}
		else
		{
			pauseDown = false;
		}

		RENDERVIEWPORT vp2 = options->render->GetViewport();
		RENDERVIEWPORT vp1;

		vp1.X = r.x; vp1.Width  = cx;
		vp1.Y = r.y; vp1.Height = cy;

		vp1.MinZ = 0.1f;
		vp1.MaxZ = 1.0f;

		options->render->SetViewport(vp1);

		#ifndef NO_TOOLS

		mission->EditorDraw(pause ? 0.0f : dt);

		#endif

		options->render->SetViewport(vp2);

		if( pause )
		{
			const int w = 120;
			const int h =  40;

			int x = r.x + (cx - w)/2;
			int y = r.y + (cy - h)/2;

			GUIHelper::Draw2DRect(x,y,w,h,options->bkg);
			GUIHelper::DrawWireRect(x,y,w,h,options->black2Bkg[0]);

			x += (w - pauseText.w)/2;
			y += (h - pauseText.h)/2;

			options->render->Print((float)x,(float)y,options->colorTextLo,pauseText.str);
		}
	}

/*	if( waves->Visible )
	{
		GUIHelper::Draw2DRect(0,r.y + wavesRect.y,r.x + wavesRect.w,wavesRect.h,options->bkg);
	}*/

	GUIControl::Draw();

	GUIHelper::Draw2DLine(r.x + cx,r.y,r.x + cx,r.y + cy,options->black2Bkg[0]);
	GUIHelper::Draw2DLine(r.x,r.y + cy,r.x + r.w,r.y + cy,options->black2Bkg[0]);

	int x = r.x + 10 + cx;
	int y = r.y +  5;

//	options->render->Print((float)x,(float)y,options->colorTextLo,"Current mission: %s","[none]");
	options->render->Print((float)x,(float)y,options->colorTextLo,"Current mission:");

//	y += 46;

//	options->render->Print((float)x,(float)y,options->colorTextLo,"Sounds:");

	GUIHelper::DrawHorizLine(r.x + cx,r.w - cx,r.y + 72,options->black2Bkg[0]);

	GUIHelper::DrawVertLine(r.y + cy,r.h - cy,r.x + r.w/2,options->black2Bkg[0]);
}

void FormGame::Load()
{
	int index = combo->GetCurrentItemIndex();

	if( index != misIndex )
	{
		if( mission && misOk )
		{
			mission->DeleteMission();
			mission = null;
		}

		if( mission == null )
		{
			mission = (IMission *)api->CreateObject("Mission");
			Assert(mission)

			api->DelObjectExecutions(mission);
		}

		mission->Controls().ExecuteCommand(InputSrvLockMouse(false));

		ExtName * itemText = combo->GetCurrentItemText();
		if(settingsIni)
		{			
			settingsIni->SetString(null, "current_mission", itemText->str);
			settingsIni->Flush();
		}

		Info &info = names[index];

		const char *name = info.name.c_str();
		const char *file = info.file.c_str();

		if( string::NotEmpty(name) &&
			string::NotEmpty(file))
		{
			//// export mission ////

			bool done = false;
			bool skip = false;

			api->Trace("\n====================================================");
			api->Trace("Export mission: %s (%s)",name,file);
			api->Trace("====================================================\n");

			string misFile("resource\\missions\\");

			misFile += name;
			misFile += "\\";
			misFile += name;
			misFile += ".mis";

			bool misExist = options->fileService->IsExist(misFile);

			if( service->AddName(file,
					options->pathProjectXmlz,
					options->pathVersionFiles) ||	// если миссия еще не экспортилась
				misExist == false )					// или бинарный файл миссии удален
			{
				string xmlName = options->pathProjectXmlz + file/* + ".xmlz"*/;

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

			api->Trace("\n====================================================");
			api->Trace("Export mission: %s (%s): %s",name,file,done ? "done" : skip ? "skipped" : "failed");
			api->Trace("====================================================");

			////////////////////////
			float progressCount = 0.0f;
			misOk = mission->CreateMission(name, 100.0f, progressCount);
		}
		else
			misOk = false;



		options->controls->EnableControlGroup("mission.character");
		options->controls->EnableControlGroup("mission.Shooter");
		options->controls->EnableControlGroup("mission.FreeCamera",false);

		if( misOk )
		{
			options->soundService->SetDebugView(ISoundService::dv_all);

			MOSafePointer p;

			mission->FindObject(ConstString("Free camera"),p);

			camera = p.Ptr();
		}
		else
			camera = null;

		button->Enabled = misOk && camera != null;
		misIndex = index;
	}
}

void _cdecl FormGame::OnComboSelect(GUIControl *sender)
{
	Load();
}

void _cdecl FormGame::OnSoundSelect(GUIControl *sender)
{
	const UniqId & id = list->GetFocusElement();
	waves->SetNewSound(id);
	graph->SetCurrentSound(id);
	params->SetCurrentSound(id);
}

void _cdecl FormGame::OnSwitch(GUIControl *sender)
{
	camera->Activate(!camera->IsActive());
}
