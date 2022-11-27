

#pragma once

#include "..\SndBase.h"
#include "ProjectObject.h"

class ISound;
class ProjectSound;

class ProjectAnimation : public ProjectObject
{
	struct Movie;
public:
	enum Consts
	{
		//Сигнализация ошибочного значения
		invalidate_dword = 0xffffffff,
		//Глобальные константы
		c_nameLen = 128,
		c_pathLen = 256,
		c_uid_size = 16,
		c_vis_uid_size = c_uid_size*2 + 3 + 1,// XXXX-XXXX-XXXXXXXX-XXXXXXXXXXXXXXX_
		flags_lost_data_when_load =  0x00000001,
		flags_need_animation_check = 0x80000000,
		flags_init = 0,
		//Маленькое шифрование
		magic_global1 = 1664525,
		magic_global2 = 1013904223,
		magic_node1 = 22695477,
		magic_node2 = 1,
		magic_movie1 = 69069,
		magic_movie2 = 5,
		magic_event1 = 214013,
		magic_event2 = 2531011,
		magic_stop = 12345,
		//Идентификаторы загрузки
		file_actual_version = 100,
		file_error = 0,
		file_global = 1,
		file_node = 2,
		file_movie = 3,
		file_event = 4,		
		//Результат инициализации
		init_ok = 0,				//Всё хорошо
		init_er_empty_name,			//Пустое имя
		init_er_name_to_long,		//Слишком длинное имя
		init_er_already_in_prj,		//Уже есть такая анимация в проекте
		init_er_file_not_found,		//Анимационный Файл не найден
		init_er_files_to_many,		//Анимационных файлов с таким именем больше 1ого
		init_er_cant_open,			//Неполучилось открыть анимационный файл
		//Параметры событий
		event_flags_local = 0x1,	//Звук в локаторе		
		event_flags_bind = 0x2,		//При переходе с нода звук гаситься с фэйдом
		event_flags_voice = 0x4,	//Этот звук используеться для голоса
		event_tmp_flag_snd_id = 0x1000000,		
		event_tmp_flag_isloop =  0x2000000,
		event_tmp_flags_mask = 0xff000000,
	};	

	struct EventDataForCheck
	{
		dword frame;				//Кадр на котором установленно событие		
		dword eventParams;			//Параметры события
		float fadeTime;				//Время фэйда в секундах
		char sound[c_nameLen];		//Имя звука
		char locator[c_nameLen];	//Имя локатора, к которому привязываеться звук
		UniqId eventId;				//Уникальный идентификатор события
	};

	struct Event : public EventDataForCheck
	{
		dword position;				//Позиция для рисования
		Movie * movie;				//Ролик, в котором был создан эвент
		UniqId soundId;				//Идентификатор звука
		ISound * previewSound;		//Звук сервиса который надо проигрывать
	};


	//Сохранёный нод из анимационного графа
	struct PrjAnxNode
	{
		PrjAnxNode();
		~PrjAnxNode();
		dword nameOffset;			//Смещение имени нода в общей таблице
		dword nameLen;				//Размер имени нода (ускорение сохранения)
		dword clipIndex;			//Индекс используемого клипа
		dword clipFrames;			//Количество кадров в данном ноде
		dword clipsCount;			//Количество клипов в ноде
		float fps;					//Скорость проигрывания клипа
		UniqId nodeId;				//Уникальный идентификатор нода в проекте
		array<Event> events;		//События, привязанные к данному ноду
		bool isValidate;			//Такой нод существует в анимации
	};

	struct Node
	{
		dword index;				//Индекс анимационного нода на который ссылаеться эта запись
		dword framesCount;			//Количество кадров из нода, которые надо проиграть
		bool isEnable;				//Разрешено ли расставлять эвенты на ноде
	};

private:
	//Ролик
	struct Movie
	{
		Movie(const char * name);
		~Movie();

		ExtName name;				//Имя ролика
		char nameData[c_nameLen];	//Данные строки
		char misName[c_nameLen];	//Имя миссии в которой содержиться персонаж
		UniqId movieId;				//В каком ролике был проставлено событие
		dword stage;				//Состояние в проекте
		array<Node> nodes;			//Расставленые ноды ролика
		void * loadTmp;				//Для временного использования при загрузке
	};

	struct LoadPtr
	{		
		byte * data;
		dword index;
		dword size;
		dword type;
	};

public:
	ProjectAnimation(const UniqId & folderId);
	virtual ~ProjectAnimation();
	ErrorId SetAnimation(const char * anxFileName);

public:
	//Сохранить изменения
	void SaveChanges();
	//Получить имя анимации
	const char * GetAnimationName();
	//Получить путь до анимации
	const char * GetAnimationPath();
	//Проэкспортировать события в ини файл и сохранить путь экспотра для информирования
	bool ExportEventsToIni(string & exportPath, bool ignoreStages);

public:
	//Загрузить анимацию
	IAnimation * LoadAnimation();
	//Выгрузить анимацию
	void ReleaseAnimation();
	//Проверить анимацию на соответствие с текущим состоянием
	void CheckAnimation();
	//Получить наимение готовое состояние ролика или же ошибочное
	dword GetAnimationStage();

public:
	//Получить количество роликов
	dword GetMovieCount();
	//Добавить ролик
	dword AddMovie(const char * checkName, long copyFrom = -1);
	//Удалить ролик
	void DeleteMovie(dword index);
	//Проверить имя ролика на уникальность и правильность
	bool CheckMovieName(const char * name, long skipIndex);
	//Переименовать ролик
	void RenameMovie(dword index, const char * newName);
	//Получить имя ролика
	ExtName & GetMovieName(dword index);
	//Установить миссию для предпросмотра
	void SetMission(dword index, const char * missionName);
	//Получить имя миссии
	const char * GetMissionName(dword index);
	//Получить массив нодов ролика
	array<Node> & GetMovieNodes(dword index);
	//Получить стадию ролика
	dword GetMovieStage(dword index);
	//Установить стадию ролика
	void SetMovieStage(dword index, dword stage);

public:
	//Получить количество анимационных нодов
	dword GetNodesCount();
	//Получить анимационный нод
	const PrjAnxNode & GetNode(dword nodeIndex);
	//Получить имя нода
	void GetNodeName(dword nodeIndex, string & name);
	//Получить имя нода в анимации
	const char * GetAnxNodeName(dword nodeIndex);
	//Узнать валидность нода
	bool CheckNode(dword nodeIndex);
	//Добавить событие
	dword AddEvent(dword nodeIndex, dword frame, dword movieIndex, const UniqId * copyFromEventId = null);
	//Переместить событие на новый кадр
	void MoveEvent(dword nodeIndex, dword eventIndex, dword frame);
	//Поменять эвенты местами
	void ExchangeEvents(dword nodeIndex, dword eventIndex1, dword eventIndex2);
	//Установить позицию рисования эвента
	void SetEventPosition(dword nodeIndex, dword eventIndex, dword position);
	//Получить позицию рисования эвента
	dword GetEventPosition(dword nodeIndex, dword eventIndex);
	//Получить диапазон времён проигрывания волн эвента
	bool GetEventSoundTime(dword nodeIndex, dword eventIndex, float & minTime, float & maxTime);
	//Подготовить к проигрыванию звуки
	void NodePrepareForSoundPlay(dword nodeIndex, dword prepareFrames);
	//Подготовить к проигрыванию звук
	void EventPrepareForSoundPlay(dword nodeIndex, dword eventIndex);
	//Проиграть звук
	void EventSoundPlay(dword nodeIndex, dword eventIndex);
	//Остановить проигрывание звука
	void EventSoundStop(dword nodeIndex, dword eventIndex);
	//Закончился нод
	void EventSoundEndNode(dword nodeIndex, dword eventIndex);
	//Установить позицию звука для эвента
	void EventSoundPosition(dword nodeIndex, dword eventIndex, const Vector & pos);
	//Нарисовать звуки для внутренней звуковой сцены
	void EventSoundDraw();
	//Назначен зацикленный звук или нет
	bool EventSoundIsLoop(dword nodeIndex, dword eventIndex);
	//Получить имя локатора и тип звука
	const char * GetEventSoundParams(dword nodeIndex, dword eventIndex, bool * is3D = null, bool * isBind = null, bool * isVoice = null);
	//Установить параметры слушателя
	void EventsSetListener(const Vector & playerPos, const Vector & cameraPos);
	//Подготовить звуковую сцену для работы
	void EventsPrepareSounds();
	//Удалить все звуки, созданные событиями
	void EventsDeleteSounds();
	//Получить идентификатор эвента по индексу
	bool GetEventUIDbyIndex(dword nodeIndex, dword eventIndex, UniqId & uid);
	//Установить имя звука
	bool SetEventSoundName(dword nodeIndex, UniqId evtId, const char * name);
	//Получить имя звука
	const char * GetEventSoundName(dword nodeIndex, UniqId evtId);
	//Установить имя локатора
	bool SetEventLocator(dword nodeIndex, UniqId evtId, const char * locator);
	//Получить имя локатора
	const char * GetEventLocator(dword nodeIndex, UniqId evtId);
	//Установить время фэйда
	bool SetEventFadeTime(dword nodeIndex, UniqId evtId, float timeInSec);
	//Получить время фэйда
	float GetEventFadeTime(dword nodeIndex, UniqId evtId);
	//Установить/сбросить заданные флажки
	bool SetEventFlags(dword nodeIndex, UniqId evtId, dword flags, bool isSet);
	//Получить флажки
	dword GetEventFlags(dword nodeIndex, UniqId evtId);
	//Удалить событие
	void DeleteEvent(dword nodeIndex, dword eventIndex);
	//Удалить события принадлежащие ролику
	void DeleteEvents(dword nodeIndex, dword movieIndex);

private:
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * filePath);
	//Сохранить анимационный нод
	void SaveNode(dword index, const PrjAnxNode & node);
	//Сохранить ролик
	void SaveMovie(const Movie & movie);
	//Сохранить событие
	void SaveEvent(const Event & evt, const UniqId & nodeId);
	//Сохранить непреобразованные данные
	void SaveRaw(const char * src, dword size);
	//Сохранить непреобразованный идентификатор c подчёркеванием для чтения
	void SaveUId(const UniqId & id);
	//Сохранить число
	void SaveDword(dword d, dword enc1, dword enc2, dword & counter);
	//Сохранить данные
	void SaveData(const void * ptr, dword size, dword enc1, dword enc2, dword & counter);
	//Сделать шаг кодировки
	byte EncodeStep(byte b, const dword enc1, const dword enc2, dword & counter);
	//Сделать шаг разкодировки
	byte DecodeStep(byte b, const dword enc1, const dword enc2, dword & counter);
	//Сконвертировать число в текст
	void EncodeConvert(byte * dst, byte b);
	//Сконвертировать текст (2 символа) в число
	bool DecodeConvert(byte * data, byte & b);

private:
	//Первоначальная загрузка при старте редактора
	virtual ErrorId OnLoadObject(const char * defaultFilePath);
	//Загрузить имя и путь
	ErrorId LoadGlobal(LoadPtr & lprt, const char * name, const char * defaultFilePath);
	//Загрузить анимационный нод
	ErrorId LoadNode(LoadPtr & lprt, const char * defaultFilePath);
	//Загрузить ролик
	ErrorId LoadMovie(LoadPtr & lprt, const char * defaultFilePath);
	//Загрузить эвент
	ErrorId LoadEvent(LoadPtr & lprt, const char * defaultFilePath);
	//Декодировать блок данных, возвращает стадию апрува
	dword LoadDecode(LoadPtr & lprt, const dword enc1, const dword enc2, const dword stateFrom, const dword stateTo);
	//Забрать из чанка число
	bool LoadDword(LoadPtr & lprt, dword & dw);
	//Забрать из чанка данные
	bool LoadData(LoadPtr & lprt, void * to, dword size);
	//Поверить, были ли изменены события
	bool LoadMovieEventsCheck(LoadPtr & lprt, dword movieIndex);

private:
	//Изменить стадию ролика на требующую проверки
	void MoviesMarkStage(dword nodeIndex, dword stage);
	//Найти в ноде эвент
	Event * FindEvent(dword nodeIndex, const UniqId & eventId);
	//Загрузить звук для события
	void EventPrepareSound(Event & evt, bool isCreateISound);
	//Удалить звук события
	void EventDeleteSound(Event & evt);
	//Получить звук для события
	ProjectSound * EventGetSound(Event & evt);
	//Написать сообщение об изменении стадии ролика
	void ChangeMovieStageLog(dword movieIndex, const char * fromChange);



private:
	char anxName[c_nameLen];	//Имя anx файла
	char anxPath[c_pathLen];	//Относительный путь до anx файла
	array<Movie *> movies;		//Ролики данного anx
	array<PrjAnxNode> nodes;	//Анимационные ноды
	array<char> nodeNames;		//Имена нодов
	IAnimation * ani;			//Загруженая анимация
	dword flags;				//Флаги
	dword validateCounter;		//Счётчик для проверки валидности длин времени эвентов
	ISoundScene * scene;		//Сцена для прослушивания звуков
	char locatorName[c_nameLen];//Имя локатора, к которому привязываеться звук для последнего изменения
	
private:	
	static dword magicStage[movie_stage_last + 1];
	static const char * fileGlobalId;
	static const char * fileNodeId;
	static const char * fileMovieId;
	static const char * fileEventId;
};











