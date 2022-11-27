#pragma once

#include "..\..\..\Common_h\Mission.h"

#include "..\GameUI\GUIWidget.h"

class ComboPreview : public MissionObject
{
	struct Node
	{
		float beg;
		float len;
	};

	struct Combo
	{
		const char *seq;

		array<Node> nodes;

		Combo() : nodes(_FL_)
		{
		}
	};

public:

	 ComboPreview();
	~ComboPreview();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Restart();

	void Activate(bool isActive);
	void Show	 (bool isShow);

	void Command(const char *id, dword numParams, const char **params);

private:

	void Reset();

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);
	void InitTimeLine();

	void InitAspect();

	MissionObject *FindObject(const char *id)
	{
		MOSafePointer sp;

		Mission().FindObject(ConstString(id),sp);

		return sp.Ptr();
	}

private:

	IAnimation *anime;

	string combo;
	string model; const char *modelName;

	DWORD n; float time; float tt; float tl; float tlp;

	float cirWidth; float cirHeight;
	float btnWidth; float btnHeight;
	float marWidth; float marHeight;

	GUIWidget *info; float itime;
	GUIWidget *comn; float ntime;

	bool started;

	array<Combo> combos; int curCombo;

	bool canMove;

	string curNode; bool moved;

	long startCombo;

	MissionTrigger eventWin;
	MissionTrigger eventLose;

	float aspect;

};
