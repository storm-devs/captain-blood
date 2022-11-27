#ifndef _XBOX

#include "PCContentEnumerator.h"

PCLiveContentEnumerator::PCLiveContentEnumerator(ILiveService* pLiveService)
{
	m_pLiveService = pLiveService;
	//m_overlapped.hEvent = NULL;
	m_hEnum = INVALID_HANDLE_VALUE;
}

PCLiveContentEnumerator::~PCLiveContentEnumerator()
{
	End();
}

bool PCLiveContentEnumerator::Start()
{
	End();

	/*DWORD dwResult;
    DWORD cbBuffer;

	WCHAR pwszServerPath[1024];
	DWORD dwServerPathSize;
	const WCHAR pwszItemName[] = L"*";
	dwResult = XStorageBuildServerPath(0, XSTORAGE_FACILITY_PER_TITLE, NULL, 0, pwszItemName, pwszServerPath, &dwServerPathSize );

	dwResult = XOnlineStartup();
	XUSER_SIGNIN_STATE userSignState = XUserGetSigninState(0);

	if( XUserGetSigninState(0) == eXUserSigninState_NotSignedIn )
	{
		//XShowSigninUI(1,0);
	}

	// получаем список доступных в данный момент контентов, в качестве ИД пользователя используем 0, это значение
	// все равно не используется в нашем случае, так как мы используем общий контент. Поиск доступных контентов
	// производим по всем местам хранения (параметр XCONTENTDEVICE_ANY).
	dwResult = XContentCreateEnumerator(0, XCONTENTDEVICE_ANY, XCONTENTTYPE_MARKETPLACE, 0, 1, &cbBuffer, &m_hEnum);
	if(dwResult!=ERROR_SUCCESS) return false;
	if(cbBuffer!=sizeof(XCONTENT_DATA))
	{
		End();
		return false;
	}
*/
	GetNextContentData();

	return true;
}

bool PCLiveContentEnumerator::IsDone()
{
	if( IsEnd() ) return false;
	return false;
}

bool PCLiveContentEnumerator::IsValideContent()
{
	if( IsEnd() ) return false;
	return false;
}

bool PCLiveContentEnumerator::IsEnd()
{
	if( m_hEnum == INVALID_HANDLE_VALUE )
		return true;
	return true;
}

void PCLiveContentEnumerator::End()
{
	if( m_hEnum != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hEnum);
		m_hEnum = INVALID_HANDLE_VALUE;
	}
}

void PCLiveContentEnumerator::Next()
{
	if( !IsEnd() )
	{
		GetNextContentData();
	}
}

const char* PCLiveContentEnumerator::GetCurContentName()
{
	if( !IsValideContent() ) return "";

	//memcpy(m_pcContentName, m_content.szFileName, XCONTENT_MAX_FILENAME_LENGTH);
    //m_pcContentName[XCONTENT_MAX_FILENAME_LENGTH] = 0;

	return "";
}

void PCLiveContentEnumerator::GetContentData(XContentData& cd)
{
	if( IsValideContent() )
	{
		//cd.dat = m_content;
	}
}

void PCLiveContentEnumerator::GetNextContentData()
{
	// получаем данные для очередного контента
	/*DWORD dwRes = XEnumerate(m_hEnum, &m_content, sizeof(XCONTENT_DATA), NULL, &m_overlapped);
    if( dwRes != ERROR_IO_PENDING )
    {
		api->Trace("XEmumerate error");
    }*/
}

#endif
