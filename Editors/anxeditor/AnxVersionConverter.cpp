

#include "AnxBase.h"


class AnxVersionConverter : public RegObject
{
	bool Init();
	bool ConvertAnx(byte * data, dword size);
	bool Conversion_v0_to_v1(byte * data, dword size);
};

#ifndef NO_TOOLS
CREATE_CLASS(AnxVersionConverter)
#endif

bool AnxVersionConverter::Init()
{
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IFinder * finder = fs->CreateFinder("Resource", "*.anx", find_all_files_no_mirrors, _FL_);
	api->Trace("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
	api->Trace("==============================================================================\r\n");
	api->Trace("Convert all anx files in folder Resource, total find: %u", finder->Count());
	dword badCount = 0;
	for(dword i = 0; i < finder->Count(); i++)
	{
		const char * path = finder->FilePath(i);
		api->Trace("%4i: Convert anx file: %s", i, path);
		IFile * file = fs->OpenFile(path, file_open_existing, _FL_);
		if(!file)
		{
			api->Trace("    IO error. Can't open file.");
			continue;
		}
		dword size = file->Size();
		byte * data = NEW byte[size];
		bool isConvertOk = false;
		if(file->Read(data, size) == size)
		{
			isConvertOk = ConvertAnx(data, size);
		}else{
			api->Trace("    IO error. Can't read data from file.");
		}
		if(isConvertOk)
		{
			file->SetPos(0);
			if(file->Write(data, size) != size)
			{
				api->Trace("    IO error. Can't write data to file.");
			}
		}else{
			badCount++;
		}
		file->Release();
		delete data;
	}
	api->Trace("\r\n\r\nAnx conversion coplette");
	if(badCount)
	{
		api->Trace("\r\nErrors: %u", badCount);
	}		
	api->Trace("\r\n\r\n==============================================================================\r\n");
	api->Trace("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
	finder->Release();
	api->Exit();
	return true;
}

bool AnxVersionConverter::ConvertAnx(byte * data, dword size)
{
	//Проверяем идентификатор файла и версию
	AnxHeaderId * anxId = (AnxHeaderId *)data;
	if(size < sizeof(AnxHeaderId) + sizeof(AnxHeader))
	{
		api->Trace("    Data error: Incorrect file size.");
		return false;
	}
	if(!anxId->CheckId())
	{
		api->Trace("    Data error: Incorrect file ID.");
	}
	//Смотрим на версию и решаем что делать
	if(anxId->CheckVer())
	{
		return true;
	}
	if(anxId->CheckVer(0))
	{
		return Conversion_v0_to_v1(data, size);
	}
	api->Trace("    Convertion error: Unknown file version.");
	return false;	
}


/*
Проблема версии 0 заключаеться в том, что массивы указателей AnxPointer<AnxPointer<*>>
оказались непросвизленны, те в формате PC. Задумывалось что все данные лежат в формате
ХВОХ360 от старшего байта к младшему. Поэтому надо получить указатели на массивы и отсвизлить массивы.
*/
bool AnxVersionConverter::Conversion_v0_to_v1(byte * data, dword size)
{
	//Заголовок
	AnxHeader * header = (AnxHeader *)(data + sizeof(AnxHeaderId));
	AnxGlobal_4byteAlignedSwizzler(header, sizeof(AnxHeader));
	if(header->numBones == 0 || header->numBones > 256 || header->numNodes == 0)
	{
		api->Trace("    Data error: Damage file data.");
		return false;
	}	
	header->htBones.RestoreNoSwizzle(data, size);
	AnxGlobal_4byteAlignedSwizzler(header->htBones.ptr, (header->htBonesMask + 1)*sizeof(AnxPointer<AnxBone>));
	header->htBones.PrepareNoSwizzle(data);
	header->htShortBones.RestoreNoSwizzle(data, size);
	AnxGlobal_4byteAlignedSwizzler(header->htShortBones.ptr, (header->htShortBonesMask + 1)*sizeof(AnxPointer<AnxBone>));
	header->htShortBones.PrepareNoSwizzle(data);
	header->htNames.RestoreNoSwizzle(data, size);
	AnxGlobal_4byteAlignedSwizzler(header->htNames.ptr, (header->htNamesMask + 1)*sizeof(AnxPointer<AnxFndName>));
	header->htNames.PrepareNoSwizzle(data);
	header->evtPrms.RestoreNoSwizzle(data, size);
	AnxGlobal_4byteAlignedSwizzler(header->evtPrms.ptr, header->numEvtPrms*sizeof(AnxPointer<const char>));	
	header->evtPrms.PrepareNoSwizzle(data);
	AnxGlobal_4byteAlignedSwizzler(header, sizeof(AnxHeader));
	//Идентификатор файла
	AnxHeaderId * anxId = (AnxHeaderId *)data;
	anxId->Init();
	return true;
}

