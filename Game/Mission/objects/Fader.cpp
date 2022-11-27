//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// Fader
//============================================================================================
			

#include "Fader.h"


//============================================================================================

Fader::Fader()
{
	fadeOutTime = 0.5f;
	fadeInTime = 0.5f;
	time = 0.5f;
	action = act_stop;
	color = Color(0.0f, 0.0f, 0.0f, 0.0f);
	texture = null;
	varColor = null;
	varTexture = null;
}

Fader::~Fader()
{
	if(varTexture)
	{
		varTexture->ResetTexture();
		varTexture = null;
	}

	varColor = null;
	
	if(texture)
	{
		texture->Release();
		texture = null;
	}
}

//============================================================================================

//Инициализировать объект
bool Fader::Create(MOPReader & reader)
{
	Render().GetShaderId("PostProcessFillScreen_Texture", PostProcessFillScreen_Texture_id);
	Render().GetShaderId("PostProcessFillScreen_Color", PostProcessFillScreen_Color_id);
	

	varColor = Render().GetTechniqueGlobalVariable("postProcess_FillScreen_Color", _FL_);
	Assert(varColor);
	varTexture = Render().GetTechniqueGlobalVariable("postProcess_FillScreen_Texture", _FL_);
	Assert(varTexture);
	time = 0.0f;
	action = act_stop;
	fadeOutTime = reader.Float();
	fadeInTime = reader.Float();
	SetTexture(reader.String().c_str());
	Color c = reader.Colors();
	SetColor(c);
	SetAlpha(c.a);
	start.Init(reader);
	hide.Init(reader);
	stop.Init(reader);
	Activate(reader.Bool());
	return true;
}

//Пересоздать объект
void Fader::Restart()
{
	ReCreate();
}

//Активировать
void Fader::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(IsActive())
		{
			SetUpdate(&Fader::Work, ML_ACTIVATE_TRIGGER + 100);
			SetUpdate(&Fader::Draw, ML_GUI5 + 100);
			time = 0.0f;
			if(fadeOutTime > 0.0f)
			{
				SetAlpha(0.0f);
			}else{
				SetAlpha(1.0f);
			}			
			action = act_start;
			LogicDebug("Activate");
		}else{
			DelUpdate(&Fader::Work);
			DelUpdate(&Fader::Draw);
			LogicDebug("Deactivate");
		}
	}
}

//Инициализировать объект
bool Fader::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool Fader::EditMode_Update(MOPReader & reader)
{
	Activate(false);
	return true;
}

//Получить размеры описывающего ящика
void Fader::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}

//Работа
void _cdecl Fader::Work(float dltTime, long level)
{
	dltTime = api->GetDeltaTime();
	switch(action)
	{
	case act_start:
		LogicDebug("Triggering start");
		start.Activate(Mission(), false, this);
		action = act_fadeout;		
	case act_fadeout:
		time += dltTime;
		if(time >= fadeOutTime)
		{
			action = act_fadein;
			LogicDebug("Triggering hide");
			hide.Activate(Mission(), false, this);
			time = 0.0f;
			SetAlpha(1.0f);
		}else{
			Assert(fadeOutTime > 0.0f);
			SetAlpha(time/fadeOutTime);
		}
		break;
	case act_fadein:
		if(time >= fadeInTime)
		{
			action = act_stop;
			LogicDebug("Triggering stop");
			stop.Activate(Mission(), false, this);
			time = 0.0f;
			Activate(false);
		}else{
			Assert(fadeInTime > 0.0f);
			SetAlpha(1.0f - time/fadeInTime);
		}
		time += dltTime;
		break;
	}
}

//Рисование
void _cdecl Fader::Draw(float dltTime, long level)
{
	varColor->SetVector4(color.v4);
	varTexture->SetTexture(texture);
	const RENDERSCREEN & rscr = Render().GetScreenInfo2D();
	if(texture)
	{
		Render().DrawFullScreenQuad((float)rscr.dwWidth, (float)rscr.dwHeight, PostProcessFillScreen_Texture_id);
	}else{
		Render().DrawFullScreenQuad((float)rscr.dwWidth, (float)rscr.dwHeight, PostProcessFillScreen_Color_id);
	}

}

void Fader::SetTexture(const char * fileName)
{
	if(texture)
	{
		texture->Release();
	}
	if(fileName && fileName[0])
	{
		texture = Render().CreateTexture(_FL_, fileName);
	}else{
		texture = Render().getWhiteTexture();
		texture->AddRef();
	}
	varTexture->SetTexture(texture);
}

void Fader::SetColor(const Color & c)
{
	color = c;
	color.Clamp();
}

void Fader::SetAlpha(float alpha)
{
	color.a = Clampf(alpha);
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(Fader, "Fader", '1.00', 0x0fffffff, "Fader in/out", "Effects")	
	MOP_FLOATEXC("Fade out", 1.2f, 0.0f, 1000.0f, "Fade out time in seconds")
	MOP_FLOATEXC("Fade in", 1.2f, 0.0f, 1000.0f, "Fade in time in seconds")
	MOP_STRINGC("Image", "", "Texture image for filling screen")
	MOP_COLORC("Color", Color(0.0f), "Filling color if dont set image")
	MOP_GROUPBEGC("Start", "Triggering when fader start")
		MOP_MISSIONTRIGGER("Start.")
	MOP_GROUPEND()
	MOP_GROUPBEGC("Hide", "Triggering when fader hide screen work")
		MOP_MISSIONTRIGGER("Hide.")
	MOP_GROUPEND()
	MOP_GROUPBEGC("End", "Triggering when fader end work")
		MOP_MISSIONTRIGGER("End.")
	MOP_GROUPEND()
	MOP_BOOL("Active", false)
MOP_ENDLIST(Fader)



