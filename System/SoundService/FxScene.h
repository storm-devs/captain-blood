

#ifndef _FxScene_h_
#define _FxScene_h_


#include "FxBase.h"


class __declspec( uuid("{66207DD2-B90A-4cef-80D3-8CEFC466E105}")) FxScene : public FxBase
{
	struct Delay
	{
		Delay();
		~Delay();
		void Release();

		void SetDelay(dword size);
		void SetDelayMs(float timeInMs);
		void Reset();
		
		void Set(float v);
		float Predelay(float timeInMs, float g);
		float Damp(float v, float kDamp);
		float Comb(float v, float g, float kDamp);
		float AllPass(float v, float g, float kDamp);
				
	protected:
		void Next();

	protected:
		float * cur;
		float * buffer;
		float * endPrt;
		dword size;
	public:
		float damp;
	};


public:
	enum Consts
	{
		c_processblock = 64,		//Размер процесируемого блока в сэмплах
		c_disp_comb = 5,			//Количество паралельных фильтров с разным временем отклика (comb)
		c_disp_allpass = 4,			//Длинна цепочки последовательных allpass фильтров		
		c_predelay_max_ms = 510,	//Максимсальное время предварительной задержки
		c_early_max_ms = 1010,		//Максимальное время вторичных отражений
		flag_init_value = 0,		//Значение флагов при конструировании объекта
		flag_isInit = 1,			//Инициализированы буфера
		flag_isUpdate = 2,			//Были обновлены параметры
	};

public:
	//Параметры среды
	struct EnvParams
	{
		union
		{
			struct
			{
				float predelayTime;			//Задержка до первого отражённого звука 0..500мс
				float earlyTime;			//Время всех вторичных отражений 0..1000мс
				float earlyAttenuation;		//Коэфициент затухания первичных отражений
				float damping;				//Поглощение средой высокочастотной составляющей (захламлёность)
				float dispersion;			//Степень рассеивания		
				float wet;					//Какая часть рассеяного звука попадает в выходной сигнал (0..1)
				float dry;					//Какая часть исходного звука попадает в выходной сигнал (0..1)
				float unuse;				//Не используеться
			};
			float val[8];					//Представление в виде набора чисел
		};
	};


public:
	FxScene();
	~FxScene();

public:
	//Установить параметры среды
	void SetParams(const EnvParams & data);
	//Очистить буфера, и перейти в неактивное состояние
	void Clear(bool isSync = true);
	//Перевести эффект-процессор в исходное состояние
	virtual void STDMETHODCALLTYPE Reset();

protected:
	//Обсчитать полный буффер
	virtual void STDMETHODCALLTYPE Process(float * __restrict ptr, dword count);
	//Просчитать буфера звука
	void WaveProcess(float * __restrict buffer, dword samples);

private:
	CritSection syncro;					//Точка синхронизации при изменении параметров
	CritSection accessSync;				//Точка синхронизации для использования ресурсами
	dword flags;						//Инициализированны ли буфера
	Delay predelay;						//Предварительная линия задержки
	Delay early;						//Линия задержки вторичных отражений
	Delay combR[5];						//Рассеивающии фильтры первой ступени
	Delay combL[5];						//Рассеивающии фильтры первой ступени
	Delay allpassR[6];					//Рассеивающии фильтры второй ступени
	Delay allpassL[6];					//Рассеивающии фильтры второй ступени

private:
	EnvParams params;					//Сохранёные параметры
	float predelayTime;					//Время до первого отражения
	float earlyTime;					//Время первичных отражений	
	float earlyFadeMin;					//Минимальный порог возвращаемого значение
	float earlyFadeDlt;					//Амплитуда разброса первичных отражений
	float earlyDamp;					//Коэфициент фильтрации для линии первичных отражений
	float earlyWeight;					//Вес первычных отражений в сигнале ревербирации
	float diffuseWeight;				//Вес рассеяных отражений в сигнале ревербирации
	float wet;							//Какая часть рассеяного звука попадает в выходной сигнал
	float dry;							//Какая часть исходного звука попадает в выходной сигнал	
	float diffuseCombGain;				//Коэфициент затухания для Comb-фильтров
	float diffuseCombDamp;				//Коэфициент фильтрации для Comb-фильтров
	float diffuseAllpassGain;			//Коэфициент затухания для Allpass-фильтров
	float diffuseAllpassDamp;			//Коэфициент фильтрации для Allpass-фильтров
	float envDampingKoef;				//Общий коэфициент фильтрации отражений
	float envDampingKoefRever;			//Обратное значение к envDampingKoef
	float envDampingR, envDampingL;		//Текущее значение фильтрации отражений

private:
	float normalizeEarlyTime;			//Коэфицинт нормализации времени первичных отражений
	float normalizeEarlyVolume;			//Коэфицинт нормализации громкости первичных отражений	
	static const float timeEarly[18][2];//Таблица первичных отражений
};

#endif

