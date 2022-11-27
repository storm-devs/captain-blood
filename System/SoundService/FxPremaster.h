//Динамический компрессор-лимитер, имитирующий перегрузку уха

#ifndef _FxPremaster_h_
#define _FxPremaster_h_


#include "FxBase.h"


class __declspec( uuid("{2EDA4C75-A51A-4918-B33E-40548B333EE2}")) FxPremaster : public FxBase
{
public:
	enum Consts
	{
		c_samples_per_block = 64,	//Со скольки сэмпплов накапливать велечину мягкого ограничения
		c_comp_threshold_up = 90,	//Верхний порог компрессора
		c_comp_threshold_down = 75,	//Нижний порог компрессора
		c_limit_threshold = 95,	//Сколько процентов*1000 диапазона использовать как неограниченные
	};

public:
	FxPremaster();
	~FxPremaster();

private:
	//Сбросить параметры
	virtual void STDMETHODCALLTYPE Reset();
	//Процессировать пришедший буфер
	virtual void STDMETHODCALLTYPE Process(float * __restrict buffer, dword samples);

private:
	//float currentLimitLevel;
	//float currentCompressionLevel;
	float compressorThreshold;
	float compressorThresholdStep;
	float compressorRatio;
	float compressorRatioStep;
	float compressorGain;
	float compressorGainStep;



	float softLimitGain;
	float softLimitNormalizer;
	dword blockPosition;
	dword blockCounter;
	float history[64];

	byte tmp[64];
};

#endif

