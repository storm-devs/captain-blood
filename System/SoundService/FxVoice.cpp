

#include "FxVoice.h"

long FxVoice::earsSamples = 0;
const float FxVoice::basicVolume = 0.8f;		//Базовая громкость
const float FxVoice::basicPanDelta = 0.4f;		//Изменение громкости в зависимости от панарамы

FxVoice::FxVoice()
{
	FXRegistrationInfo(FxVoice, L"Storm engine FxVoice", L"Spirenkov Maxim");
	Reset();
}

FxVoice::~FxVoice()
{
}

void STDMETHODCALLTYPE FxVoice::Process(float * __restrict ptr, dword count)
{
	while(count > 0)
	{
		dword c = coremin(count, c_processblock);				
		WaveProcess(ptr, c);								
		ptr += c << 1;
		count -= c;
	}	
}

//Просчитать буфера звука
void FxVoice::WaveProcess(float * __restrict buffer, dword samples)
{
	//Базовые параметры
	const dword mask = ARRSIZE(ringBuffer) - 1;
	dword count = samples << 1;
	//Подготавливаем параметры расчёта
	syncro.Enter();
	float currentVolume = processVolume;
	float currentFadeVolume = fadeVolume;
	float currentFadeDelta = fadeDelta;
	bool is3D = (locatorsCount > 0);
	if(is3D)
	{
		float dirFront = locators[0].dirFront;
		float dirRight = locators[0].dirRight;
		float att = locators[0].attenuation;
		syncro.Leave();
		//Громкость с учётом затухания
		currentVolume = Clampf(att)*currentVolume;
		if(currentVolume > 0.5f/65536.0f)
		{
			//Панорама
			float testPanR = Clampf(basicVolume - basicPanDelta*dirRight);
			float testPanL = Clampf(basicVolume + basicPanDelta*dirRight);
			//Смещение волны от центра		
			long deltaPan = long(dirRight*earsSamples);
			long deltaPanR = -earsSamples - deltaPan;
			long deltaPanL = -earsSamples + deltaPan;
			//Коэфициент направления назад
			float dirK = coremax(-dirFront, 0.0f);
			float panK = 1.0f - coremin(dirK*3.0f, 1.0f)*0.05f;
			testPanR *= panK;
			testPanL *= panK;
	/*
			static const dword filterSize = 2;
			float fltK = dirK*0.8f;
			float nrmK = fltK*(1.0f/filterSize);
			float origK = 1.0f - fltK;
	*/
			//Процессируем 3D звук
			for(dword i = 0; i < count; i += 2)
			{
				//Сохраняем сэмпл в буфере			
				ringBuffer[ringBufferPos] = buffer[i];
				//Текущий сэмпл
				float currentSample = ringBuffer[(ringBufferPos - earsSamples) & c_hystorymask];			
				//Считаем текущую статическую гроскость
				currentFadeVolume += currentFadeDelta;
				currentFadeVolume = Clampf(currentFadeVolume);
				float vol = currentVolume*currentFadeVolume;
				//Выбираем звук с учётом ориентации
				float curR = ringBuffer[(ringBufferPos + deltaPanL) & c_hystorymask];
				float curL = ringBuffer[(ringBufferPos + deltaPanR) & c_hystorymask];			
				/*
				//Считаем приглушённую составляющую если звук находиться сзади
				float fltR = 0.0f;
				float fltL = 0.0f;			
				for(dword j = 0; j < filterSize; j++)
				{
					fltR += ringBuffer[(ringBufferPos + deltaPanR - j*4) & c_hystorymask];
					fltL += ringBuffer[(ringBufferPos + deltaPanL - j*4) & c_hystorymask];				
				}
				//Считаем текущее значение с учётом положения
				curR = curR*origK + fltR*nrmK;
				curL = curL*origK + fltL*nrmK;
				*/
				//Подмешиваем смещённую, приглушенную волну в текущему значению
				curR = Lerp(currentSample, curR, 0.1f);
				curL = Lerp(currentSample, curL, 0.1f);
				//Сохраняем результат, применяя громкости панарамы и волны
				buffer[i + 0] = vol*testPanR*curR;
				buffer[i + 1] = vol*testPanL*curL;
				//Переводим указатель на следующий сэмпл
				ringBufferPos = (ringBufferPos + 1) & c_hystorymask;
			}
		}else{
			for(dword i = 0; i < count; i += 2)
			{
				buffer[i + 0] = 0.0f;
				buffer[i + 1] = 0.0f;
			}
		}
	}else{
		syncro.Leave();
		//Процессируем стерео звук
		if(currentVolume > 0.5f/65536.0f)
		{
			for(dword i = 0; i < count; i += 2)
			{
				//Считаем текущую статическую гроскость
				currentFadeVolume += currentFadeDelta;
				currentFadeVolume = Clampf(currentFadeVolume);
				float vol = currentVolume*currentFadeVolume;
				//Применяем на стерео канал
				buffer[i + 0] *= vol;
				buffer[i + 1] *= vol;
			}
		}else{
			for(dword i = 0; i < count; i += 2)
			{
				buffer[i + 0] = 0.0f;
				buffer[i + 1] = 0.0f;
			}
		}
	}
	syncro.Enter();
	//Возвращаемые параметры
	fadeVolume = currentFadeVolume;
	retFadeVolume = fadeVolume;
	retCurrVolume = currentVolume*currentFadeVolume;
	syncro.Leave();
}

//Перевести эффект-процессор в исходное состояние
void STDMETHODCALLTYPE FxVoice::Reset()
{
	syncro.Enter();
	processVolume = 1.0f;
	fadeVolume = 1.0f;
	fadeDelta = 0.0f;
	retFadeVolume = 1.0f;
	retCurrVolume = 1.0f;
	locatorsCount = 0;
	ringBufferPos = 0;
	for(long i = 0; i < c_hystorysize; i++)
	{
		ringBuffer[i] = 0.0f;
	}
	syncro.Leave();	
}

//Начать процесс плавного наростания кромкости до нормальной
void FxVoice::FadeIn(float stepFadeDelta)
{
	syncro.Enter();
	fadeVolume = 0.0f;
	fadeDelta = stepFadeDelta;
	syncro.Leave();
}

//Начать процесс плавного уменьшения громкости
void FxVoice::FadeOut(float stepFadeDelta)
{
	syncro.Enter();
	fadeDelta = stepFadeDelta;
	syncro.Leave();
}

//Установить пространственное расположение звука
void FxVoice::SetLocators(const Locator * locs, dword count)
{
	Assert(count <= c_maxlocators);
	syncro.Enter();
	for(dword i = 0; i < count; i++)
	{
		locators[i] = locs[i];
	}
	locatorsCount = count;
	syncro.Leave();
}

//Установить текущую константную громкость
void FxVoice::SetVolume(float volume)
{
	syncro.Enter();
	processVolume = volume;
	syncro.Leave();
}

//Разрешить пространственные эффекты
void FxVoice::EnableEnvironment(bool isEbable)
{	
}


//Получить действующую громкость
float FxVoice::GetCurrentVolume()
{
	syncro.Enter();
	float vol = retCurrVolume;
	syncro.Leave();
	return vol;
}

//Получить множитель фейда
float FxVoice::GetFadeVolume()
{
	syncro.Enter();
	float vol = retFadeVolume;
	syncro.Leave();
	return vol;
}

//Инициализировать константы
void FxVoice::StaticInit(dword sampleRates)
{
	float earsDistTime = c_eardist*0.001f/c_soundspeed;
	earsSamples = long(earsDistTime*sampleRates);
}