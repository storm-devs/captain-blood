
#include "FormGameList.h"


FormGameListElement::FormGameListElement(FormListBase & owner, ProjectSound * sound, State state) : FormListElement(owner)
{
	Assert(sound);
	id = sound->GetId();
	name.SetString(sound->GetName().str);
	SetState(state);	
}

//Получить идентификатор звука
const UniqId & FormGameListElement::GetId()
{
	return id;
}

//Установить состояние
void FormGameListElement::SetState(State state)
{
	color = options->colorTextLo;
	image = &options->imageSound;
	if(state != s_ok)
	{
		color = (color & 0x00ffffff) | 0x50000000;
		switch(state)
		{
		case s_cantexport:			
			image = &options->imageExportError;
			break;
		case s_notinbank:
			image = &options->imageStageDamage;
			break;
		}
	}
}

//Нарисовать линию
void FormGameListElement::Draw(const GUIRectangle &rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	//Иконка объекта
	DrawIcon(rect, image);
	//Текст
	DrawText(rect, &name, true, color);
}

//Если линия должна быть выше чем в параметре то вернуть true
bool FormGameListElement::IsLessThen(const FormListElement *el) const
{
	return string::Compare(name.str, ((FormGameListElement *)el)->name.str) <= 0;
}

//Проверить на совпадение итема
bool FormGameListElement::IsThis(const char *str, dword hash, dword len) const
{
	if(name.hash == hash && name.len == len)
	{
		if(string::IsEqual(str, name.str))
		{
			return true;
		}
	}
	return false;
}

//Проверить попадание по префиксу
bool FormGameListElement::IsPreficsEqual(const char *pref) const
{
	return string::EqualPrefix(name.str, pref);
}


FormGameList::FormGameList(GUIControl * parent, GUIRectangle & rect) : FormListBase(parent, GUIRectangle(rect), false)
{
	updatePointer = 0;
	SetCaption("Sounds:", null);
}

FormGameList::~FormGameList()
{
}

//Получить текущий редактируемый звук
const UniqId & FormGameList::GetFocusElement()
{
	FormGameListElement * le = (FormGameListElement *)GetLine(GetFocus());
	if(!le) return UniqId::zeroId;
	return le->GetId();
}

//Инициализировать лист
void FormGameList::DoInitList()
{
	updatePointer = 0;
	const array<ProjectSound *> & sounds = project->SoundArray();
	for(dword i = 0 ; i < sounds.Size(); i++)
	{
		if(sounds[i])
		{
			FormGameListElement::State state = UpdateSound(sounds[i], true);
			AddElement(NEW FormGameListElement(*this, sounds[i], state));			
		}
	}
}

//Рисование
void FormGameList::Draw()
{
	//Рисуем список
	FormListBase::Draw();
	//Массив текущих звуков
	const array<ProjectSound *> & sounds = project->SoundArray();
	//Количество, которое обновим за этот кадр	
	float deltaTime = api->GetNoScaleDeltaTime();
	if(deltaTime > 0.1f) deltaTime = 0.1f;
	dword count = (dword)(deltaTime*sounds.Size());
	if(count > sounds.Size()) count = sounds.Size();	
	//Обновляем группу звуков
	for(dword i = 0; i < count; i++, updatePointer++)
	{
		if(updatePointer >= sounds.Size())
		{
			updatePointer = 0;
		}
		FormGameListElement::State res = UpdateSound(sounds[updatePointer], false);
		if(res != FormGameListElement::s_ok)
		{
			//По какой то причине сломалась выгрузка
			ChangeState(sounds[updatePointer]->GetId(), res);
		}
	}
	//Обновляем звук в фокусе
	FormGameListElement * le = (FormGameListElement *)GetLine(GetFocus());
	if(le)
	{
		ProjectSound * snd = project->SoundGet(le->GetId());
		if(snd)
		{
			FormGameListElement::State res = UpdateSound(snd, true);
			if(res != FormGameListElement::s_ok)
			{
				//По какой то причине сломалась выгрузка
				ChangeState(snd->GetId(), res);
			}
		}
	}
}

//Обновить звук
FormGameListElement::State FormGameList::UpdateSound(ProjectSound * sound, bool isCheckIds)
{
	Assert(sound);
	static EditPatchSoundData psd;
	FormGameListElement::State state = FormGameListElement::s_ok;
	if(sound->GetPatchSoundData(psd))
	{
		if(!options->sa->EditPatchSound(psd, isCheckIds))
		{
			state = FormGameListElement::s_notinbank;
		}
	}else{
		state = FormGameListElement::s_cantexport;
	}
	return state;
}

//Изменить состояние звука
void FormGameList::ChangeState(const UniqId & id, FormGameListElement::State state)
{
	long count = GetNumLines();
	for(long j = 0; j < count; j++)
	{
		FormGameListElement * line = (FormGameListElement *)GetLine(j);
		if(line && line->GetId() == id)
		{
			line->SetState(state);
			break;	
		}
	}
}