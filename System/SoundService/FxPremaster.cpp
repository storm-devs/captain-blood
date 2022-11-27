/*
	При выходе сигнала из диапазона порог компрессора начинает опускаться,
	наклонная компрессирования начинает также опускаться. Также в пропорции
	ослабляеться сигнал.
	in ->[Gain]->[compressor]->[soft limiter]-> out

*/

#include "FxPremaster.h"


FxPremaster::FxPremaster()
{
	FXRegistrationInfo(FxPremaster, L"Storm engine FxPremaster", L"Spirenkov Maxim");
	Assert(c_comp_threshold_up > c_comp_threshold_down);
	Assert(c_comp_threshold_down > 0);
	Assert(c_comp_threshold_up < 100);
	Assert(c_limit_threshold > c_comp_threshold_up);
	Assert(c_limit_threshold < 100);
	Assert((c_samples_per_block & (c_samples_per_block - 1)) == 0);
	Assert(c_samples_per_block >= 16);
	Reset();
}

FxPremaster::~FxPremaster()
{
}

//Сбросить параметры
void STDMETHODCALLTYPE FxPremaster::Reset()
{
	compressorThreshold = c_comp_threshold_up*0.01f;
	compressorThresholdStep = 0.0f;
	compressorRatio = 1.0f;
	compressorRatioStep = 0.0f;
	compressorGain = 1.0f;
	compressorGainStep = 0.0f;



	softLimitGain = 1.0f;
	softLimitNormalizer = 1.0f;

	blockPosition = 0;
	blockCounter = 0;
	for(dword i = 0; i < ARRSIZE(history); i++) history[i] = 0.0f;	
}

//Процессировать пришедший буфер
void STDMETHODCALLTYPE FxPremaster::Process(float * __restrict buffer, dword samples)
{
	//Расчитываем максимальное значение на каждый блок
	dword count = samples << 1;
	float maxVolume = 0.0f;
	dword counter = blockCounter;	
	for(dword i = 0; i < count; i++)
	{
		//Считываем значение и получаем абсолютное
		float v = buffer[i];
		float absv = fabsf(v);		
		//Максимальное значение для текущего блока
		maxVolume = coremax(absv, maxVolume);
		counter++;
		if(counter == c_samples_per_block)
		{
			//Сохраняем громкость блока
			history[blockPosition] = maxVolume;
			blockPosition = (blockPosition + 1) & (ARRSIZE(history) - 1);
			//Вычисляем текущие параметры компрессора
			float historyMax = 0.0f;
			for(dword j = 0; j < ARRSIZE(history); j++)
			{
				historyMax = coremax(historyMax, history[j]);
			}
			//Расчитываем требуемый коэфициент усиления
			float compressorGainNew = coremin(1.0f/historyMax, 1.0f);
			float blendTimeInSec;
			if(compressorGainNew < compressorGain)
			{
				blendTimeInSec = 2.0f;
			}else{
				blendTimeInSec = 4.0f;
			}
			compressorGainStep = (compressorGainNew - compressorGain)/(44100*blendTimeInSec);
			//Корректируем максимальное значение в соответствие текущего коэфициента усиления
			historyMax *= compressorGain;
			//Порог компрессора
			float thresholdK = Clampf((historyMax - 1.0f)/(3.0f - 1.0f));
			float threshold  = c_comp_threshold_up*0.01f + thresholdK*0.01f*(c_comp_threshold_down - c_comp_threshold_up);
			compressorThresholdStep = (threshold  - compressorThreshold)*(1.0f/c_samples_per_block);
			//Коэфициент ослабления сигнала выше порогового уровня
			if(historyMax > compressorThreshold)
			{
				float ratio = (1.0f - compressorThreshold)/(historyMax - compressorThreshold);
				compressorRatioStep = (ratio - compressorRatio)*(1.0f/c_samples_per_block);
			}else{
				compressorRatioStep = 0.0f;
			}
			//Начинаем расчёт нового блока
			maxVolume = 0.0f;
			counter = 0;			
		}
		//Ослабление сигнала
		absv *= compressorGain;		
		//Компрессирование сигнала
		if(absv > compressorThreshold)
		{
			absv = (absv - compressorThreshold)*compressorRatio + compressorThreshold;
		}
		//Константное приведение полного диапазона под лимитер
		absv *= c_limit_threshold*0.01f;
		//Обновляем параметры компресора
		compressorThreshold += compressorThresholdStep;
		compressorRatio += compressorRatioStep;
		compressorGain += compressorGainStep;
		//Лимитирование сигнала
		static const float limitThreshold = c_limit_threshold*0.01f;
		static const float limitNormalize = 1.0f/(1.0f - c_limit_threshold*0.01f);
		static const float limitScale = 1.0f - c_limit_threshold*0.01f;
		if(absv > limitThreshold)
		{			
			float limitValue = (absv - limitThreshold)*limitNormalize;
			float limitedValue = 1.0f - 1.0f/(1.0f + limitValue);
			absv = limitedValue*limitScale + limitThreshold;
		}
		//Сохраняем полученое значение в соответствие со знаком
		if(v >= 0.0f)
		{
			buffer[i] = absv;
		}else{
			buffer[i] = -absv;
		}
	}
	blockCounter = counter;
}

