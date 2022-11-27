//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// CameraTracksFile
//============================================================================================

#ifndef _CameraTracksFile_h_
#define _CameraTracksFile_h_

#pragma pack(push, 1)

#define CameraTracksID	(unsigned long)(('C' << 0) | ('T' << 8) | ('F' << 16)  | (' ' << 24))	
#define CameraTracksVer	(unsigned long)(('1' << 0) | ('.' << 8) | ('0' << 16)  | (' ' << 24))

struct CameraTracksHeader
{
	unsigned long id;
	unsigned long ver;
	unsigned long numPoints;
};

struct CameraTracksPoint
{
	//Линия игрока
	float p1x, p1y, p1z;	//Первая точка
	float p2x, p2y, p2z;	//Вторая точка
	//Позиция камеры
	float px, py, pz;		//Позиция камеры
	float qx, qy, qz, qw;	//Направление камеры
};

#pragma pack(pop)

#endif
