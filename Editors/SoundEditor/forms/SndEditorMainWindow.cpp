//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditorMainWindow
//============================================================================================

#include "SndEditorMainWindow.h"
#include "FormButton.h"

//#define ENABLE_REVER_DEVELOPE

//============================================================================================

SndEditorMainWindow::SndEditorMainWindow() : GUIWindow(null, 0, 0, 100, 100)
{
	setCloseState = false;
	//Настраиваем окно
	GetRender()->SetBackgroundColor(options->bkg);
	bPopupStyle = true;
	//controlState = false;
	GUIRectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = GetRender()->GetScreenInfo3D().dwWidth;
	rect.h = GetRender()->GetScreenInfo3D().dwHeight;
	SetClientRect(rect);
	SetDrawRect(rect);
	tabs[0].name = options->GetString(SndOptions::s_form_sounds);
	tabs[0].createTab = &SndEditorMainWindow::CreateTabSounds;
	tabs[1].name = options->GetString(SndOptions::s_form_waves);
	tabs[1].createTab = &SndEditorMainWindow::CreateTabWaves;
	tabs[2].name = options->GetString(SndOptions::s_form_animation);
	tabs[2].createTab = &SndEditorMainWindow::CreateTabAnimation;
	tabs[3].name = options->GetString(SndOptions::s_form_polygon);
	tabs[3].createTab = &SndEditorMainWindow::CreateTabPolygon;
	tabs[4].name = options->GetString(SndOptions::s_form_game);
	tabs[4].createTab = &SndEditorMainWindow::CreateTabGame;
	//Считаем геометрические параметры закладок
	float y = (TabHeight - options->uiFont->GetHeight())*0.5f + 0.49f;
	float x = TabSpaceStart*0.7f;
	infoStartX = 0;
	for(long i = 0; i < ARRSIZE(tabs); i++)
	{
		Tab & tab = tabs[i];
		tab.tab = null;
		float w = options->uiFont->GetLength(tab.name) + 0.49f;
		tab.textX = x + TabSpaceStart;
		tab.textY = y;
		tab.xs = long(x);
		float tabW = TabSpaceStart + TabSpaceEnd + w;
		tab.width = (long)tabW;
		x += tab.width;
		tab.xe = long(x);
		tab.points[0].x = tab.xs - TabSpaceStart/2;
		tab.points[0].y = TabHeight;
		tab.points[1].x = tab.xs + TabSpaceStart/2 - 5;
		tab.points[1].y = 5;
		tab.points[2].x = tab.xs + TabSpaceStart/2 + 3;
		tab.points[2].y = 0;
		tab.points[3].x = tab.xe - 3;
		tab.points[3].y = 0;
		tab.points[4].x = tab.xe;
		tab.points[4].y = 3;
		tab.points[5].x = tab.xe;
		tab.points[5].y = TabHeight;
		infoStartX += tab.width;
	}
	infoStartX += 10 + TabSpaceStart + TabSpaceEnd;
	selTab = 0;
	GUIRectangle r;
	r.x = rect.x + rect.w - TabHeight;
	r.y = (TabHeight - CloseButtonSize)/2;
	r.w = CloseButtonSize;
	r.h = CloseButtonSize;
	FormButton * close = NEW FormButton(this, r);
	close->image = &options->imageCloseEditor;
	close->Hint = options->GetString(SndOptions::s_hint_exit_from_editor);
	close->onUp.SetHandler(this, (CONTROL_EVENT)&SndEditorMainWindow::OnCloseApp);


#if defined(ENABLE_REVER_DEVELOPE) && !defined(NO_TOOLS)
	//Тестовая часть кода
	FxScene fx;
	prevId.Build();
	Assert(sizeof(src) == sizeof(prc));
	//Частотная характеристика фильтра	
	for(dword i = 0; i < t_spectSize; i++)
	{
		//
		memset(src, 0, sizeof(src));
		fx.Restart();
		//
		float freq = i*(1.0f/(t_spectSize - 1.0f))*(t_spectMax - t_spectMin) + t_spectMin;
		float kw = freq*2.0f*PI/44100.0f;
		float energyIn = 0.0f;
		for(dword j = 0; j < t_spectLen; j++)
		{
			float v = sinf(kw*j);
			src[j*2] = v;
			energyIn += fabs(v);
		}
		fx.WaveProcess(src, t_spectLen);
		float energyOut = 0.0f;
		for(dword j = 0; j < t_spectLen; j++)
		{
			energyOut += fabs(src[j*2]);
		}
		spect[i] = 0.1f*energyOut/energyIn;
	}
	//Строим импульстую характеристику для единичного импульса
	memset(src, 0, sizeof(src));
	memset(prc, 0, sizeof(prc));
	fx.Restart();
	src[5*2] = 1.0f;
	memcpy(prc, src, sizeof(prc));
	fx.WaveProcess(prc, ARRSIZE(prc)/2);
	memset(prev, 0, sizeof(prev));
	for(dword i = 0; i < ARRSIZE(prc)/2; i++)
	{
		prev[i] = (short)long(32767.0f*prc[i*2]);
	}
#endif
}

SndEditorMainWindow::~SndEditorMainWindow()
{	
}

void SndEditorMainWindow::OnCreate()
{
	options->gui_manager->SetKeyboardFocus(this);
	//Application->Show(tools = NEW AnxEditorToolBar(options));
	SetFocus();
	long needTab = 0;
	if(options->policy == policy_designer)
	{
		needTab = 2;
	}
	
	if(options->isDeveloperMode)
	{
		needTab = 1;
	}
	
	SelectTab(needTab);
}


//============================================================================================



void SndEditorMainWindow::Draw()
{
	//В режиме девелопера меняем режимы редактора
	if(options->isEnableSwitchPolicy)
	{
		dword policy = options->policy;
		if(GetAsyncKeyState(VK_F9) < 0 && policy != policy_sounder)
		{
			options->policy = policy_sounder;
			SelectTab(selTab);
		}else
		if(GetAsyncKeyState(VK_F10) < 0 && policy != policy_designer)
		{
			options->policy = policy_designer;
			SelectTab(selTab);
		}
	}
	project->Update(api->GetNoScaleDeltaTime());
	dword bkgColor = options->bkg;
	GetRender()->Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, bkgColor, 1.0f, 0);
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	GUIHelper::Draw2DRect(0, 0, rect.w, TabHeight, options->black2Bkg[5]);
	for(long i = ARRSIZE(tabs) - 1; i >= 0; i--)
	{
		if(i != selTab)
		{
			DrawTab(tabs[i], false);
		}
	}
	DrawTab(tabs[selTab], true);
	GUIHelper::Draw2DLine(rect.x, TabHeight, tabs[selTab].xs - TabSpaceStart/2, TabHeight, options->black2Bkg[0]);
	GUIHelper::Draw2DLine(tabs[selTab].xe, TabHeight, rect.x + rect.w, TabHeight, options->black2Bkg[0]);
	if(options->isEnableSwitchPolicy)
	{
		switch(options->policy)
		{
		case policy_sounder:
			options->render->Print(float(rect.x + infoStartX), rect.y + 2.0f, options->colorTextHi, "Modes: F9 - >sounder<, F10 - designer");
			break;
		case policy_designer:
			options->render->Print(float(rect.x + infoStartX), rect.y + 2.0f, options->colorTextHi, "Modes: F9 - sounder, F10 - >designer<");
			break;
		}
	}else{
		if(options->policy == policy_designer)
		{
			options->render->Print(float(rect.x + infoStartX), rect.y + 2.0f, options->colorTextHi, "Designer mode");
		}
	}
	if(project->IsWaitToSave())
	{
		GUIHelper::DrawSprite(rect.x + rect.w - SaveImageRightPos, (TabHeight - SaveImageSize)/2, SaveImageSize, SaveImageSize, &options->imageWaitToSave);
		if(setCloseState)
		{
			project->FlushData();
		}		
	}else{
		if(setCloseState)
		{
			api->Exit();
		}
	}
	GUIControl::Draw();


#if defined(ENABLE_REVER_DEVELOPE) && !defined(NO_TOOLS)
	if(GetAsyncKeyState('1') >= 0)
	{	
		static long timescale = 1;
		static long timepos = 0;
		if(GetAsyncKeyState(VK_UP) < 0)
		{
			timescale--;
			if(timescale < 1) timescale = 1;
		}
		if(GetAsyncKeyState(VK_DOWN) < 0)
		{
			timescale++;
			if(timescale > 50) timescale = 50;
		}
		if(GetAsyncKeyState(VK_LEFT) < 0)
		{
			if(GetAsyncKeyState(VK_CONTROL) < 0)
			{
				timepos--;
			}else{
				timepos -= 100;
			}
			if(timepos < 0) timepos = 0;
		}
		if(GetAsyncKeyState(VK_RIGHT) < 0)
		{
			if(GetAsyncKeyState(VK_CONTROL) < 0)
			{
				timepos++;
			}else{
				timepos += 100;
			}
			if(timepos > ARRSIZE(src)) timepos = ARRSIZE(src);
		}
		if(GetAsyncKeyState('P') < 0)
		{			
			SoundsEngine::EditPrewiewWaveParams prevParams;
			prevParams.data = (byte *)prev;
			prevParams.dataSize = (ARRSIZE(prc)/2)*sizeof(short);
			prevParams.isStereo = false;
			prevParams.sampleRate = 44100;
			options->soundService->Engine().EditPrewiewPlay(prevId.data, &prevParams, null, false);
			Sleep(300);
		}
		GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, 0xff000000);
		float yS = src[0];
		float yP = prc[0];
		float y = rect.y + rect.h*0.5f;
		float s = -200.0f;		
		options->render->Print(10.0f, 10.0f, 0xffffffff, "Scale: 1:%i", timescale);
		for(long i = 0, pos = timepos; i < rect.w && pos < ARRSIZE(src); i++)
		{
			//Временные графики
			float minuss = 0.0f;
			float pluss = 0.0f;
			float minusp = 0.0f;
			float plusp = 0.0f;
			for(long j = 0; j < timescale; j++)
			{
				if(pos >= ARRSIZE(src)) break;
				if(src[pos] < 0.0f) minuss = coremin(minuss, src[pos]);
				if(src[pos] > 0.0f) pluss = coremax(pluss, src[pos]);
				if(prc[pos] < 0.0f) minusp = coremin(minusp, prc[pos]);
				if(prc[pos] > 0.0f) plusp = coremax(plusp, prc[pos]);				
				pos += 2;
			}
			float ys = pluss > -minuss ? pluss : minuss;
			float yp = plusp > -minusp ? plusp : minusp;
			if(i > 0)
			{
				GUIHelper::Draw2DLine(rect.x + i - 1.0f, yP*s + y + 200, rect.x + (float)i, yp*s + y + 200, 0xff0000ff);
				GUIHelper::Draw2DLine(rect.x + i - 1.0f, yS*s + y - 200, rect.x + (float)i, ys*s + y - 200, 0xffff0000);
			}
			yP = yp;
			yS = ys;
			//АЧХ
			long spos = (i + timepos)/timescale;
			if(spos < t_spectSize)
			{
				float sp = spect[spos]*100.0f;
				GUIHelper::Draw2DLine(rect.x + (float)i, (float)rect.y + rect.h, rect.x + (float)i, rect.y + rect.h - sp, 0xff00ff00);
			}
		}
	}
#endif

}

//Нарисовать закладку
inline void SndEditorMainWindow::DrawTab(Tab & tab, bool isSelect)
{
	if(!isSelect)
	{		
		GUIHelper::DrawPolygon(tab.points, ARRSIZE(tab.points), options->black2Bkg[7]);
	}else{
		GUIHelper::DrawPolygon(tab.points, ARRSIZE(tab.points), options->bkg);
	}
	options->render->Print(tab.textX, tab.textY, options->colorTextHi, tab.name);
	tab.points[0].y--;
	GUIHelper::DrawLines(tab.points, ARRSIZE(tab.points), options->black2Bkg[0]);
	tab.points[0].y++;
}

//Сообщения
bool SndEditorMainWindow::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	cursor_pos -= rect.pos;
	if(message == GUIMSG_LMB_DOWN)
	{
		if(cursor_pos.y < TabHeight && cursor_pos.x < tabs[ARRSIZE(tabs) - 1].xe)
		{
			if(!IsInsideTab(tabs[selTab], cursor_pos))
			{
				for(long i = 0; i < ARRSIZE(tabs); i++)
				{
					if(i != selTab)
					{
						if(IsInsideTab(tabs[i], cursor_pos))
						{							
							SelectTab(i);
							break;
						}
					}
				}
			}
		}
	}
	return GUIWindow::ProcessMessages(message, lparam, hparam);
}

//Нажатие кнопок
void SndEditorMainWindow::KeyPressed(int key, bool bSysKey)
{
/*	if(options->kbFocus > 0)
	{
		SndBaseControl * ctrl = options->kbFocus[options->kbFocus - 1];
		ctrl->KeyboardProcess(key, bSysKey);
		if(key == VK_TAB && bSysKey)
		{
			if(options->kbFocus > 0 && options->kbFocus[options->kbFocus - 1] == ctrl)
			{
				ctrl->ProcessMessages(GUIMSG_FORCE_DWORD, SndBaseControl::sm_switchfocus, 0);
			}			
		}
	}*/
}


//Проверить на попадание в закладку
bool SndEditorMainWindow::IsInsideTab(Tab & tab, GUIPoint & p)
{	
	if(p.y < 3) return false;
	GUIPoint * ps = &tab.points[ARRSIZE(tab.points) - 1];
	for(long i = 0; i < ARRSIZE(tab.points); i++)
	{
		GUIPoint * pe = &tab.points[i];
		long dx = pe->x - ps->x;
		long dy = pe->y - ps->y;
		long px = p.x - ps->x;
		long py = p.y - ps->y;
		if(dy*px - dx*py >= 0)
		{
			return false;
		}
		ps = pe;
	}
	return true;
}

//Выделить закладку
void SndEditorMainWindow::SelectTab(long tabIndex)
{
	options->WavePreviewStop();
	if(tabs[selTab].tab)
	{
		delete tabs[selTab].tab;
		tabs[selTab].tab = null;
	}
	selTab = tabIndex;
	tabs[selTab].tab = (this->*tabs[selTab].createTab)();
}


#include "FormSounds.h"
#include "FormWaves.h"
#include "FormAnimation.h"
#include "FormPolygon.h"
#include "FormGame.h"

//Создание закладак по необходимости
GUIControl * SndEditorMainWindow::CreateTabSounds()
{ 
	GUIRectangle rect = GetClientRect();
	rect.Top = TabHeight;
	rect.Height -= TabHeight;
	return NEW FormSounds(this, rect);
}

GUIControl * SndEditorMainWindow::CreateTabWaves()
{ 
	GUIRectangle rect = GetClientRect();
	rect.Top = TabHeight;
	rect.Height -= TabHeight;
	return NEW FormWaves(this, rect);
}

GUIControl * SndEditorMainWindow::CreateTabAnimation()
{ 
	GUIRectangle rect = GetClientRect();
	rect.Top = TabHeight;
	rect.Height -= TabHeight;
	return NEW FormAnimation(this, rect);
}

GUIControl * SndEditorMainWindow::CreateTabPolygon()
{ 
	GUIRectangle rect = GetClientRect();
	rect.Top = TabHeight;
	rect.Height -= TabHeight;
	return NEW FormPolygon(this, rect);
}

GUIControl * SndEditorMainWindow::CreateTabGame()
{ 
	GUIRectangle rect = GetClientRect();
	rect.Top = TabHeight;
	rect.Height -= TabHeight;
	return NEW FormGame(this, rect);
}

//Эвент закрытия редактора
void _cdecl SndEditorMainWindow::OnCloseApp(GUIControl* sender)
{
	setCloseState = true;
}


