



#ifndef _MutexProtector_h_
#define _MutexProtector_h_

#include <windows.h>

class MutexProtector
{
public:
	MutexProtector(const char * iniName)
	{
		hMutex = NULL;
		if(!iniName || !iniName[0]) throw "Invalidate ini name pointer";
		long i = 0;
		for(; i < sizeof(mutexName) && iniName[i] && iniName[i] != '.'; i++)
		{
			char c = iniName[i];
			bool isNotReplace = false;
			if(c >= '0' && c <= '9') isNotReplace = true;
			if(c >= 'A' && c <= 'Z') isNotReplace = true;
			if(c >= 'a' && c <= 'z') isNotReplace = true;
			mutexName[i] = isNotReplace ? c : '_';
		}
		const char * addStr = "_mutex for diasable clones.";
		for(; i < sizeof(mutexName) && *addStr; i++) mutexName[i] = *addStr++;
		if(i >= sizeof(mutexName)) i = sizeof(mutexName) - 1;
		mutexName[i] = 0;
	}

	~MutexProtector()
	{
		if(hMutex)
		{
			::CloseHandle(hMutex);
			hMutex = NULL;
		}
	}

	bool Check()
	{
		if(hMutex) return true;
		//Недопустим запуск второй копии редактора
		hMutex = ::CreateMutex(NULL, TRUE, mutexName);
		if(GetLastError() == ERROR_ALREADY_EXISTS || hMutex == NULL)
		{
			return false;
		}
		return true;
	}

private:
	HANDLE hMutex;
	char mutexName[252];	
};





#endif
