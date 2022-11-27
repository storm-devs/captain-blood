#include "ContentGUIUpdater.h"
#include "defines.h"

XContentGUIUpdater::XContentGUIUpdater(ContentManager* pManager, ILiveContent* pContent, long nContentIndex)
{
	m_pManager = pManager;
	m_pContent = pContent;
	m_nContentIndex = nContentIndex;

	m_bUpdate = false;
	m_pPicWidget = NULL;
}

XContentGUIUpdater::~XContentGUIUpdater()
{
	StopPictureProcessing();
}

void XContentGUIUpdater::Update()
{
	if( m_bUpdate )
	{
		// закончили получение картинки - ставим ее
		if( !m_pContent->IsPicTextureGetProcessing() )
		{
			ITexture* pPic = m_pContent->GetPicTexture();
			SetGUIPicture(pPic, "Error");
			m_bUpdate = false;
		}
	}
}

void XContentGUIUpdater::StartUpdater(MissionObject* pPicWidget, MissionObject* pDescrWidget)
{
	if( !m_pContent ) return;

	// описание контента
	if( pDescrWidget && m_pContent->GetDisplayName() )
	{
		MO_IS_IF(tid, "GUIWidget", pDescrWidget)
		{
			char param[1024];
			crt_snprintf( param,sizeof(param), "%s", m_pContent->GetDisplayName() );
			pDescrWidget->Command("SetString",1,(const char**)&param);
		}
	}

	// картинка контента
	m_bUpdate = false;
	if( pPicWidget )
	{
		m_pPicWidget = pPicWidget;

		ITexture* pPic = m_pContent->GetPicTexture();
		if( pPic )
		{
			SetGUIPicture(pPic, "");
		}
		else
		{
			m_bUpdate = true;
			SetGUIPicture(NULL, "Downloading");
		}
	}
}

void XContentGUIUpdater::StopPictureProcessing()
{
	if( m_pContent )
	{
		m_pContent->ReleasePicTexture();
	}
	SetGUIPicture(NULL, "Downloading");
	m_bUpdate = false;
}

void XContentGUIUpdater::SetGUIPicture(ITexture* pTexture, const char* pcDefTextureName)
{
	if( !m_pPicWidget ) return;

	ITexture* pTex = pTexture;

	if( !pTex )
	{
		IRender* pRender = (IRender*)api->GetService("DX9Render");
		pTex = (ITexture*)pRender->CreateTexture(_FL_,"%s",pcDefTextureName);
	}

	if( pTex )
	{
		; // set texture
	}

	if( pTex != pTexture )
	{
		RELEASE( pTex );
	}
}
