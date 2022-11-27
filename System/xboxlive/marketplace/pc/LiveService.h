/*#ifndef _XBOX
#ifndef _LIVESERVICE_H_
#define _LIVESERVICE_H_

#include "..\ILiveService.h"

class LiveService : public ILiveService
{
public:
	LiveService();
	virtual ~LiveService();

	virtual bool Init();
	virtual bool Frame(float dltTime);

	virtual const char* GetErrorText() {return m_pcErrorText;}

protected:
	void Release();
	void SetError(const char * frmt,...);

	char m_pcErrorText[1024];
};

#endif
#endif
*/