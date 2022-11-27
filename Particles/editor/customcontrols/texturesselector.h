//****************************************************************
#ifndef TEXTURE_SELECTOR
#define TEXTURE_SELECTOR

#include "..\..\..\common_h\gui.h"
#include "texview.h"




class TTextureSelector : public GUIWindow
{
	struct FrameInfo
	{
		//U - координата
		float U;
		//V - координата
		float V;

		//Ширина 0..1
		float W;

		//Высота 0..1
		float H;

		bool Draw;

		float selectionX;
		float selectionY;
		float selectionW;
		float selectionH;
	};

	array<FrameInfo> Frames;

	GUIImage* GlobalTexture;
	TTexPanel* Panel1;

	GUIPanel* GridColor;
	GUIPanel* SelectColor;

	GUIButton* btnSize16;
	GUIButton* btnSize32;
	GUIButton* btnSize64;
	GUIButton* btnSize128;

	GUILabel* ColorDesc;
	GUILabel* ColorDesc2;
	GUIListBox* lstFrames;

	GUIButton* btnAddFrame;
	GUIButton* btnRemoveFrame;

	GUIButton* btnClose;


	float GridMode; /*размер сетки тут */

	float Scale;
	int OffsetX;
	int OffsetY;

	DWORD GridLineColor;
	DWORD SelectionColor;

	

public:
	
	TTextureSelector ();
	~TTextureSelector ();

	void AddFrame (float U, float V, float W, float H);
	int GetFrameCount ();
	void GetFrame (int index, float& U, float& V, float& W, float& H);

	void _cdecl RenderViewport (GUIControl* sender);

	void _cdecl btn16Pressed (GUIControl* sender);
	void _cdecl btn32Pressed (GUIControl* sender);
	void _cdecl btn64Pressed (GUIControl* sender);
	void _cdecl btn128Pressed (GUIControl* sender);

	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

	virtual void MouseMove (int button, const GUIPoint& pt);

	void _cdecl RenderGridColor (GUIControl* sender);
	void _cdecl SelectColorDialog (GUIControl* sender);
	void _cdecl ApplyColor (GUIControl* sender);

	void _cdecl btnAddFramePressed (GUIControl* sender);
	void _cdecl btnRemoveFramePressed (GUIControl* sender);
	void _cdecl SelectFrame (GUIControl* sender);

	void _cdecl RenderSelColor (GUIControl* sender);
	void _cdecl SelectColorDialog2 (GUIControl* sender);
	void _cdecl ApplyColor2 (GUIControl* sender);


	void AssignTexture ();

	
	

  
};



#endif