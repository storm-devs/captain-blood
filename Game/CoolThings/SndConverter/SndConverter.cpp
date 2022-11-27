#include "SndConverter.h"

struct ITextFile : public IFile
{
	void Print(const char *format, ...)
	{
		va_list  args;
		va_start(args,format);

		static char buf[1024];

		Write(buf,crt_vsnprintf(buf,sizeof(buf),format,args));

		va_end(args);
	}
};

struct UniqId
{
	dword data[4];

	__forceinline UniqId()
	{ 
		Reset();
	}

	//Обнулить идентификатор
	__forceinline void Reset()
	{ 
		data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0; 
	}

	__forceinline bool IsValidate() const
	{
		return (data[0] | data[1] | data[2] | data[3]) != 0;
	}

	#ifndef NO_TOOLS
	#include <Rpc.h>
	#pragma comment(lib, "Rpcrt4")
	#endif

	#ifndef NO_TOOLS
	//Сгенерировать уникальный идентификатор
	void Build()
	{
		UUID uid;
		RPC_STATUS retUuidCreate = UuidCreate(&uid);
		Assert(retUuidCreate == RPC_S_OK);
		Assert(sizeof(uid) == sizeof(UniqId));
		memcpy(&data, &uid, sizeof(UniqId));
		Assert(IsValidate());
	#else
	//Сюда не должно попадать
	Assert(false);
	#endif
	}

	//Сконвертировать идентификатор в строку, оканчивающуюся на 0 (забирать сразу в свой буфер)
	const char *ToString() const
	{
		static char buffer[(sizeof(data)*2 + 3 + 1) + 16];
		static const char * hex = "0123456789ABCDEF\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
		const byte * src = (const byte *)&data;	
		char * dst = buffer;
		for(dword i = 0; i < sizeof(data); i++)
		{
			if(i == 2 || i == 4 || i == 8)
			{
				*dst = '-';
				dst++;
			}
			byte b = *src;
			dst[0] = hex[(b >> 4) & 0xf];
			dst[1] = hex[b & 0xf];
			src++;
			dst+= 2;
		}
		*dst = 0;
		return buffer;
	}

	//Востановить идентификатор из строки
	bool FromString(const char *s)
	{
		data[0] = data[1] = data[2] = data[3] = 0;
		if(!s) return false;
		dword buf[4];
		buf[0] = buf[1] = buf[2] = buf[3] = 0;
		const char * src = s;
		byte * dst = (byte *)buf;
		for(dword i = 0; i < sizeof(data)*2; i++, src++)
		{
			if(i == 2*2 || i == 4*2 || i == 8*2)
			{
				if(*src != '-')
				{
					//Ошибка формата
					return false;
				}
				src++;
			}
			dword hex = 0;
			switch(*src)
			{
			case '0': hex = 0; break;
			case '1': hex = 1; break;
			case '2': hex = 2; break;
			case '3': hex = 3; break;
			case '4': hex = 4; break;
			case '5': hex = 5; break;
			case '6': hex = 6; break;
			case '7': hex = 7; break;
			case '8': hex = 8; break;
			case '9': hex = 9; break;
			case 'A': 
			case 'a': hex = 10; break;
			case 'B': 
			case 'b': hex = 11; break;
			case 'C': 
			case 'c': hex = 12; break;
			case 'D': 
			case 'd': hex = 13; break;
			case 'E': 
			case 'e': hex = 14; break;
			case 'F': 
			case 'f': hex = 15; break;
			default:
				//Ошибка формата
				return false;
			};
			if((i & 1) == 0)
			{
				*dst = hex << 4;			
			}else{
				*dst |= hex;
				dst++;
			}
		}
		data[0] = buf[0];
		data[1] = buf[1];
		data[2] = buf[2];
		data[3] = buf[3];
		return true;
	}
};
/*
const char *GetUniqID(const char *id, dword len)
{
	static char uid[32 + 3 + 1] = "0000-0000-00000000-0000000000000000";

	memcpy(uid + 21,id,len);

//	UniqId ID;
//	Verify(ID.FromString(uid))

	return uid;
}
*/
struct SndProject
{
	struct ExtName
	{
		const char *str;	//Строка
		dword len;			//Длина имени в символах
		dword hash;			//Хэшь имени
		long w;				//Ширина имени в пикселях для текущего шрифта
		long h;
	};

	enum Consts
	{
		prjStringLen = 64 - sizeof(ExtName),
		prjWaveIdLen = 14,
	};

	struct PrjString
	{
		ExtName	info;
		char	data[prjStringLen];
	};

	enum SoundBaseParamConsts
	{
		sbpc_db_min = -96,
		sbpc_db_max = 48,
		sbpc_db_init = 0,
		sbpc_count_max = 64,
		sbpc_count_inf = -1,
		sbpc_count_init = 16,
		sbpc_priority_min = 0,
		sbpc_priority_norm = 5,
		sbpc_priority_max = 10,
		sbpc_priority_init = sbpc_priority_norm,
		//Методы выбора волны
		sbpc_select_rnd = 0,			//Выбирать волны случайным образом
		sbpc_select_rnd_queue = 1,		//Выбирать волны случайно, избегая повторений с учётом весов
		sbpc_select_queue = 2,			//Выбирать волны случайно, избегая повторений и отыгрывая все волны по разу
		sbpc_select_sequence = 3,		//Выбирать волны последовательно как они заданны
		sbpc_select_min = sbpc_select_rnd,
		sbpc_select_max = sbpc_select_sequence,
		//Зацикленные звуки
		sbpc_loop_disable = 0,			//Звук не зациклен
		sbpc_loop_one_wave = 1,			//Отыгрывать зацикленно звук с 1й волной
//		sbpc_loop_sel_wave_rnd = 2,		//Отыгрывать зацикленно звук каждый раз выбирая новую волну как в mode_select_rnd
//		sbpc_loop_sel_wave_que = 3,		//Отыгрывать зацикленно звук каждый раз выбирая новую волну как в mode_select_queue
		sbpc_loop_min = sbpc_loop_disable,
//		sbpc_loop_max = sbpc_loop_sel_wave_que,
		sbpc_loop_max = sbpc_loop_one_wave,
		//Какие эффекты применять 
		sbpc_flags_fx_full = 0,			//Применять к звуку все эффекты
		sbpc_flags_fx_premaster = 1,	//Пропусить эфект окружающей среды
		sbpc_flags_fx_master = 2,		//Пропусить эфект окружающей среды и премастера
		sbpc_flags_fx_music = 3,		//Музыка
		sbpc_flags_fx_init = sbpc_flags_fx_full,
		sbpc_flags_fx_min = sbpc_flags_fx_full,
		sbpc_flags_fx_max = sbpc_flags_fx_music,
		sbpc_flags_fx_mask = 15,
		//Флаги
		sbpc_flags_init = sbpc_flags_fx_full,
		sbpc_flags_phoneme = 16,		//Генерить для волн фонемы
	};

	struct SoundBaseParams
	{
		long priority;				//Приоритет по отношению к остальным звукам
		long maxCount;				//Максимальное количество одновременно проигрываемых данных звуков
		float volume;				//Общая громкость звука
		long selectMethod;			//Метод выбора волн
		long loopMode;				//Режим зацикленного проигрывания
		long flags;					//Флаги
		dword reserved[10];
	};

	struct SoundBaseParamsData
	{
		PrjString name;	SoundBaseParams params;

		static const char *srcName()
		{
			return "sndbparams.v10";
		}

		void Save(ITextFile *ini)
		{
			ini->Print("[Setup]\r\n");

			ini->Print("priority = %d\r\n", params.priority);
			ini->Print(	  "count = %d\r\n", params.maxCount);
			ini->Print(	 "volume = %f\r\n", params.volume);
			ini->Print("selector = %d\r\n", params.selectMethod);
			ini->Print(	   "loop = %d\r\n", params.loopMode);
			ini->Print(		 "fx = %d\r\n", params.flags);
			ini->Print(	"phoneme = %d\r\n",(params.flags&SndProject::sbpc_flags_phoneme) ? 1 : 0);
		}
	};

	struct SoundAttGraph
	{
		float c[4];					//Коэфициенты графика затухания
		float minDist;				//Минимальныя дистанция
		float maxDist;				//Максимальныя дистанция
		dword reserved[10];
	};

	struct AttGraphData
	{
		PrjString name; SoundAttGraph attGraph;

		static const char *srcName()
		{
			return "attenuations.v10";
		}

		void Save(ITextFile *ini)
		{
			ini->Print("[Attenuation]\r\n");

			ini->Print("mindist = %f\r\n",attGraph.minDist);
			ini->Print("maxdist = %f\r\n",attGraph.maxDist);

			for( int j = 0 ; j < ARRSIZE(attGraph.c); j++ )
			{
				ini->Print("c%d = %f\r\n",j,attGraph.c[j]);
			}
		}
	};

	struct PrjFolder
	{
		PrjString name;
		long parent;
		long child;
		long next;
		long prev;
		dword reserved[4];
	};
};

template <class DataType>
void Convert(array<DataType> &list, array<UniqId> &uids,
	IFileService *storage, const char *src, const char *dst, const char *to)
{
	string name(src); name += "\\"; name += DataType::srcName();

	IFile *file = storage->OpenFile(name.c_str(),file_open_existing_for_read,_FL_);

	if( file )
	{
		if( dword count = file->Size()/sizeof(DataType))
		{
			string folder(dst); folder += "\\"; folder += to; folder += "\\";

			bool ok = storage->CreateFolder(folder);
			Assert(ok)

			DataType data; string temp;

			file->SetPos(0);

			for( dword i = 0 ; i < count ; i++ )
			{
				dword size = file->Read(&data,sizeof(DataType));
				Assert(size == sizeof(DataType))

				temp = folder; temp += data.name.data; temp += ".txt";

				ITextFile *ini = (ITextFile *)storage->OpenFile(temp,file_create_always,_FL_);
				Assert(ini)

				UniqId &id = uids[uids.Add()];

				id.Build();

				ini->Print("id = %s\r\n\r\n",id.ToString());

				data.Save(ini);

				list.Add(data);

				RELEASE(ini)
			}
		}

		RELEASE(file)
	}
}

struct WaveOptions
{
	dword samplerate;		//Частота сэмплирования
	byte channels;			//Количество каналов
	byte format;			//Формат волны
	byte compression;		//Степень сжатия
	byte platform;			//Платформа
	dword reserved[2];
};

struct GUIPoint
{
	int x;
	int y;
};

struct ProjectWave
{
	enum GlobalConsts
	{
		gc_nameLen = 48,
	};

	struct DataChunk
	{
		long version;					//Версия данных
		long folderIndex;				//Папка в которой лежит волна
		GUIPoint hotPreview[64];		//Предпросмотр (высота 0..16)
		dword hotPreviewCount;			//Количество точек в предпросмотре
		dword hotPreviewStep;			//Сколько отсчётов в 1ой точке предпросмотра
		float playTime;					//Время проигрывания
		char playTimeData[16];			//Строковой вариант времени проигрывания
		dword bytesPerSample;			//Байт на выборку
		dword samplesCount;				//Количество отсчётов в файле
		long loopCount;					//-1 нет цикла, 0 бесконечный цикл
		dword loopStartOffset;			//Смещение начала цикла
		dword loopEndOffset;			//Смещение окончания цикла
		char nameData[gc_nameLen];		//Имя волны
		char sourcePath[MAX_PATH + 1];	//Откуда взяли
		dword sourceLowDateTime;		//Время последней модификации файла (lo)
		dword sourceHighDateTime;		//Время последней модификации файла (hi)
		WAVEFORMATEX waveFormat;		//Формат волны
		WaveOptions exportOptionsPC;	//Опции выгрузи для PC
		WaveOptions exportOptionsXBOX;	//Опции выгрузи для XBOX
		dword dataSize;					//Размер данных
		dword flags;					//Различные флажки
		dword waveCheckSum;				//Контрольная сумма
		UniqId id;						//Уникальный идентификатор волны
		dword reserved[8];
	};
};

struct Target
{
	struct ProjectWave
	{
		struct DataChunk
		{
			char idString[64];				//Идентификатор преобразованный в строку
			long version;					//Версия данных
			dword dataSize;					//Размер данных
			float playTime;					//Время проигрывания
			dword samplesCount;				//Количество отсчётов в файле
			dword bytesPerSample;			//Байт на выборку
			WAVEFORMATEX waveFormat;		//Формат волны		
			GUIPoint hotPreview[64];		//Предпросмотр (высота 0..16)
			dword hotPreviewCount;			//Количество точек в предпросмотре
			dword hotPreviewStep;			//Сколько отсчётов в 1ой точке предпросмотра		
			dword sourceLowDateTime;		//Время последней модификации файла (lo)
			dword sourceHighDateTime;		//Время последней модификации файла (hi)
			dword waveCheckSum;				//Контрольная сумма
			WaveOptions exportOptionsPC;	//Опции выгрузи для PC
			WaveOptions exportOptionsXBOX;	//Опции выгрузи для XBOX		
			char sourcePath[1024];			//Откуда взяли
			char note[4096];				//Коментарии
		};
	};
};

struct ProjectSound
{
	enum Consts
	{
		c_namelen = 16,
	};

	struct Wave
	{
		long waveIndex;				//Индекс волны в проекте
		float weight;				//Веc выбора волны (ненормализованое значение)
		float time;					//Время дополнительной активности волны (занимает виртуально канал)
		float volume;				//Громкость 0..1
	};

	struct SoundDataChunk
	{
		char nameData[c_namelen];	//Имя звука
		long folderIndex;			//Папка в которой лежит волна
		long baseParams;			//Базовые параметры звука
		long attenuationIndex;		//Индекс графика затухания		
		Wave waves[256];			//Волны
		long wavesCount;			//Количество волн
		dword reserved[16];
	};
};

void BuildPath(IFileService *storage, string &path, array<SndProject::PrjFolder> &folders, long folderIndex)
{
	if( folders[folderIndex].parent >= 0 )
	{
		BuildPath(storage,path,folders,folders[folderIndex].parent);

		path += folders[folderIndex].name.data;
		path += "\\";

		storage->CreateFolder(path);
	}
}

CREATE_CLASS(SndConverter)

bool SndConverter::Init()
{
	IFileService *storage = (IFileService *)api->GetService("FileService");
	Assert(storage);

	api->Trace("===================================== SndConverter::BEGIN =======================\n");

	const char *src = "sound project";
	const char *dst = "sound project_\0";

	const char *err	= "";
	bool ok = true;

	if( storage->IsExist(dst))
	{
		SHFILEOPSTRUCT fos = {0};

		fos.wFunc	= FO_DELETE;
		fos.pFrom	= dst;
		fos.fFlags	= FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;

		ok = !SHFileOperation(&fos);

		if( !ok )
			err = "(can't delete root folder)";
	}

	if( ok )
	{
		ok = storage->CreateFolder(dst);

		if( !ok )
			err = "(can't create root folder)";
	}

	api->Trace("Prepare root folder: %s %s",ok ? "done" : "failed",err);

	if( ok )
	{
		array<UniqId> sids(_FL_);
		array<UniqId> aids(_FL_);

		array<SndProject::SoundBaseParamsData> slist(_FL_);
		array<SndProject::AttGraphData>		   alist(_FL_);

		Convert<SndProject::SoundBaseParamsData>(slist,sids,storage,src,dst,"setups");
		Convert<SndProject::AttGraphData>		(alist,aids,storage,src,dst,"attenuations");

		////////////////////////////////////////////////////

		dword waveCount = 0;
		char *waveID = null;

		array<UniqId> waveUID(_FL_);

		string name(src); name += "\\"; name += "waves.v10";

		IFile *file = storage->OpenFile(name.c_str(),file_open_existing_for_read,_FL_);

		if( file )
		{
			dword size = file->Size();

			waveCount = size/SndProject::prjWaveIdLen;
			Assert(size == waveCount*SndProject::prjWaveIdLen)

			file->SetPos(0);

			waveID = NEW char[size];
			file->Read(waveID,size);

			RELEASE(file)

			waveUID.AddElements(waveCount);

			for( int i = 0 ; i < waveUID ; i++ )
			{
				waveUID[i].Build();
			}
		}

		array<SndProject::PrjFolder> folders(_FL_);

		name = src; name += "\\"; name += "folders.v10";

		file = storage->OpenFile(name.c_str(),file_open_existing_for_read,_FL_);

		if( file )
		{
			dword count = (file->Size() + sizeof(long))/sizeof(SndProject::PrjFolder);

			if( count )
			{
				file->SetPos(0);

				long firstEmpty;

				dword size = file->Read(&firstEmpty,sizeof(firstEmpty));
				Assert(size == sizeof(firstEmpty))

				folders.AddElements(count);

				size = file->Read(folders.GetBuffer(),folders.GetDataSize());
				Assert(size == folders.GetDataSize())

				for( int k = 0 ; k < folders ; k++ )
				{
					SndProject::PrjFolder &folder = folders[k];

					if( folder.parent < 0 )
					{
						if( string::IsEqual(folder.name.data,"Waves:"))
						{
							folder.parent = -10;
						}
						else
						if( string::IsEqual(folder.name.data,"Sounds:"))
						{
							folder.parent = -20;
						}
					}
				}

			/*	for( int k = 0 ; k < folders ; k++ )
				{
					api->Trace("%d %s %d",k,folders[k].name.data,folders[k].parent);
				}*/
			}

			RELEASE(file)
		}

		/////////////////////////////////

		name = src; name += "\\movies\\";

		IFinder *finder = storage->CreateFinder(name, "*.txt",find_no_recursive|find_no_mirror_files | find_no_files_from_packs,_FL_);
		Assert(finder);

		if( dword count = finder->Count())
		{
			string folder(dst); folder += "\\"; folder += "Movies"; folder += "\\";

			bool ok = storage->CreateFolder(folder);
			Assert(ok)

			string temp;

			array<BYTE> buff(_FL_);

			for( dword i = 0 ; i < count ; i++ )
			{
				IFile *mov = storage->OpenFile(finder->FilePath(i),file_open_existing_for_read,_FL_);

				if( mov )
				{
					dword fileSize = mov->Size();

					buff.Empty();
					buff.AddElements(fileSize);

					dword size = mov->Read(buff.GetBuffer(),buff.GetDataSize());
					Assert(size == buff.GetDataSize())

					temp = folder; temp += finder->Name(i);

					IFile *bin = storage->OpenFile(temp,file_create_always,_FL_);
					Assert(bin)

					const char *text = (const char *)buff.GetBuffer();

					const char *glb_ = crt_strstr(text,"glb_");
					Assert(glb_)

					dword off = glb_ - text + 4;

					bin->Write(text,off);

					UniqId ID; ID.Build();

					const char *id = ID.ToString();

					bin->Write(id,strlen(id));
					bin->Write("_",1);

					bin->Write(text + off,fileSize - off);

					RELEASE(bin)
				}
			}
		}

		////////////////////////////////

		name = src; name += "\\waves\\";

		ProjectWave::DataChunk wave;

	/*	IFinder *finder = storage->CreateFinder(name, "*.*",find_no_recursive|find_no_mirror_files,_FL_);
		Assert(finder);

		if( dword count = finder->Count())*/
		if( dword count = waveCount )
		{
			string folder(dst); folder += "\\"; folder += "Waves"; folder += "\\";

			bool ok = storage->CreateFolder(folder);
			Assert(ok)

			string temp;

			Target::ProjectWave::DataChunk dest = {0};

			const int buff_size = 1024*128;
			BYTE buff[buff_size];

			char waveName[SndProject::prjWaveIdLen + 1] = {0};

			for( dword i = 0 ; i < count ; i++ )
			{
				const char *id = waveID + SndProject::prjWaveIdLen*i;

				if( !id[0] )
					continue;

				memcpy(waveName,id,SndProject::prjWaveIdLen);

				temp = name; temp += waveName;

			//	IFile *wav = storage->OpenFile(finder->FilePath(i),file_open_existing_for_read,_FL_);
				IFile *wav = storage->OpenFile(temp,file_open_existing_for_read,_FL_);

				if( wav )
				{
					dword size = wav->Read(&wave,sizeof(ProjectWave::DataChunk));
					Assert(size == sizeof(ProjectWave::DataChunk))

					temp = folder;

					long folderIndex = wave.folderIndex;

				//	api->Trace("%d",folderIndex);

					Assert(folderIndex >= 0)
				//	Assert(folderIndex < folders)

					if( folderIndex < folders )
					{
						BuildPath(storage,temp,folders,folderIndex);
					}
					else
					{
						api->Trace("wave %s: invalid folder index",/*finder->Name(i)*/waveName);
					}

				//	Assert(folders[folderIndex].parent == -10)
				//	api->Trace("    %d",folderIndex);

				//	temp += finder->Name(i);
					temp += wave.nameData;

					temp += ".pwv";

					IFile *bin = storage->OpenFile(temp,file_create_always,_FL_);
					Assert(bin)

					ZeroMemory(&dest,sizeof(dest) - sizeof(dest.note));

				//	crt_strcpy(dest.idString,sizeof(dest.idString),
				//		GetUniqID(/*finder->Name(i)*/waveName,SndProject::prjWaveIdLen));
					crt_strcpy(dest.idString,sizeof(dest.idString),
						waveUID[i].ToString());

					dest.dataSize			= wave.dataSize;
					dest.playTime			= wave.playTime;
					dest.samplesCount		= wave.samplesCount;
					dest.bytesPerSample		= wave.bytesPerSample;
					dest.waveFormat			= wave.waveFormat;

					memcpy(dest.hotPreview,wave.hotPreview,sizeof(wave.hotPreview));

					dest.hotPreviewCount	= wave.hotPreviewCount;
					dest.hotPreviewStep		= wave.hotPreviewStep;
					dest.sourceLowDateTime	= wave.sourceLowDateTime;
					dest.sourceHighDateTime	= wave.sourceHighDateTime;
					dest.waveCheckSum		= wave.waveCheckSum;
					dest.exportOptionsPC	= wave.exportOptionsPC;
					dest.exportOptionsXBOX	= wave.exportOptionsXBOX;

					crt_strcpy(dest.sourcePath,sizeof(dest.sourcePath),wave.sourcePath);

					bin->Write(&dest,sizeof(dest));

					dword dataSize = wav->Size() - sizeof(ProjectWave::DataChunk);

				/*	if( abs(int(dataSize - dest.dataSize)) > 32 )
					{
						api->Trace("wave %s: invalid actual data size: %d instead %d",
							finder->Name(i),dataSize,dest.dataSize);
					}*/

					for( dword quant,total = 0 ; total < dataSize ; total += quant )
					{
						quant = buff_size;

						if( total + quant > dataSize )
							quant = dataSize - total;

						size = wav->Read(buff,quant);
						Assert(size == quant)

						bin->Write(buff,quant);
					}

					RELEASE(bin)
				}
				else
				{
					api->Trace("source wave not found: %s",temp.c_str());
				}

				RELEASE(wav)
			}
		}

		/////////////////

		name = src; name += "\\"; name += "sounds.v10";

		file = storage->OpenFile(name.c_str(),file_open_existing_for_read,_FL_);

		if( file )
		{
			if( dword count = file->Size()/sizeof(ProjectSound::SoundDataChunk))
			{
				string folder(dst); folder += "\\"; folder += "Sounds"; folder += "\\";

				bool ok = storage->CreateFolder(folder);
				Assert(ok)

				ProjectSound::SoundDataChunk data; string temp;

				file->SetPos(0);

				for( dword i = 0 ; i < count ; i++ )
				{
					dword size = file->Read(&data,sizeof(ProjectSound::SoundDataChunk));
					Assert(size == sizeof(ProjectSound::SoundDataChunk))

					if( !data.nameData[0] )
						continue;

					temp = folder;

					long folderIndex = data.folderIndex;

				//	api->Trace("%d",folderIndex);

					Assert(folderIndex >= 0)
				//	Assert(folderIndex < folders)

					if( folderIndex < folders )
					{
						BuildPath(storage,temp,folders,folderIndex);
					}
					else
					{
						api->Trace("sound %s: invalid folder index",data.nameData);
					}

				//	Assert(folders[folderIndex].parent == -20)
				//	api->Trace("    %d",folderIndex);

					temp += data.nameData; temp += ".txt";

					ITextFile *ini = (ITextFile *)storage->OpenFile(temp,file_create_always,_FL_);
					Assert(ini)

					ini->Print("[Sound]\r\n");

					UniqId id; id.Build();

					ini->Print("id = %s\r\n",id.ToString());

					bool setValid =
						data.baseParams >= 0 &&
						data.baseParams < sids;

					bool attValid =
						data.attenuationIndex >= 0 &&
						data.attenuationIndex < aids;

					if( setValid )
					{
						ini->Print("setupid = %s\r\n",
							sids[data.baseParams].ToString());
					}
					else
					{
						ini->Print("setupid = %s\r\n","0000-0000-00000000-0000000000000000");
					}

					if( attValid )
					{
						ini->Print("attenuationid = %s\r\n",
							aids[data.attenuationIndex].ToString());
					}
					else
					{
						ini->Print("attenuationid = %s\r\n","0000-0000-00000000-0000000000000000");
					}
	
				//	ini->Print(		 "setupid = %d\r\n",data.baseParams);
				//	ini->Print("attenuationid = %d\r\n",data.attenuationIndex);
					ini->Print(	  "wavescount = %d\r\n",data.wavesCount);

					if( setValid )
					{
						ini->Print("\r\n");

						slist[data.baseParams].Save(ini);
					}

					if( attValid )
					{
						ini->Print("\r\n");

						alist[data.attenuationIndex].Save(ini);
					}

					for( int j = 0 ; j < data.wavesCount ; j++ )
					{
						ini->Print("\r\n[wave_%d]\r\n",j);

						const ProjectSound::Wave &wave = data.waves[j];

					//	ini->SetString(section,"id",wave.waveId.ToString());

						if( wave.waveIndex >= 0 &&
							wave.waveIndex < (long)waveCount )
						{
						/*	char *id = waveID + wave.waveIndex*SndProject::prjWaveIdLen;

							ini->Print("id = %s\r\n",
								GetUniqID(id,SndProject::prjWaveIdLen));*/
							ini->Print("id = %s\r\n",
								waveUID[wave.waveIndex].ToString());
						}
						else
						{
							ini->Print("id = %s\r\n",
								"0000-0000-00000000-0000000000000000");
						}

						ini->Print("weight = %f\r\n",wave.weight);
						ini->Print(	 "time = %f\r\n",wave.time);
						ini->Print("volume = %f\r\n",wave.volume);
					}

					RELEASE(ini)
				}
			}

			RELEASE(file)
		}

		DELETE_ARRAY(waveID)
	}

	api->Trace(ok ? "\nDONE" : "\nFAILED");

	api->Trace("\n===================================== SndConverter::END   =======================\n");

	api->Exit();

	return true;
}
