//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListAnimations
//============================================================================================

#include "FormListNodes.h"

FormLNodeElement::FormLNodeElement(FormListBase & _list, ProjectAnimation * pa, long n, string & tmp) : FormListElement(_list)
{
	pa->GetNodeName(n, tmp);	
	crt_strncpy(buffer, sizeof(buffer), tmp.c_str(), sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = 0;
	name.Init(buffer);
	const ProjectAnimation::PrjAnxNode & anxNode = pa->GetNode(n);
	notValidForUse = (anxNode.fps < 1.0f) || (anxNode.clipFrames < 2) || !anxNode.isValidate;
	node = n;
}

FormLNodeElement::~FormLNodeElement()
{
}

//Получить индекс нода
long FormLNodeElement::GetNode()
{
	if(notValidForUse)
	{
		return -1;
	}
	return node;
}

//Получить имя
const char * FormLNodeElement::GetName()
{
	return name.str;
}

//Нарисовать линию
void FormLNodeElement::Draw(const GUIRectangle & rect)
{
	//Фон выделения	и фокус
	DrawCursor(rect);
	//DrawSelect(rect);
	//Текст
	DrawText(rect, &name);
	if(notValidForUse)
	{
		dword color = (options->bkg & 0x00ffffff) | 0x80000000;
		GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, color);
	}
}

//Если параметры линия должна быть выше чем данная то вернуть true
bool FormLNodeElement::IsLessThen(const FormListElement * el) const
{
	FormLNodeElement & el2 = *(FormLNodeElement *)el;
	if(string::Compare(name.str, el2.name.str) > 0)
	{
		return false;
	}
	return true;
}

//Проверить на совпадение итема
bool FormLNodeElement::IsThis(const char * str, dword hash, dword len) const
{
	if(hash == name.hash && len == name.len)
	{
		if(string::IsEqual(str, name.str))
		{
			return true;
		}
	}
	return false;
}

//Проверить попадание по префиксу
bool FormLNodeElement::IsPreficsEqual(const char * pref) const
{
	return string::EqualPrefix(name.str, pref);
}

//Активировать элемент
void FormLNodeElement::OnAction()
{	
	((FormListNodes &)List()).DoAddNewNode(null);
}


FormListNodes::FormListNodes(FormListAnimationsOptions & opt, GUIControl * parent, const GUIRectangle & rect) 
: FormListBase(parent, rect),
formOptions(opt)
{
	buttonNew = ButtonAdd();
	buttonNew->Hint = options->GetString(SndOptions::s_hint_ani_add_node);
	buttonNew->text.SetString("+");
	buttonNew->onUp.SetHandler(this, (CONTROL_EVENT)&FormListNodes::DoAddNewNode);
	isSaveFocus = false;
	InitList();
}

FormListNodes::~FormListNodes()
{
	
}

//Получить анимацию в фокусе
long FormListNodes::GetFocusNode()
{
	FormLNodeElement * line = (FormLNodeElement *)GetLine(GetFocus());
	if(!line) return -1;
	return line->GetNode();
}

//Разрешить - запретить
void FormListNodes::SetEnabled(bool isEnable)
{
	Enabled = isEnable;
	buttonNew->Enabled = isEnable;
}

//Инициализировать лист
void FormListNodes::DoInitList()
{
	//Анимация с которой работаем
	isSaveFocus = false;
	ProjectAnimation * pa = project->AnimationGet(formOptions.currentAnimation);
	if(pa)
	{
		if(formOptions.animationForNodes != pa->GetName().str)
		{
			formOptions.lastNode.Empty();
			formOptions.animationForNodes = pa->GetName().str;
		}
		SetCaption(pa->GetName().str, &options->imageAnimation);
		dword count = pa->GetNodesCount();
		string tmp;
		for(dword i = 0; i < count; i++)
		{
			AddElement(NEW FormLNodeElement(*this, pa, i, tmp));
		}
	}
}

//Событие изменения фокуса
void FormListNodes::OnChangeFocus()
{
	FormListBase::OnChangeFocus();
	FormLNodeElement * line = (FormLNodeElement *)GetLine(GetFocus());
	if(line)
	{
		if(buttonNew)
		{
			buttonNew->Enabled = (line->GetNode() >= 0);
		}			
		if(isSaveFocus)
		{
			formOptions.lastNode = line->GetName();
		}			
	}
}

//Лист был обновлён
void FormListNodes::OnListUpdated()
{
	if(formOptions.lastNode.Len() > 0)
	{		
		SetFocus(formOptions.lastNode.c_str());
	}
	isSaveFocus = true;
}

void _cdecl FormListNodes::DoAddNewNode(GUIControl* sender)
{
	onAddNode.Execute(this);
}

