#ifndef _ILIVESERVICE_H_
#define _ILIVESERVICE_H_

#include "core.h"

class ITexture;

// Game profile namespace (Contents, Achievements, Profiles)
namespace GP
{
	struct ContentDescribe
	{
		// битовые флаги наличия данных о контенте
		enum ContentDataFlag
		{
			cdf_DisplayName = 0x00000001,
			cdf_PlugState = 0x00000002,
			cdf_ImageTexture = 0x00000004,

			cdf_all = 0xFFFFFFFF
		};
		ContentDataFlag flags;

		// имя контента
		const char* pcDisplayName;
		// состояние подключен/отключен
		bool bPlugState;
		// текстура изображения контента
		ITexture* pTexture;
	};

	struct SettingsData
	{
		// отключение звука
		bool voiceMuted;
		// уровень громкости звука
		float voiceVolume;
		// уровень громкости звука
		float globalVolume;
		// уровень громкости звука
		float fxVolume;
		// уровень громкости звука
		float musicVolume;

		// чувствительность контроллера (1-нормальная, 2-повышеная, 0.5-низкая)
		float controlSensitivity;

		// уровень сложности (0-easy, 1-normal, 2-hard)
		long gameDifficulty;
		// включение/отключение автоприцеливания
		bool gameAutoaim;
		// инверсия вертикальной оси вращения
		bool gameYAxisInversion;
		// включение/отключение вибрации джойстика
		bool vibration;

		// идентификатор языка (4 символа + завершающий 0)
		char idLang[5];
	};

	enum Achievements
	{
		First_Blood = 0,
		Murderer,
		Executor,
		Reaper,
		Seamen,
		Musketeer,
		Duelist,
		Butcher,
		Dead_shot,
		Knight,
		Marksman,
		Destroyer,
		Slayer,
		Swashbuckler,
		Killa,
		Rage,
		Weaponmaster,
		Miser,
		Ruthless,
		Berserker,
		Bosun,
		Captain,
		Caper,
		Jolly_Roger,
		Pirate_King,
		Avenger,
		Defender_of_the_Weak,
		Master_of_Revenge,
		Conqueror,
		Champion,
		Captain_Blood,
		Shark_food,
		Trickster,
		Door_cracker,
		Juggernaut,
		Looter,
		Moneybags,
		Treasure_hunter,
		Master,
		Gunmaster,
		Catapult,
		Novice,
		Survivor,
		Veteran,
		Manslaughter,
		Meat_grinder,
		Seawolf,
		Terror_of_the_Sea,

		Achievement_max_count
	};

	typedef unsigned long LiveServiceState;
	enum LiveServiceStateMask
	{
		LSSM_Ready = 1,
		LSSM_SignIn = 2,
		LSSM_ControllerPluged = 4
	};
};

class ILiveService : public Service
{
protected:
	// конструктор
	ILiveService() {}

public:
	// деструктор
	virtual ~ILiveService() {}

	// инициализация
	virtual bool Init() = 0;
	// отработка в начале каждого кадра
	virtual void StartFrame(float dltTime) = 0;

	// готовность сервиса
	virtual GP::LiveServiceState GetServiceState() = 0;

	// включить слежение за определенным количеством пользователей
	virtual void SetCheckedUsersQuantity(int n) = 0;

	//--------------------------------------------------------------------
	// Работа с ачивментами
	//--------------------------------------------------------------------
	// инкремент ачивмента для текущего игрока
	virtual void Achievement_Increment(dword id, int count) = 0;
	// сброс счетчика для ачивмента
	virtual void Achievement_SetZero(dword id) = 0;
	// инкремент ачивмента для мультиплеерного игрока
	virtual void Achievement_IncrementMP(dword player, dword id, int count) = 0;
	// сброс счетчика для мультиплеерного ачивмента
	virtual void Achievement_SetZeroMP(dword player, dword id) = 0;

	// Получить синглплеерный ачивмент
	virtual void Achievement_Earn(dword liveID) = 0;
	// Получить мультиплеерный ачивмент
	virtual void Achievement_EarnMP(dword player, dword liveID) = 0;

	//--------------------------------------------------------------------
	// Работа с сейв данными
	//--------------------------------------------------------------------
	// есть ли сейв (при первом запуске еще нет)
	virtual bool Data_IsLoadEnable() = 0;
	// записать данные (т.к. данные пишутся в профайл, то актуален TCR#136:
	// "вызов не чаще одного раза в 5 минут")
	virtual bool Data_Save() = 0;
	// считать данные
	virtual bool Data_Load() = 0;
	// записать данные для настройки
	virtual bool Option_Save() = 0;
	// идет процесс отложенной записи сейва?
	virtual bool Data_SaveLoadIsProcessing() = 0;
	// успешная или нет запись? (действительна после окончания процесса записи)
	virtual bool Data_SaveLoadIsSuccessful() = 0;

	//--------------------------------------------------------------------
	// Работа с профилем (данные профиля)
	//--------------------------------------------------------------------
	// получить данные из профайла
	virtual const GP::SettingsData& GetProfileSettings () = 0;

	//--------------------------------------------------------------------
	// Работа с подгружаемым контентом
	//--------------------------------------------------------------------
	// состояние готовности сервиса работы с контентами
	virtual bool Content_IsReadyService() = 0;
	// количество доступных контентов
	virtual long Content_GetQuantity() = 0;
	// получить описание контента
	virtual bool Content_GetDescribe(long nContent,GP::ContentDescribe& descr) = 0;
	// подключить контент к игре
	virtual bool Content_Plugin(long nContent) = 0;
	// отключить контент из игры
	virtual bool Content_Unplug(long nContent) = 0;

	//--------------------------------------------------------------------
	// Для дебага
	//--------------------------------------------------------------------
	virtual void SetCurrentMissionPtr(void* pMis) = 0;
};

#endif
