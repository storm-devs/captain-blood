#pragma once

#include "..\..\..\..\..\Common_h\Core.h"
#include "..\..\..\..\..\Common_h\Math3D\Vector.h"

class CharacterRope
{
	struct Node
	{
		Vector p; float len; float inv;
		Vector q;

		Vector v;
		Vector a;

		Vector f;

		Vector bonePos;

		Matrix m;

		Vector n;
	};

	struct Sphe
	{
		Vector p; float r; dword c;

		Sphe()
		{
			r = 0.0f;
		}
	};

public:

	CharacterRope() :
		nodes(_FL_),
		sphes(_FL_)
	{
	}

	void Create(Vector pos, int n, Vector to);
	void Create(Vector pos,		   Vector to, bool reset = true);

//	void Add(float len);
	void Add(float len, const Vector &p);

	void SetSegmentLen(int i, float len)
	{
		nodes[i].len = len;
		nodes[i].inv = 1.0f/len;
	}

	void SetSegmentLen(int i, float len, const Vector &p)
	{
		nodes[i].len = len;
		nodes[i].inv = 1.0f/len;

		nodes[i].p = p;
		nodes[i].q = p;
	}

	void Beg(float dltTime);
	void End(float dltTime);

	operator int()
	{
		return nodes;
	}

	const Vector &operator [](int i)
	{
		return nodes[i].p;
	}

	float GetSegLen(int i)
	{
		return nodes[i].len;
	}

	const Vector &GetBonePos(int i)
	{
		return nodes[i].bonePos;
	}

	void SetBonePos(int i, const Vector &p)
	{
		nodes[i].bonePos = p;
	}

	const Vector &GetOrigin()
	{
		return origin;
	}

	const Vector &GetDir()
	{
		return dir;
	}

	void SetOrigin(const Vector &v)
	{
		origin = v;
	}

	void SetDir(const Vector &v)
	{
		dir = v;
	}

	void SetK(float k)
	{
		m_k = k;
	}

	void SetM(float m)
	{
		m_m = m;
	}

	void SetN(float n)
	{
		m_n = n;
	}

	const Vector &GetForce(int i)
	{
		return nodes[i].a;
	}

	void SetMatrix(int i, const Matrix &m)
	{
		if( i < 0 )
			mat = m;
		else
			nodes[i].m = m;
	}

	void SetNormal(int i, const Vector &n)
	{
		if( i < 0 )
			nor = n;
		else
			nodes[i].n = n;
	}

	const Matrix &GetMatrix(int i)
	{
		if( i < 0 )
			return mat;
		else
			return nodes[i].m;
	}

	const Vector &GetNormal(int i)
	{
		if( i < 0 )
			return nor;
		else
			return nodes[i].n;
	}

	void SetGlobal(const Matrix &m)
	{
		glo = m;
	}

private:

	Matrix mat;
	Vector nor;

	Matrix glo;

	Vector origin;
	Vector dir;

	float m_k;
	float m_m;

	array<Node> nodes;

	float m_n;

public:

	void Collide(CharacterRope &rope);

public:

	long m_a;
	long m_b;

	long m_t;

public:

	array<Sphe> sphes;

	void AddSphere(int n = 1)
	{
		for( int i = 0 ; i < n ; i++ )
			sphes.Add();
	}

	void SetSphere(int i, Vector p, float r, dword c)
	{
		sphes[i].p = p;
		sphes[i].r = r;
		sphes[i].c = c;
	}

};
