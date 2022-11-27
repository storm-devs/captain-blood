//============================================================================================
// Spirenkov Maxim, 2007
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationMoveClip
//============================================================================================

#ifndef _NodeAnimationMoveClip_h_
#define _NodeAnimationMoveClip_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeAnimation.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxToolListBox.h"

class NodeAnimationMoveClip : public GUIWindow
{
//--------------------------------------------------------------------------------------------
public:
	NodeAnimationMoveClip(AnxOptions & options, AntFile & _ant);
	~NodeAnimationMoveClip();

//--------------------------------------------------------------------------------------------
public:
	virtual void Draw();
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtReadBoneTransform(GUIControl * sender);



public:
	GUIEventHandler* t_OnOK;
	GUIControl * t_OnOK_Sender;

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	AntFile & ant;

	AnxToolEdit * frameNumber;
	AnxToolEdit * posX;
	AnxToolEdit * posY;
	AnxToolEdit * posZ;
	AnxToolEdit * angX;
	AnxToolEdit * angY;
	AnxToolEdit * angZ;

	long frame;
	Vector pos, ang;


	AnxToolButton * buttonOk;
	AnxToolButton * buttonCancel;
};

#endif

