

#ifndef _MissionFile_h_
#define _MissionFile_h_


#pragma pack(push, 1)

#define MISSION_FILE_ID		"Mission "
#define MISSION_FILE_VER	'20.1'


struct MissionFileId
{
	char id[8];
	long ver;
	long objectsCount;
};




#pragma pack(pop)

#endif

