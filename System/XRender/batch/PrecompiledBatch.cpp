#include "..\..\..\common_h\render.h"

#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#include "PrecompiledBatch.h"
#include "..\VariablesDB.h"
#include "..\Render.h"


#define DUMP_FILE_NAME "game:\\dump.txt"

bool bGlobalDump = false;

precompiledBatch::precompiledBatch(const char * szShdrName) : vs_consts_v4 (_FL_),
                                                              ps_consts_v4 (_FL_),

									                          ps_consts_b (_FL_),
									                          vs_consts_b (_FL_),

									                          tex_consts (_FL_),

									                          dynamicDrawRS(_FL_, 8),
									                          dynamicDrawSS(_FL_, 8)
{
	originalColorMask = 0x0F;
	originalGpuCull = GPUCULL_NONE_FRONTFACE_CCW;
	originalZEnable = 0;
	originalHyperZEnable = 0;


	bDepthEnable = false;
	bDepthWriteEnable = false;


	bHiZEnable = false;
	bHiZWriteEnable = false;
	bNeedFlushHiperZ = true;
	shaderName = szShdrName;
	pWhiteTexture = (IDirect3DBaseTexture9*)NGRender::pRS->getWhiteTexture()->GetBaseTexture();

	dwBytesAllocated = 0;

	sourcePrecompiledBuffer = NULL;

	m_pVertexShader = NULL;
	m_pPixelShader = NULL;

	m_pDB = NULL;
	bTemp_VertexShaderNotSet = true;
	bTemp_PixelShaderNotSet = true;

	commandBuffer_temp = NULL;

	bLogOnlyWhenParse = false;

	baseData = 0;
	
	dwParser_Offset = 0;

	dwIndxOffset_Offset = GPU_INVALID_OFFSET;
	dwDepthControl_Offset = GPU_INVALID_OFFSET;
	dwColorMask_Offset = GPU_INVALID_OFFSET;
	dwBlendControl0_Offset = GPU_INVALID_OFFSET;
	dwBlendControl1_Offset = GPU_INVALID_OFFSET;
	dwBlendControl2_Offset = GPU_INVALID_OFFSET;
	dwBlendControl3_Offset = GPU_INVALID_OFFSET;
	dwHiZControl_Offset = GPU_INVALID_OFFSET;


	dwSetCullMode_Offset = GPU_INVALID_OFFSET;
	dwSetVertexShader_Offset_ = GPU_INVALID_OFFSET;
	dwSetPixelShader_Offset_ = GPU_INVALID_OFFSET;
	dwDrawToken_Offset = GPU_INVALID_OFFSET;
	dwEndCommandBuffer_Offset = GPU_INVALID_OFFSET;
	dwSizeInDwords = 0;
	dwCurrentSizeInDwords = 0;

	for (DWORD i = 0; i < MAX_STREAMS_COUNT; i++)
	{
		dwStreamSource_Offsets[i] = GPU_INVALID_OFFSET;
	}
	
	for (DWORD i = 0; i < MAX_TEXTURES_COUNT; i++)
	{
		dwTexture_Offsets[i] = GPU_INVALID_OFFSET;
	}

	for (DWORD i = 0; i < MAX_ALU_CONST_COUNT; i++)
	{
		dwALUConst_Offsets[i] = GPU_INVALID_OFFSET;
	}

	for (DWORD i = 0; i < MAX_ALU_CONST_COUNT_BOOL; i++)
	{
		dwALUConstBool_Offsets[i] = GPU_INVALID_OFFSET;
	}


	dwCommandBufferDataSize = 0;
	dwCommandBufferData = NULL;

	dwHeaderSize_temp = 0;
	pHeaderBytes_temp = NULL;

	dwInitializationSize_temp = 0;
	pInitializationBytes_temp = NULL;

	dipPacketHeader.CountMinusOne = (sizeof(GPUCOMMAND_DRAW_INDEX)/sizeof(DWORD)) - 1;
	dipPacketHeader.OpCode = GPUCOMMANDOP_DRAW;
	dipPacketHeader.PacketType = GPU_PACKET_TYPE_3; 

	dipPacket.VizQueryId = 0;
	dipPacket.UseVizQuery = 0;
	dipPacket.Endian = GPUENDIAN_8IN16;
	dipPacket.IndexBase = 0x0;
	dipPacket.IndexSize = 0;
	dipPacket.NumIndices = 0;
	dipPacket.IndexType = GPUINDEXTYPE_16BIT;
	dipPacket.MajorMode = 0;
	dipPacket.NotEndOfPacket = 0;
	dipPacket.PrimType = GPUPRIMTYPE_TRILIST;
	dipPacket.SrcSelect = GPUINDEXSELECT_DMA;



	dpPacketHeader.CountMinusOne = (sizeof(GPUCOMMAND_DRAW_AUTO)/sizeof(DWORD)) - 1;
	dpPacketHeader.OpCode = GPUCOMMANDOP_DRAW;
	dpPacketHeader.PacketType = GPU_PACKET_TYPE_3; 

	dpPacket.VizQueryId = 0;
	dpPacket.UseVizQuery = 0;
	dpPacket.PrimType = GPUPRIMTYPE_TRILIST;
	dpPacket.SrcSelect = GPUINDEXSELECT_AUTO;
	dpPacket.MajorMode = 0;
	dpPacket.NotEndOfPacket = 0;
	dpPacket.IndexSize = 1;
	dpPacket.NumIndices = 0;



}


precompiledBatch::~precompiledBatch()
{
	Cleanup();
}

D3DCommandBuffer * precompiledBatch::getCommandBuffer()
{
	return commandBuffer_temp;
}

D3DCommandBuffer * precompiledBatch::getSourceCommandBuffer()
{
	return sourcePrecompiledBuffer;
}

void precompiledBatch::CleanupTempData()
{
	if (sourcePrecompiledBuffer)
	{
		sourcePrecompiledBuffer->Release();
		sourcePrecompiledBuffer = NULL;
	}

}

void precompiledBatch::Cleanup()
{
	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
		m_pPixelShader = NULL;
	}

	if (m_pVertexShader)
	{
		m_pVertexShader->Release();
		m_pVertexShader = NULL;
	}

	CleanupTempData();


	commandBuffer_temp = NULL;

	if (pHeaderBytes_temp)
	{
		free(pHeaderBytes_temp);
		pHeaderBytes_temp = NULL;
		dwHeaderSize_temp = 0;
	}

	if (pInitializationBytes_temp)
	{
		free(pInitializationBytes_temp);
		pInitializationBytes_temp = NULL;
		dwInitializationSize_temp = 0;
	}

	if (dwCommandBufferData)
	{
		XPhysicalFree(dwCommandBufferData);
		dwCommandBufferData = NULL;
	}

	dwCommandBufferDataSize = 0;

	dwDepthControl_Offset = GPU_INVALID_OFFSET;
	dwColorMask_Offset = GPU_INVALID_OFFSET;
	dwBlendControl0_Offset = GPU_INVALID_OFFSET;
	dwBlendControl1_Offset = GPU_INVALID_OFFSET;
	dwBlendControl2_Offset = GPU_INVALID_OFFSET;
	dwBlendControl3_Offset = GPU_INVALID_OFFSET;


	dwSetCullMode_Offset = GPU_INVALID_OFFSET;
	dwSetVertexShader_Offset_ = GPU_INVALID_OFFSET;
	dwSetPixelShader_Offset_ = GPU_INVALID_OFFSET;
	dwDrawToken_Offset = GPU_INVALID_OFFSET;
	dwEndCommandBuffer_Offset = GPU_INVALID_OFFSET;
	dwSizeInDwords = 0;
	dwCurrentSizeInDwords = 0;

	for (DWORD i = 0; i < MAX_STREAMS_COUNT; i++)
	{
		dwStreamSource_Offsets[i] = GPU_INVALID_OFFSET;
	}

	for (DWORD i = 0; i < MAX_TEXTURES_COUNT; i++)
	{
		dwTexture_Offsets[i] = GPU_INVALID_OFFSET;
	}

	for (DWORD i = 0; i < MAX_ALU_CONST_COUNT; i++)
	{
		dwALUConst_Offsets[i] = GPU_INVALID_OFFSET;
	}

	for (DWORD i = 0; i < MAX_ALU_CONST_COUNT_BOOL; i++)
	{
		dwALUConstBool_Offsets[i] = GPU_INVALID_OFFSET;
	}


	ps_consts_v4.DelAll();
	vs_consts_v4.DelAll();
	ps_consts_b.DelAll();
	vs_consts_b.DelAll();
	tex_consts.DelAll();

}


//===========================================================================================================================================

void precompiledBatch::DeleteTraceFile()
{
	DeleteFile(DUMP_FILE_NAME);
}

void precompiledBatch::Trace (const char * str, ...)
{
	if (!bGlobalDump)
	{
		return;
	}

	static char tempBuffer[16384];

	va_list args;
	va_start(args, str);
	vsnprintf_s(tempBuffer, 16384 - 4, 16384-4, str, args);
	va_end(args);

/*
	FILE* file = 0;
	fopen_s(&file, DUMP_FILE_NAME, "a+b");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		long len = strlen(tempBuffer);
		fwrite(tempBuffer, len, 1, file);
		fclose(file);
	}
*/

	OutputDebugString(tempBuffer);
}


//Парсер коммандного буфера
//===========================================================================================================================================
void precompiledBatch::parseCommandBuffer(BYTE * memoryPtr)
{
	baseData = memoryPtr;

	//По данным шагаем dword'ами
	const DWORD * dataStream = (DWORD *)baseData;

	Trace("Parse command buffer addr = 0x%08X\n", (DWORD)baseData);
	Trace("=====================================================================\n");

	//Общий хидер
	GPUCOMMANDBUFFER_HEADER_PACKET_COMMON header_s;

	//Больше 65K пакетов быть не может
	for (DWORD i = 0; i < 0xFFFF; i++)
	{
		//Рассчитываем текущий offset
		dwParser_Offset = (((DWORD)dataStream - (DWORD)baseData));

		//Считываем хидер пакета
		header_s.dword = *dataStream;

		Trace("offset = 0x%08X ", dwParser_Offset);

		//Рассчитываем "магическую" константу являющуюся признаком окончания command buffer
		DWORD dwMagicConstEndOfBuffer = GPUGenerateMagicValue(dataStream);


		dataStream++;


		//нашли окончание command buffer'a
		//это de-constructed буффер, к рендеру не готовый
		if (header_s.dword == 0x00000000)
		{
			DWORD endToken = *dataStream;
			if (endToken == GPU_END_CB_TOKEN)
			{
				Trace("deconstructed command buffer - parse end!, parsed %d bytes\n", dwParser_Offset);
				Trace("=====================================================================\n");

				if (bLogOnlyWhenParse == false)
				{
					dwEndCommandBuffer_Offset = dwParser_Offset;
					Assert (dwEndCommandBuffer_Offset <= 262140);
					dwSizeInDwords = (WORD)(dwEndCommandBuffer_Offset / sizeof(DWORD));
					dwCurrentSizeInDwords = dwSizeInDwords;
				}

				baseData = 0;
				return;
			} else
			{
				Assert(false);
			}
		}


		//нашли окончание command buffer'a
		//это constructed буффер, готовый к рендеру
		if (header_s.dword == dwMagicConstEndOfBuffer)
		{
			DWORD endToken = *dataStream;
			if (endToken == GPU_END_CB_TOKEN)
			{
				Trace("constructed command buffer - parse end!, parsed %d bytes\n", dwParser_Offset);
				Trace("=====================================================================\n");

				if (bLogOnlyWhenParse == false)
				{
					dwEndCommandBuffer_Offset = dwParser_Offset;
					Assert (dwEndCommandBuffer_Offset <= 262140);
					dwSizeInDwords = (WORD)(dwEndCommandBuffer_Offset / sizeof(DWORD));
					dwCurrentSizeInDwords = dwSizeInDwords;
				}
				baseData = 0;
				return;
			} else
			{
				Assert(false);
			}
		}


		//Смторим на типа пакета...
		switch (header_s.Packet0.PacketType)
		{
		case GPU_PACKET_TYPE_0:
			{
				//Это настройки для render targets 0..3
				if (GPUREG_BLENDCONTROL1 == header_s.Packet0.BaseRegisterIndex && header_s.Packet0.CountMinusOne == 2)
				{
					//FIXME
					//OutputDebugString("----------------------- need to crop this block !!!!!!!!!!!! --------------------------------------------------------\n");
				}

				ParseType0Packet (dataStream, (header_s.Packet0.CountMinusOne+1), header_s.Packet0.BaseRegisterIndex);
				dataStream += (header_s.Packet0.CountMinusOne+1);
				break;
			}
		case GPU_PACKET_TYPE_1:
			{
				DWORD regData1 = *dataStream;
				Trace("Write register[%d] = %d\n", header_s.Packet1.regIndex1, regData1);
				dataStream++;

				DWORD regData2 = *dataStream;
				Trace("Write register[%d] = %d\n", header_s.Packet1.regIndex2, regData2);
				dataStream++;

				break;
			}
		case GPU_PACKET_TYPE_2:
			{
				Trace("Filler packet\n");
				Trace("{\n");
				Trace("}\n");
				Assert(header_s.Packet2.Reserved == 0x0);

				break;
			}
		case GPU_PACKET_TYPE_3:
			{
				ParseType3Packet (header_s.Packet3.OpCode, dataStream, (header_s.Packet3.CountMinusOne+1));
				DWORD dwStepSize = (header_s.Packet3.CountMinusOne + 1);
				dataStream += dwStepSize;
				break;
			}
		} // awitch ()
	} // for()


	Assert(false);
}


//Парсер значений vertex fetch constant
//===========================================================================================================================================
void precompiledBatch::ParseVertexFetchConst(DWORD dwStreamIndex, DWORD dwRegisterElementIndex, DWORD regData)
{
	Trace("   {\n");

	//Vertex fetch константы состоят из двух dword всегда
	switch (dwRegisterElementIndex)
	{
	case 0:
		{
			if (bLogOnlyWhenParse == false)
			{
				Assert(dwStreamIndex >= 0 && dwStreamIndex < MAX_STREAMS_COUNT);
				dwStreamSource_Offsets[dwStreamIndex] = dwParser_Offset;
			}

			GPUVERTEX_FETCH_CONSTANT_0 data;
			data.dword[0] = regData;

			Trace("       Type = %d\n", data.Type);
			Trace("       BaseAddress = 0x%08X (Effective : 0x%08X)\n", data.BaseAddress, data.BaseAddress << 12);

			Assert(data.Type == 0 || data.Type == GPUCONSTANTTYPE_INVALID_VERTEX || data.Type == GPUCONSTANTTYPE_VERTEX);
			break;
		}
	case 1:
		{
			GPUVERTEX_FETCH_CONSTANT_1 data;
			data.dword[0] = regData;

			Trace("       Endian = %d\n", data.Endian);
			Trace("       Size = %d\n", data.Size);
			Trace("       AddressClamp = %d\n", data.AddressClamp);
			Trace("       RequestSize = %d\n", data.RequestSize);
			Trace("       ClampDisable = %d\n", data.ClampDisable);

			break;
		}
	default:
		Assert(false);
	}

	Trace("   }\n");
}

//Парсер значений texture fetch constant
//===========================================================================================================================================
void precompiledBatch::ParseTextureFetchConst(DWORD dwRegIndex, DWORD dwRegisterElementIndex, DWORD regData)
{
	Trace("   {\n");

	//Texture fetch константы состоят из шести dword всегда
	switch (dwRegisterElementIndex)
	{
	case 0:
		{
/*
			if (dwRegIndex >= MAX_TEXTURES_COUNT)
			{
				int a = 0;
			}
*/

			if (bLogOnlyWhenParse == false)
			{
				Assert(dwRegIndex >= 0 && dwRegIndex < MAX_TEXTURES_COUNT);
				dwTexture_Offsets[dwRegIndex] = dwParser_Offset;
			}

			GPUTEXTURE_FETCH_CONSTANT_0 data;
			data.dword[0] = regData;

			Trace("       Type = %d\n", data.Type);
			Trace("       SignX = %d\n", data.SignX);
			Trace("       SignY = %d\n", data.SignX);
			Trace("       SignZ = %d\n", data.SignX);
			Trace("       SignW = %d\n", data.SignW);

			Trace("       ClampX = %d\n", data.ClampX);
			Trace("       ClampY = %d\n", data.ClampY);
			Trace("       ClampZ = %d\n", data.ClampZ);

			Trace("       Pitch = %d (Effective : %d)\n", data.Pitch, data.Pitch<<5);
			Trace("       Tiled = %d\n", data.Tiled);

			Assert(data.Type == 0 || data.Type == GPUCONSTANTTYPE_TEXTURE || data.Type == GPUCONSTANTTYPE_INVALID_TEXTURE);
			
			break;
		}
	case 1:
		{
			GPUTEXTURE_FETCH_CONSTANT_1 data;
			data.dword[0] = regData;

			Trace("       DataFormat = %d\n", data.DataFormat);
			Trace("       Endian = %d\n", data.Endian);
			Trace("       RequestSize = %d\n", data.RequestSize);
			Trace("       Stacked = %d\n", data.Stacked);
			Trace("       ClampPolicy = %d\n", data.ClampPolicy);
			Trace("       BaseAddress = 0x%08X (Effective : 0x%08X)\n", data.BaseAddress, data.BaseAddress << 12);

			break;
		}
	case 2:
		{
			Trace("       TexSize\n");
			break;
		}
	case 3:
		{
			GPUTEXTURE_FETCH_CONSTANT_3 data;
			data.dword[0] = regData;

			Trace("       NumFormat = %d\n", data.NumFormat);
			Trace("       SwizzleX = %d\n", data.SwizzleX);
			Trace("       SwizzleY = %d\n", data.SwizzleY);
			Trace("       SwizzleZ = %d\n", data.SwizzleZ);
			Trace("       SwizzleW = %d\n", data.SwizzleW);
			Trace("       ExpAdjust = %d\n", data.ExpAdjust);
			Trace("       MagFilter = %d\n", data.MagFilter);
			Trace("       MinFilter = %d\n", data.MinFilter);
			Trace("       MipFilter = %d\n", data.MipFilter);
			Trace("       AnisoFilter = %d\n", data.AnisoFilter);
			Trace("       BorderSize = %d\n", data.BorderSize);
			break;
		}
	case 4:
		{
			GPUTEXTURE_FETCH_CONSTANT_4 data;
			data.dword[0] = regData;

			Trace("       VolMagFilter = %d\n", data.VolMagFilter);
			Trace("       VolMinFilter = %d\n", data.VolMinFilter);
			Trace("       MinMipLevel = %d\n", data.MinMipLevel);
			Trace("       MaxMipLevel = %d\n", data.MaxMipLevel);
			Trace("       MagAnisoWalk = %d\n", data.MagAnisoWalk);
			Trace("       MinAnisoWalk = %d\n", data.MinAnisoWalk);
			Trace("       LODBias = %d\n", data.LODBias);
			Trace("       GradExpAdjustH = %d\n", data.GradExpAdjustH);
			Trace("       GradExpAdjustV = %d\n", data.GradExpAdjustV);

			break;
		}
	case 5:
		{
			GPUTEXTURE_FETCH_CONSTANT_5 data;
			data.dword[0] = regData;

			Trace("       BorderColor = %d\n", data.BorderColor);
			Trace("       ForceBCWToMax = %d\n", data.ForceBCWToMax);
			Trace("       TriClamp = %d\n", data.TriClamp);
			Trace("       AnisoBias = %d\n", data.AnisoBias);
			Trace("       Dimension = %d\n", data.Dimension);
			Trace("       PackedMips = %d\n", data.PackedMips);
			Trace("       MipAddress = 0x%08X (Effective : 0x%08X)\n", data.MipAddress, data.MipAddress<<12);

			break;
		}
	default:
		Assert(false);
	}

	Trace("   }\n");
}



//Парсер пакетов 0 типа (установка пачки регистров GPU)
//===========================================================================================================================================
void precompiledBatch::ParseType0Packet (const DWORD * dataStream, DWORD count, DWORD gpuRegisterBase)
{
	Trace("Write DWORD to %d registers (base index = 0x%08X)\n", count, gpuRegisterBase);
	Trace("{\n");

	for (DWORD regIdx = 0; regIdx < count; regIdx++)
	{
		dwParser_Offset = (((DWORD)dataStream - (DWORD)baseData));

		Trace("   ");
		DWORD regData = *dataStream;

		bool bNotFound = false;

		DWORD regIndex = gpuRegisterBase + regIdx;
		GPUREGISTER gpuRegIndex = (GPUREGISTER)regIndex;

		switch (gpuRegIndex)
		{
		case GPUREG_WAITUNTIL:
			{
				Trace("GPUREG_WAITUNTIL\n");
				break;
			}
		case GPUREG_COHERSIZEHOST:
			{
				Trace("GPUREG_COHERSIZEHOST = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERBASEHOST:
			{
				Trace("GPUREG_COHERBASEHOST = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERSTATUSHOST:
			{
				GPU_COHERSTATUS dta;
				dta.dword = regData;

				Trace("GPUREG_COHERSTATUSHOST = [%d, %d, %d, %d, %d, %d, %d, %d]\n", dta.DestBase0Enable, dta.DestBase1Enable, dta.DestBase2Enable, dta.DestBase3Enable, dta.DestBase4Enable, dta.DestBase5Enable, dta.DestBase6Enable, dta.DestBase7Enable);
				break;
			}

		case GPUREG_CPUINTERRUPTACK:
			{
				Trace("GPUREG_CPUINTERRUPTACK\n");
				break;
			}

		case GPUREG_CALLBACKADDRESS:
			{
				Trace("GPUREG_CALLBACKADDRESS\n");
				break;
			}

		case GPUREG_GPRMANAGEMENT:
			{
				Trace("GPUREG_GPRMANAGEMENT\n");
				break;
			}

		case GPUREG_INSTSTOREMANAGEMENT:
			{
				Trace("GPUREG_INSTSTOREMANAGEMENT\n");
				break;
			}

		case GPUREG_INVALIDATECONSTANTS:
			{
				Trace("GPUREG_INVALIDATECONSTANTS\n");
				break;
			}

		case GPUREG_BCCONTROL:
			{
				Trace("GPUREG_BCCONTROL\n");
				break;
			}

		case GPUREG_SURFACEINFO: 
			{
				Trace("GPUREG_SURFACEINFO\n");
				break;
			}
		case GPUREG_COLOR0INFO:
			{
				Trace("GPUREG_COLOR0INFO\n");
				break;
			}
		case GPUREG_DEPTHINFO:
			{
				Trace("GPUREG_DEPTHINFO\n");
				break;
			}
		case GPUREG_COLOR1INFO:
			{
				Trace("GPUREG_COLOR1INFO\n");
				break;
			}
		case GPUREG_COLOR2INFO:
			{
				Trace("GPUREG_COLOR2INFO\n");
				break;
			}
		case GPUREG_COLOR3INFO:
			{
				Trace("GPUREG_COLOR3INFO\n");
				break;
			}
		case GPUREG_COHERDESTBASE0:
			{
				Trace("GPUREG_COHERDESTBASE0\n");
				break;
			}
		case GPUREG_COHERDESTBASE1:
			{
				Trace("GPUREG_COHERDESTBASE1 = 0x%08X\n", regData);
				break;
			}

		case GPUREG_COHERDESTBASE2:
			{
				Trace("GPUREG_COHERDESTBASE2 = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERDESTBASE3:
			{
				Trace("GPUREG_COHERDESTBASE3 = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERDESTBASE4:
			{
				Trace("GPUREG_COHERDESTBASE4 = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERDESTBASE5:
			{
				Trace("GPUREG_COHERDESTBASE5 = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERDESTBASE6:
			{
				Trace("GPUREG_COHERDESTBASE6 = 0x%08X\n", regData);
				break;
			}
		case GPUREG_COHERDESTBASE7:
			{
				Trace("GPUREG_COHERDESTBASE7 = 0x%08X\n", regData);
				break;
			}
		case GPUREG_SCREENSCISSORTL:
			{
				Trace("GPUREG_SCREENSCISSORTL\n");
				break;
			}
		case GPUREG_SCREENSCISSORBR:
			{
				Trace("GPUREG_SCREENSCISSORBR\n");
				break;
			}

		case GPUREG_WINDOWOFFSET:
			{
				Trace("GPUREG_WINDOWOFFSET\n");
				break;
			}

		case GPUREG_WINDOWSCISSORTL:
			{
				Trace("GPUREG_WINDOWSCISSORTL\n");
				break;
			}
		case GPUREG_WINDOWSCISSORBR:
			{
				Trace("GPUREG_WINDOWSCISSORBR\n");
				break;
			}

		case GPUREG_MAXVTXINDX: 
			{
				Trace("GPUREG_MAXVTXINDX\n");
				break;
			}

		case GPUREG_MINVTXINDX:
			{
				Trace("GPUREG_MINVTXINDX\n");
				break;
			}
		case GPUREG_INDXOFFSET:
			{
				dwIndxOffset_Offset = dwParser_Offset;
				Trace("GPUREG_INDXOFFSET = 0x%08X\n", regData);
				break;
			}
		case GPUREG_MULTIPRIMIBRESETINDX:
			{
				Trace("GPUREG_MULTIPRIMIBRESETINDX\n");
				break;
			}
		case GPUREG_COLORMASK:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwColorMask_Offset = dwParser_Offset;
				}

				GPU_COLORMASK dta;
				dta.dword = regData;

				originalColorMask = dta.Write0;

				Trace("GPUREG_COLORMASK [%d, %d, %d, %d]\n", dta.Write0, dta.Write1, dta.Write2, dta.Write3);
				break;
			}
		case GPUREG_BLENDRED:
			{
				Trace("GPUREG_BLENDRED\n");
				break;
			}
		case GPUREG_BLENDGREEN:
			{
				Trace("GPUREG_BLENDGREEN\n");
				break;
			}
		case GPUREG_BLENDBLUE:
			{
				Trace("GPUREG_BLENDBLUE\n");
				break;
			}

		case GPUREG_BLENDALPHA:
			{
				Trace("GPUREG_BLENDALPHA\n");
				break;
			}
		case GPUREG_STENCILREFMASKBF:
			{
				Trace("GPUREG_STENCILREFMASKBF\n");
				break;
			}
		case GPUREG_STENCILREFMASK:
			{
				Trace("GPUREG_STENCILREFMASK\n");
				break;
			}
		case GPUREG_ALPHAREF:
			{
				float * pFloat = (float *)&regData;
				Trace("GPUREG_ALPHAREF [%f]\n", pFloat);
				break;
			}
		case GPUREG_VPORTXSCALE:
			{
				Trace("GPUREG_VPORTXSCALE\n");
				break;
			}

		case GPUREG_VPORTXOFFSET:
			{
				Trace("GPUREG_VPORTXOFFSET\n");
				break;
			}
		case GPUREG_VPORTYSCALE:
			{
				Trace("GPUREG_VPORTYSCALE\n");
				break;
			}
		case GPUREG_VPORTYOFFSET:
			{
				Trace("GPUREG_VPORTYOFFSET\n");
				break;
			}
		case GPUREG_VPORTZSCALE:
			{
				Trace("GPUREG_VPORTZSCALE\n");
				break;
			}
		case GPUREG_VPORTZOFFSET:
			{
				Trace("GPUREG_VPORTZOFFSET\n");
				break;
			}

		case GPUREG_PROGRAMCONTROL:
			{
				GPU_PROGRAMCONTROL dta;
				dta.dword = regData;

				Trace("GPUREG_PROGRAMCONTROL (VsMaxReg = %d, PsMaxReg = %d)...\n", dta.VsMaxReg, dta.PsMaxReg);
				break;
			}

		case GPUREG_CONTEXTMISC:
			{
				GPU_CONTEXTMISC dta;
				dta.dword = regData;

				Trace("GPUREG_CONTEXTMISC (SampleControl = %d)...\n", dta.SampleControl);
				break;
			}
		case GPUREG_INTERPOLATORCONTROL:
			{
				GPU_INTERPOLATORCONTROL dta;
				dta.dword = regData;

				Trace("GPUREG_INTERPOLATORCONTROL (ParamShade = %d, SamplingPattern = %d)...\n", dta.ParamShade, dta.SamplingPattern);
				break;
			}
		case GPUREG_WRAPPING0:
			{
				Trace("GPUREG_WRAPPING0\n");
				break;
			}
		case GPUREG_WRAPPING1:
			{
				Trace("GPUREG_WRAPPING1\n");
				break;
			}

		case GPUREG_DEPTHCONTROL:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwDepthControl_Offset = dwParser_Offset;
				}

				GPU_DEPTHCONTROL dta;
				dta.dword = regData;

				originalZEnable = dta.ZWriteEnable;

				Trace("GPUREG_DEPTHCONTROL [zenable = %d, zwriteenable = %d]\n", dta.ZEnable, dta.ZWriteEnable);
				break;
			}

		case GPUREG_BLENDCONTROL0:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwBlendControl0_Offset = dwParser_Offset;
				}

				GPU_BLENDCONTROL dta;
				dta.dword = regData;

				Trace("GPUREG_BLENDCONTROL0 color[%d, %d, %d] alpha[%d, %d, %d]\n", dta.ColorBlendOp, dta.ColorDestBlend, dta.ColorSrcBlend, dta.AlphaBlendOp, dta.AlphaDestBlend, dta.AlphaSrcBlend);
				break;
			}
		case GPUREG_COLORCONTROL:
			{
				GPU_COLORCONTROL dta;
				dta.dword = regData;

				Trace("GPUREG_COLORCONTROL [alpha test = %d, alpha func = %d, alpha to mask = %d]\n", dta.AlphaTestEnable, dta.AlphaFunc, dta.AlphaToMaskEnable);
				break;
			}
		case GPUREG_HICONTROL:
			{
				GPU_HICONTROL dta;
				dta.dword = regData;

				if (bLogOnlyWhenParse == false)
				{
					dwHiZControl_Offset = dwParser_Offset;
				}

				originalHyperZEnable = dta.HiZWriteEnable;

				Trace("GPUREG_HICONTROL (HiZEnable = %d)...\n", dta.HiZEnable);

				break;
			}
		case GPUREG_CLIPCONTROL:
			{
				GPU_CLIPCONTROL dta;
				dta.dword = regData;

				Trace("GPUREG_CLIPCONTROL [%d, %d, %d, %d, %d, %d], mode %d, disable %d, cullonly %d\n", dta.ClipPlaneEnable0, dta.ClipPlaneEnable1, dta.ClipPlaneEnable2, dta.ClipPlaneEnable3, dta.ClipPlaneEnable4, dta.ClipPlaneEnable5, dta.ClipPlaneMode, dta.ClipDisable, dta.ClipPlaneCullOnlyEnable);
				break;
			}
		case GPUREG_MODECONTROL:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwSetCullMode_Offset = dwParser_Offset;
				}

				GPU_MODECONTROL dta;
				dta.dword = regData;

				originalGpuCull = (GPUCULL)dta.CullMode;

				Trace("GPUREG_MODECONTROL (CullMode = %d, MSAA = %d)...\n", dta.CullMode, dta.MsaaEnable);

				
				break;
			}
		case GPUREG_VTECONTROL:
			{
				Trace("GPUREG_VTECONTROL\n");
				break;
			}

		case GPUREG_EDRAMMODECONTROL:
			{
				GPU_EDRAMMODECONTROL dta;
				dta.dword = regData;

				const char * szMode = "unkonwn";
				switch (dta.EdramMode)
				{
				case GPUEDRAMMODE_NOP:
					szMode = "GPUEDRAMMODE_NOP";
					break;
				case GPUEDRAMMODE_COLOR_DEPTH:
					szMode = "GPUEDRAMMODE_COLOR_DEPTH";
					break;
				case GPUEDRAMMODE_DOUBLE_DEPTH:
					szMode = "GPUEDRAMMODE_DOUBLE_DEPTH";
					break;
				case GPUEDRAMMODE_COPY:
					szMode = "GPUEDRAMMODE_COPY";
					break;
				}


				Trace("GPUREG_EDRAMMODECONTROL [mode = '%s', %d]\n", szMode, dta.ColorDepthMacro);
				break;
			}
		case GPUREG_BLENDCONTROL1:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwBlendControl1_Offset = dwParser_Offset;
				}

				Trace("GPUREG_BLENDCONTROL1\n");
				break;
			}
		case GPUREG_BLENDCONTROL2:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwBlendControl2_Offset = dwParser_Offset;
				}

				Trace("GPUREG_BLENDCONTROL2\n");
				break;
			}
		case GPUREG_BLENDCONTROL3:
			{
				if (bLogOnlyWhenParse == false)
				{
					dwBlendControl3_Offset = dwParser_Offset;
				}

				Trace("GPUREG_BLENDCONTROL3\n");
				break;
			}

		case GPUREG_POINTSIZE:
			{
				Trace("GPUREG_POINTSIZE\n");
				break;
			}
		case GPUREG_POINTMINMAX:
			{
				Trace("GPUREG_POINTMINMAX\n");
				break;
			}
		case GPUREG_LINECONTROL:
			{
				Trace("GPUREG_LINECONTROL\n");
				break;
			}
		case GPUREG_OUTPUTPATHCONTROL:
			{
				Trace("GPUREG_OUTPUTPATHCONTROL\n");
				break;
			}
		case GPUREG_HOSCONTROL:
			{
				Trace("GPUREG_HOSCONTROL\n");
				break;
			}
		case GPUREG_HOSMAXTESSLEVEL:
			{
				Trace("GPUREG_HOSMAXTESSLEVEL\n");
				break;
			}
		case GPUREG_HOSMINTESSLEVEL:
			{
				Trace("GPUREG_HOSMINTESSLEVEL\n");
				break;
			}

		case GPUREG_HOSREUSEDEPTH:
			{
				Trace("GPUREG_HOSREUSEDEPTH\n");
				break;
			}
		case GPUREG_GROUPPRIMTYPE:
			{
				Trace("GPUREG_GROUPPRIMTYPE\n");
				break;
			}
		case GPUREG_GROUPFIRSTDECR:
			{
				Trace("GPUREG_GROUPFIRSTDECR\n");
				break;
			}
		case GPUREG_GROUPDECR:
			{
				Trace("GPUREG_GROUPDECR\n");
				break;
			}
		case GPUREG_GROUPVECT0CONTROL:
			{
				Trace("GPUREG_GROUPVECT0CONTROL\n");
				break;
			}
		case GPUREG_GROUPVECT1CONTROL:
			{
				Trace("GPUREG_GROUPVECT1CONTROL\n");
				break;
			}
		case GPUREG_GROUPVECT0FMTCONTROL:
			{
				Trace("GPUREG_GROUPVECT0FMTCONTROL\n");
				break;
			}
		case GPUREG_GROUPVECT1FMTCONTROL:
			{
				Trace("GPUREG_GROUPVECT1FMTCONTROL\n");
				break;
			}

		case GPUREG_MPASSPSCONTROL:
			{
				Trace("GPUREG_MPASSPSCONTROL\n");
				break;
			}
		case GPUREG_VIZQUERY:
			{
				Trace("GPUREG_VIZQUERY\n");
				break;
			}
		case GPUREG_ENHANCE:
			{
				Trace("GPUREG_ENHANCE\n");
				break;
			}

		case GPUREG_SCLINECONTROL:
			{
				Trace("GPUREG_SCLINECONTROL\n");
				break;
			}
		case GPUREG_AACONFIG:
			{
				Trace("GPUREG_AACONFIG\n");
				break;
			}
		case GPUREG_VTXCONTROL:
			{
				Trace("GPUREG_VTXCONTROL\n");
				break;
			}
		case GPUREG_GBVERTCLIPADJ:
			{
				Trace("GPUREG_GBVERTCLIPADJ\n");
				break;
			}
		case GPUREG_GBVERTDISCADJ:
			{
				Trace("GPUREG_GBVERTDISCADJ\n");
				break;
			}
		case GPUREG_GBHORZCLIPADJ:
			{
				Trace("GPUREG_GBHORZCLIPADJ\n");
				break;
			}
		case GPUREG_GBHORZDISCADJ:
			{
				Trace("GPUREG_GBHORZDISCADJ\n");
				break;
			}
		case GPUREG_VSCONST:
			{
				Trace("GPUREG_VSCONST\n");
				break;
			}

		case GPUREG_PSCONST:
			{
				Trace("GPUREG_PSCONST\n");
				break;
			}
		case GPUREG_DEBUGMISC0:
			{
				Trace("GPUREG_DEBUGMISC0\n");
				break;
			}
		case GPUREG_DEBUGMISC1:
			{
				Trace("GPUREG_DEBUGMISC1\n");
				break;
			}
		case GPUREG_AAMASK:
			{
				Trace("GPUREG_AAMASK\n");
				break;
			}
		case GPUREG_VERTEXREUSEBLOCKCONTROL:
			{
				Trace("GPUREG_VERTEXREUSEBLOCKCONTROL\n");
				break;
			}
		case GPUREG_OUTDEALLOCCONTROL:
			{
				Trace("GPUREG_OUTDEALLOCCONTROL\n");
				break;
			}

		case GPUREG_COPYCONTROL:
			{
				Trace("GPUREG_COPYCONTROL\n");
				break;
			}
		case GPUREG_COPYDESTBASE:
			{
				Trace("GPUREG_COPYDESTBASE\n");
				break;
			}
		case GPUREG_COPYDESTPITCH:
			{
				Trace("GPUREG_COPYDESTPITCH\n");
				break;
			}
		case GPUREG_COPYDESTINFO:
			{
				Trace("GPUREG_COPYDESTINFO\n");
				break;
			}
		case GPUREG_HICLEAR:
			{
				Trace("GPUREG_HICLEAR\n");
				break;
			}
		case GPUREG_DEPTHCLEAR:
			{
				Trace("GPUREG_DEPTHCLEAR\n");
				break;
			}
		case GPUREG_COLORCLEAR:
			{
				Trace("GPUREG_COLORCLEAR\n");
				break;
			}
		case GPUREG_COLORCLEARLO:
			{
				Trace("GPUREG_COLORCLEARLO\n");
				break;
			}

		case GPUREG_COPYFUNC:
			{
				Trace("GPUREG_COPYFUNC\n");
				break;
			}
		case GPUREG_COPYREF:
			{
				Trace("GPUREG_COPYREF\n");
				break;
			}
		case GPUREG_COPYMASK:
			{
				Trace("GPUREG_COPYMASK\n");
				break;
			}
		case GPUREG_COPYSURFACESLICE:
			{
				Trace("GPUREG_COPYSURFACESLICE\n");
				break;
			}
		case GPUREG_SAMPLECOUNTCONTROL:
			{
				Trace("GPUREG_SAMPLECOUNTCONTROL\n");
				break;
			}
		case GPUREG_SAMPLECOUNTADDRESS:
			{
				Trace("GPUREG_SAMPLECOUNTADDRESS\n");
				break;
			}

		case GPUREG_POLYOFFSETFRONTSCALE:
			{
				Trace("GPUREG_POLYOFFSETFRONTSCALE\n");
				break;
			}
		case GPUREG_POLYOFFSETFRONTOFFSET:
			{
				Trace("GPUREG_POLYOFFSETFRONTOFFSET\n");
				break;
			}
		case GPUREG_POLYOFFSETBACKSCALE:
			{
				Trace("GPUREG_POLYOFFSETBACKSCALE\n");
				break;
			}
		case GPUREG_POLYOFFSETBACKOFFSET:
			{
				Trace("GPUREG_POLYOFFSETBACKOFFSET\n");
				break;
			}
		case GPUREG_POINTXRAD:
			{
				Trace("GPUREG_POINTXRAD\n");
				break;
			}
		case GPUREG_POINTYRAD:
			{
				Trace("GPUREG_POINTYRAD\n");
				break;
			}
		case GPUREG_POINTCONSTANTSIZE:
			{
				Trace("GPUREG_POINTCONSTANTSIZE\n");
				break;
			}
		case GPUREG_POINTCULLRAD:
			{
				Trace("GPUREG_POINTCULLRAD\n");
				break;
			}
		default:
			bNotFound = true;

		} //switch


		//Если не нашли проверяем не одна из константа ли это, и номер константы вычисляем
		if (bNotFound)
		{
			if (regIndex > GPUREG_CALLBACKADDRESS && regIndex < GPUREG_GPRMANAGEMENT)
			{
				Trace("Pix, Debug data... [0x%08X]\n", regData);
				bNotFound = false;
			}


			if (regIndex >= GPUREG_CLIPPLANE0 && regIndex < GPUREG_CLIPPLANE0+4)
			{
				DWORD dwComponent = (regIndex - GPUREG_CLIPPLANE0);

				float val = *((float*)&regData);
				Trace("ClipPlane0[%d] = %f\n", dwComponent, val);
				bNotFound = false;
			}

			if (regIndex >= GPUREG_CLIPPLANE1 && regIndex < GPUREG_CLIPPLANE1+4)
			{
				DWORD dwComponent = (regIndex - GPUREG_CLIPPLANE1);

				float val = *((float*)&regData);
				Trace("ClipPlane1[%d] = %f\n", dwComponent, val);
				bNotFound = false;
			}

			if (regIndex >= GPUREG_CLIPPLANE2 && regIndex < GPUREG_CLIPPLANE2+4)
			{
				DWORD dwComponent = (regIndex - GPUREG_CLIPPLANE2);

				float val = *((float*)&regData);
				Trace("ClipPlane2[%d] = %f\n", dwComponent, val);
				bNotFound = false;
			}

			if (regIndex >= GPUREG_CLIPPLANE3 && regIndex < GPUREG_CLIPPLANE3+4)
			{
				DWORD dwComponent = (regIndex - GPUREG_CLIPPLANE3);

				float val = *((float*)&regData);
				Trace("ClipPlane3[%d] = %f\n", dwComponent, val);
				bNotFound = false;
			}

			if (regIndex >= GPUREG_CLIPPLANE4 && regIndex < GPUREG_CLIPPLANE4+4)
			{
				DWORD dwComponent = (regIndex - GPUREG_CLIPPLANE4);

				float val = *((float*)&regData);
				Trace("ClipPlane4[%d] = %f\n", dwComponent, val);
				bNotFound = false;
			}

			if (regIndex >= GPUREG_CLIPPLANE5 && regIndex < GPUREG_CLIPPLANE5+4)
			{
				DWORD dwComponent = (regIndex - GPUREG_CLIPPLANE5);

				float val = *((float*)&regData);
				Trace("ClipPlane5[%d] = %f\n", dwComponent, val);
				bNotFound = false;
			}


			//Vertex или Pixel shader константа
			if (regIndex >= GPUREG_ALUCONSTANTS && regIndex < GPUREG_ALUCONSTANTS+(GPU_ALU_CONSTANTS*4))
			{
				DWORD dwAluConstIndex = (regIndex - GPUREG_ALUCONSTANTS) / sizeof(DWORD);
				DWORD dwComponent = (regIndex - GPUREG_ALUCONSTANTS) - (dwAluConstIndex * sizeof(DWORD));


				if (bLogOnlyWhenParse == false)
				{
					if (dwComponent == 0)
					{
						dwALUConst_Offsets[dwAluConstIndex] = dwParser_Offset;
					}
				}

				float val = *((float*)&regData);

				Trace("Alu const [%d].v[%d] = %f\n", dwAluConstIndex, dwComponent, val);
				bNotFound = false;
			}


			//Установка врешинного потока или установка текстуры
			if (regIndex >= GPUREG_FETCHCONSTANTS && regIndex < GPUREG_FETCHCONSTANTS+(GPU_FETCH_CONSTANTS*sizeof(GPUFETCH_CONSTANT)/sizeof(DWORD)))
			{
				DWORD dwFetchConstIndex = (regIndex-GPUREG_FETCHCONSTANTS)/(sizeof(GPUFETCH_CONSTANT)/sizeof(DWORD));

				DWORD dwFetchConstElement = (regIndex-GPUREG_FETCHCONSTANTS) - dwFetchConstIndex*(sizeof(GPUFETCH_CONSTANT)/sizeof(DWORD));

				//Вертексные потоки устанавливаются с последнего регистра и растут в обратную сторону
				//31 - это потоки 0, 1, 2
				//30 - это потоки 3, 4, 5
				if (dwFetchConstIndex == 31)
				{
					DWORD dwVertexStreamIndex = 95 - ((regIndex-GPUREG_FETCHCONSTANTS)/(sizeof(GPUVERTEX_FETCH_CONSTANT)/sizeof(DWORD)));

					//они пачками по 3 штуки, поэтому вместо 6 всего 2 индекса...
					dwFetchConstElement = dwFetchConstElement % 2;
					Trace("Vertex Fetch const [%d].v[%d]\n", dwVertexStreamIndex, dwFetchConstElement);
					ParseVertexFetchConst(dwVertexStreamIndex, dwFetchConstElement, regData);
				} else
				{
					//Установка текстуры с нулевого регистра и растет вниз как обычно
					Trace("Texture Fetch const [%d].v[%d]\n", dwFetchConstIndex, dwFetchConstElement);
					ParseTextureFetchConst(dwFetchConstIndex, dwFetchConstElement, regData);
				}

				bNotFound = false;
			}


			//
			if (regIndex >= GPUREG_FLOWCONSTANTS && regIndex < GPUREG_FLOWCONSTANTS+GPU_FLOW_CONSTANTS)
			{
				//
				//Trace("Flow const\n");
				bNotFound = false;
			}

			//Bool константы упакованы побитно, в одном dword 32 константы (256 штук - 8 dword)
			if (regIndex >= GPUREG_BOOLEANCONSTANTS && regIndex < GPUREG_BOOLEANCONSTANTS+GPU_BOOLEAN_CONSTANTS/32)
			{
				//
				Trace("Bool const\n");
				bNotFound = false;
				DWORD dwBoolIndex = regIndex-GPUREG_BOOLEANCONSTANTS;
				Assert (dwBoolIndex >= 0 && dwBoolIndex < MAX_ALU_CONST_COUNT_BOOL)
				dwALUConstBool_Offsets[dwBoolIndex] = dwParser_Offset;
			}

			//int константы упакованы (32 штуки)
			if (regIndex >= GPUREG_INTEGERCONSTANTS && regIndex < GPUREG_INTEGERCONSTANTS+GPU_INTEGER_CONSTANTS)
			{
				//
				Trace("Integer const\n");
				bNotFound = false;
			}

			if (regIndex >= GPUREG_FLUSHFETCHCONSTANTS && regIndex < GPUREG_FLUSHFETCHCONSTANTS+3)
			{
				//
				Trace("Flush fetch const [%d] = 0x%08X\n", regIndex-GPUREG_FLUSHFETCHCONSTANTS, regData);
				bNotFound = false;
			}
		}


		if (bNotFound)
		{
			Trace("---- Unknown GPU const !!! [0x%08X]\n", regIndex);
			Assert(false);
		}

		dataStream++;
	}

	Trace("}\n");
}

//Парсер пакетов 3 типа (выполнение GPU операции с параметрами)
//===========================================================================================================================================
void precompiledBatch::ParseType3Packet (DWORD opCode, const DWORD * dataStream, DWORD count)
{
	dwParser_Offset = (((DWORD)dataStream - (DWORD)baseData));
	GPUCOMMANDOP code = (GPUCOMMANDOP)opCode;

	switch (code)
	{
	case GPUCOMMANDOP_NOP:
		{
			break;
		}
	case GPUCOMMANDOP_REG_RMW:
		{
			break;
		}
	case GPUCOMMANDOP_DRAW:
		{
			bool bNotProcessed = true;


			//Рисование неиндексного примитива
			if ((count*sizeof(DWORD)) == sizeof(GPUCOMMAND_DRAW_AUTO))
			{
				bNotProcessed = false;
				Trace ("Draw (DrawPrimitive)\n");
				Trace ("{\n");

				GPUCOMMAND_DRAW_AUTO* drawData = (GPUCOMMAND_DRAW_AUTO*)dataStream;

				Trace ("  VisQueryId = %d\n", drawData->VizQueryId);
				Trace ("  UseVizQuery = %d\n", drawData->UseVizQuery);

				TracePrimitiveType (drawData->PrimType);

				switch (drawData->SrcSelect)
				{
				case GPUINDEXSELECT_DMA:
					Trace ("  SrcSelect = GPUINDEXSELECT_DMA\n");
					break;
				case GPUINDEXSELECT_IMMEDIATE:
					Trace ("  SrcSelect = GPUINDEXSELECT_IMMEDIATE\n");
					break;
				case GPUINDEXSELECT_AUTO:
					Trace ("  SrcSelect = GPUINDEXSELECT_AUTO\n");
					break;
				default:
					Assert(false);
				}

				Trace ("  MajorMode = %d\n", drawData->MajorMode);
				Trace ("  NotEndOfPacket = %d\n", drawData->NotEndOfPacket);
				Trace ("  NumIndices = %d\n", drawData->NumIndices);


				Trace ("}\n");
			} 


			//Рисование индексного примитива
			if ((count*sizeof(DWORD)) == sizeof(GPUCOMMAND_DRAW_INDEX))
			{
				if (bLogOnlyWhenParse == false)
				{
					dwDrawToken_Offset = dwParser_Offset;
				}

				bNotProcessed = false;

				Trace ("Draw (DrawIndexedPrimitive)\n");
				Trace ("{\n");

				GPUCOMMAND_DRAW_INDEX* drawData = (GPUCOMMAND_DRAW_INDEX*)dataStream;

				Trace ("  VisQueryId = %d\n", drawData->VizQueryId);
				Trace ("  UseVizQuery = %d\n", drawData->UseVizQuery);

				TracePrimitiveType (drawData->PrimType);

				Assert (drawData->SrcSelect == GPUINDEXSELECT_DMA);
				Trace ("  SrcSelect = GPUINDEXSELECT_DMA\n");
				Trace ("  MajorMode = %d\n", drawData->MajorMode);

				if (drawData->IndexType == GPUINDEXTYPE_16BIT)
				{
					Trace ("  IndexType = GPUINDEXTYPE_16BIT\n");
				} else
				{
					if (drawData->IndexType == GPUINDEXTYPE_32BIT)
					{
						Trace ("  IndexType = GPUINDEXTYPE_32BIT\n");
					} else
					{
						Assert(false);
					}
				}

				Trace ("  NotEndOfPacket = %d\n", drawData->NotEndOfPacket);
				Trace ("  NumIndices = %d\n", drawData->NumIndices);
				Trace ("  IndexBase = 0x%08X (Effective : 0x%08X)\n", drawData->IndexBase, drawData->IndexBase << 5);
				Trace ("  IndexSize = %d\n", drawData->IndexSize);

				switch(drawData->Endian)
				{
				case GPUENDIAN_NONE:
					Trace ("  Endian = GPUENDIAN_NONE\n");
					break;
				case GPUENDIAN_8IN16:
					Trace ("  Endian = GPUENDIAN_8IN16\n");
					break;
				case GPUENDIAN_8IN32:
					Trace ("  Endian = GPUENDIAN_8IN32\n");
					break;
				case GPUENDIAN_16IN32:
					Trace ("  Endian = GPUENDIAN_16IN32\n");
					break;
				default:
					Assert(false);
				}

				Trace ("}\n");
			}

			//Если DRAW не распознали
			Assert(bNotProcessed == false);


			break;
		}
	case GPUCOMMANDOP_VIZ_QUERY:
		{
			break;
		}
	case GPUCOMMANDOP_SET_STATE:
		{
			break;
		}
	case GPUCOMMANDOP_WAIT_FOR_IDLE:
		{
			break;
		}
	case GPUCOMMANDOP_LOAD_SHADER:
		{
			//Загрузить shader на исполнение

			GPUCOMMAND_LOAD_SHADER* loadShader = (GPUCOMMAND_LOAD_SHADER*)dataStream;

			Trace ("Load shader\n");
			Trace ("{\n");

			//Вершинный шейдер
			if (loadShader->Type == GPULOADTYPE_VERTEX)
			{
				Trace ("  Type = GPULOADTYPE_VERTEX\n");
				dwSetVertexShader_Offset_ = dwParser_Offset;
			} else
			{
				//Пиксельный шейдер
				if (loadShader->Type == GPULOADTYPE_PIXEL)
				{
					Trace ("  Type = GPULOADTYPE_PIXEL\n");
					dwSetPixelShader_Offset_ = dwParser_Offset;
				} else
				{
					//неизвестный шейдер
					Assert(false);
				}
			}

			Trace ("  Adress = 0x%08X (Effective : 0x%08X)\n", loadShader->Address, loadShader->Address << 5);
			Trace ("  Size = %d\n", loadShader->Size);
			Trace ("}\n");

			break;
		}
	case GPUCOMMANDOP_LOAD_SHADER_IMMEDIATE:
		{
			break;
		}
	case GPUCOMMANDOP_SET_CONSTANT:
		{
			break;
		}
	case GPUCOMMANDOP_LOAD_ALU_CONSTANT:
		{
			break;
		}
	case GPUCOMMANDOP_DRAW_IMMEDIATE:
		{
			break;
		}
	case GPUCOMMANDOP_MPEG_INDEX:
		{
			break;
		}
	case GPUCOMMANDOP_INVALIDATE_STATE:
		{
			break;
		}
	case GPUCOMMANDOP_WAIT_REG_MEM:
		{
			GPUCOMMAND_WAIT_REG_MEM* waitRegMem = (GPUCOMMAND_WAIT_REG_MEM*)dataStream;

			Trace ("WaitRegMem\n");
			Trace ("{\n");
			Trace (" addr = 0x%08X\n", waitRegMem->Address);
			Trace (" func = %d\n", waitRegMem->Function);
			Trace (" register = 0x%08X\n", waitRegMem->Register);
			Trace (" waitinterval = %d\n", waitRegMem->WaitInterval);
			Trace (" ref = %d\n", waitRegMem->Reference);
			Trace ("}\n");

			break;
		}
	case GPUCOMMANDOP_MEM_WRITE:
		{
			break;
		}
	case GPUCOMMANDOP_REG_TO_MEM:
		{
			break;
		}
	case GPUCOMMANDOP_INDIRECT_BUFFER :
		{
			break;
		}
	case GPUCOMMANDOP_COND_WRITE:
		{
			break;
		}
	case GPUCOMMANDOP_EVENT_WRITE:
		{
			break;
		}
	case GPUCOMMANDOP_ME_INIT:
		{
			break;
		}
	case GPUCOMMANDOP_FIX_2_FLT_REG:
		{
			break;
		}
	case GPUCOMMANDOP_MEM_WRITE_COUNTER:
		{
			break;
		}
	case GPUCOMMANDOP_WAIT_REG_EQ:
		{
			break;
		}
	case GPUCOMMANDOP_WAIT_REG_GTE:
		{
			break;
		}
	case GPUCOMMANDOP_CPU_INTERRUPT:
		{
			break;
		}
	case GPUCOMMANDOP_EVENT_WRITE_SHADER:
		{
			break;
		}
	case GPUCOMMANDOP_EVENT_WRITE_CACHE_FLUSH:
		{
			break;
		}
	case GPUCOMMANDOP_EVENT_WRITE_SCREEN_EXTENT:
		{
			break;
		}
	case GPUCOMMANDOP_EVENT_WRITE_ZPASS_DONE:
		{
			break;
		}
	case GPUCOMMANDOP_CONTEXT_UPDATE:
		{
			break;
		}
	case GPUCOMMANDOP_SET_BIN_MASK_LO:
		{
			break;
		}
	case GPUCOMMANDOP_SET_BIN_MASK_HI:
		{
			break;
		}
	case GPUCOMMANDOP_SET_BIN_SELECT_LO:
		{
			break;
		}
	case GPUCOMMANDOP_SET_BIN_SELECT_HI:
		{
			break;
		}
	default:
		//Неизвестный тип команды...
		Assert(false);
	}
}

//Вывести в лог, тип примитива
//===========================================================================================================================================
void precompiledBatch::TracePrimitiveType (DWORD dwPrimType)
{
	switch (dwPrimType)
	{
	case GPUPRIMTYPE_NONE:
		Trace ("  PrimeType = GPUPRIMTYPE_NONE\n");
		break;
	case GPUPRIMTYPE_POINTLIST:
		Trace ("  PrimeType = GPUPRIMTYPE_POINTLIST\n");
		break;
	case GPUPRIMTYPE_LINELIST:
		Trace ("  PrimeType = GPUPRIMTYPE_LINELIST\n");
		break;
	case GPUPRIMTYPE_LINESTRIP:
		Trace ("  PrimeType = GPUPRIMTYPE_LINESTRIP\n");
		break;
	case GPUPRIMTYPE_TRILIST:
		Trace ("  PrimeType = GPUPRIMTYPE_TRILIST\n");
		break;
	case GPUPRIMTYPE_TRIFAN:
		Trace ("  PrimeType = GPUPRIMTYPE_TRIFAN\n");
		break;
	case GPUPRIMTYPE_TRISTRIP:
		Trace ("  PrimeType = GPUPRIMTYPE_TRISTRIP\n");
		break;
	case GPUPRIMTYPE_TRI_WITH_WFLAGS:
		Trace ("  PrimeType = GPUPRIMTYPE_TRI_WITH_WFLAGS\n");
		break;
	case GPUPRIMTYPE_RECTLIST:
		Trace ("  PrimeType = GPUPRIMTYPE_RECTLIST\n");
		break;
	case GPUPRIMTYPE_LINELOOP:
		Trace ("  PrimeType = GPUPRIMTYPE_LINELOOP\n");
		break;
	case GPUPRIMTYPE_QUADLIST:
		Trace ("  PrimeType = GPUPRIMTYPE_QUADLIST\n");
		break;
	case GPUPRIMTYPE_QUADSTRIP:
		Trace ("  PrimeType = GPUPRIMTYPE_QUADSTRIP\n");
		break;
	case GPUPRIMTYPE_POLYGON:
		Trace ("  PrimeType = GPUPRIMTYPE_POLYGON\n");
		break;
	case GPUPRIMTYPE_2D_COPY_RECT_LIST_V0:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_COPY_RECT_LIST_V0\n");
		break;
	case GPUPRIMTYPE_2D_COPY_RECT_LIST_V1:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_COPY_RECT_LIST_V1\n");
		break;
	case GPUPRIMTYPE_2D_COPY_RECT_LIST_V2:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_COPY_RECT_LIST_V2\n");
		break;
	case GPUPRIMTYPE_2D_COPY_RECT_LIST_V3:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_COPY_RECT_LIST_V3\n");
		break;
	case GPUPRIMTYPE_2D_FILL_RECT_LIST:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_FILL_RECT_LIST\n");
		break;
	case GPUPRIMTYPE_2D_LINE_STRIP:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_LINE_STRIP\n");
		break;
	case GPUPRIMTYPE_2D_TRI_STRIP:
		Trace ("  PrimeType = GPUPRIMTYPE_2D_TRI_STRIP\n");
		break;
	default:
		//Неизвестный тип примитива
		Assert(false);
	}
}


void precompiledBatch::debugOutCommandBuffer(BYTE * bDataPtr, bool bDumpToLog)
{
	bGlobalDump = bDumpToLog;
	bLogOnlyWhenParse = true;

	parseCommandBuffer(bDataPtr);
}

DWORD precompiledBatch::getBytesAllocated()
{
	return dwBytesAllocated;
}

void precompiledBatch::offlineCreate (D3DCommandBuffer * cmdPrecompiledBuffer, D3DVertexShader* vertexShaderToCreateFixup, D3DPixelShader* pixelShaderToCreateFixup, bool bDumpToLog)
{
	sourcePrecompiledBuffer = cmdPrecompiledBuffer;
	sourcePrecompiledBuffer->AddRef();

	bLogOnlyWhenParse = false;
	bGlobalDump = bDumpToLog;


	cmdPrecompiledBuffer->BeginFixupCreation();
	{

		if (vertexShaderToCreateFixup)
		{
			dwVSO_Temp = cmdPrecompiledBuffer->CreateVertexShaderFixup(D3DFIXUP_DYNAMIC, vertexShaderToCreateFixup, 0, 0);
		} else
		{
			dwVSO_Temp = GPU_INVALID_OFFSET;
		}

		if (pixelShaderToCreateFixup)
		{
			dwPSO_Temp = cmdPrecompiledBuffer->CreatePixelShaderFixup(D3DFIXUP_DYNAMIC, pixelShaderToCreateFixup, 0, 0);
		} else
		{
			dwPSO_Temp = GPU_INVALID_OFFSET;
		}
	}
	cmdPrecompiledBuffer->EndFixupCreation();



	dwHeaderSize_temp = 0;
	dwCommandBufferDataSize = 0;
	dwInitializationSize_temp = 0;
	cmdPrecompiledBuffer->Deconstruct( 0, NULL, &dwHeaderSize_temp, NULL, &dwCommandBufferDataSize, NULL, &dwInitializationSize_temp );

	//Хвост учитываем, т.к. будем добавлять токены...
	dwCommandBufferDataSize = dwCommandBufferDataSize + sizeof (GPUCOMMAND_DRAW_INDEX) * 2;
	
	pHeaderBytes_temp = ( BYTE* )malloc( dwHeaderSize_temp );
	pInitializationBytes_temp = ( BYTE* )malloc( dwInitializationSize_temp );
	dwCommandBufferData = ( BYTE* )XPhysicalAlloc( dwCommandBufferDataSize, MAXULONG_PTR, 4096, PAGE_READWRITE | PAGE_WRITECOMBINE );
	
	
	Assert( pHeaderBytes_temp != NULL && dwCommandBufferData != NULL && pInitializationBytes_temp != NULL );

	dwBytesAllocated += dwHeaderSize_temp;
	dwBytesAllocated += dwInitializationSize_temp;
	dwBytesAllocated += (dwCommandBufferDataSize + 4095 & ~4095);


	cmdPrecompiledBuffer->Deconstruct( 0, ( D3DCommandBuffer* )pHeaderBytes_temp, &dwHeaderSize_temp, dwCommandBufferData, &dwCommandBufferDataSize, pInitializationBytes_temp, &dwInitializationSize_temp);

	

	commandBuffer_temp = (D3DCommandBuffer*)pHeaderBytes_temp;
	commandBuffer_temp->BeginReconstruction( 0, dwCommandBufferData, pInitializationBytes_temp);
	commandBuffer_temp->EndReconstruction();

	parseCommandBuffer(dwCommandBufferData);

	Assert(dwSetVertexShader_Offset_ != GPU_INVALID_OFFSET);
	//Assert(dwSetPixelShaderOffset != GPU_INVALID_OFFSET);
	Assert(dwDrawToken_Offset != GPU_INVALID_OFFSET);
	Assert(dwEndCommandBuffer_Offset != GPU_INVALID_OFFSET);

	//Проверяем, что draw пакет последний в command buffer
	Assert(dwDrawToken_Offset+sizeof(GPUCOMMAND_DRAW_INDEX) == dwEndCommandBuffer_Offset);


	Assert(dwDepthControl_Offset != GPU_INVALID_OFFSET);
	Assert(dwColorMask_Offset != GPU_INVALID_OFFSET);


	Assert (dwIndxOffset_Offset != GPU_INVALID_OFFSET);



	bGlobalDump = false;
}



void precompiledBatch::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, DWORD dwOffsetInBytes)
{
	Assert(StreamNumber >= 0 && StreamNumber < MAX_STREAMS_COUNT);
	DWORD dwOffset = dwStreamSource_Offsets[StreamNumber];
	if (dwOffset == GPU_INVALID_OFFSET)
	{
		return;
	}

	if (pStreamData == NULL)
	{
		return;
	}

	//Кратность 4-ке проверяем
	Assert((dwOffsetInBytes & 0x3) == 0);

	GPUVERTEX_FETCH_CONSTANT * streamDataConst = (GPUVERTEX_FETCH_CONSTANT*)(dwCommandBufferData + dwOffset);

	*streamDataConst = pStreamData->Format;
	streamDataConst->BaseAddress = (GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST char*)((pStreamData->Format.BaseAddress << 2))+dwOffsetInBytes) >> 2);
}


void precompiledBatch::SetDrawParameters (GPUPRIMTYPE primType, DWORD trianglesCount, DWORD dwStartVertex)
{
	Assert(bTemp_VertexShaderNotSet == false);
/*
	Assert (trianglesCount <= 65532);
*/
	
	


	dpPacket.PrimType = primType;


	//start vertex учитываем...
	DWORD * indxOffsetPtr = (DWORD *)(dwCommandBufferData + dwIndxOffset_Offset);
	*indxOffsetPtr = dwStartVertex;

	//Считаем количество индексов
	const UINT * ptParams = D3DPRIMITIVEVERTEXCOUNT[primType];
	DWORD dwNumIndices = (ptParams[0] * trianglesCount) + ptParams[1];


	Assert (dwNumIndices <= 65532);

	BYTE * bufferData = (dwCommandBufferData + dwDrawToken_Offset-sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3));

	//настраиваем batch
	((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
	bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

	dpPacket.NumIndices = dwNumIndices;
	*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;

	//Оригинальный размер (dp на 2 dword меньше dip)
	dwCurrentSizeInDwords = (dwSizeInDwords - 2);

/*
	if (trianglesCount <= 21844)
	{
		//настраиваем batch
		((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
		bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

		dpPacket.NumIndices = dwNumIndices;
		*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;

		//Оригинальный размер (dp на 2 dword меньше dip)
		dwCurrentSizeInDwords = (dwSizeInDwords - 2);
	} else
	{
		if (trianglesCount <= 43688)
		{
			//настраиваем первый batch
			//-----------------------------------------------
			((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
			bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

			dpPacket.NumIndices = 65532;
			*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;
			bufferData += sizeof(GPUCOMMAND_DRAW_AUTO);


			//настраиваем второй batch (хвост)
			//-----------------------------------------------
			dwNumIndices -= 65532;

			((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
			bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

			dpPacket.NumIndices = dwNumIndices;
			*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;

			//добавили 1 пакет - 2 dword'а + 1 dword header...
			//-----------------------------------------------
			DWORD dwAdditionalSizeInDwords = (sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3) + sizeof(GPUCOMMAND_DRAW_AUTO)) / sizeof(DWORD);
			dwCurrentSizeInDwords = dwSizeInDwords-2+dwAdditionalSizeInDwords;


		} else
		{
			if (trianglesCount <= 65532)
			{
				//настраиваем первый batch
				//-----------------------------------------------
				((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
				bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

				dpPacket.NumIndices = 65532;
				*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;
				bufferData += sizeof(GPUCOMMAND_DRAW_AUTO);


				//настраиваем второй batch
				//-----------------------------------------------
				((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
				bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

				dpPacket.NumIndices = 65532;
				*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;
				bufferData += sizeof(GPUCOMMAND_DRAW_AUTO);

				//настраиваем третий batch (хвост)
				//-----------------------------------------------
				dwNumIndices -= (65532*2);

				((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dpPacketHeader.dword;
				bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

				dpPacket.NumIndices = dwNumIndices;
				*((GPUCOMMAND_DRAW_AUTO*)bufferData) = dpPacket;
				bufferData += sizeof(GPUCOMMAND_DRAW_AUTO);

				//добавили 2 пакета - 8 dword's + 2 dword header
				//-----------------------------------------------
				DWORD dwAdditionalSizeInDwords = (sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3) + sizeof(GPUCOMMAND_DRAW_AUTO)+sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3) + sizeof(GPUCOMMAND_DRAW_AUTO)) / sizeof(DWORD);
				dwCurrentSizeInDwords = dwSizeInDwords-2+dwAdditionalSizeInDwords;

			} else
			{
				Assert(false && "Need todo 4 batches...");
			}
		}
	}
*/
}

void precompiledBatch::SetDrawIndexedParameters (GPUPRIMTYPE primType, IDirect3DIndexBuffer9 *pIndexData, DWORD trianglesCount, dword StartIndex)
{
	Assert(primType == GPUPRIMTYPE_TRILIST);
	Assert(bTemp_VertexShaderNotSet == false);

	Assert (trianglesCount <= 65532);
	if (pIndexData == NULL)
	{
		return;
	}

#ifdef _DEBUG
	D3DINDEXBUFFER_DESC desc;
	pIndexData->GetDesc(&desc);
	Assert (desc.Format == D3DFMT_INDEX16);
#endif


	dipPacket.PrimType = primType;

	DWORD dwOffset = StartIndex * sizeof(WORD);
	DWORD dwNumIndices = (trianglesCount * 3);
	BYTE * bufferData = (dwCommandBufferData + dwDrawToken_Offset-sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3));

	if (trianglesCount <= 21844)
	{
		//настраиваем batch
		((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dipPacketHeader.dword;
		bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

		dipPacket.NumIndices = dwNumIndices;
		dipPacket.IndexSize = dwNumIndices;
		dipPacket.IndexBase = GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(dwOffset + pIndexData->Address));

		*((GPUCOMMAND_DRAW_INDEX*)bufferData) = dipPacket;

		//Оригинальный размер
		dwCurrentSizeInDwords = dwSizeInDwords;
	} else
	{
		if (trianglesCount <= 43688)
		{
			//настраиваем первый batch
			//-----------------------------------------------
			((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dipPacketHeader.dword;
			bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

			dipPacket.NumIndices = 65532;
			dipPacket.IndexSize = 65532;
			dipPacket.IndexBase = GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(dwOffset + pIndexData->Address));

			*((GPUCOMMAND_DRAW_INDEX*)bufferData) = dipPacket;
			bufferData += sizeof(GPUCOMMAND_DRAW_INDEX);


			//настраиваем второй batch (хвост)
			//-----------------------------------------------
			dwNumIndices -= 65532;

			((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dipPacketHeader.dword;
			bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

			dipPacket.NumIndices = dwNumIndices;
			dipPacket.IndexSize = dwNumIndices;
			dipPacket.IndexBase = GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(dwOffset + pIndexData->Address + (65532 * sizeof(WORD))));

			*((GPUCOMMAND_DRAW_INDEX*)bufferData) = dipPacket;
			bufferData += sizeof(GPUCOMMAND_DRAW_INDEX);

			//добавили 1 пакет - 4 dword'а + 1 dword header...
			//-----------------------------------------------
			DWORD dwAdditionalSizeInDwords = (sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3)+sizeof(GPUCOMMAND_DRAW_INDEX)) / sizeof(DWORD);
			dwCurrentSizeInDwords = dwSizeInDwords+dwAdditionalSizeInDwords;

		} else
		{
			if (trianglesCount <= 65532)
			{
				//настраиваем первый batch
				//-----------------------------------------------
				((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dipPacketHeader.dword;
				bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

				dipPacket.NumIndices = 65532;
				dipPacket.IndexSize = 65532;
				dipPacket.IndexBase = GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(dwOffset + pIndexData->Address));

				*((GPUCOMMAND_DRAW_INDEX*)bufferData) = dipPacket;
				bufferData += sizeof(GPUCOMMAND_DRAW_INDEX);

				//настраиваем второй batch
				//-----------------------------------------------
				((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dipPacketHeader.dword;
				bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);

				dipPacket.NumIndices = 65532;
				dipPacket.IndexSize = 65532;
				dipPacket.IndexBase = GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(dwOffset + pIndexData->Address + (65532 * sizeof(WORD))));

				*((GPUCOMMAND_DRAW_INDEX*)bufferData) = dipPacket;
				bufferData += sizeof(GPUCOMMAND_DRAW_INDEX);

				//настраиваем третий batch (хвост)
				//-----------------------------------------------
				dwNumIndices -= (65532*2);

				((GPUCOMMANDBUFFER_HEADER_PACKET3*)bufferData)->dword = dipPacketHeader.dword;
				bufferData += sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3);
				
				dipPacket.NumIndices = dwNumIndices;
				dipPacket.IndexSize = dwNumIndices;
				dipPacket.IndexBase = GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(dwOffset + pIndexData->Address + ((65532 * 2) * sizeof(WORD))));

				*((GPUCOMMAND_DRAW_INDEX*)bufferData) = dipPacket;

				//добавили 2 пакета - 8 dword's + 2 dword header
				//-----------------------------------------------
				DWORD dwAdditionalSizeInDwords = (sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3)+sizeof(GPUCOMMAND_DRAW_INDEX)+sizeof(GPUCOMMANDBUFFER_HEADER_PACKET3)+sizeof(GPUCOMMAND_DRAW_INDEX)) / sizeof(DWORD);
				dwCurrentSizeInDwords = dwSizeInDwords+dwAdditionalSizeInDwords;

			} else
			{
				Assert(false && "Need todo 4 batches...");
			}
		}
	}
}

/*
void precompiledBatch::tempFixupVertexShaderAndPixelShader()
{
	//надо эти проставить через стандартные
	if (dwVSO_Temp != GPU_INVALID_OFFSET)
	{
		commandBuffer_temp->BeginDynamicFixups();
		commandBuffer_temp->SetVertexShader(dwVSO_Temp, m_pVertexShader);

		if (dwPSO_Temp != GPU_INVALID_OFFSET)
		{
			commandBuffer_temp->SetPixelShader(dwPSO_Temp, m_pPixelShader);
		}

		commandBuffer_temp->EndDynamicFixups();
	} else
	{
		Assert(false);
	}



	//SetVertexShader(m_pVertexShader);
	//SetPixelShader(m_pPixelShader);


	int a = 0;

}
*/

void precompiledBatch::Freeze()
{
	Assert (dwCommandBufferData);
	XPhysicalProtect(dwCommandBufferData, dwCommandBufferDataSize, PAGE_READWRITE);
}

void precompiledBatch::SetVertexShaderPtr (IDirect3DVertexShader9 *pVertexShader)
{
	if (m_pVertexShader)
	{
		m_pVertexShader->Release();
		m_pVertexShader = NULL;
	}

	m_pVertexShader = pVertexShader;
	if (m_pVertexShader)
	{
		m_pVertexShader->AddRef();
	}

	Assert (pVertexShader->IsBound());

/*
	GPUCOMMAND_LOAD_SHADER* data = GPUGetVertexShaderPhysicalAddress(pVertexShader);

	GPUCOMMAND_LOAD_SHADER * shaderLoad = (GPUCOMMAND_LOAD_SHADER*)(dwCommandBufferData + dwSetVertexShader_Offset_);
	Assert (shaderLoad->Type == GPULOADTYPE_VERTEX);

	shaderLoad->Address = 2 + (GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(data->Address << 5)) >> 5);
	shaderLoad->Size = data->Size;
*/

	if (dwVSO_Temp != GPU_INVALID_OFFSET)
	{
		commandBuffer_temp->BeginDynamicFixups();
		commandBuffer_temp->SetVertexShader(dwVSO_Temp, m_pVertexShader);
		commandBuffer_temp->EndDynamicFixups();
	}

	bTemp_VertexShaderNotSet = false;
	
}

void precompiledBatch::SetPixelShaderPtr (IDirect3DPixelShader9 *pPixelShader)
{
	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
		m_pPixelShader = NULL;
	}

	m_pPixelShader = pPixelShader;
	if (m_pPixelShader)
	{
		m_pPixelShader->AddRef();
	}


	if (dwSetPixelShader_Offset_ == GPU_INVALID_OFFSET)
	{
		return;
	}


	
/*
	GPUCOMMAND_LOAD_SHADER* data = GPUGetPixelShaderPhysicalAddress(pPixelShader);

	GPUCOMMAND_LOAD_SHADER * shaderLoad = (GPUCOMMAND_LOAD_SHADER*)(dwCommandBufferData + dwSetPixelShader_Offset_);
	Assert (shaderLoad->Type == GPULOADTYPE_PIXEL);

	//Assert (data->Type == GPULOADTYPE_PIXEL);

	shaderLoad->Address = 2 + (GPU_CONVERT_CPU_TO_GPU_ADDRESS((CONST void*)(data->Address << 5)) >> 5);
	shaderLoad->Size = data->Size;
*/
	
	if (dwPSO_Temp != GPU_INVALID_OFFSET)
	{
		commandBuffer_temp->BeginDynamicFixups();
		commandBuffer_temp->SetPixelShader(dwPSO_Temp, m_pPixelShader);
		commandBuffer_temp->EndDynamicFixups();
	}


	bTemp_PixelShaderNotSet = false;
}





void precompiledBatch::SetCullMode (GPUCULL cullMode)
{
	if (dwSetCullMode_Offset == GPU_INVALID_OFFSET)
	{
		return;
	}

	GPU_MODECONTROL * modeControlReg = (GPU_MODECONTROL *)(dwCommandBufferData + dwSetCullMode_Offset);
	modeControlReg->CullMode = cullMode;
}


void precompiledBatch::AddTextureSampler (const char * szName, DWORD dwSamplerIdx, D3DTEXTUREFILTERTYPE minFilter, D3DTEXTUREFILTERTYPE magFilter, D3DTEXTUREFILTERTYPE mipFilter, D3DTEXTUREADDRESS clampU, D3DTEXTUREADDRESS clampV)
{
	Assert(dwSamplerIdx >= 0 && dwSamplerIdx < MAX_TEXTURES_COUNT);
	DWORD dwOffset = dwTexture_Offsets[dwSamplerIdx];
	if (dwOffset == GPU_INVALID_OFFSET)
	{
		return;
	}

	textureSampler & sampler_const = tex_consts[tex_consts.Add()];
	sampler_const.fixupPtrInCommandBuffer = (dwCommandBufferData + dwOffset);
	sampler_const.dwStage = dwSamplerIdx;
	sampler_const.tex = m_pDB->getTextureOutput(szName);
	sampler_const.clampU = (GPUCLAMP)clampU;
	sampler_const.clampV = (GPUCLAMP)clampV;
	sampler_const.minFilter = (GPUMINMAGFILTER)minFilter;
	sampler_const.magFilter = (GPUMINMAGFILTER)magFilter;
	sampler_const.mipFilter = (GPUMIPFILTER)mipFilter;


	Assert (sampler_const.tex);
}

void precompiledBatch::AddVertexShaderConstVector (const char * szName, DWORD dwIndex, DWORD dwRegCount)
{
	Assert(dwIndex >= 0 && dwIndex < MAX_ALU_CONST_COUNT);
	Assert((dwIndex+dwRegCount-1) >= 0 && (dwIndex+dwRegCount-1) < MAX_ALU_CONST_COUNT);

	DWORD dwOffset = dwALUConst_Offsets[dwIndex];
	for (DWORD i = 0; i < dwRegCount; i++)
	{
		Assert (dwALUConst_Offsets[dwIndex+i] != GPU_INVALID_OFFSET)
	}

	Assert(dwRegCount <= 256);

	constVector4 & vs_const = vs_consts_v4[vs_consts_v4.Add()];
	vs_const.fixupPtrInCommandBuffer = (dwCommandBufferData + dwOffset);
	vs_const.dwRegCount = dwRegCount;
	vs_const.dwStartIdx = dwIndex;
	//vs_const.tempDBName = szName;
	vs_const.sourceData = m_pDB->getVectorOutput(szName);
	Assert (vs_const.sourceData);
}


void precompiledBatch::AddPixelShaderConstVector (const char * szName, DWORD dwIndex, DWORD dwRegCount)
{
	dwIndex += 256;
	Assert(dwIndex >= 0 && dwIndex < MAX_ALU_CONST_COUNT);
	Assert((dwIndex+dwRegCount-1) >= 0 && (dwIndex+dwRegCount-1) < MAX_ALU_CONST_COUNT);

	DWORD dwOffset = dwALUConst_Offsets[dwIndex];
	for (DWORD i = 0; i < dwRegCount; i++)
	{
		Assert (dwALUConst_Offsets[dwIndex+i] != GPU_INVALID_OFFSET)
	}

	Assert(dwRegCount <= 256);

	constVector4 & ps_const = ps_consts_v4[ps_consts_v4.Add()];
	ps_const.fixupPtrInCommandBuffer = (dwCommandBufferData + dwOffset);
	ps_const.dwRegCount = dwRegCount;
	ps_const.dwStartIdx = dwIndex;
	//ps_const.tempDBName = szName;
	ps_const.sourceData = m_pDB->getVectorOutput(szName);
	Assert (ps_const.sourceData);
}


void precompiledBatch::AddVertexShaderConstBool (const char * szName, DWORD dwIndex, DWORD dwRegCount)
{
/*
	if (dwRegCount != 32)
	{
		//bool ставятся не пачками по 32 - ошибка!!!
		api->Trace("ERROR : Bool registers must sets by chunks of 32 elements!!!");
		Assert(false);
	}
*/

	if ((dwIndex % 32) != 0)
	{
		//регистры bool начинаются не кратно 32 - ошибка!!!
		api->Trace("ERROR : Bool registers address must be 32 bools aligned!!!");
		Assert(false);
	}

	dwIndex = dwIndex >> 5;
	Assert(dwIndex >= 0 && dwIndex < MAX_ALU_CONST_COUNT_BOOL);

	DWORD dwOffset = dwALUConstBool_Offsets[dwIndex];
	constBool & vs_const_b = vs_consts_b[vs_consts_b.Add()];
	vs_const_b.fixupPtrInCommandBuffer = (dwCommandBufferData + dwOffset);
	vs_const_b.dwRegCount = 1;
	vs_const_b.sourceData = m_pDB->getBool32Output(szName);
	Assert (vs_const_b.sourceData);
}

void precompiledBatch::AddPixelShaderConstBool (const char * szName, DWORD dwIndex, DWORD dwRegCount)
{
/*
	if (dwRegCount != 32)
	{
		//bool ставятся не пачками по 32 - ошибка!!!
		api->Trace("ERROR : Bool registers must sets by chunks of 32 elements!!!");
		Assert(false);
	}
*/

	if ((dwIndex % 32) != 0)
	{
		//регистры bool начинаются не кратно 32 - ошибка!!!
		api->Trace("ERROR : Bool registers address must be 32 bools aligned!!!");
		Assert(false);
	}

	dwIndex = dwIndex >> 5;
	dwIndex += 4;
	Assert(dwIndex >= 0 && dwIndex < MAX_ALU_CONST_COUNT_BOOL);

	DWORD dwOffset = dwALUConstBool_Offsets[dwIndex];
	constBool & ps_const_b = ps_consts_b[ps_consts_b.Add()];
	ps_const_b.fixupPtrInCommandBuffer = (dwCommandBufferData + dwOffset);
	ps_const_b.dwRegCount = 1;
	ps_const_b.sourceData = m_pDB->getBool32Output(szName);
	Assert (ps_const_b.sourceData);
}


/*
void precompiledBatch::AddVertexShaderConst (const char * szName, DWORD dwIndex, DWORD dwRegCount, CommandBufferCompiler::ConstantType type)
{
	Assert(dwIndex >= 0 && dwIndex < MAX_ALU_CONST_COUNT);
	Assert((dwIndex+dwRegCount-1) >= 0 && (dwIndex+dwRegCount-1) < MAX_ALU_CONST_COUNT);

	DWORD dwOffset = dwALUConst_Offsets[dwIndex];
	for (DWORD i = 0; i < dwRegCount; i++)
	{
		Assert (dwALUConst_Offsets[dwIndex+i] != GPU_INVALID_OFFSET)
	}
	
	switch (type)
	{
	case CommandBufferCompiler::CT_BOOL:
		{
			constBool & vs_const_b = vs_consts_b[vs_consts_b.Add()];
			vs_const_b.dwOffsetInCB = dwOffset;
			vs_const_b.dwRegCount = dwRegCount;
			vs_const_b.sourceData = m_pDB->getBoolOutput(szName);
			Assert (vs_const_b.sourceData);
		}
	case CommandBufferCompiler::CT_FLOAT:
	case CommandBufferCompiler::CT_FLOAT3:
	case CommandBufferCompiler::CT_FLOAT4:
	case CommandBufferCompiler::CT_MATRIX4x4:
		{
			constVector4 & vs_const = vs_consts_v4[vs_consts_v4.Add()];
			vs_const.dwOffsetInCB = dwOffset;
			vs_const.dwRegCount = dwRegCount;
			vs_const.sourceData = m_pDB->getVectorOutput(szName);
			Assert (vs_const.sourceData);
			break;
		}
	default:
		Assert(false);
	}
}

void precompiledBatch::AddPixelShaderConst (const char * szName, DWORD dwIndex, DWORD dwRegCount, CommandBufferCompiler::ConstantType type)
{
	dwIndex += 256;
	Assert(dwIndex >= 0 && dwIndex < MAX_ALU_CONST_COUNT);
	Assert((dwIndex+dwRegCount-1) >= 0 && (dwIndex+dwRegCount-1) < MAX_ALU_CONST_COUNT);

	DWORD dwOffset = dwALUConst_Offsets[dwIndex];
	for (DWORD i = 0; i < dwRegCount; i++)
	{
		Assert (dwALUConst_Offsets[dwIndex+i] != GPU_INVALID_OFFSET)
	}

	switch (type)
	{
	case CommandBufferCompiler::CT_BOOL:
		{
			constBool & ps_const_b = ps_consts_b[ps_consts_b.Add()];
			ps_const_b.dwOffsetInCB = dwOffset;
			ps_const_b.dwRegCount = dwRegCount;
			ps_const_b.sourceData = m_pDB->getBoolOutput(szName);
			Assert (ps_const_b.sourceData);
		}
	case CommandBufferCompiler::CT_FLOAT:
	case CommandBufferCompiler::CT_FLOAT3:
	case CommandBufferCompiler::CT_FLOAT4:
	case CommandBufferCompiler::CT_MATRIX4x4:
		{
			constVector4 & ps_const = ps_consts_v4[ps_consts_v4.Add()];
			ps_const.dwOffsetInCB = dwOffset;
			ps_const.dwRegCount = dwRegCount;
			ps_const.sourceData = m_pDB->getVectorOutput(szName);
			Assert (ps_const.sourceData);
			break;
		}
	default:
		Assert(false);
	}

}

*/


void precompiledBatch::SetColorMask (DWORD dwMask)
{
	if (dwColorMask_Offset == GPU_INVALID_OFFSET)
	{
		return;
	}

	GPU_COLORMASK * colorMaskControl = (GPU_COLORMASK *)(dwCommandBufferData + dwColorMask_Offset);
	colorMaskControl->Write0 = (dwMask & 0x0F);
	colorMaskControl->Write1 = 0;
	colorMaskControl->Write2 = 0;
	colorMaskControl->Write3 = 0;
}

void precompiledBatch::SetBlendOp (GPUBLEND dwSrcBlendOp, GPUBLEND dwDestBlendOp)
{
	if (dwBlendControl0_Offset == GPU_INVALID_OFFSET)
	{
		return;
	}

	GPU_BLENDCONTROL * blendPacket = (GPU_BLENDCONTROL *)(dwCommandBufferData + dwBlendControl0_Offset);
	blendPacket->ColorDestBlend = dwDestBlendOp;
	blendPacket->AlphaDestBlend = dwDestBlendOp;
	blendPacket->ColorSrcBlend = dwSrcBlendOp;
	blendPacket->AlphaSrcBlend = dwSrcBlendOp;


}

void precompiledBatch::SetHyperZEnable (bool bHyperZEnable, bool bHyperZWriteEnable)
{
	bHiZEnable = bHyperZEnable;
	bHiZWriteEnable = bHyperZWriteEnable;

	GPU_HICONTROL * hiZControl = (GPU_HICONTROL *)(dwCommandBufferData + dwHiZControl_Offset);
	hiZControl->HiZWriteEnable = bHyperZWriteEnable;
	hiZControl->HiZEnable = bHyperZEnable;
	hiZControl->HiZFunc = GPUHIZFUNC_LESS_EQUAL;
	hiZControl->HiBaseAddr = 0x0;

	hiZControl->HiStencilEnable = false;
	hiZControl->HiStencilWriteEnable = false;
	hiZControl->HiStencilFunc = GPUHISTENCILFUNC_EQUAL;
	hiZControl->HiStencilRef = 0x0;
}

void precompiledBatch::SetHyperZWriteEnable (bool bHyperZWriteEnable)
{
	GPU_HICONTROL * hiZControl = (GPU_HICONTROL *)(dwCommandBufferData + dwHiZControl_Offset);
	hiZControl->HiZWriteEnable = bHyperZWriteEnable;
}

void precompiledBatch::SetZWriteEnable(bool bDepthWriteEnabled)
{
	GPU_DEPTHCONTROL * depthControl = (GPU_DEPTHCONTROL *)(dwCommandBufferData + dwDepthControl_Offset);
	depthControl->ZWriteEnable = bDepthWriteEnabled;
}

void precompiledBatch::SetZEnable(bool bDepthEnabled, bool bDepthWriteEnabled)
{
	bDepthEnable = bDepthEnabled;
	bDepthWriteEnable = bDepthWriteEnabled;

	bNeedFlushHiperZ = !bDepthEnabled;

	GPU_DEPTHCONTROL * depthControl = (GPU_DEPTHCONTROL *)(dwCommandBufferData + dwDepthControl_Offset);
	depthControl->ZEnable = bDepthEnabled;
	depthControl->ZWriteEnable = bDepthWriteEnabled;
}

void precompiledBatch::SetVariablesDatabase(VariablesDB * pDB)
{
	m_pDB = pDB;
}

void precompiledBatch::UpdateVariablesFromDB()
{
	NGRender::RenderStat & stats = NGRender::pRS->renderStats();

	stats.dwUpdatesCount++;

	DWORD dwPsConstSize = ps_consts_v4.Size();

	//потому что последняя константа фейк, для префетча
	dwPsConstSize = dwPsConstSize - 1;
	constVector4 * __restrict pPsConstData = ps_consts_v4.GetBuffer();
	constVector4 * __restrict pPsConstPrefetchData = ps_consts_v4.GetBuffer();
	pPsConstPrefetchData++;

	for (DWORD i = 0; i < dwPsConstSize; i++)
	{
		stats.dwConstBytes += (pPsConstData->dwRegCount * sizeof(Vector4));
		stats.dwConstCount += pPsConstData->dwRegCount;
		Assert(stats.dwConstCount <= 92000);

		__dcbt(0, pPsConstPrefetchData->sourceData);

		SetAluConstVector4(pPsConstData->fixupPtrInCommandBuffer, pPsConstData->sourceData, pPsConstData->dwRegCount);
		pPsConstData++;
		pPsConstPrefetchData++;
	}


	
	DWORD dwPsConstSizeB = ps_consts_b.Size();
	constBool * __restrict pPsConstDataB = ps_consts_b.GetBuffer();
	for (DWORD i = 0; i < dwPsConstSizeB; i++)
	{
		stats.dwConstBytes += sizeof(DWORD);
		stats.dwConstCount++;

		Assert(stats.dwConstCount <= 92000);

		SetAluConstsBool32(pPsConstDataB->fixupPtrInCommandBuffer, pPsConstDataB->sourceData->value);
		pPsConstDataB++;
	}


	DWORD dwVsConstSize = vs_consts_v4.Size();
	//потому что последняя константа фейк, для префетча
	dwVsConstSize = dwVsConstSize - 1;

	constVector4 * __restrict pVsConstData = vs_consts_v4.GetBuffer();
	constVector4 * __restrict pVsConstPrefetchData = vs_consts_v4.GetBuffer();
	pVsConstPrefetchData++;


	for (DWORD i = 0; i < dwVsConstSize; i++)
	{
		stats.dwConstBytes += (pVsConstData->dwRegCount * sizeof(Vector4));
		stats.dwConstCount += pVsConstData->dwRegCount;
		Assert(stats.dwConstCount <= 92000);

		__dcbt(0, pVsConstPrefetchData->sourceData);

		SetAluConstVector4(pVsConstData->fixupPtrInCommandBuffer, pVsConstData->sourceData, pVsConstData->dwRegCount);
		pVsConstData++;
		pVsConstPrefetchData++;

	}

	DWORD dwVsConstSizeB = vs_consts_b.Size();
	constBool * __restrict pVsConstDataB = vs_consts_b.GetBuffer();
	for (DWORD i = 0; i < dwVsConstSizeB; i++)
	{
		stats.dwConstBytes += sizeof(DWORD);
		stats.dwConstCount++;

		Assert(stats.dwConstCount <= 92000);

		SetAluConstsBool32(pVsConstDataB->fixupPtrInCommandBuffer, pVsConstDataB->sourceData->value);
		pVsConstDataB++;

	}

	DWORD dwTexConstSize = tex_consts.Size();
	textureSampler * __restrict pTexConstData = tex_consts.GetBuffer();

	for (DWORD i = 0; i < dwTexConstSize; i++)
	{
		stats.dwConstBytes += sizeof(GPUTEXTURE_FETCH_CONSTANT) + (sizeof(DWORD) * 2);

		SetTexture(pTexConstData->fixupPtrInCommandBuffer, *pTexConstData->tex, pTexConstData->minFilter, pTexConstData->magFilter, pTexConstData->mipFilter, pTexConstData->clampU, pTexConstData->clampV);
		pTexConstData++;
	}
}


DWORD precompiledBatch::getPhysicalSize()
{
	return dwCommandBufferDataSize;
}
BYTE* precompiledBatch::getPhysicalBytes()
{
	return dwCommandBufferData;
}

DWORD precompiledBatch::getHeaderSize()
{
	return dwHeaderSize_temp;

}
BYTE* precompiledBatch::getHeaderBytes()
{
	return pHeaderBytes_temp;
}

DWORD precompiledBatch::getInitializationSize()
{
	return dwInitializationSize_temp;
}

BYTE* precompiledBatch::getInitializationBytes()
{
	return pInitializationBytes_temp;
}

DWORD precompiledBatch::getSizeInDwords()
{
	return dwCurrentSizeInDwords;
}



void precompiledBatch::DynamicDraw_SetVertexShader(IDirect3DVertexShader9 *pBoundedVS)
{
	pDynamicDrawVS = pBoundedVS;
}

void precompiledBatch::DynamicDraw_SetPixelShader(IDirect3DPixelShader9 *pPS)
{
	pDynamicDrawPS = pPS;
}

void precompiledBatch::DynamicDraw_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	if (State == D3DRS_ZENABLE)
	{
		bNeedFlushHiperZ = !Value;
	}

	if (State == D3DRS_CULLMODE)
	{
		originalGpuCull = (GPUCULL)Value;
	}

	dynamicDrawRState * state = NULL;
	for (DWORD i = 0; i < dynamicDrawRS.Size(); i++)
	{
		if(dynamicDrawRS[i].State == State && dynamicDrawRS[i].Value == Value)
		{
			state = &dynamicDrawRS[i];
			break;
		}
	}

	if (state == NULL)
	{
		state = &dynamicDrawRS[dynamicDrawRS.Add()];
	}

	state->State = State;
	state->Value = Value;
}

void precompiledBatch::DynamicDraw_SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	Assert (Value != 0xFFFFFFFF);

	dynamicDrawSState * state = NULL;
	for (DWORD i = 0; i < dynamicDrawSS.Size(); i++)
	{
		if(dynamicDrawSS[i].Sampler == Sampler && dynamicDrawSS[i].Type == Type)
		{
			state = &dynamicDrawSS[i];
			break;
		}
	}

	if (state == NULL)
	{
		state = &dynamicDrawSS[dynamicDrawSS.Add()];;
	} /*else
	{
		if (Type == D3DSAMP_MIPFILTER && Value != state->Value)
		{
			int a = 0;
		}
	}

	if (Type == D3DSAMP_MIPFILTER && Value == 2)
	{
		int a = 0;
	}*/

	state->Sampler = Sampler;
	state->Type = Type;
	state->Value = Value;
}

void precompiledBatch::DynamicDraw_SetupGPU(IDirect3DDevice9* device)
{
	for (DWORD i = 0; i < dynamicDrawRS.Size(); i++)
	{
		dynamicDrawRState & s = dynamicDrawRS[i];
		device->SetRenderState(s.State, s.Value);
	}


	for (DWORD i = 0; i < dynamicDrawSS.Size(); i++)
	{
		dynamicDrawSState & s = dynamicDrawSS[i];
		device->SetSamplerState(s.Sampler, s.Type, s.Value);
	}

	device->SetVertexDeclaration(NULL);
	device->SetVertexShader(pDynamicDrawVS);
	device->SetPixelShader(pDynamicDrawPS);


	static BOOL tempBoolVector[32];


	DWORD dwPsConstSize = ps_consts_v4.Size();
	dwPsConstSize = dwPsConstSize - 1;
	constVector4 * __restrict pPsConstData = ps_consts_v4.GetBuffer();
	for (DWORD i = 0; i < dwPsConstSize; i++)
	{
		device->SetPixelShaderConstantF((pPsConstData->dwStartIdx-256), (float *)pPsConstData->sourceData, pPsConstData->dwRegCount);
		pPsConstData++;
	}


	DWORD dwPsConstSizeB = ps_consts_b.Size();
	constBool * __restrict pPsConstDataB = ps_consts_b.GetBuffer();
	for (DWORD i = 0; i < dwPsConstSizeB; i++)
	{
		for (DWORD n = 0; n < 32; n++)
		{
			tempBoolVector[n] = pPsConstDataB->sourceData->get(n);
		}

		device->SetPixelShaderConstantB(pPsConstDataB->dwStartIdx, &tempBoolVector[0], 32);
		pPsConstDataB++;
	}


	DWORD dwVsConstSize = vs_consts_v4.Size();
	dwVsConstSize = dwVsConstSize - 1;
	constVector4 * __restrict pVsConstData = vs_consts_v4.GetBuffer();

	for (DWORD i = 0; i < dwVsConstSize; i++)
	{
		device->SetVertexShaderConstantF(pVsConstData->dwStartIdx, (float *)pVsConstData->sourceData, pVsConstData->dwRegCount);
		pVsConstData++;
	}

	DWORD dwVsConstSizeB = vs_consts_b.Size();
	constBool * __restrict pVsConstDataB = vs_consts_b.GetBuffer();
	for (DWORD i = 0; i < dwVsConstSizeB; i++)
	{
		for (DWORD n = 0; n < 32; n++)
		{
			tempBoolVector[n] = pVsConstDataB->sourceData->get(n);
		}

		device->SetVertexShaderConstantB(pVsConstDataB->dwStartIdx, &tempBoolVector[0], 32);
		pVsConstDataB++;
	}

	DWORD dwTexConstSize = tex_consts.Size();
	textureSampler * __restrict pTexConstData = tex_consts.GetBuffer();

	for (DWORD i = 0; i < dwTexConstSize; i++)
	{
		device->SetTexture(pTexConstData->dwStage, *pTexConstData->tex);
		pTexConstData++;
	}

	

}

const char * precompiledBatch::GetShaderName()
{
	return shaderName.c_str();
}


void precompiledBatch::ApplyStateHacksInplace(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks)
{
	if (bDisableHacks)
	{
		return;
	}

	if (hacks[SH_CULLMODE].enabled)
	{
		dword cullValue = D3DCULL_NONE;
		switch (hacks[SH_CULLMODE].v)
		{
		case SHCM_NONE:
			cullValue = D3DCULL_NONE;
			break;
		case SHCM_CW:
			cullValue = D3DCULL_CW;
			break;
		case SHCM_CCW:
			cullValue = D3DCULL_CCW;
			break;
		case SHCM_INVERSE:
			if (originalGpuCull == D3DCULL_CW) cullValue = D3DCULL_CCW;
			if (originalGpuCull == D3DCULL_CCW) cullValue = D3DCULL_CW;
			break;
		}
		SetCullMode((GPUCULL)cullValue);
	} 

	if (hacks[SH_ZWRITE].enabled)
	{
		if (hacks[SH_ZWRITE].v == SHZW_ENABLE)
		{
			SetHyperZWriteEnable (true);
			SetZWriteEnable(true);
		} else
		{
			SetHyperZWriteEnable (false);
			SetZWriteEnable(false);
		}
	}

	if (hacks[SH_COLORWRITE].enabled)
	{
		if (hacks[SH_COLORWRITE].v == SHCW_ENABLE)
		{
			SetColorMask(0x0F);
		} else
		{
			SetColorMask(0x0);
		}
	}



	//Это все не перебиваеться из CB, поэтому можно в девайс ставить...
	if (hacks[SH_CLIPPLANEENABLE].enabled)
	{
		device->SetRenderState(D3DRS_CLIPPLANEENABLE, hacks[SH_CLIPPLANEENABLE].v);
	}

	if (hacks[SH_SCISSORRECT].enabled)
	{
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, hacks[SH_SCISSORRECT].v);

	}




}

void precompiledBatch::RestoreStateHacksInplace(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks)
{
	if (bDisableHacks)
	{
		return;
	}

	//Восстанавливаем, что испортили в CB
	if (hacks[SH_CULLMODE].enabled)
	{
		SetCullMode(originalGpuCull);
	} 

	if (hacks[SH_ZWRITE].enabled)
	{
		SetHyperZWriteEnable (originalZEnable);
		SetZWriteEnable(originalHyperZEnable);
	}

	if (hacks[SH_COLORWRITE].enabled)
	{
		SetColorMask(originalColorMask);
	}


	//востанавливаем что попортили в GPU
	if (hacks[SH_CLIPPLANEENABLE].enabled)
	{
		device->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
	}

	if (hacks[SH_SCISSORRECT].enabled)
	{
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	}


}

void precompiledBatch::DynamicDraw_RestoreStateHacks(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks)
{
	//востанавливаем что попортили в GPU
	if (hacks[SH_CLIPPLANEENABLE].enabled)
	{
		device->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
	}

	if (hacks[SH_SCISSORRECT].enabled)
	{
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	}

	//остальное не надо, т.к. заново переставляется все равно
}

void precompiledBatch::DynamicDraw_ApplyStateHacks(IDirect3DDevice9* device, TStateHack * hacks, bool bDisableHacks)
{
	if (bDisableHacks)
	{
		return;
	}

	//cull hack
	if (hacks[SH_CULLMODE].enabled)
	{
		dword cullValue = D3DCULL_NONE;
		switch (hacks[SH_CULLMODE].v)
		{
		case SHCM_NONE:
			cullValue = D3DCULL_NONE;
			break;
		case SHCM_CW:
			cullValue = D3DCULL_CW;
			break;
		case SHCM_CCW:
			cullValue = D3DCULL_CCW;
			break;
		case SHCM_INVERSE:
			if (originalGpuCull == D3DCULL_CW) cullValue = D3DCULL_CCW;
			if (originalGpuCull == D3DCULL_CCW) cullValue = D3DCULL_CW;
			break;
		}
		device->SetRenderState(D3DRS_CULLMODE, cullValue);
	} 


	if (hacks[SH_CLIPPLANEENABLE].enabled)
	{
		device->SetRenderState(D3DRS_CLIPPLANEENABLE, hacks[SH_CLIPPLANEENABLE].v);
	}

	if (hacks[SH_SCISSORRECT].enabled)
	{
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, hacks[SH_SCISSORRECT].v);
	}

	if (hacks[SH_COLORWRITE].enabled)
	{
		if (hacks[SH_COLORWRITE].v == SHCW_ENABLE)
		{
			device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
		} else
		{
			device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0);
		}
	}

	if (hacks[SH_ZWRITE].enabled)
	{
		if (hacks[SH_ZWRITE].v == SHZW_ENABLE)
		{
			device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		} else
		{
			device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}
	}



}


void precompiledBatch::InsertPrefetchFiller()
{
	//
	constVector4 & filler = ps_consts_v4[ps_consts_v4.Add()];
	filler.dwRegCount = 0;
	filler.dwStartIdx = 0;
	filler.fixupPtrInCommandBuffer = NULL;
	filler.sourceData = NULL;


	//
	filler = vs_consts_v4[vs_consts_v4.Add()];
	filler.dwRegCount = 0;
	filler.dwStartIdx = 0;
	filler.fixupPtrInCommandBuffer = NULL;
	filler.sourceData = NULL;
}

#endif