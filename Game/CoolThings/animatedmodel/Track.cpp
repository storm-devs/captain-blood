#include "Track.h"

void Node::Init(MOPReader &reader)
{
	Vector node_pos = reader.Position();

	//

	pos = node_pos;

	Matrix m(reader.Angles());
	Matrix n = m;

	float k = reader.Float();

	normal	= n.vy;
	vx		= n.vx;

	tangent  = n.vz*2.5f;
	tangent *= k;

	binormal = -tangent;

	za = Deg2Rad(reader.Float());
	zt = Deg2Rad(reader.Float());
	zl =		 reader.Float();

	ya = Deg2Rad(reader.Float());
	yt = Deg2Rad(reader.Float());
	yl =		 reader.Float();

	xa = Deg2Rad(reader.Float());
	xt = Deg2Rad(reader.Float());
	xl =		 reader.Float();

	sc =		 reader.Float();
	st = Deg2Rad(reader.Float());
	sl =		 reader.Float();

	const char *type = reader.Enum().c_str();

	switch( type[0] )
	{
		case 'A':
			moveMode = move_arrive;
			break;
		case 'W':
			moveMode = move_wait;
			break;
		case 'L':
			moveMode = move_linear;
			break;
		case 'S':
			moveMode = move_step;
			break;
	}

	time = reader.Float();

	smooth = reader.Float();

	node = reader.String().c_str();

//	realNode = node;

	blendTime = reader.Float();

	event.Init(reader);
}

void Track::DrawPath(dword i1, dword i2, IRender &render, Matrix &matWorld, bool showRotation, bool showScale)
{
	render.FlushBufferedLines();

	Node &node = nodes[i1];
	Node &next = nodes[i2];

	Vector a;
	Vector b;

	if( showRotation )
	{
		a = node.zp;

		for( dword i = 0 ; i < node.bakes.Size() ; i++ )
		{
			b = node.bakes[i].zp;

			render.DrawBufferedLine(a*matWorld,0x900000ff,b*matWorld,0x90ffffff);

			a = b;
		}

		render.DrawBufferedLine(a*matWorld,0x900000ff,next.zp*matWorld,0x90ffffff);

		a = node.yp;

		for( dword i = 0 ; i < node.bakes.Size() ; i++ )
		{
			b = node.bakes[i].yp;

			render.DrawBufferedLine(a*matWorld,0x90008f00,b*matWorld,0x90ffffff);

			a = b;
		}

		render.DrawBufferedLine(a*matWorld,0x90008f00,next.yp*matWorld,0x90ffffff);

		a = node.xp;

		for( dword i = 0 ; i < node.bakes.Size() ; i++ )
		{
			b = node.bakes[i].xp;

			render.DrawBufferedLine(a*matWorld,0x90ff0000,b*matWorld,0x90ffffff);

			a = b;
		}

		render.DrawBufferedLine(a*matWorld,0x90ff0000,next.xp*matWorld,0x90ffffff);
	}

	if( showScale )
	{
		a = node.sp;

		for( dword i = 0 ; i < node.bakes.Size() ; i++ )
		{
			b = node.bakes[i].sp;

			render.DrawBufferedLine(a*matWorld,0x90000000,b*matWorld,0x90ffffff);

			a = b;
		}

		render.DrawBufferedLine(a*matWorld,0x90000000,next.sp*matWorld,0x90ffffff);
	}

	a = node.pos;

	for( dword i = 0 ; i < node.bakes.Size() ; i++ )
	{
		b = node.bakes[i].pos;

		render.DrawBufferedLine(a*matWorld,0x90ff0000,b*matWorld,0x90ffffff);

		a = b;
	}

	render.DrawBufferedLine(a*matWorld,0x90ff0000,next.pos*matWorld,0x90ffffff);

	render.FlushBufferedLines();
}

void Track::GetNodeMatrix(Matrix &m, Node &node, Matrix &matWorld, Matrix &matAngle, float modelScale)
{
	m.BuildScale(modelScale*node.sc);

	Matrix n;

	if( lookMode >= look_follow )
	{
		Vector node_tangent = node.tangent*matAngle;

		if( lookMode != look_full )
			n.BuildOrient(node_tangent.GetXZ(),Vector(0.0f,1.0f,0.0f));
		else
			n.BuildOrient(node_tangent,node.normal*matAngle);
	}

	m.Rotate(		0,node.ya,		 0);
	m.Rotate(-node.xa,		0,		 0);
	m.Rotate(		0,		0,-node.za);

	m = m*n;

	if( lookMode == look_static )
	{
		m = m*matAngle;
	}

	m.Move(node.pos*matWorld);
}

Vector CubicBezier(Vector &p0, Vector &p1, Vector &p2, Vector &p3, float mu)
{
	Vector a,b,c,p;

	c = 3.0f*(p1 - p0);
	b = 3.0f*(p2 - p1) - c;
	a = p3 - p0 - c - b;

	p = a*mu*mu*mu + b*mu*mu + c*mu + p0;

	return p;
}
/*
float Divide(float base, const Vector &p, array<Bake> &bakes, const Vector &p0, const Vector &p1, const Vector &p2, const Vector &p3)
{
	float d = (p3 - p0).GetLength();

	if( d <= 0.2f )
		return d;

	Vector a = p1 - p0; a.Normalize();
	Vector b = p3 - p2; b.Normalize();

	Vector n = p3 - p0; n.Normalize();

	float k0 = a|n;
	float k1 = b|n;

	if( k0 > 0.0f && k1 > 0.0f && k0*k1 > 0.98f )
		return d;

	Vector p4 = (p0 + p1)*0.5f;
	Vector p5 = (p1 + p2)*0.5f;
	Vector p6 = (p2 + p3)*0.5f;
	Vector p7 = (p4 + p5)*0.5f;
	Vector p8 = (p5 + p6)*0.5f;
	Vector p9 = (p7 + p8)*0.5f;

	float time = Divide(base,p,bakes,p0,p4,p7,p9);

	Bake bake;

	bake.pos  = p9;
	bake.time = base + time;
	bake.look = bake.pos - (bakes ? bakes.LastE().pos : p);

	bakes.Add(bake);

	return time + Divide(bake.time,p,bakes,p9,p8,p6,p3);
}
*/
float Divide(float base, const Vector &p, array<Bake> &bakes, const Vector &p0, const Vector &p1, const Vector &p2, const Vector &p3)
{
		   Vector p4; p4 = (p0 + p1)*0.5f;
	static Vector p5; p5 = (p1 + p2)*0.5f;
		   Vector p6; p6 = (p2 + p3)*0.5f;
	static Vector p7; p7 = (p4 + p5)*0.5f;
		   Vector p8; p8 = (p5 + p6)*0.5f;
		   Vector p9; p9 = (p7 + p8)*0.5f; static Vector n;

	//// left node ////
	
	n = p9 - p0;

	float time = n|n; // time*time

	if( time <= 0.04f )
	{
		time = sqrtf(time);
	}
	else
	{
		n.Normalize();

		Vector a = p4 - p0; a.Normalize();
		Vector b = p9 - p7; b.Normalize();

		float k0 = a|n;
		float k1 = b|n;

		if( k0 > 0.0f && k1 > 0.0f && k0*k1 > 0.98f )
		{
			time = sqrt(time);
		}
		else
		{
			time = Divide(base,p,bakes,p0,p4,p7,p9);
		}
	}

	///////////////////

	Bake bake;

	bake.pos  = p9;
	bake.time = base + time;
	bake.look = bake.pos - (bakes ? bakes.LastE().pos : p);

	bakes.Add(bake);

	//// right node ////

	n = p3 - p9;

	float next = n|n; // next*next

	if( next <= 0.04f )
	{
		next = sqrtf(next);
	}
	else
	{
		n.Normalize();

		Vector a = p8 - p9; a.Normalize();
		Vector b = p3 - p6; b.Normalize();

		float k0 = a|n;
		float k1 = b|n;

		if( k0 > 0.0f && k1 > 0.0f && k0*k1 > 0.98f )
		{
			next = sqrt(next);
		}
		else
		{
			next = Divide(bake.time,p,bakes,p9,p8,p6,p3);
		}
	}

	////////////////////

	return time + next;
}

void Track::CreateData()
{
	float stp = 1.0f/40;

	for( long j = 0 ; j < nodes ; j++ )
	{
		Node &node = nodes[j];
		Node &next = j < nodes - 1 ? nodes[j + 1] : nodes[0];

	/*	//// расчет длины сегмента ////

		{
			Vector ba,bb,bc,bd;

			ba = node.pos; bb = ba + node.tangent;
			bd = next.pos; bc = bd + next.binormal;

			Vector prev(node.pos);
			Vector next;

			node.len = 0;

			for( float t = stp ; t <= 1.0f ; t += stp )
			{
				next = CubicBezier(ba,bb,bc,bd,t);

				node.len += (next - prev).GetLength();

				prev = next;
			}
		}*/

		Vector xz = node.normal^node.tangent;
		Vector yz = node.normal;

		xz.Normalize();
		yz.Normalize();

		node.zp = node.pos + xz*node.za;
		node.yp = node.pos + xz*node.ya;

		node.xp = node.pos + yz*node.xa;
		node.sp = node.pos + yz*node.sc;

		//// создание списка подузлов ////

		float delta = 0.2f;

		stp = 0.001f;

		float len = 0;
		float t = stp;

		array<Bake> &bakes = node.bakes;

		Vector ba,bb,bc,bd;

		ba = node.pos; bb = ba + node.tangent;
		bd = next.pos; bc = bd + next.binormal;

		Matrix mat; mat.BuildOrient(node.tangent,node.normal);

		Vector tg,bi;

		Vector za,zb,zc,zd;

		tg = Vector(cosf(node.zt)*node.zl,sinf(node.zt)*node.zl,0.0f);
		bi = Vector(cosf(next.zt)*next.zl,sinf(next.zt)*next.zl,0.0f);

		za = Vector(0.0f,node.za,0.0f); zb = za + tg;
		zd = Vector(0.0f,next.za,0.0f); zc = zd - bi;

		node.ztg = Vector(tg.y,0.0f,tg.x)*mat;

		Vector ya,yb,yc,yd;

		tg = Vector(cosf(node.yt)*node.yl,sinf(node.yt)*node.yl,0.0f);
		bi = Vector(cosf(next.yt)*next.yl,sinf(next.yt)*next.yl,0.0f);

		ya = Vector(0.0f,node.ya,0.0f); yb = ya + tg;
		yd = Vector(0.0f,next.ya,0.0f); yc = yd - bi;

		node.ytg = Vector(tg.y,0.0f,tg.x)*mat;

		Vector xa,xb,xc,xd;

		tg = Vector(cosf(node.xt)*node.xl,sinf(node.xt)*node.xl,0.0f);
		bi = Vector(cosf(next.xt)*next.xl,sinf(next.xt)*next.xl,0.0f);

		xa = Vector(0.0f,node.xa,0.0f); xb = xa + tg;
		xd = Vector(0.0f,next.xa,0.0f); xc = xd - bi;

		node.xtg = Vector(0.0f,tg.y,tg.x)*mat;

		Vector sa,sb,sc,sd;

		tg = Vector(cosf(node.st)*node.sl,sinf(node.st)*node.sl,0.0f);
		bi = Vector(cosf(next.st)*next.sl,sinf(next.st)*next.sl,0.0f);

		sa = Vector(0.0f,node.sc,0.0f); sb = sa + tg;
		sd = Vector(0.0f,next.sc,0.0f); sc = sd - bi;

		node.stg = Vector(0.0f,tg.y,tg.x)*mat;

		////

		Vector n = bd - ba;

		len = n|n; // len*len

		if( len <= 0.04f )
		{
			len = sqrtf(len);
		}
		else
		{
			len = Divide(0.0f,node.pos,bakes,ba,bb,bc,bd);
		}

		node.len = len;

	/*	Vector pd = next.pos - node.pos; pd.Normalize();
		Vector cd;

		float prev_d = delta; float thr = node.len;
		{
			Vector prev(node.pos);
			Vector next;

			while( len < node.len )
			{
				float d = 0.0f;

				for( ;; )
				{
					next = CubicBezier(ba,bb,bc,bd,t);

					cd = next - prev;

					d  = cd.GetLength();
					t += stp;

					cd.Normalize();

					float ang = pd|cd;

					if( d >= delta )
					{
						if( d > prev_d*1.5f || len + d >= node.len || d > thr || d*1.5f > node.len - len )
							break;

						if( ang < 0.97f )
						{
							if( bakes > 1 && prev_d > d*2.0f )
							{
								dword last = bakes.Last();

								const Bake &ab = bakes[last - 1];
									  Bake &bb = bakes[last];

								t	 = ab.t;
								prev = ab.pos;
								len	 = ab.time;

								thr = 0.5f*(prev_d + d);

								bakes.DelIndex(last);

								continue;
							}

							break;
						}
					}
				}

				len += d; prev_d = d; thr = node.len;

				if( len >= node.len )
					break;

				Bake &bake = bakes[bakes.Add()];

				bake.t = t - stp;

				bake.pos = next;

				bake.look = next - prev;

				bake.time = len;

				prev = next; pd = cd;
			}
		}*/

		for( int i = 0 ; i < bakes ; i++ )
		{
			Bake &bake = bakes[i];

			float k = bake.time/node.len;

			bake.up.Lerp(
				node.normal,next.normal,k);

			bake.za = CubicBezier(za,zb,zc,zd,k).y;
			bake.ya = CubicBezier(ya,yb,yc,yd,k).y;
			bake.xa = CubicBezier(xa,xb,xc,xd,k).y;

			bake.sc = CubicBezier(sa,sb,sc,sd,k).y;

			Vector xz;
			Vector yz;

			if((node.vx|next.vx) > (node.normal|next.normal))
			{
				xz.Lerp(node.vx,next.vx,k);

				yz = bake.look^xz;
			}
			else
			{
				yz.Lerp(node.normal,next.normal,k);

				xz = yz^bake.look;
			}

			xz.Normalize();
			yz.Normalize();

			bake.zp = bake.pos + xz*bake.za;
			bake.yp = bake.pos + xz*bake.ya;

			bake.xp = bake.pos + yz*bake.xa;
			bake.sp = bake.pos + yz*bake.sc;
		}
	}
}
