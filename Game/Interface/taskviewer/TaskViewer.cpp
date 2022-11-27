
#include "TaskViewer.h"

//Вершины для рендера картинок
Vector4 TaskViewer::vertices[4];

TaskViewer::TaskViewer() : lines(_FL_)
{
	back = null;
	mask = null;
	font = null;
	texVar = null;
	w = h = 0.0f;
	vertices[0] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	vertices[1] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	vertices[2] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[3] = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
	showPosition = 0.0f;
}

TaskViewer::~TaskViewer()
{
	if(texVar)
	{
		texVar->SetTexture(null);
		texVar = null;
	}
	RELEASE(back);
	RELEASE(mask);
	RELEASE(font);
}

//Инициализировать объект
bool TaskViewer::Create(MOPReader & reader)
{
	Render().GetShaderId("Interface_TaskviewerPictureMask", Interface_TaskviewerPictureMask_id);
	Render().GetShaderId("Interface_TaskviewerPicture", Interface_TaskviewerPicture_id);




	if(!texVar)
	{
		texVar = Render().GetTechniqueGlobalVariable("TaskViewer_Texture", _FL_);
	}
	RELEASE(back);
	RELEASE(mask);
	RELEASE(font);
	const char * s = reader.String().c_str();
	if(s && s[0])
	{
	//	back = Render().CreateTexture(_FL_, s);
		back = Render().CreateTextureFullQuality(_FL_, s);
	}
	s = reader.String().c_str();
	if(s && s[0])
	{
	//	mask = Render().CreateTexture(_FL_, s);
		mask = Render().CreateTextureFullQuality(_FL_, s);
	}
	string str = reader.LocString();
	dword textColor = reader.Colors().GetDword();
	font = Render().CreateFont(reader.String().c_str());
	float size = reader.Float();
	if(font)
	{
		float scale = Render().GetScreenInfo2D().dwWidth/1024.0f;
		font->SetHeight(size*scale);
		font->SetColor(textColor);
		font->SetTechnique("Interface_TaskviewerFont");		
	}
	w = reader.Float();
	h = reader.Float();
	SetText(str);
	showTime = currentTime = reader.Float();
	showPosition = 0.0f;
	drawPriority = reader.Long();
	Show(reader.Bool());
	Activate(reader.Bool());
	return true;
}

//Инициализировать объект
bool TaskViewer::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обновить параметры
bool TaskViewer::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обработчик команд для объекта
void TaskViewer::Command(const char * id, dword numParams, const char ** params)
{
	if( numParams < 1 )
		return;

	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"Task"))
	{		
		SetText(params[0]);
	}
}

//Показать/скрыть объект
void TaskViewer::Show(bool isShow)
{
	if(EditMode_IsOn())
	{
		if(EditMode_IsSelect())
		{
			isShow = true;
		}
	}
	MissionObject::Show(isShow);
	if(isShow)
	{
		LogicDebug("Show");
		Activate(false);
		Activate(IsActive());
	}else{
		LogicDebug("Hide");
		DelUpdate(&TaskViewer::Draw);
	}
}

//Активировать
void TaskViewer::Activate(bool isActive)
{
	if(EditMode_IsOn())
	{
		if(EditMode_IsSelect())
		{
			isActive = true;
		}
	}
	bool isChange = isActive != IsActive();
	MissionObject::Activate(isActive);
	if(!IsShow())
	{
		LogicDebug("Save active state, but not applyed, becose element not visible");
		return;
	}
	if(isActive)
	{
		LogicDebug("Activate");
		if(isChange)
		{
			showPosition = 0.0f;
		}
		currentTime = showTime;
		if(back || mask)
		{
			SetUpdate(&TaskViewer::Draw, ML_GUI3 + drawPriority);
		}
	}else{
		LogicDebug("Deactivate");
	}
}

#ifndef MIS_STOP_EDIT_FUNCS
//Выделить объект
void TaskViewer::EditMode_Select(bool isSelect)
{
	MissionObject::EditMode_Select(isSelect);
	Show(IsShow());
	Activate(IsActive());
}
#endif

//Нарисовать модельку
void _cdecl TaskViewer::Draw(float dltTime, long level)
{	
	if(InterfaceUtils::IsHide()) return;
	if(EditMode_IsOn())
	{
		if(!EditMode_IsSelect()) return;
	}else{
		if(showTime > 0.0f)
		{
			currentTime -= dltTime;
			if(currentTime <= 0.0f)
			{
				Activate(false);
			}
		}
	}
	//Анализируем смещение
	const float moveSpeed = 2.0f;
	if(IsActive())
	{
		showPosition += dltTime*moveSpeed;
		if(showPosition > 1.0f)
		{
			showPosition = 1.0f;
		}
	}else{
		showPosition += dltTime*moveSpeed;
		if(showPosition >= 2.0f)
		{
			showPosition = 0.0f;
			if(!EditMode_IsOn())
			{
				DelUpdate(&TaskViewer::Draw);
			}
			return;
		}
	}
	if(EditMode_IsOn())
	{
		if(!EditMode_IsSelect()) return;
		showPosition = 1.0f;
	}
	//Сохраняем текущие параметры
	RENDERVIEWPORT savedVP = Render().GetViewport();
	Matrix savedView = Render().GetView();
	Matrix savedPrj = Render().GetProjection();
	//Новый вьюпорт
//	dword width = Render().GetScreenInfo().dwWidth;
//	dword height = Render().GetScreenInfo().dwHeight;
	const RENDERVIEWPORT &nat = Render().GetViewport();
	dword width  = nat.Width;
	dword height = nat.Height;
	RENDERVIEWPORT vp;
	vp.Width = dword(w*width);
	vp.Height = dword(h*height*InterfaceUtils::AspectRatio(Render()));
	vp.X = nat.X + 0;
	vp.Y = nat.Y + height - vp.Height - 1 - InterfaceUtils::HideFieldSize(Render());
	if(vp.Width < 16) vp.Width = 16;
	if(vp.Width > width) vp.Width = width;
	if(vp.Height < 16) vp.Height = 16;
	if(vp.Y + vp.Height > height) vp.Height = height - vp.Y;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	Render().SetViewport(vp);
	//Рисуем элементы
	RENDERRECT rect;		
	rect.x1 = vp.X;
	rect.y1 = vp.Y;
	rect.x2 = vp.X + vp.Width - 1;
	rect.y2 = vp.Y + vp.Height - 1;
	Render().Clear(1, &rect, CLEAR_ZBUFFER, 0, 1.0f, 0);
	if(mask)
	{
		vertices[0].z = showPosition;
		vertices[1].z = showPosition;
		vertices[2].z = showPosition;
		vertices[3].z = showPosition;
		texVar->SetTexture(mask);
		Render().DrawPrimitiveUP(Interface_TaskviewerPictureMask_id, PT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
		texVar->SetTexture(null);
	}
	if(back)
	{
		vertices[0].z = 1.0f;
		vertices[1].z = 1.0f;
		vertices[2].z = 1.0f;
		vertices[3].z = 1.0f;
		texVar->SetTexture(back);
		Render().DrawPrimitiveUP(Interface_TaskviewerPicture_id, PT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
		texVar->SetTexture(null);
	}
	if(font && lines > 0)
	{
		float y = (vp.Height - font->GetHeight()*(lines-1))*0.5f;
		for(long i = 0; i < lines; i++)
		{
			float x = (vp.Width - lines[i].width)*0.5f;			
			font->Print(x, y, lines[i].text);
			y += lines[i].height;
		}
	}
	//Востанавливаем параметры
	Render().SetViewport(savedVP);
	Render().SetView(savedView);
	Render().SetProjection(savedPrj);
}

//Установить текст
void TaskViewer::SetText(const char * rawText)
{
	lines.DelAll();
	totalTextHeight = 0.0f;
	string str;
	if(!rawText) rawText = "";
	if(rawText[0] == '#')
	{
		//Подменим идентификатор на реальную строку
		// потом
		str = rawText;
	}else{
		str = rawText;
	}

	InterfaceUtils::WordWrapString(str, font, w*Render().GetScreenInfo2D().dwWidth);

	//Разделяем строки
	dword index = lines.Add();
	for(dword i = 0; i < str.Size(); i++)
	{
		if(str[i] == '\\')
		{
			if(i + 1 < str.Size())
			{
				if(str[i + 1] == 'n')
				{
					if(lines[index].text.IsEmpty())
					{
						lines[index].text = " ";
					}
					index = lines.Add();
					i++;
					continue;
				}else
				if(str[i + 1] >= '0' && str[i + 1] <= '9')
				{
					long num = 0;
					for(i++; i < str.Size(); i++)
					{						
						if(str[i] >= '0' && str[i] <= '9')
						{
							num = num*10 + str[i];
						}
					}
					if(num < 1) num = 1;
					if(num > 255) num = 255;
					lines[index].text += char(num);
					continue;
				}
			}
		}
		else
		if(str[i] == '\n')
		{
			if(lines[index].text.IsEmpty())
			{
				lines[index].text = " ";
			}
			index = lines.Add();
			continue;
		}
		lines[index].text += str[i];
	}
	if(font)
	{
		for(long i = 0; i < lines; i++)
		{
			lines[i].width = font->GetLength(lines[i].text);
			lines[i].height = font->GetHeight(lines[i].text);
			totalTextHeight += lines[i].height;
		}
	}
}

const char * TaskViewer::comment = 
"UI element for show task description.\n"
"Commands:\n"
"   Task [new string]\n"
"  \n"
"For next line use \"\\n\" \n"
" ";


MOP_BEGINLISTCG(TaskViewer, "Task viewer", '1.00', 100000000, TaskViewer::comment, "Interface")
	MOP_STRING("Background", "")
	MOP_STRING("Show mask", "")
	MOP_LOCSTRING("Text")
	MOP_COLOR("Text color", Color(0xffffffffL))
	MOP_STRING("Font name", "DemoFont")
	MOP_FLOATEX("Font size", 20.0f, 4.0f, 1000.0f)
	MOP_FLOATEX("Width", 0.87f, 0.0f, 1.0f)
	MOP_FLOATEX("Height", 0.125f, 0.0f, 1.0f)
	MOP_FLOATEXC("Active time", 3.3f, 0.0f, 100.0f, "For active all time set 0")
	MOP_LONG("Draw priority", 0)
	MOP_BOOLC("Show", true, "Show or hide instantly")
	MOP_BOOLC("Active", true, "Show or hide with animation")
MOP_ENDLIST(TaskViewer)
