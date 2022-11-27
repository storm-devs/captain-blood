#include "Track.h"
#include "TrackService.h"

const int drop_count = TrackManager::drop_count;

WaterTrack::WaterTrack() : objectMatrix(false),
	bakes(_FL_),
	verts(_FL_),
	burun(_FL_),
	drops(_FL_,drop_count)
{
	fTrackHeight = 0.7f;

	fTeleportDistance = 1.0f;

	fGlobalScale = 1.0f;

	currentV = 0.0f;

	pTrailTexture = NULL;
	FoamLifeTime = 3.0f;

	BurunParticle = NULL;
	BurunBackParticle = NULL;

	fGlobalTime = 0.0f;

	trackTex1 = NULL;
	trackTex2 = NULL;
	trackTexBlend = NULL;

	pTrackTexture[0] = null;
	pTrackTexture[1] = null;
	pTrackTexture[2] = null;
	pTrackTexture[3] = null;

	pMaster.Reset();

	bPrevPosInited = false;

	fTime = 0.0f;

	frame = 0.0f;
	slide = 0.0f;

	bSmoothSpeedInited = false;
	fSmoothSpeed = 0.0f;

	m_speed = 0.0f;

	m_index = 0;

	manager = null;

	m_k1 = 1.0f;
	m_k2 = 1.0f;


	pFace = null;

	loaded = false;
}


WaterTrack::~WaterTrack()
{
	Texture = null;
	trackTexBlend = null;

	trackTex1 = null;
	trackTex2 = null;

	RELEASE(pFace);

	RELEASE(pTrailTexture);

	RELEASE(pTrackTexture[0]);
	RELEASE(pTrackTexture[1]);
	RELEASE(pTrackTexture[2]);
	RELEASE(pTrackTexture[3]);

	RELEASE(BurunParticle);
	RELEASE(BurunBackParticle);
}

void _cdecl WaterTrack::UpdateMapping(const char *group, MissionObject *sender)
{
	RELEASE(pFace)

	if( TextureMapper::GetMapping(Mission(),place))
	{
		pFace = Render().CreateTexture(_FL_,place.diff);

		if( string::IsEmpty(place.diff))
		{
			LogicDebugError("Water texture not specified.");
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

//Создание объекта
bool WaterTrack::Create(MOPReader &reader)
{
	loaded = false;

	Render().GetShaderId("SeaTrack", SeaTrack_id);
	Render().GetShaderId("Fly", Fly_id);

	Texture = Render().GetTechniqueGlobalVariable("FlyTexture",_FL_);

	UpdateMapping(null,null);

	MOSafePointer sp;
	static const ConstString objectId("TrackManager");
	Mission().CreateObject(sp,"TrackManager",objectId);

	manager = (TrackManager *)sp.Ptr();
	Assert(manager)

	trackTexBlend = Render().GetTechniqueGlobalVariable("TrackBlend", _FL_);

	trackTex1 = Render().GetTechniqueGlobalVariable("TrackTexture1", _FL_);
	trackTex2 = Render().GetTechniqueGlobalVariable("TrackTexture2", _FL_);

	EditMode_Update(reader);

	SetUpdate((MOF_UPDATE)&WaterTrack::Work,ML_ALPHA4);

	if( EditMode_IsOn() || !loaded )
	{
		Registry(GroupId('M','p','p','U'),&WaterTrack::UpdateMapping,ML_FIRST);
	}

	return true;
}

//Инициализировать объект
bool WaterTrack::EditMode_Create(MOPReader &reader)
{
	return Create(reader);
}

//Обновление параметров
bool WaterTrack::EditMode_Update(MOPReader &reader)
{
	MissionObjectName = reader.String();

	FindObject(MissionObjectName,pMaster);

	ConstString BurunParticles = reader.String();
	RELEASE(BurunParticle);
	BurunParticle = Particles().CreateParticleSystemEx(BurunParticles.c_str(),_FL_);

	BurunParticles = reader.String();
	RELEASE(BurunBackParticle);
	BurunBackParticle = Particles().CreateParticleSystemEx(BurunParticles.c_str(),_FL_);

	fGlobalScale = reader.Float();

	back_offset = reader.Position()*fGlobalScale;

	const char *szTextureName = NULL;
	IBaseTexture *tex = NULL;

	for( dword n = 0 ; n < 4 ; n++ )
	{
		szTextureName = reader.String().c_str();
		tex = Render().CreateTexture(_FL_, szTextureName);
		RELEASE(pTrackTexture[n]);
		pTrackTexture[n] = tex;
	}

	waterLevel = reader.Float();
	fTrackHeight = reader.Float();

	offset = reader.Position() * fGlobalScale;

	beg.p = reader.Position(); beg.p.y = offset.y;
	beg.t = reader.Position(); beg.t.y = offset.y;

	end.p = reader.Position(); end.p.y = offset.y;
	end.t = reader.Position(); end.t.y = offset.y;

	long count = reader.Long();

	Vector pos = reader.Position();
	Vector ang = reader.Angles();

	bakesMatrix.Build(ang,pos);

	maxDistToBorn = reader.Float()*fGlobalScale; m_delta = maxDistToBorn;
	maxDistToBorn = maxDistToBorn*maxDistToBorn;

	BornExpansion = reader.Float()*fGlobalScale;
	ExpansionSpeed = reader.Float()*fGlobalScale;

	FoamLifeTime = reader.Float()*fGlobalScale;

	fFoamAnimWidth = reader.Float()*fGlobalScale;

	BuildPath(count);

	slide = 0.0f;

	m_speed = 0.0f; bPrevPosInited = false; bSmoothSpeedInited = false;
	fSmoothSpeed = 0.0f;
	fTime = 0.0f;

	szTextureName = reader.String().c_str();
	tex = Render().CreateTexture(_FL_, szTextureName);
	RELEASE(pTrailTexture);
	pTrailTexture = tex;
	
	fTeleportDistance = reader.Float()*fGlobalScale;
	fTeleportDistance = fTeleportDistance*fTeleportDistance;

	bPrevPosInited = false;

	bWorkOnlyWithActive	 = reader.Bool();
	bWorkOnlyWithVisible = reader.Bool();

	fSpeedFactor = reader.Float();
	fAlphaScale	 = reader.Float()*0.01f;

	m_speedK = reader.Float();

	noSwing = reader.Bool();

	bDrawDebug = reader.Bool();

	Activate(reader.Bool());

	Show(true);
	return true;
}

void WaterTrack::Restart()
{
	pMaster.Reset();

	if( MissionObjectName.NotEmpty() )
		FindObject(MissionObjectName,pMaster);

	ObjectPath.clear();
	m_speed = 0.0f; bPrevPosInited = false; bSmoothSpeedInited = false; slide = 0.0f;
	fTime = 0.0f;
	fSmoothSpeed = 0.0f;
}

Vector CubicBezier_(Vector &p0, Vector &p1, Vector &p2, Vector &p3, float mu)
{
	Vector a,b,c,p;

	c = 3.0f*(p1 - p0);
	b = 3.0f*(p2 - p1) - c;
	a = p3 - p0 - c - b;

	p = a*mu*mu*mu + b*mu*mu + c*mu + p0;

	return p;
}

void WaterTrack::BuildPath(long count)
{
	bakes.DelAll();

	Vector beg_p = bakesMatrix.MulVertexByInverse(beg.p);
	Vector beg_t = bakesMatrix.MulVertexByInverse(beg.t);

	Vector end_p = bakesMatrix.MulVertexByInverse(end.p);
	Vector end_t = bakesMatrix.MulVertexByInverse(end.t);

	float stp = 1.0f/count;

	for( float t = 0.0f ; t < 1.0f + stp ; t += stp )
	{
		Bake &bake = bakes[bakes.Add()];

		bake.p = CubicBezier_(beg_p,beg_t,end_t,end_p,t);
		bake.time = 0.0f;
	}

	Vector p;
	Vector q; Vector up(0.0f,1.0f,0.0f);

	for( int i = 0 ; i < bakes ; i++ )
	{
		if( i < 1 )
		{
			Vector t = beg_p; t.x = 0.0f;

			p = bakes[i].p - t;
		}
		else
			p = bakes[i].p - bakes[i - 1].p;

		if( i < bakes - 1 )
		{
			q = bakes[i + 1].p - bakes[i].p;
		}
		else
			q = Vector(0.0f,0.0f,-1.0f);

		p.Lerp(p,q,0.5f);

		p = p^up;
		p.Normalize();

		bakes[i].n	 = p;
		bakes[i].len = 0.2f*fFoamAnimWidth*2.0f + i/float(count - 1)*1.5f;
	}

	for( int i = 0 ; i < bakes ; i++ )
	{
		bakes[i].q = bakes[i].p; bakes[i].q.x *= -1.0;
		bakes[i].m = bakes[i].n; bakes[i].m.x *= -1.0;

		bakes[i].p = bakesMatrix.MulVertex(bakes[i].p);
		bakes[i].n = bakesMatrix.MulNormal(bakes[i].n);

		bakes[i].q = bakesMatrix.MulVertex(bakes[i].q);
		bakes[i].m = bakesMatrix.MulNormal(bakes[i].m);
	}

	count = bakes;

	verts.DelAll();
	verts.AddElements((count + 0)*4);

	long n = count/3;

	if( n < 1 )
		n = 1;

	burun.DelAll();
	burun.AddElements((n + 1)*4);
}

WaterTrack::Drop *WaterTrack::GetFreeDrop()
{
	for( int i = 0 ; i < drops ; i++ )
		if( !drops[i].busy )
			return &drops[i];

	if( drops < drop_count )
		return &drops[drops.Add()];

	return null;
}

void _cdecl WaterTrack::Work(float fDeltaTime, long level)
{
	GetMatrix(objectMatrix);

	if( !pMaster.Ptr() && MissionObjectName.NotEmpty() )
	{
		FindObject(MissionObjectName,pMaster);

		if( !pMaster.Ptr())
			 MissionObjectName.Empty();
	}

	if( !pMaster.Ptr())
	{
		ObjectPath.clear();
		m_speed = 0.0f; bPrevPosInited = false; bSmoothSpeedInited = false; slide = 0.0f;
		fTime = 0.0f;
		fSmoothSpeed = 0.0f;
		return;
	}

	if( !EditMode_IsOn())
	{
		if( !IsActive() || IsDead())
		{
			ObjectPath.clear();
			m_speed = 0.0f; bPrevPosInited = false; bSmoothSpeedInited = false; slide = 0.0f;
			fTime = 0.0f;
			fSmoothSpeed = 0.0f;
			return;
		}

		if( bWorkOnlyWithActive )
		{
			if( !pMaster.Ptr()->IsActive())
			{
				ObjectPath.clear();
				m_speed = 0.0f; bPrevPosInited = false; bSmoothSpeedInited = false; slide = 0.0f;
				fTime = 0.0f;
				fSmoothSpeed = 0.0f;
				return;
			}
		}

		if( bWorkOnlyWithVisible )
		{
			if( !pMaster.Ptr()->IsShow())
			{
				ObjectPath.clear();
				m_speed = 0.0f; bPrevPosInited = false; bSmoothSpeedInited = false; slide = 0.0f;
				fTime = 0.0f;
				fSmoothSpeed = 0.0f;
				return;
			}
		}
	}

	if( fDeltaTime > 0.0f )
	{
		if( fDeltaTime < 0.02 )
			fDeltaTime = 0.02f;
	}

	fGlobalTime += fDeltaTime;

	backPos = objectMatrix * back_offset;
	backPos.y = fTrackHeight;

	fTime += fDeltaTime; 

	curPos = objectMatrix * offset;
	curPos.y = fTrackHeight;

	if( BurunParticle || BurunBackParticle )
	{
		Matrix m = objectMatrix;
		if( BurunParticle )
		{
			m.pos = curPos;
			BurunParticle->SetTransform(m);
		}
		if( BurunBackParticle )
		{
			m.pos = backPos;
			BurunParticle->SetTransform(m);
		}
	}

	Matrix view = Render().GetView();

	if( !noSwing )
	{
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
	}

	if( bDrawDebug )
	{
		Vector prev_p = objectMatrix.MulVertex(beg.p);
		Vector prev_q = objectMatrix.MulVertex(beg.p);

		for( int i = 0 ; i < bakes ; i++ )
		{
			Vector p = objectMatrix.MulVertex(bakes[i].p);
			Vector n = objectMatrix.MulNormal(bakes[i].n);

			Vector q = objectMatrix.MulVertex(bakes[i].q);
			Vector m = objectMatrix.MulNormal(bakes[i].m);

			Render().DrawLine(prev_p,0xffffff00,p,0xffffff00);
			Render().DrawLine(prev_q,0xffffff00,q,0xffffff00);

			Render().DrawLine(p,0xffffff00,p + n*bakes[i].len*2.0f,0xffffff00);
			Render().DrawLine(q,0xffffff00,q + m*bakes[i].len*2.0f,0xffffff00);

			Render().DrawSphere(p,0.2f,0xffff0000);
			Render().DrawSphere(q,0.2f,0xffff0000);

			prev_p = p;
			prev_q = q;
		}

		Vector beg_p = objectMatrix.MulVertex(beg.p);
		Vector end_p = objectMatrix.MulVertex(end.p);

		Vector beg_t = objectMatrix.MulVertex(beg.t);
		Vector end_t = objectMatrix.MulVertex(end.t);

		Render().DrawLine(beg_p,0xffff0000,beg_t,0xffff0000);
		Render().DrawLine(end_p,0xffff0000,end_t,0xffff0000);

		Render().DrawSphere(beg_p,0.4f,0xff0000ff);
		Render().DrawSphere(end_p,0.4f,0xff0000ff);

		Render().DrawSphere(beg_t,0.2f,0xffffff00);
		Render().DrawSphere(end_t,0.2f,0xffffff00);

		Vector q = objectMatrix.MulVertex(bakes[0].q);

		Render().DrawSphere(q,0.4f,0xff0000ff);

		q = objectMatrix.MulVertex(bakes.LastE().q);

		Render().DrawSphere(q,0.4f,0xff0000ff);
	}

	float fSpeed = 0.0f;

	if( fDeltaTime > 0.0f )
	{
		if( bPrevPosInited )
		{
			float distance = (vPrevPos - backPos).GetLength();

			if( distance < maxDistToBorn )
			{
				if( fTime > 0.001f )
				{
					fSpeed = distance/fTime;
					fTime = 0.0f;

					m_speed = Lerp(m_speed,fSpeed,0.1f);
				}
			}
			else
			{
				m_speed = 0.0f; fTime = 0.0f;
			}
		}
		vPrevPos = backPos;
		bPrevPosInited = true;


		if (!bSmoothSpeedInited)
		{
			fSpeed = 0.0f;
			fSmoothSpeed = fSpeed;
			bSmoothSpeedInited = true;
		} else
		{
			fSmoothSpeed = Lerp(fSmoothSpeed,fSpeed,0.1f);
		}
	}


	if (BurunParticle || BurunBackParticle)
	{
		float scaleK = Clampf((fSmoothSpeed-3.0f) / 25.0f);
		if( BurunParticle )
			BurunParticle->SetScale(scaleK);
		if( BurunBackParticle )
			BurunBackParticle->SetScale(scaleK);
	}


	Color alpha(0.85f);
	float blendK = Clampf((fSmoothSpeed-2.0f) / 25.0f);
	alpha.Lerp(Color(0.03f), Color(0.85f), blendK);
	alpha.a = alpha.r;
	dword dwAlpha = alpha.GetDword();

	float animSpeed = 10.0f;

	slide -= (fSmoothSpeed / 120.0f) * fDeltaTime/fFoamAnimWidth;

	frame += fDeltaTime * animSpeed;
	if (frame >= 4.0f)
	{
		frame -= 4.0f;
	}

	long FrameIndexLong = fftol(frame);
	float FrameBlendK = (frame - FrameIndexLong);


	if (trackTexBlend)
	{
		trackTexBlend->SetFloat(FrameBlendK);
	}
	
	if (trackTex1)
	{
		trackTex1->SetTexture(pTrackTexture[FrameIndexLong % 4]);
	}
	if (trackTex2)
	{
		trackTex2->SetTexture(pTrackTexture[(FrameIndexLong+1) % 4]);
	}

	float len = 0.0f; int cnt = bakes;

	float offset_ = slide*3.0f;
	float sk = fSmoothSpeed/100.0f;

	float sk_ = sk*1.5f; if( sk_ > 1.0f ) sk_ = 1.0f;

	for( int i = 0 ; i < cnt ; i++ )
	{
		float k = i/float(cnt - 1);

		Vert &p = verts[i*2 + 0];
		Vert &q = verts[i*2 + 1];

		Vert &r = verts[i*2 + 0 + cnt*2];
		Vert &s = verts[i*2 + 1 + cnt*2];

		float n_len = bakes[i].len*4.0f*2.0f*(0.9f - sk*0.7f)*fFoamAnimWidth*2.0f;

		p.p = bakes[i].p;
		q.p = bakes[i].n*n_len + p.p;

		float off = Lerp(
			sin(fGlobalTime*2.0f + i*2.0f/* - slide*/),
			cos(0.5f*PI*i/(cnt - 1)),
			sk_)*fFoamAnimWidth;

		float shi =	sin((fGlobalTime + i*0.03f)*5.0f)*fFoamAnimWidth*k*k*sk_*6.0f;

		q.p -= bakes[i].n*(off + shi);

		r.p = bakes[i].q;
		s.p = bakes[i].m*n_len + r.p;

		s.p -= bakes[i].m*(off - shi);

		if( i == cnt - 2 )
		{
			float k = 1.0f + sk*0.5f*fFoamAnimWidth*2.0f;

			q.p -= bakes[i].n*k;
			s.p -= bakes[i].m*k;
		}
		else
		if( i == cnt - 1 )
		{
			float k = 1.0f + sk*0.5f*fFoamAnimWidth*2.0f;

			q.p -= bakes[i].n*k;
			s.p -= bakes[i].n*k;
		}

		p.p = objectMatrix.MulVertex(p.p);
		q.p = objectMatrix.MulVertex(q.p);

		p.p.y = offset.y;

		p.p.y += off*0.5f*(1.0f + sk*2.0f);
		q.p.y = waterLevel;

		r.p = objectMatrix.MulVertex(r.p);
		s.p = objectMatrix.MulVertex(s.p);

		r.p.y = offset.y;

		r.p.y += off*0.5f*(1.0f + sk*2.0f);
		s.p.y = waterLevel;

		Color c;
		
		c.a = (1.0f - k*k*0.9f)*(0.3f + sk*0.7f);

		p.c = c;
		q.c = c;

		r.c = c;
		s.c = c;

		p.u = 0.8f; p.v = len*0.1f + offset_;
		q.u = 0.0f; q.v = len*0.1f + offset_;

		p.u -= k*0.15f;
		p.u -= 0.2f*(1.0f - sk);

		r.u = p.u;
		s.u = q.u;

		r.v = p.v;
		s.v = q.v;

		if( i < cnt - 1 )
			len += (bakes[i + 1].p - bakes[i].p).GetLength()*(1.0f - k*0.85f)/fFoamAnimWidth*2.0f*0.25f;
	}

	Render().SetWorld(Matrix());

	Render().DrawPrimitiveUP(SeaTrack_id,
		PT_TRIANGLESTRIP,verts/2 - 2,verts.GetBuffer()			,sizeof(Vert));
	Render().DrawPrimitiveUP(SeaTrack_id,
		PT_TRIANGLESTRIP,verts/2 - 2,verts.GetBuffer() + verts/2,sizeof(Vert));

	cnt = burun/4; int ci = burun/2 - 1;

	for( int i = 0 ; i < cnt ; i++ )
	{
		float k = 1.0f - i/float(cnt - 1);

		Vert &p = burun[ci - i*2 + 0];
		Vert &q = burun[ci - i*2 - 1];

		Vert &r = burun[ci + i*2 + 1];
		Vert &s = burun[ci + i*2 + 2];

		float n_len = k*(1.0f + sk*3.0f)*fFoamAnimWidth*2.0f;

		p.p = bakes[i].p;
		q.p = bakes[i].n*n_len + p.p;

		float kk = m_speed/100.0f; if( kk > 1.0f ) kk = 1.0f;

		bakes[i].time -= kk*k*fDeltaTime*10.0f;

		if( bakes[i].time < 0.0f )
		{
			Drop *drop = GetFreeDrop();

			if( drop )
			{
				Vector p1,n1;
				Vector p2,n2;

				if( i )
				{
					p1 = bakes[i - 1].p;
					n1 = bakes[i - 1].n;
				}
				else
				{
					p1.Lerp(bakes[0].p,bakes[0].q,0.5f);
					n1 = bakesMatrix.vz;
				}

				p2 = bakes[i + 1].p;
				n2 = bakes[i + 1].n;

				float pk = RRnd(0.0f,1.0f);
				float zz = RRnd(0.7f,1.0f);

				Vector n;

				n.Lerp(n1,n2,pk);
				n.Lerp(n,Vector(0.0f,1.0f,0.0f),0.7f*kk*k*zz);

				drop->busy = true;

				drop->p.Lerp(p1,p2,pk);
				drop->v = n*3.0f*k*kk*2.0f*2.0f*zz;

				drop->q = objectMatrix.MulVertex(drop->p); drop->q.y = offset.y;
				drop->w = objectMatrix.MulNormal(drop->v);

				drop->time = 0.0f;

				drop->a = p2 - p1;
				drop->a.Normalize();

				bakes[i].time = 0.1f;

				Drop *next = GetFreeDrop();

				if( next )
				{
					next->busy = true;

					next->p = bakesMatrix.MulVertexByInverse(drop->p);
					next->v = bakesMatrix.MulNormalByInverse(drop->v);

					next->p.x *= -1;
					next->v.x *= -1;

					next->p = bakesMatrix.MulVertex(next->p);
					next->v = bakesMatrix.MulNormal(next->v);

					next->q = objectMatrix.MulVertex(next->p); next->q.y = offset.y;
					next->w = objectMatrix.MulNormal(next->v);

					next->time = 0.0f;

					next->a = bakesMatrix.MulNormalByInverse(drop->a);

					next->a.x *= -1;

					next->a = bakesMatrix.MulNormal(next->a);
				}
			}
		}

		r.p = bakes[i].q;
		s.p = bakes[i].m*n_len + r.p;

		p.p = objectMatrix.MulVertex(p.p);
		q.p = objectMatrix.MulVertex(q.p);

		p.p.y = offset.y;

		p.p.y += sk*k*fFoamAnimWidth*2.0f;
		q.p.y  = waterLevel;

		r.p = objectMatrix.MulVertex(r.p);
		s.p = objectMatrix.MulVertex(s.p);

		r.p.y = offset.y;

		r.p.y += sk*k*fFoamAnimWidth*2.0f;
		s.p.y  = waterLevel;

		Color c; c.a = sk*k;

		p.c = c;
		q.c = c;

		r.c = c;
		s.c = c;

		p.u = 0.8f; p.v = i*0.2f + offset_;
		q.u = 0.0f; q.v = i*0.2f + offset_;

		p.u -= (1.0f - k)*0.2f;
		p.u -= 0.2f*(1.0f - sk);

		r.u = p.u;
		s.u = q.u;

		r.v = p.v;
		s.v = q.v;
	}

	Render().DrawPrimitiveUP(SeaTrack_id,
		PT_TRIANGLESTRIP,burun - 2,burun.GetBuffer(),sizeof(Vert));

	PathPoint p;
	p.p = backPos;
	p.nrm = -objectMatrix.vx;
	p.time = 0.0f;
	p.speed = 0.0f;
	p.fDelta = fDeltaTime;
	p.v = currentV;
	currentV += 0.025f * fGlobalScale;

	
	if (!EditMode_IsOn())
	{
		if (ObjectPath.size() <= 0)
		{
			ObjectPath.push(p);

			vPrevPos = backPos;
			bPrevPosInited = true;

		} else
		{
			dword dwLastIndex = ObjectPath.last();
			Vector delta = ObjectPath[dwLastIndex].p - backPos;
			float sqrDist = delta | delta;

			if (sqrDist >= fTeleportDistance)
			{
				ObjectPath.clear();
				fSpeed = 0.0f;
			} else
			{
				if (sqrDist >= maxDistToBorn)
				{
					p.fDelta = fTime;

					p.speed = fSmoothSpeed*m_speedK;

					p.i = m_index++;

					p.a = RRnd(0.3f,1.0f);
					p.b = RRnd(0.3f,1.0f);

					p.k1 = Lerp(m_k1,RRnd(0.5f,1.5f),0.3f);
					p.k2 = Lerp(m_k2,RRnd(0.5f,1.5f),0.3f);

					//Стираются данные автоматом, т.к. это ринг буффер
					ObjectPath.push(p);

					if (ObjectPath.size() >= 3)
					{
						dword dwLastIndex = ObjectPath.last();

						const Vector& v_prev = ObjectPath[dwLastIndex-2].p;
						const Vector& v_cur = ObjectPath[dwLastIndex-1].p;
						const Vector& v_next = ObjectPath[dwLastIndex].p;

						Vector delta1 = v_cur - v_prev;
						Vector delta2 = v_next - v_cur;

						delta1.y = 0.0f;
						delta2.y = 0.0f;

						delta1.Rotate_PI2_CCW();
						delta2.Rotate_PI2_CCW();

						Vector & nrm = ObjectPath[dwLastIndex-1].nrm;
						nrm = (delta1 + delta2);
						nrm.NormalizeXZ();
					}
				}
			}
		}
	}

	

	for (dword n = 0 ; n < ObjectPath.size(); n++)
	{
		ObjectPath[n].time += fDeltaTime;

		if (n == 0)
		{
			if (ObjectPath[n].time > FoamLifeTime)
			{
				ObjectPath.pop();
			}
		}
	}

	
	TrackVertex trail_vb[TRAIL_SIZE*2];
	TrackVertex trail_v_[TRAIL_SIZE*2];


	int polyCount = -2;
	int count = (int)ObjectPath.size()-1;


	float texScale = 0.25f;


	for (int n = 0 ; n < count; n++)
	{

		const Vector& v_prev = ObjectPath[n].p;
		const Vector& v_cur = ObjectPath[n+1].p;
		const Vector& v_nrm = ObjectPath[n+1].nrm;
		const float& v_time = ObjectPath[n+1].time;
		int v_i = ObjectPath[n+1].i;

		float v_glob = ObjectPath[n+1].time;

		float tune = 0.0f;
		float t_tv = 0.0f;

		float k1 = ObjectPath[n+1].k1;
		float k2 = ObjectPath[n+1].k2;

		if( n == count - 1 )
		{
			tune = (objectMatrix.pos - v_cur).GetLength() - m_delta;

			tune = fabsf(tune);

			t_tv = tune/m_delta*(0.025f*fGlobalScale);
		}

		Vector vOffset = v_nrm*(2.5f + (v_time*ExpansionSpeed*1.75f + BornExpansion)*0.5f)*fFoamAnimWidth*2.0f;

		if (bDrawDebug)
		{
			Render().DrawLine(v_prev, 0xFFFFFFFF, v_cur, 0xFFFFFFFF);

			float r = Clampf(0.015f * ObjectPath[n+1].speed);
			Color c(r, r, r);


			Render().Print(v_cur + Vector(0.0f, 5.0f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "%3.2f, %f", ObjectPath[n+1].speed, ObjectPath[n+1].fDelta);


			Render().DrawLine(v_cur, c.GetDword(), v_cur + vOffset, c.GetDword());
			Render().DrawLine(v_cur, c.GetDword(), v_cur - vOffset, c.GetDword());
		}



		Color alpha = (dword)-1;

		alpha.r = 1.0f - v_time/FoamLifeTime*2.0f;

		if( alpha.r < 0.0f )
			alpha.r = 0.0f;

		float fSpeed1 = ObjectPath[n+1].speed;
		float fSpeed2 = ObjectPath[n].speed;

		alpha.a = alpha.r * coremin(fSpeed1, fSpeed2)/100.0f;

		alpha.a = Clampf(alpha.a * fAlphaScale);

		alpha.a *= 0.8f;

		Color alph_(alpha);

		alpha.a *= ObjectPath[n+1].a;
		alph_.a *= ObjectPath[n+1].b;

		dword dw = alpha.GetDword();
		dword d_ = alph_.GetDword();

		float off_ = sin(fGlobalTime*2.0f + v_i*2.0f)*fFoamAnimWidth*1.3f;


		trail_vb[n*2+0].color = dw;
		trail_vb[n*2+1].color = dw;

		Vector center_off = 0.0f;

		Vector vl;
 
		vl = v_cur + k1*v_nrm*((bakes.LastE().p - bakes.LastE().q).GetLength()*0.5f)*0.8f*fFoamAnimWidth*2.0f + v_nrm*off_ - center_off;

		trail_vb[n*2+0].pos = vl - vOffset*1.5f;
		trail_vb[n*2+1].pos = vl + vOffset*1.5f;

		if( n == count - 2 )
		{
			alpha.a *= 0.75f;

			trail_vb[n*2+0].color = alpha;
			trail_vb[n*2+1].color = alpha;

			trail_vb[n*2+1].pos += vOffset*0.1f;
		}
		else
		if( n == count - 1 )
		{
			trail_vb[n*2+0].color = 0;
			trail_vb[n*2+1].color = 0;

			trail_vb[n*2+1].pos += vOffset*0.2f;

			trail_vb[n*2+0].pos += objectMatrix.vz*tune;
			trail_vb[n*2+1].pos += objectMatrix.vz*tune;
		}

		trail_vb[n*2+0].pos.y = fTrackHeight + 1.0f*alpha.a;

		trail_vb[n*2+0].u = 0.95f;
		trail_vb[n*2+1].u = 0.0f;

		trail_vb[n*2+0].v = ObjectPath[n+1].v + t_tv;
		trail_vb[n*2+1].v = trail_vb[n*2+0].v + t_tv;

		//

		trail_v_[n*2+0].color = d_;
		trail_v_[n*2+1].color = d_;

		Vector vr;

		vr = v_cur - k2*v_nrm*((bakes.LastE().p - bakes.LastE().q).GetLength()*0.5f)*0.8f*fFoamAnimWidth*2.0f + v_nrm*off_ + center_off;

		vr += Vector(0.0f,0.1f,0.0f);

		trail_v_[n*2+0].pos = vr + vOffset*1.5f;
		trail_v_[n*2+1].pos = vr - vOffset*1.5f;

		if( n == count - 2 )
		{
			alpha.a *= 0.75f;

			trail_v_[n*2+0].color = alph_;
			trail_v_[n*2+1].color = alph_;

			trail_v_[n*2+1].pos -= vOffset*0.1f;
		}
		else
		if( n == count - 1 )
		{
			trail_v_[n*2+0].color = 0;
			trail_v_[n*2+1].color = 0;

			trail_v_[n*2+1].pos -= vOffset*0.2f;

			trail_v_[n*2+0].pos += objectMatrix.vz*tune;
			trail_v_[n*2+1].pos += objectMatrix.vz*tune;
		}

		trail_v_[n*2+0].pos.y = fTrackHeight + 1.0f*alpha.a;

		trail_v_[n*2+0].u = 0.95f;
		trail_v_[n*2+1].u = 0.0f;

		trail_v_[n*2+0].v = ObjectPath[n+1].v + t_tv + 0.5f;
		trail_v_[n*2+1].v = trail_v_[n*2+0].v + t_tv;

		polyCount+=2;

		
	}

	Render().SetWorld(Matrix());

	if( polyCount > 0 )
	{
		Render().DrawPrimitiveUP(SeaTrack_id,PT_TRIANGLESTRIP,polyCount,trail_vb,sizeof(TrackVertex));
		Render().DrawPrimitiveUP(SeaTrack_id,PT_TRIANGLESTRIP,polyCount,trail_v_,sizeof(TrackVertex));
	}

	for( int i = 0 ; i < drops ; i++ )
	{
		Drop &drop = drops[i];

		if( !drop.busy )
			continue;

		drop.time += fDeltaTime;
	}

	TrackManager::Vertex *v = manager->LockBuffer();

	int pn = 0;

	for( int i = 0 ; i < drops ; i++ )
	{
		Drop &drop = drops[i];

		if( !drop.busy )
			continue;

		Vector t = drop.p;

		float t2 = drop.time*drop.time;

		t.x += 1.2f*drop.v.x*drop.time + 0.5f*drop.a.x*t2;
		t.z += 1.5f*drop.v.z*drop.time + 0.5f*drop.a.z*t2;

		float r = (0.3f + 0.5f*drop.time)*fFoamAnimWidth*2.0f*place.scale;

		v[0].p = Vector(-r,-r,0.0f);
		v[1].p = Vector(-r, r,0.0f);
		v[2].p = Vector( r, r,0.0f);
		v[3].p = Vector( r,-r,0.0f);

		t = objectMatrix.MulVertex(t);

		t -= 0.5f*objectMatrix.vz*15.0f*t2;

		Vector a(0.0f,-5.0f*5.0f,0.0f);

		t.y = drop.q.y + drop.w.y*4.0f*drop.time + a.y*t2;

		if( t.y < waterLevel - r )
		{
			drop.busy = false; continue;
		}

		v[0].q = t;
		v[1].q = t;
		v[2].q = t;
		v[3].q = t;

		const float tl = place.place.t; const float tr = tl + place.place.w;
		const float tt = place.place.u; const float tb = tt + place.place.h;

		v[0].tu = tl; v[0].tv = tb;
		v[1].tu = tl; v[1].tv = tt;
		v[2].tu = tr; v[2].tv = tt;
		v[3].tu = tr; v[3].tv = tb;

		Color c(0xffffffff);

		c.a *= drop.time*2.0f;

		dword col = c.GetDword();

		v[0].cl = col;
		v[1].cl = col;
		v[2].cl = col;
		v[3].cl = col;

		v += 4; pn++;
	}

	manager->UnlockBuffer();

	if( pn > 0 )
	{
		manager->Prepare();

		Render().SetWorld(Matrix());

		if( Texture && pFace )
			Texture->SetTexture(pFace);

		Render().DrawIndexedPrimitive(Fly_id,PT_TRIANGLELIST,0,pn*4,0,pn*2);
	}

	if( !noSwing )
	{
		Render().SetView(view);
	}
}

void WaterTrack::Activate(bool bIsActive)
{
	MissionObject::Activate(bIsActive);
}

//Получить матрицу объекта
Matrix & WaterTrack::GetMatrix(Matrix & mtx)
{
	if( !pMaster.Validate())
	{
		mtx.SetIdentity();
		return mtx;
	}

	if( pMaster.Ptr())
	{
		pMaster.Ptr()->GetMatrix(mtx);
		mtx.pos.y = offset.y;
	}else{
		mtx.SetIdentity();
	}
	return mtx;
}

MOP_BEGINLISTG(WaterTrack, "WaterTrack", '1.00', 999999, "Arcade Sea")

		MOP_STRINGC("Mission object name", "", "С этого объекта будет считана позиция и угол для пены")

		MOP_STRINGC("Burun particles", "", "Имя партиклов для бурунов с носа корабля")
		MOP_STRINGC("Burun back particles", "", "Имя партиклов для бурунов с кормы корабля")

		MOP_FLOATC("Track global scale", 1.0f, "Для масштабирования настроеного трека, под размер корабля")
		

		MOP_POSITIONC("BackOffset", Vector(0.000000f, 0.500000f, -8.966932f), "Позиция для расчета следа от киля");

		MOP_STRINGC("BortsTexture_1", "hulltrail00.txx", "Текстура пены вокруг бортов, кадр 1");
		MOP_STRINGC("BortsTexture_2", "hulltrail01.txx", "Текстура пены вокруг бортов, кадр 2");
		MOP_STRINGC("BortsTexture_3", "hulltrail02.txx", "Текстура пены вокруг бортов, кадр 3");
		MOP_STRINGC("BortsTexture_4", "hulltrail03.txx", "Текстура пены вокруг бортов, кадр 4");

		MOP_FLOATC("Water height", 0.0f, "Уровень воды");
		MOP_FLOATC("Water Track height", 0.7f, "Высота трека над водой");

		MOP_POSITIONC("Offset", Vector(0.000000f, 0.007686f, 24.744034f), "Позиция носовой части корабля");

		MOP_POSITION("Beg pos"	  , 0.0f);
		MOP_POSITION("Beg tangent", 0.0f);

		MOP_POSITION("End pos"	  , 0.0f);
		MOP_POSITION("End tangent", 0.0f);

		MOP_LONGEX("Bakes count", 10, 2, 30);

		MOP_POSITION("Border pos", 0.0f);
		MOP_ANGLES("Border ang", 0.0f);

		MOP_FLOATC("Dist to born", 5.0f, "Дистанция пройдя которую корабль рождает килевую пену");
		MOP_FLOATC("Born expansion", 8.477020f, "Ширина килевой пены в момент рождения");
		MOP_FLOATC("Expansion speed", 1.800000f, "Скорость расширения килевой пены");
		MOP_FLOATC("Foam seconds lifetime", 9.0f, "Время жизни килеой пены (пена в любом случае не может состоять более чем из 256 сегентов)");


		MOP_FLOATC("Bort Foam animation width", 0.5f, "На сколько сильно бортовая пена колышется...");

		MOP_STRINGC("Foam Texture", "shiptrail.txx", "Текстура килевой пены");

		MOP_FLOATC("Teleport distance", 50.0f, "Дистанция пройдя которую перемещение считается телепортацией, пена обнуляется");

		MOP_BOOLC("Work only with active objects", true, "Отображать трейл, только за активными объектами");
		MOP_BOOLC("Work only with visible objects", true, "Отображать трейл, только за видимыми объектами");


		MOP_FLOATC("Back Foam Alpha Speed K", 10.0f, "Коэфицент регулирующий зависимость прозрачности от скорости м/сек (чем меньше, тем раньше яркая станет)");

		MOP_FLOATC("Back Foam Alpha Percent", 100.0f, "Коэфицент общую прозрачность килевой пены, в процентах от оригинала");

		MOP_FLOAT("Speed aspect", 1.0f);


		MOP_BOOLC("No swing", true, "No swing model in swing machine")


		MOP_BOOLC("DrawDebug", false, "Рисовать debug");

		MOP_BOOLC("Active", true, "Активность трейла");

MOP_ENDLIST(WaterTrack)
