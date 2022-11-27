#ifndef _ICONTENT_H_
#define _ICONTENT_H_

#include "..\..\..\Common_h\Mission.h"
#include "defines.h"

class IContentManager : public Service
{
public:
	struct ContentDescribe
	{
		enum ContentDataFlag
		{
			cdf_DisplayName = 0x00000001,
			cdf_PlugState = 0x00000002,
			cdf_ImageTexture = 0x00000004,

			cdf_all = 0xFFFFFFFF
		};
		ContentDataFlag flags;

		const char* pcDisplayName;
		bool bPlugState;
		ITexture* pTexture;
	};

	enum ContentStatus
	{
		ContentStatus_NewContents,
		ContentStatus_NotReady,
		ContentStatus_NoContents
	};

public:
	virtual ~IContentManager() {}

	virtual ContentStatus GetStatus() = 0;

	virtual void UpdateDownloadList() = 0;
	virtual void UpdateContentList() = 0;

	virtual long GetContentQuantity() = 0;
	virtual bool GetContentDescribe(long nContent,ContentDescribe& descr) = 0;
	virtual bool PlugContent(long nContent) = 0;
	virtual bool UnplugContent(long nContent) = 0;
};

#endif
