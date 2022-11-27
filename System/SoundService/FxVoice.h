

#ifndef _FxVoice_h_
#define	_FxVoice_h_

#include "FxBase.h"


class __declspec( uuid("{9415D925-D44E-4252-9976-BD079F0625DD}")) FxVoice : public FxBase
{
public:
	enum Consts
	{
		c_soundspeed = 340,		//Cкорость звука в воздузе, метров в секунду
		c_eardist = 200,		//Растояние между ушами в милиметрах
		c_processblock = 64,	//Размер процесируемого блока в сэмплах
		c_maxlocators = 1,		//Наибольшее количество пронстранственных представлений одного звука
		c_hystorysize = 64,		//Размер буфера истории звука
		c_hystorymask = c_hystorysize - 1,
	};




public:
	//Определение местоположения звука или отражённой волны
	struct Locator
	{
		__forceinline Locator()
		{
			dirFront = 0.0f;
			dirRight = 0.0f;
			attenuation = 0.0f;
			delay = 0.0f;
		};

		float dirFront;			//Направление от слушателя на звук. 1 спереди, -1 сзади
		float dirRight;			//Направление от слушателя на звук. 1 справа, -1 слева
		float attenuation;		//Громкость для текущего удаления
		float delay;			//Задержка в секундах
	};

	
	
	/*



struct Params
{
Params()
{
states = 0;
volume = 1.0f;
fadeDelta = 0.0f;
posers = 0;
fadeVolume = 1.0f;
currVolume = 1.0f;
};

dword states;		//Управляюище состояния
float volume;		//Громкость звука
float fadeDelta;	//Изменение громкости фейда
long posers;		//Количество позеров (0 значит волна в 2D)
Poser pos[8];		//Представления волны в пространстве
float fadeVolume;	//Громкость фейда
float currVolume;	//Результирующая громкость		
};
*/

public:
	FxVoice();
	~FxVoice();

public:
	//Перевести эффект-процессор в исходное состояние
	virtual void STDMETHODCALLTYPE Reset();
	//Начать процесс плавного наростания кромкости до нормальной
	void FadeIn(float stepFadeDelta);
	//Начать процесс плавного уменьшения громкости
	void FadeOut(float stepFadeDelta);
	//Установить пространственное расположение звука
	void SetLocators(const Locator * locs, dword count);
	//Установить текущую константную громкость
	void SetVolume(float volume);

	//Разрешить пространственные эффекты
	void EnableEnvironment(bool isEbable);


	//Получить действующую громкость
	float GetCurrentVolume();
	//Получить множитель фейда
	float GetFadeVolume();

	//Инициализировать константы
	static void StaticInit(dword sampleRates);


private:
	virtual void STDMETHODCALLTYPE Process(float * __restrict ptr, dword count);
	//Просчитать буфера звука
	void WaveProcess(float * __restrict buffer, dword samples);

private:
	CritSection syncro;					//Точка синхронизации
	float processVolume;				//Статическая громкость
	float fadeVolume;					//Громкость фейдера
	float fadeDelta;					//Шаг изменения громкости фэйдера
	float retFadeVolume;				//Возвращаемая громкость фейда
	float retCurrVolume;				//Возвращаемая текущая громкость
	Locator locators[c_maxlocators];	//Ориентация волны и первичные отражения
	dword locatorsCount;				//Количество активных волн
	float ringBuffer[c_hystorysize];	//Кольцевой буфер с историей волны
	dword ringBufferPos;				//Текущий указатель в кольцевом буфере
	static long earsSamples;			//Количество сэмплов для фазового сдвига
	static const float basicVolume;		//Базовая громкость
	static const float basicPanDelta;	//Изменение громкости в зависимости от панарамы
};


#endif
