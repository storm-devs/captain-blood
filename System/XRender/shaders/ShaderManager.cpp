#include "ShaderManager.h"
#include "..\Render.h"

#include "Shader.h"
#include "ShaderVar.h"

#include "..\..\..\common_h\FileService.h"

//#define CompiledShadersName		"modules\\Shaders\\CompiledShaders.obj"
#define CompiledShadersName30		"modules\\Win32Shaders30.fxo"
#define CompiledShadersNameX360		"game:\\X360Shaders.fxo"
#define CompiledShadersNameX360_PC		"modules\\X360Shaders.fxo"


#define CompiledShadersName30_system		"modules\\Win32Shaders30_system.fxo"
#define CompiledShadersNameX360_system		"game:\\X360Shaders_system.fxo"
#define CompiledShadersNameX360_PC_system		"modules\\X360Shaders_system.fxo"


ShadersManager* ShadersManager::manager = null;


#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif



#ifndef _XBOX
void ClearReadOnlyAttribute (const char* path)
{
	dword attributes = ::GetFileAttributes(path);
	if(attributes != INVALID_FILE_ATTRIBUTES)
	{
		attributes &= ~FILE_ATTRIBUTE_READONLY;
		attributes &= ~FILE_ATTRIBUTE_SYSTEM;
		::SetFileAttributes(path, attributes);
	}
}
#endif


ShadersManager::ShadersManager() : 
#if !(defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER))
								   variables(_FL_, 128),
								   texture_variables(_FL_, 128),
								   variables_hash(_FL_, 128),
#endif
								   fx_files (_FL_),

								   shadersArrayLin (_FL_),
								   shadersArray (_FL_)


{
	dwPCBtime = 0;
	systemShadersCount = 0;
	d3d = NULL;
	dwSearchTime = 0;
	shadersArray.SetBadFind(NULL);

#if !(defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER))
	varPool.Destroy();
#endif

	manager = this;
}

ShadersManager::~ShadersManager()
{
	Destroy();
}


void ShadersManager::SetD3D(IDirect3DDevice9* device)
{
	d3d = device;
}

void ShadersManager::Destroy()
{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	//ничего не делать...
#else
	varPool.Destroy();
#endif

	for (dword n = 0; n < shadersArrayLin.Size(); n++)
	{
		DELETE (shadersArrayLin[n]);
	}

	shadersArrayLin.DelAll();
}

long ShadersManager::FindInFiles (const char* fileName)
{
	for (dword n = 0; n < fx_files.Size(); n++)
	{
		if (fx_files[n] == fileName)
		{
			return n;
		}
	}

	return -1;
}

void ShadersManager::MoveToUp (const char* fileName)
{
	long idx = FindInFiles(fileName);
	if (idx < 0) return;

	string tmp = fx_files[idx];
	fx_files.DelIndex(idx);

	fx_files.Insert(tmp, 0);

}

void ShadersManager::InsertShaderName (const char* szName, bool bLast)
{
	/*
	string fullName;

	if (!bLast)
	{
		fullName = "modules\\Shaders\\";
	}
	fullName += szName;
	*/

	const char * fullName = szName;

	//api->Trace("Validate shader : '%s'", fullName.c_str());

	if (!bLast)
	{
		if (FindInFiles(fullName) < 0)
		{
			fx_files.Insert(fullName, 0);
		} else
		{
			MoveToUp (fullName);
		}
		return;
	}

	string defaultMacros;
	IFileService * fs = (IFileService *)api->GetService("FileService");

	IDataFile * file = fs->OpenDataFile(fullName, file_open_fromdisk, _FL_);


	//max: ILoadBuffer * loadBuffer = fs->LoadData(fullName, _FL_);

	//max: if (loadBuffer)
	if(file)
	{
		/*
		if (NGRender::pRS->Is30ShadersAvailable() || mode >= SCM_ONLY_COMPILE_PC_SHADERMODEL_3)
		{
			defaultMacros.Format("\r\n#line 0 \"%s\"\r\n#define SHADERS_3_0\r\n#define Technique(a,b) b a; technique a\r\n", fullName);
		} else
		{
			defaultMacros.Format("\r\n#line 0 \"%s\"\r\n#define Technique(a,b) b a; technique a\r\n", fullName);
		}
		*/


		defaultMacros.Format("\r\n#line 0 \"%s\"\r\n#define Technique(a,b) b x__##a; technique a\r\n", fullName);

		
		dword fileSize = file->Size();

		dword newSize = fileSize+defaultMacros.Size();
		char* newBuffer = NEW char [newSize];
		memcpy(newBuffer, defaultMacros.c_str(), defaultMacros.Size());
		//max: memcpy(newBuffer+defaultMacros.Size(), loadBuffer->Buffer(), loadBuffer->Size());
		if(file->Read(newBuffer+defaultMacros.Size(), fileSize) == fileSize)
		{
		



/*
		if (NGRender::pRS->Is30ShadersAvailable() || mode >= SCM_ONLY_COMPILE_PC_SHADERMODEL_3)
		{
			for (dword n = 0; n < newSize-6; n++)
			{
				if (newBuffer[n+0] == 'p' && newBuffer[n+1] == 's' && newBuffer[n+2] == '_' && newBuffer[n+3] == '2' && newBuffer[n+4] == '_' && newBuffer[n+5] == '0')
				{
					newBuffer[n+3] = '3';
					continue;
				}
				if (newBuffer[n+0] == 'v' && newBuffer[n+1] == 's' && newBuffer[n+2] == '_' && newBuffer[n+3] == '2' && newBuffer[n+4] == '_' && newBuffer[n+5] == '0')
				{
					newBuffer[n+3] = '3';
					continue;
				}
			}
		}
*/



			ID3DXEffectCompiler* pEffect = null;
			LPD3DXBUFFER pErrors;

#ifndef _XBOX
			HRESULT res = D3DXCreateEffectCompiler(newBuffer, newSize, NULL, &m_includeHandler, 0, &pEffect, &pErrors);
#else
			D3DXSHADER_COMPILE_PARAMETERS asmParameters;
			memset(&asmParameters, 0, sizeof(asmParameters));
			asmParameters.Flags = D3DXSHADEREX_OPTIMIZE_UCODE | D3DXSHADER_PARTIALPRECISION | D3DXSHADER_PREFER_FLOW_CONTROL /*| D3DXSHADEREX_REMOVE_UNUSED_PS_INPUTS*/;
			asmParameters.PixelShaderSamplerRegisterCount = 8;
			asmParameters.VertexShaderSamplerRegisterCount = 8; 
			HRESULT res = D3DXCreateEffectCompilerEx(newBuffer, newSize, NULL, &m_includeHandler, 0, &pEffect, &pErrors, &asmParameters);
#endif


			if (FindInFiles(fullName) < 0)
			{
				if (bLast)
				{
					fx_files.Add(fullName);
				} else
				{
					fx_files.Insert(fullName, 0);
				}
			} else
			{
				MoveToUp (fullName);
			}

			if (FAILED(res))
			{
				bErrorsWhileValidation = true;

				OutputDebugString((CHAR *)pErrors->GetBufferPointer());
				api->Trace ("'%s' %s\n", fullName, (CHAR *)pErrors->GetBufferPointer());  


#ifndef _XBOX
				MessageBox((HWND)api->Storage().GetLong("system.hwnd"), "Shader validation error !", "Error in validate shader", MB_OK | MB_ICONERROR);
#endif
				Assert(false);

				long idx = FindInFiles(fullName);
				if (idx >= 0)
				{
					fx_files.DelIndex(idx);
				}

				if (pErrors)
				{
					OutputDebugString((CHAR *)pErrors->GetBufferPointer());
					api->Trace ("Validate error: '%s' %s\n", fullName, (CHAR *)pErrors->GetBufferPointer());  
				}

				RELEASE(file);
				DELETE(newBuffer);

				return;
			}


			RELEASE(pErrors);
			RELEASE(pEffect);
			RELEASE(file);
			DELETE(newBuffer);		
			return;
		}
		RELEASE(file);
		DELETE(newBuffer);		
	}
	api->Trace("Validate Can't load FX shader '%s'", szName);
}

void ShadersManager::Compile (const char * dir, const char * objectFile)
{
	shadersRootDir = dir;
	shadersRootDir += "\\";

	bErrorsWhileValidation = false;

	dword dwStartTime = GetTickCount();

	m_includeHandler.FakeMode (false);
	fx_files.DelAll();

	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);


	
	//Нет ли уже откомпилированных шейдеров ?
	array<byte> loadBuffer(_FL_);
	IDataFile * file = fs->OpenDataFile(objectFile, file_open_fromdisk, _FL_);
	if(file)
	{
		loadBuffer.AddElements(file->Size());
		if(file->Read(loadBuffer.GetBuffer(), loadBuffer.Size()) != loadBuffer.Size())
		{
			loadBuffer.Empty();
		}
		RELEASE(file);
	}
	if(loadBuffer.Size() > 0)
	//max: ILoadBuffer * loadBuffer = fs->LoadData(objectFile, _FL_);
	//max: if (loadBuffer)
	{
		api->Trace("Use prebuilded shaders file '%s'", objectFile);
		CreateShader((char *)loadBuffer.GetBuffer(), loadBuffer.Size(), "Generated.obj", false, "anyname.obj");

		Assert(shadersArrayLin.Size() > 0);
		//max: RELEASE(loadBuffer);

		dword dwEndTime = GetTickCount();
		api->Trace("Shaders loading time : %.2f sec. (%d shaders, %.2f sec. per shader)", ((dwEndTime-dwStartTime)/1000.0f), shadersArrayLin.Size(), ((dwEndTime-dwStartTime)/1000.0f) / (float)shadersArrayLin.Size());

		return;
	} 



	//Надо компилировать
	//------------------------------------------------------------------
	IFinder * finder = NULL;

	dword dwTotalSize = 0;
	char* bigBuffer = NULL;
	dword dwFileIndex = 0;
	string defaultMacros;

	finder = fs->CreateFinder(dir, "*.fx", find_all_files_no_mirrors | find_no_files_from_packs, _FL_);
	Assert(finder);

#ifdef _XBOX
	int shadersCount = 0;
	string tmp;
#endif

	string mess;
	for(dword i = 0; i < finder->Count(); i++)
	{
		const char* shaderPath = finder->FilePath(i);
/*
		NGRender::pRS->ShowStartupScreen(2000005, "Validate :", finder->Name(i));
*/

#ifdef _XBOX
		shadersCount++;
/*
		NGRender::pRS->ShowStartupScreen(2000005, "Validate :", shaderPath);
*/

#endif

		InsertShaderName(shaderPath, true);
	}

	RELEASE(finder);

	
#ifdef _XBOX
	tmp.Format("X360: Shaders preprocessed '%d'\n", shadersCount);
	OutputDebugString(tmp.c_str());
#endif


	//Никаких шейдеров не нашли - выход...
	if (fx_files.Size() <= 0)
	{
		return;
	}
	

	//---------------------------
	for (dword n = 0; n < fx_files.Size(); n++)
	{
		//max: ILoadBuffer * loadBuffer = fs->LoadData(fx_files[n].c_str(), _FL_);
		IDataFile * file = fs->OpenDataFile(fx_files[n].c_str(), file_open_fromdisk, _FL_);
		if(file)
		{
			loadBuffer.Empty();
			loadBuffer.AddElements(file->Size());
			if(file->Read(loadBuffer.GetBuffer(), loadBuffer.Size()) != loadBuffer.Size())
			{
				loadBuffer.Empty();
			}
			RELEASE(file);
		}		
		if (loadBuffer.Size() > 0)
		{
			//api->Trace("FX: %s", fx_files[n].c_str());

			if (dwFileIndex == 0)
			{
#ifdef _XBOX
				defaultMacros.Format("\r\n#line 0 \"%s\"\r\n#define SHADERS_3_0\r\n#define _XBOX360\r\n#define Technique(a,b) b x__##a; technique a\r\n", fx_files[n].c_str());
#else
				defaultMacros.Format("\r\n#line 0 \"%s\"\r\n#define SHADERS_3_0\r\n#define Technique(a,b) b x__##a; technique a\r\n", fx_files[n].c_str());
#endif
			} else
			{
				defaultMacros.Format("\r\n#line 0 \"%s\"\r\n#define FX_FILE_%d\r\n", fx_files[n].c_str(), dwFileIndex);
			}

			dword dwOffset = dwTotalSize;
			dwTotalSize += defaultMacros.Size();
			dwTotalSize += loadBuffer.Size();
			bigBuffer = (char*)resize(bigBuffer, dwTotalSize, _FL_);
			memcpy(bigBuffer+dwOffset, defaultMacros.c_str(), defaultMacros.Size());
			memcpy(bigBuffer+dwOffset+defaultMacros.Size(), loadBuffer.GetBuffer(), loadBuffer.Size());
			//max: RELEASE(loadBuffer);
			loadBuffer.Empty();
			dwFileIndex++;
		}
	}



	//--------------
	for (dword n = 0; n < dwTotalSize-6; n++)
	{
		if (bigBuffer[n+0] == 'p' && bigBuffer[n+1] == 's' && bigBuffer[n+2] == '_' && bigBuffer[n+3] == '2' && bigBuffer[n+4] == '_' && bigBuffer[n+5] == '0')
		{
			bigBuffer[n+3] = '3';
			continue;
		}
		if (bigBuffer[n+0] == 'v' && bigBuffer[n+1] == 's' && bigBuffer[n+2] == '_' && bigBuffer[n+3] == '2' && bigBuffer[n+4] == '_' && bigBuffer[n+5] == '0')
		{
			bigBuffer[n+3] = '3';
			continue;
		}

		if (bigBuffer[n+0] == '#' && bigBuffer[n+1] == 'i' && bigBuffer[n+2] == 'n' && bigBuffer[n+3] == 'c' && bigBuffer[n+4] == 'l' && bigBuffer[n+5] == 'u')
		{
			bigBuffer[n+0] = '/';
			bigBuffer[n+1] = '/';
		}

	}


	//--------------

	LPD3DXBUFFER pPreprocessErrors = NULL;
	LPD3DXBUFFER pPreprocessedShaderForSave = NULL;
	HRESULT preprocessRes = D3DXPreprocessShader(bigBuffer, dwTotalSize, NULL, NULL, &pPreprocessedShaderForSave, &pPreprocessErrors);

	if (pPreprocessErrors)
	{
		OutputDebugString((const char * )pPreprocessErrors->GetBufferPointer());
		pPreprocessErrors->Release();
		pPreprocessErrors = NULL;
	}

	Assert(preprocessRes == D3D_OK && pPreprocessedShaderForSave != NULL && pPreprocessErrors == NULL);

	void * ShaderTextPtr = pPreprocessedShaderForSave->GetBufferPointer();
	DWORD ShaderTextSize = pPreprocessedShaderForSave->GetBufferSize();

	string debugfileName = objectFile;
#ifndef STOP_DEBUG
#ifdef _XBOX
	debugfileName += ".x360.fx";	
#else
	debugfileName += ".win32.fx";
#endif
	{
		fs->BuildPath(debugfileName.c_str(), debugfileName);
		FILE * fl = null;
		fopen_s(&fl, debugfileName.c_str(), "w+b");
		if(fl)
		{
			fwrite(ShaderTextPtr, ShaderTextSize, 1, fl);
			fclose(fl);
		}
	}
#endif
	pPreprocessedShaderForSave->Release();
	pPreprocessedShaderForSave = NULL;

/*
	NGRender::pRS->ShowStartupScreen(2000006, "Generate FXO file");
*/
	
	api->Trace("Generate FXO file '%s'.", objectFile);
	CreateShader(bigBuffer, dwTotalSize, "Generated.fx", true, objectFile);

/*
	NGRender::pRS->ShowStartupScreen(2000007, "FXO build - done. Game now exit.");
*/

	DELETE(bigBuffer);



	dword dwEndTime = GetTickCount();
	api->Trace("Shader compile time : %.2f min.", ((dwEndTime-dwStartTime)/1000.0f) / 60.0f);

	
/*
	if (!bSystem)
	{
		NGRender::pRS->DumpAllMemoryUsageToLog();

		//после компиляции выходим из игры...

		for (int i = 0; i < 10; i++)
		{
			NGRender::pRS->ShowStartupScreen(2000007, "FXO build - done. Game now exit.");
			Sleep(50);
		}

		//Аццкий хак
		exit(0);
	}
*/	
}

const char* ShadersManager::GetShadersRootDir ()
{
	return shadersRootDir.c_str();
}

/*
void ShadersManager::BuildSystemShaders ()
{
	OutputDebugString("Start build system shaders...\n");

	const char* compiledDir = "modules\\SystemShaders";

#ifndef _XBOX
	const char* compiledFileName = CompiledShadersName30_system;
#else
	const char* compiledFileName = CompiledShadersNameX360_system;
#endif

	Compile(compiledDir, compiledFileName, true);


	systemShadersCount = shadersArrayLin.Size();


	OutputDebugString("System shaders builded...\n");
}
*/

bool ShadersManager::IsNeedBuild()
{
#ifndef STOP_DEBUG
#ifndef _XBOX
	const char* compiledFileName = CompiledShadersName30;
#else
	const char* compiledFileName = CompiledShadersNameX360;
#endif
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	string path;
	fs->BuildPath(compiledFileName, path);
	FILE * fl = null;
	fopen_s(&fl, path.c_str(), "rb");
	if(fl)
	{
		fclose(fl);
	}
	return !fl;
#else
	return false;
#endif
}

void ShadersManager::Build ()
{
	OutputDebugString("Start build game shaders...\n");

	const char* compiledDir = "modules\\Shaders";
	
#ifndef _XBOX
	const char* compiledFileName = CompiledShadersName30;
#else
	const char* compiledFileName = CompiledShadersNameX360;
#endif

	Compile(compiledDir, compiledFileName);
	OutputDebugString("Game shaders builded...\n");
}



void ShadersManager::CreateShader (char* buffer, dword size, const char* szFileName, bool bWriteObjFile, const char* objFileName)
{
//	NGRender::pRS->LockResourceCreation("ShaderManager::CreateShader");


	m_includeHandler.FakeMode (true);

#ifndef _XBOX
	dword flags = D3DXSHADER_PREFER_FLOW_CONTROL/* | D3DXSHADER_SKIPOPTIMIZATION*/;
#else
	dword flags = D3DXSHADER_PREFER_FLOW_CONTROL | D3DXSHADER_MICROCODE_BACKEND_NEW;
	
#endif

	char* FXObjData = buffer;
	dword FXObjSize = size;

	IFileService * fs = (IFileService *)api->GetService("FileService");
	if (bWriteObjFile)
	{
		ID3DXEffectCompiler* FXCompiler = null;
		LPD3DXBUFFER pParseErrors;
		
#ifndef _XBOX
		D3DXCreateEffectCompiler(buffer, size, NULL, &m_includeHandler, flags, &FXCompiler, &pParseErrors);
#else
		D3DXSHADER_COMPILE_PARAMETERS asmParameters;
		memset(&asmParameters, 0, sizeof(asmParameters));
		asmParameters.Flags = D3DXSHADEREX_OPTIMIZE_UCODE | D3DXSHADEREX_REMOVE_UNUSED_PS_INPUTS;
		D3DXCreateEffectCompilerEx(buffer, size, NULL, &m_includeHandler, flags, &FXCompiler, &pParseErrors, &asmParameters);
#endif

		if(pParseErrors)
		{
			OutputDebugString((CHAR *)pParseErrors->GetBufferPointer());
			api->Trace ("FX Parser error '%s'\n", (CHAR *)pParseErrors->GetBufferPointer());  
			pParseErrors->Release();
			pParseErrors = NULL;
		}
			
		if (!FXCompiler)
		{
			//NGRender::pRS->UnLockResourceCreation("ShaderManager::CreateShader");
			return;
		}


		LPD3DXBUFFER pFXObject;
		LPD3DXBUFFER pCompileErrors;
		HRESULT compileRES = FXCompiler->CompileEffect(flags, &pFXObject, &pCompileErrors);
		if (FAILED(compileRES))
		{
			OutputDebugString((CHAR *)pCompileErrors->GetBufferPointer());
			api->Trace ("FX Compilation error '%s'\n", (CHAR *)pCompileErrors->GetBufferPointer());  
			RELEASE(FXCompiler);
			//NGRender::pRS->UnLockResourceCreation("ShaderManager::CreateShader");
			return;
		}

		if (pCompileErrors)
		{
			OutputDebugString((CHAR *)pCompileErrors->GetBufferPointer());
			api->Trace ("FX Compilation error '%s'\n", (CHAR *)pCompileErrors->GetBufferPointer());  
			pCompileErrors->Release();
			pCompileErrors = NULL;
		}
		

		if (!bErrorsWhileValidation)
		{
#ifndef _XBOX
			ClearReadOnlyAttribute(objFileName);
#endif
#ifndef STOP_DEBUG
			FILE * fl = null;
			fopen_s(&fl, objFileName, "w+b");
			if(fl)
			{
				fwrite(pFXObject->GetBufferPointer(), pFXObject->GetBufferSize(), 1, fl);
				fclose(fl);
			}
#endif
		}


		FXObjData = (char*)pFXObject->GetBufferPointer();
		FXObjSize = pFXObject->GetBufferSize();

		RELEASE(FXCompiler);
	}


	NGRender::pRS->LockResourceCreation("D3DXCreateEffect");


	ID3DXEffect* pEffect = NULL;
	LPD3DXBUFFER pErrors = NULL;
	HRESULT res = D3DXCreateEffect(d3d, FXObjData, FXObjSize, NULL, &m_includeHandler, /*D3DXFX_NOT_CLONEABLE |*/ flags, NULL, &pEffect, &pErrors);

	NGRender::pRS->UnLockResourceCreation("D3DXCreateEffect");


	if (FAILED(res))
	{
		if (pErrors)
		{
			OutputDebugString((CHAR *)pErrors->GetBufferPointer());
			api->Trace ("%s\n", (CHAR *)pErrors->GetBufferPointer());  
		}
		//NGRender::pRS->UnLockResourceCreation("ShaderManager::CreateShader");
		return;
	}

	if (pErrors)
	{
		OutputDebugString((CHAR *)pErrors->GetBufferPointer());
		api->Trace ("%s\n", (CHAR *)pErrors->GetBufferPointer());  
		pErrors->Release();
		pErrors = NULL;
	}


	api->Trace ("Shader compilation done...");

	CreateVaribles(pEffect);


	MEMORYSTATUS pMemStatusBufferBefore;
	GlobalMemoryStatus(&pMemStatusBufferBefore);


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	array<Shader*> tempShaderArray(_FL_, 256);
	texFinder.Init(pEffect, NGRender::pRS->D3D());
#endif

	D3DXHANDLE hTechnique;
	D3DXHANDLE hOld;
	HRESULT findRes = pEffect->FindNextValidTechnique (NULL, &hTechnique);
	hOld = hTechnique;

	for (;;)
	{
		D3DXTECHNIQUE_DESC tDesc;
		HRESULT getdescRes = pEffect->GetTechniqueDesc(hTechnique, &tDesc);
		Assert(getdescRes == D3D_OK);

		D3DXHANDLE tech_handle = pEffect->GetTechniqueByName(tDesc.Name);


		Shader* new_shader = NEW Shader(pEffect, tech_handle);

		new_shader->SetShaderName(tDesc.Name);

		if (!new_shader->CreateDeclaration())
		{
			api->Trace("Technique '%s' error in declaration !!", tDesc.Name);
			DELETE(new_shader);
		} else
		{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
			tempShaderArray.Add(new_shader);

			precompiledBatch* newBatch = NEW precompiledBatch(tDesc.Name);
			shadersArray.Add(tDesc.Name, newBatch);
			shadersArrayLin.Add(newBatch);
			
#else
			shadersArray.Add(tDesc.Name, new_shader);
			shadersArrayLin.Add(new_shader);
#endif
		}

		HRESULT findnextRes = pEffect->FindNextValidTechnique (hOld, &hTechnique);
		if (findnextRes != D3D_OK)
		{
			break;
		}

		hOld = hTechnique;
	} 

	pEffect->Release ();


	MEMORYSTATUS pMemStatusBufferAfter;
	GlobalMemoryStatus(&pMemStatusBufferAfter);


	DWORD dwAvailPhysBefore = pMemStatusBufferBefore.dwAvailPhys;
	DWORD dwAvailPhysAfter = pMemStatusBufferAfter.dwAvailPhys;

	DWORD dwExecutedUsageMemory = dwAvailPhysBefore - dwAvailPhysAfter;

	static char ttt[256];
	crt_snprintf(ttt, 256, "\n\nFX Shaders used : %3.2fMb memory, Shaders count [%d]\n", dwExecutedUsageMemory / 1024.0f / 1024.0f, shadersArrayLin.Size());
	OutputDebugString(ttt);


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	DWORD dwPCBStart = GetTickCount();


	GlobalMemoryStatus(&pMemStatusBufferBefore);


	for (dword n = 0; n < tempShaderArray.Size(); n++)
	{
		tempShaderArray[n]->setTextureFinder(&texFinder);
	}

	D3DDevice* pCommandBufferDevice = NULL;
	Direct3D_CreateDevice( 0, D3DDEVTYPE_COMMAND_BUFFER, NULL, 0, NULL, &pCommandBufferDevice );




	for (dword n = 0; n < tempShaderArray.Size(); n++)
	{
		NGRender::pRS->LockResourceCreation("BuildChangeList");
		tempShaderArray[n]->BuildChangeList(pCommandBufferDevice, &varsDatabase, shadersArrayLin[n]);
		NGRender::pRS->UnLockResourceCreation("BuildChangeList");
	}


	pCommandBufferDevice->Release();
	pCommandBufferDevice = NULL;

	for (dword n = 0; n < tempShaderArray.Size(); n++)
	{
		tempShaderArray[n]->setTextureFinder(NULL);
	}
	texFinder.Release();


	DWORD dwAllocatedBytesForCB = 0;
	for (dword n = 0; n < shadersArrayLin.Size(); n++)
	{
		batchRunner.calculateMaxSize(shadersArrayLin[n]);
		shadersArrayLin[n]->CleanupTempData();
		dwAllocatedBytesForCB += shadersArrayLin[n]->getBytesAllocated();
	}


	pMemStatusBufferAfter;
	GlobalMemoryStatus(&pMemStatusBufferAfter);

	dwAvailPhysBefore = pMemStatusBufferBefore.dwAvailPhys;
	dwAvailPhysAfter = pMemStatusBufferAfter.dwAvailPhys;

	dwExecutedUsageMemory = dwAvailPhysBefore - dwAvailPhysAfter;

	crt_snprintf(ttt, 256, "\n\nCommand buffer Shaders used : (%3.2fMb) %3.2fMb memory, Shaders count [%d], %3.2fMb per shader\n", dwAllocatedBytesForCB  / 1024.0f / 1024.0f, dwExecutedUsageMemory / 1024.0f / 1024.0f, shadersArrayLin.Size(), dwExecutedUsageMemory / 1024.0f / 1024.0f / (float)shadersArrayLin.Size());
	OutputDebugString(ttt);



	//Надо удалить все shader
	//-------------------------------------
	NGRender::pRS->LockResourceCreation("Delete FX");
	for (dword n = 0; n < tempShaderArray.Size(); n++)
	{
		DELETE (tempShaderArray[n]);
	}
	NGRender::pRS->UnLockResourceCreation("Delete FX");
	tempShaderArray.DelAll();

	DWORD dwPCBEnd = GetTickCount();

	dwPCBtime = dwPCBEnd - dwPCBStart;


#else


	for (dword n = 0; n < shadersArrayLin.Size(); n++)
	{
		shadersArrayLin[n]->BuildChangeList();
	}

	for (dword n = 0; n < shadersArrayLin.Size(); n++)
	{
		shadersArrayLin[n]->AttachStandartVariables();
	}

#endif




	
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	GlobalMemoryStatus(&pMemStatusBufferBefore);


	batchRunner.create();

	GlobalMemoryStatus(&pMemStatusBufferAfter);


	dwAvailPhysBefore = pMemStatusBufferBefore.dwAvailPhys;
	dwAvailPhysAfter = pMemStatusBufferAfter.dwAvailPhys;

	dwExecutedUsageMemory = dwAvailPhysBefore - dwAvailPhysAfter;

	crt_snprintf(ttt, 256, "\n\nCommand executor cache used : %3.2fMb memory\n", dwExecutedUsageMemory / 1024.0f / 1024.0f);
	OutputDebugString(ttt);

	int a = 0;


	


#endif

	fx_files.DelAll();
		
	//NGRender::pRS->UnLockResourceCreation("ShaderManager::CreateShader");

}



void ShadersManager::CreateVaribles (ID3DXEffect* pEffect)
{
	MEMORYSTATUS pMemStatusBufferBefore;
	GlobalMemoryStatus(&pMemStatusBufferBefore);


	D3DXEFFECT_DESC fxDesc;
	pEffect->GetDesc(&fxDesc);
	D3DXPARAMETER_DESC paramDesc;
	for (UINT i = 0; i < fxDesc.Parameters; i++)
	{
		D3DXHANDLE paramHandle = pEffect->GetParameter(NULL, i);
		pEffect->GetParameterDesc(paramHandle, &paramDesc);

		if (paramDesc.Class == D3DXPC_SCALAR && paramDesc.Type == D3DXPT_BOOL)
		{
			if (paramDesc.Elements != 32)
			{
				api->Trace("ERROR : '%s' Bool registers must sets by chunks of 32 elements!!!", paramDesc.Name);
				Assert(false);
			}
		}

		const char* szVariableName = paramDesc.Name;

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
		varsDatabase.AddVariable (szVariableName, pEffect, paramHandle);
#else
		ShaderVariable* var = varPool.Allocate();
		var->Init(pEffect, paramHandle, szVariableName);

		if (paramDesc.Class == D3DXPC_OBJECT && paramDesc.Type == D3DXPT_TEXTURE && paramDesc.Elements == 0) 
		{
			texture_variables.Add(var);
		}
		
		variables.Add(var);
		variables_hash.Add(szVariableName, var);
#endif


	}

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#ifndef STOP_DEBUG
	varsDatabase.Debug_MakeItConst();
#endif


	varsDatabase.Init();
#endif



	MEMORYSTATUS pMemStatusBufferAfter;
	GlobalMemoryStatus(&pMemStatusBufferAfter);

	DWORD dwAvailPhysBefore = pMemStatusBufferBefore.dwAvailPhys;
	DWORD dwAvailPhysAfter = pMemStatusBufferAfter.dwAvailPhys;

	DWORD dwExecutedUsageMemory = dwAvailPhysBefore - dwAvailPhysAfter;

	static char ttt[256];
	crt_snprintf(ttt, 256, "\n\nVariables used : %3.2fMb memory\n", dwExecutedUsageMemory / 1024.0f / 1024.0f);
	OutputDebugString(ttt);


}





void ShadersManager::StartFrame()
{
	dwSearchTime = 0;
}

dword ShadersManager::GetTimeForSearch()
{
	return dwSearchTime;
}

/*
Shader* ShadersManager::FindShaderByName (const char* shaderName)
{
}
*/

IVariable* ShadersManager::getShaderVariable (const char* shaderName, const char * srcFile, long srcLine)
{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	IVariable * var = varsDatabase.getShaderVariable(shaderName, srcFile, srcLine);
	if (var == NULL)
	{
		api->Trace("Variable '%s' not found", shaderName);
		OutputDebugString("!!!! Problem variable : ");
		OutputDebugString(shaderName);
		OutputDebugString("\n");
	}
	Assert(var)
	return var;

#else

	ShaderVariable* var = NULL;
	variables_hash.Find(shaderName, var);
	if (var == NULL)
	{
		api->Trace("Variable '%s' not found", shaderName);
		OutputDebugString("!!!! Problem variable : ");
		OutputDebugString(shaderName);
		OutputDebugString("\n");

	}
	Assert(var)


	return var;

#endif

}


void ShadersManager::OnLostDevice()
{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	//На боксе ничего делать не надо
#else
	for (dword n = 0; n < shadersArrayLin.Size(); n++)
	{
		shadersArrayLin[n]->OnLostDevice();
	}
#endif
}

void ShadersManager::OnResetDevice()
{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	//На боксе ничего делать не надо
#else
	for (dword n = 0; n < shadersArrayLin.Size(); n++)
	{
		shadersArrayLin[n]->OnResetDevice();
	}
#endif
}



void ShadersManager::ResetAllTextureVariables()
{
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	varsDatabase.ResetAllTextureVariables();
#else
	for (DWORD i = 0; i < texture_variables.Size(); i++)
	{
		texture_variables[i]->ResetTexture();
	}
#endif
}