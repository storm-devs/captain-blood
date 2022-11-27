#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\IGUIElement.h"

class ObjectTip : public MissionObject
{
	struct TipInfo
	{
		ConstString targetName;
		ConstString widgetName;

		MOSafePointer target;
		MOSafePointer widget;

		Vector off;

		float x,y;
		float w,h;

		IGUIElement::Layout layout;
	};

public:

	 ObjectTip();
	~ObjectTip();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Restart();

	void Command(const char *id, dword numParams, const char **params);

	void PostCreate();

private:

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

private:

	array<TipInfo> tips;

	bool scale;
	bool debug;

	bool hmirr;

	bool mshow;

};
