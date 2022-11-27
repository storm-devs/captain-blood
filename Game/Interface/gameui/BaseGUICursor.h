#pragma once

#include "BaseGUIElement.h"

class BaseGUICursor : public MissionObject
{
public:

	 BaseGUICursor();
	~BaseGUICursor();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Restart();

//	void PostCreate();

	void Show	 (bool isShow);

	void SetPosition(float  x, float  y)
	{
		m_px = m_x = x;
		m_py = m_y = y; m_init = false;

		m_moved = false;

		#ifndef _XBOX

		const RENDERVIEWPORT &vp = Render().GetViewport();

		POINT pt = {
			LONG(x*vp.Width),
			LONG(y*vp.Height)};

		HWND h = (HWND)api->Storage().GetLong("system.hwnd");

		ClientToScreen(h,&pt);

		SetCursorPos(pt.x,pt.y);

		#endif
	}

	void GetPosition(float &x, float &y)
	{
		x = m_x;
		y = m_y;
	}

	bool IsHided()
	{
		return m_hided;
	}

	bool IsMoved()
	{
		return m_moved;
	}

	bool IsClicked()
	{
		return m_clicked;
	}

private:

	void _cdecl InitFn(float dltTime, long level);

	void _cdecl Update(float dltTime, long level);
	void _cdecl Redraw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

private:

	MOSafePointer cursor;

//	const char *cursorName;

	float m_px,m_x; float m_dx;
	float m_py,m_y; float m_dy;

	float m_off_x;
	float m_off_y;

	bool m_init;

	bool m_hided; float m_time;

	bool m_moved;
	bool m_drawOnPause;

	ICoreStorageLong *m_active;

	long m_lb;
	long m_rb;

	bool m_clicked;

	long m_cnt;

	long m_mh;
	long m_mv;

	bool m_nomove;

	bool m_show;
	bool m_hide;

	long m_nodraw;

};
