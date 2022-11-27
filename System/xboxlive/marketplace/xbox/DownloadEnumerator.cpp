#ifdef _XBOX

#pragma comment(lib,"Xonline.lib")

#include "DownloadEnumerator.h"

XBoxLiveDownloadEnumerator::XBoxLiveDownloadEnumerator() :
	m_offerFree(_FL_),
	m_offerPaid(_FL_)
{
	m_overlapped.hEvent = NULL;
	m_hEnum = INVALID_HANDLE_VALUE;
}

XBoxLiveDownloadEnumerator::~XBoxLiveDownloadEnumerator()
{
	End();

	m_offerFree.DelAll();
	m_offerPaid.DelAll();
}

bool XBoxLiveDownloadEnumerator::Start(DWORD dwTitleID)
{
	m_dwTitleID = dwTitleID;

	End();

	DWORD dwResult;
    DWORD cbBuffer;

	dwResult = XMarketplaceCreateOfferEnumerator(0, 0xFFFFFFFF, 0xFFFFFFFF, 1, &cbBuffer, &m_hEnum);
	if(dwResult!=ERROR_SUCCESS) return false;
	if(cbBuffer!=sizeof(XMARKETPLACE_CONTENTOFFER_INFO))
	{
		End();
		return false;
	}

	PrepareOverlapped();
	GetNextContentData();

	return true;
}

bool XBoxLiveDownloadEnumerator::Frame()
{
	if( IsEnd() )
	{
		if( m_hEnum != INVALID_HANDLE_VALUE )
			End();
		return false;
	}

	// описание контента получено?
	if( IsDone() )
	{
		if( IsValideContent() )
		{
			if( m_curEnumerateInfo.dwTitleID == m_dwTitleID )
			{
				// платные(и еще не купленные) контенты в один список
				if( m_curEnumerateInfo.dwPointsPrice > 0 &&
					m_curEnumerateInfo.fUserHasPurchased != TRUE )
				{
					long n = m_offerPaid.Add();
					m_offerPaid[n].offerInfo = m_curEnumerateInfo;
					m_offerPaid[n].bDownloadedState = false;
				}
				// бесплатные в другой
				else
				{
					long n = m_offerFree.Add();
					m_offerFree[n].offerInfo = m_curEnumerateInfo;
					m_offerFree[n].bDownloadedState = false;
				}
			}
		}
		GetNextContentData();
	}

	return true;
}

bool XBoxLiveDownloadEnumerator::IsCompliteEnumerate()
{
	// идет процесс получения контентов, годных к загрузке
	if( m_hEnum != INVALID_HANDLE_VALUE )
		return false;

	return true;
}

bool XBoxLiveDownloadEnumerator::CheckNewItem(ILiveContent** pContents, long nContentsQ)
{
	// нет контентов вообще
	if( m_offerFree.Size()==0 && m_offerPaid.Size()==0 )
		return false;

	long n,i;
	bool bNew = false;

	// бежим по бесплатным контентам и сверяем их со списком уже загруженных контентов
	for(n=0; n<m_offerFree; n++)
	{
		CONST BYTE * pbContentID = (CONST BYTE*)&m_offerFree[n].offerInfo.qwOfferID;
		for(i=0; i<nContentsQ; i++)
			if( pContents[i]->IsContentMatching(pbContentID) )
				break;
		if( i<nContentsQ )
		{
			m_offerFree[n].bDownloadedState = true;
			bNew = true;
		}
		else
			m_offerFree[n].bDownloadedState = false;
	}

	// бежим по платным контентам и сверяем их со списком уже загруженных контентов
	for(n=0; n<m_offerPaid; n++)
	{
		CONST BYTE * pbContentID = (CONST BYTE*)&m_offerPaid[n].offerInfo.qwOfferID;
		for(i=0; i<nContentsQ; i++)
			if( pContents[i]->IsContentMatching(pbContentID) )
				break;
		if( i<nContentsQ )
		{
			m_offerPaid[n].bDownloadedState = true;
			bNew = true;
		}
		else
			m_offerPaid[n].bDownloadedState = false;
	}

	return bNew;
}

bool XBoxLiveDownloadEnumerator::LaunchDownloading()
{
	if( m_offerFree.Size()==0 && m_offerPaid.Size()==0 ) return false;

	long n,i;
	HRESULT hRes;
	DWORD dwRes;
	DWORD dwUserIndex = 0;
	ULONGLONG* pOfferIDs = null;
	bool bSuccess = true;

	// загружаем бесплатные контенты
	if( m_offerFree.Size() > 0 )
	{
		pOfferIDs = NEW ULONGLONG[m_offerFree.Size()];
		Assert( pOfferIDs );
		for( n=i=0; n<m_offerFree; n++ )
		{
			// уже загруженные контенты не показываем
			if( m_offerFree[n].bDownloadedState ) continue;
			pOfferIDs[i++] = m_offerFree[n].offerInfo.qwOfferID;
		}

		dwRes = XShowMarketplaceDownloadItemsUI(dwUserIndex, XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_FREEITEMS,
			pOfferIDs, i, &hRes, NULL );

		if( dwRes != ERROR_SUCCESS )
			bSuccess = false;
		DELETE( pOfferIDs );
	}

	// загружаем платные контенты
	if( bSuccess && m_offerPaid.Size() > 0 )
	{
		pOfferIDs = NEW ULONGLONG[m_offerPaid.Size()];
		Assert( pOfferIDs );
		for( n=i=0; n<m_offerPaid; n++ )
		{
			// уже загруженные контенты не показываем
			if( m_offerPaid[n].bDownloadedState ) continue;
			pOfferIDs[i++] = m_offerPaid[n].offerInfo.qwOfferID;
		}

		dwRes = XShowMarketplaceDownloadItemsUI(dwUserIndex, XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_PAIDITEMS,
			pOfferIDs, i, &hRes, NULL );

		if( dwRes != ERROR_SUCCESS )
			bSuccess = false;
		DELETE( pOfferIDs );
	}

	return bSuccess;
}

bool XBoxLiveDownloadEnumerator::IsDone()
{
	if( IsEnd() ) return false;
	return XHasOverlappedIoCompleted(&m_overlapped);
}

bool XBoxLiveDownloadEnumerator::IsValideContent()
{
	if( IsEnd() ) return false;
	DWORD dwRes = XGetOverlappedResult(&m_overlapped,NULL,FALSE);
	if( dwRes == ERROR_SUCCESS ) return true;
	return false;
}

bool XBoxLiveDownloadEnumerator::IsEnd()
{
	if( INVALID_HANDLE_VALUE == m_hEnum ) return true;
	DWORD dwRes = XGetOverlappedResult(&m_overlapped,NULL,FALSE);
	if( dwRes == ERROR_NO_MORE_FILES )
		return true;
	if( dwRes == ERROR_SUCCESS || dwRes == ERROR_IO_INCOMPLETE )
		return false;

	dword dwExtErrCode = 0xFFFF & XGetOverlappedExtendedError(&m_overlapped);
	if( dwExtErrCode == ERROR_NO_MORE_FILES )
		return true;

	api->Trace ( "XBoxLive downloading enumeration error! error code=%u, extended error code=%u", dwRes, dwExtErrCode );
	return true;
}

void XBoxLiveDownloadEnumerator::End()
{
	CloseOverlapped();

	if( m_hEnum != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hEnum);
		m_hEnum = INVALID_HANDLE_VALUE;
	}
}

void XBoxLiveDownloadEnumerator::GetNextContentData()
{
	// получаем данные для очередного контента
	DWORD dwRes = XEnumerate(m_hEnum, &m_curEnumerateInfo, sizeof(XMARKETPLACE_CONTENTOFFER_INFO), NULL, &m_overlapped);
    if( dwRes != ERROR_IO_PENDING )
    {
		api->Trace("XEmumerate error");
    }
}

void XBoxLiveDownloadEnumerator::PrepareOverlapped()
{
	ZeroMemory( &m_overlapped, sizeof( m_overlapped ) );
	m_overlapped.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

void XBoxLiveDownloadEnumerator::CloseOverlapped()
{
	if( m_overlapped.hEvent != NULL )
	{
		CloseHandle(m_overlapped.hEvent);
		m_overlapped.hEvent = NULL;
	}
}

#endif
