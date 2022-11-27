//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//
// AnxEditor
//===========================================================================================================================
// AntFile
//============================================================================================

#include "AntFile.h"
#include "AnxProject.h"
#include "ant.h"
#include "AnxBase.h"


#define ANT_FILE_MAX_BONES	256					//Максимальное количество костей в скелете
#define ANT_CLIP_MAX_LOADED_SIZE	1024*256	//максимальный размер сжатого клипа

//============================================================================================

AntFile::AntFile(AnxOptions & options) : opt(options), bone(_FL_)
{
	ant = null;
	antSize = 0;
	pantIndex = -1;
	next = options.firstAnt;
	opt.firstAnt = this;
	loadError = "Not start loading";
	notFullControl = true;
	clipStartFrame = 0;
	clipNumFrames = 0;
	fps = 15.0f;
	probability = 1.0f;
	trackConst = 0;
	track8 = 0;
	track16 = 0;
	track24 = 0;
	trackFloat = 0;
	trackTotal = 0;
	clipSize = 0;
	globalPosBones = 0;
	quaternionError = opt.quaternionError;
	positionError = opt.positionError;
	scaleError = opt.scaleError;
	useCounter = 0;
}

AntFile::~AntFile()
{
	if(ant) delete ant;
	opt.project->clipsRepository.Del(pantIndex);	
	loadError = null;
	if(opt.firstAnt != this)
	{
		for(AntFile * a = opt.firstAnt; a; a = a->next)
		{
			if(a->next == this)
			{
				a->next = next;
				break;
			}
		}
		Assert(a);
	}else{
		opt.firstAnt = next;
	}
}

//============================================================================================

//Загрузить файл
bool AntFile::Load(const char * fileName)
{
	Assert(!ant);
	clipStartFrame = 0;
	clipNumFrames = 0;
	//Выделяем имя
	name = fileName;
	for(long c = name.Len() - 1; c >= 0; c--)
	{
		if(name[(dword)c] == '\\') break;
	}
	if(c >= 0) name.Delete(0, c + 1);
	for(c = name.Len() - 1; c >= 0; c--)
	{
		if(name[(dword)c] == '.') break;
	}
	if(c > 0) name.Delete(c, name.Len() - c);
	//Открываем файл
	AnxStreamFile file(false, true);
	loadError = "Can't file open";
	if(!file.Open(fileName)) return false;
	//Читаем заголовок
	AntFileHeader hdr;
	loadError = "Error read header from file";
	if(!file.ReadStreamData(&hdr, sizeof(AntFileHeader))) return false;
	//Проверяем данные
	loadError = "Invalidate file ID";
	if(hdr.id != ANTFILE_ID) return false;
	loadError = "Invalidate file version";
	if(hdr.ver != ANTFILE_VER) return false;
	loadError = "Invalidate file content (bonesCount)";
	if(hdr.bonesCount < 1) return false;
	if(hdr.bonesCount > ANT_FILE_MAX_BONES) return false;
	loadError = "Invalidate file content (framesCount)";
	if(hdr.framesCount < 1) return false;
	fps = hdr.defaultFPS;
	probability = 1.0f;
	//Вычисляем размер
	dword size = hdr.stringsTableSize;
	size += hdr.bonesCount*sizeof(AntFileBone);
	size += hdr.bonesCount*hdr.framesCount*sizeof(AntFileTrackElement);
	//Загружаем данные
	antSize = sizeof(AntFileHeader) + size;
	ant = NEW byte[antSize];
	*(AntFileHeader *)ant = hdr;
	loadError = "Error read data from file";
	if(!file.ReadStreamData(ant + sizeof(AntFileHeader), size)) return false;
	//Обновляем указатели
	UpdateInfo();
	//Проверяем данные
	loadError = "Invalidate bones hierarchy";
	for(dword i = 0; i < numBones; i++)
	{
		if(bones[i].parentIndex == ANTFILE_NOPARENT) continue;
		if(bones[i].parentIndex < 0) return false;
		if(bones[i].parentIndex >= long(i)) return false;
	}
	loadError = "Invalidate bone name";
	for(i = 0; i < numBones; i++)
	{
		if(bones[i].nameIndex >= header->stringsTableSize) return false;
	}
	//!!! Сбросим всё масштабирование в 1.0 !!!
	for(i = 0; i < numBones; i++)
	{
		AntFileTrackElement * t = (AntFileTrackElement *)(ant + tracksOffset) + numFrames*i;
		for(dword j = 0; j < numFrames; j++)
		{
			t[j].sx = 1.0f;
			t[j].sy = 1.0f;
			t[j].sz = 1.0f;
		}
	}
	//Всё нормально
	clipStartFrame = 0;
	clipNumFrames = numFrames;
	loadError = "File successful loaded";
	//Сжимаем данные	
	return PackAnt();
}

//Адаптировать данные на текущий мастер-скелет
void AntFile::ApplyMasterSkeleton(string * errorMessage)
{
	UseAnt();
	//Мастер скелет
	Assert(opt.project->skeleton);
	MasterSkeleton * s = opt.project->skeleton;
	//Массив костей совместимый с мастер-скелетом
	notFullControl = false;
	bone.Empty();
	Bone b;
	long assignedBones = 0;
	for(long i = 0; i < s->bones; i++)
	{
		for(dword j = 0; j < numBones; j++)
		{
			const char * boneName = &strings[bones[j].nameIndex];
			if(s->bones[i]->name == boneName) break;
		}
		if(j < numBones)
		{		
			b.track = tracksOffset + numFrames*j*sizeof(AntFileTrackElement);
			assignedBones++;
		}else{
			b.track = 0;
			notFullControl = true;
		}		
		bone.Add(b);
	}
	UpdateInfo();
	//Сообщение об несоотвесчтвии скелетов
	if(assignedBones != numBones)
	{
		string asg = " Ant file: ";
		asg += name;
		asg += "\n Bones in clip: ";
		asg += numBones;
		asg += "\n Assigned bones in clip for master skeleton: ";
		asg += assignedBones;
		asg += "\n Total bones in master skeleton: ";
		asg += s->bones;
		if(!errorMessage)
		{
			opt.gui_manager->MessageBox(asg, "Not identical skeletons", GUIMB_OK);
		}else{
			*errorMessage = asg;
		}
	}
	FreeAnt();
}

//Получить из файла мастер-скелет
MasterSkeleton * AntFile::CreateMasterSkeleton()
{
	UseAnt();
	//Создаём скелет
	MasterSkeleton * s = NEW MasterSkeleton(opt);
	//Копирование костей
	for(dword i = 0; i < numBones; i++)
	{
		Assert(bones[i].parentIndex <= long(i));
		MasterSkeleton::Bone * b = NEW MasterSkeleton::Bone();
		b->name = &strings[bones[i].nameIndex];
		b->parent = bones[i].parentIndex;
		AntFileTrackElement & te = *((AntFileTrackElement *)(ant + tracksOffset) + numFrames*i);
		b->q.x = te.qx;
		b->q.y = te.qy;
		b->q.z = te.qz;
		b->q.w = te.qw;
		b->p.x = te.px;
		b->p.y = te.py;
		b->p.z = te.pz;
		b->s.x = te.sx;
		b->s.y = te.sy;
		b->s.z = te.sz;
		s->bones.Add(b);
	}
	FreeAnt();
	return s;
}

//Обновить описание файла
void AntFile::UpdateInfo()
{
	if(!ant)
	{
		header = null;
		strings = null;
		bones = null;
		numBones = null;
		tracksOffset = 0;
//		numFrames = 0;
	}else{
		header = (AntFileHeader *)ant;
		strings = (char *)(ant + sizeof(AntFileHeader));
		bones = (AntFileBone *)(ant + sizeof(AntFileHeader) + header->stringsTableSize);
		numBones = header->bonesCount;
		tracksOffset = (byte *)(bones + header->bonesCount) - ant;
		numFrames = header->framesCount;
	}
}

//Подготовить трек для использования
AntFileTrackElement * AntFile::PrepareTrack(long boneIndex, array<AntFileTrackElement> & staticTrack)
{
	Assert(bone);
	if(bone[boneIndex].track)
	{
		return (AntFileTrackElement *)(ant + bone[boneIndex].track);
	}
	//Заполняем положением кости из мастер-скелета
	Assert(opt.project->skeleton);
	MasterSkeleton * s = opt.project->skeleton;
	Assert(bone.Size() == s->bones.Size());
	MasterSkeleton::Bone * sb = s->bones[boneIndex];
	AntFileTrackElement t;
	t.qx = sb->q.x;
	t.qy = sb->q.y;
	t.qz = sb->q.z;
	t.qw = sb->q.w;
	t.px = sb->p.x;
	t.py = sb->p.y;
	t.pz = sb->p.z;
	t.sx = sb->s.x;
	t.sy = sb->s.y;
	t.sz = sb->s.z;
	//
	staticTrack.Empty();
	staticTrack.AddElements(numFrames);
	AntFileTrackElement * buffer = staticTrack.GetBuffer();
	AntFileTrackElement * bufferSize = buffer + numFrames;
	for(; buffer < bufferSize; buffer++)
	{
		*buffer = t;
	}
	return staticTrack.GetBuffer();
}

//Записать данные в поток
void AntFile::Write(AnxStream & stream)
{
	Assert(pantIndex >= 0);
	stream.EnterWriteSection("ANT file data ex -> ");
	long ver = 3;
	stream << ver;
	stream << pantIndex;
	stream << antSize;
	stream << bone;
	for(long i = 0; i < bone; i++)
	{
		long track = bone[i].track;
		long tmp = 0;
		stream << tmp;
		stream << track;
	}
	stream << clipStartFrame << clipNumFrames;
	stream << name;
	stream << fps << probability;
	stream << clipSize;
	stream << notFullControl;
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void AntFile::Read(AnxStream & stream)
{
	if(ant) delete ant; ant = null;
	UpdateInfo();
	if(pantIndex >= 0)
	{
		opt.project->clipsRepository.Del(pantIndex);
		pantIndex = -1;
	}
	useCounter = 0;
	string sid;
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		long ver = -1;
		if(sid == "ANT file data -> ")
		{
			ver = 0;
		}else
		if(sid == "ANT file data ex -> ")
		{
			ver = 1;
		}else{
			throw "Invalidate section id";
		}
		dword tsize = 0;
		void * tant = null;
		if(ver)
		{
			stream >> ver;
			if(ver < 3)
			{
				stream.Read(tant, tsize);
				pantIndex = opt.project->clipsRepository.Add((byte *)tant, tsize);
				delete tant;
			}else{
				stream >> pantIndex;
				pantIndex = opt.project->clipsRepository.TranslateIndex(pantIndex);
			}
			stream >> antSize;
			if(ver == 1)
			{
				UseAnt();
				if(!ant || !PackAnt()) throw "Incorrect pack operation";
			}
		}else{
			stream.Read(tant, antSize);
			ant = (byte *)tant;
			if(!PackAnt()) throw "Incorrect pack operation";
			UseAnt();			
		}		
		long nBones = 0;
		stream >> nBones;
		bone.Reserve(nBones);
		for(long i = 0; i < nBones; i++)
		{
			Bone b;
			long tmp;
			stream >> tmp;
			long track = -1;
			stream >> track;
			if(track < 0) throw "Invalidate file content";
			b.track = track;
			bone.Add(b);
		}
		Assert(nBones == opt.project->skeleton->bones.Size());
		stream >> clipStartFrame >> clipNumFrames;
		stream >> name;
		stream >> fps >> probability;
		stream >> clipSize;
		stream >> notFullControl;
		stream.ExitReadSection();
		UpdateInfo();
		useCounter = 1;
		FreeAnt();
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor ant section: %s; error: %s", sid.GetBuffer(), err);
		throw "Can't load clip";
	}catch(...){
		api->Trace("AnxEditor ant section: Unknow error when read clip data");
		throw "Can't load clip";
	}
#endif
}

//Выгрузить данные в клип
dword AntFile::ExportData(AnxTrackHeader * bones, AnxMvTrackHeader * mt, bool globalPos, byte * data, array<byte> & buffer)
{
	dword dataSize = 0;
	ComponentInfo cmp[7];
	UseAnt();
	Assert(ant);
	Assert(bone);
	//Мастер скелет
	MasterSkeleton * s = opt.project->skeleton;
	Assert(s);
	trackConst = 0;
	track8 = 0;
	track16 = 0;
	track24 = 0;
	trackFloat = 0;
	trackTotal = 0;
	clipSize = 0;
	globalPosBones = 0;
	dword curNumFrames = GetNumFrames(mt != null);
	Assert(curNumFrames > 0);
	buffer.Empty();
	buffer.AddElements(curNumFrames*3*sizeof(float) + curNumFrames*8*sizeof(TempTrack));
	//Трек смещений
	if(mt)
	{
		if(clipNumFrames > 1)
		{
			//Строим разностные треки
			TrackAccessor root(ant, bone[0].track, clipStartFrame, s->bones[0]);
			float * track = (float *)buffer.GetBuffer();
			float px1 = root(0, AnxTrackHeaderBase::index_px);
			float pz1 = root(0, AnxTrackHeaderBase::index_pz);
			for(dword i = 0; i < curNumFrames; i++)
			{
				float px2 = root(i + 1, AnxTrackHeaderBase::index_px);
				float pz2 = root(i + 1, AnxTrackHeaderBase::index_pz);
				*track++ = px2 - px1;
				*track++ = pz2 - pz1;
				px1 = px2;
				pz1 = pz2;
			}
			//Создаём упакованые треки компонентов
			cmp[0].track = (TempTrack *)(buffer.GetBuffer() + curNumFrames*2*sizeof(float));
			cmp[0].shift = AnxTrackHeaderBase::code_shift_dx;
			cmp[0].index = AnxTrackHeaderBase::index_dx;
			cmp[1].track = cmp[0].track + curNumFrames;
			cmp[1].shift = AnxTrackHeaderBase::code_shift_dz;
			cmp[1].index = AnxTrackHeaderBase::index_dz;
			track = (float *)buffer.GetBuffer();
			PackTrackComponent(track, 2*sizeof(float), curNumFrames, cmp[0], mt->range[cmp[0].index], positionError);
			PackTrackComponent(track + 1, 2*sizeof(float), curNumFrames, cmp[1], mt->range[cmp[1].index], positionError);
			//Группируем компонентные треки в общий
			TrackInfo ti = SetTrack(cmp, 2, curNumFrames, data, dataSize);
			mt->desc = ti.desc;
			mt->data.ptr = ti.ptr;
		}else{
			mt->desc = AnxTrackHeaderBase::code_const;
			mt->data.offset = 0;
			mt->range[0].min = 0.0f;
			mt->range[0].delta = 0.0f;
			mt->range[1].min = 0.0f;
			mt->range[1].delta = 0.0f;
		}
	}
	//Треки костей
	cmp[0].track = (TempTrack *)(buffer.GetBuffer() + curNumFrames*3*sizeof(float));
	cmp[0].shift = AnxTrackHeaderBase::code_shift_qx;
	cmp[0].index = AnxTrackHeaderBase::index_qx;
	cmp[1].track = cmp[0].track + curNumFrames;
	cmp[1].shift = AnxTrackHeaderBase::code_shift_qy;
	cmp[1].index = AnxTrackHeaderBase::index_qy;
	cmp[2].track = cmp[1].track + curNumFrames;
	cmp[2].shift = AnxTrackHeaderBase::code_shift_qz;
	cmp[2].index = AnxTrackHeaderBase::index_qz;
	cmp[3].track = cmp[2].track + curNumFrames;
	cmp[3].shift = AnxTrackHeaderBase::code_shift_qw;
	cmp[3].index = AnxTrackHeaderBase::index_qw;
	cmp[4].track = cmp[3].track + curNumFrames;
	cmp[4].shift = AnxTrackHeaderBase::code_shift_px;
	cmp[4].index = AnxTrackHeaderBase::index_px;
	cmp[5].track = cmp[4].track + curNumFrames;
	cmp[5].shift = AnxTrackHeaderBase::code_shift_py;
	cmp[5].index = AnxTrackHeaderBase::index_py;
	cmp[6].track = cmp[5].track + curNumFrames;
	cmp[6].shift = AnxTrackHeaderBase::code_shift_pz;
	cmp[6].index = AnxTrackHeaderBase::index_pz;
	for(long i = 0; i < bone; i++)
	{
		TrackAccessor track(ant, bone[i].track, clipStartFrame, s->bones[i]);
		AnxTrackHeader & th = bones[i];
		//Определим в какой системе выгружать трек позиций		
		bool isGlobalPositionTrack = false;
		if(globalPos && s->bones[i]->parent >= 0)
		{
			isGlobalPositionTrack = IsNeedGlobalPosition(track, curNumFrames);
			if(isGlobalPositionTrack)
			{
				ConvertToGlobal(i, buffer.GetBuffer(), curNumFrames, mt != null);
				globalPosBones++;
			}
		}
		//Создаём упакованые треки кватерниона
		for(long c = 0; c < 4; c++)
		{
			if(bone[i].track)
			{
				//Трэк
				PackTrackComponent(track.Get(cmp[c].index), sizeof(AntFileTrackElement), curNumFrames, cmp[c], th.range[cmp[c].index], quaternionError);
			}else{
				//Константа, поскольку трек отсутствует
				AnxTrackHeaderBase::Range & r = th.range[cmp[c].index];
				r.min = track(0, cmp[c].index);
				r.delta = 0.0f;
				cmp[c].type = AnxTrackHeaderBase::code_const;
				cmp[c].size = 0;
			}
		}
		//Упаковываем треки позиции
		for(long c = 4, j = 0; c < 7; c++, j++)
		{
			//Определяем константные треки
			bool isConstTrack = false;
			float constValue = 0.0f;
			if(mt != null && i == 0)
			{
				//Есть трек перемещения, поэтому позицию по xz обнуляем
				if(cmp[c].index != AnxTrackHeaderBase::index_py)
				{
					isConstTrack = true;
					constValue = 0.0f;
				}
			}
			if(!bone[i].track && !isConstTrack && !isGlobalPositionTrack)
			{
				isConstTrack = true;
				constValue = track(0, cmp[c].index);
			}
			if(!isConstTrack)
			{
				float * src = null;
				dword size = 0;
				if(!isGlobalPositionTrack)
				{
					src = track.Get(cmp[c].index);
					size = sizeof(AntFileTrackElement);
				}else{
					src = ((float *)buffer.GetBuffer()) + j;
					size = sizeof(Vector);
				}
				PackTrackComponent(src, size, curNumFrames, cmp[c], th.range[cmp[c].index], quaternionError);
			}else{
				//Константа
				AnxTrackHeaderBase::Range & r = th.range[cmp[c].index];
				r.min = constValue;
				r.delta = 0.0f;
				cmp[c].type = AnxTrackHeaderBase::code_const;
				cmp[c].size = 0;
			}
		}
		//Группируем компонентные треки в общий
		TrackInfo ti = SetTrack(cmp, 7, curNumFrames, data, dataSize);
		th.desc = ti.desc | (isGlobalPositionTrack ? AnxTrackHeaderBase::flag_global_pos : 0);
		th.data.ptr = ti.ptr;
	}
	//Полный размер клипа для статистики
	clipSize = dataSize + s->bones.Size()*sizeof(AnxTrackHeader) + sizeof(AnxClip);
	if(mt)
	{
		clipSize += sizeof(AnxMvTrackHeader);
	}
	FreeAnt();
	return dataSize;
}

//Упаковать компоненту трека
void AntFile::PackTrackComponent(const void * src, dword pitchInBytes, dword count, ComponentInfo & cmp, AnxTrackHeaderBase::Range & range, float error)
{
	if(error < 1e-20f)
	{
		error = 1e-20f;
	}
	//Определяем диапазон	
	float tmin = *(const float *)src;
	float tmax = tmin;	
	const byte * ptr = (const byte *)src;
	for(dword i = 1; i < count; i++)
	{
		ptr += pitchInBytes;
		float v = *(float *)ptr;		
		if(tmin > v) tmin = v;
		if(tmax < v) tmax = v;		
	}
	float delta = tmax - tmin;
	//Константу возвращаем сразу
	if(delta <= error)
	{
		//Константа
		range.min = tmin;
		range.delta = 0.0f;
		cmp.type = AnxTrackHeaderBase::code_const;
		cmp.size = 0;
		return;
	}
	//Считаем треки в упакованном виде
	double packDelta8 = 0xff/delta;
	double packDelta16 = 0xffff/delta;
	double packDelta24 = 0xffffff/delta;
	float unPackDelta8 = delta/0xff;
	float unPackDelta16 = delta/0xffff;
	float unPackDelta24 = delta/0xffffff;
	bool isCanUse8 = true;
	bool isCanUse16 = true;
	bool isCanUse24 = true;
	ptr = (const byte *)src;
	TempTrack * tt = cmp.track;
	for(dword i = 0; i < count; i++)
	{
		TempTrack & cur = tt[i];
		float v = *(float *)ptr;
		cur.vFloat = v;
		ptr += pitchInBytes;
		if(isCanUse8)
		{
			cur.v8 = byte(packDelta8*(v - tmin));
			volatile byte tmp8 = cur.v8;
			float v8 = tmp8*unPackDelta8 + tmin;
			if(fabs(v8 - v) > error)
			{
				isCanUse8 = false;
			}
		}
		if(isCanUse16)
		{
			cur.v16 = word(packDelta16*(v - tmin));
			volatile word tmp16 = cur.v16;
			float v16 = tmp16*unPackDelta16 + tmin;
			if(fabs(v16 - v) > error)
			{
				isCanUse16 = false;
			}
		}
		if(isCanUse24)
		{
			cur.v24 = dword(packDelta24*(v - tmin)) & 0xffffff;
			volatile dword tmp24 = cur.v24;
			float v24 = tmp24*unPackDelta24 + tmin;
			if(fabs(v24 - v) > error)
			{
				isCanUse24 = false;
			}
		}
	}
	if(isCanUse8)
	{
		//8 bit упаковка
		range.min = tmin;
		range.delta = unPackDelta8;
		cmp.type = AnxTrackHeaderBase::code_ui8;
		cmp.size = 1;
	}else
	if(isCanUse16)
	{
		//16 bit упаковка
		range.min = tmin;
		range.delta = unPackDelta16;
		cmp.type = AnxTrackHeaderBase::code_ui16;
		cmp.size = 2;
	}else
	if(isCanUse24)
	{
		//24 bit упаковка
		range.min = tmin;
		range.delta = unPackDelta24;
		cmp.type = AnxTrackHeaderBase::code_ui24;
		cmp.size = 3;
	}else{
		//Неупакованные данные
		range.min = 0.0f;
		range.delta = 1.0f;
		cmp.type = AnxTrackHeaderBase::code_float;
		cmp.size = 4;
	}
}

//Разместить трек в буфере
AntFile::TrackInfo AntFile::SetTrack(ComponentInfo * cmp, dword count, dword numFrames, byte * data, dword & size)
{
	//Получаем размер и ставим оффсеты
	for(dword i = 0, s = 0; i < count; i++)
	{
		cmp[i].offset = s;
		s += cmp[i].size;
	}
	//Собираем трек в буфере
	TrackInfo ti;
	ti.ptr = data + size;
	size += s*numFrames;
	ti.desc = s & AnxTrackHeaderBase::size_mask;
	for(dword i = 0; i < count; i++)
	{
		ComponentInfo & ci = cmp[i];
		ti.desc |= ci.type << ci.shift;
		SetTrackComponent(ti.ptr, s, ci.offset, numFrames, ci.track, ci.type);		
		//Статистика
		switch(ci.type & AnxTrackHeaderBase::code_mask)
		{
		case AnxTrackHeaderBase::code_const:
			trackConst++;
			break;
		case AnxTrackHeaderBase::code_ui8:
			track8++;
			break;
		case AnxTrackHeaderBase::code_ui16:
			track16++;
			break;
		case AnxTrackHeaderBase::code_ui24:
			track24++;
			break;
		case AnxTrackHeaderBase::code_float:
			trackFloat++;
			break;
		}
		trackTotal++;
	}
	Assert((ti.desc & AnxTrackHeaderBase::size_mask) == s);
	return ti;
}

//Разместить элемент трека в нужном месте
void AntFile::SetTrackComponent(byte * track, dword elementSize, dword offset, dword count, TempTrack * src, dword type)
{
	track += offset;
	switch(type & AnxTrackHeaderBase::code_mask)
	{
	case AnxTrackHeaderBase::code_const:
		return;
	case AnxTrackHeaderBase::code_ui8:		
		for(dword i = 0; i < count; i++)
		{
			*track = src->v8;
			track += elementSize;
			src++;
		}
		return;
	case AnxTrackHeaderBase::code_ui16:
		for(dword i = 0; i < count; i++)
		{
			*(word *)track = src->v16;
			track += elementSize;
			src++;
		}
		return;
	case AnxTrackHeaderBase::code_ui24:
		for(dword i = 0; i < count; i++)
		{
			track[0] = (src->v24 >> 0);
			track[1] = (src->v24 >> 8);
			track[2] = (src->v24 >> 16);
			track += elementSize;
			src++;
		}
	case AnxTrackHeaderBase::code_float:
		for(dword i = 0; i < count; i++)
		{
			*(float *)track = src->vFloat;
			track += elementSize;
			src++;
		}
		return;
	}
	Assert(false);
}

//Определить нужна ли глобальная позиция
bool AntFile::IsNeedGlobalPosition(TrackAccessor & track, dword numFrames)
{
	Vector pos, minPos, maxPos;
	minPos.x = track(0, AnxTrackHeaderBase::index_px);
	minPos.y = track(0, AnxTrackHeaderBase::index_py);
	minPos.z = track(0, AnxTrackHeaderBase::index_pz);
	maxPos = minPos;
	float maxR = ~minPos;
	for(dword n = 1; n < numFrames; n++)
	{
		pos.x = track(n, AnxTrackHeaderBase::index_px);
		pos.y = track(n, AnxTrackHeaderBase::index_py);
		pos.z = track(n, AnxTrackHeaderBase::index_pz);
		minPos.Min(pos);
		maxPos.Max(pos);
		float r = ~pos;
		if(r > maxR)
		{
			maxR = r;
		}
	}					
	if(maxR > 0.1f)
	{
		const float maxDelta = 0.01f;
		if(~(maxPos - minPos) > maxDelta*maxDelta)
		{
			return true;
		}
	}
	return false;
}

//Пересчитать позиции трека в глобальную систему
void AntFile::ConvertToGlobal(dword boneIndex, byte * buffer, dword numFrames, bool isMovement)
{
	//Мастер скелет
	MasterSkeleton * s = opt.project->skeleton;
	//Копируем локальные позиции преобразуемого трека
	Vector * pos = (Vector *)buffer;
	TrackAccessor globalTrack(ant, bone[boneIndex].track, clipStartFrame, s->bones[boneIndex]);
	for(dword i = 0; i < numFrames; i++)
	{
		pos[i].x = globalTrack(i, AnxTrackHeaderBase::index_px);
		pos[i].y = globalTrack(i, AnxTrackHeaderBase::index_py);
		pos[i].z = globalTrack(i, AnxTrackHeaderBase::index_pz);
	}
	//Преобразуем позиции в глобальную систему
	Quaternion q;
	for(long b = s->bones[boneIndex]->parent; b >= 0; b = s->bones[b]->parent)
	{
		TrackAccessor track(ant, bone[b].track, clipStartFrame, s->bones[b]);
		for(dword i = 0; i < numFrames; i++)
		{
			//Строим матрицу		
			q.x = track(i, AnxTrackHeaderBase::index_qx);
			q.y = track(i, AnxTrackHeaderBase::index_qy);
			q.z = track(i, AnxTrackHeaderBase::index_qz);
			q.w = track(i, AnxTrackHeaderBase::index_qw);
			Matrix mtx;
			q.GetMatrix(mtx);
			if(b != 0 || isMovement == false)
			{
				mtx.pos.x = track(i, AnxTrackHeaderBase::index_px);
				mtx.pos.y = track(i, AnxTrackHeaderBase::index_py);
				mtx.pos.z = track(i, AnxTrackHeaderBase::index_pz);
			}else{
				mtx.pos.x = 0.0f;
				mtx.pos.y = track(i, AnxTrackHeaderBase::index_py);
				mtx.pos.z = 0.0f;
			}
			//Преобразуем локальную позицию в глобальную
			pos[i] = mtx*pos[i];
		}
	}
}



//Получить количество кадров в клипе
dword AntFile::GetNumFrames(bool applyPosition)
{
	dword curNumFrames = applyPosition ? clipNumFrames - 1 : clipNumFrames;
	if(curNumFrames == 0) curNumFrames = 1;
	return curNumFrames;
}

//Получить количество кадров в файле
dword AntFile::GetAntFrames(bool applyPosition)
{
	dword curNumFrames = applyPosition ? numFrames - 1 : numFrames;
	return curNumFrames;
}

//Получить размер исходных данных в клипе
dword AntFile::GetAntDataSize()
{
	return antSize - sizeof(AntFileHeader) - sizeof(AntFileBone)*numBones;
}

//Перевернуть клип
void AntFile::Reverse()
{
	UseAnt();
	if(!ant || header->framesCount < 2)
	{
		FreeAnt();
		return;
	}
	AntFileTrackElement tmp;
	AntFileTrackElement * track = (AntFileTrackElement *)(ant + tracksOffset);
	dword counter = (header->framesCount + 1)/2;
	for(dword b = 0; b < header->bonesCount; b++)
	{
		for(dword i = 0; i < counter; i++)
		{
			long i1 = i;
			long i2 = header->framesCount - 1 - i;
			tmp = track[i1];
			track[i1] = track[i2];
			track[i2] = tmp;
		}
		track += header->framesCount;
	}
	PackAnt();
}

//Проредить клип, удалив каждый второй ключ
void AntFile::Reduce()
{
	UseAnt();
	if(!ant || clipNumFrames < 2)
	{
		FreeAnt();
		return;
	}
	AntFileTrackElement * dst = (AntFileTrackElement *)(ant + tracksOffset);
	AntFileTrackElement * src = (AntFileTrackElement *)(ant + tracksOffset);
	dword pitch = header->framesCount;
	header->framesCount /= 2;
	clipStartFrame /= 2;
	clipNumFrames /= 2;
	fps *= 0.5f;
	for(dword b = 0; b < header->bonesCount; b++, src += pitch)
	{		
		for(dword i = 0; i < header->framesCount; i++)
		{
			*dst++ = src[i*2];
			Assert(i*2 < pitch);
		}
	}
	antSize = (byte *)dst - (byte *)ant;
	UpdateInfo();
	if(clipStartFrame > numFrames - 1) clipStartFrame = numFrames - 1;
	if(clipStartFrame + clipNumFrames > numFrames)
	{
		clipNumFrames = numFrames - clipStartFrame;
	}
	PackAnt();
	string errorStr;
	ApplyMasterSkeleton(&errorStr);
}

//Получить текущий кадр
void AntFile::GetFrame(long frame, Matrix & transform)
{
	transform.SetIdentity();
	UseAnt();
	if(!ant || !numFrames || !numBones)
	{
		FreeAnt();
		return;
	}
	//Получаем кадр в диапазоне
	if(frame < 0)
	{
		frame = 0;
	}
	if(frame >= (long)clipNumFrames)
	{
		frame = clipNumFrames - 1;
	}
	frame += clipStartFrame;
	if(frame < 0)
	{
		frame = 0;
	}
	if(frame >= (long)(numFrames - 1))
	{
		frame = numFrames - 1;
	}
	//Получаем матрицу трека
	AntFileTrackElement * rootTrack = (AntFileTrackElement *)(ant + bone[0].track);
	AntFileTrackElement & afte = *((AntFileTrackElement *)(ant + bone[0].track) + frame);
	Quaternion q(afte.qx, afte.qy, afte.qz, afte.qw);
	q.Normalize();
	q.GetMatrix(transform);
	transform.pos = Vector(afte.px, afte.py, afte.pz);	
	FreeAnt();
}

//Переместить трек
void AntFile::MoveTrackTo(long frame, const Matrix & newTransform)
{	
	UseAnt();
	if(!ant || !numFrames || !numBones)
	{
		FreeAnt();
		return;
	}
	//Получаем кадр в диапазоне
	if(frame < 0)
	{
		frame = 0;
	}
	if(frame >= (long)clipNumFrames)
	{
		frame = clipNumFrames - 1;
	}
	frame += clipStartFrame;
	if(frame < 0)
	{
		frame = 0;
	}
	if(frame >= (long)(numFrames - 1))
	{
		frame = numFrames - 1;
	}
	//Получаем матрицу для преобразования трека
	AntFileTrackElement * rootTrack = (AntFileTrackElement *)(ant + bone[0].track);
	AntFileTrackElement & afte = *((AntFileTrackElement *)(ant + bone[0].track) + frame);
	Quaternion q(afte.qx, afte.qy, afte.qz, afte.qw);
	q.Normalize();
	Matrix boneTransform;
	q.GetMatrix(boneTransform);
	boneTransform.pos = Vector(afte.px, afte.py, afte.pz);
	Matrix inverseTransform(boneTransform);
	inverseTransform.Inverse();
	Matrix transform(inverseTransform, newTransform);
	//Мастер скелет
	Assert(opt.project->skeleton);
	MasterSkeleton * s = opt.project->skeleton;
	//Пересчитываем треки рутовых костей
	for(long boneIndex = 0; boneIndex < bone; boneIndex++)
	{
		if(s->bones[boneIndex]->parent >= 0)
		{
			continue;
		}
		AntFileTrackElement * track = (AntFileTrackElement *)(ant + bone[boneIndex].track);
		for(dword i = 0; i < numFrames; i++)
		{
			AntFileTrackElement & ae = track[i];
			Quaternion q(ae.qx, ae.qy, ae.qz, ae.qw);
			q.Normalize();
			Matrix mtx;
			q.GetMatrix(mtx);
			mtx.pos = Vector(ae.px, ae.py, ae.pz);
			Matrix result(mtx, transform);
			ae.px = result.pos.x;
			ae.py = result.pos.y;
			ae.pz = result.pos.z;
			q.Set(result);
			ae.qx = q.x;
			ae.qy = q.y;
			ae.qz = q.z;
			ae.qw = q.w;
		}
	}
	PackAnt();
}

//Разархивировать данные для использования
void AntFile::UseAnt()
{
	useCounter++;
	if(ant) return;
	if(pantIndex < 0) return;
	dword pantSize = 0;
	byte * pant = opt.project->clipsRepository.Get(pantIndex, pantSize);
	ant = NEW byte[antSize];
	if(UnPack(pant, pantSize, ant, antSize))
	{
		UpdateInfo();
	}else{
		FreeAnt();
	}
}

//Освободить разархивированные ресурсы
void AntFile::FreeAnt()
{
	useCounter--;
	Assert(useCounter >= 0);
	if(useCounter <= 0)
	{
		if(ant) delete ant; ant = null;
		UpdateInfo();
	}
}

//Запокавать загруженные данные
bool AntFile::PackAnt()
{
	Assert(ant);
	dword pantSize = antSize + 16;
	byte * pant = NEW byte[pantSize];
	if(!Pack(ant, antSize, pant, pantSize))
	{
		delete ant; ant = null;
		antSize = 0;
		delete pant; pant = null;
		pantSize = 0;
		return false;
	}
	pantIndex = opt.project->clipsRepository.Add(pant, pantSize);
	delete pant;
	useCounter = 1;
	FreeAnt();
	return true;
}

//Сжать данные
bool AntFile::Pack(void * src, dword srcSize, void * dst, dword & dstSize)
{
	if(!src || srcSize == 0 || !dst || srcSize + 1 > dstSize)
	{
		return false;
	}
	//Жмём 2-мя методами и выбираем лучший
	byte & type = *(byte *)dst;
	type = 'S';
	dst = (byte *)dst + 1;
	opt.compressor.Compress((const byte *)src, srcSize, Compressor::cm_fast);
	const byte * buffer = opt.compressor.Buffer();
	dstSize = opt.compressor.Size();
	if(dstSize >= srcSize)
	{
		buffer = (const byte *)src;
		dstSize = srcSize;
		type = 'N';
	}
	memcpy(dst, buffer, dstSize);
	dstSize++;
	return true;
}

//Расжать данные
bool AntFile::UnPack(void * src, dword srcSize, void * dst, dword dstSize)
{
	if(!src || !srcSize || !dst || !dstSize)
	{
		return false;
	}
	byte & type = *(byte *)src;
	src = (byte *)src + 1;
	srcSize--;
	if(type == 'S')
	{
		dword dsize = dstSize;
		if(Compressor::Decompress((const byte *)src, srcSize, (byte *)dst, dsize) != Compressor::dr_ok)
		{
			return false;
		}
		if(dsize != dstSize)
		{
			return false;
		}		
	}else
	if(type == 'N')
	{
		if(srcSize != dstSize + 1) return false;
		memcpy(dst, src, dstSize);
	}else{
		return false;
	}
	return true;
}


__forceinline AntFile::TrackAccessor::TrackAccessor(byte * base, long offset, long firstFrame, MasterSkeleton::Bone * b)
{
	if(offset)
	{
		track = (AntFileTrackElement *)(base + offset);
		track += firstFrame;
		bone = null;
	}else{
		track = null;
		bone = b;
	}
}

__forceinline float AntFile::TrackAccessor::operator () (dword frame, AnxTrackHeaderBase::Flags index)
{
	if(track)
	{
		switch(index)
		{
		case AnxTrackHeaderBase::index_qx: return track[frame].qx;
		case AnxTrackHeaderBase::index_qy:	return track[frame].qy;
		case AnxTrackHeaderBase::index_qz:	return track[frame].qz;
		case AnxTrackHeaderBase::index_qw:	return track[frame].qw;
		case AnxTrackHeaderBase::index_px: return track[frame].px;
		case AnxTrackHeaderBase::index_py:	return track[frame].py;
		case AnxTrackHeaderBase::index_pz:	return track[frame].pz;
		}
	}else{
		switch(index)
		{
		case AnxTrackHeaderBase::index_qx: return bone->q.x;
		case AnxTrackHeaderBase::index_qy:	return bone->q.y;
		case AnxTrackHeaderBase::index_qz:	return bone->q.z;
		case AnxTrackHeaderBase::index_qw:	return bone->q.w;
		case AnxTrackHeaderBase::index_px: return bone->p.x;
		case AnxTrackHeaderBase::index_py:	return bone->p.y;
		case AnxTrackHeaderBase::index_pz:	return bone->p.z;
		}
	}
	Assert(false);
	return 0.0f;
}

__forceinline float * AntFile::TrackAccessor::Get(AnxTrackHeaderBase::Flags index)
{
	if(track)
	{
		switch(index)
		{
		case AnxTrackHeaderBase::index_qx: return &track[0].qx;
		case AnxTrackHeaderBase::index_qy:	return &track[0].qy;
		case AnxTrackHeaderBase::index_qz:	return &track[0].qz;
		case AnxTrackHeaderBase::index_qw:	return &track[0].qw;
		case AnxTrackHeaderBase::index_px: return &track[0].px;
		case AnxTrackHeaderBase::index_py:	return &track[0].py;
		case AnxTrackHeaderBase::index_pz:	return &track[0].pz;
		}
	}
	return null;
}

__forceinline AntFile::ComponentInfo::ComponentInfo()
{
	track = null;
	shift = 0;
	type = AnxTrackHeaderBase::code_const;
	offset = 0;
	size = 0;
}