


#include "FormPolygonList.h"
#include "..\FormEditName.h"


FormPolygonElement::FormPolygonElement(FormListBase & _list, const char * _name, ElementType _type, void * _ptr) : FormListElement(_list)
{
	type = _type;
	ptr = _ptr;
	name.SetString(_name);
	if(type == et_sound)
	{
		//Ищем индекс звука в проекте и экспортим его
		const array<ProjectSound *> & sounds = project->SoundArray();
		for(dword i = 0; i < sounds.Size(); i++)
		{
			ProjectSound * sound = sounds[i];
			if(!sound) continue;
			const ExtName & sname = sound->GetName();
			if(sname.hash == name.hash && sname.len == name.len)
			{
				if(string::IsEqual(sname.str, name.str))
				{				
					if(sound->MakePreview())
					{
						soundId = sound->GetId();
					}
					break;
				}
			}
		}
	}
}

FormPolygonElement::~FormPolygonElement()
{
	ProjectSound * sound = project->SoundGet(soundId);
	if(sound)
	{
		sound->StopPreview();
	}
}

//Нарисовать линию
void FormPolygonElement::Draw(const GUIRectangle & rect)
{
	DrawCursor(rect);
	if(type == et_sound)
	{
		DrawIcon(rect, &options->imageSound);
	}else
	if(type == et_geometry)
	{
		DrawIcon(rect, &options->imageModel);
	}	
	DrawText(rect, &name, true);
	DrawSelect(rect);
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormPolygonElement::IsLessThen(const FormListElement * el) const
{
	if(((FormPolygonElement *)el)->type != type)
	{
		return (type == et_sound);
	}
	const char * str1 = (*(FormPolygonElement *)el).name.data.c_str();
	const char * str2 = name.data.c_str();
	return string::Compare(str1, str2) > 0;
}

//Проверить на совпадение итема
bool FormPolygonElement::IsThis(const char * str, dword hash, dword len) const
{
	if(hash == name.hash && len == name.len && string::IsEqual(str, name.str))
	{
		return true;
	}
	return false;
}

//Проверить попадание по префиксу
bool FormPolygonElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);
}

FormPolygonList::FormPolygonList(GUIControl * parent, GUIRectangle rect) : FormListBase(parent, rect, true), elements(_FL_, 64)
{
	SetFrame(true);
	SetCaption(options->GetString(SndOptions::s_poly_list_caption), null);
	buttonDelete = ButtonAdd();	
	buttonDelete->image = &options->imageDelete;
	buttonDelete->Hint = options->GetString(SndOptions::s_hint_poly_delete);
	buttonDelete->onUp.SetHandlerEx(this, &FormPolygonList::OnDeleteItems);
	buttonAddModel = ButtonAdd();
	buttonAddModel->image = &options->imageModel;
	buttonAddModel->Hint = options->GetString(SndOptions::s_hint_poly_add_model);
	buttonAddModel->onUp.SetHandlerEx(this, &FormPolygonList::OnAddModel);
	buttonAddSound = ButtonAdd();
	buttonAddSound->image = &options->imageSound;
	buttonAddSound->Hint = options->GetString(SndOptions::s_hint_poly_add_sound);
	buttonAddSound->onUp.SetHandlerEx(this, &FormPolygonList::OnAddSound);
}

FormPolygonList::~FormPolygonList()
{
	for(dword i = 0; i < elements.Size(); i++)
	{
		delete elements[i];
	}
}

//Добавить элемент
void FormPolygonList::Add(const char * name, FormPolygonElement::ElementType type, void * ptr)
{	
	FormPolygonElement * el = NEW FormPolygonElement(*this, name, type, ptr);
	elements.Add(el);
}

//Удалить элемент
void FormPolygonList::Del(dword index)
{
	delete elements[index];
	elements.DelIndex(index);
}

void FormPolygonList::DoInitList()
{
	for(dword i = 0; i < elements.Size(); i++)
	{
		AddElement(elements[i]);
	}
}

//Получить количество элементов
dword FormPolygonList::GetElementsCount()
{
	return elements.Size();
}

//Получить указатель элемента
void * FormPolygonList::GetElementPtr(dword index)
{
	return elements[index]->ptr;
}

//Получить тип элемента
FormPolygonElement::ElementType FormPolygonList::GetElementType(dword index)
{
	return elements[index]->type;
}

//Выделен ли итем
bool FormPolygonList::GetElementSelect(dword index)
{
	if(elements[index]->IsSelect() || elements[index]->IsFocus())
	{
		return true;
	}
	return false;
}

//Элемент с фокусом
long FormPolygonList::GetFocusIndex()
{
	for(dword i = 0; i < elements.Size(); i++)
	{
		if(elements[i]->IsFocus()) return i;
	}
	return -1;
}

//Установить фокус
void FormPolygonList::SetFocusIndex(dword index)
{
	elements[index]->SetFocus();
}

//Получить имя добавляемого звука
const char * FormPolygonList::GetAddSoundName()
{
	return creationSoundName.c_str();
}

//Получить путь добавляемой модельки
const char * FormPolygonList::GetAddModelPath()
{
	return creationModelPath.c_str();
}

void _cdecl FormPolygonList::OnAddSound(GUIControl * sender)
{
	GUIPoint p = buttonAddSound->GetClientRect().pos;
	FormEditName * en = NEW FormEditName(this, p, FormEditName::pp_right_top);
	en->onOk.SetHandlerEx(this, &FormPolygonList::OnAddSoundOk);
	en->onCheck.SetHandlerEx(this, &FormPolygonList::OnCheckNameForEmpty);
	en->Caption = "Enter sound name for add to scene:";
	options->gui_manager->ShowModal(en);
}

void _cdecl FormPolygonList::OnAddSoundOk(GUIControl * sender)
{
	FormEditName * en = (FormEditName *)sender;
	creationSoundName = en->GetText();
	onCreateSound.Execute(this);
}

void _cdecl FormPolygonList::OnAddModel(GUIControl * sender)
{
	GUIPoint p = buttonAddModel->GetClientRect().pos;
	FormEditName * en = NEW FormEditName(this, p, FormEditName::pp_right_top);
	en->onOk.SetHandlerEx(this, &FormPolygonList::OnAddModelOk);
	en->onCheck.SetHandlerEx(this, &FormPolygonList::OnCheckNameForEmpty);
	en->Caption = "Enter model path for add to scene:";
	options->gui_manager->ShowModal(en);
}

void _cdecl FormPolygonList::OnAddModelOk(GUIControl * sender)
{
	FormEditName * en = (FormEditName *)sender;
	creationModelPath = en->GetText();		
	onCreateModel.Execute(this);
}

void _cdecl FormPolygonList::OnDeleteItems(GUIControl * sender)
{
	onDeleteItems.Execute(this);
}

void _cdecl FormPolygonList::OnCheckNameForEmpty(GUIControl * sender)
{
	FormEditName * en = (FormEditName *)sender;
	const char * str = en->GetText();
	if(string::IsEmpty(str))
	{
		en->MarkTextAsIncorrect();
	}
}
