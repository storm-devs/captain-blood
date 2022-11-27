#include "contenttest.h"
#include "contentmanager.h"
#include "ContentGUIUpdater.h"

CREATE_SERVICE(ContentManager, 120)

ContentTest::ContentTest() :
	m_aGUIUpdater(_FL_)
{
	m_pContentManager = (IContentManager*)api->GetService("ContentManager");
	Assert( m_pContentManager );
}

ContentTest::~ContentTest()
{
}

bool ContentTest::Create(MOPReader & reader)
{
	m_pcWidgetPicture = reader.String().c_str();
	m_pcWidgetDescribe = reader.String().c_str();

	SetUpdate(&ContentTest::Realize, ML_GUI1-1);

	return true;
}

void _cdecl ContentTest::Realize(float fDeltaTime, long level)
{
	for( long n=0; n<m_aGUIUpdater; n++ )
		m_aGUIUpdater[n]->Update();
}

void ContentTest::Command(const char * id, dword numParams, const char ** params)
{
	// если неактивны, то не принимаем команды
	if( !IsActive() ) return;
	// нет комманды нет и действий
	if( !id ) return;

	// подключаем контент
	if( string::IsEqual(id,"OnContent") )
	{
		if( numParams < 1 )
			api->Trace("ContentTest::Command() : Missing params quantity (command = %s, numParams = %d)", id, numParams);
		else
			PlugContent(atoi(params[0]), true);
	}
	// отключаем контент
	else if( string::IsEqual(id,"OffContent") )
	{
		if( numParams < 1 )
			api->Trace("ContentTest::Command() Missing params quantity (command = %s, numParams = %d)", id, numParams);
		else
			PlugContent(atoi(params[0]), false);
	}
	// устанавливаем данные для контента
	else if( string::IsEqual(id,"SetData") )
	{
		if( numParams < 1 )
			api->Trace("ContentTest::Command() Missing params quantity (command = %s, numParams = %d)", id, numParams);
		else
			SetContentData(atoi(params[0]));
	}
	else if( string::IsEqual(id,"ResetData") )
	{
		if( numParams < 1 )
			api->Trace("ContentTest::Command() Missing params quantity (command = %s, numParams = %d)", id, numParams);
		else
			ResetContentData(atoi(params[0]));
	}
}

void ContentTest::PlugContent(long nContentIndex, bool bPlugIn)
{
	if( nContentIndex < m_pContentManager->GetContentQuantity() )
	{
		if( bPlugIn )
			m_pContentManager->PlugContent( nContentIndex );
		else
			m_pContentManager->UnplugContent( nContentIndex );
	}
	else
		api->Trace("ContentTest::PlugContent(%d,%s) : Incorrect content index where contents quantity only %d", nContentIndex, bPlugIn?"true":"false", m_pContentManager->GetContentQuantity() );
}

void ContentTest::SetContentData(long nContentIndex)
{
	if( nContentIndex<0 || nContentIndex>=m_pContentManager->GetContentQuantity() )
		return;

	// уже есть апдейтер - не надо ничего устанавливать
	XContentGUIUpdater* pUpdater = FindUpdater(nContentIndex);
	if( pUpdater )
		return;

	// заводим новый апдейтер
	pUpdater = NEW XContentGUIUpdater((ContentManager*)m_pContentManager, ((ContentManager*)m_pContentManager)->GetContentByIndex( nContentIndex ), nContentIndex);
	Assert(pUpdater);
	m_aGUIUpdater.Add(pUpdater);

	// ищем элементы интерфейса для апдейтера
	char temp[1024];
	// виджет для картинки
	MOSafePointer pWidgetPicture;
	if( m_pcWidgetPicture )
	{
		crt_snprintf( temp,sizeof(temp), "%s_%d", m_pcWidgetPicture,nContentIndex );
		FindObject(ConstString(temp), pWidgetPicture);
	}
	// виджет для описания
	MOSafePointer pWidgetDescribe;
	if( m_pcWidgetDescribe )
	{
		crt_snprintf( temp,sizeof(temp), "%s_%d", m_pcWidgetDescribe,nContentIndex );
		FindObject(ConstString(temp), pWidgetDescribe);
	}

	// устанавливаем виджеты в апдейтер
	pUpdater->StartUpdater(pWidgetPicture.Ptr(), pWidgetDescribe.Ptr());
}

void ContentTest::ResetContentData(long nContentIndex)
{
	for( long n=0; n<m_aGUIUpdater; n++ )
		if( m_aGUIUpdater[n]->GetContentIndex() == nContentIndex )
		{
			DELETE( m_aGUIUpdater[n] );
			m_aGUIUpdater.DelIndex( n );
		}
}

XContentGUIUpdater* ContentTest::FindUpdater(long nContentIndex)
{
	for( long n=0; n<m_aGUIUpdater; n++ )
		if( m_aGUIUpdater[n]->GetContentIndex() == nContentIndex )
			return m_aGUIUpdater[n];
	return NULL;
}


static char GUIDescription[] =
"Downloadable XLive content manager.\n"
"Commands:\n"
"OnContent <number> - plugin content by index\n"
"OffContent <number> - plugoff content by index\n"
"SetData <number> - set content data (picture & text) into GUI\n"
"ResetData <number> - reset content data (picture & text) into GUI (free memory)";

MOP_BEGINLISTCG(ContentTest, "XLive content manager", '1.00', 100, GUIDescription, "Managment")
	MOP_STRINGC ("Widget for picture", "", "result GUI name as \"<this string>_<content number>\"");
	MOP_STRINGC ("Widget for describe text", "", "result GUI name as \"<this string>_<content number>\"");
MOP_ENDLIST(ContentTest)
