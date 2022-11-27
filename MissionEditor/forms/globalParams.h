#ifndef _GLOBAL_PARAMS_H___
#define _GLOBAL_PARAMS_H___



struct globalParams
{
	string defer_load_name;
	string folder_to_create;
	string LoadedMissionName;
	string LoadedMissionDir;
	string loadedFileName;

	string __tmpText;
	string StartDirectory; //Папка откуда стартовал редактор...
	string MissionsSrcLocalFolder;

	string VSSUserName;
	string SaveBoxText;


};


extern globalParams * gp;



#endif