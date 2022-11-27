#include "hint.h"


csHint::csHint()
{
	pFnt = NULL;

	HintText = "";
}

csHint::~csHint()
{
	if (pFnt)
	{
		pFnt->Release();
		pFnt = NULL;
	}
}


//Создание объекта
bool csHint::Create(MOPReader & reader)
{
	EditMode_Update (reader);
	return true;
}


//Обновление параметров
bool csHint::EditMode_Update(MOPReader & reader)
{
	const char * FontName = reader.String().c_str();

	if (pFnt)
	{
		pFnt->Release();
		pFnt = NULL;
	}

	pFnt = Render().CreateFont(FontName);

	pFnt->SetHeight(reader.Float());

	HintText = reader.LocString();

	fBaseShowTime = fShowTime = reader.Float();

	Activate(reader.Bool());

	return true;
}


void _cdecl csHint::Realize(float fDeltaTime, long level)
{
	if (fShowTime > 0 && IsActive())
	{
		fShowTime = fShowTime - fDeltaTime;
		if (fShowTime < 0.0f)
		{
			Activate(false);
		}
	}

	float fPixelWidth = pFnt->GetLength("%s", HintText.c_str());
	float fPixelHeight = pFnt->GetHeight("%s", HintText.c_str());

	RENDERVIEWPORT vp = Render().GetViewport();

	float xOffset = ((float)vp.Width - fPixelWidth) / 2.0f;
	float yOffset = ((float)vp.Height - fPixelHeight) / 2.0f;

	



	pFnt->Print(xOffset, yOffset, "%s", HintText.c_str());
	
	
}


void csHint::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		fShowTime = fBaseShowTime;
		SetUpdate((MOF_UPDATE)&csHint::Realize, ML_GUI1);
	} else
	{
		DelUpdate((MOF_UPDATE)&csHint::Realize);
	}
}

void csHint::Show(bool isShow)
{
	MissionObject::Show(isShow);
	Activate(isShow);
}

/*
MOP_BEGINLISTG(csHint, "Hint", '1.00', 100, "Interface")
	MOP_STRING("Font name", "DemoFont")
	MOP_FLOAT ("Font size", 20.0f);
	MOP_LOCSTRING("Hint Text")
	MOP_FLOAT ("Show time", 2.0);
	MOP_BOOL("Active", true)
MOP_ENDLIST(csHint)
*/