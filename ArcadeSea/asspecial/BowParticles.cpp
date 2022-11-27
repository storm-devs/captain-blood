#include "..\pch.h"
#include ".\bowparticles.h"
#include "BowWaveEmitter.h"
#include <cmath>

BowParticles::BowParticles(BowWaveEmitter& main) :
main_(main),
partData_(__FILE__, __LINE__),

bowParticlesTexture_(NULL),
varPartTexture_(NULL),
varMInvView_(NULL),
varMWorldViewProj_(NULL),

particlesVB_(NULL),
particlesTVB_(NULL),
particlesIB_(NULL)
{
	
}

BowParticles::~BowParticles(void)
{
	if (bowParticlesTexture_)
		bowParticlesTexture_->Release();
	varPartTexture_ = NULL;
	varMInvView_ = NULL;
	varMWorldViewProj_ = NULL;

	if (particlesVB_)
		particlesVB_->Release();
	if (particlesTVB_)
		particlesTVB_->Release();
	if (particlesIB_)
		particlesIB_->Release();
}

void BowParticles::Init()
{
	main_.Render().GetShaderId("BowParticles", BowParticles_id);

	main_.GetParams().isEmissionStopped_ = false;
	CreateBuffers();
}

void BowParticles::Work(float deltaTime)
{
	static float time = 0.0f;
	time += deltaTime;

	
	const float minSpeed = 3.0f;
	const float minAngSpeed = 30.0f*PI/180.0f;
	if (main_.GetHostLinearSpeed().GetLength() < minSpeed && abs(main_.GetHostAngularSpeed()) < minAngSpeed)
		main_.GetParams().isEmissionStopped_ = true;
	else
		main_.GetParams().isEmissionStopped_ = false;

	float speedScale = Min(1.0f, (main_.GetHostLinearSpeed().GetLength() + 0.2f));
	for (unsigned int i = 0; i < partData_.Size(); ++i)
		if (partData_[i].isActive)
		{
			partData_[i].solver.Step(deltaTime*speedScale);
			partData_[i].ttl = partData_[i].solver.GetRelTiming() > main_.GetParams().parabolaTrackCoef ?
				1.0f - (partData_[i].solver.GetRelTiming()-main_.GetParams().parabolaTrackCoef)/(1.0f-main_.GetParams().parabolaTrackCoef) : 1.0f;
			partData_[i].angle -= partData_[i].rotateSpeed*deltaTime;
		}


	EmitParticles(deltaTime);
	UpdatePTransforms(deltaTime);
	RenderPSystem();
}

void BowParticles::CreateBuffers()
{
	if (particlesVB_)
		particlesVB_->Release();
	if (particlesTVB_)
		particlesTVB_->Release();
	if (particlesIB_)
		particlesIB_->Release();
	if (bowParticlesTexture_)
		bowParticlesTexture_->Release();

	unsigned int maxPCount = main_.GetParams().maxPartCount;

	particlesVB_ = main_.Render().CreateVertexBuffer(maxPCount*4*sizeof(PGeometry), sizeof(PGeometry), __FILE__, __LINE__);
	particlesTVB_ = main_.Render().CreateVertexBuffer(maxPCount*4*sizeof(PTransform), sizeof(PTransform), __FILE__, __LINE__);
	particlesIB_ = main_.Render().CreateIndexBuffer(maxPCount*6*sizeof(unsigned short), __FILE__, __LINE__);

	Assert(particlesVB_);
	Assert(particlesTVB_);
	Assert(particlesIB_);

	float size;
	PGeometry * pData = (PGeometry *)particlesVB_->Lock();
	if (pData)
	{
		for (unsigned int i = 0; i < maxPCount; ++i)
		{
			size = (main_.GetParams().maxParticleSize-main_.GetParams().minParticleSize)*rand()/RAND_MAX+main_.GetParams().minParticleSize;
			for (unsigned int j = 0; j < 4; ++j)
			{
				pData[i*4 + j].corner = (float)j;
				pData[i*4 + j].size = size;
			}
		}
		particlesVB_->Unlock();
	}

	unsigned short * iData = (unsigned short *)particlesIB_->Lock();
	if (iData)
	{
		for (unsigned int i = 0; i < maxPCount; ++i)
		{
			iData[i*6 + 0] = i*4 + 0;
			iData[i*6 + 1] = i*4 + 1;
			iData[i*6 + 2] = i*4 + 2;

			iData[i*6 + 3] = i*4 + 0;
			iData[i*6 + 4] = i*4 + 2;
			iData[i*6 + 5] = i*4 + 3;
		}
		particlesIB_->Unlock();
	}


	varPartTexture_ = main_.Render().GetTechniqueGlobalVariable("bowWaveTexture", __FILE__, __LINE__);
	varMInvView_ = main_.Render().GetTechniqueGlobalVariable("matInvView", __FILE__, __LINE__);
	varMWorldViewProj_ = main_.Render().GetTechniqueGlobalVariable("matWorldViewProj", __FILE__, __LINE__);
	bowParticlesTexture_ = main_.Render().CreateTexture(__FILE__, __LINE__, main_.GetParams().texName);
	

	for (unsigned int i = 0; i < main_.GetParams().maxPartCount; ++i)
		partData_.Add();
	UpdatePTransforms(0.0f);
}

void BowParticles::EmitParticles(float deltaTime)
{
	for (unsigned int i = 0; i < partData_.Size(); ++i)
	{
		if (partData_[i].isActive && partData_[i].solver.IsFinished())
			partData_[i].isActive = false;
	}

	if (main_.GetParams().isEmissionStopped_)
		return;


	Matrix m;
	Vector emitter = main_.GetParams().pos;
	float A[2], C[2];
	C[0] = main_.GetParams().curveHeight[0];
	C[1] = main_.GetParams().curveHeight[1];
	A[0] = (-main_.GetParams().endLine - C[0])/(main_.GetParams().curveWidth[0]*main_.GetParams().curveWidth[0]);
	A[1] = (-main_.GetParams().endLine - C[1])/(main_.GetParams().curveWidth[1]*main_.GetParams().curveWidth[1]);

	Vector endPos;

	// кол-во активированных частиц
	unsigned int activatedCount = 0;		
	// максимальное количество, которое можем активировать за прошедшее время.
	unsigned int maxActivateCount = (unsigned int)(main_.GetParams().maxPartCount*deltaTime/main_.GetParams().maxParticleLifeTime);

	Matrix hostMatrix;
	if (main_.GetHost())
		main_.GetHost()->GetMatrix(hostMatrix);
	float width = Max(main_.GetParams().curveWidth[0], main_.GetParams().curveWidth[1]);
	for (unsigned int i = 0; i < partData_.Size(); ++i)
	{
		if (!partData_[i].isActive)
		{
			float x = 2*width*rand()/RAND_MAX - width;

			float zl = A[0]*x*x + C[0];
			float zu = A[1]*x*x + C[1];

			if (zl < -main_.GetParams().endLine)
				zl = -main_.GetParams().endLine;

			if (zu < -main_.GetParams().endLine)
				zu = -main_.GetParams().endLine;

			float z = zl + (zu-zl)*rand()/RAND_MAX;

			endPos.x = x;
			endPos.y = 0;
			endPos.z = z;

			partData_[i].angle = 0;
			partData_[i].rotateSpeed = (main_.GetParams().maxRotateSpeed - main_.GetParams().minRotateSpeed)*PI*rand()/RAND_MAX/180.0f + main_.GetParams().minRotateSpeed*PI/180.0f;
			partData_[i].solver.SetParams(	hostMatrix.MulVertex(main_.GetParams().pos),
				hostMatrix.MulVertex(main_.GetParams().pos + endPos), main_.GetHostLinearSpeed(), 
				(main_.GetParams().maxHeight-main_.GetParams().minHeight)*rand()/RAND_MAX+main_.GetParams().minHeight,
				(main_.GetParams().maxParticleLifeTime-main_.GetParams().minParticleLifeTime)*rand()/RAND_MAX + main_.GetParams().minParticleLifeTime,
				main_.GetParams().parabolaTrackCoef);
			partData_[i].isActive = true;
			++activatedCount;
		}

		if (activatedCount > maxActivateCount)
			return;

	}
}

void BowParticles::UpdatePTransforms(float deltaTime)
{
	// TODO: сделать флаг Discard
	PTransform * pData = (PTransform *)particlesTVB_->Lock();
	Vector center;
	float angle;
	float ttl;
	if (pData)
	{
		for (unsigned int i = 0; i < main_.GetParams().maxPartCount; ++i)
		{
			center = partData_[i].solver.GetCurrentPosition();
			angle = partData_[i].angle;
			ttl = partData_[i].ttl;
			for (unsigned int j = 0; j < 4; ++j)
			{
				pData[i*4+j].vCenter = center;
				pData[i*4+j].angle = angle;
				pData[i*4+j].ttl = ttl;
			}

		}
		particlesTVB_->Unlock();
	}
}

void BowParticles::RenderPSystem()
{
	Matrix m;

	varMInvView_->SetMatrix(Matrix(main_.Render().GetView()).Inverse());
	varMWorldViewProj_->SetMatrix(main_.Render().GetView()*main_.Render().GetProjection());
	varPartTexture_->SetTexture(bowParticlesTexture_);

	main_.Render().SetStreamSource(0, particlesVB_);
	main_.Render().SetStreamSource(1, particlesTVB_);
	main_.Render().SetIndices(particlesIB_);

	main_.Render().DrawIndexedPrimitive(BowParticles_id, PT_TRIANGLELIST, 0, main_.GetParams().maxPartCount*4, 0, main_.GetParams().maxPartCount*2);
}