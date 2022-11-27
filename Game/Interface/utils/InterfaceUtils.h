#pragma once

//#ifndef _InterfaceUtils_h_

#include "..\..\..\common_h\mission.h"

class InterfaceUtils : public Service
{
public:
	InterfaceUtils();
	virtual ~InterfaceUtils();

	//Рисовать ли UI
	inline static bool IsHide()
	{
		if(largeshot && largeshot->Get(0) != 0)
		{
			return true;
		}
		if(hideHUD && hideHUD->Get(0) != 0)
		{
			return true;
		}
		return false;
	}

	inline static float AspectRatio(IRender & render)
	{
	//	return 1.0f;

	//	const RENDERVIEWPORT &vp = render.GetViewport(); return vp.Width/float(vp.Height);

	//	return render.IsWideScreen() ? 1.3333333333f : 1.0f;


	//JOKER:
	//return render.IsWideScreen() ? 1.0f : 1.3333333333f;
		return render.GetWideScreenAspectWidthMultipler();

	/*	const RENDERVIEWPORT &wp = render.GetFullScreenViewPort();

		bool wideScreen = render.IsWideScreen() || wp.Height/float(wp.Width) < 0.7f;

		return wideScreen ? 1.3333333333f : 1.0f;*/

	//	const RENDERVIEWPORT &vp = render.GetViewport();

	//	return vp.Width/float(vp.Height) > 1.5f ? 16.0f/9.0f : 1.0f;
	}

	inline static dword HideFieldSize(IRender & render)
	{
		return 0;
	}

	inline static float ScreenCoord2UIRectCoord(float t) // 0 <= t <= 1
	{
	#ifdef _XBOX
		/*

		const float safeZone = 0.05f; // 5% от каждой стороны экрана

		return safeZone + t*(1.0f - 2.0f*safeZone);

		*/

		return t;

	#else

		return t;

	#endif
	}

	inline static float ScreenSize2UIRectSize(float size) // 0 <= t <= 1
	{
	#ifdef _XBOX

		/*
		const float safeZone = 0.05f; // 5% от каждой стороны экрана

		return size*(1.0f - 2.0f*safeZone);
		*/

		return size;

	#else

		return size;

	#endif
	}

	inline static const ConstString & GetCursorName()
	{
		static const ConstString cursorName("Cursor");
		return cursorName;
	}

	inline static const ConstString & GetBaseId()
	{
		static const ConstString baseId("BaseGUIElement");
		return baseId;
	}

	
	inline static const ConstString & GetButtonId()
	{
		static const ConstString buttonId("GUIButton");
		return buttonId;
	}

	inline static const ConstString & GetSliderId()
	{
		static const ConstString sliderId("GUISlider");
		return sliderId;
	}

	inline static const ConstString & GetPbarId()
	{
		static const ConstString pbarId("csProgressBar");
		return pbarId;		
	};

	inline static const ConstString & GetWidgetId()
	{
		static const ConstString widgetId("GUIWidget");
		return widgetId;		
	};

	

	// заменяет пробелы на '\n' в местах где надо сделать перенос
	static dword WordWrapString(string &str, IFont *font, float outputPixelWidth);
	static dword WordWrapString(char text[], IFont *font, float outputPixelWidth, dword textMaxSize);
	
	static ICoreStorageLong * largeshot;
	static ICoreStorageLong * hideHUD;
};

//#endif
