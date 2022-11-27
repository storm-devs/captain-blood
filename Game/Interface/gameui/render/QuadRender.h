#pragma once

#include "..\..\..\..\common_h/Render.h"

class IRender;
class ITexture;
class IVBuffer;
class IIBuffer;
class IVariable;
class string;

#include "..\BaseGUIElement.h"

class QuadRender
{
	IRender*	m_render;
	ITexture*	m_texture;
	IVariable*	m_texVar;

	float		m_aspect;

//	ShaderId GUI_Quad_id;
	ShaderId GUI_Quad_Black_id;

	bool m_black;

	IVariable *BlackScale;

public:

	 QuadRender(void);
	~QuadRender(void);

	void SetTexture(const char *texName, bool black = false);

	bool Loaded()
	{
		return m_texture != null;
	}

	void DrawQuad(
		float  x, float  y, float  w, float  h,
		float tu, float tv, float tw, float th,	float alpha,
		float xScaleCenter = 1.0f,
		float yScaleCenter = 1.0f, bool vertical = false, bool invert = false);

	void CalcScreenRect(
		BaseGUIElement::Rect &rect, float xScaleCenter = 1.0f, float yScaleCenter = 1.0f) const;

	void SetBlack(bool black)
	{
		m_black = black;
	}

};
