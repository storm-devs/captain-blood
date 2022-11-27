//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormListBase
//============================================================================================

#include "FormListBase.h"
#include "..\sliders\FormScrollBar.h"


FormListElement::FormListElement(FormListBase & _list) : list(_list)
{
	isSelect = false;
	index = -1;
}

FormListElement::~FormListElement()
{
}

//Установить на линию фокус
void FormListElement::SetFocus()
{
	if(list.focusPosition >= 0 && list.focusPosition < list.elements)
	{
		list.elements[list.focusPosition]->OnLostFocus();
	}
	list.focusPosition = index;
	OnSetFocus();
	list.SystemCheckFocus();
}

//Если на данном щелчке можно проводить операции селекта, вернуть true
bool FormListElement::MouseClick(const GUIRectangle & rect, const GUIPoint & pos)
{
	return true;
}

//Эвенты
void FormListElement::Event(const FormListEventData & data)
{
}

//Над линией едет мышиный курсор
void FormListElement::OnMouseMove(const GUIRectangle & rect, const GUIPoint & pos)
{
}

//На линию был установлен фокус
void FormListElement::OnSetFocus()
{
}

//С линии убрали фокус
void FormListElement::OnLostFocus()
{
}

//Линию выбрали
void FormListElement::OnSetSelect()
{
}

//Линия потеряла выбор
void FormListElement::OnLostSelect()
{
}

//Активировать элемент
void FormListElement::OnAction()
{
}

//Разослать эвент остальным всем элементам списка
void FormListElement::SendEvent(const FormListEventData & data)
{
	list.SendEvent(data);
}

//Разослать эвент остальным всем элементам списка
void FormListElement::SendEvent(dword id)
{
	FormListEventData data = {id};
	list.SendEvent(data);		 
}

//На следующем кадре подняться выше по иерархии
void FormListElement::UpByHerarchy()
{
	list.SystemUpByHerarchy();
}

//На следующем кадре перейти в этот элемент
void FormListElement::Enter()
{
	list.SystemEnterTo(this);
}

//В клавиатурном фокусе ли лист
bool FormListElement::IsKbFoces()
{
	return options->kbFocus == &list;
}

//Нарисовать курсор
void FormListElement::DrawCursor(const GUIRectangle & rect)
{
	if(IsFocus())
	{				
		if(IsKbFoces())
		{
			GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, options->bkg2White[2]);
		}else{
			dword color = options->bkg2White[4] & 0x00ffffff;
			color |= 0x50000000;
			GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, color);
		}			
	}
}

//Нарисовать выделение
void FormListElement::DrawSelect(const GUIRectangle & rect)
{
	if(isSelect)
	{
		GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, options->colorSelect);
	}
}

//Нарисовать иконку
void FormListElement::DrawIcon(const GUIRectangle & rect, const GUIImage * image)
{
	if(!image) return;
	const long s = FormListBase::c_line_image_size;
	long x = rect.x + (FormListBase::c_line_leftspace - s)/2;
	long y = rect.y + (rect.h - s)/2;		
	GUIHelper::DrawSprite(x, y, s, s, (GUIImage *)image);
}

//Вывести текст
void FormListElement::DrawText(const GUIRectangle & rect, const ExtName * text, bool isImageSpace, dword color)
{
	if(!text || string::IsEmpty(text->str)) return;
	long x = rect.x + (isImageSpace ? FormListBase::c_line_leftspace : FormListBase::c_border);
	long y = rect.y + (rect.h - text->h)/2;
	options->render->Print(float(x), float(y), color, text->str);
}

FormListBase::FormListBase(GUIControl * parent, GUIRectangle rect, bool _disableAutoDelete) : GUIControl(parent),
																		elements(_FL_, 1024),
																		list(_FL_, 1)
{
	//Подстраеваем размер списка
	long count = 0;
	rect.h = FindListHeight(rect.h, &count);
	SetDrawRect(rect);
	SetClientRect(rect);
	//Таблица видимых строк
	list.AddElements(count);
	lineWidthWoScroll = rect.w - 2;
	lineWidthWScroll = lineWidthWoScroll - c_scroll_width;
	for(long i = 0; i < list; i++)
	{
		ListLine & line = list[i];
		line.rect.x = rect.x + 1;
		line.rect.y = rect.y + c_path_height + i*c_line_height;
		line.rect.w = lineWidthWoScroll;
		line.rect.h = c_line_height;
		ClientToScreen(line.rect);
		line.element = null;
	}
	isDrawFrame = false;
	weelBlocker = false;
	isDisableSort = false;
	needMoveUH = false;
	needMoveDH = null;
	disableAutoDelete = _disableAutoDelete;
	imagePath = null;
	scrollBar = null;
	buttonsWidth = c_border*2;
	memset(spellingBuffer, 0, ARRSIZE(spellingBuffer));
	spellingTimer = 0.0f;
}

FormListBase::~FormListBase()
{
	SystemClearList(null);
}

//Найти реальную высоту списка исходя из данной высоты
long FormListBase::FindListHeight(dword srcHeight, long * count)
{
	long c = (srcHeight - c_path_height + c_line_height - 1)/c_line_height;
	if(c < 1) c = 1;
	long h = c_path_height + c_line_height*c;
	if(h > (long)srcHeight && c > 1)
	{
		h -= c_line_height;
		c--;		
	}
	if(count) *count = c;
	return h;
}

//Инициализировать лист
void FormListBase::InitList()
{
	SystemClearList(null);
	DoInitList();
	SystemPrepareList();	
}

//Установить режим для рамки
void FormListBase::SetFrame(bool isDraw)
{
	isDrawFrame = isDraw;
}

//Установить фокус
void FormListBase::SetFocus(long line)
{
	if(line >= 0 && line < elements)
	{
		elements[line]->SetFocus();
	}
}

//Установить фокус на нужном элементе
void FormListBase::SetFocus(const char * name)
{
	dword len = 0;
	dword hash = string::HashNoCase(name, len);
	for(long i = 0; i < elements; i++)
	{
		if(elements[i]->IsThis(name, hash, len))
		{
			elements[i]->SetFocus();
			return;
		}
	}
}

//Переместить фокус и проследить чтобы он был видн
void FormListBase::MoveFocus(long delta)
{
	if(focusPosition < 0)
	{
		return;
	}
	if(delta >= 0)
	{
		focusPosition += delta;
		if(focusPosition >= elements) focusPosition = elements - 1;
		elements[focusPosition]->SetFocus();
		if(scrollBar)
		{
			long d = focusPosition - scrollBar->GetPosition();
			if(d >= list)
			{
				scrollBar->SetPosition(focusPosition - (list - 1));
			}
			if(focusPosition >= elements - 1)
			{
				scrollBar->SetPosition(scrollBar->GetPosition() + delta);
			}
		}
	}else{
		focusPosition += delta;
		if(focusPosition < 0) focusPosition = 0;
		elements[focusPosition]->SetFocus();
		if(scrollBar)
		{
			if(focusPosition < scrollBar->GetPosition())
			{
				scrollBar->SetPosition(focusPosition);
			}
		}
	}
}

//Установить/снять выделение
void FormListBase::SetSelect(long from, long to, bool isSelect)
{
	if(from > to) Swap(from, to);
	if(to < 0 || from >= elements) return;
	if(from < 0) from = 0;
	if(to >= elements) to = elements - 1;
	if(isSelect)
	{
		for(long i = from; i <= to; i++)
		{
			FormListElement * e = elements[i];
			if(!e->isSelect)
			{
				e->isSelect = true;
				e->OnSetSelect();
			}
		}
	}else{
		for(long i = from; i <= to; i++)
		{
			FormListElement * e = elements[i];
			if(e->isSelect)
			{
				e->isSelect = false;
				e->OnLostSelect();
			}
		}
	}
}

//Получить количество выделенных
dword FormListBase::GetSelectCount()
{
	dword count = 0;
	for(long i = 0; i < elements; i++)
	{
		count += (elements[i]->isSelect ? 1 : 0);
	}
	return count;
}

//Получить список выделенных
void FormListBase::FillSelectList(array<FormListElement *> & selected)
{
	selected.Empty();
	//Перебираем выделенные линии
	for(long i = 0; i < elements; i++)
	{
		if(elements[i]->IsSelect() || elements[i]->IsFocus())
		{
			selected.Add(elements[i]);
		}
	}
	if(selected.Size() == 0)
	{
		if(focusPosition >= 0 && focusPosition < elements)
		{
			selected.Add(elements[focusPosition]);
		}
	}
}

//Подняться по иерархии
void FormListBase::DoUpByHerarchy()
{
}

//Опуститься по иерархии ниже
void FormListBase::DoEnterTo(FormListElement * element)
{
}

//Лист был обновлён
void FormListBase::OnListUpdated()
{
}

//Получить нажатую кнопку
void FormListBase::OnKeyPressed(dword code, dword isSys)
{
}

//Событие изменения фокуса
void FormListBase::OnChangeFocus()
{
	onChangeFocus.Execute(this);
}

//Пришёл клавиатурный фокус
void FormListBase::OnKbFocus()
{
}

//Был поворот колеса, true подвинуть автоматически фокус
bool FormListBase::OnWeel(bool isUp)
{
	return true;
}

//Рисование
void FormListBase::Draw()
{	
	if(spellingTimer > 0.0f)
	{
		spellingTimer -= api->GetDeltaTime();
		if(spellingTimer <= 0.0f)
		{
			memset(spellingBuffer, 0, ARRSIZE(spellingBuffer));
			spellingTimer = 0.0f;
		}
	}
	if(!Visible) return;
	weelBlocker = false;
	if(scrollBar)
	{
		if(currentPos != scrollBar->GetPosition())
		{
			currentPos = scrollBar->GetPosition();
			if(focusPosition < scrollBar->GetPosition())
			{
				//SetFocus(scrollBar->GetPosition());
			}
			if(focusPosition > scrollBar->GetPosition() + list - 1)
			{
				//SetFocus(scrollBar->GetPosition() + list - 1);
			}
			SystemUpdateDrawList();
		}		
	}
	//Выполняем команды перехода по иерархиям
	if(needMoveUH)
	{
		SystemClearList(null);
		DoUpByHerarchy();
		SystemPrepareList();
	}else
	if(needMoveDH)
	{
		SystemClearList(needMoveDH);
		DoEnterTo(needMoveDH);
		if(!disableAutoDelete)
		{
			delete needMoveDH;
		}
		SystemPrepareList();
	}
	//Прямоугольник контрола
	GUIRectangle r = GetDrawRect();
	ClientToScreen(r);
	//Картинка для пути
	GUIHelper::Draw2DRect(r.x, r.y, r.w, c_path_height - 1, options->bkg2White[1]);
	if(imagePath)
	{
		long x = r.x + (c_path_leftspace - c_path_image_size)/2;
		long y = r.y + (c_path_height - c_path_image_size)/2;
		GUIHelper::DrawSprite(x, y, c_path_image_size, c_path_image_size, imagePath);
	}
	//Текст для пути
	if(!caption.str && Caption.Len())
	{
		const char * filler = options->GetString(SndOptions::s_filler);
		caption.Init(filler);
		dword fillerWidth = caption.w;
		long x = imagePath ? c_path_leftspace : c_path_leftspace_no_image;		
		long headerWidth = GetDrawRect().w - x - buttonsWidth;
		if(headerWidth > c_space)
		{
			//Вычитаем символы из середины до тех пор пока не будет достаточной ширины
			captionData = Caption;
			long delPos = -1;
			while(true)
			{
				caption.Init(captionData.c_str());
				if(caption.w < headerWidth) break;				
				if(delPos < 0)
				{
					//Первичная инициализация усекания строки
					headerWidth -= fillerWidth;
					if(headerWidth <= 0)
					{
						delPos = 0;
						captionData.Empty();
						break;
					}					
					delPos = captionData.Len()/3;
					if(delPos < 0) delPos = 0;
				}
				//Корректируем позицию выгрызания
				if(delPos >= (long)captionData.Len()/3) delPos--;				
				if(delPos < 0) delPos = 0;				
				//Удаляем с заданной позиции
				captionData.Delete(delPos, 1);
			}
			if(delPos >= 0)
			{
				captionData.Insert(delPos, filler);
				caption.Init(captionData.c_str());
			}
		}
	}
	if(caption.str)
	{
		float x = (float)r.x + (imagePath ? (float)c_path_leftspace : (float)c_path_leftspace_no_image);
		float y = r.y + (c_path_height - caption.h)*0.5f;
		options->render->Print(x, y, options->colorTextLo, caption.str);
	}
	long y = r.y + c_path_height - 2;
	GUIHelper::Draw2DLine(r.x, y, r.x + r.w, y, options->black2Bkg[4]);
	//Список волн
	GUIPoint splPoint;
	long drawSpelling = 0;
	for(long i = 0; i < list; i++)
	{
		ListLine & line = list[i];
		if(line.element)
		{
			line.element->Draw(line.rect);
			if(spellingBuffer[0] != 0 && line.element->IsFocus())
			{
				splPoint.x = line.rect.x + line.rect.w/2;				
				if(line.rect.y < r.y + r.h/2 + r.h/4)
				{
					splPoint.y = line.rect.y + line.rect.h;
					drawSpelling = 1;
				}else{
					splPoint.y = line.rect.y;
					drawSpelling = -1;
				}
			}
		}
	}
	//Рамка
	if(isDrawFrame)
	{
		GUIHelper::DrawWireRect(r.x, r.y, r.w, r.h, options->black2Bkg[0]);
	}
	if(!Enabled)
	{
		GUIHelper::Draw2DRect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, 0x1f808080);
	}else{
		if(drawSpelling)
		{
			spellingDraw.data = spellingBuffer;
			Assert(spellingDraw.data.Len() > 0);
			spellingDraw.data.Lower();
			spellingDraw.Init(spellingDraw.data.c_str());
			long space = 5;			
			long w = spellingDraw.w + space*2;
			long h = spellingDraw.h + space*2;
			long y = drawSpelling > 0 ? splPoint.y : splPoint.y - h - 1;
			GUIHelper::Draw2DRect(splPoint.x, y, w, h, options->bkg2White[7]);
			GUIHelper::DrawWireRect(splPoint.x, y, w, h, options->black2Bkg[0]);
			options->render->Print(splPoint.x + (float)space, y + (float)space, options->black2Bkg[0], spellingDraw.str);			
			ExtName lastChar;
			lastChar.Init(spellingDraw.data.c_str() + spellingDraw.data.Len() - 1);
			GUIHelper::Draw2DLine(splPoint.x + w - space - lastChar.w - 1, y + h - space + 1, splPoint.x + w - space + 1, y + h - space + 1, options->bkg2White[7] ^ 0x0000ffff);
		}
	}
	//Дочернии объекты и тп
	GUIControl::Draw();
}

//Сообщения
bool FormListBase::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
	switch(message)
	{
	case GUIMSG_MOUSEMOVE:
		for(long i = 0; i < list; i++)
		{
			ListLine & line = list[i];
			if(line.element)
			{
				if(line.rect.Inside(cursor_pos))
				{
					line.element->OnMouseMove(line.rect, cursor_pos);
				}
			}
		}
		break;
	case GUIMSG_LMB_DOWN:
	case GUIMSG_LMB_DBLCLICK:
		if(options->IsHighlight(this))
		{
			options->kbFocus = this;
			options->gui_manager->SetKeyboardFocus(this);
			for(long i = 0; i < list; i++)
			{			
				ListLine & line = list[i];
				FormListElement * elm = line.element;
				if(elm)
				{
					if(line.rect.Inside(cursor_pos))
					{
						if(elm->MouseClick(line.rect, cursor_pos))
						{
							if(message == GUIMSG_LMB_DBLCLICK)
							{
								SetSelect(0, elements, false);
								elm->SetFocus();
								elm->OnAction();
							}else
							if(GetAsyncKeyState(VK_CONTROL) < 0)
							{
								SetSelect(elm->index, elm->index, !elm->isSelect);
								elm->SetFocus();
							}else
							if(GetAsyncKeyState(VK_SHIFT) < 0)
							{
								long from = elm->index;
								long to = focusPosition;
								if(to >= 0)
								{
									SetSelect(0, elements, false);
									SetSelect(from, to, true);
								}else{
									SetSelect(0, elements, false);
									SetSelect(elm->index, elm->index, true);
									elm->SetFocus();
								}
							}else{
								SetSelect(0, elements, false);
								elm->SetFocus();
							}
						}
					}
				}
			}
			OnKbFocus();
		}
		break;
	case GUIMSG_KEYPRESSED:
		OnKeyPressed(lparam, hparam);
		if(options->kbFocus == this)
		{
			switch(lparam)
			{
			case VK_UP:
				MoveFocus(-1);
				break;
			case VK_DOWN:
				MoveFocus(1);
				break;
			case VK_HOME:
				MoveFocus(-elements);
				break;
			case VK_END:
				MoveFocus(elements);
				break;
			case VK_PRIOR:			
				MoveFocus(-coremax(list/2, 1));
				break;
			case VK_NEXT:
				MoveFocus(coremax(list/2, 1));
				break;
			case VK_INSERT:
				if(focusPosition >= 0 && focusPosition < elements)
				{
					bool curState = elements[focusPosition]->IsSelect();
					SetSelect(focusPosition, focusPosition, !curState);
					if(curState != elements[focusPosition]->IsSelect())
					{
						MoveFocus(1);
					}					
				}
				break;
			case VK_RETURN:
				if(focusPosition >= 0 && !hparam)
				{
					elements[focusPosition]->OnAction();
				}
				break;
			case VK_TAB:
				if(hparam)
				{
					for(GUIControl * ctrl = GetParent(); ctrl; ctrl = ctrl->GetParent())
					{
						((FormListBase *)ctrl)->ProcessMessages(GUIMSG_FORCE_DWORD, sndmsg_switchfocus, (DWORD)this);
					}
				}
				break;
			case VK_SHIFT:
			case VK_CONTROL:
				break;
			}
			if((lparam >= '0' && lparam <= '9') || (lparam >= 'A' && lparam <= 'Z') || (lparam == ' '))
			{
				spellingTimer = spellingWaitTimeInMs*0.001f;
				for(dword i = 0; i < ARRSIZE(spellingBuffer) - 1; i++)
				{
					if(!spellingBuffer[i])
					{
						spellingBuffer[i] = (char)lparam;
						spellingBuffer[i + 1] = 0;
						break;
					}
				}
				if(spellingBuffer[0])
				{
					if(!TrySetFocus(spellingBuffer))
					{
						//Если последнии буквы совпадают то пробуем слить						
						dword len = string::Len(spellingBuffer);
						if(len > 1)
						{
							bool isTryLastChar = false;
							if(spellingBuffer[len - 1] == spellingBuffer[len - 2])
							{
								spellingBuffer[len - 1] = 0;
								if(!TrySetFocus(spellingBuffer))
								{
									isTryLastChar = true;
								}
							}
							if(isTryLastChar)
							{
								spellingBuffer[0] = spellingBuffer[string::Len(spellingBuffer)];
								spellingBuffer[1] = 0;
								if(!TrySetFocus(spellingBuffer))
								{
									memset(spellingBuffer, 0, ARRSIZE(spellingBuffer));
									spellingTimer = 0.0f;
								}
							}
						}

					}
				}
			}
		}
		break;
	case GUIMSG_WHEEL_UP:
		if(!weelBlocker)
		{
			weelBlocker = true;
			if(OnWeel(true))
			{
				MoveFocus(-1);
			}
		}
		break;
	case GUIMSG_WHEEL_DOWN:
		if(!weelBlocker)
		{
			weelBlocker = true;
			if(OnWeel(false))
			{
				MoveFocus(1);
			}
		}
		break;
	}
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Добавить элемент в список
void FormListBase::AddElement(FormListElement * el)
{
	Assert(el);
	elements.Add(el);
}

//Разослать эвент всем элементам списка
void FormListBase::SendEvent(const FormListEventData & data)
{
	Assert(&data);
	for(long i = 0; i < elements; i++)
	{
		elements[i]->Event(data);
	}	
}

//Установить новый путь
void FormListBase::SetCaption(const char * str, GUIImage * image)
{
	Caption = str;
	captionData.Empty();
	caption.str = null;
	imagePath = image;
}

//Запретить сортировку
void FormListBase::DisableSort(bool isDisable)
{
	isDisableSort = isDisable;
}

//Добавить кнопку к списку и откоректировать прямоугольник для следующей
FormButton * FormListBase::ButtonAdd()
{
	GUIRectangle r = GetDrawRect();
	r.h = c_path_height - c_border*2;
	r.x = r.w - r.h - buttonsWidth;
	r.w = r.h;
	r.y = c_border - 1;
	FormButton * button = NEW FormButton(this, r);
	buttonsWidth += r.h + c_border;
	return button;
}

//Удалить кнопки
void FormListBase::ButtonsClear()
{
	buttonsWidth = c_border*2;
}

//Попытаться установить фокус на строку префикса
bool FormListBase::TrySetFocus(const char * prefix)
{
	if(!prefix || !prefix[0] || !Visible || !Enabled)
	{
		return false;
	}
	long ptr = focusPosition >= 0 ? focusPosition + 1 : 0;
	for(dword i = 0; i < elements.Size(); i++, ptr++)
	{						
		if(ptr >= (long)elements.Size()) ptr = 0;
		FormListElement * elm = elements[ptr];
		if(elm->IsPreficsEqual(prefix))
		{
			elm->SetFocus();
			return true;
		}
	}
	return false;
}

//Очистить список
void FormListBase::SystemClearList(FormListElement * exludeThat)
{
	if(!disableAutoDelete)
	{
		for(long i = 0; i < elements; i++)
		{
			if(elements[i] != exludeThat)
			{
				delete elements[i];
			}		
		}
	}
	elements.Empty();
}

//Обновить список
void FormListBase::SystemPrepareList()
{
	if(scrollBar)
	{
		delete scrollBar;
		scrollBar = null;
	}
	//Текущая позиция теперь на начале
	currentPos = 0;
	focusPosition = 0;
	//Сортируем
	/*
	for(dword i = 0; i < elements.Size(); i++)
	{
		for(dword j = i + 1; j < elements.Size(); j++)
		{
			if(elements[j]->IsLessThen(elements[i]))
			{
				FormListElement * element = elements[j];
				elements[j] = elements[i];
				elements[i] = element;
			}
		}
	}
	*/
	if(!isDisableSort)
	{
		elements.QSort(&FormListBase::SystemListLineSort);
	}
	//Устанавливаем элементам индексы
	for(long i = 0; i < elements; i++)
	{
		if(!disableAutoDelete)
		{
			Assert(elements[i]->index == -1);
		}
		elements[i]->index = i;
	}
	//Устанавливаем ширину
	dword w = lineWidthWoScroll;
	if(elements.Size() > list.Size())
	{
		w = lineWidthWScroll;
		GUIRectangle r = GetClientRect();
		r.x = r.w - c_scroll_width - 1;
		r.w = c_scroll_width;
		r.y = c_path_height - 1;
		r.h -= c_path_height;
		scrollBar = NEW FormScrollBar(this, r);
		scrollBar->SetRange(0, elements.Size() - (list.Size()*7)/8);
		scrollBar->SetPosition(currentPos);
		scrollBar->SetPageSize(1);
		scrollBar->SetButtonStep(1);
	}else{
		if(scrollBar)
		{
			delete scrollBar;
			scrollBar = null;
		}
	}
	for(long i = 0; i < list; i++)
	{
		list[i].rect.w = w;
	}
	//Обновляем отображаемый список
	SystemUpdateDrawList();
	//Обнуляем все переходы
	needMoveUH = false;
	needMoveDH = null;
	//Выделаем элемент
	if(currentPos >= elements) currentPos = elements - 1;
	if(currentPos < 0) currentPos = 0;	
	focusPosition = -1;
	if(elements > 0)
	{
		elements[currentPos]->SetFocus();		
	}
	OnListUpdated();
	OnChangeFocus();
}

//Функция сравнения для сортировки
bool FormListBase::SystemListLineSort(FormListElement * const & v1, FormListElement * const & v2)
{
	return v1->IsLessThen(v2);
}

//Обновляем отображаемый список
void FormListBase::SystemUpdateDrawList()
{
	if(scrollBar)
	{
		scrollBar->SetPosition(currentPos);
	}
	for(long i = 0; i < list; i++)
	{		
		long index = currentPos + i;
		if(index >= 0 && index < elements)
		{
			list[i].element = elements[index];
		}else{
			list[i].element = null;
		}
	}
}

//Проверить попадание фокуса в видимую область
void FormListBase::SystemCheckFocus()
{
	if(focusPosition < 0 || focusPosition > elements)
	{
		focusPosition = -1;
		OnChangeFocus();
		return;
	}
	if(focusPosition < currentPos)
	{
		currentPos = focusPosition;
		SystemUpdateDrawList();
		OnChangeFocus();
		return;
	}
	if(focusPosition - currentPos >= list)
	{
		currentPos = focusPosition - list + 1;
		SystemUpdateDrawList();
		OnChangeFocus();
		return;
	}
	OnChangeFocus();
}

//На следующем кадре подняться выше по иерархии
void FormListBase::SystemUpByHerarchy()
{
	needMoveUH = true;
}

//На следующем кадре перейти в папку
void FormListBase::SystemEnterTo(FormListElement * element)
{
	needMoveDH = element;
}

