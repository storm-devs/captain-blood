#include "TrailManager.h"
#include "..\..\..\Common_h\SetThreadName.h"

	const int trail_max_size =  1024;
//	const int trail_max_size =  4096;
//	const int trail_max_size =  8192;
//	const int trail_max_size = 16384;

static dword next = 1;

mathinline dword mathcall MyRand(void)
{
	next = next*1103515245 + 12345;
//	return((dword)(next/65536)%32768);
	return((dword)(next&0xffff0000 >> 16)&0x7fff);
}

mathinline float mathcall MyRRnd(float a, float b)
{
	static const float k = 1.0f/RAND_MAX;

	return a + MyRand()*((b - a)*k);
}

//#define RRnd(a,b) MyRRnd(a,b)

TrailManager:: TrailManager() : trails(_FL_, 64), qverts(_FL_, 1536)
{
	pVBuffer = null;
	pIBuffer = null;

	TrailTexture = null;
	TrailBumpMap = null;

	pFace = null;
	pBump = null;

	m_isMultiThreading = (api->GetThreadingInfo() != ICore::mt_none);

	if (m_isMultiThreading)
	{
		m_hStartEvent = CreateEvent(null, false, false, null); Assert(m_hStartEvent);
		m_hExitEvent = CreateEvent(null, false, false, null); Assert(m_hExitEvent);
		m_hDoneEvent = CreateEvent(null, false, false, null); Assert(m_hDoneEvent);

		DWORD dwTrailThreadID = 0;
		m_hThread = CreateThread(NULL, 16384, (LPTHREAD_START_ROUTINE)&WorkThread, this, CREATE_SUSPENDED, &dwTrailThreadID); Assert(m_hThread);
#ifdef _XBOX
		::XSetThreadProcessor(m_hThread, 3);
#endif
		XSetThreadName(dwTrailThreadID, "Trail::Work");
		ResumeThread(m_hThread);
	}
	else
	{
		m_hThread = null;
		m_hStartEvent = null;
		m_hExitEvent = null;
		m_hDoneEvent = null;
	}

	loaded = false;
}

TrailManager::~TrailManager()
{
	RELEASE(pVBuffer)
	RELEASE(pIBuffer)

	TrailTexture = NULL;
	TrailBumpMap = NULL;

	RELEASE(pFace)
	RELEASE(pBump)

	for( int i = 0 ; i < trails ; i++ )
		delete trails[i];

	if (m_isMultiThreading)
	{
		SetEvent(m_hExitEvent);
		for (int i=0; i<5; i++)
		{
			if (WaitForSingleObject(m_hThread, 2000) == WAIT_OBJECT_0 )
				break;
			
			api->Trace("TrailManager: Killing thread problem!");
		}

		CloseHandle(m_hThread);
		CloseHandle(m_hStartEvent);
		CloseHandle(m_hExitEvent);
		CloseHandle(m_hDoneEvent);
	}
}

ITrail *TrailManager::Add()
{
	for( int i = 0 ; i < trails ; i++ )
		if( !trails[i]->busy )
			break;

	if( i < trails )
	{
		trails[i]->busy = true;

		return trails[i];
	}

	Trail *trail = NEW Trail();

	trails.Add(trail);

	trail->busy = true;

	return trail;
}

void _cdecl TrailManager::UpdateMapping(const char *group, MissionObject *sender)
{
	RELEASE(pFace)
	RELEASE(pBump)

	if( TextureMapper::GetMapping(Mission(),place))
	{
		pFace = Render().CreateTexture(_FL_,place.diff);
		pBump = Render().CreateTexture(_FL_,place.spec);

		if( string::IsEmpty(place.diff))
		{
			LogicDebugError("Trail diff texture not specified.");
		}
		if( string::IsEmpty(place.spec))
		{
			LogicDebugError("Trail spec texture not specified.");
		}

		loaded = true;

		if( EditMode_IsOn() == false )
		{
			Unregistry(GroupId('M','p','p','U'));
		}
	}
	else
	{
		pFace = Render().getWhiteTexture();
		pFace->AddRef();

		pBump = Render().getWhiteTexture();
		pBump->AddRef();

		LogicDebugError("TextureMapper not found.");
	}
}

bool TrailManager::Create(MOPReader &reader)
{
	loaded = false;

	Render().GetShaderId("Trail",Trail_id);

	TrailTexture = Render().GetTechniqueGlobalVariable("TrailTexture"	,_FL_);
	TrailBumpMap = Render().GetTechniqueGlobalVariable("TrailTexture_BM",_FL_);

	UpdateMapping(null,null);

	CreateBuffers();
	Show(true);

	if( EditMode_IsOn() || !loaded )
	{
		Registry(GroupId('M','p','p','U'),&TrailManager::UpdateMapping,ML_FIRST);
	}

	return true;
}

void TrailManager::Activate(bool isActive)
{
	//
}

void TrailManager::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
	{
		if (m_isMultiThreading)
			ResetEvent(m_hDoneEvent);

		SetUpdate(&TrailManager::Update,ML_EXECUTE1);
		SetUpdate(&TrailManager::Draw  ,ML_PARTICLES3);
	}
	else
	{
		DelUpdate(&TrailManager::Update);
		DelUpdate(&TrailManager::Draw);
	}

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}

void TrailManager::Command(const char *id, dword numParams, const char **params)
{
	//
}

dword __stdcall TrailManager::WorkThread(LPVOID lpParameter)
{
	TrailManager * tm = (TrailManager *)lpParameter;

	HANDLE m_hWaitObjects[] = {tm->m_hStartEvent, tm->m_hExitEvent};
	
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(sizeof(m_hWaitObjects)/sizeof(m_hWaitObjects[0]), m_hWaitObjects, FALSE, INFINITE);
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		tm->Execute( tm->deltaTime );

		SetEvent( tm->m_hDoneEvent );
	}

	return 0;
}

void _cdecl TrailManager::Update(float dltTime, long level)
{
	if( !pVBuffer )
		return;

	const Plane * rs_frustum = Render().GetFrustum();
	for (int i=0; i<6; i++)
		frustum[i] = rs_frustum[i];

	deltaTime = dltTime;

	if (m_isMultiThreading)
		SetEvent( m_hStartEvent );
	else
		Execute(deltaTime);
}

void TrailManager::Execute(float dltTime)
{
	if( !pVBuffer )
		return;

	if( dltTime < 0.00003f )
		return;

	for( long i = 0 ; i < trails ; i++ )
	{
		if( !trails[i]->busy )
			continue;

		Trail &trail = *trails[i];

	//	Matrix m = trail.p->GetMatrix(m);
		Matrix m = trail.m;

	//	m.Normalize();

		float r = trail.lr;

		Vector ds = m.pos - trail.lm.pos;
	
	//	float l = ds.GetLength();
		float l = ds.Normalize();

		if( trail.started || l > r || trail.time > 0.3f )
		{
			if( !trail.started )
			{
				if( l >= r )
				{
				//	ds.Normalize();

					Matrix n = m;

					n.pos = trail.lm.pos;

					while( l >= r )
					{
						float s = MyRRnd(0.5f,1.5f)*trail.rBeg*0.5f;

						trail.rad = s;

						n.pos += ds*(r + s);

						Block &block = reg.Get();

						block.pos  = n.pos;
						block.show = true;

					//	block.time = 0.0f;
						block.time = coremax(0.0f,dltTime*(1.0f - (r + s)/l));

					//	block.live = 2.5f;
						block.live = MyRRnd(trail.liveMin,trail.liveMax);

						block.rk = trail.rEnd/trail.rBeg;

						block.windDir = trail.windDir;
					//	block.windVel = trail.windVel;

						block.force = -n.vz*trail.forceVal;

						block.i = trail.li++;

						trail.lm = n;

						const int off_cnt = 4;

						if( trail.cnt > off_cnt )
						{
							Matrix mmm = m;

							mmm.pos = 0.0f;

							Matrix rot;

							rot.RotateZ(MyRRnd(0.0f,2*PI));

							rot = rot*mmm;

							trail.beg = trail.end;

							trail.end = rot.vx;
						//	trail.end.Normalize();
							trail.end = trail.end*MyRRnd(0.0f,trail.rBeg*0.5f*trail.offStr);

							trail.cnt = 0;
						}
						else
							trail.cnt++;

						Vector v;
						
						v.Lerp(trail.beg,trail.end,trail.cnt/float(off_cnt));

						block.off = v;
						block.r   = s;

						trail.lr = s;

						block.color = trail.color;

						block.cl = MyRRnd(0.70f,1.0f);
					//	block.cl = MyRRnd(0.65f,1.0f);

						l -= r + s*2;
						r  = s;
					}

					trail.time = 0.0f;
				}
				else
				{
					if( trail.staticLive )
					{
						float s = MyRRnd(0.5f,1.5f)*trail.rBeg*0.5f;

						trail.rad = s;

						Block &block = reg.Get();

						block.pos  = m.pos;
						block.show = true;

						block.time = 0.0f;

					//	block.live = 2.5f;
						block.live = MyRRnd(trail.liveMin,trail.liveMax);

						block.rk = trail.rEnd/trail.rBeg;

						block.windDir = trail.windDir;
					//	block.windVel = trail.windVel;

						block.force = -m.vz*trail.forceVal;

						block.i = trail.li++;

						trail.lm = m;

						const int off_cnt = 4;

						if( trail.cnt > off_cnt )
						{
							Matrix mmm = m;

							mmm.pos = 0.0f;

							Matrix rot;

							rot.RotateZ(MyRRnd(0.0f,2*PI));

							rot = rot*mmm;

							trail.beg = trail.end;

							trail.end = rot.vx;
						//	trail.end.Normalize();
							trail.end = trail.end*MyRRnd(0.0f,trail.rBeg*0.5f*trail.offStr);

							trail.cnt = 0;
						}
						else
							trail.cnt++;

						Vector v;
						
						v.Lerp(trail.beg,trail.end,trail.cnt/float(off_cnt));

						block.off = v;
						block.r   = s;

						trail.lr = s;

						block.color = trail.color;

						block.cl = MyRRnd(0.70f,1.0f);
					//	block.cl = MyRRnd(0.65f,1.0f);
					}

					trail.time = 0.0f;
				}
			}
			else
			{
				Block &block = reg.Get();

				block.pos  = m.pos;
				block.show = true;

				block.time = 0.0f;

			//	block.live = 2.5f;
				block.live = MyRRnd(trail.liveMin,trail.liveMax);

				block.rk = trail.rEnd/trail.rBeg;

				block.windDir = trail.windDir;
			//	block.windVel = trail.windVel;

				block.force = -m.vz*trail.forceVal;

				block.i = trail.li++;

				block.r = r;

				block.color = trail.color;

				trail.lr = r;
				trail.lm = m;

				trail.time = 0.0f;

				trail.started = false;
			}
		}
		else
			trail.time += dltTime;
	}

	qverts.Empty();

	for( int j = 0 ; j < reg ; j++ )
	{
		Block &block = reg[j];
		
		if ( !block.show )
			continue;

		block.time += dltTime;

		if( block.time >= block.live )
		{
			block.show = false;	
			reg.Release(j);
			continue;
		}
		else
		{
			float t = block.time;

			if( t < 0.0f )
				t = 0.0f;

			float ttt = 1.0f + t;

			ttt *= ttt*ttt;

			block.pos +=	  block.force*(10.0f*dltTime/ttt);

		}

		Vector pos = block.pos;
		float	 t = block.time;

		if( t < 0.0f )
			t = 0.0f;

		float k = t*0.9f;

		float r = Lerp(1.0f,block.rk,k);

		if( r < 0.0f )
			r = 0.0f;

		float z = block.r*2;

		z *= r;

		float ang = t*0.9f;

		if( block.i&1 )
			ang = -ang;

		pos += block.off*(t*7.0f);

		pos += block.windDir*t;

		z *= 1.4142f;

		if( frustum[1].Dist(pos) < -z ||
			frustum[2].Dist(pos) < -z ||
			frustum[3].Dist(pos) < -z ||
			frustum[4].Dist(pos) < -z )
			continue;

	/*	const float dt = 1.0f/8;

		const float tl = dt*0; const float tr = tl + dt;
		const float tt = dt*0; const float tb = tt + dt;*/

		k = 1.0f - t/block.live;

		float al = k*k;

		float cl = block.cl;

		Color c; 
		Vector in(1.0f,1.0f,0.7f);

		if( t < 0.05f )
		{
			float kk = /*sqrtf*/(t*20.0f);
			Vector v; v.Lerp(Vector(1.0f,0.3f,0.0f),in,kk);
			c = Vector4(v.x,v.y,v.z,Lerp(1.0f,0.5f,kk));
		}
		else
			if( t < 0.1f )
			{
				float kk = /*sqrtf*/((t - 0.05f)*20.0f);
				Vector v; v.Lerp(in,Vector(cl,cl,cl),kk);
				c = Vector4(v.x,v.y,v.z,Lerp(0.5f,1.0f,kk));
			}
			else
				c = Vector4(cl,cl,cl,al);

		c *= block.color;

		float a = 1.0f;

		c.a *= a;
		
		c.Clamp();

		dword col = c;

		QVertex & v = qverts[qverts.Add()];

		v.pos = pos;
		v.col = col;
		v.ang = ang;
		v.z = z;
	}
}

const float ANG1 =  PI*0.25f;
const float ANG2 =  PI*0.75f;
const float ANG3 = -PI*0.75f;
const float ANG4 = -PI*0.25f;

void _cdecl TrailManager::Draw(float dltTime, long level)
{
	if( !pVBuffer )
		return;

	if (m_isMultiThreading)
		WaitForSingleObject(m_hDoneEvent, INFINITE);

	if( dltTime < 0.00003f )
		dltTime = 0.0f;

	Render().SetWorld(Matrix());

	if( TrailTexture && pFace )
		TrailTexture->SetTexture(pFace);

	if( TrailBumpMap && pBump )
		TrailBumpMap->SetTexture(pBump);

	Vertex *verts = (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);
	Vertex * __restrict p = verts;

	int pn = 0;

	for( int j = 0 ; j < qverts ; j++ )
	{
		if( pn >= trail_max_size )
		{
			pVBuffer->Unlock();

			Render().SetStreamSource(0,pVBuffer);
			Render().SetIndices(pIBuffer,0);

			Render().DrawIndexedPrimitive(Trail_id, 
				PT_TRIANGLELIST,0,pn*4*1,0,pn*2*1);

			pn = 0;

			verts = (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);
			p = verts;
		}

		const QVertex & v = qverts[j];

	/*	const float dt = 1.0f/8;

		const float tl = dt*0; const float tr = tl + dt;
		const float tt = dt*0; const float tb = tt + dt;*/
		const float tl = place.place.t; const float tr = tl + place.place.w;
		const float tt = place.place.u; const float tb = tt + place.place.h;

		p[0].q = v.pos;
		p[0].cl = v.col;
		p[0].tu = tl; p[0].tv = tt;
		p[0].az = v.ang + ANG3;
		p[0].r = v.z;

		p[1].q = v.pos;
		p[1].cl = v.col;
		p[1].tu = tl; p[1].tv = tb;
		p[1].az = v.ang + ANG2;
		p[1].r = v.z;

		p[2].q = v.pos;
		p[2].cl = v.col;
		p[2].tu = tr; p[2].tv = tb;
		p[2].az = v.ang + ANG1;
		p[2].r = v.z;

		p[3].q = v.pos;
		p[3].cl = v.col;
		p[3].tu = tr; p[3].tv = tt;
		p[3].az = v.ang + ANG4;
		p[3].r = v.z;

		p += 4; pn++;
	}

	pVBuffer->Unlock();

	if( pn > 0 )
	{
		Render().SetStreamSource(0,pVBuffer);
		Render().SetIndices(pIBuffer,0);

		Render().DrawIndexedPrimitive(Trail_id, 
			PT_TRIANGLELIST,0,pn*4*1,0,pn*2*1);
	}
}

void TrailManager::CreateBuffers()
{
	RELEASE(pVBuffer)
	RELEASE(pIBuffer)

	pVBuffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*trail_max_size*4,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVBuffer)

	pIBuffer = Render().CreateIndexBuffer(
		sizeof(WORD)  *trail_max_size*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(pIBuffer)

	WORD *p = (WORD*)pIBuffer->Lock();
	Assert(p)

	for( long i = 0 ; i < trail_max_size ; i++ )
	{
		p[i*6 + 0] = WORD(i*4 + 0);
		p[i*6 + 1] = WORD(i*4 + 1);
		p[i*6 + 2] = WORD(i*4 + 2);
		p[i*6 + 3] = WORD(i*4 + 0);
		p[i*6 + 4] = WORD(i*4 + 2);
		p[i*6 + 5] = WORD(i*4 + 3);
	}

	pIBuffer->Unlock();
}

MOP_BEGINLIST(TrailManager, "", '1.00', 100)
MOP_ENDLIST(TrailManager)
