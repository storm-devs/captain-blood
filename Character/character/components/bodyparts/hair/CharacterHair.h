#pragma once

#include "CharacterRope.h"

//#include "..\..\..\..\..\Common_h\Mission.h"
#include "..\CharacterBlender.h"

#include "..\..\..\..\auxiliary objects\hair params\HairParams.h"

class CharacterHair : public CharacterBlender
{
	struct Place
	{
		Place(long i_ = -1, long j_ = -2) : i(i_),j(j_)
		{
		}

		long i;
		long j;
	};

	struct Collider
	{
		int a;
		int b; float x; float r; Vector p; dword c;
	};

public:

	CharacterHair();

	virtual void Init(const InitData &data);

	virtual void Update(float dltTime);
	virtual void GetBoneTransform(int i, BlendData &data);

	virtual void Draw(float dltTime);

	virtual void ResetAnimation(IAnimation *animation);

/*	void SetMatrix(const Matrix &m)
	{
		mat = m;
	}*/
	void SetMatrix(const Matrix &m);

	void SetDeltaPos(const Vector &pos)
	{
	//	dltPos = pos;
	}

private:

	void Invalidate();
	void DrawRope(CharacterRope &rope);

	void  SetParams();
	void FindParams();

private:

	array<Place> table;

	IAnimation *anime;
	IRender	   *render;

	Matrix mat;
//	Vector dltPos;

	bool started;

	Matrix currMat;
	Matrix prevMat;

	Vector curp;
	Vector prep;

	Vector apos; Vector diff;

//	long root;

	array<CharacterRope> ropes;
	array<Collider>		 colls;

	Character	 *chr;
	MOSafePointer params;

	bool empty;

	bool canUpdate;

};
