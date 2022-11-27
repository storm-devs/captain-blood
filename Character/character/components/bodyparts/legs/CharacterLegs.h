#pragma once

#include "..\CharacterBlender.h"

class CharacterLegs : public CharacterBlender
{
	struct LegInfo
	{
		long th;
		long kn;
		long fo;
		long to;

		Matrix tho;
		Matrix kno;
		Matrix foo;

		Matrix thn;
		Matrix knn;
		Matrix fon;

		bool found; Vector pf;

		float k;
	};

public:

	 CharacterLegs();
	~CharacterLegs();

	virtual void Init(const InitData &data);

	virtual void Update(float dltTime);
	virtual void GetBoneTransform(int i, BlendData &data);

	virtual float GetBoneBlend(long boneIndex);

	virtual void ResetAnimation(IAnimation *animation);

private:

	void Invalidate();

	void UpdateLeg(float dltTime, LegInfo &leg);

private:

	IAnimation *ani;

	LegInfo lleg;
	LegInfo rleg;

	float rdy; float pdy;
	float cdy;

	float moved;

	Vector lfd,lfc;
	Vector rfd,rfc;

	float upd;

	bool enabled;
	bool key;

	Matrix prev;

	float blend;

public:

	void setEnabled(bool en);

};
