#ifndef _XBOX

#include "shlobj.h"
#include "cfg_switches.h"
#include "d_types.h"

//#pragma comment(lib, "shell32")

#define SWP_JOINSTRINGS2(S1, S2) S1##S2
#define SWP_JOINSTRINGS4(S1, S2, S3, S4) S1##S2##S3##S3
#define SWP_GAME_TITLE_FOLDER GAME_TITLE "\\"
#define SWP_PROFILES_ROOT_FOLDER "My Games"
#define SWP_PROFILES_FOLDER SWP_PROFILES_ROOT_FOLDER "\\" GAME_TITLE "\\"


class SystemWorkPath
{
public:
	enum Type
	{
		t_error,
		t_profile,
		t_ini,
		t_content,
	};

public:
	SystemWorkPath(Type type)
	{
		memset(path, 0, MAX_PATH);
		dword fpType = t_error;
		const char * addPath = null;
		switch(type)
		{
		case t_profile:
		case t_ini:		
			fpType = CSIDL_PERSONAL;
			addPath = SWP_PROFILES_FOLDER;
			break;
		case t_content:
			fpType = CSIDL_APPDATA;
			addPath = SWP_GAME_TITLE_FOLDER;
			break;
		default:
			swp_assert(false);
		}
		HRESULT hr = ::SHGetFolderPath(0, fpType | CSIDL_FLAG_CREATE, 0, SHGFP_TYPE_CURRENT, path);
		swp_assert(hr == S_OK);
		//Ищем окончание строки и приписываем дополнительный путь
		for(pathLen = 0; path[pathLen] && pathLen < MAX_PATH; pathLen++);
		swp_assert(pathLen < MAX_PATH);
		swp_assert(pathLen > 0);
		if(path[pathLen - 1] != '\\' && path[pathLen - 1] != '/')
		{
			path[pathLen] = '\\';
			pathLen++;
			swp_assert(pathLen < MAX_PATH);
			path[pathLen] = 0;
		}
		//Добавляем дополнительный путь
		swp_assert(addPath != null);
		while(*addPath)
		{
			swp_assert(pathLen < MAX_PATH);
			path[pathLen] = *addPath;
			addPath++;
			pathLen++;
		}
		swp_assert(pathLen < MAX_PATH);
		path[pathLen] = 0;
	}

	void AppendFileName(const char * file)
	{
		if(!file || !file[0]) return;
		dword i = pathLen;
		for(; *file && i < MAX_PATH; i++, file++) path[i] = *file;
		swp_assert(i < MAX_PATH);
		path[i] = 0;
	}

	const char * GetPath()
	{
		return path;
	}

private:
	inline void swp_assert(bool v)
	{
#ifndef STOP_ASSERTS
		if(!v)
		{
			_asm int 3;
		}
#endif
	};

private:
	char path[MAX_PATH];
	long pathLen;
};

#endif