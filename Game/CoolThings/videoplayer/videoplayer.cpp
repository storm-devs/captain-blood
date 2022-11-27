#include "videoplayer.h"


VideoPlayer::VideoPlayer()
{
	pCodec = 0;
	bEditModePause = true;
	bLoopPlay = false;
	bStartUpActivate = false;
	bFirstFrame = true;
	bBeforeGeom = false;
	bPlayByRestore = false;
	bPause = false;
}

VideoPlayer::~VideoPlayer()
{
	if( pCodec ) delete pCodec;
}


//Создание объекта
bool VideoPlayer::Create(MOPReader & reader)
{
	EditMode_Update (reader);
	MissionObject::Activate(false);

	if (bBeforeGeom)
	{
		SetUpdate(&VideoPlayer::Realize, ML_FILL+1); // рисуемся сразу после фона
	} else
	{
		SetUpdate(&VideoPlayer::Realize, ML_GUI5+1); // рисуемся после интерфейсов
	}
	bFirstFrame = true;

	return true;
}

void VideoPlayer::PostCreate()
{
}

//Обновление параметров
bool VideoPlayer::EditMode_Update(MOPReader & reader)
{
	xpos = reader.Float() * 0.01f;
	ypos = reader.Float() * 0.01f;
	width = reader.Float() * 0.01f;
	height = reader.Float() * 0.01f;
	sFileName = reader.String().c_str();
	bEditModePause = reader.Bool();

	bBeforeGeom = reader.Bool();

	if( !EditMode_IsOn() ) bEditModePause = false;

	triggerEnd.Init( reader );

	bLoopPlay = reader.Bool();

	bStartUpActivate = reader.Bool();
	bFirstFrame = true;

	return true;
}


void _cdecl VideoPlayer::Realize(float fDeltaTime, long level)
{
	// создаем плеер если надо - на первом кадре
	if( bFirstFrame && api->IsActive() )
	{
		bFirstFrame = false;
		Activate(bStartUpActivate);
		if( pCodec && bPause )
			pCodec->Pause(bPause);
	}

	if( pCodec && IsActive() )//&& !bPause )
	{
		if( !pCodec->Frame(bEditModePause || bPause) )
		{
			Activate(false);
			triggerEnd.Activate( Mission(), false );
		}
	}
}


void VideoPlayer::Activate(bool isActive)
{
	if( !sFileName || !sFileName[0] )
	{
		LogicDebug("Video player have empty file name. Force deactivate.");
		isActive = false;
	}

	if( bFirstFrame )
	{
		bStartUpActivate = isActive;
		LogicDebug("Set active state before first frame. Switch initial activate to %s", isActive?"activate":"deactivate");
		return;
	}

	if( isActive == IsActive() )
	{
		LogicDebug("Video player already %s", isActive?"activated":"deactivated");
	}
	MissionObject::Activate(isActive);

	if( isActive )
	{
		StartVideo();
		Registry(ACTIVATE_EVENT_GROUP, &VideoPlayer::ActivateEvent, 0);
		Registry(DEACTIVATE_EVENT_GROUP, &VideoPlayer::DeactivateEvent, 0);
	}
	else
	{
		FinishVideo();
		Unregistry(ACTIVATE_EVENT_GROUP);
		Unregistry(DEACTIVATE_EVENT_GROUP);
	}
}

void _cdecl VideoPlayer::ActivateEvent(const char * group, MissionObject * sender)
{
	//Activate(bStoredActivate);
	if( pCodec && bPlayByRestore )
		pCodec->Pause(false);

	bPause = false;
}

//Выполнить событие деактивации
void _cdecl VideoPlayer::DeactivateEvent(const char * group, MissionObject * sender)
{
	//bStoredActivate = IsActive();
	//Activate(false);
	bPlayByRestore = false;
	if( pCodec )
	{
		bPlayByRestore = pCodec->IsProcessed();
		pCodec->Pause(true);
	}

	bPause = true;
}

void VideoPlayer::StartVideo()
{
	// создадим кодек если его нет
	if( !pCodec )
		pCodec = Render().CreateVideoCodec(true);

	// если кодек есть то устанавливаем для него данные
	if( pCodec )
	{
		// откроем файл в кодеке
		char fullname[MAX_PATH];
		#ifdef _XBOX
			crt_snprintf(fullname, sizeof(fullname), "GAME:\\%s", sFileName);
		#else
			crt_snprintf(fullname, sizeof(fullname), "resource\\videos\\%s", sFileName);
		#endif
		if( !pCodec->IsCurrentFile(fullname) )
		{
			//pCodec->OpenFile(fullname);
			// открываем файл с опцией зацикленное проигрывание или нет
			pCodec->OpenFile(fullname,bLoopPlay);
		}

		// назначим область вывода для видео картинки
		pCodec->SetViewPosition(xpos,ypos,width,height,true);
		// запустим кодек
		pCodec->Start();
	}
	else
		// при ошибке создания проигрывателя видео - мы запускаем тригер, будто бы видео завершилось
		// так как в обработке на кадре данный тригер срабатывает только при наличии кодека (а у нас его даже нет)
		triggerEnd.Activate( Mission(), false );
}

void VideoPlayer::FinishVideo()
{
	if( pCodec )
	{
		pCodec->Stop();
		delete pCodec;
		pCodec = 0;
	}
}

void VideoPlayer::Show(bool isShow)
{
	if( isShow == IsShow() )
		return;

	MissionObject::Show(isShow);

/*	if( bFirstFrame )
		return;

	if( isShow )
		StartVideo();
	else
		FinishVideo();*/
}


MOP_BEGINLISTG(VideoPlayer, "VideoPlayer", '1.00', 100, "Interface")
	MOP_FLOATEXC ("X Pos", 0.0f, 0.0f, 100.0f, "Позиция левого верхнего угла для отображния видео");
	MOP_FLOATEXC ("Y Pos", 0.0f, 0.0f, 100.0f, "Позиция левого верхнего угла для отображния видео");
	MOP_FLOATEXC ("Width", 100.0f, 0.0f, 100.0f, "Ширина окна для отображния видео");
	MOP_FLOATEXC ("Height", 100.0f, 0.0f, 100.0f, "Высота окна для отображния видео");
	MOP_STRINGC ("VideoFileName", "", "Имя видео файла");
	MOP_BOOLC ("EditMode Pause", true, "Пауза. Работает только в режиме редактора миссии");
	MOP_BOOLC ("Before geom", false, "Отрисовывать видео проигрыватель до геометрии, для композинга видео это");
	MOP_MISSIONTRIGGER("End Video Trigger");
	MOP_BOOL ("Loop play", false);
	MOP_BOOL ("Active", true);
MOP_ENDLIST(VideoPlayer)
