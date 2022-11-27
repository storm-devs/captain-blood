
#include "IncludeHandler.h"
#include "ShaderManager.h"
#include "..\..\..\common_h\FileService.h"





#ifdef _XBOX
HRESULT RenderIncludeHandler::Open(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes, LPSTR pFullPath, DWORD cbFullPath)
#else
HRESULT RenderIncludeHandler::Open(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
#endif
{

#ifdef _XBOX
	string t;
	//t.Format("X360: Include %s\n", pFileName);
	//OutputDebugString(t.c_str());
#endif


	if (m_bFake)
	{
		return D3D_OK;
	}

	
	//"modules\\Shaders\\"
	string fullPath = ShadersManager::manager->GetShadersRootDir();
	fullPath += pFileName;
	

	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);

	IDataFile * file = fs->OpenDataFile(fullPath.c_str(), file_open_fromdisk, _FL_);
	//max: ILoadBuffer * loadBuffer = fs->LoadData(fullPath.c_str(), _FL_);

	if (!file)
	{
#ifdef _XBOX
		t.Format("X360: Can't Include %s\n", fullPath.c_str());
		OutputDebugString(t.c_str());
#endif
		return D3DXERR_INVALIDDATA;
	} 

	dword fileSize = file->Size();
	void * pBuffer = NEW byte[fileSize];
	*ppData = pBuffer;
	*pBytes = fileSize;
	if(file->Read(pBuffer, fileSize) != fileSize)
	{
#ifdef _XBOX
		t.Format("X360: Can't Include (io error. can't read from file) %s\n", fullPath.c_str());
		OutputDebugString(t.c_str());
#endif
		return D3DXERR_INVALIDDATA;
	}
	//max: memcpy(pBuffer, loadBuffer->Buffer(), loadBuffer->Size());
	//max: RELEASE(loadBuffer);
	RELEASE(file);


	fs->BuildPath(fullPath, fullPath);
	ShadersManager::manager->InsertShaderName(fullPath, false);

	//api->Trace("WARNING: Ignoring #include directive");

#ifdef _XBOX
	cbFullPath = 0;
	pFullPath = NULL;
#endif

	return S_OK; 
}

HRESULT RenderIncludeHandler::Close(THIS_ LPCVOID pData)
{

	DELETE(pData);

	return S_OK; 
}



RenderIncludeHandler::RenderIncludeHandler()
{
	m_bFake = false;
}

RenderIncludeHandler::~RenderIncludeHandler()
{

}


void RenderIncludeHandler::FakeMode (bool bFake)
{
	m_bFake = bFake;
}



