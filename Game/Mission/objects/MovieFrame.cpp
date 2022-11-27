//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// MovieFrame
//============================================================================================
			

#include "MovieFrame.h"


//============================================================================================

MovieFrame::MovieFrame()
{
	time = 0.0f;
	speed = -1.0f;
	action = act_hide;
	color = Color(0.0f, 0.0f, 0.0f, 0.0f);
	varColor = null;
	largeshot = null;
}

MovieFrame::~MovieFrame()
{
	varColor = NULL;
	RELEASE(largeshot);
}

//============================================================================================

//Инициализировать объект
bool MovieFrame::Create(MOPReader & reader)
{
	Render().GetShaderId("PostProcessFillScreen_Color", PostProcessFillScreen_Color_id);

	if(!varColor)
	{
		varColor = Render().GetTechniqueGlobalVariable("postProcess_FillScreen_Color", _FL_);
		Assert(varColor);
	}
	if(!largeshot)
	{
		largeshot = api->Storage().GetItemLong("system.screenshot.Largeshot", _FL_);
	}
	time = 0.0f;
	speed = reader.Float();
	if(speed > 1e-3f)
	{
		speed = 1.0f/speed;
	}else{
		speed = -1.0f;
	}
	height = reader.Float()*0.01f;
	MissionObject::Activate(false);
	action = act_hide;
	color = reader.Colors();
	color.Clamp();
	Show(reader.Bool());
	Activate(reader.Bool());
	return true;
}

//Пересоздать объект
void MovieFrame::Restart()
{
	ReCreate();
}

//Показать/скрыть объект
void MovieFrame::Show(bool isShow)
{
	MissionObject::Show(isShow);
	if(isShow)
	{
		if(action != act_hide)
		{
			Activate(IsActive());
		}
		LogicDebug("Show");
	}else{
		DelUpdate();
		LogicDebug("Hide");
	}
}

//Активировать
void MovieFrame::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			SetUpdate();
			switch(action)
			{
			case act_activate:
				break;
			case act_show:
				break;
			case act_deactivate:
				action = act_activate;
				break;
			case act_hide:
				time = 0.0f;
				action = act_activate;
				break;
			}
			LogicDebug("Activate");
		}else{
			switch(action)
			{
			case act_activate:
				action = act_deactivate;
				SetUpdate();
				break;
			case act_show:
				time = 1.0f;
				action = act_deactivate;
				SetUpdate();
				break;
			case act_deactivate:
				SetUpdate();
				break;
			case act_hide:
				break;
			}
			LogicDebug("Deactivate");
		}
	}
}

//Инициализировать объект
bool MovieFrame::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool MovieFrame::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Получить размеры описывающего ящика
void MovieFrame::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}

//Рисование
void _cdecl MovieFrame::Draw(float dltTime, long level)
{
	if(largeshot)
	{
		if(largeshot->Get(0))
		{
			return;
		}
	}
	switch(action)
	{
	case act_activate:
		if(speed > 0.0f)
		{
			time += speed*dltTime;
			if(time >= 1.0f)
			{
				time = 1.0f;
				action = act_show;
			}
		}else{
			time = 1.0f;
			action = act_show;
		}		
		break;
	case act_show:
		break;
	case act_deactivate:
		if(speed > 0.0f)
		{
			time -= speed*dltTime;
			if(time <= 0.0f)
			{
				time = 0.0f;
				action = act_hide;
				DelUpdate();
				return;
			}
		}else{
			time = 0.0f;
			action = act_hide;
			DelUpdate();
			return;
		}
		break;
	case act_hide:
		DelUpdate();
		return;
	}
	varColor->SetVector4(color.v4);
	float h = 2.0f*height*time;
	if(h <= 1e-10f)
	{
		return;
	}
	//Треугольники
	//0,5--------1
	//|    \     |
	//4----------2,3
	//
	//
	//6,11-------7
	//|     \    |
	//10--------8,9
	static QuadVertex vrt[12] = {
		{Vector4(-1.0f, -1.0f, 0.1f, 1.0f), 0.0f, 0.0f},		//0
		{Vector4(1.0f, -1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//1
		{Vector4(1.0f, -1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//2
		{Vector4(1.0f, -1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//3
		{Vector4(-1.0f, -1.0f, 0.1f, 1.0f), 0.0f, 0.0f},		//4
		{Vector4(-1.0f, -1.0f, 0.1f, 1.0f), 0.0f, 0.0f},		//5
		{Vector4(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//6
		{Vector4(1.0f, 1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//7
		{Vector4(1.0f, 1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//8
		{Vector4(1.0f, 1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//9
		{Vector4(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f, 0.0f},			//10
		{Vector4(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f, 0.0f}			//11
	};
	//Заполняем высоты
	vrt[4].pos.y = vrt[3].pos.y = vrt[2].pos.y = -1.0f + h;
	vrt[11].pos.y = vrt[7].pos.y = vrt[6].pos.y = 1.0f - h;
	//Рисуем
	Render().DrawPrimitiveUP(PostProcessFillScreen_Color_id, PT_TRIANGLELIST, 4, vrt, sizeof(QuadVertex));

}

__forceinline void MovieFrame::SetUpdate()
{
	MissionObject::SetUpdate(&MovieFrame::Draw, ML_GUI1 - 1);
}

__forceinline void MovieFrame::DelUpdate()
{
	MissionObject::DelUpdate(&MovieFrame::Draw);
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MovieFrame, "Movie frame", '1.00', 0x0fffffff, "Black rectangles at top and bottom screen\n show - instant show/hide\nactivate - show slide effect", "Effects")	
	MOP_FLOATEXC("Slide time", 0.8f, 0.0f, 100.0f, "Slide animation time in seconds")
	MOP_FLOATEXC("Height", 10.0f, 0.0f, 100.0f, "Heigh of top or bottom field in percent")
	MOP_COLORC("Color", Color(0.0f), "Filling color")
	MOP_BOOLC("Show", true, "Show or hide frame")
	MOP_BOOLC("Active", false, "Change fileds position open or closed")
MOP_ENDLIST(MovieFrame)



