//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListAnimations
//============================================================================================

#include "FormListAnimations.h"
#include "..\FormNewAnimation.h"
#include "..\FormEditName.h"
#include "..\FormMessageBox.h"

FormLAniElement::FormLAniElement(FormListBase & _list, ProjectAnimation * pa, long m) : FormListElement(_list)
{
	stage = null;
	mst = movie_stage_first - 1;
	if(pa)
	{		
		animationId = pa->GetId();
		movie = m;
		if(movie >= 0)
		{			
			crt_strncpy(buffer, sizeof(buffer), pa->GetMovieName(movie).str, sizeof(buffer) - 1);
			image = &options->imageAniMovie;
			mst = pa->GetMovieStage(movie);
			SetStageImage(mst);
		}else{
			crt_strncpy(buffer, sizeof(buffer), pa->GetName().str, sizeof(buffer) - 1);
			image = &options->imageAnimation;
			SetStageImage(pa->GetAnimationStage());
		}
	}else{
		crt_strncpy(buffer, sizeof(buffer), options->GetString(SndOptions::s_folder_up), sizeof(buffer) - 1);
		image = &options->imageUp;		
		movie = -1;
		animationId.Reset();
	}
	buffer[sizeof(buffer) - 1] = 0;
	name.Init(buffer);
}

FormLAniElement::~FormLAniElement()
{

}

//Получить анимацию
const UniqId & FormLAniElement::GetAnimation()
{
	return animationId;
}

//Получить индекс ролика
long FormLAniElement::GetMovie()
{
	return movie;
}

//Изменить стадию ролика
void FormLAniElement::DoChangeMovieStage()
{
	Assert(movie >= 0);
	ProjectAnimation * pa = project->AnimationGet(animationId);
	Assert(pa);
	mst = pa->GetMovieStage(movie);
	if(options->policy == policy_designer)
	{
		switch(mst)
		{
		case movie_stage_movie:
			pa->SetMovieStage(movie, movie_stage_events);
			break;
		case movie_stage_events:
			pa->SetMovieStage(movie, movie_stage_approval);
			break;
		case movie_stage_approval:
			pa->SetMovieStage(movie, movie_stage_done);
			break;
		case movie_stage_done:
			pa->SetMovieStage(movie, movie_stage_movie);
			break;
		case movie_stage_check:
			pa->SetMovieStage(movie, movie_stage_approval);
			break;
		case movie_stage_damage:
			pa->SetMovieStage(movie, movie_stage_movie);
			break;
		default:
			Assert(false);
		}
	}else{
		if(mst == movie_stage_events)
		{
			pa->SetMovieStage(movie, movie_stage_approval);
		}else
		if(mst == movie_stage_approval)
		{
			pa->SetMovieStage(movie, movie_stage_events);
		}else{
			Assert(false);
		}
	}
	UpdateMovieStage();
	SetFocus();
}

//Обновить состояние стадии ролика
void FormLAniElement::UpdateMovieStage()
{
	if(movie >= 0)
	{
		ProjectAnimation * pa = project->AnimationGet(animationId);
		Assert(pa);
		mst = pa->GetMovieStage(movie);
		SetStageImage(mst);
	}
}

//Нарисовать линию
void FormLAniElement::Draw(const GUIRectangle & rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	//DrawSelect(rect);
	//Иконка объекта
	DrawIcon(rect, image);
	//Текст
	DrawText(rect, &name);
	//Рисуем картинку стадии ролика или анимации
	if(stage)
	{
		long x = rect.x + rect.w - c_buttonsarea;
		long y = rect.y + (rect.h - c_image_h)/2;
		dword color = 0xffffffff;
		GUIHelper::DrawSprite(x, y, c_image_w, c_image_h, stage);
		if(options->policy != policy_designer)
		{
			if(mst == movie_stage_events || mst == movie_stage_approval)
			{
				GUIHelper::DrawWireRect(x - 2, y - 2, c_image_w + 4, c_image_h + 4, (options->black2Bkg[0] & 0x00ffffff) | 0x80000000);
			}else{
				//GUIHelper::Draw2DRect(x, y, c_image_w, c_image_h, (options->bkg & 0x00ffffff) | 0x80000000);				
			}
		}
	}
	if(movie < 0 && animationId.IsValidate())
	{
		DrawSelect(rect);
	}
	//GUIHelper::DrawWireRect(rect.x, rect.y, rect.w, rect.h, options->black2Bkg[0]);
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormLAniElement::IsLessThen(const FormListElement * el) const
{
	FormLAniElement & el2 = *(FormLAniElement *)el;
	if(!animationId.IsValidate())
	{
		return true;
	}
	//Анимации
	if(movie < 0)
	{
		if(!el2.animationId.IsValidate())
		{
			return true;
		}
		const char * name1 = name.str;
		const char * name2 = el2.name.str;
		if(string::Compare(name1, name2) > 0)
		{
			return false;
		}
		return true;
	}
	if(el2.movie < 0)
	{
		return false;
	}
	//Ролики
	const char * name1 = name.str;
	const char * name2 = el2.name.str;
	if(string::Compare(name1, name2) > 0)
	{
		return false;
	}
	return true;		
}

//Проверить на совпадение итема
bool FormLAniElement::IsThis(const char * str, dword hash, dword len) const
{
	if(!animationId.IsValidate())
	{
		return false;
	}
	if(name.hash == hash && name.len == len)
	{
		if(string::IsEqual(name.str, str))
		{
			return true;
		}		
	}
	return false;
}

//Проверить попадание по префиксу
bool FormLAniElement::IsPreficsEqual(const char * pref) const
{
	if(!animationId.IsValidate())
	{
		return false;
	}
	return string::EqualPrefix(name.str, pref);
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormLAniElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	//Проверим на попадание по кнопкам
	if(movie >= 0)
	{
		long bx = rect.x + rect.w - c_buttonsarea;
		if(pos.x >= bx && pos.x <= bx + c_image_w)
		{
			bool canChange = false;
			if(options->policy != policy_designer)
			{
				ProjectAnimation * pa = project->AnimationGet(animationId);
				Assert(pa);
				dword mst = pa->GetMovieStage(movie);
				if(mst == movie_stage_events || mst == movie_stage_approval)
				{
					canChange = true;
				}
			}else{
				canChange = true;
			}
			if(canChange)
			{
				SetFocus();
				FormListAnimations & list =(FormListAnimations &)List();
				list.DoChangeMovieStageAccept(bx + c_buttonsarea/2, rect.y + rect.h/2);
				return false;
			}
		}
	}
	return true;
}

//Эвенты
void FormLAniElement::Event(const FormListEventData & data)
{
}

//Активировать элемент
void FormLAniElement::OnAction()
{	
	if(movie < 0)
	{
		if(animationId.IsValidate())
		{
			Enter();			
		}else{
			UpByHerarchy();
		}
	}
}

//На линию был установлен фокус
void FormLAniElement::OnSetFocus()
{

}


//Установить картинку для состояния
void FormLAniElement::SetStageImage(dword mst)
{
	switch(mst)
	{
	case movie_stage_movie:
		stage = &options->imageStageMovie;
		break;
	case movie_stage_events:
		stage = &options->imageStageEvents;
		break;
	case movie_stage_approval:
		stage = &options->imageStageApproval;
		break;
	case movie_stage_done:
		stage = &options->imageStageDone;
		break;
	case movie_stage_check:
		stage = &options->imageStageCheck;
		break;
	case movie_stage_damage:
		stage = &options->imageStageDamage;
		break;
	default:
		Assert(false);
	}
}


FormListAnimations::FormListAnimations(FormListAnimationsOptions & opt, GUIControl * parent, const GUIRectangle & rect) 
: FormListBase(parent, rect),
formOptions(opt)
{
	buttonNew = null;
	buttonCopy = null;
	buttonRename = null;
	buttonDel = null;
	buttonExport = null;	
	makeCopyFrom = -1;
	InitList();
}

FormListAnimations::~FormListAnimations()
{
	
}

//Получить анимацию в фокусе
const UniqId & FormListAnimations::GetFocusAnimation()
{
	FormLAniElement * line = (FormLAniElement *)GetLine(GetFocus());
	if(!line) return UniqId::zeroId;
	return line->GetAnimation();
}

//Получить анимацию в фокусе
long FormListAnimations::GetFocusMovie()
{
	FormLAniElement * line = (FormLAniElement *)GetLine(GetFocus());
	if(!line) return -1;
	return line->GetMovie();
}

//Обновить состояние стадии роликов
void FormListAnimations::UpdateMovieStages()
{
	long linesCount = GetNumLines();
	for(long i = 0; i < linesCount; i++)
	{
		FormLAniElement * line = (FormLAniElement *)GetLine(i);
		if(line) line->UpdateMovieStage();
	}
}

//Инициализировать лист
void FormListAnimations::DoInitList()
{
	ButtonsClear();
	if(buttonNew)
	{
		delete buttonNew; 
		buttonNew = null; 
	}
	if(buttonRename)
	{ 
		delete buttonRename; 
		buttonRename = null; 
	}
	if(buttonDel)
	{ 
		delete buttonDel; 
		buttonDel = null; 
	}
	if(buttonExport)
	{
		delete buttonExport;
		buttonExport = null;
	}
	//Анимация с которой работаем
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	if(pa)
	{
		if(options->policy == policy_designer)
		{
			buttonExport = ButtonAdd();
			buttonExport->Hint = options->GetString(SndOptions::s_hint_ani_expc_ani);
			buttonExport->text.SetString("E");
			buttonExport->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoExpAnimation);
			buttonDel = ButtonAdd();
			buttonDel->Hint = options->GetString(SndOptions::s_hint_ani_del_movie);
			buttonDel->text.SetString("D");		
			buttonDel->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoDelMovie);
			buttonRename = ButtonAdd();
			buttonRename->Hint = options->GetString(SndOptions::s_hint_ani_rename);
			buttonRename->text.SetString("R");
			buttonRename->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoRename);
			buttonMission = ButtonAdd();
			buttonMission->Hint = options->GetString(SndOptions::s_hint_ani_set_mis);
			buttonMission->text.SetString("M");
			buttonMission->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoSetMission);
			buttonCopy = ButtonAdd();
			buttonCopy->Hint = options->GetString(SndOptions::s_hint_ani_add_movie);
			buttonCopy->text.SetString("C");
			buttonCopy->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoCopyMovie);
			buttonNew = ButtonAdd();
			buttonNew->Hint = options->GetString(SndOptions::s_hint_ani_add_movie);
			buttonNew->text.SetString("N");
			buttonNew->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoAddNewMovie);
		}
		SetCaption(pa->GetName().str, &options->imageAnimation);
		AddElement(NEW FormLAniElement(*this, null, -1));
		dword count = pa->GetMovieCount();
		for(dword i = 0; i < count; i++)
		{
			AddElement(NEW FormLAniElement(*this, pa, i));
		}
	}else{
		if(options->policy == policy_designer)
		{
			buttonExport = ButtonAdd();
			buttonExport->Hint = options->GetString(SndOptions::s_hint_ani_exps_ani);
			buttonExport->text.SetString("E");
			buttonExport->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoExpAnimation);
			buttonDel = ButtonAdd();
			buttonDel->Hint = options->GetString(SndOptions::s_hint_ani_del_ani);
			buttonDel->text.SetString("D");
			buttonDel->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoDelAnimation);
			buttonNew = ButtonAdd();
			buttonNew->Hint = options->GetString(SndOptions::s_hint_ani_add_ani);
			buttonNew->text.SetString("N");
			buttonNew->onUp.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoAddNewAnimation);
		}
		SetCaption(options->GetString(SndOptions::s_ani_animations), &options->imageAnimations);
		const array<ProjectAnimation *> & anis = project->AnimationArray();
		for(dword i = 0; i < anis.Size(); i++)
		{
			AddElement(NEW FormLAniElement(*this, anis[i], -1));
		}
	}
}

//Подняться по иерархии
void FormListAnimations::DoUpByHerarchy()
{
	formOptions.currentAnimation.Reset();
	DoInitList();
}

//Опуститься по иерархии ниже
void FormListAnimations::DoEnterTo(FormListElement * element)
{	
	FormLAniElement * ae = (FormLAniElement *)element;
	if(ae->GetMovie() >= 0)
	{
		formOptions.currentAnimation.Reset();
	}else{
		formOptions.currentAnimation = ae->GetAnimation();
	}	
	DoInitList();
}

//Событие изменения фокуса
void FormListAnimations::OnChangeFocus()
{
	FormListBase::OnChangeFocus();
	if(buttonRename)
	{
		buttonRename->Enabled = true;
		buttonMission->Enabled = true;
		buttonCopy->Enabled = true;
	}	
	const UniqId & aniId = GetFocusAnimation();
	if(aniId.IsValidate())
	{
		if(GetFocusMovie() < 0)
		{
			ProjectAnimation * pa = project->AnimationGet(aniId);
			if(pa)
			{
				formOptions.setCursorOnItem = pa->GetAnimationName();
			}
		}
	}else{
		if(GetFocusMovie() < 0 && buttonRename)
		{
			buttonRename->Enabled = false;
			buttonMission->Enabled = false;
			buttonCopy->Enabled = false;
		}
	}
}

//Лист был обновлён
void FormListAnimations::OnListUpdated()
{
	if(formOptions.setCursorOnItem.Len() > 0)
	{		
		SetFocus(formOptions.setCursorOnItem.c_str());
	}
}

//Подтвердить изменение стадии ролика
void FormListAnimations::DoChangeMovieStageAccept(long x, long y)
{
	GUIPoint p;
	p.x = x;
	p.y = y;
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
				options->GetString(SndOptions::s_ani_change_stage), FormMessageBox::m_yesno, null, 
				p.y < (long)options->screenHeight/2 ? FormMessageBox::pp_left_top : FormMessageBox::pp_left_bottom, &p);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoChangeMovieStageProcess);
	options->gui_manager->ShowModal(mb);	

}

void _cdecl FormListAnimations::DoChangeMovieStageProcess(GUIControl* sender)
{
	FormLAniElement * line = (FormLAniElement *)GetLine(GetFocus());
	if(line)
	{
		line->DoChangeMovieStage();
		onChangeFocus.Execute(this);
	}
}


void _cdecl FormListAnimations::DoAddNewAnimation(GUIControl* sender)
{
	GUIRectangle r = buttonNew->GetClientRect();
	GUIPoint pos = r.pos;
	pos.y += r.h;
	FormNewAnimation * na = NEW FormNewAnimation(this, pos);
	na->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoAddNewAnimationProcess);
	options->gui_manager->ShowModal(na);
}

void _cdecl FormListAnimations::DoAddNewAnimationProcess(GUIControl* sender)
{
	FormNewAnimation * na = (FormNewAnimation *)sender;
	InitList();
	SetFocus(na->GetAnimationName());
}

void _cdecl FormListAnimations::DoDelAnimation(GUIControl* sender)
{
	ProjectAnimation * pa = project->AnimationGet(GetFocusAnimation());
	Assert(pa);
	string msg = options->GetString(SndOptions::s_ani_del_animation);
	msg += pa->GetName().str;
	Assert(buttonDel);
	GUIRectangle r = buttonDel->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
		msg.c_str(), FormMessageBox::m_yesno, null, 
		FormMessageBox::pp_left_top, &p);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoDelAnimationProcess);
	options->gui_manager->ShowModal(mb);	
}


void _cdecl FormListAnimations::DoDelAnimationProcess(GUIControl* sender)
{
	const UniqId & focusAnimation = GetFocusAnimation();
	if(focusAnimation.IsValidate()) return;
	long focusIndex = GetFocus() - 1;
	if(focusIndex < 0) focusIndex = 0;
	project->AnimationDelete(focusAnimation);
	formOptions.currentAnimation.Reset();
	InitList();
	SetFocus(focusIndex);
}

void _cdecl FormListAnimations::DoExpAnimation(GUIControl* sender)
{
	bool isIgnoreStages = (GetAsyncKeyState(VK_SHIFT) < 0);
	bool isOk = false;
	string path;
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	if(pa)
	{
		isOk = pa->ExportEventsToIni(path, isIgnoreStages);
	}else{		
		long linesCount = GetNumLines();
		isOk = true;
		string tmp;
		for(long i = 0; i < linesCount; i++)
		{
			FormLAniElement * line = (FormLAniElement *)GetLine(i);
			if(line->IsSelect() || line->IsFocus())
			{
				ProjectAnimation * pa = project->AnimationGet(line->GetAnimation());
				if(pa)
				{
					if(pa->ExportEventsToIni(tmp, isIgnoreStages))
					{
						path += tmp;
					}else{
						isOk = false;
						path += "Error: ";
						path += tmp;
					}
					path += "\n";
				}
			}
		}
		if(path.IsEmpty())
		{
			path = options->GetString(SndOptions::s_ani_er_noselect);
		}
	}
	const char * caption = "";
	if(isOk)
	{
		caption = options->GetString(SndOptions::s_ani_exp_to);
	}else{
		caption = options->GetString(SndOptions::s_ani_er_evt_exp);
	}
	GUIRectangle r = buttonExport->GetDrawRect();
	buttonExport->ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormMessageBox * mb = NEW FormMessageBox(caption, path.c_str(), FormMessageBox::m_ok, null, FormMessageBox::pp_left_top, &p);
	if(isOk)
	{
		mb->headerColor = options->bkg | 0xff0000f0;
	}
	options->gui_manager->ShowModal(mb);
}


void _cdecl FormListAnimations::DoAddNewMovie(GUIControl* sender)
{
	makeCopyFrom = -1;
	Assert(buttonNew);
	GUIRectangle r = buttonNew->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);
	fen->Caption = options->GetString(SndOptions::s_ani_add_new_movie);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoAddNewMovie_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoAddNewMovie_Ok);
	fen->MarkTextAsIncorrect();
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormListAnimations::DoAddNewMovie_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	Assert(fen);	
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	if(!pa->CheckMovieName(fen->GetText(), -1))
	{
		fen->MarkTextAsIncorrect();
	}
}

void _cdecl FormListAnimations::DoAddNewMovie_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	Assert(fen);
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	string movieName = fen->GetText();
	pa->AddMovie(movieName.c_str(), makeCopyFrom);
	makeCopyFrom = -1;
	InitList();
	SetFocus(movieName.c_str());
}

void _cdecl FormListAnimations::DoCopyMovie(GUIControl* sender)
{
	long focusMovie = GetFocusMovie();
	if(focusMovie >= 0)
	{
		DoAddNewMovie(sender);
		makeCopyFrom = GetFocusMovie();
	}
}

void _cdecl FormListAnimations::DoSetMission(GUIControl* sender)
{
	if(GetFocusMovie() < 0)
	{
		return;
	}
	Assert(buttonMission);
	GUIRectangle r = buttonMission->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);
	fen->Caption = options->GetString(SndOptions::s_ani_set_mission);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoSetMission_Ok);
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	long movieIndex = GetFocusMovie();
	Assert(movieIndex >= 0);
	Assert(buttonMission);
	fen->SetText(pa->GetMissionName(movieIndex));
	fen->MarkTextAsIncorrect();
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormListAnimations::DoSetMission_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	Assert(fen);
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	long movieIndex = GetFocusMovie();
	Assert(movieIndex >= 0);
	pa->SetMission(movieIndex, fen->GetText());
	onChangeFocus.Execute(this);
}


void _cdecl FormListAnimations::DoRename(GUIControl* sender)
{
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	long movieIndex = GetFocusMovie();
	Assert(movieIndex >= 0);
	Assert(buttonRename);
	GUIRectangle r = buttonRename->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormEditName * fen = NEW FormEditName(this, p, FormEditName::pp_left_top);
	fen->Caption = options->GetString(SndOptions::s_ani_add_new_movie);
	fen->onCheck.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoRename_Check);
	fen->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoRename_Ok);
	fen->SetText(pa->GetMovieName(movieIndex).str);
	fen->MarkTextAsIncorrect();
	options->gui_manager->ShowModal(fen);
}

void _cdecl FormListAnimations::DoRename_Check(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	Assert(fen);	
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	long movieIndex = GetFocusMovie();
	Assert(movieIndex >= 0);
	if(!pa->CheckMovieName(fen->GetText(), movieIndex))
	{
		fen->MarkTextAsIncorrect();
	}
}

void _cdecl FormListAnimations::DoRename_Ok(GUIControl* sender)
{
	FormEditName * fen = (FormEditName *)sender;
	Assert(fen);
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	long movieIndex = GetFocusMovie();
	Assert(movieIndex >= 0);
	string movieName = fen->GetText();
	pa->RenameMovie(movieIndex, movieName.c_str());
	InitList();
	SetFocus(movieName.c_str());
}

void _cdecl FormListAnimations::DoDelMovie(GUIControl* sender)
{
	long mi = GetFocusMovie();
	if(mi < 0) return;
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	string msg = options->GetString(SndOptions::s_ani_del_movie);
	msg += pa->GetMovieName(mi).str;
	Assert(buttonDel);
	GUIRectangle r = buttonDel->GetDrawRect();
	ClientToScreen(r);
	GUIPoint p = r.pos;
	p.y += r.h + 3;
	FormMessageBox * mb = NEW FormMessageBox(options->GetString(SndOptions::s_form_attention_op),
				msg.c_str(), FormMessageBox::m_yesno, null, 
				FormMessageBox::pp_left_top, &p);
	mb->onOk.SetHandler(this, (CONTROL_EVENT)&FormListAnimations::DoDelMovie_Ok);
	options->gui_manager->ShowModal(mb);
}

void _cdecl FormListAnimations::DoDelMovie_Ok(GUIControl* sender)
{
	long focusIndex = GetFocus();
	long mi = GetFocusMovie();
	Assert(mi >= 0);
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	Assert(pa);
	pa->DeleteMovie(mi);
	InitList();
	long numLines = GetNumLines();
	if(focusIndex >= numLines)
	{
		focusIndex = numLines - 1;
		if(focusIndex < 0) focusIndex = 0;
	}
	SetFocus(focusIndex);
}


