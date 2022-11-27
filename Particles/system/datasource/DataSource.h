#ifndef _PARTICLE_DATA_SOURCE_H_
#define _PARTICLE_DATA_SOURCE_H_

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "..\datadesc\data_desc.h"
#include "..\..\icommon\names.h"
#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\templates.h"
#include "..\..\icommon\types.h"
#include "fieldlist.h"




class TextFile;
class DataObject;
class DataColor;
class DataBool;
class DataFloat;
class DataGraph;
class DataString;
class DataPosition;
class DataUV;
class TextFile;

#ifndef _XBOX
class TiXmlDocument;
class TiXmlElement;
#endif

class MemFile;

class IParticleManager;

//Источник данных для партиклов...
class DataSource
{

public:

	//Описание партикла (используеться при создании системы)
	struct ParticleDesc
	{
		ParticleType Type;
		FieldList Fields;

		ParticleDesc ()
		{
			Type = UNKNOWN_PARTICLE;
		}
	
/*
	protected:
	
		ParticleDesc & operator = (const ParticleDesc & source) 
		{ 
			return *this;
		}
*/

	};

	//Описание эмиттера (используеться при создании системы)
	struct EmitterDesc
	{
		EmitterType Type;
		FieldList Fields;
		array<ParticleDesc> Particles;

		EmitterDesc () : Particles (_FL_, 1)
		{
			Type = UNKNOWN_EMITTER;
		}
	};

private:
	array<EmitterDesc> Emitters;

	//Загрузить точечный эмиттер
	void CreatePointEmitter (MemFile* pMemFile);

#ifndef _XBOX
	void CreatePointEmitterXML (TiXmlElement* root);
#endif
	
	//Загрузить BillBoard партикл
	void CreateBillBoardParticle (array<ParticleDesc> &Particles, MemFile* pMemFile);

	//Загрузить Model партикл
	void CreateModelParticle (array<ParticleDesc> &Particles, MemFile* pMemFile);


#ifndef _XBOX
	//Загрузить BillBoard партикл
	void CreateBillBoardParticleXML (array<ParticleDesc> &Particles, TiXmlElement* root);


	//Загрузить Model партикл
	void CreateModelParticleXML (array<ParticleDesc> &Particles, TiXmlElement* root);
#endif


	int FindEmitter (const char* Name);
	

protected:


 virtual ~DataSource ();

public:


	FieldList* CreateEmptyPointEmitter (const char* EmitterName);
	FieldList* CreateBillBoardParticle (const char* ParticleName, const char* EmitterName);
	FieldList* CreateModelParticle (const char* ParticleName, const char* EmitterName);

	void DeletePointEmitter (FieldList* pEmitter);
	void DeleteBillboard (FieldList* pEmitter, FieldList* pParticles);
	void DeleteModel (FieldList* pEmitter, FieldList* pParticles);
 
	void Destroy ();

	//---------- Создание/удаление --------------------
	DataSource ();
	bool Release ();


// ========================= Load & Save =======================================
	//Сохранить/восстановить из файла
	virtual void Write (MemFile* pMemFile);

#ifndef _XBOX
	virtual void WriteXML (TextFile* xmlFile, dword level);

	virtual void LoadXML (TiXmlDocument* xmlFile);

#endif

	
	virtual void Load (MemFile* pMemFile);

	



	virtual int GetEmitterCount ();
	DataSource::EmitterDesc* GetEmitterDesc (int Index);

	void NewForceFieldRegistred();
};







#endif