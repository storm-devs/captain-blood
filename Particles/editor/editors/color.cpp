#ifndef _XBOX

#include "color.h"
#include "..\..\system\datasource\datacolor.h"


ColorEditor::ColorEditor ()
{
	pEditor = NULL;
}

ColorEditor::~ColorEditor ()
{
}

  
void ColorEditor::Release ()
{
 delete this;
}  

void ColorEditor::BeginEdit (GUIControl* form, DataColor* EditedColor)
{
	this->EditedColor = EditedColor;

	GUIRectangle rect = form->GetClientRect();
	pEditor = NEW GUIColorEditor (form, 10, 50, rect.w -20, rect.h - 100);

	DWORD ValCount = EditedColor->GetValuesCount();

	for (DWORD n = 0; n < ValCount; n++)
	{
		const ColorVertex& ClrVrx = EditedColor->GetByIndex(n);
		
		GUIColorEditor::ColorPoint ptColor;
		ptColor.pos = ClrVrx.Time;
		ptColor.max = ClrVrx.MaxValue;
		ptColor.min = ClrVrx.MinValue;
		pEditor->Points.Add(ptColor);
	}


}

void ColorEditor::EndEdit ()
{
	if (pEditor) delete pEditor;
	pEditor = NULL;
}

void ColorEditor::Apply ()
{
	DWORD ColorsCount = pEditor->Points.Size();
	ColorVertex* ColorArray = NEW ColorVertex[ColorsCount];
	for (DWORD n = 0; n < ColorsCount; n++)
	{
		ColorArray[n].Time = pEditor->Points[n].pos;
		ColorArray[n].MinValue = pEditor->Points[n].min;
		ColorArray[n].MaxValue = pEditor->Points[n].max;
	}

	EditedColor->SetValues(ColorArray, ColorsCount);
	delete ColorArray;
}




#endif