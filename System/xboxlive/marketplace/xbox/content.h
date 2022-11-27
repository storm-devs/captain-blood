#ifdef _XBOX
#ifndef _CONTENT_H_
#define _CONTENT_H_

#include "..\..\..\..\Common_h\Mission.h"
#include "..\defines.h"

class XBoxLiveContent : public ILiveContent
{
public:
	XBoxLiveContent(const XCONTENT_DATA& cd, long nID);
	virtual ~XBoxLiveContent();

	bool IsPluged() {return m_bPlug;}
	bool PlugContent();
	void UnplugContent();

	const char* GetContentName() {return "";}
	const char* GetDisplayName();

	ITexture* GetPicTexture();
	bool IsPicTextureGetProcessing();
	void ReleasePicTexture();

	bool IsContentMatching(CONST BYTE * pbContentID);

protected:
	DWORD m_dwUserIndex;
	bool m_bPlug;
	char m_pcRootName[64];
	IMirrorPath* m_pMirrorPath;

	long m_nUniqueID;
	XCONTENT_DATA m_cdat;
	char m_pcDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];

	ITexture* m_pPicTexture;
	BYTE* m_pbThumbnailImage;
	DWORD m_dwThumbnailBytes;
	XOVERLAPPED m_GetPictureOverlapped;

	void CloseGetPictureOverlapped();
};

#endif
#endif
