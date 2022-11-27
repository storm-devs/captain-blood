#include "CannonBall.h"

#include "..\..\..\Common_h\ICharactersArbiter.h"
#include <stdlib.h>
#include <stdio.h>

const ConstString FlyBalls::trailManagerId("TrailManager");

FlyBalls::FlyBalls()// : trajectory (_FL_)
{
	flyModel = null;

	CameraTargetHeight = 0.8f;
	explodeLightTime = -10.0f;

	camController = NULL;

	light = NULL;

	e_light = NULL;

	flyGMXModel = NULL;
	pFlyParticles = NULL;
	pFlySound = NULL;

	bfirstTime = true;

	kLerp = 1.0f;


	fActiveTime = 0.0f;
	fActivationDelay = 0.0f;
	LightTime = 0.0f;
	light = NULL;

	bFlyed = false;

	mBallRotation = 0.0f;

	Reset();

	trail1 = NULL;
	trail2 = NULL;

	trailUpdated = -1.0f;
}

FlyBalls::~FlyBalls()
{
	if (light)
	{
		Render().ReleaseLight(light);
	}

	if (e_light)
	{
		Render().ReleaseLight(e_light);
	}

	if (flyGMXModel)
	{
		flyGMXModel->Release();
		flyGMXModel = NULL;
	}

	if (pFlyParticles)
	{
		pFlyParticles->Release();
		pFlyParticles = NULL;
	}

	if (pFlySound)
	{
		pFlySound->Release();
		pFlySound = NULL;
	}

	flyGMXModel = NULL;
	pFlyParticles = NULL;
	pFlySound = NULL;

//	if( trail1 )
//		trail1->Release();

//	if( trail2 )
//		trail2->Release();
}

void FlyBalls::Restart()
{
	end = org_end;

	BuildPath();
	Reset();

	Activate(m_active);
}

void FlyBalls::Reset ()
{
	vPrevPos = start;

	vCameraPos = start;
	vCameraLookTo = start;
	mBallRotation = 0.0f;
	explodeLightTime = -10.0f;
	fActiveTime = 0.0f;
	bfirstTime = true;
	bFlyed = true;
	fActivationDelay = FRAND(randTime);
	fActivationDelay += fMinDelay;
	fActivationDelay = fActivationDelay * fTimeMultipler;

	if (pFlyParticles)
	{
		pFlyParticles->Teleport(Matrix(Vector(0.0f), start));
	}

}
/*
void FlyBalls::BuildTrajectory ()
{
	trajectory.DelAll();

	int n = 50;

	Vector v = end - start; v.y = 0.0f;

	float d = v.Normalize(); d /= 50;
	float x = 0.0f;

	Vector p = start;

	for( int i = 0 ; i < n ; i++ )
	{
		p.y = start.y + m_a*x*x + m_b*x;
		trajectory.Add(p);
		p += v*d;
		x += d;
	}

	return;

	//trajectory.Add(start);
	//trajectory.Add(vMiddlePoint);

	trajectory.DelAll();

	if( start.y == end.y && trajectoryHeight1 <= 0.0f )
	{
		trajectory.Add(start);
		trajectory.Add((start + end)*0.5f);
		trajectory.Add(end);
		return;
	}

//	dword dwSteps = 10;
	dword dwStepsSum = 50;

	if (bRopedCamera)
	{
		dwStepsSum = 300;
	}

	Vector a = end - start;
	a.y = 0.0f;
	Vector b = vMiddlePoint - start;
	b.y = 0.0f;

	Vector vLerp;
	Vector pt;

	dword dwSteps = dword(b.GetLength()/a.GetLength()*dwStepsSum);

	if( dwSteps > 0 )
	{
		vLerp = (vMiddlePoint-start) / (float)dwSteps;
	
		if( kLerp < 1.0f )
		{
			float fMaxHL = 1.0f/dwSteps;
			float fMaxHQ = fMaxHL*fMaxHL;

			for( dword n = 0 ; n <= dwSteps ; n++ )
			{
				pt = start + vLerp*float(n);
				float cval = float(dwSteps - n);
				pt.y = (1.0f - Lerp(cval*fMaxHL,cval*cval*fMaxHQ,kLerp))*trajectoryHeight1;
				pt.y = start.y + pt.y;
				trajectory.Add(pt);
			}
		}
		else
		{
			float fMaxH = 1.0f / ((float)dwSteps * (float)dwSteps);

			for (dword n = 0; n <= dwSteps; n++)
			{
				pt = start + (vLerp * (float)n);
				float cval = (float)(dwSteps - n);
				pt.y = (1.0f - ((cval * cval) * fMaxH)) * (trajectoryHeight1);
				pt.y = start.y + pt.y;
				trajectory.Add(pt);
			}
		}
	}
	
	dwSteps = dwStepsSum - dwSteps;

	if( dwSteps > 0 )
	{		
		vLerp = (end-vMiddlePoint) / (float)dwSteps;

		if( kLerp < 1.0f )
		{
			float fMaxHL = 1.0f/dwSteps;
			float fMaxHQ = fMaxHL*fMaxHL;

			for( dword n = 1 ; n <= dwSteps ; n++ )
			{
				pt = vMiddlePoint + vLerp*float(n);
				float cval = float(n);
				pt.y = (1.0f - Lerp(cval*fMaxHL,cval*cval*fMaxHQ,kLerp))*trajectoryHeight2;
				pt.y = end.y + pt.y;
				trajectory.Add(pt);
			}
		}
		else
		{
			float fMaxH = 1.0f / ((float)dwSteps * (float)dwSteps);

			for (dword n = 1; n <= dwSteps; n++)
			{
				pt = vMiddlePoint + (vLerp * (float)n);

				float cval = (float)(n);
				pt.y = (1.0f - ((cval * cval) * fMaxH)) * (trajectoryHeight2);
				pt.y = end.y + pt.y;
				trajectory.Add(pt);
			}
		}
	}
}
*/
//Создание объекта
bool FlyBalls::Create(MOPReader & reader)
{
	EditMode_Update (reader);
	return true;
}


//Обновление параметров
bool FlyBalls::EditMode_Update(MOPReader & reader)
{
	start = reader.Position();
	end = reader.Position();
	trajectoryTime = reader.Float();

	trajectoryHeight = reader.Float();

	org_end = end;

	org_d = (end - start).GetLength();
	org_t = trajectoryTime;
	org_h = trajectoryHeight;

/*	if( end.y >= start.y )
	{
		if( trajectoryHeight < 0.0f )
			trajectoryHeight = 0.0f;
	}
	else
	{
		float dy = start.y - end.y;

		if( trajectoryHeight < -dy )
			trajectoryHeight = -dy;
	}*/

	randTime = reader.Float();
	fActivationDelay = FRAND(randTime);

	fMinDelay = reader.Float();

	fActivationDelay += fMinDelay;
	fActivationDelay = fActivationDelay * fTimeMultipler;

	startParticles = reader.String();
	startSound = reader.String();

	flyParticles = reader.String();
	bParticlesTrajOrient = reader.Bool();
	flySound = reader.String();
	flyModel = reader.String().c_str();

	endParticles = reader.String();
	endMatrix.Build(reader.Angles(), end);
	endSound = reader.String();

	//// trail ////

	useTrail = reader.Bool();

	trailBegin = reader.Float();
	trailEnd   = reader.Float();

	trailDelayMin = reader.Float();
	trailDelayMax = reader.Float();

	trailColor = reader.Colors();

	trailOffStr = reader.Float();

	trailDir = Matrix(reader.Angles()).vz;
	trailVel = reader.Float();

	forceValue = reader.Float();

	staticLive = reader.Bool();

	///////////////

	trailName = reader.String();

	///////////////

	trailAlpha = reader.Float();

	///////////////

	endTrigger.Init(reader);

	bExplode = reader.Bool();
	fExplodeRadius = reader.Float();
	fExplodeDamage = reader.Float();
	fExplodePower = reader.Float();

	vRotationVector.Rand(Vector(-5.0f), Vector(5.0f));

	////////////

	BuildPath();

	////////////

	bBornLight = reader.Bool();
	LightRadius = reader.Float();
	LightColor = reader.Colors();
	LightColor *= reader.Float();
	LightTime = reader.Float();;

	e_bBornLight = reader.Bool();
	e_LightRadius = reader.Float();
	e_LightColor = reader.Colors();
	e_LightColor *= reader.Float();
	e_LightTime = reader.Float();;

	Activate(m_active = reader.Bool());

	bContinues = reader.Bool();
	bCamShock = reader.Bool();

	masterObject = reader.String();
	masterObjectDist = reader.Float();

	bBallRotation	   = reader.Bool();
	 ballRotationSpeed = reader.Float();

	//// опять трэйл ////

	if( trailName.NotEmpty() )
	{
		if( FindObject(trailName,trailParams))
		{
			TrailParams *p = (TrailParams *)trailParams.Ptr();

			trailUpdated = p->updated;

			////////////////////

			useTrail = p->use;

			trailBegin = p->radBegin;
			trailEnd   = p->radEnd;

			trailDelayMin = p->fadeMin;
			trailDelayMax = p->fadeMax;

			trailColor = p->color;

			trailOffStr = p->offStrength;

			trailDir = p->windDir;
			trailVel = p->windVel;

			forceValue = p->forceValue;

			staticLive = p->staticLive;

			/////////////

			bBallRotation	   = p->ballRot;
			 ballRotationSpeed = p->ballRotSpeed;

			/////////////

			trailAlpha = 1.0f;
		}
		else
			trailUpdated = -1.0f;
	}

	/////////////////////

	bRopedCamera = reader.Bool();
	RopedCameraDistance = reader.Float();
	CameraTargetHeight = reader.Float();

	resetFlyParticles = reader.Bool();

//	BuildTrajectory();

//	fTimeMultipler = trajectory.Size() / trajectoryTime;
//	Vector d = end - start; d.y = 0.0f;
//	fTimeMultipler = d.GetLength()/trajectoryTime;
	fTimeMultipler = 1.0f;

	e_LightTime = e_LightTime * fTimeMultipler;
	LightTime = LightTime * fTimeMultipler;

	fActivationDelay = fActivationDelay * fTimeMultipler;

	if (EditMode_IsOn())
	{
		SetUpdate((MOF_UPDATE)&FlyBalls::EditModeRealize, ML_PARTICLES2);
	}

	if (flyGMXModel)
	{
		flyGMXModel->Release();
		flyGMXModel = NULL;
	}

	if (pFlyParticles)
	{
		pFlyParticles->Release();
		pFlyParticles = NULL;
	}

	if (pFlySound)
	{
		pFlySound->Release();
		pFlySound = NULL;
	}

	pFlyParticles = Particles().CreateParticleSystemEx(flyParticles.c_str(), _FL_);
	if (pFlyParticles)
	{
		pFlyParticles->Teleport(Matrix(Vector(0.0f), start));
		pFlyParticles->PauseEmission(true);
	}

	flyGMXModel = Geometry().CreateScene(flyModel, &Animation(), &Particles(), &Sound(), _FL_);



	Reset();



	MGIterator *iter = &Mission().GroupIterator(GroupId('C','M','C','R'), _FL_);
	if (!iter->IsDone())
	{   
		camController = iter->Get();     
	}
	iter->Release();


	vCameraPos = start;

	//// Trail ////

	MOSafePointer sp;

	Mission().CreateObject(sp,"TrailManager",trailManagerId);

	ITrailManager *tm = (ITrailManager *)sp.Ptr();
	Assert(tm)

	if( useTrail )
	{
		if( !trail2 )
		{
			trail2 = tm->Add();
			Assert(trail2)
		}

		Color c(trailColor);

		c.a *= trailAlpha;

		trail2->SetParams(trailBegin,trailEnd,trailDelayMin,trailDelayMax,
			c,trailOffStr,trailVel,trailDir,forceValue);

		trail2->EnableStaticLive(staticLive);
	}
	else
	{
		if( trail2 )
		{
			trail2->Release();
			trail2 = NULL;
		}
	}

	return true;
}

void FlyBalls::BuildPath()
{
	trajectoryHeight1 = trajectoryHeight;
	trajectoryHeight2 = trajectoryHeight - (end.y - start.y);

	vMiddlePoint = (start + end)*0.5f + Vector(0.0f,trajectoryHeight,0.0f);

	vMiddlePoint.y = start.y + trajectoryHeight1;

	////

	Vector vx = end - start; vx.y = 0.0f;

	float y2 = vMiddlePoint.y - start.y;

	float x3 = vx.Normalize();
	float y3 = end.y - start.y;

	m_a = (2*y3 - 4*y2)/(x3*x3);
	m_b = (4*y2 - y3)/x3;

/*	float y = start.y + trajectoryHeight1;

	float k = 1.0f/(1.0f + sqrtf(trajectoryHeight1/trajectoryHeight2));

	vMiddlePoint.Lerp(end,start,k);

	vMiddlePoint.y = y;

	kLerp = 1.0f;

	if( end.y >= start.y )
	{
		if( vMiddlePoint.y < end.y )
		{
			vMiddlePoint = end;

			kLerp = trajectoryHeight1/(trajectoryHeight1 - trajectoryHeight2);

			trajectoryHeight1 = end.y - start.y;
			trajectoryHeight2 = 0.0f;
		}
	}
	else
	{
		if( vMiddlePoint.y < start.y )
		{
			vMiddlePoint = start;

			kLerp = 1.0f - trajectoryHeight1/(trajectoryHeight1 - trajectoryHeight2);

			trajectoryHeight1 = 0.0f;
			trajectoryHeight2 = start.y - end.y;
		}
	}*/

	minY = MIN (start.y, end.y);
	maxY = MAX (start.y, end.y);
	difY = maxY - minY;
}

void _cdecl FlyBalls::EditModeRealize(float fDeltaTime, long level)
{
	if (!EditMode_IsVisible()) return;

	if (!Mission().EditMode_IsAdditionalDraw()) return;

	Render().DrawSphere(end, 0.1f, 0xFFFF0000);
	Render().DrawSphere(start, 0.1f, 0xFF00FF00);


	Render().DrawSphere(vMiddlePoint, 0.1f, 0xFF00FF00);

	//Render().DrawLine(start, 0xFFFFFFFF, vMiddlePoint, 0xFFFFFFFF);
	//Render().DrawLine(end, 0xFFFFFFFF, vMiddlePoint, 0xFFFFFFFF);

	dword dwColor = 0xFFFF0000; 

	bool isSelected = EditMode_IsSelect();
	if (isSelected)
	{
		dwColor = 0xFF0000FF;
	}

	Render().FlushBufferedLines();

/*	if (trajectory.Size() > 0)
	{
		Vector vStart = trajectory[0];
		for (dword n = 1; n < trajectory.Size(); n+=4)
		{
			Render().DrawBufferedLine(vStart, dwColor, trajectory[n], dwColor);
			vStart = trajectory[n];
		}
		Render().DrawBufferedLine(vStart, dwColor, end, dwColor);
	}*/

	int n = 10;

	Vector v = (end - start)/float(n); v.y = 0.0f;

	float d = v.GetLength();
	float x = 0.0f;

	Vector p = start;
	Vector q = p;

	for( int i = 0 ; i < n ; i++ )
	{
		p.y = start.y + m_a*x*x + m_b*x;

		Render().DrawBufferedLine(q,dwColor,p,dwColor);

		q = p;

		p += v;
		x += d;
	}

	Render().DrawBufferedLine(q,dwColor,end,dwColor);

	Render().FlushBufferedLines();

	v.y = m_a*d*d + m_b*d;

	Matrix m;
//	m.BuildView(start, trajectory[1], Vector(0.0f, 1.0f, 0.0f));
	m.BuildView(start, start + v, Vector(0.0f, 1.0f, 0.0f));
	m.Inverse();
	m.pos = start;
	
	Render().DrawMatrix(m);
}

void _cdecl FlyBalls::Realize(float fDeltaTime, long level)
{
//	float dTime = fDeltaTime;

	if( EditMode_IsOn())
	{
		if( !trailParams.Validate())
		{
			if( trailName.NotEmpty() )
				FindObject(trailName,trailParams);
			else
				trailParams.Reset();
		}

		TrailParams *p = (TrailParams *)trailParams.Ptr();

		if( p && p->updated != trailUpdated )
		{
			trailUpdated = p->updated;

			//////////////////////////

			useTrail = p->use;

			trailBegin = p->radBegin;
			trailEnd   = p->radEnd;

			trailDelayMin = p->fadeMin;
			trailDelayMax = p->fadeMax;

			trailColor = p->color;

			trailOffStr = p->offStrength;

			trailDir = p->windDir;
			trailVel = p->windVel;

			forceValue = p->forceValue;

			staticLive = p->staticLive;

			//////////////////////////

			trailAlpha = 1.0f;

			//////////////////////////

			MOSafePointer sp;
			
			Mission().CreateObject(sp,"TrailManager",trailManagerId);

			ITrailManager *tm = (ITrailManager *)sp.Ptr();
			Assert(tm)

			if( useTrail )
			{
				if( !trail2 )
				{
					trail2 = tm->Add();
					Assert(trail2)
				}

				Color c(trailColor);

				c.a *= trailAlpha;

				trail2->SetParams(trailBegin,trailEnd,trailDelayMin,trailDelayMax,
					c,trailOffStr,trailVel,trailDir,forceValue);

				trail2->EnableStaticLive(staticLive);
			}
			else
			{
				if( trail2 )
				{
					trail2->Release();
					trail2 = NULL;
				}
			}

			//////////////////////////////

			bBallRotation	   = p->ballRot;
			 ballRotationSpeed = p->ballRotSpeed;
		}
	}

	Matrix objMatrix(true);

	if( masterObject.NotEmpty() && bfirstTime )
	{
		MOSafePointer sp;

		if( Mission().FindObject(masterObject,sp))
		{
			if((sp.Ptr()->GetMatrix(objMatrix).pos - end).GetLength() > masterObjectDist )
			{
				return;
			}
		}
	}

	Vector vCamPos = Render().GetView().GetCamPos();

	if ((fActiveTime - fActivationDelay) > LightTime && light && light->isEnabled())
	{
			light->Enable(false);
	}

	if (explodeLightTime > 0.0f)
	{
		explodeLightTime -= (fDeltaTime * fTimeMultipler);
		if (explodeLightTime <= 0.0f && e_light)
		{
			e_light->Enable(false);
		}
	}



	if (!bFlyed)
	{
		return;
	}

	fActiveTime += (fDeltaTime * fTimeMultipler);

	bTeleported = false;


	//Ядро активно...
	if (fActiveTime < fActivationDelay)
	{
		return;
	}


	if (bBallRotation)
	{
		mBallRotation += (vRotationVector * fDeltaTime)*ballRotationSpeed;
	}

	if (bfirstTime)
	{
	
		//Тут родить партиклы и звук в start точке....

//		testTimer = 0.0f;

		Matrix m;

		Vector dir = (end - start)*0.01f;
		dir.y = 0.0f;
		float x = dir.GetLength();
		dir.y = m_a*x*x + m_b*x;

	//	m.BuildView(start, trajectory[1], Vector(0.0f, 1.0f, 0.0f));
		m.BuildView(start, start + dir, Vector(0.0f, 1.0f, 0.0f));

		m.Inverse();
		m.pos = start;

		Particles().CreateParticleSystemEx2(startParticles.c_str(), m, true, _FL_);
		Sound().Create3D(startSound, start, _FL_);


		if (pFlyParticles)
		{
			pFlyParticles->Teleport(Matrix(Vector(0.0f), start));
			pFlyParticles->PauseEmission(false);
			bTeleported = true;

			if( resetFlyParticles )
				pFlyParticles->Restart(100);
		}

		if (pFlySound)
		{
			pFlySound->SetPosition(start);
			pFlySound->Play();
		}


		if(!light)
		{
			if (bBornLight)
			{
				light = Render().CreateLight(start, LightColor, LightRadius, 0.0f);
			}
		}else{
			light->Set(start, LightColor, LightRadius, 0.0f);
		}

		if (light)
		{
			light->Enable(true);
		}

		bfirstTime = false;

		if( trail2 )
			trail2->Reset();

	}

//	testTimer += dTime;

	//Время на траектории...
	float fTrajTime = (fActiveTime - fActivationDelay);



//	long numPoints = trajectory.Size();

	//Долетели...
//	if (fTrajTime >= (numPoints-1))
	if (fTrajTime >= trajectoryTime)
	{
		bFlyed = false;

		if( !EditMode_IsOn())
		{
			LogicDebug("CannonBall \"%s\" endTrigger activated:",GetObjectID().c_str());
			endTrigger.Activate(Mission(), false, this);
		}

		//Тут родить партиклы и звук в end точке....

		if (pFlyParticles)
		{
			pFlyParticles->Teleport(Matrix(Vector(0.0f), start));
			pFlyParticles->PauseEmission(true);
		}

		if (pFlySound)
		{
			pFlySound->Stop();
		}


		IParticleSystem *_ps = Particles().CreateParticleSystemEx2(endParticles.c_str(), endMatrix, true, _FL_);
	//	Assert(_ps)

		Sound().Create3D(endSound, end, _FL_);


		if( bExplode )
		{
			MOSafePointer sp;

			static const ConstString objectId("CharactersArbiter");
			if( Mission().FindObject(objectId,sp))
			{
				ICharactersArbiter *arbiter = (ICharactersArbiter *)sp.Ptr();

				if( arbiter )
				{
					arbiter->Boom(this, DamageReceiver::ds_cannon, end, fExplodeRadius, fExplodeDamage, fExplodePower);
				}
			}
		}



		if (bContinues)
		{
			Reset();
		}


		if (bCamShock)
		{
			float dist = Vector (vCamPos - end).GetLength();
			if (dist < 15.0f)
			{
				float power = (1.0f - (dist / 15.0f)) * 0.18f;

				if (camController)
				{
					const char* params[3];

					char buf[16];
					crt_snprintf(buf, sizeof(buf), "%f", power);

					params[0] = buf; //"0.14"; //amplitude
					params[1] = "0.95"; //time
					params[2] = "40.0"; //intensity
					camController->Command("shockPolar", 3, params);
				}
			}
		}


		explodeLightTime = e_LightTime;

		if(!e_light)
		{
			if (e_bBornLight)
			{
				e_light = Render().CreateLight(end, e_LightColor, e_LightRadius, 0.0f);
			}
		}else{
			e_light->Set(end, e_LightColor, e_LightRadius, 0.0f);
		}

		if (e_light)
		{
			e_light->Enable(true);
		}


		return;
	}



/*	float p = fTrajTime;
	if(p < 0.0f) p = 0.0f;
	if(p > float(numPoints - 1)) p = float(numPoints - 1);
	long i1 = long(p);
	long i2 = i1 + 1;
	if(i2 > numPoints - 1) i2 = numPoints - 1;
	float k = p - i1;

	//
	//

	Vector pos;
	pos.Lerp(trajectory[i1], trajectory[i2], k);*/
	float x = fTrajTime/trajectoryTime;

	Vector pos = end - start; pos.y = 0.0f; float d = pos.Normalize();

	x *= d;

	pos = start + pos*x; pos.y = start.y + m_a*x*x + m_b*x;


	//========================================================================


	//========================================================================

	Matrix mtxBall = Matrix (mBallRotation, pos);


	//По траектории ориентация...
	if (bParticlesTrajOrient)
	{
		Vector vDir = (vPrevPos - pos);
		mtxBall.BuildView(Vector(0.0f), vDir, Vector(0.0f, 1.0f, 0.0f));
		mtxBall.pos = pos;
	}


	if (pFlyParticles && !bTeleported)
	{
		pFlyParticles->SetTransform(mtxBall);

		//Render().DrawMatrix(mtxBall);


	}

	if (pFlySound)
	{
		pFlySound->SetPosition(mtxBall.pos);
	}

	if (flyGMXModel)
	{
		if( EditMode_IsOn() && Mission().EditMode_IsAdditionalDraw())
			Render().DrawMatrix(mtxBall);

		flyGMXModel->SetTransform(mtxBall);
		//flyGMXModel->SetTransform(Matrix(Vector(0.0f), mtxBall.pos));
		flyGMXModel->Draw();

		Matrix m;

	//	m.BuildOriented(pos,pos + pos - vPrevPos,Vector(0.0f,0.1f,0.0f));
		m = mtxBall;

		if( trail1 )
			trail1->Update(m);

		if( trail2 )
			trail2->Update(m);
	}

	vPrevPos = pos;

	if (bCamShock)
	{
		float fDistance = Vector (vCamPos - mtxBall.pos).GetLength();
		if (fDistance < 10.0f)
		{
			float power = (1.0f - (fDistance / 10.0f)) * 0.08f;


			if (camController)
			{
				const char* params[3];
				
				char buf[16];
				crt_snprintf(buf, sizeof(buf), "%f", power);

				params[0] = buf; //"0.14"; //amplitude
				params[1] = "0.95"; //time
				params[2] = "40.0"; //intensity
				camController->Command("shockPolar", 3, params);

			}
		}
	}

	vCameraLookTo = (mtxBall.pos + Vector(0.0f, CameraTargetHeight, 0.0f));

	Vector dir = (vCameraLookTo - vCameraPos);
	float fDist = dir.Normalize();
	vCameraPos += dir * Clampf((fDist - RopedCameraDistance), 0.0f, 1000.0f);

	//Render().DrawSphere(pos, 0.5f);
}

void _cdecl FlyBalls::RealizeView(float fDeltaTime, long level)
{

	if (bRopedCamera)
	{
		Matrix mView;
		mView.BuildView(vCameraPos, vCameraLookTo, Vector(0.0f, 1.0f, 0.0f));
		Render().SetView(mView);
	}

	

}

void FlyBalls::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if( isActive )
	{
		Reset();

		if (pFlySound)
		{
			pFlySound->Stop();
		}else{
			pFlySound = Sound().Create3D(flySound, start, _FL_, false, false);
		}

		SetUpdate((MOF_UPDATE)&FlyBalls::Realize	,ML_PARTICLES2);
		SetUpdate((MOF_UPDATE)&FlyBalls::RealizeView,ML_CAMERAMOVE_FREE + 10);
		LogicDebug("Activate");
	}
	else
	{
		Reset();

		DelUpdate((MOF_UPDATE)&FlyBalls::Realize);
		DelUpdate((MOF_UPDATE)&FlyBalls::RealizeView);

		if( pFlyParticles )
			pFlyParticles->PauseEmission(true);

		if( pFlySound )
		{
			pFlySound->Release();
			pFlySound = null;
		}

		LogicDebug("Deactivate");
	}
}

void FlyBalls::Show(bool isShow)
{
	MissionObject::Show(isShow);
	LogicDebug(isShow ? "Show" : "Hide");
	Activate(isShow);
}

void FlyBalls::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	LogicDebug("Command: %s", id);
	if( string::IsEqual(id,"camera_off"))
	{
		bRopedCamera = false;
	}
	else
	if( string::IsEqual(id,"camera_on"))
	{
		bRopedCamera = true;
	}
	else
	if( string::IsEqual(id,"teleport_end"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <teleport> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <teleport> error. Locator name not specified.");
			return;
		}

		MOSafePointer sp;

		FindObject(ConstString(params[0]),sp);
		MissionObject * p = sp.Ptr();

		if( p )
		{
			Matrix m; p->GetMatrix(m);

			end = m.pos;

			//// адаптация высоты и времени полета ////

			float k = (end - start).GetLength()/org_d;

			trajectoryTime	 = org_t*k;
			trajectoryHeight = org_h*k;

			///////////////////////////////////////////

			BuildPath();

			endMatrix.pos = end;

			LogicDebug("Command <teleport>. Teleport by object \"%s\" complete.",params[0]);
		}
		else
		{
			LogicDebugError("Command <teleport> error. Object \"%s\" not found.",params[0]);
			return;
		}
	}
}


MOP_BEGINLISTC(FlyBalls, "Cannon Ball", '1.00', 10000, "Commands:\n camera_off - disable roped camera \n camera_on - enable roped camera \n teleport_end <object name> - teleport end position by object place")

	MOP_POSITION ("Start", Vector(0.0f));
	MOP_POSITION ("End", Vector(0.0f));
	MOP_FLOATEXC ("Trajectory time", 1.0f, 0.01f, 10000.0f, "За сколько времени в сек. ядро пролетит всю траекторию");
	MOP_FLOATEXC ("Trajectory Height", 0.5f, -1000.0f, 1000.0f, "Высота верхней точки траектории");

	MOP_FLOATC ("Activation delay", 0.0f, "Задержка выстрела после активации объекта (рандомом выпадает от 0 до этого времени) + Min_Activation_delay");
	MOP_FLOATC ("Min Activation delay", 0.0f, "Задержка выстрела после активации объекта будет всегда больше этого времени");

	MOP_STRINGC ("Start Particles", "", "Партиклы рождаемые в начале полета");
	MOP_STRINGC ("Start Sound", "", "Звук рождаемый в начале полета");

	MOP_STRINGC ("Fly particles", "", "Партиклы для полета ядра");
	MOP_BOOL ("Particles oriented to trajectory", false);
	MOP_STRINGC ("Fly Sound", "", "Звук для полета ядра");
	MOP_STRINGC ("Fly Model", "", "Модель для полета ядра");

	
	MOP_STRINGC ("End Particles", "", "Партиклы рождаемые в конце полета");
	MOP_ANGLESC ("End Particles orient", Vector(0.0), "Поворот партикловой системы End Particles");
	MOP_STRINGC ("End Sound", "", "Звук рождаемый в конце полета");

	MOP_GROUPBEG("Trail params")

		MOP_BOOL ("Use trail", false);

		MOP_FLOAT("Beg radius", 0.55f);
		MOP_FLOAT("End radius", 2.50f);

		MOP_FLOAT("Min fade delay", 2.0f);
		MOP_FLOAT("Max fade delay", 3.0f);

		MOP_COLOR("Color", Color((dword)-1));

		MOP_FLOAT("Offset strength", 1.0f);

		MOP_GROUPBEG("Wind params")

			MOP_ANGLES("Dir", Vector(-PI*0.5f,0.0f,0.0f))
			MOP_FLOAT ("Vel", 0.0f)

		MOP_GROUPEND()

		MOP_FLOATC("Force value", 0.0f, "Скорость выбрасывания дыма");

		MOP_BOOLC("Enable static live", false, "Генерить дым при остановке");

	MOP_GROUPEND()

	MOP_STRINGC("Trail params", "", "Внешний паттерн с параметрами");

	MOP_FLOAT("Trail alpha", 0.3f);

	MOP_MISSIONTRIGGER("End Trigger");

	MOP_BOOLC ("Explode", false, "Создать физический взрыв в конце полета");
	MOP_FLOATEX("Explode Radius", 2.0f, 0.1f, 1000000.0f)
	MOP_FLOAT("Explode Damage", 100.0f)
	MOP_FLOATEX("Explode Power", 1.0f,0.1f,100.0f)

	MOP_BOOLC ("Shoot Light", false, "Создать источник света при выстреле");
	MOP_FLOATEX("Light Radius", 2.0f, 0.1f, 1000000.0f)
	MOP_COLOR("Light Color", Color(1.0f, 1.0f, 1.0f));
	MOP_FLOATEX("Light Power", 2.0f, 0.1f, 1000000.0f)
	MOP_FLOAT("Light Time", 0.5f)

	MOP_BOOLC ("Explode Light", false, "Создать источник света при взрыве");
	MOP_FLOATEX("Explode Light Radius", 2.0f, 0.1f, 1000000.0f)
	MOP_COLOR("Explode Light Color", Color(1.0f, 1.0f, 1.0f));
	MOP_FLOATEX("Explode Light Power", 2.0f, 0.1f, 1000000.0f)
	MOP_FLOAT("Explode Light Time", 0.5f)

	MOP_BOOL("Active", true)
	MOP_BOOLC("Continues", false, "Автоматически запускает себя еще раз, до бесконечности")
	MOP_BOOLC("Auto shock the camera", false, "Автоматически трясет камеру если рядом пролетает или взрывается")

	MOP_STRINGC("Master Object", "", "Объект до которого измеряется макс. дистанция, что бы работали выстрелы")
	MOP_FLOATC("Maximal Dist To Master Object", 45.0f, "Максимальная дистанция между концом траектории и 'Master Object', для того, что бы ядро стреляло")

	MOP_BOOLC("Ball rotation", true, "Ядро в полете вращается")
	MOP_FLOAT("Ball rotation speed", 1.0f)

	MOP_BOOLC("Roped camera", false, "За ядром на веревке привязана камера")
	MOP_FLOATC("Roped camera rope len", 2.0f, "Длинна веревки на которой за ядром привязана камера")
	MOP_FLOAT("Roped camera target height", 0.8f)

	MOP_BOOLC("Reset Fly particles", false, "Перезапускать партиклы полета при выстреле");

MOP_ENDLIST(FlyBalls)
