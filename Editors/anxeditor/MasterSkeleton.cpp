//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// MasterSkeleton
//============================================================================================


#include "MasterSkeleton.h"

//============================================================================================

MasterSkeleton::MasterSkeleton(AnxOptions & options) : opt(options), bones(_FL_)
{
}

MasterSkeleton::~MasterSkeleton()
{
	for(long i = 0; i < bones; i++) delete bones[i];
	bones.Empty();
}

//Записать данные в поток
void MasterSkeleton::Write(AnxStream & stream)
{
	stream.EnterWriteSection("Master skeleton -> ");
	stream.WriteVersion(1);
	stream << bones;
	for(long i = 0; i < bones; i++)
	{
		stream << bones[i]->name;
		stream << bones[i]->parent;
		stream << bones[i]->q.x;
		stream << bones[i]->q.y;
		stream << bones[i]->q.z;
		stream << bones[i]->q.w;
		stream << bones[i]->p.x;
		stream << bones[i]->p.y;
		stream << bones[i]->p.z;
		stream << bones[i]->s.x;
		stream << bones[i]->s.y;
		stream << bones[i]->s.z;
	}
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void MasterSkeleton::Read(AnxStream & stream)
{
	bones.DelAll();
	string sid;
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Master skeleton -> ") throw "Invalidate master skeleton section id";
		if(stream.ReadVersion() != 1) throw "Invalidate version master skeleton section";
		long numBones = 0;
		stream >> numBones;
		for(long i = 0; i < numBones; i++)
		{
			long j = bones.Add();
			bones[j] = NEW Bone();
			Assert(i == j);			
			stream >> bones[i]->name;
			stream >> bones[i]->parent;
			stream >> bones[i]->q.x;
			stream >> bones[i]->q.y;
			stream >> bones[i]->q.z;
			stream >> bones[i]->q.w;
			stream >> bones[i]->p.x;
			stream >> bones[i]->p.y;
			stream >> bones[i]->p.z;
			stream >> bones[i]->s.x;
			stream >> bones[i]->s.y;
			stream >> bones[i]->s.z;
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor project section: IORead graph error: %s", err ? err : "<no info>");
		for(long i = 0; i < bones; i++) delete bones[i];
		bones.Empty();
	}catch(...){
		api->Trace("AnxEditor project section: IORead graph unknow error.");
		for(long i = 0; i < bones; i++) delete bones[i];
		bones.Empty();
	}
#endif
	stream.ExitReadSection();
}

//Сравнить указаный мастер скелет с текущим
bool MasterSkeleton::Compare(MasterSkeleton * skeleton)
{
	if(!skeleton) return false;
	if(skeleton->bones.Size() != bones.Size()) return false;
	for(long i = 0; i < bones; i++)
	{
		if(bones[i]->parent >= 0)
		{
			if(bones[i]->parent != skeleton->bones[i]->parent) return false;
		}else{
			if(skeleton->bones[i]->parent >= 0) return false;
		}
		if(skeleton->bones[i]->name != bones[i]->name) return false;
	}
	return true;
}
