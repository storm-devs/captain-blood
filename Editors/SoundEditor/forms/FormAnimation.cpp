//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormEffects
//============================================================================================

#include "FormAnimation.h"
#include "external\FormTimeLine.h"
#include "external\PreviewAnimation.h"
#include "lists\FormListAnimations.h"
#include "lists\FormListNodes.h"
#include "sliders\BaseSlider.h"
#include "ComboBox\FormComboBox.h"
#include "FormCheckBox.h"


FormAnimation::FormAnimation(GUIWindow * parent, GUIRectangle & rect) : GUIControl(parent)
{
#ifndef NO_TOOLS
	options->sa->EditEnablePreview(true);
#endif
	SetClientRect(rect);
	SetDrawRect(rect);	
	GUIRectangle r;
	long tmh = (rect.h*c_timelineHeightInPercents)/100;
	//Список анимаций
	r.x = 0;
	r.y = 0;
	r.w = c_editfield_width;
	r.h = rect.h - tmh - c_editfield_height;
	listOfAnimations = NEW FormListAnimations(options->formOptions.formAnimations.aniList, this, r);
	listOfAnimations->onChangeFocus.SetHandler(this, (CONTROL_EVENT)&FormAnimation::DoSetMovie);
	//Окно предпросмотра
	r.x = c_editfield_width + 1;
	r.y = 0;
	r.w = rect.w - c_editfield_width - 2;
	r.h = rect.h - tmh - 2;
	preview = NEW PreviewAnimation(this, r);
	//Таймлайн
	r.x = 0;
	r.w = rect.w;
	r.h = tmh;
	r.y = rect.h - r.h - 1;
	timeLine = NEW FormTimeLine(this, r, preview);
	timeLine->onSelectEvent.SetHandler(this, (CONTROL_EVENT)&FormAnimation::DoChangeSelectEvent);
	//Специализированные элементы
	listOfNodes = null;
	soundSelect = null;
	locatorSelect = null;
	checkLocalGlobal = null;
	checkBind = null;
	checkVoice = null;
	fadeTime = null;
	eventPlay = null;
	eventStop = null;
	selectAnxNodeForEvent = -1;	
	//Режим дизайнера
	long params_y = listOfAnimations->GetClientRect().y + listOfAnimations->GetClientRect().h;
	r.x = 0;
	r.y = params_y;
	r.w = c_editfield_width;
	r.h = c_editfield_height;
	listOfNodes = NEW FormListNodes(options->formOptions.formAnimations.aniList, this, r);
	listOfNodes->onAddNode.SetHandler(this, (CONTROL_EVENT)&FormAnimation::DoAddNode);
	//Режим звуковика
	GUIRectangle r1;
	r.x = c_editfield_border;
	r.y = params_y + c_editfield_border;
	r.w = c_editfield_width - 2*c_editfield_border;
	r.h = c_editfield_ed_height;
	r1.x = r.x;
	r1.y = r.y + c_editfield_ed_height;
	r1.w = r.w;
	r1.h = c_editfield_edl_height;
	soundSelect = NEW FormComboBox(this, r, r1);
	soundSelect->SetTitle(&options->imageSound, options->GetString(SndOptions::s_ani_sound));
	soundSelect->Hint = options->GetString(SndOptions::s_hint_ani_sel_sound);
	soundSelect->onChange.SetHandler(this, (CONTROL_EVENT)&FormAnimation::DoSelectSound);
	const array<ProjectSound *> & sounds = project->SoundArray();
	for(dword i = 0; i < sounds.Size(); i++)
	{
		if(sounds[i])
		{
			const ExtName & name = sounds[i]->GetName();
			if(name.len > 0 && name.len < ProjectAnimation::c_nameLen)
			{
				soundSelect->AddItem(null, name.str, i);
			}
		}
	}
	r.y += c_editfield_ed_height + c_editfield_border;
	r1.y += c_editfield_ed_height + c_editfield_border;
	checkLocalGlobal = NEW FormCheckBox(this, r.pos);
	checkLocalGlobal->SetText(options->GetString(SndOptions::s_ani_event_local));
	checkLocalGlobal->onChange.SetHandlerEx(this, &FormAnimation::DoChangeLocalGlobal);
	r.y += checkLocalGlobal->GetClientRect().h + c_editfield_border;
	r1.y += checkLocalGlobal->GetClientRect().h + c_editfield_border;
	locatorSelect = NEW FormComboBox(this, r, r1);
	locatorSelect->SetTitle(&options->imageLocator, options->GetString(SndOptions::s_ani_locator));
	locatorSelect->Hint.Reserve(4096);
	locatorSelect->Hint = options->GetString(SndOptions::s_hint_ani_sel_locator);
	locatorSelect->onChange.SetHandler(this, (CONTROL_EVENT)&FormAnimation::DoSelectLocator);
	r.y += c_editfield_ed_height + c_editfield_border;
	checkBind = NEW FormCheckBox(this, r.pos);
	checkBind->SetText(options->GetString(SndOptions::s_ani_event_bind));
	checkBind->onChange.SetHandlerEx(this, &FormAnimation::DoChangeBind);
	r.y += checkBind->GetClientRect().h + c_editfield_border;
	r.h = c_editfield_slider;
	r.w = c_editfield_width*2/3 - 2*c_editfield_border;
	fadeTime = NEW BaseSlider(this, r);
	fadeTime->SetPosition(0, true);
	fadeTime->SetHorisontal();
	fadeTime->SetRange(0, 1500);
	fadeTime->SetPageSize(100);		
	fadeTime->SetRestore(true);
	fadeTime->Enabled = false;
	fadeTime->onUpdate.SetHandlerEx(this, &FormAnimation::DoChangeFadetime);
	r.y += c_editfield_slider + c_editfield_border;
	checkVoice = NEW FormCheckBox(this, r.pos);
	checkVoice->SetText(options->GetString(SndOptions::s_ani_event_voice));
	checkVoice->onChange.SetHandlerEx(this, &FormAnimation::DoChangeVoice);
	r.y += 30;
	r.w = c_editfield_button_w;
	r.h = c_editfield_button_h;
	eventPlay = NEW FormButton(this, r);
	eventPlay->image = &options->imagePlay;
	eventPlay->imageW = 32;
	eventPlay->imageH = 32;
	eventPlay->Hint = options->GetString(SndOptions::s_ani_event_play);
	eventPlay->onUp.SetHandlerEx(this, &FormAnimation::DoPlaySelectEvent);
	r.x += c_editfield_button_w + 5;
	eventStop = NEW FormButton(this, r);
	eventStop->image = &options->imageStop;
	eventStop->imageW = 32;
	eventStop->imageH = 32;
	eventStop->Hint = options->GetString(SndOptions::s_ani_event_stop);
	eventStop->onUp.SetHandlerEx(this, &FormAnimation::DoStopPlaySelectEvent);
	DoSetMovie(null);	
}

FormAnimation::~FormAnimation()
{
	project->AnimationPrepareForSounds(UniqId::zeroId);
}

//Рисование
void FormAnimation::Draw()
{
	GUIControl::Draw();
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	if(fadeTime && fadeTime->Visible)
	{
		GUIRectangle rs = fadeTime->GetDrawRect();
		fadeTime->ClientToScreen(rs);
		float x = float(rs.x + rs.w + c_editfield_border);
		float fontHeight = options->uiFont->GetHeight("0");
		float y = rs.y + (rs.h - fontHeight)*0.5f - 0.5f;
		float time = fadeTime->GetPosition()*0.001f;
		dword color = fadeTime->Enabled ? options->colorTextLo : options->black2Bkg[5];
		options->render->Print(x, y, color, "%.3f %s", time, options->GetString(SndOptions::s_units_time_sec));
	}
	long tmh = (r.h*c_timelineHeightInPercents)/100;
	GUIHelper::Draw2DLine(r.x + c_editfield_width, r.y, r.x + c_editfield_width, r.y + r.h - tmh, options->black2Bkg[0]);
	GUIRectangle laRect = listOfAnimations->GetDrawRect();
	listOfAnimations->ClientToScreen(laRect);	
	GUIHelper::Draw2DLine(laRect.x, laRect.y + laRect.h, laRect.x + laRect.w, laRect.y + laRect.h, options->black2Bkg[0]);
	GUIHelper::Draw2DLine(r.x, r.y + r.h - tmh, r.x + r.w, r.y + r.h - tmh, options->black2Bkg[0]);
}

//Установить событие
void _cdecl FormAnimation::DoSetMovie(GUIControl* sender)
{
	//Для начала спрячем всё
	timeLine->Visible = false;
	timeLine->Enabled = false;
	preview->Visible = false;
	preview->Enabled = false;
	if(listOfNodes)
	{
		listOfNodes->Visible = false;
		listOfNodes->SetEnabled(false);
	}
	HideEventParams();
	//Получим текущие параметры
	const UniqId & animationId = listOfAnimations->GetFocusAnimation();
	long movieIndex = listOfAnimations->GetFocusMovie();
	//Сообщим об изменениях проекту
	project->AnimationPrepareForSounds(animationId);
	//Поставим изменения таймлайну
	ProjectAnimation * ani = project->AnimationGet(animationId);
	if(movieIndex >= 0)
	{
		timeLine->Visible = true;
		timeLine->Enabled = true;
		preview->Visible = true;
		preview->Enabled = true;
		ani->CheckAnimation();
		listOfAnimations->UpdateMovieStages();
		timeLine->SetMovie(animationId, movieIndex);
		//Установим режим работы таймлайна в зависимости от режима редактора и стадии ролика		
		dword stage = ani->GetMovieStage(movieIndex);
		bool isEnableEditNodes = false;
		bool isEnableEditEvents = false;
		switch(options->policy)
		{
			case policy_sounder:
				if(stage == movie_stage_events)
				{					
					isEnableEditEvents = true;
				}
				break;
			case policy_designer:
				if(stage == movie_stage_movie)
				{
					isEnableEditNodes = true;
				}
				break;
			default:
				//Добавилось чтото новое
				Assert(false);
		}
		if(isEnableEditNodes && listOfNodes)
		{
			listOfNodes->Visible = true;
			listOfNodes->SetEnabled(true);
			listOfNodes->InitList();
		}		
		timeLine->EnableEditNodes(isEnableEditNodes);
		timeLine->EnableEditEvents(isEnableEditEvents);
		DoChangeSelectEvent(null);
	}else{
		timeLine->SetMovie(UniqId::zeroId, -1);
		timeLine->EnableEditNodes(false);
		timeLine->EnableEditEvents(false);
	}
}

void _cdecl FormAnimation::DoAddNode(GUIControl* sender)
{
	long index = listOfNodes->GetFocusNode();
	if(index >= 0)
	{
		timeLine->AddNode(index);
	}	
}

void _cdecl FormAnimation::DoChangeSelectEvent(GUIControl* sender)
{	
	Assert(sizeof(int) == sizeof(long));
	long selectEvent = -1;
	if(!timeLine->GetSelectedEvent((int &)selectAnxNodeForEvent, (int &)selectEvent))
	{
		selectAnxNodeForEvent = -1;
		selectEventId.Reset();
		HideEventParams();
		return;
	}
	string notInList;
	if(!soundSelect) return;
	const UniqId & animationId = listOfAnimations->GetFocusAnimation();	
	ProjectAnimation * ani = project->AnimationGet(animationId);	
	if(!ani || !ani->GetEventUIDbyIndex(selectAnxNodeForEvent, selectEvent, selectEventId))
	{
		selectAnxNodeForEvent = -1;
		selectEventId.Reset();
		HideEventParams();
		return;
	}
	long movieIndex = listOfAnimations->GetFocusMovie();
	Assert(movieIndex >= 0);
	dword movieStage = ani->GetMovieStage(movieIndex);
	//Ограничения прав
	bool isEnable = false;
	if(options->policy == policy_sounder)
	{
		if(movieStage == movie_stage_events)
		{
			isEnable = true;
		}
	}else
	if(options->policy == policy_designer)
	{
		if(movieStage == movie_stage_movie)
		{
			selectAnxNodeForEvent = -1;
			selectEventId.Reset();
			HideEventParams();
			return;
		}
	}else{
		Assert(false);
	}
	//Список звуков
	soundSelect->Visible = true;
	soundSelect->Enabled = isEnable;
	const char * soundName = ani->GetEventSoundName(selectAnxNodeForEvent, selectEventId);
	if(string::NotEmpty(soundName))
	{
		notInList = soundName;
		notInList += options->GetString(SndOptions::s_ani_not_in_list);
		soundSelect->SetUnselectText(notInList);
	}else{
		soundSelect->SetUnselectText(null);
	}
	soundSelect->SelectItemByText(soundName);
	//Список локаторов
	locatorSelect->Visible = true;
	locatorSelect->Enabled = isEnable;
	const char * locatorName = ani->GetEventLocator(selectAnxNodeForEvent, selectEventId);
	if(string::NotEmpty(locatorName))
	{
		notInList = locatorName;
		notInList += options->GetString(SndOptions::s_ani_not_in_list);
		locatorSelect->SetUnselectText(notInList);
	}else{
		locatorSelect->SetUnselectText(null);
	}
	locatorSelect->Clear();
	dword count = preview->GetLocatorsCount();
	for(dword i = 0; i < count; i++)
	{
		const char * locName = preview->GetLocatorName(i);
		long len = string::Len(locName) + 1;
		if(len > 1 && len <= ProjectAnimation::c_nameLen)
		{
			locatorSelect->AddItem(null, locName, i);
		}
	}
	locatorSelect->SelectItemByText(locatorName);
	//Время фэйда
	fadeTime->Visible = true;	
	float fTime = ani->GetEventFadeTime(selectAnxNodeForEvent, selectEventId);
	fadeTime->SetPosition(long(fTime*1000.0f));
	//Флаги
	dword flags = ani->GetEventFlags(selectAnxNodeForEvent, selectEventId);
	checkLocalGlobal->Visible = true;	
	checkLocalGlobal->SetCheck((flags & ProjectAnimation::event_flags_local) != 0);
	checkBind->Visible = true;	
	checkBind->SetCheck((flags & ProjectAnimation::event_flags_bind) != 0);
	checkVoice->Visible = true;	
	checkVoice->SetCheck((flags & ProjectAnimation::event_flags_voice) != 0);
	fadeTime->Enabled = isEnable;
	checkLocalGlobal->Enabled = isEnable;
	checkBind->Enabled = isEnable;
	checkVoice->Enabled = isEnable;
	eventPlay->Enabled = true;
	eventPlay->Visible = true;
	eventStop->Enabled = true;
	eventStop->Visible = true;
}


void FormAnimation::HideEventParams()
{
	if(!soundSelect) return;
	soundSelect->Visible = false;
	soundSelect->Enabled = false;
	locatorSelect->Visible = false;
	locatorSelect->Enabled = false;
	checkLocalGlobal->Visible = false;
	checkLocalGlobal->Enabled = false;
	checkBind->Visible = false;
	checkBind->Enabled = false;
	checkVoice->Visible = false;
	checkVoice->Enabled = false;
	fadeTime->Visible = false;
	fadeTime->Enabled = false;
	eventPlay->Enabled = false;
	eventPlay->Visible = false;
	eventStop->Enabled = false;
	eventStop->Visible = false;
}

//Изменён выделеный эвент
bool FormAnimation::IsCanEditEvent()
{
	if(selectAnxNodeForEvent < 0)
	{
		return false;
	}
	const UniqId & animationId = listOfAnimations->GetFocusAnimation();	
	ProjectAnimation * ani = project->AnimationGet(animationId);
	if(!ani)
	{
		return false;
	}	
	long movieIndex = listOfAnimations->GetFocusMovie();
	if(movieIndex < 0)
	{
		return false;
	}
	dword movieStage = ani->GetMovieStage(movieIndex);
	if(options->policy == policy_sounder)
	{
		if(movieStage == movie_stage_events)
		{
			return true;
		}
	}
	return false;
}

//Выбрать звук для события
void _cdecl FormAnimation::DoSelectSound(GUIControl* sender)
{
	if(!IsCanEditEvent()) return;
	const UniqId & animationId = listOfAnimations->GetFocusAnimation();
	ProjectAnimation * ani = project->AnimationGet(animationId);	
	Assert(ani);
	ExtName * text = soundSelect->GetCurrentItemText();
	if(text)
	{
		bool criticalError_cant_change_event_sound = ani->SetEventSoundName(selectAnxNodeForEvent, selectEventId, text->str);
		Assert(criticalError_cant_change_event_sound);
	}
}

//Выбрать локатор для события
void _cdecl FormAnimation::DoSelectLocator(GUIControl* sender)
{
	if(selectAnxNodeForEvent < 0)
	{
		preview->SelectLocator(-1);
		return;
	}
	if(!IsCanEditEvent()) return;
	const UniqId & animationId = listOfAnimations->GetFocusAnimation();
	ProjectAnimation * ani = project->AnimationGet(animationId);	
	Assert(ani);
	ExtName * text = locatorSelect->GetCurrentItemText();
	if(text)
	{
		bool criticalError_cant_change_event_locator = ani->SetEventLocator(selectAnxNodeForEvent, selectEventId, text->str);
		Assert(criticalError_cant_change_event_locator);
		long id = locatorSelect->GetCurrentItemId();
		preview->SelectLocator(id);
		locatorSelect->Hint = options->GetString(SndOptions::s_hint_ani_sel_locator);
		locatorSelect->Hint += "\n(";
		locatorSelect->Hint += text->str;
		locatorSelect->Hint += ")";
	}else{
		preview->SelectLocator(-1);
		locatorSelect->Hint = options->GetString(SndOptions::s_hint_ani_sel_locator);
	}
}

//Время фэйда
void _cdecl FormAnimation::DoChangeFadetime(GUIControl* sender)
{
	if(!IsCanEditEvent()) return;
	Assert(fadeTime);
	if(selectAnxNodeForEvent >= 0)
	{
		float fTime = fadeTime->GetPosition()*0.001f;
		const UniqId & animationId = listOfAnimations->GetFocusAnimation();	
		ProjectAnimation * ani = project->AnimationGet(animationId);
		ani->SetEventFadeTime(selectAnxNodeForEvent, selectEventId, fTime);
	}
}

//Флажки эвента
void _cdecl FormAnimation::DoChangeLocalGlobal(GUIControl* sender)
{
	if(!IsCanEditEvent()) return;
	Assert(checkLocalGlobal);
	Assert(locatorSelect);
	locatorSelect->Enabled = checkLocalGlobal->IsChecked();
	if(selectAnxNodeForEvent >= 0)
	{
		const UniqId & animationId = listOfAnimations->GetFocusAnimation();	
		ProjectAnimation * ani = project->AnimationGet(animationId);
		ani->SetEventFlags(selectAnxNodeForEvent, selectEventId, ProjectAnimation::event_flags_local, checkLocalGlobal->IsChecked());
	}
}

void _cdecl FormAnimation::DoChangeBind(GUIControl* sender)
{
	if(!IsCanEditEvent()) return;
	Assert(checkBind);
	Assert(fadeTime);
	fadeTime->Enabled = checkBind->IsChecked();
	if(selectAnxNodeForEvent >= 0)
	{
		const UniqId & animationId = listOfAnimations->GetFocusAnimation();	
		ProjectAnimation * ani = project->AnimationGet(animationId);
		ani->SetEventFlags(selectAnxNodeForEvent, selectEventId, ProjectAnimation::event_flags_bind, checkBind->IsChecked());
	}
}

void _cdecl FormAnimation::DoChangeVoice(GUIControl* sender)
{	
	if(!IsCanEditEvent()) return;
	Assert(checkVoice);
	if(selectAnxNodeForEvent >= 0)
	{
		const UniqId & animationId = listOfAnimations->GetFocusAnimation();	
		ProjectAnimation * ani = project->AnimationGet(animationId);
		ani->SetEventFlags(selectAnxNodeForEvent, selectEventId, ProjectAnimation::event_flags_voice, checkVoice->IsChecked());
	}
}

void _cdecl FormAnimation::DoPlaySelectEvent(GUIControl* sender)
{
	timeLine->StopEvent();
	timeLine->PlayEvent();	
}

void _cdecl FormAnimation::DoStopPlaySelectEvent(GUIControl* sender)
{
	timeLine->StopEvent();
}

