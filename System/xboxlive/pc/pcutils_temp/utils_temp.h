#ifndef PCUTILS_TEMP_H
#define PCUTILS_TEMP_H

#ifndef XUID
	typedef ULONGLONG XUID;
#endif

#ifndef XACHIEVEMENT_DETAILS
	typedef struct {
		DWORD dwId;
		LPWSTR pwszLabel;
		LPWSTR pwszDescription;
		LPWSTR pwszUnachieved;
		DWORD dwImageId;
		DWORD dwCred;
		FILETIME ftAchieved;
		DWORD dwFlags;
	} XACHIEVEMENT_DETAILS, *PXACHIEVEMENT_DETAILS;
#endif

#ifndef INVALID_XUID
	#define INVALID_XUID	((XUID) 0)
#endif

#endif
