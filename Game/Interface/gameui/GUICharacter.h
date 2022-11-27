#pragma once

#include "../../..\common_h/Mission.h"

#include "render/QuadRender.h"
#include "BaseGUIElement.h"

class GUICharacter : public BaseGUIElement
{
	struct Params
	{
		enum Align {Left,Center,Right,Top,Bottom};

		bool useSafeFrame;
	};

	RENDERVIEWPORT m_vp;

	Matrix m_view;
	Matrix m_proj;

	bool m_persp;
	bool m_debug;

	float m_fov;

	Vector m_ang;
	Vector m_pos;

	ShaderId Circular_shadow_id;
	ShaderId Circular_id;

	QuadRender m_quad;

	Params m_params;
	float  m_asp;

	bool m_restart;

	void ReadMOPs(MOPReader &reader);

	void _cdecl InitFunc(float deltaTime, long level);

	void _cdecl Begin(float deltaTime, long level);
	void _cdecl End	 (float deltaTime, long level);

	virtual void Restart();

	virtual void OnParentNotify(Notification event);

	virtual void OnAcquireFocus() {}
	virtual void OnLooseFocus()	  {}

	virtual void OnChildNotify(BaseGUIElement *p, ChildNotification event)
	{
		if( event == ChildSelected )
			NotifyParent(this,ChildSelected);
	}

	virtual void SetDrawUpdate()
	{
		SetUpdate((MOF_UPDATE)&GUICharacter::Begin,/*ML_DYNAMIC2*/ML_GEOMETRY3 - 1000);
		SetUpdate((MOF_UPDATE)&GUICharacter::End  ,/*ML_DYNAMIC2*/ML_GEOMETRY3 + 1000);
	}

	virtual void DelDrawUpdate()
	{
		DelUpdate((MOF_UPDATE)&GUICharacter::Begin);
		DelUpdate((MOF_UPDATE)&GUICharacter::End);
	}

	float SC2UIRC(float t);
	float SS2UIRS(float size);

public:

	GUICharacter(void);
	virtual ~GUICharacter(void);

	virtual bool Create			(MOPReader &reader);
	virtual bool EditMode_Update(MOPReader &reader);

	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(GUICharacter, BaseGUIElement);

	virtual void Show(bool isShow);

public:

	virtual void Draw()				   {}
	virtual void Update(float dltTime) {}

};
