#include "QuadRender.h"

#include "..\..\..\..\Common_h\Render.h"
#include "..\..\Utils\InterfaceUtils.h"

static inline float SC2UIRC(float t)
{
	return InterfaceUtils::ScreenCoord2UIRectCoord(t);
}

static inline float SS2UIRS(float t)
{
	return InterfaceUtils::ScreenSize2UIRectSize(t);
}

QuadRender::QuadRender(void)
{
	m_render = null;

	m_aspect = 4.0f/3.0f;

	m_texture = null;
	m_texVar  = null;

	BlackScale = null;

	m_render = (IRender*)api->GetService("DX9Render");
	Assert(m_render);

//	m_render->GetShaderId("interfaceQuad", GUI_Quad_id);
	m_render->GetShaderId("interfaceQuad_Black", GUI_Quad_Black_id);

	m_aspect = m_render->GetScreenInfo2D().dwHeight/(float)m_render->GetScreenInfo2D().dwWidth;

	m_texVar = m_render->GetTechniqueGlobalVariable("interfaceTexture",_FL_);

	BlackScale = m_render->GetTechniqueGlobalVariable("interfaceBlackScale",_FL_);
}

QuadRender::~QuadRender(void)
{
	if( m_texture )
	{
		m_texture->Release();
		m_texture = null;
	}

	m_texVar = null;
	BlackScale = null;
}

void QuadRender::SetTexture(const char *texName, bool black)
{
	IBaseTexture *oldTex = m_texture;
	
//	m_texture = (ITexture*)m_render->CreateTexture(_FL_,texName);
	m_texture = (ITexture*)m_render->CreateTextureFullQuality(_FL_,texName);

	if( oldTex )
		oldTex->Release();

	m_black = black;
}

void QuadRender::CalcScreenRect(BaseGUIElement::Rect &rect, float xScaleCenter, float yScaleCenter) const
{
	float x = rect.l;
	float y = rect.t;

	float w = rect.r - rect.l;
	float h = rect.b - rect.t;

	x -= (w*xScaleCenter - w)*0.5f;
	y -= (h*yScaleCenter - h)*0.5f;

	w *= xScaleCenter;
	h *= yScaleCenter;

	x = SC2UIRC(x*0.01f);
	y = SC2UIRC(y*0.01f);

	float x1 = x + SS2UIRS(w*0.01f);
	float y1 = y + SS2UIRS(h*0.01f);
	
	rect.l = x *m_render->GetViewport().Width  + m_render->GetViewport().X;
	rect.t = y *m_render->GetViewport().Height + m_render->GetViewport().Y;
	rect.r = x1*m_render->GetViewport().Width  + m_render->GetViewport().X;
	rect.b = y1*m_render->GetViewport().Height + m_render->GetViewport().Y;
}

void QuadRender::DrawQuad(
	float  x, float  y, float  w, float  h,
	float tu, float tv, float tw, float th,	float alpha,
	float xScaleCenter,
	float yScaleCenter, bool vertical, bool invert)
{
	x -= (w*xScaleCenter - w)*0.5f;
	y -= (h*yScaleCenter - h)*0.5f;

	w *= xScaleCenter;
	h *= yScaleCenter;

	x = /*SC2UIRC*/(x*0.01f);
	y = /*SC2UIRC*/(y*0.01f);

	float x1 = x + /*SS2UIRS*/(w*0.01f);
	float y1 = y + /*SS2UIRS*/(h*0.01f);

	float vb[] = {
		x *2.0f - 1.0f,1.0f - y *2.0f,1.0f,alpha,tu		,tv,
		x1*2.0f - 1.0f,1.0f - y *2.0f,1.0f,alpha,tu + tw,tv,
		x *2.0f - 1.0f,1.0f - y1*2.0f,1.0f,alpha,tu		,tv + th,
		x1*2.0f - 1.0f,1.0f - y1*2.0f,1.0f,alpha,tu + tw,tv + th};

/*	if( vertical )
	{
		if( invert )
		{
			vb[ 5] += th;
			vb[10] -= tw;
			vb[16] += tw;
			vb[23] -= th;
		}
		else
		{
			vb[ 4] += tw;
			vb[11] += th;
			vb[17] -= th;
			vb[22] -= tw;
		}
	}*/

	if( m_texVar )
		m_texVar->SetTexture(m_texture ? m_texture : m_render->getWhiteTexture());

	if( BlackScale )
		BlackScale->SetFloat(m_black ? 1.0f : 0.0f);

//	m_render->DrawPrimitiveUP(
//		m_black ? GUI_Quad_Black_id : GUI_Quad_id,PT_TRIANGLESTRIP,2,vb,6*sizeof(float));
	m_render->DrawPrimitiveUP(
		GUI_Quad_Black_id,PT_TRIANGLESTRIP,2,vb,6*sizeof(float));
}
