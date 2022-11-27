#pragma once

#include "..\..\SndOptions.h"

#include "..\..\..\..\common_h\Mission.h"

class PreviewAnimation : public GUIControl
{
	class Viewer : public RegObject
	{
		IRender *render;

		RENDERVIEWPORT vp1;
		RENDERVIEWPORT vp2;

		PreviewAnimation *preview;

	public:

		Viewer()
		{
			render = null;
		}

		void Init(IRender *r, RENDERVIEWPORT &vp, PreviewAnimation *p)
		{
			render = r; vp1 = vp; preview = p;
		}

		void Reset()
		{
			render = null;
		}

		void Beg(float);
		void End(float);

	};

	Viewer viewer;

	struct ToolBar
	{
		GUIRectangle r;
	};

	ToolBar bar;

	struct LocInfo
	{
		const char *tag;	// префикс для локатора

		dword beg;			// первое вхождение в таблице
		dword end;			// индекс для следующего чара

		IAnimation *ani;	// анимация персонажа
		bool		run;	// играть граф
	};

	class Transform
	{
	public:
		Transform(IGMXScene * s, GMXHANDLE hdl);
		Transform(MissionObject * mo);
		~Transform();
		Matrix GetWorldTransform();
		const char * GetName();
		
	private:
	public:	
		IGMXScene * scene;
		GMXHANDLE handle;
		MissionObject * p;
	};

public:

	PreviewAnimation(GUIControl *parent, const GUIRectangle &rect);
	virtual ~PreviewAnimation();

	IAnimation *Init(const char *misName, const char *aniName);

	void DrawBar();

	////

	void ActivateTriggers();

	void CharacterReset(bool isActivateTrigger = true);
	void CharacterRotate(float ay);

	void LockEvents(bool lock);

	void Pause(bool enable)
	{
		pause = enable;
	}

	void Delta(float dt)
	{
		deltaTime = dt;
	}

public:

	virtual void Draw()
	{
		if( !Visible )
			return;

		GUIRectangle r = GetDrawRect();
		ClientToScreen(r);

		OnDraw(r);
	}

	virtual void MouseDown(int button, const GUIPoint &pt);
	virtual void MouseUp  (int button, const GUIPoint &pt);

	virtual void MouseMove(int button, const GUIPoint &pt);

	//Рисование
	virtual void OnDraw(const GUIRectangle &screenRect);

	void GetListenerParameters(Vector & pos, Vector & dir);

private:

	void ReadTable(IGMXScene * globalModel);	// сформировать список локаторов для дополнительных чаров

private:

	class IMission *mission;

	string missionName;

	bool misOk;			// миссия   загружена
	bool aniOk;			// анимация подключена

	IAnimation *ani;

	ExtName misErr;
	ExtName aniErr;

	bool initService;

	array<Transform *> locators;

	HashTable<int, 1024, 256> locatorHash;

	int locator;	// выбранный локатор

	array<LocInfo> info;

	float blend;	// время бленда для перехода между нодами

	float deltaTime;

	GUIPoint point;

	bool leftDown;	// нажата левая кнопка мыши

	bool pause;		// находимся в режиме паузы

	bool drawSound;	// рисовать отладочную информацию для звуков

	bool modelRot;	// разрешить вращение модельки чарактера мышкой

	class IPreviewService *service;

public:

	FormButton *bBegin;
	FormButton *bEnd;

	FormButton *bNodeBegin;
	FormButton *bNodeEnd;

	FormButton *bPrev;
	FormButton *bNext;

	FormButton *bPlay;
	FormButton *bPlayNode;

//	FormButton *bPause;
	FormButton *bStop;

	FormButton *bSound;

public:

	void _cdecl Sound(GUIControl *sender);

	GUIEventHandler onUpdateSounds;

public:

	float m_time;

public:

	const string &GetMissionName()
	{
		return missionName;
	}

	const array<Transform *> &GetLocators()
	{
		return locators;
	}

	dword GetLocatorsCount()
	{
		return locators.Size();
	}

/*	const char *GetLocatorName(dword index)
	{
		return locators[index]->GetName();
	}*/
	const char *GetLocatorName(dword index, bool reset = false);

	void SelectLocator(int index)
	{
	//	Assert(index < locators)

		locator = index;
	}

	void SelectLocator(const char *name)
	{
		if( string::NotEmpty(name))
		{
			locator = locatorHash.FindObj(name, -1);
		}else{
			locator = -1;
		}			
	}

	int GetLocator()
	{
		return locator;
	}

	Matrix GetLocatorMatrix(int index)
	{
		Assert(index < locators)
		Assert(index >= 0)

		return locators[index]->GetWorldTransform();
	}

	int GetLocatorIndex(const char *name)
	{
		int index = locatorHash.FindObj(name, -1);

	//	Assert(index >= 0)

		return index;
	}

	float GetBlend()
	{
		return blend;
	}

public:

	void SetCharactersGraphInfo(struct IAnimation::GraphNativeAccessor &gna);

//	void SetIdle();

};
