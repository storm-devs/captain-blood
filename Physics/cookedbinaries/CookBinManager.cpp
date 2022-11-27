#include "CookBinManager.h"
#include "../../common_h/data_swizzle.h"

CookBinManager::CookBinManager(void) : m_saveBuffer(__FILE__, __LINE__)
{
	loadBuffer = null;

	// write header into save buffer
	m_saveBuffer.AddElements( sizeof(CookHeader) );
	CookHeader* pHeader = (CookHeader*)m_saveBuffer.GetBuffer();
	pHeader->dwAbbr = GetCookAbbr();
	pHeader->dwCookVersion = GetCookVersion();
}

CookBinManager::~CookBinManager(void)
{
	if(loadBuffer)
	{
		loadBuffer->Release();
		loadBuffer = null;
	}
}

bool CookBinManager::Create(MOPReader& reader)
{
	string path;
	loadBuffer = Files().LoadData(MakePath(path), _FL_);

	// check header of load buffer
	bool bBufferOK = false;
	if( loadBuffer )
		if( loadBuffer->Size() >= sizeof(CookHeader) )
		{
			CookHeader* pHeader = (CookHeader*)loadBuffer->Buffer();
			if( SwizzleDWord( pHeader->dwAbbr ) != GetCookAbbr() )
			{
				api->Trace("CookBinManager:: Error - incorrect cook binary file abbreviation");
			}
			else if( SwizzleDWord( pHeader->dwCookVersion ) != GetCookVersion() )
			{
				api->Trace("CookBinManager:: Error - incorrect cook binary file version. Required export!");
			}
			else
				bBufferOK = true;
		}
	// incorrect of load buffer is useless, so delete that
	if( !bBufferOK && loadBuffer )
	{
		loadBuffer->Release();
		loadBuffer = null;
	}

	if( EditMode_IsOn() )
		Registry(MG_EXPORT);

	return true;
}

bool CookBinManager::EditMode_Export()
{
	WriteAll();
	return true;
}

void CookBinManager::LoadCookedMesh(const char* fileName, const void*& data, unsigned int & dataSize)
{
	dataSize = 0;
	if(!GetLoadBuffBegin())
	{
		data = null;
		return;
	}	
	data = FindChunk(GetLoadBuffBegin(), GetLoadBuffSize(), fileName);
	if (data)
	{
		dataSize = SwizzleDWord(((DataChunk*)data)->chunkSize) - SwizzleDWord(((DataChunk*)data)->chunkNameSize) - sizeof(DataChunk);
		data = ((const char*)data) + SwizzleDWord(((DataChunk*)data)->chunkNameSize) + sizeof(DataChunk);
	}
}

const char* CookBinManager::FindChunk(const char* data, unsigned int dataSize, const char* fileName)
{
	const char * chunk = data;
	while (chunk - data < (int)dataSize)
	{
		if ( string::IsEqual(chunk+sizeof(DataChunk), fileName) )
			return chunk;

		chunk += SwizzleDWord(((DataChunk*)chunk)->chunkSize);
	}
	return NULL;
}

void CookBinManager::DelChunk(const char* fileName)
{
	if (GetSaveBuffSize()==0)
		return;

	const char * chunk = FindChunk(GetSaveBuffBegin(), GetSaveBuffSize(), fileName);
	if (chunk)
	{
		size_t first = chunk-&m_saveBuffer[0];
		dword chunkSize = SwizzleDWord(((DataChunk*)chunk)->chunkSize);
		size_t last = first + chunkSize - 1;
		// Вернуть/Пофиксить после выставки!
		//m_saveBuffer.DelRange((dword)first, (dword)last);
		// ------------------------
		if( first < last )
		{
			if( last < m_saveBuffer.Last() )
				memcpy(&m_saveBuffer[(long)first],&m_saveBuffer[last+1],m_saveBuffer.Size()-last);
			m_saveBuffer.DelRange( m_saveBuffer.Size() - chunkSize, m_saveBuffer.Last() );
		}
		// ------------------------
	}
}

void CookBinManager::SaveCookedMesh(const char* fileName, const void* data, unsigned int dataSize)
{
	DelChunk(fileName);
	unsigned int chunkNameSize = (unsigned int)strlen(fileName) + 1;
	unsigned int chunkSize = sizeof(DataChunk) + chunkNameSize + dataSize;
	m_saveBuffer.AddElements(chunkSize);

	char* pChunk = &m_saveBuffer[m_saveBuffer.Size() - chunkSize];
	((DataChunk*)pChunk)->chunkSize = chunkSize;
	((DataChunk*)pChunk)->chunkNameSize = chunkNameSize;
	memcpy(pChunk + sizeof(DataChunk), fileName, chunkNameSize);
	pChunk[sizeof(DataChunk) + chunkNameSize - 1] = 0;
	memcpy(pChunk + sizeof(DataChunk) + chunkNameSize, data, dataSize);
}

void CookBinManager::WriteAll()
{
#ifndef _XBOX
	if (m_saveBuffer.Size()==0)
		return;

	string path;
	Files().SaveData(MakePath(path), &m_saveBuffer[0], m_saveBuffer.Size());
	
	// не будем отгружать буфер записи, так как туда добавятся только те паруса,
	// чьи паттерны мы будем менять в редакторе и тогда следующая запись будет неполной.
	//m_saveBuffer.DelAll();
	//m_saveBuffer.DelRange( sizeof(CookHeader), m_saveBuffer.Last() );
#endif
}

string & CookBinManager::MakePath(string & path)
{
	path = "resource\\missions\\";
	path += Mission().GetMissionName();
	path += "\\data\\sails_";
	path += Mission().GetMissionName();
	path += ".pxm";
	return path;
}

MOP_BEGINLISTCG(CookBinManager, "", '1.00', 101, "", "");
MOP_ENDLIST(CookBinManager)