#include "DataCache.h"
#include "..\..\common_h\particles.h"
#include "..\icommon\memfile.h"

#ifndef _XBOX
	#include "..\..\common_h\tinyxml\tinyxml.h"
#endif

bool ReadingAlreadyComplete;

dword dwGlobalLoadingID = 0;

//Конструктор/деструктор
DataCacheGlobal::DataCacheGlobal () : Cache(_FL_)
{
	pFS = (IFileService*)api->GetService("FileService");
	Assert (pFS != NULL);
}

DataCacheGlobal::~DataCacheGlobal ()
{
	ResetCache ();
}


#ifndef _XBOX
//Положить в кэш данные для системы
void DataCacheGlobal::CacheSystemXML (const char* FileName)
{
	//это для индентификации....
	string NameWithExt = FileName;

	NameWithExt.DeleteLastSymbol('l');
	NameWithExt.DeleteLastSymbol('m');
	NameWithExt.DeleteLastSymbol('x');
	NameWithExt.DeleteLastSymbol('.');

	NameWithExt.AddExtention(".xps");
	NameWithExt.Lower();

	string LongFileName = "resource\\particles\\";
	LongFileName+=FileName;
	LongFileName.AddExtention(".xml");


	CreateDataSourceXML(LongFileName.GetBuffer(), NameWithExt.c_str());
}
#endif


//Положить в кэш данные для системы
void DataCacheGlobal::CacheSystem (const char* FileName)
{
	string NameWithExt = FileName;
	NameWithExt.AddExtention(".xps");
	NameWithExt.Lower();

	string LongFileName = "resource\\particles\\";
	LongFileName+=FileName;
	LongFileName.AddExtention(".xps");


	
	//Ставим файл в очередь на загрузку...
	IDataFile * pFile = pFS->OpenDataFile(LongFileName.GetBuffer(), file_open_any, _FL_);
	if (!pFile)
	{
		api->Trace("Particles: '%s' File not found !!!", LongFileName.GetBuffer());
		return;
	}

	dword filesize = pFile->Size();

	byte* buffer = NEW byte[filesize];

	pFile->Read(buffer, filesize);

	
	string fName;
	fName.GetFileName(LongFileName);

	//Создаем данные из файла...
	CreateDataSource (buffer, filesize, fName.GetBuffer());


	pFile->Release();

	//delete [] buffer;
}

//Сбросить кэш
void DataCacheGlobal::ResetCache ()
{
	for (int n = 0; n < Cache; n++)
	{
		if (Cache[n].pBuffer)
		{
			delete [] Cache[n].pBuffer;
		}
		
		if (Cache[n].pData) Cache[n].pData->Release();
	}

	Cache.DelAll();
}


//Проверить указатель на валидность
bool DataCacheGlobal::ValidatePointer (DataSource* pData)
{
	for (int n = 0; n < Cache; n++)
		if (Cache[n].pData = pData) return true;

	return false;
}

#ifndef _XBOX
void DataCacheGlobal::CreateDataSourceXML (const char* SourceFileName, const char* id_name)
{
	//надо XML оставлять в памяти
	Assert(false);

	LoadedDataSource NewDataSource;
	NewDataSource.FileName = id_name;
	NewDataSource.dwStringHash = string::HashNoCase(NewDataSource.FileName.c_str());
	NewDataSource.pData = NEW DataSource();
	Cache.Add(NewDataSource);

	//Тут открыть файл через TinyXML

	TiXmlDocument particleXML( SourceFileName );
	bool loadOkay = particleXML.LoadFile();

	if (!loadOkay)
	{
		throw ("this is temp gag, sorry !");
	}


	
	NewDataSource.pData->LoadXML(&particleXML);
}
#endif

void DataCacheGlobal::CreateDataSource (void* pBuffer, DWORD BufferSize, const char* SourceFileName)
{
	LoadedDataSource NewDataSource;
	NewDataSource.FileName = SourceFileName;
	NewDataSource.pData = NEW DataSource();
	NewDataSource.pBuffer = pBuffer;
	NewDataSource.dwStringHash = string::HashNoCase(NewDataSource.FileName.c_str());
	Cache.Add(NewDataSource);

  //api->Trace("\nCreate data source for file %s", SourceFileName);

	MemFile* ReadFile = NEW MemFile;
	ReadFile->OpenRead(pBuffer, BufferSize);
	NewDataSource.pData->Load(ReadFile);
	ReadFile->Close();
	delete ReadFile;
}


DWORD DataCacheGlobal::GetCachedCount ()
{
	return Cache.Size();
}

const char* DataCacheGlobal::GetCachedNameByIndex (DWORD Index)
{
	return Cache[Index].FileName;
}

void DataCacheGlobal::NewForceFieldRegistred()
{
	//зарегистрировали новый force field...
	//надо бы обновить кэши 

	for (dword j = 0; j < Cache.Size(); j++)
	{
		Cache[j].pData->NewForceFieldRegistred();
	}

	
}