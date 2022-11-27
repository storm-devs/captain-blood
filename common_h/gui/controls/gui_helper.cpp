#ifndef _XBOX

#include "..\..\core.h"
#include "..\..\math3d.h"
#include "..\..\render.h"
#include "gui_helper.h"


#define ZVAL 0.0f


DWORD GUIHelper::HelperColorHelp[7] = { 0xFFFF0000, 0xFFFF00FF, 0xFF0000FF, 0xFF00FFFF, 0xFF00FF00, 0xFFFFFF00, 0xFFFF0000};

Vector GUIHelper::Vertexs[256];
GUIHelper::GUIVertex GUIHelper::vertexs[64];

DWORD GUIHelper::color = 0xFFD4D0C8;

DWORD GUIHelper::line_color1 = 0xFFFFFFFF;
DWORD GUIHelper::line_color2 = 0xFF808080;
DWORD GUIHelper::line_color3 = 0xFF404040;

DWORD GUIHelper::editcolor = 0xFFFFFFFF;


DWORD GUIHelper::scrollbarcolor = 0xFFEAE8E4;

IRender * GUIHelper::rs = null;
ShaderId GUIHelper::shaderGUIBase = null;
ShaderId GUIHelper::shaderGUISprite = null;
ShaderId GUIHelper::shaderGUISelected = null;

IRender * GUIHelper::GetRender()
{
	if(!rs)
	{
		rs = (IRender*)api->GetService("DX9Render");
		rs->GetShaderId("GUIBase", shaderGUIBase);
		rs->GetShaderId("GUISprite", shaderGUISprite);
		rs->GetShaderId("GUISelected", shaderGUISelected);		
	}
	return rs;
}


__forceinline void GUIHelper::_ScreenToD3D (int sX, int sY, float &d3dX, float &d3dY)
{
	IRender* rs = GetRender();
	
	float fScrX = float(rs->GetScreenInfo3D().dwWidth) / 2.0f;
	float fScrY = float(rs->GetScreenInfo3D().dwHeight) / 2.0f;
	
	d3dX = (float)sX / fScrX - 1.0f;
	d3dY = -((float)sY / fScrY - 1.0f);
}

void GUIHelper::ScreenToD3D (int sX, int sY, float &d3dX, float &d3dY)
{
	_ScreenToD3D (sX, sY, d3dX, d3dY);
}

void GUIHelper::Draw2DRectHorizGradient (int pX, int pY, int width, int height, unsigned long color, unsigned long color2)
{
	IRender* rs = GetRender();
	
	GUIVertex * vrx = vertexs;
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);
	
	
	vrx[0].p = From;
	vrx[1].p = Vector (To.x, From.y, 0.0f);
	vrx[2].p = To;
	
	vrx[3].p = To;
	vrx[4].p = Vector (From.x, To.y, 0.0f);
	vrx[5].p = From;
	
	for (int r = 0; r < 6; r++) vrx[r].color = color;
	
	vrx[1].color = color2;
	vrx[2].color = color2;
	vrx[3].color = color2;
	

	for (r = 0; r < 6; r++) vrx[r].p.z = ZVAL;
	
	rs->DrawPrimitiveUP (shaderGUIBase, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertex));
}



void GUIHelper::Draw2DRect (int pX, int pY, int width, int height, unsigned long color)
{
	IRender* rs = GetRender();
	
	GUIVertex * vrx = vertexs;
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);
	
	
	vrx[0].p = From;
	vrx[1].p = Vector (To.x, From.y, 0.0f);
	vrx[2].p = To;
	
	vrx[3].p = To;
	vrx[4].p = Vector (From.x, To.y, 0.0f);
	vrx[5].p = From;
	
	for (int r = 0; r < 6; r++) vrx[r].color = color;

	for (r = 0; r < 6; r++) vrx[r].p.z = ZVAL;

	rs->DrawPrimitiveUP (shaderGUIBase, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertex));
}


void GUIHelper::Draw2DRect2 (int pX, int pY, int width, int height, unsigned long color)
{
	IRender* rs = GetRender();
	
	GUIVertex * vrx = vertexs;
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);
	
	
	vrx[0].p = From;
	vrx[1].p = Vector (To.x, From.y, 1.0f);
	vrx[2].p = To;
	
	vrx[3].p = To;
	vrx[4].p = Vector (From.x, To.y, 1.0f);
	vrx[5].p = From;
	
	for (int r = 0; r < 6; r++) vrx[r].color = color;

	for (r = 0; r < 6; r++) vrx[r].p.z = ZVAL;

	rs->DrawPrimitiveUP (shaderGUISelected, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertex));
}


void GUIHelper::Draw2DLine (float pX, float pY, float tX, float tY, unsigned long color)
{
	Draw2DLine ((int)pX, (int)pY, (int)tX, (int)tY, color);
}
void GUIHelper::Draw2DLine (int pX, int pY, int tX, int tY, unsigned long color)
{
	IRender* rs = GetRender();
	
	GUIVertex * vrx = vertexs;
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D (tX, tY, To.x, To.y);
	
	
	vrx[0].p = From;
	vrx[1].p = To;
	
	for (int r = 0; r < 2; r++) vrx[r].color = color;

	for (r = 0; r < 2; r++) vrx[r].p.z = ZVAL;

	rs->DrawPrimitiveUP (shaderGUIBase, PT_LINELIST, 1, vrx, sizeof (GUIVertex));
	
}



void GUIHelper::DrawSprite (int pX, int pY, int width, int height, GUIImage* image, dword color)
{
	IRender* rs = GetRender();

	//В теории это бесплатно и без выделений памяти, т.к. переменную GUITex
	//юзает GUIManager - тут просто поиск и референсирование
	IVariable* pVar = rs->GetTechniqueGlobalVariable("GUITex", _FL_);
	if (pVar)
	{
		//pVar->SetTexture(image->GetTexture());

		// Nikita: Hot Fix (Hot Dog, Hot Cat, etc.) Отрисовка без текстуры...
		if( image->GetTexture() )
			pVar->SetTexture( image->GetTexture() );
		else
			pVar->ResetTexture();
		// !Nikita: Fix
	}

	GUIVertexUV vrx[6];
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);
	From.z = ZVAL;
	To.z = ZVAL;
	
	vrx[0].p = From;
	vrx[0].color = color;
	vrx[0].u = 0.0f;
	vrx[0].v = 0.0f;
	
	
	vrx[1].p = Vector (To.x, From.y, 0);
	vrx[1].color = color;
	vrx[1].u = 1.0f;
	vrx[1].v = 0.0f;
	
	
	vrx[2].p = To;
	vrx[2].color = color;
	vrx[2].u = 1.0f;
	vrx[2].v = 1.0f;
	
	
	vrx[3].p = To;
	vrx[3].color = color;
	vrx[3].u = 1.0f;
	vrx[3].v = 1.0f;
	
	
	vrx[4].p = Vector (From.x, To.y, ZVAL);
	vrx[4].color = color;
	vrx[4].u = 0.0f;
	vrx[4].v = 1.0f;
	
	
	vrx[5].p = From;
	vrx[5].color = color;
	vrx[5].u = 0.0f;
	vrx[5].v = 0.0f;
	
	rs->DrawPrimitiveUP (shaderGUISprite, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertexUV));
}




void GUIHelper::DrawBigUpBox (int pX, int pY, int width, int height, dword dwUserColor)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft, nTop, Width-1, Height-1, dwUserColor);
	
	Draw2DLine ((nLeft+1), (nTop+1), (nLeft+1), (nTop+Height-2), line_color1);
	Draw2DLine ((nLeft+1), (nTop+1), (nLeft+Width-2), (nTop+1), line_color1);
	
	
	Draw2DLine ((nLeft+Width-2), (nTop), (nLeft+Width-2), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+Height-2), (nLeft+Width-2), (nTop+Height-2), line_color2);
	
	Draw2DLine ((nLeft+Width-1), (nTop-1), (nLeft+Width-1), (nTop+Height-1), line_color3);
	Draw2DLine (nLeft, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color3);
	
}


void GUIHelper::DrawUpBox (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft, nTop, Width, Height, color);
	
	//Яркие линии
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color1);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width-1), nTop, line_color1);
	
	
	Draw2DLine ((nLeft+Width-2), (nTop), (nLeft+Width-2), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+Height-2), (nLeft+Width-2), (nTop+Height-2), line_color2);
	
	Draw2DLine ((nLeft+Width-1), (nTop-1), (nLeft+Width-1), (nTop+Height-1), line_color3);
	Draw2DLine (nLeft, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color3);
	
}

void GUIHelper::DrawDownBox (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft, nTop, Width, Height, color);
	
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color3);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width), nTop, line_color3);
	
	Draw2DLine ((nLeft+Width-1), (nTop), (nLeft+Width-1), (nTop+Height-1), line_color1);
	Draw2DLine (nLeft+1, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color1);
	
	
	Draw2DLine ((nLeft+1), (nTop), (nLeft+1), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+1), (nLeft+Width-1), (nTop+1), line_color2);
	
}

void GUIHelper::DrawEditBox (int pX, int pY, int width, int height, bool DrawBorder)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft+2, nTop+2, Width-4, Height-4, editcolor);
	
	if (DrawBorder == false) return;
	
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color3);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width), nTop, line_color3);
	
	Draw2DLine ((nLeft+Width-1), (nTop), (nLeft+Width-1), (nTop+Height-1), line_color1);
	Draw2DLine (nLeft+1, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color1);
	
	
	Draw2DLine ((nLeft+1), (nTop), (nLeft+1), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+1), (nLeft+Width-1), (nTop+1), line_color2);
	
}


void GUIHelper::DrawEditBox2 (int pX, int pY, int width, int height, bool DrawBorder)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft+2, nTop+2, Width-4, Height-4, editcolor);
	
	if (DrawBorder == false) return;
	
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color3);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width), nTop, line_color3);
	
	Draw2DLine ((nLeft+Width-1), (nTop), (nLeft+Width-1), (nTop+Height-1), line_color1);
	Draw2DLine (nLeft+1, (nTop+Height-1), nLeft+Width+1, (nTop+Height-1), 0x00000000);
	
	
	Draw2DLine ((nLeft+1), (nTop), (nLeft+1), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+1), (nLeft+Width-1), (nTop+1), line_color2);



	
}


void GUIHelper::DrawDownBorder (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color3);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width), nTop, line_color3);
	
	Draw2DLine ((nLeft+Width-1), (nTop), (nLeft+Width-1), (nTop+Height-1), line_color1);
	Draw2DLine (nLeft+1, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color1);
	
	
	Draw2DLine ((nLeft+1), (nTop), (nLeft+1), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+1), (nLeft+Width-1), (nTop+1), line_color2);
	
	
}

void GUIHelper::DrawWireRect (int pX, int pY, int width, int height, unsigned long color)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	//Яркие линии
	Draw2DLine ((nLeft), (nTop), nLeft, nTop+Height, color);
	Draw2DLine ((nLeft), (nTop), nLeft+Width+1, nTop, color);
	
	Draw2DLine (nLeft+Width, nTop, nLeft+Width, nTop+Height, color);
	Draw2DLine (nLeft, nTop+Height, nLeft+Width + 1, nTop+Height, color);

}

void GUIHelper::DrawUpBorder (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	//Яркие линии
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color1);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width-1), nTop, line_color1);
	
	
	Draw2DLine ((nLeft+Width-2), (nTop), (nLeft+Width-2), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+Height-2), (nLeft+Width-2), (nTop+Height-2), line_color2);
	
	Draw2DLine ((nLeft+Width-1), (nTop-1), (nLeft+Width-1), (nTop+Height-1), line_color3);
	Draw2DLine (nLeft, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color3);
	
	
}




int GUIHelper::PointInRect (const GUIPoint& pt, const GUIRectangle& rt)
{
	if (pt.X <= rt.Left) return false;
	if (pt.X > (rt.Left+rt.Width-1)) return false;
	
	if (pt.Y <= rt.Top) return false;
	if (pt.Y > (rt.Top+rt.Height-1)) return false;
	
	return true;
}


int GUIHelper::MessageSpreadOnlyForUnderCursorControls (GUIMessage msg)
{
	if ((msg > GUIMSG_BEGINMOUSESECTION) &&
	(msg < GUIMSG_ENDMOUSESECTION)) return true;
	
	return false;
}


void GUIHelper::ExtractCursorPos (GUIMessage message, DWORD lparam, DWORD hparam, GUIPoint& pt)
{
	pt.X = 0;
	pt.Y = 0;
	
	if ((message > GUIMSG_BEGINMOUSESECTION) &&
	(message < GUIMSG_ENDMOUSESECTION))
	{
		pt.X = lparam;
		pt.Y = hparam;
	}
}



void GUIHelper::DrawLinesBox (int pX, int pY, int width, int height, DWORD color)
{
	int nLeft = pX;
	int nTop = pY;
	int Width = pX+width;
	int Height = pY+height;
	
	Draw2DLine (nLeft, nTop, Width, nTop, color);
	Draw2DLine (nLeft, Height, Width, Height, color);
	
	Draw2DLine (nLeft, nTop, nLeft, Height, color);
	Draw2DLine (Width, nTop, Width, Height, color);
	
	
}


void GUIHelper::Center (GUIPoint& pt, const GUIRectangle& r_to_center, const GUIRectangle& r_where_center)
{
	
	int WidthRazn = r_where_center.Width - r_to_center.Width;
	int addX = WidthRazn / 2;
	addX += r_where_center.Left;
	
	
	int HeightRazn = r_where_center.Height - r_to_center.Height;
	int addY = HeightRazn / 2;
	addY += r_where_center.Top;
	
	pt.X = addX;
	pt.Y = addY;
}


void GUIHelper::DrawVertLine (int from_y, int height, int xpos, DWORD my_color)
{
	Draw2DLine (xpos, from_y, xpos, from_y+height, my_color);
}

void GUIHelper::DrawHorizLine (int from_x, int width, int ypos, DWORD my_color)
{
	Draw2DLine (from_x, ypos, from_x+width, ypos, my_color);

}


bool GUIHelper::SetClipboardText (LPSTR lpszBuffer, HWND hWnd)
{
	if (strlen(lpszBuffer) <= 0) return false;
	
	HGLOBAL hGlobal;
	LPSTR lpszData;
	unsigned long nSize;
	OpenClipboard(hWnd);
	EmptyClipboard();
	nSize = lstrlen(lpszBuffer);
	hGlobal = GlobalAlloc(GMEM_ZEROINIT, nSize+1);
	if (hGlobal == NULL) return FALSE;
	lpszData = (LPSTR)GlobalLock(hGlobal);
	for (UINT i = 0; i < nSize + 1; ++i)
	*(lpszData + i) = *(lpszBuffer + i);
	GlobalUnlock(hGlobal);
	SetClipboardData(CF_TEXT, hGlobal);
	CloseClipboard();
	return true;
}

bool GUIHelper::GetClipboardText (LPSTR lpszBuffer, HWND hWnd)
{
//	strcpy (lpszBuffer, "aaaaazzzcxzcxcaasdasafdjfsdjcvjcxvx");
	lpszBuffer[0] = 0;
	HGLOBAL hGlobal;
	LPSTR lpszData;
	OpenClipboard(hWnd);
	hGlobal  = GetClipboardData(CF_TEXT);
	if(hGlobal)
	{
		lpszData = (LPSTR)GlobalLock(hGlobal);
		unsigned long nSize = lstrlen(lpszData);;
		if (nSize > 16380) nSize = 16380;

		for (UINT i = 0; i < nSize + 1; ++i)
			*(lpszBuffer + i) = *(lpszData + i);

		GlobalUnlock(hGlobal);
	}
	CloseClipboard();


	dword dwStringSize = strlen(lpszBuffer);
	for (dword i = 0; i < dwStringSize;i++)
	{

		if (lpszBuffer[i] == 0x0A || lpszBuffer[i] == 0x0D)
		{
			lpszBuffer[i] = 0;
			break;
		}
		
		if (lpszBuffer[i] < 32)
		{
			memmove ((lpszBuffer+i), (lpszBuffer+i+1), dwStringSize-i);
		}
	}

	return true;
}


void GUIHelper::DrawScrollBarBox (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft+2, nTop+2, Width-4, Height-4, scrollbarcolor);
	
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color3);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width), nTop, line_color3);
	
	Draw2DLine ((nLeft+Width-1), (nTop), (nLeft+Width-1), (nTop+Height-1), line_color3);
	Draw2DLine (nLeft+1, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color3);
	
	
	//Draw2DLine ((nLeft+1), (nTop), (nLeft+1), (nTop+Height-2), line_color3);
	//Draw2DLine ((nLeft+1), (nTop+1), (nLeft+Width-1), (nTop+1), line_color3);
	
}



void GUIHelper::DrawColorRect ( int pX, int pY, int width, int height, 
																unsigned long color1,
																unsigned long color2,
																unsigned long color3,
																unsigned long color4)
{
  Color rColor1 = Color(color1);
  Color rColor2 = Color(color2);
  Color rColor3 = Color(color3);
  Color rColor4 = Color(color4);

	IRender* rs = GetRender();
	
	GUIVertex vrx[6*16*16];
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);

	float stepX = (To.x - From.x) / 16.0f;
	float stepY = (To.y - From.y) / 16.0f;

	
	int pos = 0;
	for (int y = 0; y < 16; y++)
	{
		
		for (int x = 0; x < 16; x++)
		{
			Vector cFrom = From;
			cFrom.x += stepX * x;
			cFrom.y += stepY * y;

			Vector cTo = cFrom;
			cTo.x += stepX;
			cTo.y += stepY;

			Color xCol1;
			xCol1.Lerp (rColor1, rColor3, ((float)y / 16.0f));

			Color xCol2;
			xCol2.Lerp (rColor2, rColor4, ((float)y / 16.0f));

			Color xCol3;
			xCol3.Lerp (rColor1, rColor3, ((float)(y+1) / 16.0f));

			Color xCol4;
			xCol4.Lerp (rColor2, rColor4, ((float)(y+1) / 16.0f));
			

			Color reColor1;
			reColor1.Lerp (xCol2, xCol1, ((float)x / 16.0f));

			Color reColor2;
			reColor2.Lerp (xCol2, xCol1, ((float)(x+1) / 16.0f));

			Color reColor3;
			reColor3.Lerp (xCol4, xCol3, ((float)x / 16.0f));

			Color reColor4;
			reColor4.Lerp (xCol4, xCol3, ((float)(x+1) / 16.0f));


			vrx[pos+0].p = Vector (cFrom.x, cTo.y, 0);
//			vrx[pos+0].color = color3;
			vrx[pos+0].color = reColor3.GetDword();

			vrx[pos+1].p = Vector (cTo.x, cFrom.y, 0);
//			vrx[pos+1].color = color1;
			vrx[pos+1].color = reColor2.GetDword();

			vrx[pos+2].p = cTo;
//			vrx[pos+2].color = color4;
			vrx[pos+2].color = reColor4.GetDword();
			
			vrx[pos+3].p = cFrom;
//			vrx[pos+3].color = color2;
			vrx[pos+3].color = reColor1.GetDword();

			vrx[pos+4].p = Vector (cTo.x, cFrom.y, 0);
//			vrx[pos+4].color = color1;
			vrx[pos+4].color = reColor2.GetDword();

			vrx[pos+5].p = Vector (cFrom.x, cTo.y, 0);;
//			vrx[pos+5].color = color3;
			vrx[pos+5].color = reColor3.GetDword();

			pos+= 6;

		}
	}

	
	
	
	
	
	for (int r = 0; r < 6*16*16; r++) vrx[r].p.z = ZVAL;

	rs->DrawPrimitiveUP (shaderGUIBase, PT_TRIANGLELIST, 2*16*16, vrx, sizeof (GUIVertex));

}



void GUIHelper::DrawColorPickerRect (int _pX, int _pY, int _width, int _height)
{
	int pX = _pX;
	int pY = _pY;
	int width = _width;
	//int height = _height;

	int height = (_height) / 6;
	//height++;


	IRender* rs = GetRender();
	GUIVertex vrx[6];

	for (int n =0; n < 6; n++)
	{
		Vector From = Vector (0.0f);
		Vector To = Vector (0.0f);
		
		ScreenToD3D (pX, pY, From.x, From.y);
		ScreenToD3D ((pX+width), (pY+height), To.x, To.y);

		vrx[0].p = From;
		vrx[1].p = Vector (To.x, From.y, 0);
		vrx[2].p = To;
		
		vrx[3].p = To;
		vrx[4].p = Vector (From.x, To.y, 0);
		vrx[5].p = From;
		
		for (int r = 0; r < 6; r++) vrx[r].color = HelperColorHelp[n];

		vrx[2].color = HelperColorHelp[n+1];
		vrx[3].color = HelperColorHelp[n+1];
		vrx[4].color = HelperColorHelp[n+1];
		
		for (r = 0; r < 6; r++) vrx[r].p.z = ZVAL;

		rs->DrawPrimitiveUP (shaderGUIBase, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertex));

		pY+= height;

	}

}


DWORD GUIHelper::GetColorPickerColor (int _height, float GetPositionY)
{
	int pY = 0;
	int height = (_height) / 6;

	for (int n =0; n < 6; n++)
	{
		if ((GetPositionY >= pY) && (GetPositionY <= (pY+height)))
		{
			float razn = (GetPositionY-pY);
			float kBlend = (float)razn/(float)height;
			Color c1 = Color (HelperColorHelp[n]);
			Color c2 = Color (HelperColorHelp[n+1]);
			Color rColor;
			rColor.Lerp (c1, c2, kBlend);
			return rColor.GetDword ();

		}

		pY+= height;
	}


	return 0x0;


}

void GUIHelper::DrawAlphaRect (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	GUIVertex vrx[6];
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);
	
	
	vrx[0].p = From;
	vrx[1].p = Vector (To.x, From.y, 0);
	vrx[2].p = To;
	
	vrx[3].p = To;
	vrx[4].p = Vector (From.x, To.y, 0);
	vrx[5].p = From;
	
	vrx[0].color = 0xFFFFFFFF;
	vrx[1].color = 0xFFFFFFFF;
	vrx[5].color = 0xFFFFFFFF;
	
	vrx[2].color = 0xFF000000;
	vrx[3].color = 0xFF000000;
	vrx[4].color = 0xFF000000;
	
	
	
	for (int r = 0; r < 6; r++) vrx[r].p.z = ZVAL;

	rs->DrawPrimitiveUP (shaderGUIBase, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertex));
}

void GUIHelper::Draw2DRectAlpha (int pX, int pY, int width, int height, unsigned long color)
{
	IRender* rs = GetRender();
	
	GUIVertex vrx[6];
	
	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);
	
	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);
	
	
	vrx[0].p = From;
	vrx[1].p = Vector (To.x, From.y, 0);
	vrx[2].p = To;
	
	vrx[3].p = To;
	vrx[4].p = Vector (From.x, To.y, 0);
	vrx[5].p = From;
	
	for (int r = 0; r < 6; r++) vrx[r].color = color;

	for (r = 0; r < 6; r++) vrx[r].p.z = ZVAL;


	rs->DrawPrimitiveUP (shaderGUIBase, PT_TRIANGLELIST, 2, vrx, sizeof (GUIVertex));

}


void GUIHelper::DrawScrollBarUpBox (int pX, int pY, int width, int height)
{
	IRender* rs = GetRender();
	
	int nLeft = pX;
	int nTop = pY;
	int Width = width;
	int Height = height;
	
	Draw2DRect (nLeft, nTop, Width, Height, 0xFFFFFFFF);
	
	//Яркие линии
	Draw2DLine ((nLeft), (nTop), nLeft, (nTop+Height-1), line_color1);
	Draw2DLine ((nLeft), (nTop), (nLeft+Width-1), nTop, line_color1);
	
	
	Draw2DLine ((nLeft+Width-2), (nTop), (nLeft+Width-2), (nTop+Height-2), line_color2);
	Draw2DLine ((nLeft+1), (nTop+Height-2), (nLeft+Width-2), (nTop+Height-2), line_color2);
	
	Draw2DLine ((nLeft+Width-1), (nTop-1), (nLeft+Width-1), (nTop+Height-1), line_color3);
	Draw2DLine (nLeft, (nTop+Height-1), nLeft+Width, (nTop+Height-1), line_color3);

}

void GUIHelper::DrawPolygon (const GUIPoint* Points, int NumPoints, DWORD color, int offsetX, int offsetY)
{
	if (NumPoints > ARRSIZE(Vertexs)) return;
	for (int n = 0; n < NumPoints; n++)
	{
		float d3dX, d3dY;
		ScreenToD3D (Points[n].x + offsetX, Points[n].y + offsetY, d3dX, d3dY);
		Vertexs[n] = Vector (d3dX, d3dY, 0);
	}

	
//	for (int r = 0; r < NumPoints; r++) Points[r].p.z = 1.0f;


	IRender* rs = GetRender();
	rs->DrawPolygon (Vertexs, NumPoints, Color (color), Matrix(), "GUIBase");
}

void GUIHelper::DrawLines (const GUIPoint* Points, int NumPoints, DWORD color, int offsetX, int offsetY)
{
	if (NumPoints > ARRSIZE(vertexs)) return;
	for (int n = 0; n < NumPoints; n++)
	{
		GUIVertex & vrt = vertexs[n];
		ScreenToD3D (Points[n].x + offsetX, Points[n].y + offsetY, vrt.p.x, vrt.p.y);
		vrt.p.z = 0.0f;
		vrt.color = color;
	}
	IRender* rs = GetRender();

	
// 	//DWORD ff_const = rs->GetFirstFreeVertexShaderConstant ();
// 	Color clr(color);
// 
// 	/* JOKER FOR HLSL
// 	rs->SetVertexShaderConstant ((ff_const+6), &clr, 1);
// 	*/
// 
// 	IVariable* pVar = rs->GetTechniqueGlobalVariable("GUI_LinesColor", _FL_);
// 	if (pVar)
// 	{
// 		pVar->SetVector4(clr.v4);
// 		pVar->Release();
// 	}
// 
// 
// //	for (r = 0; r < NumPoints; r++) Points[r].p.z = 1.0f;

	rs->DrawPrimitiveUP (shaderGUIBase, PT_LINESTRIP, (NumPoints-1), vertexs, sizeof (GUIVertex));
}

void GUIHelper::Draw2DLines (const GUIPoint* Points, int NumPoints, DWORD color, int offsetX, int offsetY)
{
	IRender* rs = GetRender();

	NumPoints &= ~1;

	while(NumPoints > 0)
	{
		dword count = NumPoints;
		if (count > ARRSIZE(vertexs))
		{
			count = ARRSIZE(vertexs);
		}
		NumPoints -= count;
		for (dword n = 0; n < count; n++)
		{
			GUIVertex & vrt = vertexs[n];
			ScreenToD3D (Points[n].x + offsetX, Points[n].y + offsetY, vrt.p.x, vrt.p.y);
			vrt.p.z = 0.0f;
			vrt.color = color;
		}
		rs->DrawPrimitiveUP (shaderGUIBase, PT_LINELIST, count/2, vertexs, sizeof (GUIVertex));

		Points += count;
	}
}

#endif
