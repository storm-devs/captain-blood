#include "datasource.h"

#include "databool.h"
#include "datacolor.h"
#include "datafloat.h"
#include "datagraph.h"
#include "dataposition.h"
#include "datauv.h"
#include "datastring.h"

#include "..\..\TextFile.h"

#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "..\..\..\Common_h\data_swizzle.h"

#define HEADER "PSYS"
#define VERSION "v3.5"




BEGIN_DATA_DESC(PointEmitterDesc)
DATA_STRING (GUID_EMITTER_NAME, EMITTER_NAME, "Emiter name");
DATA_FLOAT (GUID_EMITTER_LIFETIME, EMITTER_LIFETIME, "Emiter lifetime");
DATA_POSITION (GUID_EMITTER_POSITION, EMITTER_POSITION, "Position");
DATA_GRAPH (GUID_EMISSION_DIR_X, EMISSION_DIR_X, "Emission dir PITCH");
DATA_GRAPH (GUID_EMISSION_DIR_Y, EMISSION_DIR_Y, "Emission dir YAW");
DATA_GRAPH (GUID_EMISSION_DIR_Z, EMISSION_DIR_Z, "Emission dir ROLL");
DATA_BOOL (GUID_EMITTER_LOOPING, EMITTER_LOOPING, "Looping");
DATA_FLOAT (GUID_EMITTER_SIZEX, EMITTER_SIZEX, "Size X");
DATA_FLOAT (GUID_EMITTER_SIZEY, EMITTER_SIZEY, "Size Y");
DATA_FLOAT (GUID_EMITTER_SIZEZ, EMITTER_SIZEZ, "Size Z");
END_DATA_DESC(PointEmitterDesc)

BEGIN_DATA_DESC(BillboardParticleDesc)
DATA_POSITION (GUID_PARTICLE_EXTERNALFORCE, PARTICLE_EXTERNALFORCE, "External force");
DATA_STRING(GUID_PARTICLE_NAME, PARTICLE_NAME, "Name");
DATA_GRAPH (GUID_PARTICLE_EMISSION_RATE, PARTICLE_EMISSION_RATE, "Emission rate");
DATA_FLOAT (GUID_PARTICLE_MAX_COUNT, PARTICLE_MAX_COUNT, "Max Particles count");
//      DATA_GRAPH (PARTICLE_TRACK_X);
//      DATA_GRAPH (PARTICLE_TRACK_Y);
//      DATA_GRAPH (PARTICLE_TRACK_Z);
DATA_GRAPH (GUID_PARTICLE_LIFE_TIME, PARTICLE_LIFE_TIME, "Life time");
DATA_GRAPH (GUID_PARTICLE_MASS, PARTICLE_MASS, "Mass");
DATA_GRAPH (GUID_PARTICLE_SPIN, PARTICLE_SPIN, "Spin");
DATA_GRAPH (GUID_PARTICLE_VELOCITY_POWER, PARTICLE_VELOCITY_POWER, "Initial Velocity");
DATA_GRAPH (GUID_PARTICLE_INITIALSPIN, PARTICLE_INITIALSPIN, "Initial Rotate");
DATA_GRAPH (GUID_PARTICLE_SPIN_DRAG, PARTICLE_SPIN_DRAG, "Spin friction");
DATA_GRAPH (GUID_PARTICLE_SIZE, PARTICLE_SIZE, "Size percent");
DATA_GRAPH (GUID_PARTICLE_INITSIZE, PARTICLE_INITSIZE, "Initial Size");
DATA_GRAPH (GUID_PARTICLE_ANIMFRAME, PARTICLE_ANIMFRAME, "Animation frame");
DATA_COLOR (GUID_PARTICLE_COLOR, PARTICLE_COLOR, "Color");
DATA_GRAPH (GUID_PARTICLE_COLORMULTIPLY, PARTICLE_COLORMULTIPLY, "Color_Multipler");
DATA_UV (GUID_PARTICLE_FRAMES, PARTICLE_FRAMES, "Texture");
DATA_GRAPH (GUID_PARTICLE_TRANSPARENCY, PARTICLE_TRANSPARENCY, "Transparency");
DATA_GRAPH (GUID_PARTICLE_DRAG, PARTICLE_DRAG, "Friction");
//      DATA_GRAPH (PARTICLE_PHYSIC_BLEND);
DATA_STRING (GUID_ATTACHEDEMITTER_NAME, ATTACHEDEMITTER_NAME, "Attached emiter");
DATA_GRAPH (GUID_PARTICLE_GRAVITATION_K, PARTICLE_GRAVITATION_K, "Gravity strength");
DATA_BOOL (GUID_PARTICLE_DIR_ORIENT, PARTICLE_DIR_ORIENT, "Direction orient");
DATA_BOOL (GUID_PARTICLE_LOCAL_SYSTEM, PARTICLE_LOCAL_SYSTEM, "Animate in Local System");
DATA_GRAPH (GUID_PARTICLE_ADDPOWER, PARTICLE_ADDPOWER, "Add Technique Power");
DATA_STRING (GUID_FORCEFIELDS_NAME, FORCEFIELDS_NAME, "Forcefiels");
DATA_GRAPH (GUID_PARTICLE_SIZE, PARTICLE_SIZE, "Size percent");
DATA_GRAPH (GUID_PARTICLE_SIZEINC, PARTICLE_SIZEINC, "Size increment");


DATA_GRAPH (GUID_PARTICLE_LIGHTPOWER, PARTICLE_LIGHTPOWER, "Inverse Light power");



DATA_GRAPH (GUID_PARTICLE_INITPOSX, PARTICLE_INITPOSX, "Initial position X");
DATA_GRAPH (GUID_PARTICLE_INITPOSY, PARTICLE_INITPOSY, "Initial position Y");
DATA_GRAPH (GUID_PARTICLE_INITPOSZ, PARTICLE_INITPOSZ, "Initial position Z");
DATA_BOOL (GUID_PARTICLE_DISTORTED, PARTICLE_DISTORTED, "Distorted")


DATA_GRAPH (GUID_PARTICLE_DISTORTED_POWER, PARTICLE_DISTORTED_POWER, "Distorsion power in percent");

DATA_BOOL (GUID_PARTICLE_ZXALIGN, PARTICLE_ZXALIGN, "XZ_Aligned")


//DATA_BOOL (GUID_PARTICLE_AFFECT_AMBIENT, PARTICLE_AFFECT_AMBIENT, "Always affect ambient light")
DATA_GRAPH (GUID_PARTICLE_AMBIENTPOWER, PARTICLE_AMBIENTPOWER, "Inverse Ambient power");

END_DATA_DESC(BillboardParticleDesc)

BEGIN_DATA_DESC(ModelParticleDesc)
DATA_STRING(GUID_PARTICLE_NAME, PARTICLE_NAME, "Name");
DATA_GRAPH (GUID_PARTICLE_EMISSION_RATE, PARTICLE_EMISSION_RATE, "Emission rate");
DATA_FLOAT (GUID_PARTICLE_MAX_COUNT, PARTICLE_MAX_COUNT, "Max Particles count");
//      DATA_GRAPH (PARTICLE_TRACK_X);
//      DATA_GRAPH (PARTICLE_TRACK_Y);
//      DATA_GRAPH (PARTICLE_TRACK_Z);
DATA_GRAPH (GUID_PARTICLE_LIFE_TIME, PARTICLE_LIFE_TIME, "Life time");
DATA_GRAPH (GUID_PARTICLE_MASS, PARTICLE_MASS, "Mass");
DATA_GRAPH (GUID_PARTICLE_SPIN_X, PARTICLE_SPIN_X,  "Spin X");
DATA_GRAPH (GUID_PARTICLE_SPIN_Y, PARTICLE_SPIN_Y, "Spin Y");
DATA_GRAPH (GUID_PARTICLE_SPIN_Z, PARTICLE_SPIN_Z, "Spin Z");
DATA_GRAPH (GUID_PARTICLE_VELOCITY_POWER, PARTICLE_VELOCITY_POWER, "Initial Velocity");
DATA_GRAPH (GUID_PARTICLE_SPIN_DRAGX, PARTICLE_SPIN_DRAGX, "Spin Friction X");
DATA_GRAPH (GUID_PARTICLE_SPIN_DRAGY, PARTICLE_SPIN_DRAGY, "Spin Friction Y");
DATA_GRAPH (GUID_PARTICLE_SPIN_DRAGZ, PARTICLE_SPIN_DRAGZ, "Spin Friction Y");
DATA_GRAPH (GUID_PARTICLE_DRAG, PARTICLE_DRAG, "Friction");
//      DATA_GRAPH (PARTICLE_PHYSIC_BLEND);
DATA_STRING (GUID_ATTACHEDEMITTER_NAME, ATTACHEDEMITTER_NAME, "Attached emitter");
DATA_GRAPH (GUID_PARTICLE_GRAVITATION_K, PARTICLE_GRAVITATION_K, "Gravity strength");
DATA_STRING (GUID_PARTICLE_GEOM_NAMES, PARTICLE_GEOM_NAMES, "Model names");
DATA_STRING (GUID_FORCEFIELDS_NAME, FORCEFIELDS_NAME, "Forcefiels");
END_DATA_DESC(ModelParticleDesc)


//---------- Создание/удаление --------------------
DataSource::DataSource () : Emitters(_FL_, 1)
{
}

DataSource::~DataSource ()
{
	Destroy ();
}


bool DataSource::Release ()
{
	delete this;
	return true;
}


// ========================= Load & Save =======================================
//Сохранить/восстановить из файла
void DataSource::Write (MemFile* pMemFile)
{
	pMemFile->Write(HEADER, 4);
	pMemFile->Write(VERSION, 4);

	DWORD dwEmittersCount = Emitters.Size();
	pMemFile->WriteType(dwEmittersCount);

	for (DWORD n = 0; n < dwEmittersCount; n++)
	{
		pMemFile->WriteType(Emitters[n].Type);
		Emitters[n].Fields.Write(pMemFile);
		DWORD dwParticlesSize = Emitters[n].Particles.Size();
		pMemFile->WriteType(dwParticlesSize);
		for (DWORD i = 0; i < dwParticlesSize; i++)
		{
			pMemFile->WriteType(Emitters[n].Particles[i].Type);
			Emitters[n].Particles[i].Fields.Write(pMemFile);
		}
	}
}


#ifndef _XBOX
void DataSource::WriteXML (TextFile* xmlFile, dword level)
{


	xmlFile->Write(level, "<Particles>\n");



	DWORD dwEmittersCount = Emitters.Size();
	for (DWORD n = 0; n < dwEmittersCount; n++)
	{
		xmlFile->Write((level+1), "<Emitter val = \"POINT_EMITTER\">\n");

		Emitters[n].Fields.WriteXML(xmlFile, (level+1));


		DWORD dwParticlesSize = Emitters[n].Particles.Size();
		for (DWORD i = 0; i < dwParticlesSize; i++)
		{
			switch (Emitters[n].Particles[i].Type)
			{
			case BILLBOARD_PARTICLE: 
				xmlFile->Write((level+2), "<Particle val = \"BILLBOARD_PARTICLE\">\n");
				break;
			case MODEL_PARTICLE: 
				xmlFile->Write((level+2), "<Particle val = \"MODEL_PARTICLE\">\n");
				break;
			}

			Emitters[n].Particles[i].Fields.WriteXML(xmlFile, (level+2));

			xmlFile->Write((level+2), "</Particle>\n");



		}

		xmlFile->Write((level+1), "</Emitter>\n");


	}




	xmlFile->Write(level, "</Particles>\n");

	/*
	pMemFile->Write(HEADER, 4);
	pMemFile->Write(VERSION, 4);

	DWORD dwEmittersCount = Emitters.Size();
	pMemFile->WriteType(dwEmittersCount);

	for (DWORD n = 0; n < dwEmittersCount; n++)
	{
	pMemFile->WriteType(Emitters[n].Type);
	Emitters[n].Fields.Write(pMemFile);
	DWORD dwParticlesSize = Emitters[n].Particles.Size();
	pMemFile->WriteType(dwParticlesSize);
	for (DWORD i = 0; i < dwParticlesSize; i++)
	{
	pMemFile->WriteType(Emitters[n].Particles[i].Type);
	Emitters[n].Particles[i].Fields.Write(pMemFile);
	}
	}
	*/
}

void DataSource::LoadXML (TiXmlDocument* xmlFile)
{
	TiXmlElement* rootNode = xmlFile->FirstChildElement( "Particles" );
	if (!rootNode)
	{
		api->Trace ("XML Particles: Incorrect file type");
		return;
	}

	for(TiXmlElement* child = rootNode->FirstChildElement(); child; child = child->NextSiblingElement())
	{
		string NodeName = child->Value();
		string NodeVal = child->Attribute("val");

		if (NodeName == "Emitter" && NodeVal == "POINT_EMITTER")
		{
			CreatePointEmitterXML (child);
		}

	}
}
#endif

void DataSource::Load (MemFile* pMemFile)
{
	//Проверяем ID
	char Id[5];
	Id[4] = 0;
	pMemFile->Read(Id, 4);

	if (strcmp (Id, HEADER) != 0)
	{
		api->Trace ("Particles: Incorrect file type");
		return;
	}

	//Проверяем Версию
	char Ver[5];
	Ver[4] = 0;
	pMemFile->Read(Ver, 4);



	/* // show warnings
	if (strcmp (Ver, VERSION) != 0) 
	api->Trace ("Particles: Warning !!! Incorrect file version %s, must be %s", Ver, VERSION);
	*/          

	//Кол-во эмиттеров...
	dword EmiterCount = 0;
	pMemFile->ReadType(EmiterCount);
	XSwizzleDWord(EmiterCount);

	Emitters.Reserve(EmiterCount);

	for (DWORD n = 0; n < EmiterCount; n++)
	{
		EmitterType emType = UNKNOWN_EMITTER;
		pMemFile->ReadType(emType);
		emType = (EmitterType)SwizzleDWord(emType);

		switch (emType)
		{
		case POINT_EMITTER:
			{
				//                              api->Trace ("Particles info: Point emitter");
				CreatePointEmitter (pMemFile);
				break;
			}

		default:
			{
				throw ("Particles: Unknown emitter type !");
				return;
			}
		} // switch
	} // for all saved emitters...
}


#ifndef _XBOX
void DataSource::CreatePointEmitterXML (TiXmlElement* root)
{
	EmitterDesc* PointEmitter = &Emitters[Emitters.Add()];
	PointEmitter->Fields.SetMasterDataSource(this);
	PointEmitter->Type = POINT_EMITTER;


	for(TiXmlElement* child = root->FirstChildElement(); child; child = child->NextSiblingElement())
	{
		string NodeName = child->Value();
		string NodeVal = child->Attribute("val");

		if (NodeName == "Fields")
		{
			PointEmitter->Fields.LoadXML(child, &PointEmitterDesc);
			PointEmitter->Fields.Convert (&PointEmitterDesc);
		}


		if (NodeName == "Particle" && NodeVal == "BILLBOARD_PARTICLE")
		{
			CreateBillBoardParticleXML (PointEmitter->Particles, child);
		}

		if (NodeName == "Particle" && NodeVal == "MODEL_PARTICLE")
		{
			CreateModelParticleXML (PointEmitter->Particles, child);
		}

	}
}
#endif

void DataSource::CreatePointEmitter (MemFile* pMemFile)
{
	//api->Trace ("Particles info: Point emitter");
	EmitterDesc* PointEmitter = &Emitters[Emitters.Add()];
	PointEmitter->Fields.SetMasterDataSource(this);
	PointEmitter->Fields.Load(pMemFile, &PointEmitterDesc);

	PointEmitter->Fields.Convert (&PointEmitterDesc);
	PointEmitter->Type = POINT_EMITTER;

	dword ParticlesCount = 0;
	pMemFile->ReadType(ParticlesCount);
	XSwizzleDWord(ParticlesCount);

	PointEmitter->Particles.Reserve(ParticlesCount);

	for (DWORD n = 0; n < ParticlesCount; n++)
	{
		ParticleType ptType = UNKNOWN_PARTICLE;
		pMemFile->ReadType(ptType);
		ptType = (ParticleType)SwizzleDWord(ptType);

		switch(ptType) 
		{
		case BILLBOARD_PARTICLE:
			{       
				//api->Trace ("Particles info: Billboard particle");
				CreateBillBoardParticle (PointEmitter->Particles, pMemFile);
				break;
			}
		case MODEL_PARTICLE:
			{       
				//api->Trace ("Particles info: Model particle");
				CreateModelParticle (PointEmitter->Particles, pMemFile);
				break;
			}
		default:
			{
				throw ("Particles: Unknown particle type !!!!");
			}
		} // SWITCH
	} // For all particles
}

//Создает BillBoard парикл
void DataSource::CreateBillBoardParticle (array<ParticleDesc> &Particles, MemFile* pMemFile)
{
	ParticleDesc *pDesc = &Particles[Particles.Add()];
	pDesc->Type = BILLBOARD_PARTICLE;
	pDesc->Fields.SetMasterDataSource(this);
	pDesc->Fields.Load(pMemFile, &BillboardParticleDesc);
	pDesc->Fields.Convert(&BillboardParticleDesc);

	DataGraph* pInitSpin = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITIALSPIN);
	if (pInitSpin) pInitSpin->SetNegative(true);

	DataGraph* pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SIZE);
	if (pSize) pSize->SetNegative(true);


	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SIZEINC);
	if (pSize) pSize->SetNegative(true);


	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITSIZE);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_GRAVITATION_K);
	if (pSize) pSize->SetNegative(true);






	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSX);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSY);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSZ);
	if (pSize) pSize->SetNegative(true);



	DataGraph* pMass = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_MASS);
	if (pMass) pMass->SetNegative(false);


}

//Создает Model парикл
void DataSource::CreateModelParticle (array<ParticleDesc> &Particles, MemFile* pMemFile)
{
	ParticleDesc *pDesc = &Particles[Particles.Add()];
	pDesc->Type = MODEL_PARTICLE;
	pDesc->Fields.SetMasterDataSource(this);
	pDesc->Fields.Load(pMemFile, &ModelParticleDesc);
	pDesc->Fields.Convert(&ModelParticleDesc);
}


void DataSource::Destroy ()
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		Emitters[n].Fields.DelAll();
		for (DWORD i = 0; i < Emitters[n].Particles.Size(); i++)
		{
			Emitters[n].Particles[i].Fields.DelAll();
		}
	}

	Emitters.DelAll();


}


int DataSource::GetEmitterCount ()
{
	return Emitters.Size();
}

DataSource::EmitterDesc* DataSource::GetEmitterDesc (int Index)
{
	return &Emitters[Index];
}

FieldList* DataSource::CreateEmptyPointEmitter (const char* EmitterName)
{
	EmitterDesc* PointEmitter = &Emitters[Emitters.Add()];
	PointEmitter->Fields.SetMasterDataSource(this);
	PointEmitter->Fields.Convert (&PointEmitterDesc);
	PointEmitter->Type = POINT_EMITTER;

	DataString* pEmitterName = PointEmitter->Fields.FindStringByGUID(GUID_EMITTER_NAME);

	if (pEmitterName)
	{
		pEmitterName->SetValue(EmitterName);
	}

	DataFloat* pEmitterLifeTime = PointEmitter->Fields.FindFloatByGUID(GUID_EMITTER_LIFETIME);

	if (pEmitterLifeTime)
	{
		pEmitterLifeTime->SetValue(1.0f);
	}

	DataGraph* pAngleX = PointEmitter->Fields.FindGraphByGUID(GUID_EMISSION_DIR_X);
	if (pAngleX) pAngleX->SetNegative(true);

	DataGraph* pAngleY = PointEmitter->Fields.FindGraphByGUID(GUID_EMISSION_DIR_Y);
	if (pAngleY) pAngleY->SetNegative(true);

	DataGraph* pAngleZ = PointEmitter->Fields.FindGraphByGUID(GUID_EMISSION_DIR_Z);
	if (pAngleZ) pAngleZ->SetNegative(true);

	return &PointEmitter->Fields;
}

int DataSource::FindEmitter (const char* Name)
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		DataString* pString = Emitters[n].Fields.FindStringByGUID(GUID_EMITTER_NAME);
		if (pString)
		{
			if (crt_stricmp (pString->GetValue(), Name) == 0)
			{
				return n;
			}
		}
	}

	return -1;
}

FieldList* DataSource::CreateBillBoardParticle (const char* ParticleName, const char* EmitterName)
{
	int EmitterIndex = FindEmitter(EmitterName);
	if (EmitterIndex == -1) return NULL;

	ParticleDesc *pDesc = &Emitters[EmitterIndex].Particles[Emitters[EmitterIndex].Particles.Add()];
	pDesc->Type = BILLBOARD_PARTICLE;
	pDesc->Fields.SetMasterDataSource(this);
	pDesc->Fields.Convert(&BillboardParticleDesc);

	DataString* pParticleName = pDesc->Fields.FindStringByGUID(GUID_PARTICLE_NAME);
	if (pParticleName) pParticleName->SetValue(ParticleName);

	DataString* pAttachedEmitter = pDesc->Fields.FindStringByGUID(GUID_ATTACHEDEMITTER_NAME);
	if (pAttachedEmitter) pAttachedEmitter->SetValue("none");

	DataGraph* pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITSIZE);
	if (pSize)
	{
		pSize->SetDefaultValue(3.0f, 2.0f);
		pSize->SetNegative(true);
	}

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_LIGHTPOWER);
	if (pSize)
	{
		pSize->SetDefaultValue(0.0f, 0.0f);
		pSize->SetNegative(false);
	}





	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SIZE);
	if (pSize)
	{
		pSize->SetDefaultValue(100.0f, 100.0f);
		pSize->SetNegative(true);
	}

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_GRAVITATION_K);
	if (pSize) pSize->SetNegative(true);



	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SIZEINC);
	if (pSize)
	{
		pSize->SetDefaultValue(0.0f, 0.0f);
		pSize->SetNegative(true);
	}

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSX);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSY);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSZ);
	if (pSize) pSize->SetNegative(true);


	DataGraph* pEmissionRate = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_EMISSION_RATE);
	if (pEmissionRate) pEmissionRate->SetDefaultValue(10.0f, 10.0f);

	DataGraph* pLifeTime = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_LIFE_TIME);
	if (pLifeTime) pLifeTime->SetDefaultValue(10.0f, 10.0f);


	DataFloat* pMaxCount = pDesc->Fields.FindFloatByGUID(GUID_PARTICLE_MAX_COUNT);
	if (pMaxCount) pMaxCount->SetValue(100);

	DataColor* pColorG = pDesc->Fields.FindColorByGUID(GUID_PARTICLE_COLOR);
	if (pColorG) pColorG->SetDefaultValue(Color(1.0f, 1.0f, 1.0f, 1.0f));

	DataGraph* pColorGMul = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_COLORMULTIPLY);
	if (pColorGMul) pColorGMul->SetDefaultValue(1.0f, 1.0f);



	DataGraph* pGravityK = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_GRAVITATION_K);
	if (pGravityK) pGravityK->SetDefaultValue(100.0f, 100.0f);

	/*
	DataGraph* pTrackX = pDesc->Fields.FindGraph(PARTICLE_TRACK_X);
	if (pTrackX) pTrackX->SetNegative(true);

	DataGraph* pTrackY = pDesc->Fields.FindGraph(PARTICLE_TRACK_Y);
	if (pTrackY) pTrackY->SetNegative(true);

	DataGraph* pTrackZ = pDesc->Fields.FindGraph(PARTICLE_TRACK_Z);
	if (pTrackZ) pTrackZ->SetNegative(true);
	*/

	DataGraph* pVelocity = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_VELOCITY_POWER);
	if (pVelocity) pVelocity->SetNegative(true);

	DataGraph* pMass = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_MASS);
	if (pMass) pMass->SetNegative(false);

	DataGraph* pSpin = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SPIN);
	if (pSpin) pSpin->SetNegative(true);

	DataGraph* pInitSpin = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITIALSPIN);
	if (pInitSpin) pInitSpin->SetNegative(true);

	return &pDesc->Fields;
}

FieldList* DataSource::CreateModelParticle (const char* ParticleName, const char* EmitterName)
{
	int EmitterIndex = FindEmitter(EmitterName);
	if (EmitterIndex == -1) return NULL;

	ParticleDesc *pDesc = &Emitters[EmitterIndex].Particles[Emitters[EmitterIndex].Particles.Add()];
	pDesc->Type = MODEL_PARTICLE;
	pDesc->Fields.SetMasterDataSource(this);
	pDesc->Fields.Convert(&ModelParticleDesc);

	DataString* pParticleName = pDesc->Fields.FindStringByGUID(GUID_PARTICLE_NAME);
	if (pParticleName) pParticleName->SetValue(ParticleName);

	DataString* pAttachedEmitter = pDesc->Fields.FindStringByGUID(GUID_ATTACHEDEMITTER_NAME);
	if (pAttachedEmitter) pAttachedEmitter->SetValue("none");

	DataFloat* pMaxCount = pDesc->Fields.FindFloatByGUID(GUID_PARTICLE_MAX_COUNT);
	if (pMaxCount) pMaxCount->SetValue(100);

	DataGraph* pEmissionRate = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_EMISSION_RATE);
	if (pEmissionRate) pEmissionRate->SetDefaultValue(10.0f, 10.0f);

	DataGraph* pLifeTime = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_LIFE_TIME);
	if (pLifeTime) pLifeTime->SetDefaultValue(10.0f, 10.0f);

	DataGraph* pGravityK = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_GRAVITATION_K);
	if (pGravityK) pGravityK->SetDefaultValue(100.0f, 100.0f);
	/*
	DataGraph* pTrackX = pDesc->Fields.FindGraph(PARTICLE_TRACK_X);
	if (pTrackX) pTrackX->SetNegative(true);

	DataGraph* pTrackY = pDesc->Fields.FindGraph(PARTICLE_TRACK_Y);
	if (pTrackY) pTrackY->SetNegative(true);

	DataGraph* pTrackZ = pDesc->Fields.FindGraph(PARTICLE_TRACK_Z);
	if (pTrackZ) pTrackZ->SetNegative(true);
	*/

	DataGraph* pVelocity = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_VELOCITY_POWER);
	if (pVelocity) pVelocity->SetNegative(true);

	DataGraph* pMass = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_MASS);
	if (pMass) pMass->SetNegative(false);


	DataGraph* pSpinX = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SPIN_X);
	if (pSpinX) pSpinX->SetNegative(true);

	DataGraph* pSpinY = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SPIN_Y);
	if (pSpinY) pSpinY->SetNegative(true);

	DataGraph* pSpinZ = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SPIN_Z);
	if (pSpinZ) pSpinZ->SetNegative(true);

	return &pDesc->Fields;
}


void DataSource::DeletePointEmitter (FieldList* pEmitter)
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		if (&Emitters[n].Fields == pEmitter)
		{
			Emitters[n].Fields.DelAll();
			Emitters.ExtractNoShift(n);
			return;
		}
	}
}

void DataSource::DeleteBillboard (FieldList* pEmitter, FieldList* pParticles)
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		if (&Emitters[n].Fields == pEmitter)
		{
			for (DWORD i = 0; i < Emitters[n].Particles.Size(); i++)
			{
				if (&Emitters[n].Particles[i].Fields == pParticles)
				{
					Emitters[n].Particles[i].Fields.DelAll();
					Emitters[n].Particles.Extract(i);
					return;
				}
			}

			
		}
	}
}

void DataSource::DeleteModel (FieldList* pEmitter, FieldList* pParticles)
{
	for (DWORD n = 0; n < Emitters.Size(); n++)
	{
		if (&Emitters[n].Fields == pEmitter)
		{
			for (DWORD i = 0; i < Emitters[n].Particles.Size(); i++)
			{
				if (&Emitters[n].Particles[i].Fields == pParticles)
				{
					Emitters[n].Particles[i].Fields.DelAll();
					Emitters[n].Particles.Extract(i);
					return;
				}
			}

			
		}
	}
}

void DataSource::NewForceFieldRegistred()
{
	for (dword i = 0; i < Emitters.Size(); i++)
	{
		Emitters[i].Fields.UpdateCache();

		for (dword j = 0; j < Emitters[i].Particles.Size(); j++)
		{
			Emitters[i].Particles[j].Fields.UpdateCache();
		}
	}

}


#ifndef _XBOX
//Загрузить BillBoard партикл
void DataSource::CreateBillBoardParticleXML (array<ParticleDesc> &Particles, TiXmlElement* root)
{
	ParticleDesc *pDesc = &Particles[Particles.Add()];
	pDesc->Type = BILLBOARD_PARTICLE;
	pDesc->Fields.SetMasterDataSource(this);


	TiXmlElement* fieldsNode = root->FirstChildElement("Fields");
	if (fieldsNode)
	{
		pDesc->Fields.LoadXML(fieldsNode, &BillboardParticleDesc);
	}


	pDesc->Fields.Convert(&BillboardParticleDesc);

	DataGraph* pInitSpin = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITIALSPIN);
	if (pInitSpin) pInitSpin->SetNegative(true);

	DataGraph* pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SIZE);
	if (pSize) pSize->SetNegative(true);


	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_SIZEINC);
	if (pSize) pSize->SetNegative(true);


	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITSIZE);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_GRAVITATION_K);
	if (pSize) pSize->SetNegative(true);






	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSX);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSY);
	if (pSize) pSize->SetNegative(true);

	pSize = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_INITPOSZ);
	if (pSize) pSize->SetNegative(true);



	DataGraph* pMass = pDesc->Fields.FindGraphByGUID(GUID_PARTICLE_MASS);
	if (pMass) pMass->SetNegative(false);



}



//Загрузить Model партикл
void DataSource::CreateModelParticleXML (array<ParticleDesc> &Particles, TiXmlElement* root)
{
	ParticleDesc *pDesc = &Particles[Particles.Add()];
	pDesc->Type = MODEL_PARTICLE;
	pDesc->Fields.SetMasterDataSource(this);

	TiXmlElement* fieldsNode = root->FirstChildElement("Fields");
	if (fieldsNode)
	{
		pDesc->Fields.LoadXML(fieldsNode, &ModelParticleDesc);
	}


	pDesc->Fields.Convert(&ModelParticleDesc);
}

#endif