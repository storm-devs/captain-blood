#ifndef _XBOX

#include "uv.h"
#include "..\..\system\datasource\datauv.h"


extern IGUIManager* Application;

UVEditor::UVEditor ()
{
	TSelectorForm = NULL;
	EditedUV = NULL;
}

UVEditor::~UVEditor ()
{
}

void UVEditor::Release ()
{
	delete this;
}


void UVEditor::BeginEdit (GUIControl* form, DataUV* EditedUV)
{
	this->EditedUV = EditedUV;
	TSelectorForm = NEW TTextureSelector ();
	TSelectorForm->OnClose = (CONTROL_EVENT)&UVEditor::CloseForm;

	DWORD FramesCount = EditedUV->GetFrameCount();

	for (DWORD n = 0; n < FramesCount; n++)
	{
		Vector4 Frame = EditedUV->GetValue(n);
		TSelectorForm->AddFrame(Frame.x, Frame.y, Frame.z-Frame.x, Frame.w-Frame.y);
	}

	Application->ShowModal(TSelectorForm);


}

void UVEditor::EndEdit ()
{
	TSelectorForm = NULL;
}

void UVEditor::Apply ()
{
}

void _cdecl UVEditor::CloseForm (GUIControl* sender)
{
	int FrameCount = TSelectorForm->GetFrameCount();
	Vector4* FramesArray = NEW Vector4[FrameCount];

	for (int n = 0; n < FrameCount; n++)
	{
		Vector4 FrameInfo;
		TSelectorForm->GetFrame(n, FrameInfo.x, FrameInfo.y, FrameInfo.z, FrameInfo.w);

		FramesArray[n].x = FrameInfo.x;
		FramesArray[n].y = FrameInfo.y;
		FramesArray[n].z = FrameInfo.x + FrameInfo.z;
		FramesArray[n].w = FrameInfo.y + FrameInfo.w;
	}

	EditedUV->SetValues(FramesArray, FrameCount);

	delete FramesArray;
}

#endif