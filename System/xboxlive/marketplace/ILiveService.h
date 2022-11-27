/*#ifndef _ILIVESERVICE_H_
#define _ILIVESERVICE_H_

//#include "..\..\..\common_h\core.h"

class ILiveService //: public Service
{
public:
	ILiveService() {}
	virtual ~ILiveService() {}

	virtual bool Init() = 0;
	virtual bool Frame(float dltTime) = 0;

	virtual const char* GetErrorText() = 0;
};

#endif
*/