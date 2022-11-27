#pragma once

#include "..\..\..\Common_h\Mission.h"

class CircularBar : public MissionObject
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

	 CircularBar();
	~CircularBar();

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
		m_x = x*0.01f;
		m_y = y*0.01f;
	}

private:

	void _cdecl Draw(float dltTime, long level);

	void CreateBuffer();

	void InitParams		(MOPReader &reader);

	void InitAspect();

private:

	IVariable	 *BarTexture;
	IBaseTexture *texture;

	IVBuffer *buffer;

	float m_x;
	float m_y;

	float m_w;
	float m_h;

	long m_drawPriority;

	float m_inner;
	float m_outer;

	float m_max;
	float m_pos;

	float m_aspect;

};
