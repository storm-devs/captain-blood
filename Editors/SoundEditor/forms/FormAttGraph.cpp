

#include "FormAttGraph.h"
#include "FormEdit.h"
#include "ComboBox\FormComboBox.h"
#include "Lists\FormListBase.h"
#include "FormEditName.h"
#include "FormButton.h"
#include "FormMessageBox.h"



FormAttGraph::PointSlider::PointSlider(GUIControl * parent, const GUIRectangle & rect) : BaseSlider(parent, rect)
{
	SetVertical();
	SetSnapSlider(false);
	SetInverse(true);
	SetRange(0, 10000);
	SetPosition(0, true);
	SetPageSize((6*rect.w*(maxValue - minValue))/(7*rect.h) + 1);
}

//Установить значение 0..1
void FormAttGraph::PointSlider::SetValue(float v)
{
	v = Clampf(v, 0.0f, 1.0f)*10000.0f;
	SetPosition(long(v));
}

float FormAttGraph::PointSlider::GetValue()
{
	return GetPosition()*0.0001f;
}

//Рисование
void FormAttGraph::PointSlider::Draw()
{
	GUIRectangle r;
	CalcSliderRect(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg2White[7]);
	GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
}

FormAttGraph::EarSlider::EarSlider(GUIControl * parent, const GUIRectangle & rect) : BaseSlider(parent, rect)
{
	SetHorisontal();
	SetSnapSlider(false);
	SetRange(0, 10000);
	SetPageSize((rect.h*(maxValue - minValue))/rect.w + 1);
}

//Установить значение 0..1
void FormAttGraph::EarSlider::SetValue(float v)
{
	v = Clampf(v, 0.0f, 1.0f)*10000.0f;
	SetPosition(long(v));
}

//Возвращает значение 0..1
float FormAttGraph::EarSlider::GetValue()
{
	return GetPosition()*0.0001f;
}

//Применять ли модифицированную громкость к звуку
bool FormAttGraph::EarSlider::IsActive()
{
	return (options->dndControl == this);
}

//Рисование
void FormAttGraph::EarSlider::Draw()
{
	if(!Visible) return;
	GUIRectangle rs = GetDrawRect();
	ClientToScreen(rs);
	GUIRectangle r;
	CalcSliderRect(r);
	GUIHelper::DrawSprite(r.x + (r.w - c_ear_size)/2, r.y + (r.h - c_ear_size)/2, c_ear_size, c_ear_size, &options->imageEar);
	//GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);	
}

//Обновились параметры слайдера
void FormAttGraph::EarSlider::OnUpdateParameters()
{
	BaseSlider::OnUpdateParameters();
//	sliderSize = sliderStatic;
}


FormAttGraph::FormAttGraph(GUIControl * parent, const GUIRectangle & rect, bool isLightVersion) : GUIControl(parent)
{
	isLightVersion = false;
	SetClientRect(rect);
	SetDrawRect(rect);
	currentSound = null;
	isInitState = false;
	earPlay = false;
	title.Init(options->GetString(SndOptions::s_snd_attenuation_curve));
	//Вертикальные слайдеры-точки
	for(long i = 0; i < 4; i++)
	{
		c[i] = 0.0f;
		p[i] = null;
	}
	minDist = 0.0f;
	maxDist = 1.0f;
	GUIRectangle r = rect;
	r.x = r.y = 0;
	GUIRectangle gr = CalcGraphRect(r);
	GUIRectangle cr, r2;
	cr.y = gr.y + c_combo_h;
	cr.w = c_slider_width;
	cr.h = gr.h - c_combo_h;
	long slidersPos[4] = {c_sliderpos0, c_sliderpos1, c_sliderpos2, c_sliderpos3};
	for(long i = 0; i < 4; i++)
	{
		cr.x = gr.x + (gr.w*slidersPos[i])/100;	
		p[i] = NEW PointSlider(this, cr);
		p[i]->onUpdate.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnUpdate);
		p[i]->SetValue(0.5f);
	}
	//Горизонтальный слайдер-ухо
	if(!isLightVersion)
	{
		cr.x = gr.x - c_ear_size/2;
		cr.y = rect.h - c_down_space + c_space;
		cr.w = gr.w + c_ear_size;
		cr.h = c_ear_size;
		ear = NEW EarSlider(this, cr);
	}else{
		ear = null;
	}
	//Дистанции
	//min
	if(!isLightVersion)
	{
		cr.x = gr.x - c_ear_size/2 - c_space - c_edit_w;
	}else{
		cr.x = gr.x - c_edit_w/2;
	}
	cr.y = rect.h - c_down_space - c_edit_h/2;
	cr.w = c_edit_w;
	cr.h = c_edit_h;
	editMinDist = NEW FormEdit(this, cr);
	editMinDist->floatValue = &minDist;
	editMinDist->UpdateText();
	editMinDist->onAccept.SetHandlerEx(this, &FormAttGraph::OnChangeMin);
	long tw = long(options->uiFont->GetLength(options->GetString(SndOptions::s_snd_graph_minDist)) + 0.5f);
	textMinDist.x = cr.x + cr.w - tw;
	textMinDist.y = cr.y + c_edit_h + c_space;
	//max
	if(!isLightVersion)
	{
		cr.x = gr.x + gr.w + c_ear_size/2 + c_space;
	}else{
		cr.x = gr.x + gr.w - c_edit_w/2;
	}
	editMaxDist = NEW FormEdit(this, cr);
	editMaxDist->floatValue = &maxDist;
	editMaxDist->UpdateText();
	editMaxDist->onAccept.SetHandlerEx(this, &FormAttGraph::OnChangeMax);
	textMaxDist.x = cr.x;
	textMaxDist.y = cr.y + c_edit_h + c_space;
	if(!isLightVersion)
	{
		//Список доступных графиков
		cr.x = r.w - c_combo_w - (c_button_w + sgc_header_controls_space)*3 - c_space;
		cr.y = r.y + (c_title_h - c_combo_h - 1)/2;
		cr.w = c_combo_w;
		cr.h = c_combo_h;
		r2.x = cr.x;
		r2.y = cr.y + cr.h + 1;
		r2.w = c_combo_list_w;
		r2.h = FormListBase::FindListHeight(c_combo_list_h);
		graphsList = NEW FormComboBox(this, cr, r2);
		graphsList->SetTitle(&options->imageAttenuations, options->GetString(SndOptions::s_snd_sel_attenuations));
		graphsList->Hint = options->GetString(SndOptions::s_hint_sound_attenuations);
		graphsList->onChange.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnChangeGraph);
		FillPresetsList();
		//Кнопки
		r.x = cr.x + cr.w + sgc_header_controls_space;
		r.y = cr.y;
		r.w = c_button_w;
		r.h = c_combo_h;
		buttonAdd = NEW FormButton(this, r);
		buttonAdd->image = &options->imageAddAtts;
		buttonAdd->onUp.SetHandlerEx(this, &FormAttGraph::OnAttGraphAdd);
		buttonAdd->Hint = options->GetString(SndOptions::s_hint_sound_att_add);
		r.x += c_button_w + sgc_header_controls_space;
		buttonRename = NEW FormButton(this, r);
		buttonRename->image = &options->imageRename;
		buttonRename->onUp.SetHandlerEx(this, &FormAttGraph::OnAttGraphRename);
		buttonRename->Hint = options->GetString(SndOptions::s_hint_sound_att_rename);	
		r.x += c_button_w + sgc_header_controls_space;
		if(options->policy == policy_designer)
		{
			buttonDelete = NEW FormButton(this, r);
			buttonDelete->image = &options->imageDelete;
			buttonDelete->onUp.SetHandlerEx(this, &FormAttGraph::OnAttGraphDelete);
			buttonDelete->Hint = options->GetString(SndOptions::s_hint_sound_att_add);
		}else{
			buttonDelete = null;
		}
	}else{
		graphsList = null;
		buttonAdd = null;
		buttonRename = null;
		buttonDelete = null;
	}
	//Расчитываем график
	UpdateGraph(false);
	OnChangeGraph(null);	
}


FormAttGraph::~FormAttGraph()
{
}

//Пересчитать график
void FormAttGraph::UpdateGraph(bool updateSoundParams)
{
	//Обновляем график
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIRectangle gr = CalcGraphRect(r);
	segments[0].x = r.x + c_space + (r.w*c_rightleft_free)/100;
	segments[0].y = gr.y;
	for(long i = 0; i < c_segments; i++)
	{
		float pos = i*(1.0f/float(c_segments - 1));
		float kDist = minDist + (maxDist - minDist)*pos;
		float k = (kDist*kDist - minDist*minDist)/(maxDist*maxDist - minDist*minDist);
		float att = AttenuationBySpline(k, c[0], c[1], c[2], c[3]);
		float x = gr.x + pos*gr.w;
		float y = gr.y + (1.0f - Clampf(att))*gr.h;
		segments[i + 1].x = long(x);
		segments[i + 1].y = long(y);
	}
	segments[c_segments + 1].x = r.x + r.w - c_space - (r.w*c_rightleft_free)/100;
	segments[c_segments + 1].y = segments[c_segments].y;
	//Сохраняем состояние в проекте
	if(updateSoundParams && !isInitState)
	{
		ProjectObject * saveObject = null;
		SoundAttGraph * ag = GetCurrentObject(saveObject);
		if(ag)
		{
			bool needUpdate = false;
			for(long i = 0; i < ARRSIZE(ag->c); i++)
			{
				if(fabsf(ag->c[i] - c[i]) > 1e-10f)
				{
					ag->c[i] = c[i];
					needUpdate = true;
				}
			}
			if(fabsf(ag->minDist - minDist) > 1e-10f)
			{
				ag->minDist = minDist;
				needUpdate = true;
			}
			if(fabsf(ag->maxDist - maxDist) > 1e-10f)
			{
				ag->maxDist = maxDist;
				needUpdate = true;
			}
			if(needUpdate && saveObject)
			{
				saveObject->SetToSave();
			}
		}
	}
	if(currentSound) currentSound->UpdateExportParams();
}

//Установить текущий звук, в который прописывать выбранный график
void FormAttGraph::SetCurrentSound(const UniqId & id)
{
	currentSound = project->SoundGet(id);
	previewWaveId.Reset();
	OnRestoreSelect(null);
}

//Установить текущую волну для настройки графика
void FormAttGraph::SetCurrentWave(const UniqId & id)
{
	previewWaveId = id;
	ProjectWave * pw = project->WaveGet(previewWaveId);
	if(pw)
	{
		if(earPlay && pw->IsPlay())
		{
			pw->Stop();
		}
		ear->Visible = true;
	}else{
		earPlay = false;
	}
}

//Рисование
void FormAttGraph::Draw()
{
	//Постоянно контролируем волну для прослушивания
	ProjectWave * wave = project->WaveGet(previewWaveId);
	if(ear)
	{
		ear->Visible = (wave != null);
	}
	//Рисуем элементы
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, r.h, options->bkg);
	GUIHelper::Draw2DRect(r.x, r.y, r.w, c_title_h, options->bkg2White[1]);
	GUIHelper::Draw2DLine(r.x, r.y + c_title_h, r.x + r.w, r.y + c_title_h, options->black2Bkg[4]);
	bool isEarActive = false;
	long earPos = 0;
	if(ear)
	{
		earPos = long(segments[1].x + ear->GetValue()*(segments[c_segments].x - segments[1].x) + 0.45f);
		isEarActive = ear->IsActive();	
		if(!isEarActive)
		{
			GUIHelper::Draw2DLine(earPos, segments[1].y - c_space, earPos, r.y + r.h, options->black2Bkg[6]);
		}
	}
	GUIHelper::Draw2DLine(segments[0].x, segments[c_segments + 1].y, segments[c_segments + 1].x, segments[c_segments + 1].y, options->black2Bkg[3]);
	GUIHelper::DrawLines(segments, c_segments + 2, options->black2Bkg[0]);
	GUIHelper::Draw2DLine(segments[1].x, segments[1].y - c_space, segments[1].x, r.y + r.h, options->black2Bkg[3]);
	GUIHelper::Draw2DLine(segments[c_segments].x, segments[1].y - c_space, segments[c_segments].x, r.y + r.h, options->black2Bkg[3]);
	if(isEarActive)
	{
		GUIHelper::Draw2DLine(earPos, segments[1].y - c_space, earPos, r.y + r.h, options->black2Bkg[0]);
		if(wave && !wave->IsPlay())
		{
			earPlay = true;
			wave->Play();
		}
	}else{
		if(earPlay)
		{
			if(wave && wave->IsPlay())
			{
				wave->Stop();
			}
		}
		earPlay = false;
	}
	if(wave && earPlay)
	{
		SoundBankFileAttenuation abatt;
		abatt.c[0] = c[0];
		abatt.c[1] = c[1];
		abatt.c[2] = c[2];
		abatt.c[3] = c[3];
		abatt.minDist2 = minDist*minDist;
		abatt.maxDist2 = maxDist*maxDist;
		abatt.kNorm2 = 1.0f/(abatt.maxDist2 - abatt.minDist2);
		float dist = minDist + ear->GetValue()*(maxDist - minDist);
		float vol = abatt.Attenuation(dist*dist);			
		wave->PlayVolume(vol);
	}
	options->uiFont->SetColor(options->colorTextLo);
	options->uiFont->Print(float(r.x + textMinDist.x), float(r.y + textMinDist.y), options->GetString(SndOptions::s_snd_graph_minDist));
	options->uiFont->Print(float(r.x + textMaxDist.x), float(r.y + textMaxDist.y), options->GetString(SndOptions::s_snd_graph_maxDist));
	options->uiFont->Print(float(r.x + sgc_header_left_space), float(r.y + (c_title_h - title.h)/2), title.str);	
	//GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[5]);
	GUIControl::Draw();
	ProjectObject * saveObject = null;
	SoundAttGraph * ag = GetCurrentObject(saveObject);
	if(!ag)
	{
		dword c = (options->bkg & 0x00ffffff) | 0xc0000000;
		GUIHelper::Draw2DRectAlpha(r.x, r.y + c_title_h + 1, r.w, r.h - c_title_h - 1, c);
	}
}

//Обновить состояние графика
void _cdecl FormAttGraph::OnUpdate(GUIControl* sender)
{
	for(long i = 0; i < 4; i++)
	{
		if(p[i])
		{
			c[i] = SoundAttGraph::cR[i].minValue + p[i]->GetValue()*(SoundAttGraph::cR[i].maxValue - SoundAttGraph::cR[i].minValue);
		}		
	}
	UpdateGraph(true);
}

//Изменилась нижняя граница
void _cdecl FormAttGraph::OnChangeMin(GUIControl* sender)
{	
	if(minDist > SoundAttGraph::minD.maxValue) minDist = SoundAttGraph::minD.maxValue;
	if(minDist > maxDist - SoundAttGraph::mmDist) minDist = maxDist - SoundAttGraph::mmDist;
	if(minDist < SoundAttGraph::minD.minValue)
	{
		minDist = SoundAttGraph::minD.minValue;
		if(maxDist < minDist + SoundAttGraph::mmDist) maxDist = minDist + SoundAttGraph::mmDist;
	}
	editMinDist->UpdateText();
	UpdateGraph(true);
}

//Изменилась верхняя граница
void _cdecl FormAttGraph::OnChangeMax(GUIControl* sender)
{
	if(maxDist < SoundAttGraph::maxD.minValue) maxDist = SoundAttGraph::maxD.minValue;
	if(maxDist < minDist + SoundAttGraph::mmDist) maxDist = minDist + SoundAttGraph::mmDist;
	if(maxDist > SoundAttGraph::maxD.maxValue)
	{
		maxDist = SoundAttGraph::maxD.maxValue;
		if(minDist > maxDist - SoundAttGraph::mmDist) minDist = maxDist - SoundAttGraph::mmDist;
	}
	editMaxDist->UpdateText();
	UpdateGraph(true);
}

//Расчитать прямоугольник графика из общего
__forceinline GUIRectangle FormAttGraph::CalcGraphRect(const GUIRectangle & cur)
{
	GUIRectangle r;
	//Рабочая область
	long rlf = (cur.w*c_rightleft_free)/100 + c_space;
	r.x = cur.x + rlf;
	r.y = cur.y + c_space + c_title_h;
	r.w = cur.w - rlf*2;
	r.h = cur.h - c_space - c_down_space - c_title_h;
	//Поля прямых участков
	long border = (r.w*c_borders_in_percents)/100;
	r.x += border;
	r.w -= border*2;
	return r;
}

//Прочитать параметры графика
void FormAttGraph::ReadCurrentParams()
{
	ProjectObject * saveObject = null;
	SoundAttGraph * ag = GetCurrentObject(saveObject);
	if(ag)
	{
		isInitState = true;
		SoundAttGraph current = *ag;
		for(long i = 0; i < ARRSIZE(ag->c); i++)
		{
			c[i] = ag->c[i];
			float v = (c[i] - SoundAttGraph::cR[i].minValue)/(SoundAttGraph::cR[i].maxValue - SoundAttGraph::cR[i].minValue);
			p[i]->SetValue(v);
		}
		minDist = ag->minDist;
		editMinDist->UpdateText();
		maxDist = ag->maxDist;
		editMaxDist->UpdateText();
		OnUpdate(null);		
		if(memcmp(&current, ag, sizeof(SoundAttGraph)) != 0)
		{
			if(saveObject) saveObject->SetToSave();
		}
		isInitState = false;
	}
}

void _cdecl FormAttGraph::OnChangeGraph(GUIControl* sender)
{
	const UniqId * selUid = null;
	if(graphsList)
	{
		selUid = graphsList->GetCurrentItemUId();
	}
	if(selUid)
	{
		if(selUid->IsValidate())
		{
			if(!project->AttGraphGet(*selUid))
			{			
				//Ненашли при попытки установки глобальный пакаметр, значит переключаемся на локальные
				selUid = null;
			}
		}
	}
	if(selUid)
	{
		for(long i = 0; i < ARRSIZE(p); i++)
		{
			p[i]->Enabled = true;
		}
		ear->Enabled = true;
		editMinDist->Enabled = true;
		editMaxDist->Enabled = true;
		//Выбираем созданный
		if(currentSound)
		{
			currentSound->SetGlobalAttenuation(*selUid);
		}
		ReadCurrentParams();
	}else{
		for(long i = 0; i < ARRSIZE(p); i++)
		{
			p[i]->Enabled = false;
		}
		ear->Enabled = false;
		editMinDist->Enabled = false;
		editMaxDist->Enabled = false;
	}
	graphsList->SelectItemByUId(UniqId::zeroId);
}

void _cdecl FormAttGraph::OnAttGraphAdd(GUIControl* sender)
{
	if(!buttonAdd) return;
	//Создаём новый график
	GUIPoint pfen = buttonAdd->GetDrawRect().pos;
	pfen.x += buttonAdd->GetDrawRect().w;
	pfen.y += buttonAdd->GetDrawRect().h;
	FormEditName * fen = NEW FormEditName(this, pfen, FormEditName::pp_right_top);
	fen->Caption = options->GetString(SndOptions::s_snd_enter_new_att);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnCheckGraphName);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnAddNewGraph);
	fen->onCancel.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnRestoreSelect);
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormAttGraph::OnCheckGraphName(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	if(!project->AttGraphCheckUniqeName(name))
	{
		fen->MarkTextAsIncorrect();
	}
}

void _cdecl FormAttGraph::OnAddNewGraph(GUIControl* sender)
{
	if(!graphsList) return;
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	UniqId id;
	if(project->AttGraphAdd(name, &id))
	{
		if(currentSound) currentSound->SetGlobalAttenuation(id);
		graphsList->AddItem(null, name, -1, id);			
		graphsList->SelectItemByUId(id);
		ReadCurrentParams();
	}	
}

void _cdecl FormAttGraph::OnRestoreSelect(GUIControl* sender)
{
	if(graphsList)
	{
		if(currentSound)
		{
			graphsList->SelectItemByUId(currentSound->GetGlobalAttenuation());
		}else{
			graphsList->SelectItemById(-1);
		}
	}
	OnChangeGraph(this);
	ReadCurrentParams();
}

void _cdecl FormAttGraph::OnAttGraphRename(GUIControl* sender)
{
	if(!buttonRename || !currentSound) return;
	ProjectSoundAttGraph * patt = project->AttGraphGet(currentSound->GetGlobalAttenuation());
	if(!patt) return;
	GUIPoint pfen = buttonRename->GetDrawRect().pos;
	pfen.x += buttonRename->GetDrawRect().w;
	pfen.y += buttonRename->GetDrawRect().h;
	FormEditName * fen = NEW FormEditName(this, pfen, FormEditName::pp_right_top);
	fen->Caption = options->GetString(SndOptions::s_snd_rename_att);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnCheckGraphName);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnAttGraphRenameProcess);
	fen->onCancel.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnRestoreSelect);
	fen->SetText(patt->GetName().str);
	options->gui_manager->ShowModal(fen);
	
}

void _cdecl FormAttGraph::OnAttGraphRenameProcess(GUIControl* sender)
{
	if(!graphsList) return;
	FormEditName * fen = (FormEditName *)sender;
	const char * name = fen->GetText();
	const UniqId & id = currentSound->GetGlobalAttenuation();
	project->AttGraphRename(id, name);
	FillPresetsList();
	graphsList->SelectItemByUId(id);
}


void _cdecl FormAttGraph::OnAttGraphDelete(GUIControl* sender)
{
	if(!buttonDelete) return;
	GUIRectangle r = buttonDelete->GetDrawRect();
	r.y += r.h;
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
				options->GetString(SndOptions::s_snd_delete_attenuations), FormMessageBox::m_yesno, this, 
				FormMessageBox::pp_right_top, &r.pos);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormAttGraph::OnAttGraphDeleteProcess);
	options->gui_manager->ShowModal(mb);
}

void _cdecl FormAttGraph::OnAttGraphDeleteProcess(GUIControl* sender)
{
	if(!graphsList) return;
	const UniqId * curId = graphsList->GetCurrentItemUId();
	if(!curId || !curId->IsValidate()) return;
	project->AttGraphDelete(*curId);
	if(currentSound)
	{
		if(currentSound->GetGlobalAttenuation() == *curId)
		{
			currentSound->SetGlobalAttenuation(UniqId::zeroId);
		}
	}
	graphsList->SelectItem(-1);
	FillPresetsList();
	OnChangeGraph(null);
}

//Получить текущую структуру для модификации и текущий объект для сохранения
SoundAttGraph * FormAttGraph::GetCurrentObject(ProjectObject * & saveObject)
{
	SoundAttGraph * att = null;
	saveObject = null;
	if(currentSound)
	{
		const UniqId & globalAtt = currentSound->GetGlobalAttenuation();
		if(globalAtt.IsValidate())
		{
			ProjectSoundAttGraph * patt = project->AttGraphGet(globalAtt);
			if(patt)
			{
				att = &patt->attGraph;
				saveObject = patt;
			}
		}else{
			att = &currentSound->GetLocalAttenuation();
			saveObject = currentSound;
		}		
	}
	if(isInitState)
	{
		saveObject = null;
	}
	return att;
}

//Заполнить список пресэтов
void FormAttGraph::FillPresetsList()
{
	if(!graphsList) return;
	graphsList->Clear();
	graphsList->AddItem(null, " Select preset ", -1, UniqId::zeroId);
	const array<ProjectSoundAttGraph *> & patts = project->AttGraphArray();
	for(dword i = 0; i < patts.Size(); i++)
	{
		const ExtName & ename = patts[i]->GetName();
		graphsList->AddItem(null, ename.str, -1, patts[i]->GetId());
	}
}
