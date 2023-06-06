#pragma once

#include "..\common.h"
#include "..\..\common_h\Mission.h"

class CookBinManager : public MissionObject
{
#pragma pack(push, 1)
	struct DataChunk
	{
		unsigned int chunkSize;
		unsigned int chunkNameSize;
	};
	struct CookHeader
	{
		dword dwAbbr;
		dword dwCookVersion;
	};
#pragma pack(pop)

	ILoadBuffer * loadBuffer;

	array<char>		m_saveBuffer;

	const char* FindChunk(const char* data, unsigned int dataSize, const char* fileName);
	void DelChunk(const char* fileName);
	virtual bool EditMode_Export();

	virtual bool Create(MOPReader& reader);

	char* GetSaveBuffBegin();
	dword GetSaveBuffSize();
	const char* GetLoadBuffBegin();
	dword GetLoadBuffSize();

	__forceinline dword GetCookAbbr() {dword dw = (dword)'C' + (((dword)'o')<<8) + (((dword)'o')<<16) + (((dword)'k')<<24); return dw;}
	__forceinline dword GetCookVersion() {return PX_PHYSICS_VERSION;}
public:

	CookBinManager(void);
	virtual ~CookBinManager(void);

	void LoadCookedMesh(const char* fileName, const void*& data, unsigned int & dataSize);
	void SaveCookedMesh(const char* fileName, const void* data, unsigned int dataSize);

	void WriteAll();

private:
	string & MakePath(string & path);
};

__forceinline char* CookBinManager::GetSaveBuffBegin()
{
	if( m_saveBuffer.Size() < sizeof(CookHeader) )
		return NULL;
	return &m_saveBuffer[sizeof(CookHeader)];
}

__forceinline dword CookBinManager::GetSaveBuffSize()
{
	if( m_saveBuffer.Size() > sizeof(CookHeader) )
		return m_saveBuffer.Size() - sizeof(CookHeader);
	return 0;
}

__forceinline const char* CookBinManager::GetLoadBuffBegin()
{
	if( !loadBuffer ) return NULL;
	if( loadBuffer->Size() <= sizeof(CookHeader) ) return NULL;
	const byte* pB = loadBuffer->Buffer();
	if( !pB ) return NULL;
	return (const char*)&pB[sizeof(CookHeader)];
}

__forceinline dword CookBinManager::GetLoadBuffSize()
{
	if( !loadBuffer ) return 0;
	if( loadBuffer->Size() > sizeof(CookHeader) )
		return loadBuffer->Size() - sizeof(CookHeader);
	return 0;
}
