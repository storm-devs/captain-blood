#ifdef _XBOX

#include "content.h"
#include "..\..\..\..\common_h\mission.h"

XBoxLiveContent::XBoxLiveContent(const XCONTENT_DATA& cd, long nID)
{
	// 0-3 ИД пользователя, определяется подключенным контроллером
	m_dwUserIndex = 0;

	m_nUniqueID = nID;
	m_cdat = cd;
	m_bPlug = false;
	m_pMirrorPath = NULL;

	m_pPicTexture = NULL;
	m_pbThumbnailImage = NULL;
	ZeroMemory( &m_GetPictureOverlapped, sizeof( m_GetPictureOverlapped ) );

	// создаем правильное имя контента (по имени файла)
	//memcpy(m_pcContentName, cd.szFileName, XCONTENT_MAX_FILENAME_LENGTH);
    //m_pcContentName[XCONTENT_MAX_FILENAME_LENGTH] = 0;
}

XBoxLiveContent::~XBoxLiveContent()
{
	UnplugContent();
	ReleasePicTexture();
}

bool XBoxLiveContent::PlugContent()
{
	// уже подключен
	if( IsPluged() ) return true;

	DWORD dwResult;

	// задаем путь на который монтируем данный контент
	_snprintf(m_pcRootName,sizeof(m_pcRootName),"content%d",m_nUniqueID);

	// монтируем контент на устройство заданное в параметре pcRootName.
	// можно использовать и другую функцию XContentCreateEx, она позволяет задать размер кеша для читаемых файлов,
	// что полезно в основном для файлов с большим требованием к стриму (видео например),
	// но в нашем случае нет в ней необходимости.
	dwResult = XContentCreate(m_dwUserIndex, m_pcRootName, &m_cdat, XCONTENTFLAG_OPENEXISTING, NULL, NULL, NULL);
	if( dwResult != ERROR_SUCCESS ) return false;

	// производим отражение полученного устройства в рабочую директорию игры
	IFileService * fs = (IFileService *)api->GetService("FileService");
	char pcPath[sizeof(m_pcRootName)+2];
	sprintf(pcPath,"%s:\\",m_pcRootName);
	m_pMirrorPath = fs->CreateMirrorPath(pcPath, "game:\\", _FL_);

	// устанавливаем флаг удачного подключения
	m_bPlug = true;

	return true;
}

void XBoxLiveContent::UnplugContent()
{
	if(!m_bPlug) return;

	// закрыть отражение директории контента на директорию игры
	RELEASE(m_pMirrorPath);

	// закрываем контент
	XContentClose(m_pcRootName,NULL);

	// сбрасываем флаг удачного подключения
	m_bPlug = false;
}

const char* XBoxLiveContent::GetDisplayName()
{
	if( 0 != wcstombs_s( NULL, m_pcDisplayName, sizeof(m_pcDisplayName), m_cdat.szDisplayName, sizeof(m_cdat.szDisplayName) ) )
	{
		return NULL;
	}
	return m_pcDisplayName;
}

ITexture* XBoxLiveContent::GetPicTexture()
{
	if( m_pPicTexture ) return m_pPicTexture;

	if( m_pbThumbnailImage )
	{
		DWORD dwRes = XGetOverlappedResult(&m_GetPictureOverlapped,NULL,FALSE);
		// продолжаем считывать картинку
		if( dwRes == ERROR_IO_INCOMPLETE )
			return NULL;
		// успешно считали картинку
		if( dwRes == ERROR_SUCCESS )
		{
			// get texture from memory
			m_pPicTexture = NULL;
			/*IRender::CreateTextureTGA(
			// Convert thumbnail image from PNG to texture
			if( FAILED( D3DXCreateTextureFromFileInMemory( m_pd3dDevice, m_pbThumbnailImage,
						m_dwThumbnailBytes, NULL )))//&m_pPicTexture ) ) )
			{
				api->Trace( "XBoxLiveContent::GetPicTexture: D3DXCreateTextureFromFileInMemory failed.\n" );
				return NULL;
			}*/
		}
		// закрываем
		CloseGetPictureOverlapped();
	}
	else
	{
		CloseGetPictureOverlapped();

		// Retreive the size of the thumbnail image
		if( XContentGetThumbnail( m_dwUserIndex, &m_cdat, NULL, 
								&m_dwThumbnailBytes, NULL ) != ERROR_SUCCESS )
		{
			api->Trace( "XBoxLiveContent::GetPicTexture: Failed to get thumbnail image size.\n" );
			return NULL;
		}

		// Allocate memory and retreive the actual image
		m_pbThumbnailImage = NEW BYTE[m_dwThumbnailBytes];
		if( m_pbThumbnailImage == NULL )
		{
			api->Trace( "XBoxLiveContent::GetPicTexture: Failed to allocate memory.\n" );
			return NULL;
		}

		// prepare overlapped
		m_GetPictureOverlapped.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		if( m_GetPictureOverlapped.hEvent == NULL )
		{
			CloseGetPictureOverlapped();
			api->Trace( "XBoxLiveContent::GetPicTexture: Failed to prepare overlapped.\n" );
			return NULL;
		}

		//if( XContentGetThumbnail( m_dwUserIndex, &m_cdat, pbThumbnailImage,
		//						&dwThumbnailBytes, NULL ) != ERROR_SUCCESS )
		if( XContentGetThumbnail( m_dwUserIndex, &m_cdat, m_pbThumbnailImage,
			&m_dwThumbnailBytes, &m_GetPictureOverlapped ) != ERROR_IO_PENDING )
		{
			CloseGetPictureOverlapped();
			api->Trace( "XBoxLiveContent::GetPicTexture: Failed to get thumbnail image.\n" );
			return NULL;
		}
	}
	return m_pPicTexture;
}

bool XBoxLiveContent::IsPicTextureGetProcessing()
{
	if( m_pPicTexture ) return false;
	if( m_pbThumbnailImage && m_GetPictureOverlapped.hEvent != NULL ) return true;
	return false;
}

void XBoxLiveContent::ReleasePicTexture()
{
	if( m_pPicTexture )
	{
		m_pPicTexture->Release();
		m_pPicTexture = NULL;
	}

	CloseGetPictureOverlapped();
}

bool XBoxLiveContent::IsContentMatching(CONST BYTE * pbContentID)
{
	return TRUE == XMarketplaceDoesContentIdMatch(pbContentID, &m_cdat);
}

void XBoxLiveContent::CloseGetPictureOverlapped()
{
	if( m_GetPictureOverlapped.hEvent != NULL )
	{
		CloseHandle(m_GetPictureOverlapped.hEvent);
		m_GetPictureOverlapped.hEvent = NULL;
	}

	if( m_pbThumbnailImage )
	{
		delete [] m_pbThumbnailImage;
		m_pbThumbnailImage = NULL;
	}
}

#endif
