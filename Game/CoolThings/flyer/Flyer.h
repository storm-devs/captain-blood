#pragma once

#include "..\..\..\Common_h\Mission.h"

class Flyer : public MissionObject
{
	struct Mode
	{
		IGMXScene *model;
		ISound3D  *sound;

		 Mode()
		{
			model = null;
			sound = null;
		}

		~Mode()
		{
			if( model )
				model->Release();

			if( sound )
				sound->Release();
		}
	};

public:

	 Flyer();
	~Flyer();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Restart();

	void Activate(bool isActive);
	void Show	 (bool isShow);

private:

	void BuildTrack();

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

private:

	array<Mode> modes;

	float minDelay;
	float maxDelay;

	float delay;

	Vector pos;

	Vector beg;
	Vector end;

	float off;
	float fre;
	float rad;

	float time; bool run;

	ISound3D *sound;
	ISound3D *curSound;

	int curMode;

	Vector lastPos;

	bool m_show;

};
