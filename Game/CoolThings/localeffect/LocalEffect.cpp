#include "LocalEffect.h"

const int effect_max_size =  256;
const int drops_count	  =  256;

const int buffer_max_size = 2048;
const int lines_count	  = 2048;

const int max_cloud_count =	 512;

LocalEffect:: LocalEffect() :
	 drops(_FL_,drops_count),
	 lines(_FL_,lines_count),
	clouds(_FL_,max_cloud_count)
{
	m_restart = false;

	pVBuffer = NULL;
	pIBuffer = NULL;

	pVB = NULL;
	pIB = NULL;

	pRS = (IRender*)api->GetService("DX9Render");
	Assert(pRS)

	pRS->AddRenderFilter(this,1.0f);

	NatMap = pRS->GetTechniqueGlobalVariable( "NativeTexture",_FL_);
	NorMap = pRS->GetTechniqueGlobalVariable( "NormalTexture",_FL_);
	DifMap = pRS->GetTechniqueGlobalVariable("DiffuseTexture",_FL_);
	kAspect = pRS->GetWideScreenAspectWidthMultipler()*9.0f/16.0f;

	pNor = null;
	pDif = null;

	time		= 0.0f;
	time_remain = 0.0f;

	deltaTime = 0.0f;

	//// smoke ////

	smoke.pVB = null;
	smoke.pIB = null;

	smoke.Texture = null;

	smoke.pFace = null;

	loaded = false;
}

LocalEffect::~LocalEffect()
{
	pRS->RemoveRenderFilter(this);

	if( pVBuffer )
		pVBuffer->Release();
	if( pIBuffer )
		pIBuffer->Release();

	if( pVB )
		pVB->Release();
	if( pIB )
		pIB->Release();

	NatMap = NULL;
	NorMap = NULL;
	DifMap = NULL;

	if( pNor )
		pNor->Release();
	if( pDif )
		pDif->Release();

	//// smoke ////

	if( smoke.pVB )
		smoke.pVB->Release();
	if( smoke.pIB )
		smoke.pIB->Release();

	smoke.Texture = NULL;

	if( smoke.pFace )
		smoke.pFace->Release();
}

LocalEffect::Drop *LocalEffect::GetDrop()
{
	for( int i = 0 ; i < drops ; i++ )
		if( drops[i].isBusy == 0 )
			break;

	Drop *p = NULL;

	if( i < drops )
	{
		p = &drops[i];
	}
	else
	if( drops < drops_count )
	{
		p = &drops[drops.Add()];
	}

	if( p )
	{
		float ang = (rand() & 0x1f)*(PI*2.0f/(0x1f + 1));
		p->angCos = cosf(ang);
		p->angSin = sinf(ang);
		p->spd = 0.8f + (rand() & 0xff)*(0.2f/(0xff + 1));		
	}
	return p;
}

void LocalEffect::Restart()
{
	m_restart = true;

	ReCreate();
}

void _cdecl LocalEffect::UpdateMapping(GroupId group, MissionObject *sender)
{
	RELEASE(smoke.pFace)

	if( TextureMapper::GetMapping(Mission(),cl_place))
	{
		smoke.pFace = pRS->CreateTexture(_FL_,cl_place.diff);

		if( string::IsEmpty(cl_place.diff))
		{
			LogicDebugError("Smoke texture not specified.");
		}

		loaded = true;

		if( EditMode_IsOn() == false )
		{
			Unregistry(GroupId('M','p','p','U'));
		}
	}
	else
	{
		smoke.pFace = pRS->getWhiteTexture();
		smoke.pFace->AddRef();

		LogicDebugError("TextureMapper not found.");
	}

	RELEASE(pNor)
	RELEASE(pDif)

	if( TextureMapper::GetMapping(Mission(),dr_place))
	{
		pDif = pRS->CreateTexture(_FL_,dr_place.diff);
		pNor = pRS->CreateTexture(_FL_,dr_place.spec);

		if( string::IsEmpty(dr_place.diff))
		{
			LogicDebugError("Drops diff texture not specified.");
		}
		if( string::IsEmpty(dr_place.spec))
		{
			LogicDebugError("Drops spec texture not specified.");
		}
	}
	else
	{
		pNor = pRS->getWhiteTexture();
		pNor->AddRef();

		pDif = pRS->getWhiteTexture();
		pDif->AddRef();

		LogicDebugError("TextureMapper not found.");
	}
}

bool LocalEffect::Create(MOPReader &reader)
{
	loaded = false;

	if( m_restart )
	{
		m_restart = false;
	}
	else
	{
		pRS->GetShaderId("SimpleQuad", SimpleQuad_id);
		pRS->GetShaderId("LocalEffect", LocalEffect_id);
		pRS->GetShaderId("Cloud", Cloud_id);
		pRS->GetShaderId("LocalLines", LocalLines_id);

		smoke.Texture = pRS->GetTechniqueGlobalVariable("CloudTexture",_FL_);

		CreateBuffers();

		UpdateMapping(GroupId('M','p','p','U'),null);
	}

	InitParams(reader);

	if( EditMode_IsOn() || !loaded )
	{
		Registry(GroupId('M','p','p','U'),&LocalEffect::UpdateMapping,ML_FIRST);
	}

	return true;
}

bool LocalEffect::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void LocalEffect::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
}

void LocalEffect::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&LocalEffect::Draw,ML_PARTICLES5);
	else
		DelUpdate(&LocalEffect::Draw);

	Enable(isShow && !EditMode_IsOn() && rain);

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}

void LocalEffect::Command(const char *id, dword numParams, const char **params)
{
	//
}

void LocalEffect::FilterImage(IBaseTexture *pScr, IRenderTarget *pDst)
{
	if( !pVBuffer )
		return;

	float dltTime = deltaTime;

	if( IsActive())
	{
		time		+= dltTime;
		time_remain += dltTime;
	}

//	const float add_time = 0.10f;
	const float add_time = 0.05f*1024/count/vel;

	if( time_remain > add_time )
	{
		Vector n = pRS->GetView().MulVertexByInverse(Vector(0.0f,0.0f,-1.0f)) - pRS->GetView().GetCamPos();

		float pr = n|dir.vz;

		if( RRnd(0.0f,1.0f) < pr )
		{
			Drop *p = GetDrop();

			if( p )
			{
				p->time = 0.0f;

				const float r = 1.0f - 0.24f;

				p->x = RRnd(-r,r);
				p->y = RRnd(-r,r);
			}
		}

		time_remain = 0.0f;
	}

	Vertex *p = (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);

	int pn = 0;

	float curDltTime = dltTime;
	if(!IsActive())
	{
		curDltTime = 0.0f;
	}

	const float tl1 = dr_place.place[0].t; 
	const float tl2 = dr_place.place[1].t;
	const float tr1 = tl1 + dr_place.place[0].w;
	const float tr2 = tl2 + dr_place.place[1].w;		
	const float tt1 = dr_place.place[0].u;
	const float tt2 = dr_place.place[1].u;
	const float tb1 = tt1 + dr_place.place[0].h;		 
	const float tb2 = tt2 + dr_place.place[1].h;

	static const float spl_time = 0.1f;
	static const float res_time = 1.0f;
	static const float rem_time = 1.5f;

	dword count = drops.Size();

	for( dword i = 0 ; i < count && pn < effect_max_size; i++)
	{
		Drop &drop = drops[i];
		if(!drop.isBusy) continue;

		if(drop.time > spl_time + res_time + rem_time)
		{
			drop.time = 0;
			drop.isBusy = 0;
			continue;
		}

		float alp = 1.0f - Clampf((drop.time - spl_time - res_time)*(1.0f/rem_time));
		float ble = coremin(drop.time*(1.0f/spl_time), 1.0f);
		float size = 0.1f + ble*0.15f;

		drop.time += curDltTime*drop.spd;		

		const float x = drop.x;
		const float y = drop.y;

		const float sizeCosY = size*drop.angCos;
		const float sizeSinY = size*drop.angSin;
		const float sizeCosX = sizeCosY*kAspect;
		const float sizeSinX = sizeSinY*kAspect;

		p[0].p = Vector(x - sizeCosX,y - sizeSinY,0.0f);
		p[0].u1 = tl1; p[0].v1 = tt1;
		p[0].u2 = tl2; p[0].v2 = tt2;
		p[0].a = alp; p[0].b = ble;

		p[1].p = Vector(x - sizeSinX,y + sizeCosY,0.0f);
		p[1].u1 = tl1; p[1].v1 = tb1;
		p[1].u2 = tl2; p[1].v2 = tb2;
		p[1].a = alp; p[1].b = ble;

		p[2].p = Vector(x + sizeCosX,y + sizeSinY,0.0f);
		p[2].u1 = tr1; p[2].v1 = tb1;
		p[2].u2 = tr2; p[2].v2 = tb2;
		p[2].a = alp; p[2].b = ble;

		p[3].p = Vector(x + sizeSinX,y - sizeCosY,0.0f);
		p[3].u1 = tr1; p[3].v1 = tt1;
		p[3].u2 = tr2; p[3].v2 = tt2;
		p[3].a = alp; p[3].b = ble;

		pn++;
		p += 4;
	}

	pVBuffer->Unlock();

	if( pn > 0 )
	{
		pRS->SetRenderTarget(RTO_DONTOCH_CONTEXT,pDst);
		pRS->SetViewport(pRS->GetFullScreenViewPort_3D());
		pRS->BeginScene();

		if( pScr )
		{
			NatMap->SetTexture(pScr);

			const RENDERVIEWPORT &vp = pRS->GetViewport();

			float dx = 1.0f/vp.Width;
			float dy = 1.0f/vp.Height;

			static float b[] = {
				-1.0f - dx, 1.0f + dy,0.0f,0.0f,
				-1.0f - dx,-1.0f + dy,0.0f,1.0f,
				 1.0f - dx, 1.0f + dy,1.0f,0.0f,
				 1.0f - dx,-1.0f + dy,1.0f,1.0f};

			pRS->DrawPrimitiveUP(SimpleQuad_id, PT_TRIANGLESTRIP,2,b,4*sizeof(float));

			pRS->SetStreamSource(0,pVBuffer);
			pRS->SetIndices(pIBuffer,0);

			if( pNor )
			{
				NorMap->SetTexture(pNor);
			}

			if( pDif )
			{
				DifMap->SetTexture(pDif);
			}

			pRS->DrawIndexedPrimitive(LocalEffect_id,	PT_TRIANGLELIST,0,pn*4*1,0,pn*2*1);
		}

		pRS->EndScene();
	}
}

void _cdecl LocalEffect::Draw(float dltTime, long level)
{
	deltaTime = dltTime;

	if( !rain && !smoke.use )
		return;

	Matrix view = pRS->GetView();

	Matrix m(dir);

//	Vector off(0.0f,0.0f,1.5f);
	Vector off(0.0f);

	m.pos = view.MulVertexByInverse(off);

	Vector r(20.0f);

	if( smoke.use )
	{
		Vector r(10.0f);

		for( int i = 0 ; i < smoke.clouds ; i++ )
		{
			Cloud &cloud = clouds[i];

			cloud.time += dltTime;

			Vector to(1.0f,0.0f,0.0f);

			cloud.p += to*dltTime;

			if( cloud.time > cloud.live )
			{
				cloud.p.Rand(-r,r);

				cloud.p = m.MulVertex(cloud.p);

				cloud.time = 0.0f;
				cloud.live = RRnd(3.0f,5.0f);

				cloud.rs = RRnd(1.0f,2.0f);
			}
			else
			{
				Vector p = m.MulVertexByInverse(cloud.p);

				float r = 10.0f;

				if( fabsf(p.x) > r )
				{
					if( p.x > 0.0f )
						p.x = -r + fmod(p.x,r);
					else
						p.x =  r + fmod(p.x,r);

					cloud.p = m.MulVertex(p);
				}

				if( fabsf(p.y) > r )
				{
					if( p.y > 0.0f )
						p.y = -r + fmod(p.y,r);
					else
						p.y =  r + fmod(p.y,r);

					cloud.p = m.MulVertex(p);
				}

				if( fabsf(p.z) > r )
				{
					if( p.z > 0.0f )
						p.z = -r + fmod(p.z,r);
					else
						p.z =  r + fmod(p.z,r);

					cloud.p = m.MulVertex(p);
				}
			}
		}

		Smoke::Vertex *p = (Smoke::Vertex *)smoke.pVB->Lock(0,0,LOCK_DISCARD);

		int pn = 0;

		for( int i = 0 ; i < smoke.clouds ; i++ )
		{
			Cloud &cloud = clouds[i];

			float k = cloud.time/cloud.live;

			k = 0.0f;

			float z = 3.5f*(1.0f + k*1.5f);

			p[0].p = 0.0f;//Vector(-z,-z,0.0f);
			p[1].p = 0.0f;//Vector(-z, z,0.0f);
			p[2].p = 0.0f;//Vector( z, z,0.0f);
			p[3].p = 0.0f;//Vector( z,-z,0.0f);

			Vector off(0.0f);

			Matrix m;
			
		//	Vector t = cloud.p - view.GetCamPos();
			Vector t = pRS->GetView().MulVertexByInverse(Vector(0.0f,0.0f,1.0f)) - pRS->GetView().GetCamPos();
			
			Vector u;

			Vector up(0.0f,1.0f,0.0f);
			Vector hz(1.0f,0.0f,0.0f);

			if( fabsf(t|up) < 0.7 )
				u = up;
			else
				u = hz;

			m.BuildOrient(t,u);

		//	float ang = cloud.time*0.3f;
			float ang = cloud.time*0.2f;

			ang *= cloud.rs;

			if( cloud.i&1 )
				ang = -ang;

		//	ang = 0.0f;

			Matrix n; n.BuildRotateZ(ang);

			m = n*m;

			p[0].q = cloud.p + off + m.MulVertex(Vector(-z,-z,0.0f));
			p[1].q = cloud.p + off + m.MulVertex(Vector(-z, z,0.0f));
			p[2].q = cloud.p + off + m.MulVertex(Vector( z, z,0.0f));
			p[3].q = cloud.p + off + m.MulVertex(Vector( z,-z,0.0f));

		/*	const float dt1 = 1.0f/8;

			const float tl1 = dt1*3; const float tr1 = tl1 + dt1;
			const float tt1 = dt1*0; const float tb1 = tt1 + dt1;*/
			const float tl1 = cl_place.place[0].t; const float tr1 = tl1 + cl_place.place[0].w;
			const float tt1 = cl_place.place[0].u; const float tb1 = tt1 + cl_place.place[0].h;

			p[0].u1 = tl1; p[0].v1 = tt1;
			p[1].u1 = tl1; p[1].v1 = tb1;
			p[2].u1 = tr1; p[2].v1 = tb1;
			p[3].u1 = tr1; p[3].v1 = tt1;

		/*	const float dt2 = 1.0f/8;

			const float tl2 = dt2*2; const float tr2 = tl2 + dt2;
			const float tt2 = dt2*0; const float tb2 = tt2 + dt2;*/
			const float tl2 = cl_place.place[1].t; const float tr2 = tl2 + cl_place.place[1].w;
			const float tt2 = cl_place.place[1].u; const float tb2 = tt2 + cl_place.place[1].h;

			p[0].u2 = tl2; p[0].v2 = tt2;
			p[1].u2 = tl2; p[1].v2 = tb2;
			p[2].u2 = tr2; p[2].v2 = tb2;
			p[3].u2 = tr2; p[3].v2 = tt2;

			float l = cloud.live*0.5f;

			float a = cloud.time < l ? cloud.time/l : 1.0f - (cloud.time - l)/l;

			p[0].bl = a;
			p[1].bl = a;
			p[2].bl = a;
			p[3].bl = a;

	/*	//	a *= 0.3f;
			a *= 0.2f;

		//	a = 1.0f;

			Color c = Vector4(1.0f,1.0f,1.0f,a);*/
			Color c = smoke.color; c.a *= a;

			p[0].cl = c;
			p[1].cl = c;
			p[2].cl = c;
			p[3].cl = c;

			ang = 0.0f;

			p[0].az = ang;
			p[1].az = ang;
			p[2].az = ang;
			p[3].az = ang;

			p += 4; pn++;
		}

		smoke.pVB->Unlock();

		if( pn > 0 )
		{
			pRS->SetStreamSource(0,smoke.pVB);
			pRS->SetIndices(smoke.pIB,0);

			pRS->SetWorld(Matrix());

			if( smoke.pFace )
				smoke.Texture->SetTexture(smoke.pFace);

			pRS->DrawIndexedPrimitive(Cloud_id,	PT_TRIANGLELIST,0,pn*4,0,pn*2);
		}
	}

	if( !rain )
		return;

	if( EditMode_IsOn())
		pRS->DrawBox(-r,r,m);

/*	if( IsActive())
	{
		time_r += dltTime;
	}

	const float add_time = 0.01f;

	if( time_r > add_time )
	{
		Line *p = GetLine();

		if( p )
		{
			p->p.Rand(-r,r);

			p->p = m.MulVertex(p->p);
		}

		time_r = 0.0f;
	}*/

	for( int i = 0 ; i < lines ; i++ )
	{
		Line &line = lines[i];

		line.p += dir.vz*vel*dltTime*15.0f;

		float len = vel*2.0f;

		Vector p = m.MulVertexByInverse(line.p);

		float r = 20.0f;

		float dx = p.x;
		float dy = p.y;
		float dz = p.z - r;

		if( dz > len )
		{
			line.p.x = RRnd(-r,r);
			line.p.y = RRnd(-r,r);

			line.p.z = dz - r;

			line.p = m.MulVertex(line.p);
		}
		else
		if( p.z < -(r + len*2.0f))
		{
			p.z = r + fmod(p.z + (r + len*2.0f),r);

			line.p = m.MulVertex(p);
		}
		else
		{
			if( fabsf(dx) > r )
			{
				if( p.x > 0.0f )
					p.x = -r + fmod(p.x,r);
				else
					p.x =  r + fmod(p.x,r);

				line.p = m.MulVertex(p);
			}

			if( fabsf(dy) > r )
			{
				if( p.y > 0.0f )
					p.y = -r + fmod(p.y,r);
				else
					p.y =  r + fmod(p.y,r);

				line.p = m.MulVertex(p);
			}
		}
	}

	Verte_ *p = (Verte_ *)pVB->Lock(0,0,LOCK_DISCARD);

	int pn = 0;

	for( int i = 0 ; i < lines ; i++ )
	{
		Line &line = lines[i];

	/*	if( !line.busy )
			continue;

		line.p += dir.vz*vel*dltTime*15.0f;

		if( !m.MulVertexByInverse(line.p - dir.vz*vel).InBox(-r,r) &&
		    !m.MulVertexByInverse(line.p			 ).InBox(-r,r))
		{
		//	line.busy = false;

			Vector q = -r; q.y = r.y;

			p->p.Rand(q,r);

			p->p = m.MulVertex(p->p);
		}*/

		if( pn >= buffer_max_size )
			break;

		Vector r = (line.p - view.GetCamPos())^dir.vz;

		r.Normalize();
		r *= 0.005f;

		p[0].p = line.p - r;
		p[1].p = line.p - r - dir.vz*vel;
		p[2].p = line.p + r - dir.vz*vel;
		p[3].p = line.p + r;

		p[0].tu = 0.0f; p[0].tv = 0.0f;
		p[1].tu = 0.0f; p[1].tv = 1.0f;
		p[2].tu = 1.0f; p[2].tv = 1.0f;
		p[3].tu = 1.0f; p[3].tv = 0.0f;

		pn++; p += 4;
	}

	pVB->Unlock();

	if( pn > 0 )
	{
		pRS->SetStreamSource(0,pVB);
		pRS->SetIndices(pIB,0);

		pRS->SetWorld(Matrix());

		pRS->DrawIndexedPrimitive(LocalLines_id,PT_TRIANGLELIST,0,pn*4,0,pn*2);
	}
}

void LocalEffect::Generate()
{
	lines.DelAll();

	Matrix view = pRS->GetView();

	Matrix m(dir);

//	Vector off(0.0f,0.0f,1.5f);
	Vector off(0.0f);

	m.pos = view.MulVertexByInverse(off);

	{
		Vector r(20.0f);

		for( int i = 0 ; i < count ; i++ )
		{
			Line &line = lines[lines.Add()];

			line.p.Rand(-r,r);

			line.p = m.MulVertex(line.p);
		}
	}

	clouds.DelAll();

	{
		Vector r(10.0f);

		for( int i = 0 ; i < smoke.clouds ; i++ )
		{
			Cloud &cloud = clouds[clouds.Add()];

			cloud.p.Rand(-r,r);

			cloud.p = m.MulVertex(cloud.p);

			cloud.time = 0.0f;
			cloud.live = RRnd(3.0f,5.0f);

			cloud.i = i;

			cloud.rs = RRnd(1.0f,2.0f);
		}
	}
}

void LocalEffect::CreateBuffers()
{
	if( pVBuffer )
		pVBuffer->Release();
	if( pIBuffer )
		pIBuffer->Release();

	pVBuffer = pRS->CreateVertexBuffer(
		sizeof(Vertex)*effect_max_size*4,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVBuffer)

	pIBuffer = pRS->CreateIndexBuffer(
		sizeof(WORD)  *effect_max_size*6,
		_FL_, USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(pIBuffer)

	WORD *p = (WORD*)pIBuffer->Lock();
	Assert(p)

	for( long i = 0 ; i < effect_max_size ; i++ )
	{
		p[i*6 + 0] = WORD(i*4 + 0);
		p[i*6 + 1] = WORD(i*4 + 1);
		p[i*6 + 2] = WORD(i*4 + 2);
		p[i*6 + 3] = WORD(i*4 + 0);
		p[i*6 + 4] = WORD(i*4 + 2);
		p[i*6 + 5] = WORD(i*4 + 3);
	}

	pIBuffer->Unlock();

	if( pVB )
		pVB->Release();
	if( pIB )
		pIB->Release();

	pVB = pRS->CreateVertexBuffer(
		sizeof(Verte_)*buffer_max_size*4,
		sizeof(Verte_),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVB)

	pIB = pRS->CreateIndexBuffer(
		sizeof(WORD)  *buffer_max_size*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(pIB)

	p = (WORD*)pIB->Lock();
	Assert(p)

	for( long i = 0 ; i < buffer_max_size ; i++ )
	{
		p[i*6 + 0] = WORD(i*4 + 0);
		p[i*6 + 1] = WORD(i*4 + 1);
		p[i*6 + 2] = WORD(i*4 + 2);
		p[i*6 + 3] = WORD(i*4 + 0);
		p[i*6 + 4] = WORD(i*4 + 2);
		p[i*6 + 5] = WORD(i*4 + 3);
	}

	pIB->Unlock();

	//// smoke ////

	if( smoke.pVB )
		smoke.pVB->Release();
	if( smoke.pIB )
		smoke.pIB->Release();

	smoke.pVB = pRS->CreateVertexBuffer(
		sizeof(Smoke::Vertex)*max_cloud_count*4,
		sizeof(Smoke::Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(smoke.pVB)

	smoke.pIB = pRS->CreateIndexBuffer(
		sizeof(WORD)  *max_cloud_count*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(smoke.pIB)

	p = (WORD*)smoke.pIB->Lock();
	Assert(p)

	for( long i = 0 ; i < max_cloud_count ; i++ )
	{
		p[i*6 + 0] = WORD(i*4 + 0);
		p[i*6 + 1] = WORD(i*4 + 1);
		p[i*6 + 2] = WORD(i*4 + 2);
		p[i*6 + 3] = WORD(i*4 + 0);
		p[i*6 + 4] = WORD(i*4 + 2);
		p[i*6 + 5] = WORD(i*4 + 3);
	}

	smoke.pIB->Unlock();
}

void LocalEffect::InitParams(MOPReader &reader)
{
	bool sh = reader.Bool();
	bool ac = reader.Bool();

	rain = reader.Bool();

	dir = Matrix(reader.Angles());
	vel = reader.Float();

	count = reader.Long();

	if( count > lines_count )
		count = lines_count;

	smoke.use	 = reader.Bool();
	smoke.clouds = reader.Long();

	smoke.color = reader.Colors();

	Generate();

	Show	(sh);
	Activate(ac);
}

MOP_BEGINLISTCG(LocalEffect, "Local effect", '1.00', 100, "Local effect\n\n    Use to applay local effect\n\nAviable commands list:\n\n    play - play effect\n\n        no parameters", "Default")

	MOP_BOOL("Show"  , false);
	MOP_BOOL("Active", true );

	MOP_GROUPBEG("Rain parameters")

		MOP_BOOL("Use rain", false)

		MOP_ANGLES("Direction", Vector(PI*0.5f,0.0f,0.0f))
		MOP_FLOAT ("Velocity", 1.0f)

		MOP_LONG("Drops count", 1024)

	MOP_GROUPEND()

	MOP_GROUPBEG("Smoke parameters")

		MOP_BOOL("Use smoke", false)

	//	MOP_ANGLES("Direction", Vector(PI*0.5f,0.0f,0.0f))
	//	MOP_FLOAT ("Velocity", 1.0f)

	//	MOP_LONG("Drops count", 1024)

		MOP_LONGEX("Cloud count", 160, 16, max_cloud_count)
		MOP_COLOR("Color", Color(1.0f,1.0f,1.0f,0.2f))

	MOP_GROUPEND()

MOP_ENDLIST(LocalEffect)
