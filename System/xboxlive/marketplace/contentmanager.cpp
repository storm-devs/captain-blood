#include "contentmanager.h"

#ifdef _XBOX
	#include "XBox\content.h"
	#include "XBox\ContentEnumerator.h"
	#include "XBox\DownloadEnumerator.h"
#else
	//#include "PC\LiveService.h"
	#include "..\LiveService.h"
	#include "PC\PCContentEnumerator.h"
#endif

ContentManager::ContentManager() :
	m_ContentsList(_FL_)
{
	m_pLiveService = null;
	m_pContentEnumerator = null;
	m_pDownloadEnumerator = null;
	m_dwTitleID = 0x11111111;
	m_curStatus = IContentManager::ContentStatus_NotReady;
}

ContentManager::~ContentManager()
{
	DELETE( m_pContentEnumerator );
	ReleaseContents();
	DELETE( m_pDownloadEnumerator );
	DELETE( m_pLiveService );
}

bool ContentManager::Init()
{
	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);

#ifndef _XBOX
/*	m_pLiveService = NEW LiveService;
	if( m_pLiveService )
	{
		if( !m_pLiveService->Init() )
		{
			api->Trace( "%s", m_pLiveService->GetErrorText() );
			DELETE( m_pLiveService );
		}
	}*/
#endif

	UpdateContentList();
	UpdateDownloadList();

	return true;
}

void ContentManager::StartFrame(float dltTime)
{
	// Если идет процесс енумерации, то проверяем его состояние
	if( m_pContentEnumerator )
		Enumerate_ContentFrame();

	if( m_pDownloadEnumerator )
	{
		m_pDownloadEnumerator->Frame();
	}
}

IContentManager::ContentStatus ContentManager::GetStatus()
{
	return m_curStatus;
}

void ContentManager::UpdateDownloadList()
{
	m_curStatus = IContentManager::ContentStatus_NotReady;

	if( !m_pDownloadEnumerator )
	{
		// создадим енумератор
		bool bCreated = false;
#ifdef _XBOX
		m_pDownloadEnumerator = NEW XBoxLiveDownloadEnumerator();
		bCreated = true;
#else
		m_pDownloadEnumerator = null;
#endif
		if( !m_pDownloadEnumerator )
		{
			if( bCreated )
				api->Trace("Can`t create download enumerator");
			return;
		}

		// запустим енумератор
		m_pDownloadEnumerator->Start( m_dwTitleID );
	}
}

void ContentManager::UpdateContentList()
{
	m_curStatus = IContentManager::ContentStatus_NotReady;

	if( !m_pContentEnumerator )
	{
		// создадим енумератор
		bool bCreated = false;
#ifdef _XBOX
		m_pContentEnumerator = NEW XBoxLiveContentEnumerator();
		bCreated = true;
#else
		//m_pContentEnumerator = NEW PCLiveContentEnumerator(m_pLiveService);
#endif
		if( !m_pContentEnumerator )
		{
			if( bCreated )
				api->Trace("Can`t create content enumerator");
			return;
		}

		// запустим енумератор
		Enumerate_PrepareContents();
		m_pContentEnumerator->Start();
	}
}

bool ContentManager::GetContentDescribe(long nContent,ContentDescribe& descr)
{
	if( nContent >= GetContentQuantity() ) return false;

	if( descr.flags & ContentDescribe::cdf_PlugState )
		descr.bPlugState = m_ContentsList[nContent]->IsPluged();
	if( descr.flags & ContentDescribe::cdf_DisplayName )
		descr.pcDisplayName = m_ContentsList[nContent]->GetDisplayName();
	if( descr.flags & ContentDescribe::cdf_ImageTexture )
		descr.pcDisplayName = 0;//m_ContentsList[nContent]->GetDisplayName();
	return true;
}

bool ContentManager::PlugContent(long nContent)
{
	if( nContent >= GetContentQuantity() ) return false;
	m_ContentsList[nContent]->PlugContent();
	return true;
}

bool ContentManager::UnplugContent(long nContent)
{
	if( nContent >= GetContentQuantity() ) return false;
	m_ContentsList[nContent]->UnplugContent();
	return true;
}

void ContentManager::ReleaseContents()
{
	m_ContentsList.DelAllWithPointers();
}

void ContentManager::Enumerate_ContentFrame()
{
	// больше нет контентов
	if( m_pContentEnumerator->IsEnd() )
	{
		// енумератор уже не нужен
		DELETE( m_pContentEnumerator );
		// обработаем весь список контентов
		Enumerate_ProcessingContents();

		// свяжем окончательный список со спсиком готовых для загрузки контентов
		if( m_pDownloadEnumerator )
		{
			if( m_pDownloadEnumerator->IsCompliteEnumerate() )
			{
				m_pDownloadEnumerator->CheckNewItem( m_ContentsList.GetBuffer(), m_ContentsList.Size() );
			}
		}
		return;
	}

	// описание контента получено?
	if( m_pContentEnumerator->IsDone() )
	{
		if( m_pContentEnumerator->IsValideContent() )
		{
			XContentData cd;
			m_pContentEnumerator->GetContentData(cd);
			Enumerate_AddContent(cd);
		}
		m_pContentEnumerator->Next();
		return;
	}
}

void ContentManager::Enumerate_PrepareContents()
{
	// пробежимся по всем контентам и отметим их как несуществующие
	for(long n=0; n<m_ContentsList; n++)
		m_ContentsList[n]->MarkAsUnchecked();
}

void ContentManager::Enumerate_AddContent(const XContentData& cd)
{
	// имя файла содержащего контент приводим к человеческому виду (заканчивается на 0)
	const char* filename = m_pContentEnumerator->GetCurContentName();

	// если такой контент уже есть, то не добавляем его
	// просто отметим его как существующий
	for(long n=0; n<m_ContentsList; n++)
		if( m_ContentsList[n]->GetContentName() && strcmp(m_ContentsList[n]->GetContentName(), filename) == 0 )
		{
			m_ContentsList[n]->DoCheck();
			return;
		}

	api->Trace("Add content: \"%s\"", filename);

	// получим уникальный идентификатор для нового контента
	long nUniqID = m_ContentsList.Size();

	// создаем новый контент
	ILiveContent* pCnt = null;
#ifdef _XBOX
	pCnt = (ILiveContent*)(NEW XBoxLiveContent(cd.dat,nUniqID));
#endif
	if( !pCnt )
	{
		api->Trace("Can`t create content");
		return;
	}

	// отметим его как существующий
	pCnt->DoCheck();

	// заносим контент в список созданных контентов
	m_ContentsList.Add( pCnt );

	// проверяем надо ли подключать контент
	UpdatePlugedStateForContent( pCnt );
}

void ContentManager::Enumerate_ProcessingContents()
{
	// удалим несуществующие контенты
	for(long n=0; n<m_ContentsList; n++)
	{
		if( !m_ContentsList[n]->IsChecked() )
		{
			if( m_ContentsList[n]->IsPluged() )
				m_ContentsList[n]->UnplugContent();
			DELETE( m_ContentsList[n] );
			m_ContentsList.DelIndex(n);
			n--;
			continue;
		}
	}
}

void ContentManager::UpdatePlugedStateForContent(ILiveContent* pCnt)
{
	if( !pCnt ) return;

	switch ( CheckPlugedState(pCnt) )
	{
	case ps_Unassigned:
		SetPlugedStateForConetnt(pCnt,true);
		// дальше работаем так как если бы он был подключен
	case ps_Pluged:
		if( !pCnt->IsPluged() )
			pCnt->PlugContent();
		break;
	case ps_Unpluged:
		if( pCnt->IsPluged() )
			pCnt->UnplugContent();
		break;
	}
}

ContentManager::PlugedState ContentManager::CheckPlugedState(ILiveContent* pCnt)
{
	if( !pCnt ) return ps_Unassigned;

	/*
	// Получить сервис сохранения состояния игры
	IGameState* pGS = (IGameState*)api->GetService("GameState");
	Assert(pGS);

	// Получить группу состояний контентов
	IStateGroup* pSG = pGS->GetGroup("Contents");
	if( pSG )
	{
		long nVal = pSG->GetLong(pCnt->GetContentName(),-1);
		if( nVal == 1 )
			return ps_Pluged;
		if( nVal == 0 )
			return ps_Unpluged;
	}*/

	return ps_Unassigned;
}

void ContentManager::SetPlugedStateForConetnt(ILiveContent* pCnt,bool bPlugState)
{
	if( !pCnt ) return;
/*
	// Получить сервис сохранения состояния игры
	IGameState* pGS = (IGameState*)api->GetService("GameState");
	Assert(pGS);

	// Получить группу состояний контентов
	IStateGroup* pSG = pGS->GetGroup("Contents");
	if( pSG )
	{
		pSG->SetLong ( pCnt->GetContentName(), bPlugState ? 1 : 0 );
	}
	*/
}
