#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

#ifndef X360_GPU_STRUCTS
#define X360_GPU_STRUCTS


#include <xtl.h>
#include <xgraphics.h>
#include <xboxmath.h>

#include <Assert.h>


#pragma warning(push)

// Disable nameless struct/union and zero-sized array warnings for this header.
#pragma warning(disable:4201 4200)


// The PPC back-end of the C compiler by default defines bitfields to be
// ordered from the MSB to the LSB, which is opposite the convention on
// the X86 platform.  Use the 'bitfield_order' pragma to switch the
// ordering.  Note that this does not affect endianness in any way.
#if defined(_M_PPCBE)
#pragma bitfield_order(push)
#pragma bitfield_order(lsb_to_msb)
#endif


//gpu пакет обновляющий X регистров
#define GPU_PACKET_TYPE_0 (0x0)

//gpu пакет обновляющий 2 регистра
#define GPU_PACKET_TYPE_1 (0x1)

//gpu filler пакет
#define GPU_PACKET_TYPE_2 (0x2)

//gpu пакет с операцией
#define GPU_PACKET_TYPE_3 (0x3)


//Второй dword токен'а окончания command buffer
#define GPU_END_CB_TOKEN (0xC0000000)

//Начиная с июньского XDK адреса чуть съехали
#define JUNE_2009_XDK (1)




#ifdef JUNE_2009_XDK

//июньский 2009 XDK и выше
#ifdef _DEBUG
#define GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL (216+8)
#else
#ifdef PROFILE
#define GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL (216+8)
#else
#define GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL (192)
#endif
#endif


#else

//ниже июньского 2009 XDK
#ifdef _DEBUG
#define GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL (216)
#else
#ifdef PROFILE
#define GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL (216)
#else
#define GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL (192)
#endif
#endif

#endif







#ifdef _DEBUG
	#define GPU_OFFSET_TO_VSHADER_MICROCODE_ADDR (32)
#else
	#define GPU_OFFSET_TO_VSHADER_MICROCODE_ADDR (32)
#endif



//Для изменениия размера command buffer и адреса расположения
//
//структура эта внутри - D3DCommandBuffer
typedef union {
	struct {
		DWORD sizeInDwords       : 16;  // 
		DWORD header             : 16;   //packet type
		DWORD gpuAdress          : 32;  //
	};
	DWORD dword[2];
} GPUCOMMAND_BUFFER_INTERNAL;




//Заголовок CommandBuffer пакет, тип 0
//для обновления CountMinusOne количества hardware registers
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		DWORD BaseRegisterIndex       : 15; //base register index to update GPUREGISTER
		DWORD oneRegWr                : 1;  // 
		DWORD CountMinusOne           : 14; //register count to update (need add one)
		DWORD PacketType              : 2;  //packet type
	};
	DWORD dword;
} GPUCOMMANDBUFFER_HEADER_PACKET0;


//Заголовок CommandBuffer пакет, тип 1
//для обновления 2-х hardware registers
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		DWORD regIndex2          : 11; //GPUREGISTER
		DWORD regIndex1          : 11; //GPUREGISTER 
		DWORD Reserved           : 8;  //
		DWORD PacketType         : 2;  // packet type
	};
	DWORD dword;
} GPUCOMMANDBUFFER_HEADER_PACKET1;

//Заголовок CommandBuffer пакет, тип 2
//пакет filler
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		DWORD Reserved           : 30; //
		DWORD PacketType         : 2;  //packet type
	};
	DWORD dword;
} GPUCOMMANDBUFFER_HEADER_PACKET2;

//Заголовок CommandBuffer пакет, тип 3
//для выполнения операции заданной OpCode
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		DWORD Reserved           : 8;  //
		DWORD OpCode             : 8;  //GPUCOMMANDOP
		DWORD CountMinusOne      : 14; //
		DWORD PacketType         : 2;  //packet type
	};
	DWORD dword;
} GPUCOMMANDBUFFER_HEADER_PACKET3;



//Общий union представляющий все заголовки пакетов
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		union
		{
			GPUCOMMANDBUFFER_HEADER_PACKET0 Packet0;
			GPUCOMMANDBUFFER_HEADER_PACKET1 Packet1;
			GPUCOMMANDBUFFER_HEADER_PACKET2 Packet2;
			GPUCOMMANDBUFFER_HEADER_PACKET3 Packet3;
		};
	};
	DWORD dword;
} GPUCOMMANDBUFFER_HEADER_PACKET_COMMON;




//Кусочек texture fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 0:

		DWORD Type                      : 2;    // GPUCONSTANTTYPE
		DWORD SignX                     : 2;    // GPUSIGN
		DWORD SignY                     : 2;    // GPUSIGN
		DWORD SignZ                     : 2;    // GPUSIGN
		DWORD SignW                     : 2;    // GPUSIGN
		DWORD ClampX                    : 3;    // GPUCLAMP
		DWORD ClampY                    : 3;    // GPUCLAMP
		DWORD ClampZ                    : 3;    // GPUCLAMP
		DWORD                           : 2;
		DWORD                           : 1;
		DWORD Pitch                     : 9;    // DWORD
		DWORD Tiled                     : 1;    // BOOL
	};
	DWORD dword[1];
} GPUTEXTURE_FETCH_CONSTANT_0;


//Кусочек texture fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 1:

		DWORD DataFormat                : 6;    // GPUTEXTUREFORMAT
		DWORD Endian                    : 2;    // GPUENDIAN
		DWORD RequestSize               : 2;    // GPUREQUESTSIZE
		DWORD Stacked                   : 1;    // BOOL
		DWORD ClampPolicy               : 1;    // GPUCLAMPPOLICY
		DWORD BaseAddress               : 20;   // DWORD
	};
	DWORD dword[1];
} GPUTEXTURE_FETCH_CONSTANT_1;


//Кусочек texture fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 2:

		union
		{
			GPUTEXTURESIZE_1D OneD;
			GPUTEXTURESIZE_2D TwoD;
			GPUTEXTURESIZE_3D ThreeD;
			GPUTEXTURESIZE_STACK Stack;
		} Size;
	};
	DWORD dword[1];
} GPUTEXTURE_FETCH_CONSTANT_2;


//Кусочек texture fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 3:

		DWORD NumFormat                 : 1;    // GPUNUMFORMAT
		DWORD SwizzleX                  : 3;    // GPUSWIZZLE
		DWORD SwizzleY                  : 3;    // GPUSWIZZLE
		DWORD SwizzleZ                  : 3;    // GPUSWIZZLE
		DWORD SwizzleW                  : 3;    // GPUSWIZZLE
		INT   ExpAdjust                 : 6;    // int
		DWORD MagFilter                 : 2;    // GPUMINMAGFILTER
		DWORD MinFilter                 : 2;    // GPUMINMAGFILTER
		DWORD MipFilter                 : 2;    // GPUMIPFILTER
		DWORD AnisoFilter               : 3;    // GPUANISOFILTER
		DWORD                           : 3;
		DWORD BorderSize                : 1;    // DWORD
	};
	DWORD dword[1];
} GPUTEXTURE_FETCH_CONSTANT_3;


//Кусочек texture fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 4:

		DWORD VolMagFilter              : 1;    // GPUMINMAGFILTER
		DWORD VolMinFilter              : 1;    // GPUMINMAGFILTER
		DWORD MinMipLevel               : 4;    // DWORD
		DWORD MaxMipLevel               : 4;    // DWORD
		DWORD MagAnisoWalk              : 1;    // BOOL
		DWORD MinAnisoWalk              : 1;    // BOOL
		INT   LODBias                   : 10;   // int
		INT   GradExpAdjustH            : 5;    // int
		INT   GradExpAdjustV            : 5;    // int
	};
	DWORD dword[1];
} GPUTEXTURE_FETCH_CONSTANT_4;



//Кусочек texture fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 5:

		DWORD BorderColor               : 2;    // GPUBORDERCOLOR
		DWORD ForceBCWToMax             : 1;    // BOOL
		DWORD TriClamp                  : 2;    // GPUTRICLAMP
		INT   AnisoBias                 : 4;    // int
		DWORD Dimension                 : 2;    // GPUDIMENSION
		DWORD PackedMips                : 1;    // BOOL
		DWORD MipAddress                : 20;   // DWORD
	};
	DWORD dword[1];
} GPUTEXTURE_FETCH_CONSTANT_5;



//Кусочек vertex fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 0:

		DWORD Type                      : 2;    // GPUCONSTANTTYPE
		DWORD BaseAddress               : 30;   // DWORD
	};
	DWORD dword[1];
} GPUVERTEX_FETCH_CONSTANT_0;

//Кусочек vertex fetch constant
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		// DWORD 1:

		DWORD Endian                    : 2;    // GPUENDIAN
		DWORD Size                      : 24;   // DWORD
		DWORD AddressClamp              : 1;    // GPUADDRESSCLAMP
		DWORD                           : 1;
		DWORD RequestSize               : 2;    // GPUREQUESTSIZE
		DWORD ClampDisable              : 2;    // BOOL
	};
	DWORD dword[1];
} GPUVERTEX_FETCH_CONSTANT_1;


#if defined(_M_PPCBE)
#pragma bitfield_order(pop)
#endif




//Посчитать magic значение формирующее тэг окончания commandBuffer + 
__forceinline DWORD GPUGenerateMagicValue (const void * dwMemoryPtr)
{
	DWORD dwGpuMem = GPU_CONVERT_CPU_TO_GPU_ADDRESS(dwMemoryPtr);
	DWORD dwMemPtr = (DWORD)GPU_CONVERT_GPU_TO_CPU_ADDRESS_16MB(dwGpuMem);
	return (dwMemPtr + sizeof(DWORD));
}

//Получить размер содержимого command Buffer'a в количестве DWORD
__forceinline WORD GPUGetCommandBufferSizeInDwords(D3DCommandBuffer * cmdBuffer)
{
	BYTE* ptr = (BYTE*)cmdBuffer;
	ptr += GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL;
	GPUCOMMAND_BUFFER_INTERNAL * pPtr = (GPUCOMMAND_BUFFER_INTERNAL *)ptr;

	Assert (pPtr->header == 0x8100);

	WORD sizeInDwords = pPtr->sizeInDwords;
	return sizeInDwords;
}

//Установить размер содержимого command Buffer'a в количестве DWORD
__forceinline void GPUSetCommandBufferSizeInDwords(D3DCommandBuffer * cmdBuffer, WORD sizeInDwords)
{
	BYTE* ptr = (BYTE*)cmdBuffer;
	ptr += GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL;
	GPUCOMMAND_BUFFER_INTERNAL * pPtr = (GPUCOMMAND_BUFFER_INTERNAL *)ptr;

#ifndef STOP_DEBUG
	if (pPtr->header != 0x8100)
	{
		OutputDebugString("Find here 0x81 and 0x00, then 4 bytes address and correct GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL define\n");

		char temp[64];
		for (int i = -100; i < 100; i++)
		{
			BYTE v = *(ptr + i);
			
			crt_snprintf(temp, 31, "[%d] 0x%02X\n", i, v);
			OutputDebugString(temp);
		}
	}
#endif

	Assert (pPtr->header == 0x8100);

	pPtr->sizeInDwords = sizeInDwords;
}


//Получить адресс содержимого command Buffer'a
__forceinline BYTE * GPUGetCommandBufferMemoryAddr(D3DCommandBuffer * cmdBuffer)
{
	BYTE* ptr = (BYTE*)cmdBuffer;
	ptr += GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL;
	GPUCOMMAND_BUFFER_INTERNAL * pPtr = (GPUCOMMAND_BUFFER_INTERNAL *)ptr;

	Assert (pPtr->header == 0x8100);

	BYTE * dataPtr =  (BYTE*)GPU_CONVERT_GPU_TO_CPU_ADDRESS_4KB(pPtr->gpuAdress);
	return dataPtr;
}

//Установить адресс содержимого command Buffer'a
__forceinline void GPUSetCommandBufferMemoryAddr(D3DCommandBuffer * cmdBuffer, const BYTE * addrBy_XPhysicalAlloc)
{
	BYTE* ptr = (BYTE*)cmdBuffer;
	ptr += GPU_OFFSET_TO_COMMAND_BUFFER_INTERNAL;
	GPUCOMMAND_BUFFER_INTERNAL * pPtr = (GPUCOMMAND_BUFFER_INTERNAL *)ptr;

	Assert (pPtr->header == 0x8100);

	pPtr->gpuAdress = GPU_CONVERT_CPU_TO_GPU_ADDRESS(addrBy_XPhysicalAlloc);
}



__forceinline GPUCOMMAND_LOAD_SHADER* GPUGetPixelShaderPhysicalAddress (IDirect3DPixelShader9 *pPixelShader)
{
	BYTE * tempPtr = (BYTE *)pPixelShader;
	tempPtr += 0x18;
	GPUCOMMAND_LOAD_SHADER * ptr = (GPUCOMMAND_LOAD_SHADER *)tempPtr;
	return ptr;
}

__forceinline GPUCOMMAND_LOAD_SHADER* GPUGetVertexShaderPhysicalAddress (IDirect3DVertexShader9 *pVertexShader)
{
	BYTE * tempPtr = (BYTE *)pVertexShader;
	tempPtr += 0x20;
	GPUCOMMAND_LOAD_SHADER * ptr = (GPUCOMMAND_LOAD_SHADER *)tempPtr;
	return ptr;
}



/*
extern DWORD GPUGenerateMagicValue (const void * dwMemoryPtr);
extern WORD GPUGetCommandBufferSizeInDwords(D3DCommandBuffer * cmdBuffer);
extern void GPUSetCommandBufferSizeInDwords(D3DCommandBuffer * cmdBuffer, WORD sizeInDwords);
extern BYTE * GPUGetCommandBufferMemoryAddr(D3DCommandBuffer * cmdBuffer);
extern void GPUSetCommandBufferMemoryAddr(D3DCommandBuffer * cmdBuffer, const BYTE * addrBy_XPhysicalAlloc);
*/


//Окончание command buffer
//
//структура эта внутри - данных command buffer
typedef union {
	struct {
		DWORD magicValue       : 32;   //Вычисляется
		DWORD endToken         : 32;   //0xc0000000
	};
	DWORD dword[2];


	void Place()
	{
		magicValue = GPUGenerateMagicValue(this);
		endToken = 0xc0000000;

	}

} GPUCOMMAND_BUFFER_END;


#pragma warning(pop)


#endif

#endif