#pragma once

#include "..\..\..\Common_h\Mission.h"

class Progress : public MissionObject
{
	struct Vertex
	{
		Vector p;

		float tu;
		float tv;

		float al;
	};

	ShaderId Circular_id;

public:

	 Progress();
	~Progress();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Restart();

	void Activate(bool isActive);
	void Show	 (bool isShow);

	void Command(const char *id, dword numParams, const char **params);

	void SetPos(float val);

	void MoveTo(float x, float y)
	{
		m_x = x;
		m_y = y;
	}

	void SizeTo(float w)
	{
		m_w = w;
	}

	float GetHeight()
	{
		return m_h*100.0f;
	}

private:

	void _cdecl Draw(float dltTime, long level);

	void CreateBuffer();

	void InitParams		(MOPReader &reader);

	void InitAspect();

private:

	IVariable *BarTexture;

	IBaseTexture *texture1;
	IBaseTexture *texture2;

	IVBuffer *buffer;

	float m_x;
	float m_y;

	float m_w;
	float m_h;

	long m_drawPriority;

	float m_max;
	float m_pos;

	float m_aspect;

};
