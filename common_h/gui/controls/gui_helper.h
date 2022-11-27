#ifndef _XBOX
#ifndef GUI_RENDER_HELPER
#define GUI_RENDER_HELPER

#include "gui_point.h"
#include "gui_rectangle.h"
#include "gui_messages.h"
#include "gui_image.h"


class GUIHelper
{
	friend class GUIHelperResourses;
	struct GUIVertex
	{
		Vector p;
		unsigned long color;
	};

	struct GUIVertexUV
	{
		Vector p;
		unsigned long color;
		float u;
		float v;
	};


	static DWORD HelperColorHelp[7];

	static Vector Vertexs[256];
	static GUIVertex vertexs[64];

	static DWORD color;

	static DWORD line_color1;
	static DWORD line_color2;
	static DWORD line_color3;

	static DWORD editcolor;


	static DWORD scrollbarcolor;

	static IRender * rs;
	static ShaderId shaderGUIBase;
	static ShaderId shaderGUISprite;	
	static ShaderId shaderGUISelected;

	static void _ScreenToD3D (int sX, int sY, float &d3dX, float &d3dY);

public:
	
	
	static void ScreenToD3D (int sX, int sY, float &d3dX, float &d3dY);
	static void Draw2DRect (int pX, int pY, int width, int height, unsigned long color);
	
	static void Draw2DRect2 (int pX, int pY, int width, int height, unsigned long color);
	static void Draw2DLine (int pX, int pY, int tX, int tY, unsigned long color);
	static void Draw2DLine (float pX, float pY, float tX, float tY, unsigned long color);
	
	static void Draw2DRectHorizGradient (int pX, int pY, int width, int height, unsigned long color, unsigned long color2);
	
	static void DrawSprite (int pX, int pY, int width, int height, GUIImage* image, dword color = 0xffffffff);
	
	
	static void DrawBigUpBox (int pX, int pY, int width, int height, dword dwUserColor = 0xFFD4D0C8);
	
	static void DrawUpBox (int pX, int pY, int width, int height);
	
	
	static void DrawDownBox (int pX, int pY, int width, int height);
	
	static void DrawEditBox (int pX, int pY, int width, int height, bool DrawBorder = true);
	static void DrawEditBox2 (int pX, int pY, int width, int height, bool DrawBorder = true);
	
	static void DrawDownBorder (int pX, int pY, int width, int height);
	static void DrawUpBorder (int pX, int pY, int width, int height);
	static void DrawWireRect (int pX, int pY, int width, int height, unsigned long color);
	
	static void DrawScrollBarBox (int pX, int pY, int width, int height);
	
	
	static int PointInRect (const GUIPoint& pt, const GUIRectangle& rt);
	
	static int MessageSpreadOnlyForUnderCursorControls (GUIMessage msg);
	
	static void ExtractCursorPos (GUIMessage message, DWORD lparam, DWORD hparam, GUIPoint& pt);
	
	static void DrawLinesBox (int pX, int pY, int width, int height, DWORD color);
	
	
	static void Center (GUIPoint& pt, const GUIRectangle& r_to_center, const GUIRectangle& r_where_center);
	
	static void DrawVertLine (int from_y, int height, int xpos, DWORD my_color);
	static void DrawHorizLine (int from_x, int width, int ypos, DWORD my_color);
	
	
	static bool SetClipboardText (LPSTR lpszBuffer, HWND hWnd);
	static bool GetClipboardText (LPSTR lpszBuffer, HWND hWnd);

	static void DrawColorRect ( int pX, int pY, int width, int height, 
															unsigned long color1,
															unsigned long color2,
															unsigned long color3,
															unsigned long color4);


	static void DrawColorPickerRect (int pX, int pY, int width, int height);

	static void DrawAlphaRect (int pX, int pY, int width, int height);

	static DWORD GetColorPickerColor (int height, float GetPositionY);

	static void Draw2DRectAlpha (int pX, int pY, int width, int height, unsigned long color);

	static void DrawScrollBarUpBox (int pX, int pY, int width, int height);


	static void DrawPolygon (const GUIPoint* Points, int NumPoints, DWORD color, int offsetX = 0, int offsetY = 0);

	static void DrawLines (const GUIPoint* Points, int NumPoints, DWORD color, int offsetX = 0, int offsetY = 0);

	static void Draw2DLines (const GUIPoint* Points, int NumPoints, DWORD color, int offsetX = 0, int offsetY = 0);

	static IRender * GetRender();
	
	
};



#endif

#endif