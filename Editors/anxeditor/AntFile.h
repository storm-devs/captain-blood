//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// AnxEditor
//===========================================================================================================================
// AntFile
//============================================================================================

#ifndef _AntFile_h_
#define _AntFile_h_

#include "MasterSkeleton.h"

struct AntFileHeader;
struct AntFileBone;
struct AntFileTrackElement;

class AntFile
{
	struct Bone
	{
		long track;						//Смещение до трека анимации кости от начала данных о треках
	};

	struct TrackAccessor
	{
		TrackAccessor(byte * base, long offset, long firstFrame, MasterSkeleton::Bone * b);
		float operator () (dword frame, AnxTrackHeaderBase::Flags index);
		float * Get(AnxTrackHeaderBase::Flags index);

	private:
		AntFileTrackElement * track;
		MasterSkeleton::Bone * bone;
	};

	struct TempTrack
	{
		float vFloat;
		word v16;
		byte v8;
		byte tmp;
		dword v24;
	};

	struct ComponentInfo
	{		
		ComponentInfo();
		TempTrack * track;
		dword shift;
		AnxTrackHeaderBase::Flags type;
		dword size;
		dword offset;
		AnxTrackHeaderBase::Flags index;
	};

	struct TrackInfo
	{
		dword desc;
		byte * ptr;
	};
	

//--------------------------------------------------------------------------------------------
public:
	AntFile(AnxOptions & options);
	virtual ~AntFile();

//--------------------------------------------------------------------------------------------
public:
	//Загрузить файл
	bool Load(const char * fileName);
	//Адаптировать данные на текущий мастер-скелет
	void ApplyMasterSkeleton(string * errorMessage = null);

	//Получить из файла мастер-скелет
	MasterSkeleton * CreateMasterSkeleton();

	//Записать данные в поток
	void Write(AnxStream & stream);
	//Прочитать данные из потока
	void Read(AnxStream & stream);

	//Выгрузить данные в клип
	dword ExportData(AnxTrackHeader * bones, AnxMvTrackHeader * mt, bool globalPos, byte * data, array<byte> & buffer);
	//Упаковать компоненту трека
	static void PackTrackComponent(const void * src, dword pitchInBytes, dword count, ComponentInfo & cmp, AnxTrackHeaderBase::Range & range, float error);
	//Разместить трек в буфере
	TrackInfo SetTrack(ComponentInfo * cmp, dword count, dword numFrames, byte * data, dword & size);
	//Разместить элемент трека в нужном месте
	static void SetTrackComponent(byte * track, dword elementSize, dword offset, dword count, TempTrack * src, dword type);
	//Определить нужна ли глобальная позиция
	static bool IsNeedGlobalPosition(TrackAccessor & track, dword numFrames);
	//Пересчитать позиции трека в глобальную систему
	void ConvertToGlobal(dword boneIndex, byte * buffer, dword numFrames, bool isMovement);



	//Получить количество кадров в клипе
	dword GetNumFrames(bool applyPosition);
	//Получить количество кадров в файле
	dword GetAntFrames(bool applyPosition);

	//Получить размер исходных данных в клипе
	dword GetAntDataSize();

	//Перевернуть клип
	void Reverse();
	//Проредить клип, удалив каждый второй ключ
	void Reduce();
	//Получить текущий кадр
	void GetFrame(long frame, Matrix & transform);
	//Переместить трек
	void MoveTrackTo(long frame, const Matrix & newTransform);

	//Разархивировать данные для использования
	void UseAnt();
	//Освободить разархивированные ресурсы
	void FreeAnt();

//--------------------------------------------------------------------------------------------
private:
	//Обновить описание файла
	void UpdateInfo();
	//Подготовить трек для использования
	AntFileTrackElement * PrepareTrack(long boneIndex, array<AntFileTrackElement> & staticTrack);
	//Выгрузить трек
	void ExportTrack(AnxStream & stream, const float * track, dword num, float error, bool markAsGlobalPos = false);
	//Получить значение в 8 bit
	char MakeChar(float v, float cnt, float dlt);
	//Получить значение в 16 bit
	short MakeShort(float v, float cnt, float dlt);
	//Обновить ошибку для const
	float ErrorConst(float err, float v, float cnt, float dlt);
	//Обновить ошибку для short
	float ErrorShort(float err, float v, float cnt, float dlt);
	//Обновить ошибку для char
	float ErrorChar(float err, float v, float cnt, float dlt);

	//Запокавать загруженные данные
	bool PackAnt();
	//Сжать данные
	bool Pack(void * src, dword srcSize, void * dst, dword & dstSize);
	//Расжать данные
	bool UnPack(void * src, dword srcSize, void * dst, dword dstSize);


public:
	AnxOptions & opt;				//Опции
	AntFile * next;					//Следующий в списке ant
	//Параметры клипа
	byte * ant;						//Полный ant файл
	dword antSize;					//Размер ant файла
	long pantIndex;					//Индекс сжатых данных в хранилище
	array<Bone> bone;				//Кости, применительно текущего скелета
	dword clipStartFrame;			//Начальный кадр анимации для клипа
	dword clipNumFrames;			//Количество кадров анимации в клипе
	string name;					//Имя клипа
	float fps;						//Скорость проигрывания трека
	float probability;				//Вероятность выбора трека
	dword clipSize;					//Размер выгруженного клипа	
	dword globalPosBones;			//Количество костей в клипе использующих глобальные позиции
	bool notFullControl;			//Не все кости анимированны
	//Временные параметры
	AntFileHeader * header;			//Заголовок
	char * strings;					//Таблица строк
	AntFileBone * bones;			//Кости
	dword numBones;					//Количество костей
	dword tracksOffset;				//Начало треков относительно начала ант файла
	dword numFrames;				//Количество кадров
	const char * loadError;			//Ошибка загрузки
	long useCounter;				//Счётчик пользователей распакованых данных
	//Статистика выгрузки трека
	dword trackConst;				//Константные треки
	dword track8;					//Треки с размером элемента в 1 байт
	dword track16;					//Треки с размером элемента в 2 байта
	dword track24;					//Треки с размером элемента в 3 байта
	dword trackFloat;				//Треки с размером элемента в 4 байт (плавающая точка)
	dword trackTotal;				//Всего выгружено треков
	//Допустимые ошибки при компресии треков
	float quaternionError;
	float positionError;
	float scaleError;
};

#endif

