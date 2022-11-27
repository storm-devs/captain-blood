//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormEdit
//============================================================================================
			

#include "FormEdit.h"

//============================================================================================

FormEdit::FormEdit(GUIControl* parent, const GUIRectangle & rect) : GUIEdit(parent, rect.x, rect.y, rect.w, rect.h)
{
	SelectionColor = options->colorSelect;
	isAccept = true;
	stringValue = null;
	floatValue = null;
	longValue = null;
	min = 0.0f;
	max = 0.0f;
	isLimitedNumber = false;
	this->OnChange = (CONTROL_EVENT)&FormEdit::ChangeText;
	this->OnAccept = (CONTROL_EVENT)&FormEdit::AcceptText;
}

FormEdit::~FormEdit()
{
}

void FormEdit::DrawEditBox(long nLeft, long nTop, long Width, long Height)
{
	dword bkg;
	if(isAccept)
	{
		FontColor = options->colorTextLo;
		bkg = options->bkg2White[3];
	}else{		
		FontColor = options->colorTextLo;
		bkg = options->bkg2White[7];
	}
	GUIHelper::Draw2DRect(nLeft, nTop, Width, Height, bkg);
	GUIHelper::DrawWireRect(nLeft, nTop, Width, Height, options->black2Bkg[0]);
}

void _cdecl FormEdit::ChangeText(GUIControl * c)
{
	isAccept = false;
	onChange.Execute(this);
}

void _cdecl FormEdit::AcceptText(GUIControl * c)
{
	isAccept = true;
	if(stringValue)
	{
		*stringValue = Text;
	}
	if(floatValue)
	{
		float f = (float)atof(Text);
		if(isLimitedNumber)
		{
			if(f < min) f = min;
			if(f > max) f = max;
		}
		*floatValue = f;
	}
	if(longValue)
	{
		float f = (float)atof(Text);
		if(isLimitedNumber)
		{
			if(f < min) f = min;
			if(f > max) f = max;
		}
		*longValue = (long)f;
	}
	UpdateText();
	onAccept.Execute(this);
}

void FormEdit::UpdateText()
{
	char buf[32];
	if(stringValue)
	{
		Text = *stringValue;
	}
	if(floatValue)
	{
		crt_snprintf(buf, 32, "%g", *floatValue);
		Text = buf;
	}
	if(longValue)
	{
		crt_snprintf(buf, 32, "%i", *longValue);
		Text = buf;
	}
}
