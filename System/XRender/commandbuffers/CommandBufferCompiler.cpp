#include "..\..\..\common_h\render.h"

#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#include "CommandBufferCompiler.h"
#include "..\..\..\common_h\FileService.h"
#include "..\DefaultStates\DefaultStates.h"
#include "..\Render.h"
#include "..\batch\PrecompiledBatch.h"



CommandBufferCompiler::CommandBufferCompiler()
												 
{
	dwUsedSpaceInCmdBuffer = 0;
	commandBuffer = NULL;

	pFS = (IFileService *)api->GetService("FileService");

	dwHeaderSize = 0;
	dwPhysicalSize = 0;
	dwInitializationSize = 0;


}

CommandBufferCompiler::~CommandBufferCompiler()
{
	if (commandBuffer)
	{
		commandBuffer->Release();
		commandBuffer = NULL;
	}

	if (shaderStates.pBoundedVS)
	{
		shaderStates.pBoundedVS->Release();
	}
	shaderStates.pBoundedVS = NULL;

	if (shaderStates.pBoundedPS)
	{
		shaderStates.pBoundedPS->Release();
	}
	shaderStates.pBoundedPS = NULL;

}


void DebugOut (const char * str, ...)
{
	static char tempBuffer[16384];

	va_list args;
	va_start(args, str);
	vsnprintf_s(tempBuffer, 16384 - 4, 16384-4, str, args);
	va_end(args);


	OutputDebugString(tempBuffer);
}



void CommandBufferCompiler::SetName (const char* szTechName)
{
	techName = szTechName;
}




void CommandBufferCompiler::ExtractRenderState (D3DRENDERSTATETYPE state, DWORD dwVal, bool bLog)
{
	if (bLog)
	{
		DebugOut("ExtractRenderState(%d, %d)\n", state, dwVal);
	}
	
	renderState & item = shaderStates.r_states[shaderStates.r_states.Add()];
	item.state = state;
	item.val = dwVal;
}

void CommandBufferCompiler::ExtractSamplerState (DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD dwVal)
{
	if ((type != D3DSAMP_ADDRESSU) &&
	    (type != D3DSAMP_ADDRESSV) &&
	    (type != D3DSAMP_MAGFILTER) &&
	    (type != D3DSAMP_MINFILTER ) &&
	    (type != D3DSAMP_MIPFILTER ))
	{
		return;
	}



	dword dwIdx = INVALID_ARRAY_INDEX;
	for (dword i = 0; i < shaderStates.textures.Size(); i++)
	{
		if (shaderStates.textures[i].dwStage == sampler)
		{
			dwIdx = i;
			break;
		}
	}

	if (dwIdx == INVALID_ARRAY_INDEX)
	{
		dwIdx = shaderStates.textures.Add();
	}

	texture & tex = shaderStates.textures[dwIdx];
	tex.dwStage = sampler;

	

	switch (type)
	{
	case D3DSAMP_ADDRESSU:
		tex.clampU = (D3DTEXTUREADDRESS )dwVal;
		break;
	case D3DSAMP_ADDRESSV:
		tex.clampV = (D3DTEXTUREADDRESS )dwVal;
		break;
	case D3DSAMP_MAGFILTER:
		tex.magFilter = (D3DTEXTUREFILTERTYPE)dwVal;
		break;
	case D3DSAMP_MINFILTER:
		tex.minFilter = (D3DTEXTUREFILTERTYPE)dwVal;
		break;
	case D3DSAMP_MIPFILTER:
		tex.mipFilter = (D3DTEXTUREFILTERTYPE)dwVal;
		break;
	}
}

void CommandBufferCompiler::PatchShadersAndDumpMicrocode(IDirect3DVertexDeclaration9 * pVDecl, array<DWORD> & vertexDeclStrides)
{
	Assert(shaderStates.pBoundedVS);

	shaderStates.pBoundedVS->Bind(0, pVDecl, &vertexDeclStrides[0], shaderStates.pBoundedPS);

	bool bBoundedVS = shaderStates.pBoundedVS->IsBound();
	Assert(bBoundedVS);


	if (shaderStates.pVertexShaderFunc)
	{
		ID3DXBuffer* microcodeTextVS = NULL;
		D3DXDisassembleShader(shaderStates.pVertexShaderFunc, FALSE, NULL, &microcodeTextVS);
		if (microcodeTextVS)
		{
			const char * szStr = (const char*)microcodeTextVS->GetBufferPointer();

			ParsePredefinedConstants (szStr, shaderStates.vs_consts, false);

			microcodeTextVS->Release();
			microcodeTextVS = NULL;
		}
	}

	if (shaderStates.pPixelShaderFunc)
	{
		ID3DXBuffer* microcodeTextPS = NULL;
		D3DXDisassembleShader(shaderStates.pPixelShaderFunc, FALSE, NULL, &microcodeTextPS);
		if (microcodeTextPS)
		{
			const char * szStr = (const char*)microcodeTextPS->GetBufferPointer();

			ParsePredefinedConstants (szStr, shaderStates.ps_consts, true);

			microcodeTextPS->Release();
			microcodeTextPS = NULL;
		}
	}


	ExtractMicrocode(shaderStates.pPixelShaderFunc, CommandBufferCompiler::MT_PIXEL_SHADER);
	ExtractMicrocode(shaderStates.pVertexShaderFunc, CommandBufferCompiler::MT_VERTEX_SHADER);
}

void CommandBufferCompiler::ExtractPixelShader(CONST DWORD *pShaderFunction)
{
	Assert(shaderStates.pBoundedPS == NULL);

	if (pShaderFunction == NULL)
	{
		return;
	}

	NGRender::pRS->D3D()->CreatePixelShader(pShaderFunction, &shaderStates.pBoundedPS);
	shaderStates.pPixelShaderFunc = pShaderFunction;
}

void CommandBufferCompiler::ExtractVertexShader(CONST DWORD *pShaderFunction)
{
	Assert(shaderStates.pBoundedVS == NULL);

	if (pShaderFunction == NULL)
	{
		return;
	}

	NGRender::pRS->D3D()->CreateVertexShader(pShaderFunction, &shaderStates.pBoundedVS);
	shaderStates.pVertexShaderFunc = pShaderFunction;
}

bool CommandBufferCompiler::ExtractConstants (CONST DWORD *pShaderFunction, MicrocodeType type)
{
	UINT constSize;
	D3DXCONSTANTTABLE_DESC constTableDesc;

	ID3DXConstantTable* pAttributes = NULL;
	D3DXGetShaderConstantTable(pShaderFunction, &pAttributes);

	if (pAttributes == NULL)
	{
		printf ("ERROR!!! - not constant table found\n");
		return false;
	}

//	pass & p = shaderStates;

	array<shaderConstant> * consts = &shaderStates.vs_consts;

	if (type == CommandBufferCompiler::MT_PIXEL_SHADER)
	{
		consts = &shaderStates.ps_consts;
	}

	

	bool bNeedWarning = true;
	pAttributes->GetDesc(&constTableDesc);
	for (UINT idx = 0; idx < constTableDesc.Constants; idx++)
	{
		bNeedWarning = true;
		D3DXCONSTANT_DESC constDesc;;

		D3DXHANDLE v_cont = pAttributes->GetConstant(NULL, idx);
		pAttributes->GetConstantDesc(v_cont, &constDesc, &constSize);

		if (constDesc.RegisterSet == D3DXRS_FLOAT4)
		{
			if (constDesc.Class == D3DXPC_MATRIX_COLUMNS && constDesc.Type == D3DXPT_FLOAT && constDesc.Rows == 4 && constDesc.Columns == 4)
			{
				bNeedWarning = false;
				consts->Add(shaderConstant(constDesc.Name, CT_MATRIX4x4, constDesc.RegisterIndex, constDesc.RegisterCount, constDesc.Elements));
			}

			if (constDesc.Class == D3DXPC_VECTOR && constDesc.Type == D3DXPT_FLOAT && constDesc.Rows == 1 && constDesc.Columns == 4)
			{
				bNeedWarning = false;
				consts->Add(shaderConstant(constDesc.Name, CT_FLOAT4, constDesc.RegisterIndex, constDesc.RegisterCount, constDesc.Elements));
			}

			if (constDesc.Class == D3DXPC_VECTOR && constDesc.Type == D3DXPT_FLOAT && constDesc.Rows == 1 && constDesc.Columns == 3)
			{
				bNeedWarning = false;
				consts->Add(shaderConstant(constDesc.Name, CT_FLOAT3, constDesc.RegisterIndex, constDesc.RegisterCount, constDesc.Elements));
			}

			if (constDesc.Class == D3DXPC_SCALAR && constDesc.Type == D3DXPT_FLOAT && constDesc.Rows == 1 && constDesc.Columns == 1)
			{
				bNeedWarning = false;
				consts->Add(shaderConstant(constDesc.Name, CT_FLOAT, constDesc.RegisterIndex, constDesc.RegisterCount, constDesc.Elements));
			}
		}

		if (constDesc.RegisterSet == D3DXRS_BOOL)
		{
			if (constDesc.Class == D3DXPC_SCALAR && constDesc.Type == D3DXPT_BOOL && constDesc.Rows == 1 && constDesc.Columns == 1)
			{
/*
				if (constDesc.RegisterCount != 32)
				{
					//bool ставятся не пачками по 32 - ошибка!!!
					api->Trace("ERROR : '%s' Bool registers must sets by chunks of 32 elements!!!", constDesc.Name);
					Assert(false);
				}
*/

				if ((constDesc.RegisterIndex % 32) != 0)
				{
					//регистры bool начинаются не кратно 32 - ошибка!!!
					api->Trace("ERROR : '%s' Bool registers address must be 32 bools aligned!!!", constDesc.Name);
					Assert(false);
				}

				

				bNeedWarning = false;
				consts->Add(shaderConstant(constDesc.Name, CT_BOOL, constDesc.RegisterIndex, constDesc.RegisterCount, constDesc.Elements));
			}
		}

		if (constDesc.RegisterSet == D3DXRS_SAMPLER)
		{
			//just skip, we already grab sampler description...
			bNeedWarning = false;
		}


		if (bNeedWarning)
		{
			printf ("WARNING! : skip constant with name : '%s' - unsupported type for backend!\n", constDesc.Name);

		}
	} //constTableDesc.Constants


	return true;
}


void CommandBufferCompiler::ExtractMicrocode(CONST DWORD *pShaderFunction, MicrocodeType type)
{
	if (pShaderFunction == NULL)
	{
		return;
	}

	/*
	ID3DXBuffer* microcodeText = NULL;
	D3DXDisassembleShaderEx(passDesc.pPixelShaderFunction,
	D3DXDISASSEMBLER_SHOW_TIMING_ESTIMATE | 
	D3DXDISASSEMBLER_SHOW_RUNTIME_DATA |
	D3DXDISASSEMBLER_SHOW_MICROCODE_DEFAULTS |
	D3DXDISASSEMBLER_SHOW_CONSTANT_TABLE_DEFAULTS,
	NULL,
	&microcodeText);

	if (microcodeText)
	{
		IFileService * fs = (IFileService *)api->GetService("FileService");


		static char tmp[MAX_PATH];
		crt_snprintf(tmp, MAX_PATH, "devkit:\\ps_%s_pass_%d.mc", tDesc.Name, n);

		IFile* file = fs->OpenFile(tmp, file_create_always, _FL_);

		if (file)
		{
			file->Write(microcodeText->GetBufferPointer(), microcodeText->GetBufferSize());
			file->Release();
			file = NULL;
			microcodeText->Release();
			microcodeText = NULL;
		}
	}


*/
	//------------------------------------------------------------------


	ExtractConstants(pShaderFunction, type);


	switch (type)
	{
	case CommandBufferCompiler::MT_PIXEL_SHADER:
		{
			microcode & mc = shaderStates.microcodePS;
			XGGetMicrocodeShaderParts( pShaderFunction, &mc.parts );
			IDirect3DPixelShader9* shaderBody = (IDirect3DPixelShader9*)mc.Allocate(mc.parts.cbCachedPartSize);
			XGSetPixelShaderHeader( shaderBody, mc.parts.cbCachedPartSize, &mc.parts );
			break;
		}
	case CommandBufferCompiler::MT_VERTEX_SHADER:
		{
			microcode & mc = shaderStates.microcodeVS;
			XGGetMicrocodeShaderParts( pShaderFunction, &mc.parts );
			IDirect3DVertexShader9* shaderBody = (IDirect3DVertexShader9*)mc.Allocate(mc.parts.cbCachedPartSize);
			XGSetVertexShaderHeader( shaderBody, mc.parts.cbCachedPartSize, &mc.parts );
			break;
		}
	default:
		Assert(false && "Unknown microcode type!");
	}


	


/*	
	IFile* file = pFS->OpenFile(fileName, file_create_always, _FL_);
	if (file)
	{
		//Надо еще хидер тут...
		file->Write(pShaderBody, parts.cbCachedPartSize);
		file->Write(parts.pPhysicalPart, parts.cbPhysicalPartSize);
		file->Release();
		file = NULL;
	}
*/

}


void CommandBufferCompiler::ResetToDefaultStates(D3DDevice* pCommandBufferDevice)
{
	/////////////////////////////////////////////////////////////////////////////

	DWORD dwValue = 0;
	for (dword n = 0; n < RENDER_STATES_ARRAY_SIZE; n++)
	{
		if (DefaultRenderStates::GetDefaultValue((D3DRENDERSTATETYPE)n, dwValue))
		{
			pCommandBufferDevice->SetRenderState((D3DRENDERSTATETYPE)n, dwValue);
		}
	}

	pCommandBufferDevice->SetVertexShader(NULL);
	pCommandBufferDevice->SetPixelShader(NULL);
	pCommandBufferDevice->SetVertexDeclaration(NULL);
	pCommandBufferDevice->SetStreamSource(0, NULL, 0, 0);
	pCommandBufferDevice->SetStreamSource(1, NULL, 0, 0);
	pCommandBufferDevice->SetStreamSource(2, NULL, 0, 0);
	//pCommandBufferDevice->SetStreamSource(3, NULL, 0, 0);
	pCommandBufferDevice->SetIndices(NULL);



	/////////////////////////////////////////////////////////////////////////////

}

void CommandBufferCompiler::ParsePredefinedConstants (const char* szShaderBody, array<shaderConstant> & shaderConsts, bool bPixelShader)
{
	static char digit[32];

	int stringLen = (int)strlen (szShaderBody);

	for (int i = 0; i < (stringLen-6); i++)
	{
		if (szShaderBody[i+0] == 'd' && szShaderBody[i+1] == 'e' && szShaderBody[i+2] == 'f' && szShaderBody[i+3] == ' ' && szShaderBody[i+4] == 'c')
		{
			int remainChars = (stringLen - i);
			if (remainChars > 3)
			{
				remainChars = 3;
			}


			//сканим и как только не число все...
			for (DWORD counter = 0; counter < remainChars; counter++)
			{
				char mustBeDigit = szShaderBody[i + 5 + counter];

				if (!(mustBeDigit >= '0' && mustBeDigit <= '9'))
				{
					break;
				}

				digit[counter] = mustBeDigit;
			}

			digit[counter] = 0;
			int predefinedRegIndex = atoi(digit);

			assert (predefinedRegIndex <= 255);

			shaderConstant & shConst = shaderConsts[shaderConsts.Add()];
			shConst.dwRegisterIndex = predefinedRegIndex;
			shConst.dwElements = 1;
			shConst.dwRegisterCounts = 1;
			shConst.type = CT_FLOAT4;
			shConst.szName.Format("c_defined_%d", predefinedRegIndex);
			shConst.bPredefinedInShaderBody = true;


			//дальше ставим смещение...
			i = i + 5 + counter;
		}

		if (szShaderBody[i+0] == 'd' && szShaderBody[i+1] == 'e' && szShaderBody[i+2] == 'f' && szShaderBody[i+3] == 'i' && szShaderBody[i+4] == ' ' && szShaderBody[i+5] == 'i')
		{
			int remainChars = (stringLen - i);
			if (remainChars > 2)
			{
				remainChars = 2;
			}


			//сканим и как только не число все...
			for (int counter = 0; counter < remainChars; counter++)
			{
				char mustBeDigit = szShaderBody[i + 6 + counter];

				if (!(mustBeDigit >= '0' && mustBeDigit <= '9'))
				{
					break;
				}

				digit[counter] = mustBeDigit;
			}

			digit[counter] = 0;
			int predefinedRegIndex = atoi(digit);

			/*
			Type								HLSL Constant Range						Microcode Constant Range 
			Vertex Shader Floating-Point		c0–c255									c0–c255 
			Pixel Shader Floating-Point			c0–c255									c0–c255 
			Vertex Shader Boolean				b0–b127									b0–b127 
			Pixel Shader Boolean				b0–b127									b128–b255 
			Vertex Shader Integer				i0–i15									i0–i15 
			Pixel Shader Integer				i0–i15									i16–i31 
			Vertex Shader Fetch/Sampler			s0–s3									tf16–tf19 
			Pixel Shader Fetch/Sampler			s0–s15									tf0–tf15 
			*/
			if (bPixelShader)
			{
				predefinedRegIndex -= 16;
			}

			assert (predefinedRegIndex <= 15);



			shaderConstant & shConst = shaderConsts[shaderConsts.Add()];
			shConst.dwRegisterIndex = predefinedRegIndex;
			shConst.dwElements = 1;
			shConst.dwRegisterCounts = 1;
			shConst.type = CT_INT4;
			shConst.szName.Format("i_defined_%d", predefinedRegIndex);
			shConst.bPredefinedInShaderBody = true;


			//дальше ставим смещение...
			i = i + 6 + counter;
		}


	}
}

void CommandBufferCompiler::Finalize(D3DDevice* pCommandBufferDevice, precompiledBatch * pBatch)
{
	ResetToDefaultStates(pCommandBufferDevice);

	IBaseTexture* fakeTextureStorm = NGRender::pRS->getWhiteTexture();
	IDirect3DBaseTexture9 * fakeTexture = (IDirect3DBaseTexture9 *)fakeTextureStorm->GetBaseTexture();


	D3DVertexBuffer * fakeVb = NULL;
	pCommandBufferDevice->CreateVertexBuffer(sizeof(Vector) * 4, 0, 0, 0, &fakeVb, NULL);

	D3DIndexBuffer * fakeIb = NULL;
	pCommandBufferDevice->CreateIndexBuffer(sizeof(WORD) * 6, 0, D3DFMT_INDEX16, 0, &fakeIb, 0);




	//JOKER TODO:
	//надо тут подготовить кусочек 'command buffer' и шейдерки отдельно все (кластеризовать шейдеры ???? ускорит закрузку)

	//JOKER TODO:
	//все сохдранять в формате, что бы можно было inplace поднять


	//JOKER TODO:
	//Декларации больше не нужны - т.к. шейдеры Binded уже...

/*
	Zenable 1
	CullMode 6
	AlphaBlendEnable 0
	AlphaTestEnable 0
	AlphaRef 0
	StencilMask = 0xffffffff
	StencilWriteMask = 0xffffffff
	MultiSampleMask = 0xffffffff 
*/



	//-----------------------------------------------------------------------------------------------------------------------------------------
	//
	// создаем precompiled command buffer
	//
	//-----------------------------------------------------------------------------------------------------------------------------------------

	if (commandBuffer != NULL)
	{
		//Уже запекали данный буффер - второй раз может придти, т.к. сначала systemShaders, потом обычные...
		return;
	}

	Assert(commandBuffer == NULL && "twice call !?");

/*
	D3DDevice* pCommandBufferDevice = NULL;
	Direct3D_CreateDevice( 0, D3DDEVTYPE_COMMAND_BUFFER, NULL, 0, NULL, &pCommandBufferDevice );
	pCommandBufferDevice->Release();
	pCommandBufferDevice = NULL;
*/

	pCommandBufferDevice->CreateCommandBuffer( 256 * 1024, 0, &commandBuffer );

/*
	OutputDebugString(techName.c_str());
	OutputDebugString("\n");
*/

	bool bLog = false;

/*
	if (techName == "Particles" != NULL)
	{
		DebugOut("shader : '%s' -------------------------------------------------\n", techName.c_str());
		bLog = true;
	}
*/

	
/*
	if (techName == "Create_fBm")
	{
		int a = 0;
	}
*/


	//for (dword passIdx = 0; passIdx < passes.Size(); passIdx++)
	//{
		//pass & p = shaderStates;


		//ResetToDefaultStates(pCommandBufferDevice);

		D3DTAGCOLLECTION InheritTags;

		//Отмечаем, что нам ничего не нужно...
		D3DTagCollection_SetAll(&InheritTags);

		//Нужны vertex/pixel shaderы
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_VERTEXSHADER), D3DTag_Mask(D3DTAG_VERTEXSHADER));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_PIXELSHADER), D3DTag_Mask(D3DTAG_PIXELSHADER));


		//Шейдеры эти пакеты генерируют
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_HIZENABLE), D3DTag_Mask(D3DTAG_HIZENABLE));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_HICONTROL), D3DTag_Mask(D3DTAG_HICONTROL));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_INTERPOLATORCONTROL), D3DTag_Mask(D3DTAG_INTERPOLATORCONTROL));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_PROGRAMCONTROL), D3DTag_Mask(D3DTAG_PROGRAMCONTROL));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_CONTEXTMISC), D3DTag_Mask(D3DTAG_CONTEXTMISC));


		//D3DRS_CULLMODE
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_MODECONTROL), D3DTag_Mask(D3DTAG_MODECONTROL));

		//ZBuffer
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_DEPTHCONTROL), D3DTag_Mask(D3DTAG_DEPTHCONTROL));


		//alpha blend
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_BLENDCONTROL0), D3DTag_Mask(D3DTAG_BLENDCONTROL0));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_BLENDCONTROL1), D3DTag_Mask(D3DTAG_BLENDCONTROL1));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_BLENDCONTROL2), D3DTag_Mask(D3DTAG_BLENDCONTROL2));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_BLENDCONTROL3), D3DTag_Mask(D3DTAG_BLENDCONTROL3));

		//ColorWriteEnable
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_COLORMASK), D3DTag_Mask(D3DTAG_COLORMASK));

		//alpha test
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_COLORCONTROL), D3DTag_Mask(D3DTAG_COLORCONTROL));
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_ZPASSEDRAMMODE), D3DTag_Mask(D3DTAG_ZPASSEDRAMMODE));
		
		//alpha ref
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_ALPHAREF), D3DTag_Mask(D3DTAG_ALPHAREF));

		//double speed depth render или normal render
		//в тенях используется
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_EDRAMMODECONTROL), D3DTag_Mask(D3DTAG_EDRAMMODECONTROL));

		for (dword texIdx = 0; texIdx < shaderStates.textures.Size(); texIdx++)
		{
			texture & tex = shaderStates.textures[texIdx];

			//sampler #tex.dwStage
			DWORD gpuFetchConstantTex = GPU_CONVERT_D3D_TO_HARDWARE_TEXTUREFETCHCONSTANT(tex.dwStage);
			UINT64 textureConstantMask = D3DTAG_MASKENCODE(D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantTex, D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantTex);
			D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FETCHCONSTANTS), textureConstantMask);
		}


		//vertex stream #0  (1 и 2-й тоже автоматом будут)
		DWORD vertexFetchConstant = GPU_CONVERT_D3D_TO_HARDWARE_VERTEXFETCHCONSTANT(0);
		DWORD gpuFetchConstantVrx = (vertexFetchConstant * (65536/3 + 1)) >> 16;
		UINT64 vertexConstantMask = D3DTAG_MASKENCODE(D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx, D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx);
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FETCHCONSTANTS), vertexConstantMask);



/*
		//vertex stream #1
		vertexFetchConstant = GPU_CONVERT_D3D_TO_HARDWARE_VERTEXFETCHCONSTANT(1);
		gpuFetchConstantVrx = (vertexFetchConstant * (65536/3 + 1)) >> 16;
		vertexConstantMask = D3DTAG_MASKENCODE(D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx, D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx);
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FETCHCONSTANTS), vertexConstantMask);

		//vertex stream #2
		vertexFetchConstant = GPU_CONVERT_D3D_TO_HARDWARE_VERTEXFETCHCONSTANT(2);
		gpuFetchConstantVrx = (vertexFetchConstant * (65536/3 + 1)) >> 16;
		vertexConstantMask = D3DTAG_MASKENCODE(D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx, D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx);
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FETCHCONSTANTS), vertexConstantMask);

		//vertex stream #3
		vertexFetchConstant = GPU_CONVERT_D3D_TO_HARDWARE_VERTEXFETCHCONSTANT(3);
		gpuFetchConstantVrx = (vertexFetchConstant * (65536/3 + 1)) >> 16;
		vertexConstantMask = D3DTAG_MASKENCODE(D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx, D3DTAG_START(D3DTAG_FETCHCONSTANTS) + gpuFetchConstantVrx);
		D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FETCHCONSTANTS), vertexConstantMask);
*/



		//vertex constants
		for (dword vconstIdx = 0; vconstIdx < shaderStates.vs_consts.Size(); vconstIdx++)
		{
			shaderConstant & vconst = shaderStates.vs_consts[vconstIdx];

			assert ((vconst.dwRegisterIndex + vconst.dwRegisterCounts-1) <= 255);

			if (vconst.bPredefinedInShaderBody)
			{
				precompiledBatch::Trace("vertex shader const #%d predefined in shader body\n", vconst.dwRegisterIndex);
			}


			if (vconst.type == CommandBufferCompiler::CT_BOOL || vconst.type == CommandBufferCompiler::CT_INT4)
			{
				D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FLOWCONSTANTS), D3DTag_Mask(D3DTAG_FLOWCONSTANTS));
				//D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FLOWCONSTANTS), D3DTag_ShaderConstantMask(vconst.dwRegisterIndex, vconst.dwRegisterCounts));
			} else
			{
				D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_VERTEXSHADERCONSTANTS), D3DTag_ShaderConstantMask(vconst.dwRegisterIndex, vconst.dwRegisterCounts));
			}

		}

		//pixel constants
		for (dword pconstIdx = 0; pconstIdx < shaderStates.ps_consts.Size(); pconstIdx++)
		{
			shaderConstant & pconst = shaderStates.ps_consts[pconstIdx];

			assert ((pconst.dwRegisterIndex + pconst.dwRegisterCounts-1) <= 255);

			if (pconst.bPredefinedInShaderBody)
			{
				precompiledBatch::Trace("pixel shader const #%d predefined in shader body\n", pconst.dwRegisterIndex);
			}


			if (pconst.type == CommandBufferCompiler::CT_BOOL || pconst.type == CommandBufferCompiler::CT_INT4)
			{
				D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FLOWCONSTANTS), D3DTag_Mask(D3DTAG_FLOWCONSTANTS));
				//D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_FLOWCONSTANTS), D3DTag_ShaderConstantMask(pconst.dwRegisterIndex, pconst.dwRegisterCounts));
			} else
			{
				D3DTagCollection_Clear(&InheritTags, D3DTag_Index(D3DTAG_PIXELSHADERCONSTANTS), D3DTag_ShaderConstantMask(pconst.dwRegisterIndex, pconst.dwRegisterCounts));
			}


		}


//
		
	


		pCommandBufferDevice->BeginCommandBuffer( commandBuffer, /*D3DBEGINCB_OVERWRITE_INHERITED_STATE | */D3DBEGINCB_RECORD_ALL_SET_STATE, &InheritTags, NULL, NULL, 0 );


		pBatch->DynamicDraw_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
		pBatch->DynamicDraw_SetRenderState(D3DRS_ZENABLE, TRUE);
		pBatch->DynamicDraw_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		pBatch->DynamicDraw_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		pBatch->DynamicDraw_SetRenderState(D3DRS_STENCILENABLE, FALSE);
		pBatch->DynamicDraw_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		pBatch->DynamicDraw_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		pBatch->DynamicDraw_SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
		pBatch->DynamicDraw_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pBatch->DynamicDraw_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pBatch->DynamicDraw_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		
		pBatch->DynamicDraw_SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		pBatch->DynamicDraw_SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
		


		for (dword t = 0; t < 4; t++)
		{
			pBatch->DynamicDraw_SetSamplerState(t, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			pBatch->DynamicDraw_SetSamplerState(t, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			pBatch->DynamicDraw_SetSamplerState(t, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			pBatch->DynamicDraw_SetSamplerState(t, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			pBatch->DynamicDraw_SetSamplerState(t, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		}




		for (dword texIdx = 0; texIdx < shaderStates.textures.Size(); texIdx++)
		{
			texture & tx = shaderStates.textures[texIdx];
			pCommandBufferDevice->SetTexture(tx.dwStage, fakeTexture);
	
		}


		for (dword tIdx = 0; tIdx < shaderStates.textures.Size(); tIdx++)
		{
			texture & t = shaderStates.textures[tIdx];
			pBatch->DynamicDraw_SetSamplerState(t.dwStage, D3DSAMP_ADDRESSU, t.clampU);
			pBatch->DynamicDraw_SetSamplerState(t.dwStage, D3DSAMP_ADDRESSV, t.clampV);
			pBatch->DynamicDraw_SetSamplerState(t.dwStage, D3DSAMP_MINFILTER, t.minFilter);
			pBatch->DynamicDraw_SetSamplerState(t.dwStage, D3DSAMP_MAGFILTER, t.magFilter);
			pBatch->DynamicDraw_SetSamplerState(t.dwStage, D3DSAMP_MIPFILTER, t.mipFilter);
		}


		for (dword vconstIdx = 0; vconstIdx < shaderStates.vs_consts.Size(); vconstIdx++)
		{
			shaderConstant & vconst = shaderStates.vs_consts[vconstIdx];

			if (vconst.bPredefinedInShaderBody)
			{
				continue;
			}

			//api->Trace("v: %s\n", vconst.szName.c_str());

			float tmp[4];
			tmp[0] = 1.0f;
			tmp[1] = 2.0f;
			tmp[2] = 3.0f;
			tmp[3] = 4.0f;

			BOOL myB;
			myB = false;

			int myI[4];
			myI[0] = 13;
			myI[1] = 14;
			myI[2] = 15;
			myI[3] = 16;


			switch (vconst.type)
			{
			case CommandBufferCompiler::CT_BOOL:
				pCommandBufferDevice->SetVertexShaderConstantB(vconst.dwRegisterIndex, &myB, 1);
				break;
			case CommandBufferCompiler::CT_INT4:
				pCommandBufferDevice->SetVertexShaderConstantI(vconst.dwRegisterIndex, &myI[0], 1);
				break;
			default:
				pCommandBufferDevice->SetVertexShaderConstantF(vconst.dwRegisterIndex, tmp, 1);
			}
		}

		for (dword pconstIdx = 0; pconstIdx < shaderStates.ps_consts.Size(); pconstIdx++)
		{
			shaderConstant & pconst = shaderStates.ps_consts[pconstIdx];

			if (pconst.bPredefinedInShaderBody)
			{
				continue;
			}


			//api->Trace("p: %s\n", pconst.szName.c_str());

			float tmp[4];
			tmp[0] = 1.0f;
			tmp[1] = 2.0f;
			tmp[2] = 3.0f;
			tmp[3] = 4.0f;
			
			BOOL myB;
			myB = false;

			int myI[4];
			myI[0] = 13;
			myI[1] = 14;
			myI[2] = 15;
			myI[3] = 16;

			switch (pconst.type)
			{
			case CommandBufferCompiler::CT_BOOL:
				pCommandBufferDevice->SetPixelShaderConstantB(pconst.dwRegisterIndex, &myB, 1);
				break;
			case CommandBufferCompiler::CT_INT4:
				pCommandBufferDevice->SetPixelShaderConstantI(pconst.dwRegisterIndex, &myI[0], 1);
				break;
			default:
				pCommandBufferDevice->SetPixelShaderConstantF(pconst.dwRegisterIndex, tmp, 1);
			}

		}



		pCommandBufferDevice->SetVertexShader(shaderStates.pBoundedVS);
		pCommandBufferDevice->SetPixelShader(shaderStates.pBoundedPS);

		pBatch->DynamicDraw_SetVertexShader(shaderStates.pBoundedVS);
		pBatch->DynamicDraw_SetPixelShader(shaderStates.pBoundedPS);


		//because we have bound vertex shader...
		pCommandBufferDevice->SetVertexDeclaration(NULL);

		pCommandBufferDevice->SetIndices(fakeIb);
		pCommandBufferDevice->SetStreamSource(0, fakeVb, 0, 0);
		pCommandBufferDevice->SetStreamSource(1, fakeVb, 0, 0);
		pCommandBufferDevice->SetStreamSource(2, fakeVb, 0, 0);
		//pCommandBufferDevice->SetStreamSource(3, fakeVb, 0, 0);

		DWORD dwColorMask = D3DCOLORWRITEENABLE_ALL;
		DWORD dwDestBlend = D3DBLEND_INVSRCALPHA;
		DWORD dwSrcBlend = D3DBLEND_SRCALPHA;
		DWORD dwZWriteEnable = TRUE;
		DWORD dwAlphaTest = FALSE;
		DWORD dwAlphaBlend = FALSE;
		DWORD dwDepthFunc = D3DCMP_LESSEQUAL;

		BOOL dwZEnable = TRUE;


		pCommandBufferDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
		pCommandBufferDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		pCommandBufferDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		pCommandBufferDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		pCommandBufferDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		pCommandBufferDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		pCommandBufferDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		pCommandBufferDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);



		for (dword stateIdx = 0; stateIdx < shaderStates.r_states.Size(); stateIdx++)
		{
			renderState & rs = shaderStates.r_states[stateIdx];

			pCommandBufferDevice->SetRenderState(rs.state, rs.val);
			pBatch->DynamicDraw_SetRenderState(rs.state, rs.val);

			if (rs.state == D3DRS_ALPHATESTENABLE)
			{
				dwAlphaTest = rs.val;
			}

			if (rs.state == D3DRS_ZWRITEENABLE)
			{
				dwZWriteEnable = rs.val;
			}
		

			if (rs.state == D3DRS_ALPHABLENDENABLE)
			{
				dwAlphaBlend = rs.val;
			}


			if (rs.state == D3DRS_DESTBLEND)
			{
				dwDestBlend = rs.val;
			}

			if (rs.state == D3DRS_SRCBLEND)
			{
				dwSrcBlend = rs.val;
			}

			if (rs.state == D3DRS_ZFUNC)
			{
				dwDepthFunc = rs.val;
			}
			if (rs.state == D3DRS_ZENABLE)
			{
				dwZEnable = rs.val;
			}

			if (rs.state == D3DRS_COLORWRITEENABLE)
			{
				dwColorMask = rs.val;
			}


			if (bLog)
			{
				DebugOut("SetRenderState (%d, %d)\n", rs.state, rs.val);
			}
		}




		pCommandBufferDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, 0, 2);


		pCommandBufferDevice->EndCommandBuffer();



		pBatch->offlineCreate(commandBuffer, shaderStates.pBoundedVS, shaderStates.pBoundedPS, bLog);

		if (bLog)
		{
			int a = 0;
		}

		pBatch->SetColorMask (dwColorMask);

		pBatch->SetZEnable(dwZEnable, dwZWriteEnable);
	

		if (dwDepthFunc == D3DCMP_LESSEQUAL)
		{
			pBatch->SetHyperZEnable(dwZEnable, dwZWriteEnable);
		}
		else if (dwDepthFunc == D3DCMP_ALWAYS)
		{
			pBatch->SetHyperZEnable(false, dwZWriteEnable);
		} else
		{
			pBatch->SetHyperZEnable(false, false);
		}

		if (dwAlphaBlend == FALSE)
		{
			dwSrcBlend = D3DBLEND_ONE;
			dwDestBlend = D3DBLEND_ZERO;
		}

		pBatch->SetBlendOp((GPUBLEND)dwSrcBlend, (GPUBLEND)dwDestBlend);
		pBatch->SetVertexShaderPtr(shaderStates.pBoundedVS);
		pBatch->SetPixelShaderPtr(shaderStates.pBoundedPS);
		pBatch->Freeze();


		pBatch->debugOutCommandBuffer(pBatch->getPhysicalBytes(), bLog);

		

		dwHeaderSize = 0;
		dwPhysicalSize = 0;
		dwInitializationSize = 0;
	//}






	
	//for (dword passIdx = 0; passIdx < passes.Size(); passIdx++)
	//{
		//;

		for (DWORD tmp  = 0; tmp < shaderStates.vs_consts.Size(); tmp++)
		{
			shaderConstant & sc = shaderStates.vs_consts[tmp];
			if (sc.bPredefinedInShaderBody)
			{
				continue;
			}

			switch (sc.type)
			{
			case CommandBufferCompiler::CT_BOOL:
				pBatch->AddVertexShaderConstBool(sc.szName, sc.dwRegisterIndex, sc.dwRegisterCounts);
				break;
			case CommandBufferCompiler::CT_INT4:
				Assert(false);
				break;
			default:
				pBatch->AddVertexShaderConstVector(sc.szName, sc.dwRegisterIndex, sc.dwRegisterCounts);
			}
		}

		for (DWORD tmp  = 0; tmp < shaderStates.ps_consts.Size(); tmp++)
		{
			shaderConstant & sc = shaderStates.ps_consts[tmp];
			if (sc.bPredefinedInShaderBody)
			{
				continue;
			}

			switch (sc.type)
			{
			case CommandBufferCompiler::CT_BOOL:
				pBatch->AddPixelShaderConstBool(sc.szName, sc.dwRegisterIndex, sc.dwRegisterCounts);
				break;
			case CommandBufferCompiler::CT_INT4:
				Assert(false);
				break;
			default:
				pBatch->AddPixelShaderConstVector(sc.szName, sc.dwRegisterIndex, sc.dwRegisterCounts);
			}

			
		}


		for (DWORD tmp  = 0; tmp < shaderStates.textures.Size(); tmp++)
		{
			texture & tx = shaderStates.textures[tmp];
			pBatch->AddTextureSampler(tx.variableName, tx.dwStage, tx.minFilter, tx.magFilter, tx.mipFilter, tx.clampU, tx.clampV);
		}

	//}

	pBatch->InsertPrefetchFiller();

	commandBuffer->Release();
	commandBuffer = NULL;


	fakeIb->Release();
	fakeIb = NULL;

	fakeVb->Release();
	fakeVb = NULL;
}

void CommandBufferCompiler::ExtractTexture (DWORD dwIndex, const char* texvariableName)
{
	dword dwIdx = INVALID_ARRAY_INDEX;
	for (dword i = 0; i < shaderStates.textures.Size(); i++)
	{
		if (shaderStates.textures[i].dwStage == dwIndex)
		{
			dwIdx = i;
			break;
		}
	}

	if (dwIdx == INVALID_ARRAY_INDEX)
	{
		dwIdx = shaderStates.textures.Add();
	}

	texture & tex = shaderStates.textures[dwIdx];
	tex.dwStage = dwIndex;
	tex.variableName = texvariableName;
}




#endif

