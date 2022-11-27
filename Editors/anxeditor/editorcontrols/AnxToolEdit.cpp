//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxToolEdit	
//============================================================================================
			

#include "AnxToolEdit.h"

//============================================================================================

AnxToolEdit::AnxToolEdit(GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIEdit(pParent, Left, Top, Width, Height)
{
	SelectionColor = 0xff2040c0;
	isAccept = true;
	t_OnEditUpdate = NEW GUIEventHandler();
	stringValue = null;
	floatValue = null;
	longValue = null;
	min = 0.0f;
	max = 0.0f;
	isLimitedNumber = true;
	this->OnChange = (CONTROL_EVENT)&AnxToolEdit::ChangeText;
	this->OnAccept = (CONTROL_EVENT)&AnxToolEdit::AcceptText;	
}

AnxToolEdit::~AnxToolEdit()
{
	delete t_OnEditUpdate;
}


//============================================================================================


void AnxToolEdit::DrawEditBox(long nLeft, long nTop, long Width, long Height)
{
	//GUIHelper::Draw2DRectAlpha(nLeft, nTop, Width, Height, 0xdfe0e0f0);
	if(isAccept)
	{
		FontColor = 0xff000000;
	}else{
		FontColor = 0xffff0000;
	}
	GUIHelper::Draw2DRectAlpha(nLeft, nTop, Width, Height, 0xdfeae0f0);
	GUIHelper::DrawLinesBox(nLeft, nTop, Width, Height, 0x8f000000);
}

void _cdecl AnxToolEdit::ChangeText(GUIControl * c)
{
	isAccept = false;
}

void _cdecl AnxToolEdit::AcceptText(GUIControl * c)
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
	t_OnEditUpdate->Execute(this);
	UpdateText();
}

void AnxToolEdit::UpdateText()
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
