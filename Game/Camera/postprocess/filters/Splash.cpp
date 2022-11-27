//#include "Splash.h"
#include "..\PostProcess.h"
#include "..\..\..\Interface\Utils\InterfaceUtils.h"

const float blood_pla_time = 0.55f;
const float blood_res_time = 0.80f;
const float blood_out_time = 0.80f;

const float blood_rad_min  = 0.40f;
const float blood_rad_max  = 0.50f;

const float blood_safe	   = 0.40f;

const float blood_resp	   = 1.30f;

const int blood_count = 32;

Splash:: Splash() :
	seqs  (_FL_),
	bloods(_FL_,blood_count)
{
	pVBuffer = null;
	pIBuffer = null;

	NatMap = null;

	NorMap = null;
	DifMap = null;

//	diffMap = null;
//	normMap = null;

	pNor = null;
	pDif = null;

	Screen = null;

	bloods.Reserve(blood_count);

	count = 0;

	bloodResp = 0.0f;

	//detailLevel = det_hi;

	m_aspect = 1.0f;

	pServ = null;
}

Splash::~Splash()
{
	if( pVBuffer )
		pVBuffer->Release();
	if( pIBuffer )
		pIBuffer->Release();

	NatMap = NULL;
	NorMap = NULL;
	DifMap = NULL;


	if( pNor )
		pNor->Release();
	if( pDif )
		pDif->Release();

	Screen = NULL;

/*	if( diffMap )
		diffMap->Release();
	if( normMap )
		normMap->Release();*/
}

void Splash::UserInit()
{
	/*#ifndef _XBOX

	IFileService *storage = (IFileService *)api->GetService("FileService");
	Assert(storage)

	IIniFile *ini = storage->SystemIni();//storage->OpenIniFile(api->GetVarString("Ini"),_FL_);

	if( ini )
	{
		const char *s = ini->GetString("PostEffects","Splash","hi");

		if( string::IsEqual(s,"off"))
		{
			detailLevel = det_off;
		}
		else
		if( string::IsEqual(s,"low"))
		{
			detailLevel = det_low;
		}
		else
		{
			detailLevel = det_hi;
		}

		//ini->Release();
	}

	#else

//	detailLevel = det_low;
	detailLevel = det_hi;

	#endif*/

	//if( detailLevel == det_off )
	//	return;

	float cx = (float)Render().GetFullScreenViewPort_3D().Width;
	float cy = (float)Render().GetFullScreenViewPort_3D().Height;

	m_aspect = cx/cy/InterfaceUtils::AspectRatio(Render());

	CreateBuffers();

	NatMap = Render().GetTechniqueGlobalVariable("Splash_NativeTexture" ,_FL_);

	NorMap = Render().GetTechniqueGlobalVariable("Splash_NormalTexture" ,_FL_);
	DifMap = Render().GetTechniqueGlobalVariable("Splash_DiffuseTexture",_FL_);

	Screen = Render().GetTechniqueGlobalVariable("NativeTexture",_FL_);

	const char *norName = "fx_splash_norm.txx";
	const char *difName = "fx_splash_diff.txx";

	/*if( detailLevel == det_low )
	{
		norName = "fx_splash_norm_low.txx";
		difName = "fx_splash_diff_low.txx";
	}*/

	pNor = Render().CreateTexture(_FL_,norName);
	pDif = Render().CreateTexture(_FL_,difName);

	if( !pNor || !pDif )
	{
		api->Trace("\n    SplashFilter: blood textures not found!\n");
	}

	if( !pNor || !pDif )
		return;

//	float w = 1.0f/8;
//	float h = 1.0f/8;
	float w = 1.0f;
	float h = 1.0f;

	int i = 0;

	seqs.Add();

//	for( int j = 0 ; j < 8 ; j++ )
	for( int j = 0 ; j < 1 ; j++ )
	{
		Pos p;

		p.u = j*w; p.w = w;
		p.v = i*h; p.h = h;

		seqs.LastE().poses.Add(p);
	}

	Render().GetShaderId("SimpleQuad",SimpleQuad_id);
	Render().GetShaderId("SplashFilter",SplashFilter_id);

	pServ = (PostProcessService *)api->GetService("PostProcessService");
}

void Splash::FilterImage(IBaseTexture *source, IRenderTarget *destination)
{
//	Assert(detailLevel > det_off)
	//if( detailLevel <= det_off )
	//	return;

	if( !pServ->Enabled())
		return;

	Render().PushRenderTarget();

	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,destination);
	Render().SetViewport(Render().GetFullScreenViewPort_3D());
	Render().BeginScene();

	NatMap->SetTexture(source);
	Screen->SetTexture(source);

	float dx = 1.0f/Render().GetFullScreenViewPort_3D().Width;
	float dy = 1.0f/Render().GetFullScreenViewPort_3D().Height;

	float b[] = {
		-1.0f - dx, 1.0f + dy,0.0f,0.0f,
		-1.0f - dx,-1.0f + dy,0.0f,1.0f,
		 1.0f - dx, 1.0f + dy,1.0f,0.0f,
		 1.0f - dx,-1.0f + dy,1.0f,1.0f};

	Render().DrawPrimitiveUP(SimpleQuad_id,PT_TRIANGLESTRIP,2,b,4*sizeof(float));

	Color c((dword)0xc0c0c0c0);

	Vertex *p = (Vertex *)pVBuffer->Lock(0,0,LOCK_DISCARD);

	int pn = 0;

	for( int i = 0 ; i < bloods ; i++ )
	{
		const Blood &blood = bloods[i];

		if( !blood.busy )
			continue;

		p[0].c = c;
		p[1].c = c;
		p[2].c = c;
		p[3].c = c;

		p[0].p = blood.p[0];
		p[1].p = blood.p[1];
		p[2].p = blood.p[2];
		p[3].p = blood.p[3];

		float alp = blood.a;

		p[0].a = alp;
		p[1].a = alp;
		p[2].a = alp;
		p[3].a = alp;

		float ble = 0.0f;

		float t = blood.playTime - (blood.time - blood_res_time - blood_out_time);

		if( t <= 0.0f )
			continue;

		if( t < blood.playTime )
		{
			float k = t/blood.playTime*(seqs[blood.type].poses - 1);
			float n = floor(k);

			ble = k - n;

			const Pos &p1 = seqs[blood.type].poses[int(n)];

			float tl1 = p1.u; float tr1 = tl1 + p1.w;
			float tt1 = p1.v; float tb1 = tt1 + p1.h;

			p[0].u1 = tl1; p[0].v1 = tb1;
			p[1].u1 = tl1; p[1].v1 = tt1;
			p[2].u1 = tr1; p[2].v1 = tt1;
			p[3].u1 = tr1; p[3].v1 = tb1;

			n++;

			const Pos &p2 = seqs[blood.type].poses[int(n)];

			float tl2 = p2.u; float tr2 = tl2 + p2.w;
			float tt2 = p2.v; float tb2 = tt2 + p2.h;

			p[0].u2 = tl2; p[0].v2 = tb2;
			p[1].u2 = tl2; p[1].v2 = tt2;
			p[2].u2 = tr2; p[2].v2 = tt2;
			p[3].u2 = tr2; p[3].v2 = tb2;
		}
		else
		{
			const Pos &pp = seqs[blood.type].poses[seqs[blood.type].poses - 1];

			float tl = pp.u; float tr = tl + pp.w;
			float tt = pp.v; float tb = tt + pp.h;

			p[0].u1 = tl; p[0].v1 = tb;
			p[1].u1 = tl; p[1].v1 = tt;
			p[2].u1 = tr; p[2].v1 = tt;
			p[3].u1 = tr; p[3].v1 = tb;
		}

		p[0].b = ble;
		p[1].b = ble;
		p[2].b = ble;
		p[3].b = ble;

	//	Render().DrawPrimitiveUP(PT_TRIANGLEFAN,2,p,sizeof(Vertex),"SplashFilter");

		pn++; p += 4;
	}

	pVBuffer->Unlock();

	if( pn > 0 )
	{
		Render().SetStreamSource(0,pVBuffer);
		Render().SetIndices(pIBuffer,0);

		if( NorMap && pNor )
			NorMap->SetTexture(pNor);

		if( DifMap && pDif )
			DifMap->SetTexture(pDif);

		if (!api->DebugKeyState('1'))
			Render().DrawIndexedPrimitive(SplashFilter_id, PT_TRIANGLELIST,0,pn*4,0,pn*2);
	}

	Render().EndScene();

	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
}

void Splash::CreateBuffers()
{
	if( pVBuffer )
		pVBuffer->Release();
	if( pIBuffer )
		pIBuffer->Release();

	pVBuffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*blood_count*4,
		sizeof(Vertex),
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC,POOL_DEFAULT);
	Assert(pVBuffer)

	pIBuffer = Render().CreateIndexBuffer(
		sizeof(WORD)  *blood_count*6,
		_FL_,USAGE_WRITEONLY|USAGE_DYNAMIC);
	Assert(pIBuffer)

	WORD *p = (WORD*)pIBuffer->Lock();
	Assert(p)

	for( long i = 0 ; i < blood_count ; i++ )
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

Splash::Blood *Splash::GetFreeBlood()
{
	for( int i = 0 ; i < bloods ; i++ )
		if( !bloods[i].busy )
			return &bloods[i];

	if( bloods < blood_count )
		return &bloods[bloods.Add()];

	return null;
}

void Splash::Update(float dltTime)
{
	for( int i = 0 ; i < bloods ; i++ )
	{
		Blood &blood = bloods[i];

		if( !blood.busy || blood.owner )
			continue;

		count++;

		blood.time -= dltTime;

		if( blood.time < blood_out_time )
		{
			if( blood.time < 0.0f )
			{
				blood.busy = false;

				count--;
			}
			else
			{
				blood.a = blood.time/blood_out_time;
			}
		}
	}

	Enable(count > 0);

	count = 0;
}

ITexture* Splash::GetInputTexture()
{
	return Render().GetPostprocessTexture();
}

void Splash::AddBloodPuff(MissionObject *owner)
{
	if( seqs.Size() <= 0 )
		return;

	Blood *p = GetFreeBlood();

	if( p )
	{
		Blood &blood = *p;

		blood.busy = true;

		blood.type = 0;

		blood.playTime = 0.0f;

		blood.a	   = 1.0f;
		blood.time = blood.playTime + blood_res_time + blood_out_time;

		blood.owner = owner;

		float aspect = m_aspect;

		float rx = 1.0f - blood_safe/aspect;
		float ry = 1.0f - blood_safe;

		//// распределение по экрану ////

		float a = RRnd(0.0f,2*PI);
		float r = RRnd(0.0f,1.0f);

		r = (1.0f - r*r)*0.5f + 0.5f;

		float x = cosf(a)*rx*r;
		float y = sinf(a)*ry*r;

		/////////////////////////////////

		float zz = RRnd(blood_rad_min,blood_rad_max);

		float zx = zz/aspect;
		float zy = zz;

		if( blood.type == 0 )
		{
			zx *= 1.4142f;
			zy *= 1.4142f;

			float a1 = RRnd(0.0f,2*PI);
			float a2 = a1 + PI*0.5f;

			float x1 = cosf(a1)*zx;
			float y1 = sinf(a1)*zy;

			float x2 = cosf(a2)*zx;
			float y2 = sinf(a2)*zy;

			blood.p[0] = Vector(x + x1,y + y1,0.0f);
			blood.p[1] = Vector(x + x2,y + y2,0.0f);
			blood.p[2] = Vector(x - x1,y - y1,0.0f);
			blood.p[3] = Vector(x - x2,y - y2,0.0f);
		}
		else
		{
			blood.p[0] = Vector(x - zx,y - zy,0.0f);
			blood.p[1] = Vector(x - zx,y + zy,0.0f);
			blood.p[2] = Vector(x + zx,y + zy,0.0f);
			blood.p[3] = Vector(x + zx,y - zy,0.0f);
		}
	}
}

void Splash::Update(MissionObject *owner, float dltTime)
{
	for( int i = 0 ; i < bloods ; i++ )
	{
		Blood &blood = bloods[i];

		if( !blood.busy || blood.owner != owner )
			continue;

		count++;

		blood.time -= dltTime;

		if( blood.time < blood_out_time )
		{
			if( blood.time < 0.0f )
			{
				blood.busy = false;

				count--;
			}
			else
			{
				blood.a = blood.time/blood_out_time;
			}
		}
	}
}

void Splash::Release(MissionObject *owner)
{
	for( int i = 0 ; i < bloods ; i++ )
	{
		Blood &blood = bloods[i];

		if( blood.busy && blood.owner == owner )
		{
			blood.busy = false;
			count--;
		}
	}	
}
