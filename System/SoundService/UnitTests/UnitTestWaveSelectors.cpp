

#include "..\SoundBankFile.h"
#include "SoundServiceUnitTests.h"

#ifndef STOP_DEBUG

//Существующие тесты
class UnitTestWaveSelectors
{
	enum Consts
	{
		c_minWaves = 2,			//Минимальное количество волн, попадающих на каждом тесте в звук
		c_maxWaves = 32,		//Максимальное количество звуков			
		c_testsCount = 100,		//Количество тестов (звуков с разным набором волн)
		c_precition = 10000,	//Порядок точности
		//Количество выборок звуков в каждом тесте
		c_testWaves = c_maxWaves*c_maxWaves*c_precition
	};


	struct StatisticElement
	{
		dword count;
		dword previosWave[c_maxWaves];
	};

public:
	UnitTestWaveSelectors();
	void Mode_select_rnd(bool traceDump);
	void Mode_select_queue(bool traceDump);
	void Mode_select_sequence();


private:
	void PrepareStep(long eqWaves);
	void MakeStatistics(dword count);
	void DumpStatistics(const char * source);

private:
	SoundBankFileSound sound;
	char queueBuffer[sizeof(SoundBankFileWaveQueue) + sizeof(SoundBankFileWaveQueue::Wave)*128];
	SoundBankFileWaveInfo waves[c_maxWaves];
	dword testWavesCount;
	StatisticElement statistics[c_maxWaves];
};


UnitTestWaveSelectors::UnitTestWaveSelectors()
{
	memset(&sound, 0, sizeof(sound));
	sound.waves = waves;
	memset(queueBuffer, 0, sizeof(queueBuffer));
	memset(waves, 0, sizeof(waves));	
}

void UnitTestWaveSelectors::PrepareStep(long eqWaves)
{
	//Заполняем волны
	dword maxCount = ARRSIZE(waves);
	Assert(maxCount > c_minWaves);
	if(eqWaves < 0)
	{
		sound.wavesCount = c_minWaves + (rand() % (maxCount - c_minWaves));
	}else{
		sound.wavesCount = eqWaves;
		Assert(eqWaves >= c_minWaves);
		Assert(eqWaves <= c_maxWaves);
	}
	float sum = 0.0f;
	for(dword i = 0; i < sound.wavesCount; i++)
	{
		waves[i].volume = 1.0f;
		if(eqWaves < 0)
		{
			waves[i].probability = 0.1f + Rnd(0.9f);
		}else{
			waves[i].probability = 1.0f;
		}
		waves[i].playTime = 0.0f;
		waves[i].wave = null;
		sum += waves[i].probability;
	}
	for(dword i = 0; i < sound.wavesCount; i++)
	{
		waves[i].probability /= sum;
	}
	//Инициализируем очередь
	memset(queueBuffer, 0, sizeof(queueBuffer));
	sound.InitSelectors();
	//Очищаем счётчик и буфер статистики
	testWavesCount = 0;
	memset(statistics, 0, sizeof(statistics));
}

void UnitTestWaveSelectors::MakeStatistics(dword count)
{	
	//Выполняем выборки, набирая статистику
	testWavesCount = count;
	for(dword i = 0, pindex = -1; i < count; i++)
	{
		dword index = sound.SelectWaveIndex();
		Assert(index < sound.wavesCount);
		statistics[index].count++;
		if(i > 0)
		{
			statistics[index].previosWave[pindex]++;
		}
		pindex = index;
	}
}

void UnitTestWaveSelectors::DumpStatistics(const char * source)
{
	api->Trace("==================================================================================================================");
	api->Trace("Error trace. Unit test found problem in %s wave select method. Total steps = %u", source, testWavesCount);
	api->Trace("==================================================================================================================");	
	for(dword i = 0; i < sound.wavesCount; i++)
	{
		char buffer[64*32];
		memset(buffer, 0, sizeof(buffer));
		dword count = 0;
		for(dword j = 0; j < sound.wavesCount; j++)
		{
			count += statistics[i].previosWave[j];
		}
		for(dword j = 0; j < sound.wavesCount; j++)
		{
			char buf[64];
			memset(buf, 0, sizeof(buf));
			crt_snprintf(buf, sizeof(buf) - 1, "%2i:%1.5f  ", j, statistics[i].previosWave[j]/double(count));
			crt_strcat(buffer, sizeof(buffer), buf);
		}
		double realp = statistics[i].count/double(testWavesCount);
		double delta = fabs(waves[i].probability - realp);
		api->Trace("%2i: abs(def_prob(%1.6f) - real_prob(%1.6f)) = %1.6f; detail -> count:%8i; previous waves: %s", i, waves[i].probability, realp, delta, statistics[i].count, buffer);
	}
}


void UnitTestWaveSelectors::Mode_select_rnd(bool traceDump)
{
	sound.squeue = null;
	sound.setup.mode = SoundBankFileSetup::mode_select_rnd;
	bool isErrorsFree = true;
	for(dword tests = 0; tests < c_testsCount; tests++)
	{
		PrepareStep(-1);
		//Собираем статистику
		MakeStatistics(c_testWaves);
		//Анализируем статистику на предмет расхождения вероятностей и равномерности распределения предыдущей волны
		double eps = 2.0f/c_precition;
		double kNorm = 1.0/testWavesCount;
		bool isError = false;
		for(dword i = 0; i < sound.wavesCount; i++)
		{
			//Проверяем расхождение вероятностей
			double prob = statistics[i].count*kNorm;
			double delta = fabs(waves[i].probability - prob);
			if(delta > eps)
			{
				isError = true;
			}
			//Перекос в вероятности по предыдущим волнам не должен быть большим
			dword minPr = statistics[i].previosWave[i];
			dword maxPr = minPr;
			for(dword j = 1; j < sound.wavesCount; j++)
			{
				minPr = coremin(minPr, statistics[i].previosWave[j]);
				maxPr = coremax(maxPr, statistics[i].previosWave[j]);
			}
			if(maxPr > 0)	
			{
				double delta = double(maxPr - minPr)/double(maxPr);
				if(delta*100.0 > 40)
				{
					isError = true;
				}
			}
		}
		if(traceDump)
		{
			DumpStatistics(isError ? "Unit test fault Mode_select_rnd" : "Dump Mode_select_rnd");
		}else
			if(isError)
			{
				isErrorsFree = false;
				DumpStatistics("Mode_select_rnd");
			}
	}
	if(isErrorsFree)
	{
		api->Trace("UnitTestWaveSelectors::Mode_select_rnd() complette successful...");
	}
}


void UnitTestWaveSelectors::Mode_select_queue(bool traceDump)
{
	sound.squeue = (SoundBankFileWaveQueue *)queueBuffer;
	sound.setup.mode = SoundBankFileSetup::mode_select_queue;
	bool isErrorsFree = true;
	for(dword tests = 0; tests < c_testsCount; tests++)
	{
		//Подготавливаем буфера и звук
		if(tests < c_minWaves || c_maxWaves < tests)
		{
			PrepareStep(-1);
		}else{
			PrepareStep(tests);			
		}
		//Собираем статистику
		MakeStatistics(c_testWaves);
		//Анализируем статистику на предмет расхождения вероятностей и равномерности распределения предыдущей волны
		double eps = 2.0f/c_precition;
		double kNorm = 1.0/testWavesCount;
		bool isError = false;
		for(dword i = 0; i < sound.wavesCount; i++)
		{
			//Проверяем расхождение вероятностей
			double prob = statistics[i].count*kNorm;
			double delta = fabs(waves[i].probability - prob);
			if(delta > eps)
			{
				isError = true;
			}
			//Проверяем что повторяемость предыдущей волны с таким же индекстом минимальна
			dword minPr = statistics[i].previosWave[i];
			dword minIndex = i;
			for(dword j = 1; j < sound.wavesCount; j++)
			{
				if(minPr > statistics[i].previosWave[j])
				{
					minPr = statistics[i].previosWave[j];
					minIndex = j;
				}
			}
			if(minIndex != i)	
			{
				isError = true;
			}
		}
		if(traceDump)
		{
			DumpStatistics(isError ? "Unit test fault Mode_select_queue" : "Dump Mode_select_queue");
		}else
		if(isError)
		{
			isErrorsFree = false;
			DumpStatistics("Mode_select_queue");
		}
	}
	if(isErrorsFree)
	{
		api->Trace("UnitTestWaveSelectors::Mode_select_rnd_queue() complette successful...");
	}
}

void UnitTestWaveSelectors::Mode_select_sequence()
{
	sound.selectSequenceCount = 0;
	sound.setup.mode = SoundBankFileSetup::mode_select_sequence;
	bool isErrorsFree = true;
	for(dword tests = 0; tests < c_testsCount; tests++)
	{
		//Подготавливаем буфера и звук
		PrepareStep(-1);
		//Выполняем проверку
		bool isError = false;
		for(dword i = 0, testCount = 0; i < c_testWaves; i++)
		{
			dword index = sound.SelectWaveIndex();
			if(index != testCount)
			{
				isError = true;
				break;
			}
			testCount++;
			if(testCount >= sound.wavesCount) testCount = 0;
		}
		if(isError)
		{
			isErrorsFree = false;
			api->Trace("==================================================================================================================");
			api->Trace("Error trace. Unit test found problem in Mode_select_sequence wave select method.");
			api->Trace("==================================================================================================================");	
			api->Trace("Waves count: %u, do steps %u", sound.wavesCount, i);
			break;
		}
	}
	if(isErrorsFree)
	{
		api->Trace("UnitTestWaveSelectors::Mode_select_sequence() complette successful...");
	}
}




void UnitTestWaveSelectorsProcess(bool traceDump)
{
	UnitTestWaveSelectors * ut = NEW UnitTestWaveSelectors();
	ut->Mode_select_rnd(traceDump);
	ut->Mode_select_queue(traceDump);
	ut->Mode_select_sequence();
	delete ut;
}

#endif
