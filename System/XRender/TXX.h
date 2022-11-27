//================================================================
//Texture file .txx
//================================================================

#ifndef __Texture_txx_h__
#define __Texture_txx_h__


//================================================================
//
//  Структура файла:
//
//		TEXTURE_FILE  заголовок
//
//	2D texture:
//		mip levels           |
//		width x heigth		 | TEXTURE_FILE.nmips
//		width/2 x heigth/2	 |
//		...                  |
//
//	CubeMap:
//		always sides is quads 2D textures
//		sequence order: right, left, top, bottom, front, back
//		right - positive X
//		left - negative X
//		top - positive Y
//		bottom - negative Y
//		front - positive Z
//		back - negative Z
//
//	Volume texture:
//		box level
//		box level/8
//		box level/64
//		...
//
//================================================================


#define TXX_ID			(unsigned long)(('T' << 0) | ('X' << 8) | ('X' << 16)  | (' ' << 24))
#define TXX_VER			(unsigned long)(('1' << 0) | ('.' << 8) | ('0' << 16)  | (' ' << 24))
#define TXX_MAXSIZE		2048


//Types
enum TXXType
{
	txxt_unknown = 0,
	txxt_2d,
	txxt_cube,
	txxt_volume,
	txxt_rendertarget
};

enum TXXCubeSides
{
	txxcs_right = 0,
	txxcs_left = 1,
	txxcs_top = 2,
	txxcs_bottom = 3,
	txxcs_front = 4,
	txxcs_back = 5,
};

//Formats
enum TXXFormat
{
	txxf_unknown = 0,
	txxf_a8r8g8b8,
	txxf_q8w8v8u8,
	txxf_r5g6b5,
	txxf_a1r5g5b5,
	txxf_a4r4g4b4,	
	txxf_v8u8,
	txxf_a8,
	txxf_l8,	
	txxf_dxt1,
	txxf_dxt3,
	txxf_dxt5,
	txxf_max,
};



#pragma pack(push, 1)

//File header
struct TxxFileHeader
{
	unsigned long	id;				//Идентификатор файла
	unsigned long	ver;			//Версия
	unsigned long	type;			//Тип текстуры TXXType
	unsigned long	format;			//Формат текстуры TXXFormat
	unsigned long	width;			//Ширина большего мипа
	unsigned long	height;			//Высота большего мипа
	unsigned long	depth;			//Глубина большего мипа (2D, Cubemap == 1)
	unsigned long	num_mips;		//Количество мипов в текстуре
	unsigned long	line_size;		//Размер одной строки в байтах (width*bpp/8)
	unsigned long	bpp;			//Бит на пиксель
	float			fps;			//Скорость проигрывания в fps
};

#pragma pack(pop)

#endif