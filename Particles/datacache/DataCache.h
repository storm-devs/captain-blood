#ifndef _DATA_CACHE_
#define _DATA_CACHE_

#include "..\system\datasource\datasource.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\common_h\templates.h"

class IParticleManager;

class DataCacheGlobal : public Object
{
	string NameWithExt;

	IFileService* pFS;


	struct LoadedDataSource
	{
		dword dwStringHash;
		string FileName;
		DataSource* pData;
		void* pBuffer;

		LoadedDataSource ()
		{
			dwStringHash = 0x0;
			pBuffer = NULL;
			pData = NULL;
		}
	};
	
	array<LoadedDataSource> Cache;
	

	void CreateDataSource (void* pBuffer, DWORD BufferSize, const char* SourceFileName);

#ifndef _XBOX
	void CreateDataSourceXML (const char* SourceFileName, const char* id_name);
#endif




public:

//Конструктор/деструктор
	DataCacheGlobal ();
	~DataCacheGlobal ();

//Положить в кэш данные для системы
	void CacheSystem (const char* FileName);

#ifndef _XBOX
	//Положить в кэш данные для системы
	void CacheSystemXML (const char* FileName);
#endif
	

//Сбросить кэш
	void ResetCache ();


	//Получить указатель на данные для системы партиклов
	__forceinline DataSource* GetParticleSystemDataSource (const char* FileName)
	{
		NameWithExt = FileName;
		NameWithExt.AddExtention(".xps");

		dword dwSearchHash = string::HashNoCase(NameWithExt);

		for (int n = 0; n < Cache; n++)
		{
			if (Cache[n].dwStringHash == dwSearchHash)
			{
				if (Cache[n].FileName == NameWithExt)
				{
					return Cache[n].pData;
				}
			}
		}

		return NULL;
	}


//Проверить указатель на валидность
	bool ValidatePointer (DataSource* pData);


	DWORD GetCachedCount ();
	const char* GetCachedNameByIndex (DWORD Index);

	void NewForceFieldRegistred();


};


#endif