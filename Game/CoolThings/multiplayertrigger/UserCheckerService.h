#ifndef USERCHECKERSERVICE_H
#define USERCHECKERSERVICE_H

#include "..\..\..\Common_h\IPlayerChecker.h"

class PlayerChecker : public IPlayerChecker
{
protected:
	static const int dwPlayerQnt = 4;

	string m_sPlayerName[dwPlayerQnt];
	bool m_bReassign[dwPlayerQnt];
	bool m_bPluged[dwPlayerQnt];

	bool m_bUseUser[dwPlayerQnt];

	long m_dwUserIndex[dwPlayerQnt];

public:
	PlayerChecker();
	virtual ~PlayerChecker();

	bool Init();
	void StartFrame(float dltTime);

	virtual void Start() = 0;
	virtual void Update() = 0;
	virtual IControls* SetControlService(IControls* pControl) {return NULL;}

	virtual bool IsUpdated(long idx);
	virtual bool IsPluged(long idx);
	virtual const char* GetPlayerName(long idx);
	virtual bool IsUsed(long idx);

	void SetUsed(long idx, bool bUse);
};

#ifdef _XBOX
class XBoxPlayerChecker : public PlayerChecker
{
	HANDLE m_hNotify;
public:
	XBoxPlayerChecker();
	virtual ~XBoxPlayerChecker();

	virtual void Start();
	virtual void Update();
	virtual IControls* SetControlService(IControls* pControl);

protected:
	void UpdateUser(long n);
};
#else
class PCPlayerChecker : public PlayerChecker
{
	IControls* m_pControlService;
public:
	PCPlayerChecker();
	virtual ~PCPlayerChecker();

	virtual void Start();
	virtual void Update();
	virtual IControls* SetControlService(IControls* pControl) {IControls* p=m_pControlService; m_pControlService=pControl; return p;}
};
#endif

#endif
