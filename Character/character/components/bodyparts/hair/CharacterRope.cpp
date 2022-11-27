#include "CharacterRope.h"
/*
#include "..\..\..\Character.h"
#include "..\..\..\Components\CharacterPhysics.h"
*/
void CharacterRope::Create(Vector pos, int n, Vector to)
{
	nodes.DelAll();

	origin = pos;

	float len;

	dir = (to - pos)/float(n);
	len = dir.GetLength();

	Vector v = dir;
	Vector p = pos;

	dir.Normalize();

	for( int i = 0 ; i < n ; i++ )
	{
		Node &node = nodes[nodes.Add()];

		node.len = len;
		node.inv = 1.0f/len;

		node.p = p += v;
		node.q = p;

		node.v = 0.0f;

		node.f = 0.0f;
	}

	m_k = 1.0f;
	m_m = 1.0f;

	m_n = 0.0f;
}

void CharacterRope::Create(Vector pos, Vector to, bool reset)
{
	if( reset )
		nodes.DelAll();

	origin = pos;

	dir = to;

	m_k = 1.0f;
	m_m = 1.0f;

	m_n = 0.0f;
}
/*
void CharacterRope::Add(float len)
{
	Vector p = nodes ? nodes.LastE().p : origin;

	Node &node = nodes[nodes.Add()];

	node.len = len;
	node.inv = 1.0f/len;

	node.p = p + dir*len;
	node.q = p;

	node.v = 0.0f;

	node.f = 0.0f;
}
*/
void CharacterRope::Add(float len, const Vector &p)
{
	Node &node = nodes[nodes.Add()];

	node.len = len;
	node.inv = 1.0f/len;

	node.p = p;
	node.q = p;

	node.v = 0.0f;

	node.f = 0.0f;
}

void CharacterRope::Beg(float dltTime)
{
	if( dltTime > 0.03f )
		dltTime = 0.03f;

	float inv_m = 1.0f/m_m;

	Vector fe = 0.0f;

//	;
	Matrix m;

	for( int i = nodes - 1 ; i >= 0  ; i-- )
	{
		Node &node = nodes[i];

		float len = node.len;
		float inv = node.inv;

		const Vector &b = i ? nodes[i - 1].p : origin;

	//	Vector d = node.p - b;

		Vector n  = i > 1 ? nodes[i - 1].p - nodes[i - 2].p : (i > 0 ? nodes[i - 1].p - origin : dir);

		float inl = i > 1 ? nodes[i - 1].inv : 1.0f/n.GetLength();

		n *= inl;

		m = GetMatrix(i - 1);

		m *= glo;

		m.pos = i > 0 ? nodes[i - 1].p : (i < 0 ? origin - dir : origin);

		Vector n_ = m.MulNormal(GetNormal(i - 1));

		n.Lerp(n,n_,m_n);

	//	d.Normalize();
	//	n.Normalize();

	//	float idl = i ? inv : 1.0f/d.GetLength();
	//	float inl = i > 1 ? nodes[i - 1].inv : 1.0f/n.GetLength();

	//	d *= idl;

	//	n *= inl;
		;

		const Vector g(0.0f,-15.0f,0.0f);
	
		Vector fg = g*(m_m*len*15.0f);

		Vector v = b + n*len - node.p;

	/*	float ln = (1.0f - (n|d));

		v.Normalize();
		v *= ln;*/

	//	float lk = 0.5f*inv*inv;
		float lk = 0.3f*inv*inv;

		v.x = v.x*fabsf(v.x)*lk;
		v.y = v.y*fabsf(v.y)*lk;
		v.z = v.z*fabsf(v.z)*lk;

		Vector fk = v*m_k;

	//	Vector r = d;
		Vector r = node.p - b;

		float irl = i ? inv : 1.0f/r.GetLength();

		r *= irl;
	
		Vector fs = fg + fk + fe;

		fs += node.f;

		node.a = fs*inv_m;

		node.q = node.p;
		node.p = node.p + node.v*dltTime + node.a*(dltTime*dltTime*0.5f);

		node.a = fk*(0.01f*inv_m);

		fe = r*((fg + fe)|r) - fk*0.25f;

		node.f = 0.0f;
	}
}

void CharacterRope::End(float dltTime)
{
//	if( dltTime > 0.03f )
//		dltTime = 0.03f;

	float inv_dltTime = 1.0f/dltTime;

	for( int i = 0 ; i < nodes ; i++ )
	{
		Node &node = nodes[i];

		for( int j = 0 ; j < sphes ; j++ )
		{
			Sphe &sphe = sphes[j];

			Vector d = node.p - sphe.p;

			float dl = d.x*d.x + d.y*d.y + d.z*d.z;

			if( dl < sphe.r*sphe.r )
			{
				dl = sqrtf(dl);

				node.p = sphe.p + d*(sphe.r/dl);
			}
		}
	}

/*	Vector ts = 0.0f;

	for( int i = 0 ; i < nodes - 1 ; i++ )
	{
		nodes[i].p += ts;

		const Vector &b = i ? nodes[i - 1].p : origin;

		Vector p; p.Lerp(b,nodes[i + 1].p,nodes[i].len/(nodes[i].len + nodes[i + 1].len));
		Vector q; q.Lerp(nodes[i].p,p,0.02f);

		Vector t = p - b; t.Normalize();

		t *= (q - nodes[i].p).GetLength();

		nodes[i].p = q;

	//	for( int j = i + 1 ; j < nodes ; j++ )
	//		nodes[j].p += t;

		ts += t;
	}*/

	for( int i = 0 ; i < nodes ; i++ )
	{
		Node &node = nodes[i];

		float len = node.len;

		const Vector &b = i ? nodes[i - 1].p : origin;

		Vector n = node.p - b;
			   n.Normalize();

		Vector p = b + n*len;

		Vector v = (p - node.q)*inv_dltTime;

		float d = v.x*v.x + v.y*v.y + v.z*v.z;

		if( d > 1.0f )
		{
			v.Normalize();
		}

		node.v.Lerp(node.v,v*0.8f,0.3f);

		Vector q = p - node.p;

		for( int j = i + 1 ; j < nodes ; j++ )
		{
			q *= 0.25f;

			nodes[j].p += q;
		}

		node.p = p;
	}
}

void CharacterRope::Collide(CharacterRope &rope)
{
	const float r  = 0.02f;
	const float r2 = r*r;

	const float R  = 0.05f;
	const float R2 = R*R;
	const float invR2 = 1.0f/R2;

	const float eps = 0.00003f;

	Assert(R2 > r2 && r2 > eps);

	for( int i = 0 ; i < nodes ; i++ )
	{
		for( int j = 0 ; j < rope.nodes ; j++ )
		{
			Vector d = rope.nodes[j].p - nodes[i].p;

			float dl = d.x*d.x + d.y*d.y + d.z*d.z;

			if( dl < R2 )
			{
				if( dl <  eps )
					continue;

				if( dl >= r2 )
				{
					float k = (1.0f - dl*invR2);

					//	k = k*100.0f;
					k = k*400.0f*k;

					Vector f = d*k;

					nodes[i].f -= f;
					rope.nodes[j].f += f;
				}else{
					float kdl = 1.0f/sqrtf(dl);

					Vector z = nodes[i].p + d*0.5f;
					Vector t = d*(r*0.5f*kdl);

					nodes[i].p = z - t;
					rope.nodes[j].p = z + t;

					/*	if( _isnan(		nodes[i].p.x) ||
					_isnan(rope.nodes[j].p.x))
					Assert(0)*/
				}
			}
		}
	}
}
