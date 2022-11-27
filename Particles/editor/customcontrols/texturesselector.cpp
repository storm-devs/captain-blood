#ifndef _XBOX

#include "texturesselector.h"
#include "..\..\..\common_h\particles.h"
#include "strutil.h"

//extern IParticleManager* pPS;



TTextureSelector::TTextureSelector () : GUIWindow (NULL, 100, 100, 800, 600),  Frames(_FL_)
{
	bMovable = false;
	bSystemButton = false;
	GridLineColor = 0xFF805959;
	SelectionColor = 0xFF00FF71;
	pFont->SetName("arialcyrsmall");
	OffsetX = 0;
	OffsetY = 0;


	Scale = 1.0f;
	GridMode = 32.0f;
	GlobalTexture = NEW GUIImage();

	Caption = "Texture selector";
	SetScreenCenter();

	Panel1 = NEW TTexPanel (this, 5, 5, 512, 512);
	Panel1->OnBeforeDraw = (CONTROL_EVENT)&TTextureSelector::RenderViewport;
	Panel1->LMB_Pressed = (CONTROL_EVENT)&TTextureSelector::SelectFrame;

	

	ColorDesc = NEW GUILabel (this, 530, 8, 52, 18);
	ColorDesc->Layout = GUILABELLAYOUT_Left;
	ColorDesc->Caption = "Lines color";
	ColorDesc->pFont->SetName("arialcyrsmall");

	ColorDesc2 = NEW GUILabel (this, 620, 8, 52, 18);
	ColorDesc2->Layout = GUILABELLAYOUT_Left;
	ColorDesc2->Caption = "Selection color";
	ColorDesc2->pFont->SetName("arialcyrsmall");
	

	GridColor = NEW GUIPanel (this, 530, 25, 62, 18);
	GridColor->OnBeforeDraw = (CONTROL_EVENT)&TTextureSelector::RenderGridColor;
	GridColor->OnMouseDown = (CONTROL_EVENT)&TTextureSelector::SelectColorDialog;

	SelectColor = NEW GUIPanel (this, 630, 25, 62, 18);
	SelectColor->OnBeforeDraw = (CONTROL_EVENT)&TTextureSelector::RenderSelColor;
	SelectColor->OnMouseDown = (CONTROL_EVENT)&TTextureSelector::SelectColorDialog2;



	lstFrames = NEW GUIListBox (this, 533, 100, 250, 415);
	lstFrames->MakeFlat(true);


	
	btnAddFrame = NEW GUIButton (this, 533, 70, 24, 24);
	btnAddFrame->Hint = "Add frame";
	btnAddFrame->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::btnAddFramePressed;
	btnAddFrame->Glyph->Load("peditor\\addsystem");
	btnAddFrame->FlatButton = true;


	btnRemoveFrame = NEW GUIButton (this, 533+30, 70, 24, 24);
	btnRemoveFrame->Hint = "Remove frame";
	btnRemoveFrame->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::btnRemoveFramePressed;
	btnRemoveFrame->Glyph->Load("peditor\\delsystem");
	btnRemoveFrame->FlatButton = true;


	btnClose = NEW GUIButton (this, 713, 530, 56, 28);
	btnClose->Hint = "Close window";
	btnClose->Caption = "Close";
	btnClose->pFont->SetName("arialcyrsmall");
	btnClose->FlatButton = true;
	btnClose->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::Close;


	btnSize16 = NEW GUIButton (this, 25, 530, 48, 24);
	btnSize16->Hint = "Set size 16x16";
	btnSize16->FlatButton = true;
	btnSize16->GroupIndex = 1;
	btnSize16->Glyph->Load("peditor\\16x16");
	btnSize16->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::btn16Pressed;

	btnSize32 = NEW GUIButton (this, 25+(48*1)+(10*1), 530, 48, 24);
	btnSize32->Hint = "Set size 32x32";
	btnSize32->FlatButton = true;
	btnSize32->GroupIndex = 1;
	btnSize32->Glyph->Load("peditor\\32x32");
	btnSize32->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::btn32Pressed;
	btnSize32->Down = true;


	btnSize64 = NEW GUIButton (this, 25+(48*2)+(10*2), 530, 48, 24);;
	btnSize64->Hint = "Set size 64x64";
	btnSize64->FlatButton = true;
	btnSize64->GroupIndex = 1;
	btnSize64->Glyph->Load("peditor\\64x64");
	btnSize64->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::btn64Pressed;


	btnSize128 = NEW GUIButton (this, 25+(48*3)+(10*3), 530, 48, 24);
	btnSize128->Hint = "Set size 128x128";
	btnSize128->FlatButton = true;
	btnSize128->GroupIndex = 1;
	btnSize128->Glyph->Load("peditor\\128x128");
	btnSize128->OnMousePressed = (CONTROL_EVENT)&TTextureSelector::btn128Pressed;



}


TTextureSelector::~TTextureSelector ()
{
	delete GlobalTexture;
}

void _cdecl TTextureSelector::RenderViewport (GUIControl* sender)
{
	int index = lstFrames->SelectedLine;


	Caption.Format("Texture selector (zoom: %3.2f, x: %d, y: %d)", Scale, OffsetX-256, OffsetY-256);
	AssignTexture();
	int TexW = GlobalTexture->GetWidth();
	int TexH = GlobalTexture->GetHeight();

	GUIRectangle SRect = Panel1->GetScreenRect();

	cliper.Push();
	cliper.SetRectangle(SRect);

	float ImageWidth = SRect.w * Scale;
	float ImageHeight = SRect.h * Scale;
	float ImageLeft = (float)(SRect.x+OffsetX);
	float ImageTop = (float)(SRect.y+OffsetY);

	ImageLeft -= (SRect.w * Scale * 0.5f);
	ImageTop -= (SRect.w * Scale * 0.5f);

	GUIHelper::DrawSprite((int)ImageLeft, (int)ImageTop, (int)ImageWidth, (int)ImageHeight, GlobalTexture);

	float XCount = TexW / GridMode;
	float YCount = TexH / GridMode;

	float ScreenStepX = (int)ImageWidth / XCount;
	float ScreenStepY = (int)ImageHeight / YCount;


	for (int x = 0; x <= XCount; x++)
	{
		float offset = ScreenStepX*x;
		GUIHelper::Draw2DLine(ImageLeft+offset, ImageTop, ImageLeft+offset, ImageTop+ImageHeight, GridLineColor);
	}

	for (int y = 0; y <= YCount; y++)
	{
		float offset = ScreenStepY*y;
		GUIHelper::Draw2DLine(ImageLeft, ImageTop+offset, ImageLeft+ImageWidth, ImageTop+offset, GridLineColor);
	}
 

	if (index >= 0)
	{
		XCount = TexW / Frames[index].selectionW;
		YCount = TexH / Frames[index].selectionH;
		ScreenStepX = (int)ImageWidth / XCount;
		ScreenStepY = (int)ImageHeight / YCount;


		float offsetX = ScreenStepX*Frames[index].selectionX;
		float offsetY = ScreenStepY*Frames[index].selectionY;
		GUIHelper::Draw2DLine(ImageLeft+offsetX, ImageTop+offsetY, ImageLeft+offsetX+ScreenStepX+1, ImageTop+offsetY, SelectionColor);
		GUIHelper::Draw2DLine(ImageLeft+offsetX, ImageTop+offsetY+ScreenStepY, ImageLeft+offsetX+ScreenStepX, ImageTop+offsetY+ScreenStepY, SelectionColor);
		GUIHelper::Draw2DLine(ImageLeft+offsetX, ImageTop+offsetY, ImageLeft+offsetX, ImageTop+offsetY+ScreenStepY, SelectionColor);
		GUIHelper::Draw2DLine(ImageLeft+offsetX+ScreenStepX, ImageTop+offsetY, ImageLeft+offsetX+ScreenStepX, ImageTop+offsetY+ScreenStepY, SelectionColor);
	}

	cliper.Pop();

}

void _cdecl TTextureSelector::btn16Pressed (GUIControl* sender)
{
	GridMode = 16.0f;
}

void _cdecl TTextureSelector::btn32Pressed (GUIControl* sender)
{
	GridMode = 32.0f;
}

void _cdecl TTextureSelector::btn64Pressed (GUIControl* sender)
{
	GridMode = 64.0f;
}

void _cdecl TTextureSelector::btn128Pressed (GUIControl* sender)
{
	GridMode = 128.0f;
}

bool TTextureSelector::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (message == GUIMSG_WHEEL_DOWN)
	{
		Scale += 0.1f;
		if (Scale > 10.0f) Scale = 10.0f;
	}
	if (message == GUIMSG_WHEEL_UP)
	{
		Scale -= 0.1f;
		if (Scale < 1.0f) Scale = 1.0f;
	}
	return GUIWindow::ProcessMessages(message, lparam, hparam);
}

void TTextureSelector::MouseMove (int button, const GUIPoint& pt)
{
	//if (api->Controls->GetControlStateType("RightMouseButton") == CST_ACTIVE)
	if (Panel1->RMB_Pressed)
	{
		OffsetX += (int)Application->GetCursor()->DeltaX;
		OffsetY += (int)Application->GetCursor()->DeltaY;
	}
}


void _cdecl TTextureSelector::RenderGridColor (GUIControl* sender)
{
	GUIRectangle SRect = GridColor->GetScreenRect();
	GUIHelper::Draw2DRect(SRect.x, SRect.y, SRect.w, SRect.h, GridLineColor);
}

void _cdecl TTextureSelector::SelectColorDialog (GUIControl* sender)
{
	GUIColorPicker* cPicker = NEW GUIColorPicker (300, 100, "Select grid color", 0, 0);

	Color value(GridLineColor);
	cPicker->editA->Text = IntToStr ((int)(value.a * 255.0f));
	cPicker->editR->Text = IntToStr ((int)(value.r * 255.0f));
	cPicker->editG->Text = IntToStr ((int)(value.g * 255.0f));
	cPicker->editB->Text = IntToStr ((int)(value.b * 255.0f));
	cPicker->TextIsChanged (NULL);
	cPicker->OnApply = (CONTROL_EVENT)&TTextureSelector::ApplyColor;

	Application->ShowModal(cPicker);

	//ColorEdit->SelectedColor
}

void _cdecl TTextureSelector::ApplyColor (GUIControl* sender)
{
	GUIColorPicker* cPicker = (GUIColorPicker*)sender;
	GridLineColor = cPicker->SelectedColor;
}

void _cdecl TTextureSelector::btnAddFramePressed (GUIControl* sender)
{
	FrameInfo Empty;
	Empty.U = 0.0f;
	Empty.V = 0.0f;
	Empty.H = 0.0f;
	Empty.W = 0.0f;
	Empty.Draw = false;
	Frames.Add(Empty);

	string StrFrame;
	StrFrame.Format("Frame %d", (Frames.Size()-1));
	lstFrames->Items.Add(StrFrame);
}

void _cdecl TTextureSelector::btnRemoveFramePressed (GUIControl* sender)
{
	int index = lstFrames->SelectedLine;
	if (index == -1) return;
	Frames.Extract(index);
	lstFrames->Items.Delete(index);
	lstFrames->SelectedLine = -1;
}

void _cdecl TTextureSelector::SelectFrame (GUIControl* sender)
{
	int index = lstFrames->SelectedLine;
	if (index == -1) return;


	int TexW = GlobalTexture->GetWidth();
	int TexH = GlobalTexture->GetHeight();
	GUIRectangle SRect = Panel1->GetScreenRect();
	float ImageWidth = SRect.w * Scale;
	float ImageHeight = SRect.h * Scale;
	float ImageLeft = (float)(SRect.x+OffsetX);
	float ImageTop = (float)(SRect.y+OffsetY);

	ImageLeft -= (SRect.w * Scale * 0.5f);
	ImageTop -= (SRect.w * Scale * 0.5f);


	float XCount = TexW / GridMode;
	float YCount = TexH / GridMode;
	float GridSizeFloat = GridMode;


	float ScreenStepX = (int)ImageWidth / XCount;
	float ScreenStepY = (int)ImageHeight / YCount;

	int XSelected = -1;
	int YSelected = -1;

	float MouseX;
	float MouseY;

	GUIPoint mpos = Application->GetCursor()->GetPosition();
	MouseX = (float)mpos.x;
	MouseY = (float)mpos.y;

	for (int x = 0; x <= XCount; x++)
	{
		float offset = ScreenStepX*x;
		if (MouseX >= (ImageLeft+offset)  && MouseX <= (ImageLeft+offset+ScreenStepX))
		{
			XSelected = x;
			break;
		}
		//GUIHelper::Draw2DLine(ImageLeft+offset, ImageTop, ImageLeft+offset, ImageTop+ImageHeight, GridLineColor);
	}

	for (int y = 0; y <= YCount; y++)
	{
		float offset = ScreenStepY*y;
		if (MouseY >= (ImageTop+offset)  && MouseY <= (ImageTop+offset+ScreenStepY))
		{
			YSelected = y;
			break;
		}
		//GUIHelper::Draw2DLine(ImageLeft, ImageTop+offset, ImageLeft+ImageWidth, ImageTop+offset, GridLineColor);
	}

	if (XSelected == -1) return;
	if (YSelected == -1) return;


	
	float TexelX = XSelected * GridSizeFloat;
	float TexelY = YSelected * GridSizeFloat;
	float TexelWidth = GridSizeFloat;
	float TexelHeight = GridSizeFloat;


	TexelX = TexelX / TexW;
	TexelY = TexelY / TexH;
	TexelWidth = TexelWidth /	TexW;
	TexelHeight = TexelHeight /	TexH;

	Frames[index].U = TexelX;
	Frames[index].V = TexelY;
	Frames[index].W = TexelWidth;
	Frames[index].H = TexelHeight;
	Frames[index].Draw = true;
	Frames[index].selectionX = (float)XSelected;
	Frames[index].selectionY = (float)YSelected;
	Frames[index].selectionH = GridSizeFloat;
	Frames[index].selectionW = GridSizeFloat;
 
}


void _cdecl TTextureSelector::RenderSelColor (GUIControl* sender)
{
	GUIRectangle SRect = SelectColor->GetScreenRect();
	GUIHelper::Draw2DRect(SRect.x, SRect.y, SRect.w, SRect.h, SelectionColor);
}

void _cdecl TTextureSelector::SelectColorDialog2 (GUIControl* sender)
{
	GUIColorPicker* cPicker = NEW GUIColorPicker (300, 100, "Select Selection color", 0, 0);

	Color value(SelectionColor);
	cPicker->editA->Text = IntToStr ((int)(value.a * 255.0f));
	cPicker->editR->Text = IntToStr ((int)(value.r * 255.0f));
	cPicker->editG->Text = IntToStr ((int)(value.g * 255.0f));
	cPicker->editB->Text = IntToStr ((int)(value.b * 255.0f));
	cPicker->TextIsChanged (NULL);
	cPicker->OnApply = (CONTROL_EVENT)&TTextureSelector::ApplyColor2;

	Application->ShowModal(cPicker);
}

void _cdecl TTextureSelector::ApplyColor2 (GUIControl* sender)
{
	GUIColorPicker* cPicker = (GUIColorPicker*)sender;
	SelectionColor = cPicker->SelectedColor;
}

void TTextureSelector::AddFrame (float U, float V, float W, float H)
{
	AssignTexture();

	int TexW = GlobalTexture->GetWidth();
	int TexH = GlobalTexture->GetHeight();

	FrameInfo Empty;
	//Переводим в пиксели...
	Empty.selectionW = W * TexW;
	Empty.selectionH = W * TexH;
	Empty.selectionX = U * TexW;
	Empty.selectionY = V * TexH;

	//Рассчитываем позицию...
	Empty.selectionX /= Empty.selectionW;
	Empty.selectionY /= Empty.selectionH;

	Empty.U = U;
	Empty.V = V;
	Empty.H = H;
	Empty.W = W;
	Empty.Draw = true;
	Frames.Add(Empty);

	string StrFrame;
	StrFrame.Format("Frame %d", (Frames.Size()-1));
	lstFrames->Items.Add(StrFrame);
}

int TTextureSelector::GetFrameCount ()
{
	return Frames.Size();
}

void TTextureSelector::GetFrame (int index, float& U, float& V, float& W, float& H)
{
	U = Frames[index].U;
	V = Frames[index].V;
	W = Frames[index].W;
	H = Frames[index].H;
}

void TTextureSelector::AssignTexture ()
{
	IParticleService* pPS = (IParticleService*)api->GetService("ParticleService");
	Assert (pPS);

	DWORD mCount = pPS->GetManagersCount();
	if (mCount <= 0) return;
	
	IParticleManager* pManager = pPS->GetManagerByIndex(0);

	
	if (GlobalTexture->IsEmpty())
	{
		if (!GlobalTexture->DirectLoadTGA("particles"))
		{
			GlobalTexture->DirectLoad("particles");
		}
		
		int tTexW = GlobalTexture->GetWidth();
		int tTexH = GlobalTexture->GetHeight();
		OffsetX = 256;
		OffsetY = 256;
	}

}

#endif