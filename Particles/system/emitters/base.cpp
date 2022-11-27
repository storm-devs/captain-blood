#include "base.h"
#include "..\DataSource\databool.h"
#include "..\DataSource\datacolor.h"
#include "..\DataSource\datafloat.h"
#include "..\DataSource\datagraph.h"
#include "..\DataSource\dataposition.h"
#include "..\DataSource\datauv.h"
#include "..\DataSource\datastring.h"
#include "..\..\..\common_h\core.h"
#include "..\..\icommon\names.h"
#include "..\..\manager\particlemanager.h"


#include "..\..\service\particleservice.h"


#define INTERPOLATION_STEPS 4.0f


extern ParticleService* PService;


// Конструктор / деструктор
BaseEmitter::BaseEmitter(ParticleSystem* pSystem, BillBoardProcessor* processor)
{
	ParticlesArraySize = 0;

	szName = NULL;
	m_processor = processor;

	pEmitter = NULL;
	Visible = true;
	pFields = NULL;
	IsAttachedFlag = false;
	OldMatrixNotInitialized = true;
	LifeTime = 0.0f;
	pMaster = pSystem;
	ElapsedTime = 0.0f;
	Position = Vector (0.0f);
	EmissionDirX = NULL;
	EmissionDirY = NULL;
	EmissionDirZ = NULL;
	Stoped = false;
}

BaseEmitter::~BaseEmitter()
{
	if (GetParticleCount() > 0)
	{
		m_processor->DeleteWithGUID(this);
	}
	Stoped = true;
}

//Родить новые партиклы 
void BaseEmitter::BornParticles (float DeltaTime, float fTimeScale, float fScale, const Vector& additionalVelocity)
{
	bool bLowQuality = PService->IsLowQuality();


	DeltaTime = DeltaTime * fTimeScale;
	float SavedTime = ElapsedTime;
	if (!Visible) return;
	Matrix matTransform;

	float MatrixBlend = 0.0f;
	float MatrixBlendInc = 1.0f / INTERPOLATION_STEPS;

	Vector vRazn;

	for (int i = 0; i < (int)INTERPOLATION_STEPS; i++)
	{
		vRazn = (matWorldTransformNew.pos - matWorldTransformOld.pos).Abs();
		if (vRazn.x > 0.001f && vRazn.y > 0.001f && vRazn.z > 0.001f)
		{
			BlendMatrix(matWorldTransform, matWorldTransformOld, matWorldTransformNew, MatrixBlend);
		} else
		{
			matWorldTransform = matWorldTransformNew;
		}


		Vector TransformPos = Position * matWorldTransform;
		matWorldTransform.pos = TransformPos;
		MatrixBlend += MatrixBlendInc;

		float DeltaTimeDiv = DeltaTime / INTERPOLATION_STEPS;
		IncreaseTime (DeltaTimeDiv);

		// Если запаузился эмиттер досрочный выход
		if (Stoped)
		{
			ElapsedTime = SavedTime;
			return;
		}


		for (DWORD n = 0; n < ParticlesArraySize; n++)
		{
			
			if (!ParticlesArray[n].Visible) continue;


			float EmissionRate = ParticlesArray[n].EmissionRate->GetRandomValue(ElapsedTime, LifeTime);
			EmissionRate *= DeltaTimeDiv;

			// How many particles remain unemissed from last frame
			float ParticlesRemain = ParticlesArray[n].Remain;	

			ParticlesRemain += EmissionRate;
			ParticlesArray[n].Remain = ParticlesRemain;

			

			while (ParticlesArray[n].Remain >= 1.0f)
			{
				ParticlesArray[n].Remain -= 1.0f;
				
				if (ParticlesArray[n].ActiveCount < ParticlesArray[n].MaxParticlesCount)
				{
					ParticlesArray[n].parity = !ParticlesArray[n].parity;

					if (bLowQuality && ParticlesArray[n].parity)
					{
						continue;
					}

					Vector ParticlePos = GetNewParticlePosition (DeltaTime) /** fScale*/;
					GetEmissionDirection (matTransform);

					matLastEmission = matWorldTransform * matTransform;

					Vector VelDir = matTransform.vy;
					switch(ParticlesArray[n].Type)
					{
					case BILLBOARD_PARTICLE:
						ParticlesArray[n].ActiveCount++;
						m_processor->AddParticle(additionalVelocity, pMaster, VelDir, ParticlePos, matWorldTransform, ElapsedTime, LifeTime, ParticlesArray[n].pFields, &ParticlesArray[n].ActiveCount, this, fTimeScale, fScale, bForceLocalMode);
						break;
					case MODEL_PARTICLE:
						break;
					default:
						throw("Try to emmit unknown particle type");
					} // switch
				} // Active < Max
			} // While Remain
		} // For all types
	}	 // INTERPOLATION_STEPS iterations

	matWorldTransformOld = matWorldTransformNew;

	ElapsedTime = SavedTime;
}
  

//Исполнить
void BaseEmitter::Execute (float DeltaTime, float fTimeScale, float fScale, const Vector& additionalVelocity)
{
	
	if (!Stoped && !IsAttachedFlag) BornParticles (DeltaTime, fTimeScale, fScale, additionalVelocity);

	IncreaseTime (DeltaTime*fTimeScale);

/*
	for (DWORD n = 0; n < ParticleTypes.Size(); n++)
	{
		switch (ParticleTypes[n].Type)
		{
			case BILLBOARD_PARTICLE:
				break;
			case MODEL_PARTICLE:
				break;
		}
	}
*/
}

//Присоединиться к источнику данных
void BaseEmitter::AttachToDataSource (DataSource::EmitterDesc* pEmitter)
{
	this->pEmitter = pEmitter;
	pFields = &pEmitter->Fields;
	Type = pEmitter->Type;

	Editor_UpdateCachedData ();

	EmissionDirX = pEmitter->Fields.FindGraphByGUID(GUID_EMISSION_DIR_X);
	EmissionDirY = pEmitter->Fields.FindGraphByGUID(GUID_EMISSION_DIR_Y);
	EmissionDirZ = pEmitter->Fields.FindGraphByGUID(GUID_EMISSION_DIR_Z);


	for (DWORD n = 0; n < pEmitter->Particles.Size(); n++)
	{
		DataSource::ParticleDesc* pDesc = &pEmitter->Particles[n];
		switch (pDesc->Type)
		{
			case BILLBOARD_PARTICLE:
				CreateBillBoardParticle (pDesc->Fields);
				break;
			case MODEL_PARTICLE:
				CreateModelParticle (pDesc->Fields);
				break;
			default:
				throw ("Particles: Unknown particle type !!!!");
		}

	}
}

void BaseEmitter::IncreaseTime (float DeltaTime)
{
	ElapsedTime += DeltaTime;
	if (ElapsedTime > LifeTime)
	{
		if (!Looped) Stoped = true;
		ElapsedTime -= LifeTime;
	}
}

void BaseEmitter::CreateBillBoardParticle (FieldList &Fields)
{
//	api->Trace("Create BB Particle\n");

	Assert(ParticlesArraySize < MAX_PARTICLES_COUNT)

	structParticleType* NewBillBoard = &ParticlesArray[ParticlesArraySize];
	ParticlesArraySize++;

	NewBillBoard->Type = BILLBOARD_PARTICLE;
	NewBillBoard->EmissionRate = Fields.FindGraphByGUID(GUID_PARTICLE_EMISSION_RATE);
	NewBillBoard->MaxParticlesCount = Fields.GetFloatAsInt(GUID_PARTICLE_MAX_COUNT);
	NewBillBoard->ActiveCount = 0;
	NewBillBoard->Remain = 0.0f;
	NewBillBoard->pFields = &Fields;
	NewBillBoard->Visible = true;
	NewBillBoard->parity = false;

//	api->Trace("%d", NewBillBoard->MaxParticlesCount);
	
}

void BaseEmitter::CreateModelParticle (FieldList &Fields)
{
}


void BaseEmitter::GetEmissionDirection (Matrix &matWorld)
{
	Vector DirAngles;
	DirAngles.x = EmissionDirX->GetRandomValue(ElapsedTime, LifeTime);
	DirAngles.y = EmissionDirY->GetRandomValue(ElapsedTime, LifeTime);
	DirAngles.z = EmissionDirZ->GetRandomValue(ElapsedTime, LifeTime);

	DirAngles *= MUL_DEGTORAD;

	
	matWorld = Matrix(DirAngles, Vector(0.0f));
}

void BaseEmitter::Restart ()
{
	//Удаляем свои партиклы...
	m_processor->DeleteWithGUID(this);
	Stoped = false;
	ElapsedTime = 0.0f;
}

DWORD BaseEmitter::GetParticleCount ()
{
	DWORD Count = 0;
	for (DWORD n = 0; n < ParticlesArraySize; n++)
	{
		Count += ParticlesArray[n].ActiveCount;
	}

	return Count;
}

bool BaseEmitter::IsStoped ()
{
	return Stoped;
}

void BaseEmitter::SetTransform (const Matrix& matWorld)
{
	if (OldMatrixNotInitialized)
	{
		matWorldTransformOld = matWorld;
		OldMatrixNotInitialized = false;
	} 

	matWorldTransformNew = matWorld;
	//matWorldTransform = matWorld;
}

void BaseEmitter::Teleport (const Matrix &matWorld)
{
	matWorldTransformOld = matWorld;
	matWorldTransformNew = matWorld;
}


const char* BaseEmitter::GetName ()
{
	return szName;
}

void BaseEmitter::SetAttachedFlag (bool Flag)
{
	IsAttachedFlag = Flag;
}

bool BaseEmitter::IsAttached ()
{
	return IsAttachedFlag;
}

float BaseEmitter::GetTime ()
{
	return ElapsedTime;
}

void BaseEmitter::SetTime (float Time)
{
	ElapsedTime = Time;
}

DWORD BaseEmitter::GetParticleTypesCount ()
{
	return ParticlesArraySize;
}

FieldList* BaseEmitter::GetParticleTypeDataByIndex (DWORD Index)
{
	return ParticlesArray[Index].pFields;
}

ParticleType BaseEmitter::GetParticleTypeByIndex  (DWORD Index)
{
	return ParticlesArray[Index].Type;
}

FieldList* BaseEmitter::GetData ()
{
	return pFields;
}

bool BaseEmitter::SetEnable (bool bVisible)
{
	Visible = bVisible;

	if (Visible == false)
	{
		m_processor->DeleteWithGUID(this);
	}

	for (DWORD n = 0; n < ParticlesArraySize; n++)
	{
		ParticlesArray[n].Visible = bVisible;
	}


	return Visible;
}

bool BaseEmitter::GetEnable ()
{
	return Visible;
}

int BaseEmitter::GetParticleTypeIndex (FieldList* pFields)
{
	for (DWORD n = 0; n < ParticlesArraySize; n++)
	{
		if (ParticlesArray[n].pFields == pFields) return n;
	}
	return -1;
}

bool BaseEmitter::SetParticleTypeEnable (bool bVisible, DWORD Index)
{
	ParticlesArray[Index].Visible = bVisible;

	if (bVisible == false)
	{
		m_processor->DeleteWithGUID(this);
	}

	return ParticlesArray[Index].Visible;
}

bool BaseEmitter::GetParticleTypeEnable (DWORD Index)
{
	return ParticlesArray[Index].Visible;
}

void BaseEmitter::Editor_UpdateCachedData ()
{
	szName = pEmitter->Fields.GetString(GUID_EMITTER_NAME, "NoName");
	LifeTime = pEmitter->Fields.GetFloat(GUID_EMITTER_LIFETIME);
	Assert (LifeTime > 0);
	Position = pEmitter->Fields.GetPosition(GUID_EMITTER_POSITION);
	Looped = pEmitter->Fields.GetBool(GUID_EMITTER_LOOPING, false);

	for (DWORD n = 0; n < ParticlesArraySize; n++)
	{
		ParticlesArray[n].MaxParticlesCount = ParticlesArray[n].pFields->GetFloatAsInt(GUID_PARTICLE_MAX_COUNT);
	}
	

}

void BaseEmitter::SetName (const char* Name)
{
	DataString* EmitterName = pEmitter->Fields.FindStringByGUID(GUID_EMITTER_NAME);
	Assert (EmitterName);
	EmitterName->SetValue(Name);
	Editor_UpdateCachedData ();
}

Matrix& BaseEmitter::GetTransform ()
{
	return matLastEmission;
}

void BaseEmitter::GetSystemTransform (Matrix& trans)
{
	pMaster->GetTransform(trans);
}


