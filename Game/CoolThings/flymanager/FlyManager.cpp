#include "FlyManager.h"

const float cloud_delay = 1.0f;

const int swarms_count = 16;
const int clouds_count = 32;

const int fly_max_size = 1024;

FlyManager:: FlyManager() :
	clouds(_FL_,clouds_count),
	swarms(_FL_,swarms_count)
{
//	Swarm::total = 0;
//	Cloud::total = 0;

	pVBuffer = null;
	pIBuffer = null;

	pFace = null;

	loaded = false;

	debug	  = false;
	debugDown = false;
}

FlyManager::~FlyManager()
{
	RELEASE(pVBuffer)
	RELEASE(pIBuffer)

	RELEASE(pFace)

	FlyTexture = null;
}

void _cdecl FlyManager::UpdateMapping(const char *group, MissionObject *sender)
{
	RELEASE(pFace)

	if( TextureMapper::GetMapping(Mission(),place))
	{
		pFace = Render().CreateTexture(_FL_,place.diff);

		if( string::IsEmpty(place.diff))
		{
			LogicDebugError("Fly texture not specified.");
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

		LogicDebugError("TextureMapper not found.");
	}
}

bool FlyManager::Create(MOPReader &reader)
{
	Render().GetShaderId("Fly_", Fly_id);

	CreateBuffers();

	FlyTexture = Render().GetTechniqueGlobalVariable("FlyTexture",_FL_);

	UpdateMapping(null,null);

	SetUpdate(&FlyManager::Draw,ML_ALPHA5);

	swarms.AddElements(swarms_count);

	for( int i = 0 ; i < swarms ; i++ )
	{
		swarms[i].sound = Sound().Create3D("flyer_cloud"/*"m1_torch"*/,0.0f,_FL_,false,false);
	}

	MissionObject::Show	   (true);
	MissionObject::Activate(true);

	if( EditMode_IsOn() || !loaded )
	{
		Registry(GroupId('M','p','p','U'),&FlyManager::UpdateMapping,ML_FIRST);
	}

	return true;
}

IFlysCloud *FlyManager::CreateFlys(IMission &mission, float radius, dword count, bool playSound)
{
	MOSafePointer sp;

	static const ConstString objectId("FlyManager");
	if( mission.CreateObject(sp,"FlyManager",objectId))
	{
		FlyManager *manager = (FlyManager *)sp.Ptr();

		if( manager )
		{
			return manager->CreateFlys(radius,count,playSound);
		}
		else
			return null;
	}

	return null;
}

IFlysCloud *FlyManager::CreateFlys(float radius, dword count, bool playSound)
{
	Cloud *cloud = GetFreeCloud();

	if( cloud )
	{
		cloud->busy = true;

		cloud->Init(radius,count,playSound);

	//	Cloud::total++;
	}

	return cloud;
}

Swarm *FlyManager::GetFreeSwarm()
{
	for( int i = 0 ; i < swarms ; i++ )
		if( !swarms[i].busy )
			return &swarms[i];

	return null;
}

Cloud *FlyManager::GetFreeCloud()
{
	for( int i = 0 ; i < clouds ; i++ )
		if( !clouds[i].busy )
			return &clouds[i];

	if( clouds < clouds_count )
	{
		return &clouds[clouds.Add()];
	}

	return null;
}

void _cdecl FlyManager::Draw(float dltTime, long level)
{
/*	if( api->DebugKeyState(VK_CONTROL,'D'))
	{
		if( debugDown == false )
		{
			debug = !debug;
		}

		debugDown = true;
	}
	else
	{
		debugDown = false;
	}*/

	float fadeBeg2 = place.fadeBeg*place.fadeBeg;
	float fadeEnd2 = place.fadeEnd*place.fadeEnd; float fadeLen2 = 1.0f/(fadeEnd2 - fadeBeg2);

	int cloud_total = 0;

	Vector pos = Render().GetView().GetCamPos();
	const Plane *p = Render().GetFrustum();

	int actis = 0;

	for( int i = 0 ; i < clouds ; i++ )
	{
		Cloud &cloud = clouds[i];

		if( cloud.busy == false )
			continue;

		cloud_total++;

		if( debug )
		{
			Vector a(cloud.pos);
			Vector b(cloud.pos);

			float r = cloud.r + 0.1f;

			a.x -= r;
			a.y -= r*0.5f;
			a.z -= r;

			b.x += r;
			b.y += r*0.5f;
			b.z += r;

			Render().DrawBox(a,b);
		}

		if( debug )
		{
			Vector d = cloud.pos - pos;

		/*	float len = d.GetLength() - cloud.r;

			if( len < place.fadeBeg )*/

			if( d.GetLength2() < fadeBeg2 )
			{
				actis++;

				Vector a(cloud.pos);
				Vector b(cloud.pos);

				float r = cloud.r + 0.2f;

				a.x -= r;
				a.y -= r*0.5f;
				a.z -= r;

				b.x += r;
				b.y += r*0.5f;
				b.z += r;

				Render().DrawBox(a,b,Matrix(),0xff00ff00);
			}
		}

		if( cloud.delay > 0.0f )
		{
			cloud.delay -= dltTime; continue;
		}

		Vector d = cloud.pos - pos;

	/*	float len = d.GetLength() - cloud.r;

		if( len < place.fadeBeg )*/

		if( d.GetLength2() < fadeBeg2 )
		{
			if( !cloud.swarm )
			{
				Swarm *swarm = GetFreeSwarm();

				if( swarm )
				{
					cloud.swarm = swarm;

					swarm->Init(cloud.r,cloud.cnt,cloud.play);

					swarm->pos = cloud.pos;
					swarm->a   = cloud.a;

					swarm->busy = true;

				//	Swarm::total++;
				}
				else
					cloud.delay = cloud_delay;
			}
		}
		else
		{
			if( cloud.swarm )
			{
				cloud.swarm->Release(false);

				cloud.swarm = null;

				cloud.delay = cloud_delay;
			}
		}
	}

	if( debug )
	{
		Render().Print(
			0.0f,0.0f + 8.0f,-1," [clouds] total: %d/32, busy: %d/32, active: %d",
			clouds.Size(),/*Cloud::total*/cloud_total,actis);
	}

	int swarm_total = 0;

	int fades = 0;
	int draws = 0;
	int count = 0;

	Vertex *verts = (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);
	Vertex *v = verts;

	int n = 0;

	for( int i = 0 ; i < swarms ; i++ )
	{
		Swarm &swarm = swarms[i];

		if( swarm.busy )
		{
			swarm.Update(dltTime); swarm_total++;
		}

		if( n >= fly_max_size )
			continue;

		if( swarm.busy == false )
			continue;

		if( swarm.delay > 0.0f )
			fades++;

		if( p[1].Dist(swarm.pos) < -swarm.r ||
			p[2].Dist(swarm.pos) < -swarm.r ||
			p[3].Dist(swarm.pos) < -swarm.r ||
			p[4].Dist(swarm.pos) < -swarm.r )
			continue;

		///////////////////////////

		Vector d = swarm.pos - pos;

	/*	float len = d.GetLength() - swarm.r;

		float beg = place.fadeBeg;
		float end = place.fadeEnd;

		float rad = end - beg;

		if( len > end )
			continue;

		float a = 1.0f;

		if( len > beg && rad > 0.001f )
			a = 1.0f - (len - beg)/rad;*/

		float a = d.GetAttenuation(fadeBeg2,fadeLen2);

	//	Render().Print(0.0f,300,-1," d = %f, a = %f",d.GetLength(),a);

		if( a < 0.01 )
			continue;

		////////////////////

		swarm.Move(dltTime);

		count += swarm.flys;

		if( debug )
		{
			Vector a(swarm.pos);
			Vector b(swarm.pos);

			a.x -= swarm.r;
			a.y -= swarm.r*0.5f;
			a.z -= swarm.r;

			b.x += swarm.r;
			b.y += swarm.r*0.5f;
			b.z += swarm.r;

			Render().DrawBox(a,b,Matrix(),0xffff0000);
		}

		draws++;

		for( int j = 0 ; j < swarm.flys ; j++ )
		{
			if( n >= fly_max_size )
				break;

			Fly &fly = swarm.flys[j];

			float al = a*swarm.a;

			Vector t = swarm.pos + fly.pos;

			const float r = place.size*swarm.k;

			v[0].px = -r; v[0].py = -r;
			v[1].px = -r; v[1].py =  r;
			v[2].px =  r; v[2].py =  r;
			v[3].px =  r; v[3].py = -r;

			const float tl = place.place.t; const float tr = tl + place.place.w;
			const float tt = place.place.u; const float tb = tt + place.place.h;

			v[0].tu = tl; v[0].tv = tb;
			v[1].tu = tl; v[1].tv = tt;
			v[2].tu = tr; v[2].tv = tt;
			v[3].tu = tr; v[3].tv = tb;

			v[0].al = al;
			v[1].al = al;
			v[2].al = al;
			v[3].al = al;

			v[0].q = t;
			v[1].q = t;
			v[2].q = t;
			v[3].q = t;

			v += 4; n++;
		}
	}

	pVBuffer->Unlock();

	if( n > 0 )
	{
		Render().SetStreamSource(0,pVBuffer);
		Render().SetIndices(pIBuffer,0);

		Render().SetWorld(Matrix());

		if( FlyTexture && pFace )
			FlyTexture->SetTexture(pFace);

		Render().DrawIndexedPrimitive(Fly_id,PT_TRIANGLELIST,0,n*4,0,n*2);
	}

	if( debug )
	{
		Render().Print(
			0.0f,0.0f + 8.0f + 18.0f,-1," [swarms] busy: %d/16, fade: %d, draw: %d, flys: %d/1024",
			/*Swarm::total*/swarm_total,fades,draws,count);
	}
}

void FlyManager::CreateBuffers()
{
//	RELEASE(pVBuffer)
//	RELEASE(pIBuffer)

	pVBuffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*fly_max_size*4,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVBuffer)

	pIBuffer = Render().CreateIndexBuffer(
		sizeof(WORD)  *fly_max_size*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(pIBuffer)

	WORD *p = (WORD*)pIBuffer->Lock();
	Assert(p)

	for( long i = 0 ; i < fly_max_size ; i++ )
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

MOP_BEGINLIST(FlyManager, "", '1.00', 100)
MOP_ENDLIST(FlyManager)
