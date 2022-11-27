#ifndef _XBOX

#include "gui_cliper.h"
#include "gui_helper.h"

GUICliper::GUICliper () : stack (_FL_, 256)
{
}

GUICliper::~GUICliper ()
{
}



void GUICliper::SetRectangle (const GUIRectangle& rect)
{
	
	r = rect;
	r.w += 1;
	r.h += 1;
	IRender* rs = (IRender*)api->GetService("DX9Render");
	
	// D3DVIEWPORT8 current_vp = rs->GetViewport ();
	
	
	int ScreenWidth =  rs->GetScreenInfo3D().dwWidth;
	int ScreenHeight = rs->GetScreenInfo3D().dwHeight;
	
	int clipLeft = 0;
	int clipTop = 0;
	int clipWidth = ScreenWidth; 
	int clipHeight = ScreenHeight; 
	
	
	int vLeft = r.Left;
	int vTop = r.Top;
	int vWidth = r.Width;
	int vHeight = r.Height;
	
	int raznX = (vLeft+vWidth) - (clipLeft+clipWidth);
	if (raznX > 0) vWidth -= raznX;
	
	int raznY = (vTop+vHeight) - (clipTop+clipHeight);
	if (raznY > 0) vHeight -= raznY;
	
	
	int zaehalX = vLeft - clipLeft;
	if (zaehalX < 0) 
	{
		vWidth+=zaehalX;
		vLeft = clipLeft; 
	}
	
	int zaehalY = vTop - clipTop;
	if (zaehalY < 0) 
	{
		vHeight+=zaehalY;
		vTop = 0; 
	}
	
	
	if (vLeft > ScreenWidth) {vLeft = ScreenWidth; vWidth = 0;}
	if (vTop > ScreenHeight) {vHeight = ScreenHeight; vTop = 0;}

	if (vWidth < 0 ) vWidth = 0;
	if (vHeight < 0 ) vHeight = 0;

	RENDERVIEWPORT vp;
	vp.X = vLeft;
	vp.Y = vTop;
	vp.Width = vWidth;
	vp.Height = vHeight;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	
	
	float scaleX = (float)(ScreenWidth)  /  (float)vp.Width;
	float scaleY = (float)(ScreenHeight) /  (float)vp.Height;
	
	float pX;
	float pY;
	float mX;
	float mY;
	GUIHelper::ScreenToD3D (vp.X, vp.Y, pX, pY);
	mX = -1.0f - pX;
	mY = 1.0f - pY;
	
	Matrix m;
	m.m[0][0] = scaleX;
	m.m[1][1] = scaleY;
	float incX = pX - (pX * scaleX);
	float incY = pY - (pY * scaleY);
	mX += incX;
	mY += incY;
	
	m.pos.x = mX;
	m.pos.y = mY;

	IVariable* pVar = rs->GetTechniqueGlobalVariable("GUI_CliperMatrix", _FL_);
	if (pVar)
	{
		pVar->SetMatrix(m);
	}

	/* JOKER FOR HLSL
	m.Transposition ();
	DWORD ffc = rs->GetFirstFreeVertexShaderConstant ();
	rs->SetVertexShaderConstant (ffc, m, 4);
	*/




	if (vp.Width == 0 || vp.Height == 0) 
	{
		vp.X = 0;
		vp.Y = 0;
		vp.Width = 1;
		vp.Height = 1;
	}
	rs->SetViewport (vp);
}

void GUICliper::SetInsRectangle (const GUIRectangle& rect)
{
	SetRectangle(r.Intersection(rect));
}

const GUIRectangle& GUICliper::GetRectangle ()
{
	return r;
}

void GUICliper::SetFullScreenRect ()
{
	IRender* rs = (IRender*)api->GetService("DX9Render");
	/*
	D3DVIEWPORT8 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = rs->GetScreenInfo3D().dwWidth;
	vp.Height = rs->GetScreenInfo3D().dwHeight;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	rs->SetViewport (vp);
	*/
	
	GUIRectangle r;
	r.Width = rs->GetScreenInfo3D().dwWidth;
	r.Height = rs->GetScreenInfo3D().dwHeight;
	SetRectangle (r);
}


void GUICliper::Push ()
{
	IRender* rs = (IRender*)api->GetService("DX9Render");
	RENDERVIEWPORT vp = rs->GetViewport ();
	
	
	GUIRectangle* r_new = &stack[stack.Add ()];
	r_new->Left = vp.X;
	r_new->Top = vp.Y;
	r_new->Width = vp.Width;
	r_new->Height = vp.Height;
}

void GUICliper::Pop ()
{
	int num = stack.Size();
	if (num <= 0) return;
	num--;
	SetRectangle (stack[num]);
	//stack.Remove (num);
	stack.DelIndex(num);
}


#endif