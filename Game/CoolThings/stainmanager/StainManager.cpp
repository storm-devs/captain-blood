#include "StainManager.h"
/*
	const float stain_delay = 0.2f;

	const float stain_in_time = 0.5f;

//	const float stain_res_time =  5.0f;
	const float stain_res_time = 15.0f;

	const float stain_out_time = 1.5f;

	const float stain_live_time = stain_in_time + stain_res_time + stain_out_time;

	const float stain_resp = 0.5f;
*/
//	const int stain_count = 32;
	const int stain_count = 64;

#include "..\..\..\Common_h\ishadows.h"

StainManager:: StainManager() :
	stains	 (_FL_,stain_count),
	materials(_FL_),
	list	 (_FL_, 16),
	stainsQueued(_FL_, 16)
{
	pVBuffer = null;
	bufCount = 0;

	updateBuffers = false;

	curTime = 0.0f;

	Time = null;
	K	 = null;

	stainsCount = 0;
	   triCount = 0;

	DiffMap = null;
	NormMap = null;

	ShadMap = null;

	pDiff = null;
	pNorm = null;

	pShad = null;

	stainResp = 0.0f;

	loaded = false;

	deb		= false;
	debDown = false;
}

StainManager::~StainManager()
{
	if( pVBuffer )
		pVBuffer->Release();

	Time = null;
	K	 = null;

	DiffMap = null;
	NormMap = null;
	ShadMap = null;

	if( pDiff )
		pDiff->Release();
	if( pNorm )
		pNorm->Release();

//	if( pShad )
//		pShad->Release();
}

void _cdecl StainManager::UpdateMapping(const char *group, MissionObject *sender)
{
	RELEASE(pDiff)
	RELEASE(pNorm)

	if( TextureMapper::GetMapping(Mission(),place))
	{
		pDiff = Render().CreateTexture(_FL_,place.diff);
		pNorm = Render().CreateTexture(_FL_,place.spec);

		if( string::IsEmpty(place.diff))
		{
			LogicDebugError("Stains diff texture not specified.");
		}
		if( string::IsEmpty(place.spec))
		{
			LogicDebugError("Stains spec texture not specified.");
		}

		loaded = true;

		if( EditMode_IsOn() == false )
		{
			Unregistry(GroupId('M','p','p','U'));
		}
	}
	else
	{
		pDiff = Render().getWhiteTexture();
		pDiff->AddRef();

		pNorm = Render().getWhiteTexture();
		pNorm->AddRef();

		LogicDebugError("TextureMapper not found.");
	}
}

bool StainManager::Create(MOPReader &reader)
{
	loaded = false;

	Render().GetShaderId("BloodStain",BloodStain_id);

	CreateBuffers();

	if( IShadowsService	*ser = (IShadowsService *)api->GetService("ShadowsService"))
	{
		IRenderTarget *tex = ser->GetPostProcessTexture();

		if( tex )
		{
			pShad = tex->AsTexture();
		}
		else
		{
			pShad = null;
		}
	}

	Time = Render().GetTechniqueGlobalVariable("BloodStainTime",_FL_);
	K	 = Render().GetTechniqueGlobalVariable("BloodStainK"   ,_FL_);

	DiffMap = Render().GetTechniqueGlobalVariable("BloodStainDiff",_FL_);
	NormMap = Render().GetTechniqueGlobalVariable("BloodStainNorm",_FL_);

	ShadMap = Render().GetTechniqueGlobalVariable("BloodStainShad",_FL_);

	////

	UpdateMapping(null,null);

	////

	Show	(true);
	Activate(true);

	if( EditMode_IsOn() || !loaded )
	{
		Registry(GroupId('M','p','p','U'),&StainManager::UpdateMapping,ML_FIRST);
	}

	return true;
}

void StainManager::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
	//	SetUpdate(&StainManager::Draw,ML_ALPHA5);
		SetUpdate(&StainManager::Draw,ML_ALPHA1);
	else
		DelUpdate(&StainManager::Draw);
}

void StainManager::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	//
}

void _cdecl StainManager::Draw(float dltTime, long level)
{
	int addCount = Min(int(2), int((7 + stainsQueued.Len()) >> 3));
	while (addCount>0 && !stainsQueued.IsEmpty())
	{
		AddStain(stainsQueued.LastE());
		stainsQueued.DelIndex(stainsQueued.Last());
		addCount--;
	}

	if( !IsActive())
		return;

	curTime += dltTime;

	bool debug = false;

/*	#ifndef _XBOX

	if( GetAsyncKeyState(VK_SHIFT) < 0 )
		debug = true;

	#endif*/

	int fades = 0;

	for( int i = 0 ; i < stains ; i++ )
	{
		Stain &stain = stains[i];

		if( !stain.busy )
			continue;

		stain.time -= dltTime;

		const TextureMapper::Stains::Loc &loc =
			stain.type == Soot ?
				place.soot : place.blood;

		if( stain.time > loc.out + loc.res )
		{
			float live = loc.in + loc.res + loc.out;

			if( stain.time > live )
			{
				stain.a = 0.0f;
			}
			else
			{
				stain.a = (live - stain.time)/loc.in;
			}
		}
		else
		if( stain.time < loc.out )
		{
			fades++;

			if( stain.time < 0.0f )
			{
				stain.busy = false;

				stainsCount--;
				Assert(stainsCount >= 0)

				triCount -= stain.list/3;
				Assert(triCount >= 0)

				updateBuffers = true;
			}
			else
			{
				stain.a = stain.time/loc.out;
			}
		}
		else
		{
			stain.a = 1.0f;
		}

	/*	float d = (Render().GetView().GetCamPos() - stain.center).GetLength2();

		const Plane *frustum = Render().GetFrustum();

		bool in = true;

		Vector p(stain.center);

		float z = 0.0f;

		if( frustum[1].Dist(p) < -z ||
			frustum[2].Dist(p) < -z ||
			frustum[3].Dist(p) < -z ||
			frustum[4].Dist(p) < -z )
			in = false;

		p.y += 1.0f;

		Render().Print(p,30.0f,1.0f,in ? 0xff00ff00 : 0xffff0000,"%.2f m\n%.1f\n%f",sqrtf(d),d,1.0f/d);*/
	}

	if( updateBuffers || pVBuffer->IsReseted())
		UpdateBuffers();

	updateBuffers = false;

#ifndef STOP_DEBUG
	if( debug )
	{
		for( int i = 0 ; i < stains ; i++ )
		{
			Stain &stain = stains[i];

			if( !stain.busy )
				continue;

			int count = stain.list/3;

			for( int j = 0 ; j < count ; j++ )
			{
				Vector *p = stain.list.GetBuffer() + j*3;

				Color c(0xffff0000);

				c.a = stain.a;

				Color d(0xffffffff);

				d.a = stain.a;

				Render().DrawLine(p[0],c,p[1],c);
				Render().DrawLine(p[1],c,p[2],c);
				Render().DrawLine(p[2],c,p[0],c);

				Vector x = p[1] - p[0];
				Vector y = p[2] - p[1];
				Vector z;

				z = x^y;
				z.Normalize();

				Render().DrawVector(p[0],p[0] + z,d);
				Render().DrawVector(p[1],p[1] + z,d);
				Render().DrawVector(p[2],p[2] + z,d);
			}

			Color c(0xff0000ff);

		//	c.a = stain.a;

			Render().DrawVector(stain.pos - stain.dir,stain.pos + stain.dir,c);

			Render().DrawSphere(stain.center,0.1f,c);
		}
	}
#endif

/*	if( debug )
	{
		Render().Print(0.0f, 0.0f + 200.0f,-1,"blood: %d, %d",bloodCnt,bufCount1);
		Render().Print(0.0f,14.0f + 200.0f,-1,"waste: %d, %d",wasteCnt,bufCount2);
	}*/

/*	if( api->DebugKeyState(VK_CONTROL,'S'))
	{
		if( debDown == false )
		{
			deb = !deb;
		}

		debDown = true;
	}
	else
	{
		debDown = false;
	}*/

	if( deb )
	{
		Render().Print(0.0f,0.0f + 200.0f,-1," total: %d/64, busy: %d/50, fade: %d",
			stains.Size(),stainsCount,fades);

	/*	int count = 0;

		for( int i = 0 ; i < stains ; i++ )
		{
			Stain &stain = stains[i];

			if( stain.busy )
			{
				count += stain.list/3;
			}
		}*/

		Render().Print(0.0f,0.0f + 220.0f,-1," %d/%d",triCount,stain_count*Stain::max_tri);
	}

	if( bufCount > 0 )
	{
		Render().SetStreamSource(0,pVBuffer);

		Render().SetWorld(Matrix());

		if( DiffMap && pDiff )
			DiffMap->SetTexture(pDiff);
		if( NormMap && pNorm )
			NormMap->SetTexture(pNorm);

		if( ShadMap && pShad )
			ShadMap->SetTexture(pShad);

		if( Time )
			Time->SetFloat(curTime);

		if( K )
			K->SetVector(Vector(1.0f/place.cols,1.0f/place.rows,0.0f));

		Render().SetViewport(Render().GetViewport());

		Render().SetupShadowLimiters();

		Render().DrawPrimitive(BloodStain_id,PT_TRIANGLELIST,0,bufCount);
	}
	else
		curTime = 0.0f;

/*	for( int i = 0 ; i < stains ; i++ )
	{
		const Stain &stain = stains[i];

		if( stain.busy )
		{
			Matrix m(stain.locs[0]);

			m.pos = stain.pos;

			Render().DrawMatrix(m);
			Render().DrawVector(stain.pos,stain.pos + stain.dir,-1);
		}
	}*/

/*	stainResp -= dltTime;

	if( stainResp < 0.0f )
	{
		stainResp = stain_resp;

		Matrix view = Render().GetView();

		Vector pos(0.0f);
		Vector dir(0.0f,0.0f,1.0f);

		pos = view.MulVertexByInverse(pos);
		dir = view.MulVertexByInverse(dir) - pos;

	//	float rad = 1.0f;
		float rad = 1.5f;

		AddStain(pos,dir,rad);
	}*/
}

void StainManager::UpdateBuffers()
{
	Vertex * __restrict v = (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);

	bufCount = 0;

	for( int i = 0 ; i < stains ; i++ )
	{
		if( bufCount >= stain_count*Stain::max_tri )
			break;

		Stain &stain = stains[i];

		if( !stain.busy )
			continue;

		const TextureMapper::Stains::Loc &loc =
			stain.type == Soot ?
				place.soot : place.blood;

		int count = stain.list/3;

		for( int j = 0 ; j < count ; j++ )
		{
			if( bufCount >= stain_count*Stain::max_tri )
				break;

			Vector *p = stain.list.GetBuffer() + j*3;
			Vector *t = stain.vert.GetBuffer() + j*3;

			v[0].p = p[0];
			v[0].c = loc.rgba;
			v[0].u = t[0].x; v[0].v = t[0].y;
			v[0].x = stain.locs[j].vx;
			v[0].y = stain.locs[j].vy;
			v[0].z = stain.locs[j].vz;
			v[0].t = stain.t;
			v[0].del = stain.del;
			v[0].i = stain.i; v[0].j = stain.j;
			v[0].power = loc.power;
			v[0].value = loc.value;

			v[1].p = p[1];
			v[1].c = loc.rgba;
			v[1].u = t[1].x; v[1].v = t[1].y;
			v[1].x = stain.locs[j].vx;
			v[1].y = stain.locs[j].vy;
			v[1].z = stain.locs[j].vz;
			v[1].t = stain.t;
			v[1].del = stain.del;
			v[1].i = stain.i; v[1].j = stain.j;
			v[1].power = loc.power;
			v[1].value = loc.value;

			v[2].p = p[2];
			v[2].c = loc.rgba;
			v[2].u = t[2].x; v[2].v = t[2].y;
			v[2].x = stain.locs[j].vx;
			v[2].y = stain.locs[j].vy;
			v[2].z = stain.locs[j].vz;
			v[2].t = stain.t;
			v[2].del = stain.del;
			v[2].i = stain.i; v[2].j = stain.j;
			v[2].power = loc.power;
			v[2].value = loc.value;

			v += 3; bufCount++;
		}
	}

	pVBuffer->Unlock();
}

const float _bound = 2.0f;

void StainManager::Clip(
	const Vector &a,
	const Vector &b,
	const Vector &c, array<Point> &res, float scale)
{
	float bound = _bound*scale;

	array<Point> &list = res; list.DelAll();

	list.Add(Point(a,1,2));
	list.Add(Point(b,2,0));
	list.Add(Point(c,0,1));

	int l = 0;
	int r = 0;

	for( int i = 0 ; i < list ; i++ )
	{
		if( list[i].p.x <= bound )
		{
			l++; list[i].r = false;
		}
		else
		{
			r++; list[i].r = true;
		}
	}

	if( l < 1 )
	{
		list.DelAll(); return;
	}

	if( r > 0 )
	{
		for( int i = 0 ; i < list ; i++ )
		{
			Point &p = list[i];

			if( p.r )
			{
				const Point &a = list[p.a];
				const Point &b = list[p.b];

				if( !a.r && !b.r )
				{
					Vector q = a.p;
					Vector t;

					t.x = bound;

					t.y = q.y + (p.p.y - q.y)/(p.p.x - q.x)*(t.x - q.x);
					t.z = q.z + (p.p.z - q.z)/(p.p.x - q.x)*(t.x - q.x);

					Point pt(t,p.a,i);

					q = b.p;

					t.x = bound;

					t.y = q.y + (p.p.y - q.y)/(p.p.x - q.x)*(t.x - q.x);
					t.z = q.z + (p.p.z - q.z)/(p.p.x - q.x)*(t.x - q.x);

					p.p = t;

					list.Insert(pt,i + 1);

					for( int j = 0 ; j < list ; j++ )
					{
						Point &item = list[j];

						if( item.a > i ) item.a++;
						if( item.b > i ) item.b++;
					}

					p.a = i + 1; list[list[i + 1].a].b = i + 1;

					i++;
				}
				else
				{
					Vector q = !a.r ? a.p : b.p;
					Vector t;

					t.x = bound;

					t.y = q.y + (p.p.y - q.y)/(p.p.x - q.x)*(t.x - q.x);
					t.z = q.z + (p.p.z - q.z)/(p.p.x - q.x)*(t.x - q.x);

					p.p = t;
				}
			}
		}
	}

	l = 0;
	r = 0;

	for( int i = 0 ; i < list ; i++ )
	{
		if( list[i].p.x >= 0.0f )
		{
			r++; list[i].r = true;
		}
		else
		{
			l++; list[i].r = false;
		}
	}

	if( r < 1 )
	{
		list.DelAll(); return;
	}

	if( l > 0 )
	{
		for( int i = 0 ; i < list ; i++ )
		{
			Point &p = list[i];

			if( !p.r )
			{
				const Point &a = list[p.a];
				const Point &b = list[p.b];

				if( a.r && b.r )
				{
					Vector q = a.p;
					Vector t;

					t.x = 0.0f;

					t.y = q.y + (p.p.y - q.y)/(p.p.x - q.x)*(t.x - q.x);
					t.z = q.z + (p.p.z - q.z)/(p.p.x - q.x)*(t.x - q.x);

					Point pt(t,p.a,i);

					q = b.p;

					t.x = 0.0f;

					t.y = q.y + (p.p.y - q.y)/(p.p.x - q.x)*(t.x - q.x);
					t.z = q.z + (p.p.z - q.z)/(p.p.x - q.x)*(t.x - q.x);

					p.p = t;

					list.Insert(pt,i + 1);

					for( int j = 0 ; j < list ; j++ )
					{
						Point &item = list[j];

						if( item.a > i ) item.a++;
						if( item.b > i ) item.b++;
					}

					p.a = i + 1; list[list[i + 1].a].b = i + 1;

					i++;
				}
				else
				{
					Vector q = a.r ? a.p : b.p;
					Vector t;

					t.x = 0.0f;

					t.y = q.y + (p.p.y - q.y)/(p.p.x - q.x)*(t.x - q.x);
					t.z = q.z + (p.p.z - q.z)/(p.p.x - q.x)*(t.x - q.x);

					p.p = t;
				}
			}
		}
	}

	l = 0;
	r = 0;

	for( int i = 0 ; i < list ; i++ )
	{
		if( list[i].p.y <= bound )
		{
			l++; list[i].r = false;
		}
		else
		{
			r++; list[i].r = true;
		}
	}

	if( l < 1 )
	{
		list.DelAll(); return;
	}

	if( r > 0 )
	{
		for( int i = 0 ; i < list ; i++ )
		{
			Point &p = list[i];

			if( p.r )
			{
				const Point &a = list[p.a];
				const Point &b = list[p.b];

				if( !a.r && !b.r )
				{
					Vector q = a.p;
					Vector t;

					t.y = bound;

					t.x = q.x + (p.p.x - q.x)/(p.p.y - q.y)*(t.y - q.y);
					t.z = q.z + (p.p.z - q.z)/(p.p.y - q.y)*(t.y - q.y);

					Point pt(t,p.a,i);

					q = b.p;

					t.y = bound;

					t.x = q.x + (p.p.x - q.x)/(p.p.y - q.y)*(t.y - q.y);
					t.z = q.z + (p.p.z - q.z)/(p.p.y - q.y)*(t.y - q.y);

					p.p = t;

					list.Insert(pt,i + 1);

					for( int j = 0 ; j < list ; j++ )
					{
						Point &item = list[j];

						if( item.a > i ) item.a++;
						if( item.b > i ) item.b++;
					}

					p.a = i + 1; list[list[i + 1].a].b = i + 1;

					i++;
				}
				else
				{
					Vector q = !a.r ? a.p : b.p;
					Vector t;

					t.y = bound;

					t.x = q.x + (p.p.x - q.x)/(p.p.y - q.y)*(t.y - q.y);
					t.z = q.z + (p.p.z - q.z)/(p.p.y - q.y)*(t.y - q.y);

					p.p = t;
				}
			}
		}
	}

	l = 0;
	r = 0;

	for( int i = 0 ; i < list ; i++ )
	{
		if( list[i].p.y >= 0.0f )
		{
			r++; list[i].r = true;
		}
		else
		{
			l++; list[i].r = false;
		}
	}

	if( r < 1 )
	{
		list.DelAll(); return;
	}

	if( l > 0 )
	{
		for( int i = 0 ; i < list ; i++ )
		{
			Point &p = list[i];

			if( !p.r )
			{
				const Point &a = list[p.a];
				const Point &b = list[p.b];

				if( a.r && b.r )
				{
					Vector q = a.p;
					Vector t;

					t.y = 0.0f;

					t.x = q.x + (p.p.x - q.x)/(p.p.y - q.y)*(t.y - q.y);
					t.z = q.z + (p.p.z - q.z)/(p.p.y - q.y)*(t.y - q.y);

					Point pt(t,p.a,i);

					q = b.p;

					t.y = 0.0f;

					t.x = q.x + (p.p.x - q.x)/(p.p.y - q.y)*(t.y - q.y);
					t.z = q.z + (p.p.z - q.z)/(p.p.y - q.y)*(t.y - q.y);

					p.p = t;

					list.Insert(pt,i + 1);

					for( int j = 0 ; j < list ; j++ )
					{
						Point &item = list[j];

						if( item.a > i ) item.a++;
						if( item.b > i ) item.b++;
					}

					p.a = i + 1; list[list[i + 1].a].b = i + 1;

					i++;
				}
				else
				{
					Vector q = a.r ? a.p : b.p;
					Vector t;

					t.y = 0.0f;

					t.x = q.x + (p.p.x - q.x)/(p.p.y - q.y)*(t.y - q.y);
					t.z = q.z + (p.p.z - q.z)/(p.p.y - q.y)*(t.y - q.y);

					p.p = t;
				}
			}
		}
	}

//	res = list;
}

void StainManager::AddStain(const Vector &pos, const Vector &dir, float rad, float scale, StainType type)
{
	StainQueued & st = stainsQueued[stainsQueued.Add()];
	st.pos = pos;
	st.dir = dir;
	st.rad = rad;
	st.scale = scale;
	st.type = type;
}

void StainManager::AddStain(const StainQueued & stQueue)
{
	const TextureMapper::Stains::Loc &loc =
		stQueue.type == Soot ?
			place.soot : place.blood;

	float scale = stQueue.scale * loc.scale;
	
	float bound = _bound * scale;

	Vector d = stQueue.dir; d.Normalize();

	Stain *p = GetFreeStain();

	if( p )
	{
		materials.DelAll();

		if( Physics().OverlapSphere(stQueue.pos, stQueue.rad, phys_mask(phys_bloodpatch),
			true,false,p->list,&materials))
		{
			Stain &stain = *p;

			stain.type = stQueue.type;

			stain.pos = stQueue.pos;
			stain.dir = d;

			stain.time = loc.delay + loc.in + /*loc.res*/100000.0f + loc.out;
			stain.a	   = 0.0f;

			stain.t = Vector4(curTime,loc.in,loc.out,stain.time - loc.delay);

			stain.del = loc.delay;

			stain.busy = true;

		/*	if( type == Soot )
			{
				stain.i = floor(RRnd(0.0f,2.99f));
				stain.j = floor(RRnd(0.0f,2.99f));
			}
			else // Blood
			{
				stain.i = floor(RRnd(0.0f,2.99f));
				stain.j = floor(RRnd(0.0f,2.99f));
			}*/

			{
				int i = place.cols*loc.y + loc.x + int(RRnd(0.0f,loc.n - 0.01f));

				stain.i = float(i%place.cols);
				stain.j = float(i/place.cols);
			}

			//------------ вектор куда брызг летел ----------------------------
			Vector dropVelocity = stQueue.dir;
			Vector vUp = Vector(0.0f,1.0f,0.0f);
			Vector start = stQueue.pos;
			Vector end = stQueue.pos + dropVelocity;

			//----------------- считаем матрицу для получения из позиций ---- uv координат ----------------
			//gimble lock случай, надо изменить вектор вверх...
			if((fabsf(start.x - end.x) < 0.001f) && (fabsf(start.z - end.z) < 0.001f))
			{
				vUp = Vector(1.0f,0.0f,0.0f);
			}

			Matrix mRotate;
				   mRotate.BuildView(start,end,vUp);

			Matrix mRotateZ;
				   mRotateZ.BuildRotateZ(RRnd(0.0f,2*PI));
			//	   mRotateZ.BuildRotateZ(-PI*0.7f);

			mRotate = mRotate*mRotateZ;

		/*	Matrix mScale;
			//	   mScale.BuildScale(Vector(0.5f/rad,0.5f/rad,1.0f));
				   mScale.BuildScale(Vector(1.0f,1.0f,1.0f));

			//Матрица приводящая к диапазону 0..1.0
			Matrix mNormalization;
			//	   mNormalization.BuildScale(Vector(0.5f,0.5f,1.0f));

			mNormalization = mNormalization*mScale;
			mNormalization = mNormalization*Matrix(Vector(0.0f),Vector(0.5f,0.5f,1.0f));*/

			Matrix mNormalization(Vector(0.0f),Vector(scale,scale,1.0f));

			Matrix mat = mRotate;
				   mat.Inverse();

			Matrix mFinal;
				   mFinal = mRotate*mNormalization;

		//	stain.m = mFinal;

			stain.vert.DelAll();
			stain.locs.DelAll();

			float kx = 1.0f/2.0f/scale/place.cols;
			float ky = 1.0f/2.0f/scale/place.rows;

			for( int i = 0 ; i < stain.list/3 ; i++ )
			{
				/*if( materials[i] == pmtlid_air ||
					materials[i] == pmtlid_water ) // не подходящий материал, удаляем полигон
				{
					materials.DelIndex(i); stain.list.DelRange(i*3,i*3 + 2); i--;
					continue;
				}*/

				Vector *p = stain.list.GetBuffer() + i*3;

				Vector x = p[1] - p[0];
				Vector y = p[2] - p[1];
				Vector z;

				z = x^y;
				z.Normalize();

				float k = z|d;

				if( k > -0.4f ) // слишком острый угол, удаляем полигон
				{
					materials.DelIndex(i); stain.list.DelRange(i*3,i*3 + 2); i--;
					continue;
				}

				Vector t0 = mFinal.MulVertex(p[0]);
				Vector t1 = mFinal.MulVertex(p[1]);
				Vector t2 = mFinal.MulVertex(p[2]);

				Clip(t0,t1,t2,list,scale);

				if( !list.Size())
				{
					materials.DelIndex(i); stain.list.DelRange(i*3,i*3 + 2); i--;
					continue;
				}

				Assert(list >= 3);

				Matrix m;
				
				m.BuildOrient(z,-y);
				m.Inverse();

			//	Vector e = d;
				Vector e = mat.vy;

				e = m.MulVertex(e);

				e.z = 0.0f;

				e = m.MulVertexByInverse(e);

				m.BuildOrient(-z,e);
				m.Inverse();

			//	z *= 0.0010f;
				z *= 0.0015f;

				////////////////////////////////////////
				// Нужно разобраться, почему при      //
				// клиппинге создается кривая вершина //
				////////////////////////////////////////

				for( int j = 0 ; j < list ; j++ )
				{
					const Vector &p = list[j].p;

					if( p.x > bound + 0.1f ||
						p.x <  0.0f - 0.1f ||
						p.y > bound + 0.1f ||
						p.y <  0.0f - 0.1f )
					{
						list.DelIndex(j); j--;
					}
				}

				////////////////////////////////////////

				for( int j = 0 ; j <= list - 3 ; j++ )
				{
					if( j > 0 )
					{
						i++;

						stain.list.Insert(i*3);
						stain.list.Insert(i*3);
						stain.list.Insert(i*3);

						p = stain.list.GetBuffer() + i*3;

						PhysTriangleMaterialID id = materials[i - 1];

						materials.Insert(id,i);
					}

					t0 = list[0	   ].p;
					t1 = list[j + 1].p;
					t2 = list[j + 2].p;

					p[0] = mFinal.MulVertexByInverse(t0) + z;
					p[1] = mFinal.MulVertexByInverse(t1) + z;
					p[2] = mFinal.MulVertexByInverse(t2) + z;

					t0.x *= kx; t0.y *= ky;
					t1.x *= kx; t1.y *= ky;
					t2.x *= kx; t2.y *= ky;

					stain.vert.Add(t0);
					stain.vert.Add(t1);
					stain.vert.Add(t2);

					stain.locs.Add(m);
				}
			}

			if( stain.list )
			{
				stain.center = 0.0f;

				for( int j = 0 ; j < stain.list ; j++ )
					stain.center += stain.list[j];

				stain.center *= 1.0f/stain.list.Size();

				stain.center = stain.pos + stain.dir*(stain.center - stain.pos).GetLength();

				updateBuffers = true;
			}
			else
				stain.busy = false;

			if( stain.busy )
			{
				stainsCount++; triCount += stain.list/3;
			}
		}
		else
		{
			p->busy = false;
		}
	}
}

void StainManager::ClearRad(const Vector &pos, float rad, float time)
{
	bool update = false;

	for( int i = 0 ; i < stains ; i++ )
	{
		Stain &stain = stains[i];

		if( !stain.busy )
			continue;

		if((stain.center - pos).GetLength() < rad )
		{
			if( stain.time > time )
			{
				stain.time = time;

				stain.t = Vector4(curTime - 0.5f,0.5f,time,0.5f + time);
				stain.a	= 1.0f;

				stain.del = 0.0f;

				update = true;
			}
		}
	}

	updateBuffers |= update;
}

void StainManager::CreateBuffers()
{
	if( pVBuffer )
		pVBuffer->Release();

	pVBuffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*stain_count*Stain::max_tri*3,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVBuffer)
}

StainManager::Stain *StainManager::GetFreeStain()
{
	Stain *freeStain = null;

	if( stainsCount + 1 > /*stain_count/2*/50 || triCount > 1024 )
	{
		Vector camPos = Render().GetView().GetCamPos();

		const Plane *frust = Render().GetFrustum();

		struct Res
		{
			int index; float d;

			Res()
			{
				index = -1;	d = 0.0f;
			}

			void update(float dist, int i)
			{
				if( dist > d )
				{
					d = dist; index = i;
				}
			}

		} ins,out;

		for( int i = 0 ; i < stains ; i++ )
		{
			Stain &stain = stains[i];

			if( stain.busy == false )
			{
				freeStain = &stain; continue;
			}

			if( stain.time <= stain.t.z )
				continue;

			const Vector &p = stain.center;

			float d = (camPos - p).GetLength2();

			const float r = 1.5f;

			if( frust[1].Dist(p) < -r ||
				frust[2].Dist(p) < -r ||
				frust[3].Dist(p) < -r ||
				frust[4].Dist(p) < -r )
			{
				out.update(d,i);
			}
			else
			{
				ins.update(d,i);
			}
		}

		if( ins.index >= 0 || out.index >= 0 )
		{
			int index = out.index;

			if( index < 0 )
				index = ins.index;

			Stain &stain = stains[index];

			float fadeOut = stain.t.z;

			stain.time = fadeOut;

			stain.t = Vector4(curTime - 0.5f,0.5f,fadeOut,0.5f + fadeOut);
			stain.a	= 1.0f;

			stain.del = 0.0f;

			updateBuffers = true;
		}
	}
	else
	{
		for( int i = 0 ; i < stains ; i++ )
			if( !stains[i].busy )
				return &stains[i];
	}

	if( freeStain )	return freeStain;

	if( stains < stain_count )
		return &stains[stains.Add()];

	return null;
}

MOP_BEGINLIST(StainManager, "", '1.00', 2000)
MOP_ENDLIST(StainManager)
