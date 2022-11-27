#ifdef _XBOX

#include "ContentEnumerator.h"

#define ContentEnumeratorEventName	"ContentEnumerator"

XBoxLiveContentEnumerator::XBoxLiveContentEnumerator()
{
	m_overlapped.hEvent = NULL;
	m_hEnum = INVALID_HANDLE_VALUE;
}

XBoxLiveContentEnumerator::~XBoxLiveContentEnumerator()
{
	End();
}

bool XBoxLiveContentEnumerator::Start()
{
	End();

	DWORD dwResult;
    DWORD cbBuffer;

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

	PrepareOverlapped();
	GetNextContentData();

	return true;
}

bool XBoxLiveContentEnumerator::IsDone()
{
	if( IsEnd() ) return false;
	return XHasOverlappedIoCompleted(&m_overlapped);
}

bool XBoxLiveContentEnumerator::IsValideContent()
{
	if( IsEnd() ) return false;
	DWORD dwRes = XGetOverlappedResult(&m_overlapped,NULL,FALSE);
	if( dwRes == ERROR_SUCCESS ) return true;
	return false;
}

bool XBoxLiveContentEnumerator::IsEnd()
{
	if( INVALID_HANDLE_VALUE == m_hEnum ) return true;
	DWORD dwRes = XGetOverlappedResult(&m_overlapped,NULL,FALSE);
	if( dwRes == ERROR_NO_MORE_FILES )
		return true;
	if( dwRes == ERROR_SUCCESS || dwRes == ERROR_IO_INCOMPLETE )
		return false;

	// Часто, когда заканчивается енумерация, возвращается код ошибки в исполнении.
	// на самом деле, если проверить расширенное сообщение об ошибке, то оказывается
	// что просто закончились файлы (конец перечисления)
	dword dwExtErrCode = 0xFFFF & XGetOverlappedExtendedError(&m_overlapped);
	if( dwExtErrCode == ERROR_NO_MORE_FILES )
		return true;

	// сюда мы не должны попадать - произошла ошибка при загрузке контента
	// если мы будем считать что еще не конец, тогда мы будем пытаться вечно грузить контент
	// в противном случае мы можем недогрузить остальные контенты, так как в енумераторе нет
	// возможности перейти к следующему контенту
	api->Trace ( "XBoxLive content enumeration error! error code=%u, extended error code=%u", dwRes, dwExtErrCode );
	return true;
}

void XBoxLiveContentEnumerator::End()
{
	CloseOverlapped();

	if( m_hEnum != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hEnum);
		m_hEnum = INVALID_HANDLE_VALUE;
	}
}

void XBoxLiveContentEnumerator::Next()
{
	if( !IsEnd() )
	{
		GetNextContentData();
	}
}

const char* XBoxLiveContentEnumerator::GetCurContentName()
{
	if( !IsValideContent() ) return null;

	memcpy(m_pcContentName, m_content.szFileName, XCONTENT_MAX_FILENAME_LENGTH);
    m_pcContentName[XCONTENT_MAX_FILENAME_LENGTH] = 0;

	return m_pcContentName;
}

void XBoxLiveContentEnumerator::GetContentData(XContentData& cd)
{
	if( IsValideContent() )
	{
		cd.dat = m_content;
	}
}

void XBoxLiveContentEnumerator::GetNextContentData()
{
	// получаем данные для очередного контента
	DWORD dwRes = XEnumerate(m_hEnum, &m_content, sizeof(XCONTENT_DATA), NULL, &m_overlapped);
    if( dwRes != ERROR_IO_PENDING )
    {
		api->Trace("XEmumerate error");
    }
}

void XBoxLiveContentEnumerator::PrepareOverlapped()
{
	ZeroMemory( &m_overlapped, sizeof( m_overlapped ) );
	m_overlapped.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

void XBoxLiveContentEnumerator::CloseOverlapped()
{
	if( m_overlapped.hEvent != NULL )
	{
		CloseHandle(m_overlapped.hEvent);
		m_overlapped.hEvent = NULL;
	}
}

#endif
