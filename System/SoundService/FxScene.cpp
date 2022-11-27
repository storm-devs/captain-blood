
#include "FxScene.h"
#include "SoundsEngine.h"


const float FxScene::timeEarly[18][2] =  {
	{0.0143f, 0.841f}, //1
	{0.0215f, 0.504f}, //2
	{0.0225f, -0.491f}, //3
	{0.0268f, 0.379f}, //4
	{0.0270f, 0.380f}, //5
	{0.0298f, 0.346f}, //6
	{0.0458f, -0.289f}, //7
	{0.0485f, 0.272f}, //8
	{0.0572f, 0.192f}, //9
	{0.0587f, 0.193f}, //10
	{0.0595f, 0.217f}, //11
	{0.0612f, -0.181f}, //12
	{0.0707f, 0.180f}, //13
	{0.0708f, 0.181f}, //14
	{0.0726f, -0.176f}, //15
	{0.0741f, 0.142f}, //16
	{0.0753f, -0.167f}, //17
	{0.0797f, 0.134f} //18
};



__forceinline FxScene::Delay::Delay()
{
	cur = null;
	buffer = null;
	endPrt = null;
	size = 0;
	damp = 0.0f;
}

__forceinline FxScene::Delay::~Delay()
{
	Release();
}

__forceinline void FxScene::Delay::Release()
{
	if(buffer) delete buffer;
	buffer = null;
}

__forceinline void FxScene::Delay::SetDelay(dword size)
{
	Assert(!buffer);
	buffer = NEW float[size];
	endPrt = buffer + size;
	this->size = size;
	Reset();
}

__forceinline void FxScene::Delay::SetDelayMs(float timeInMs)
{
	SetDelay(dword(SoundsEngine::GetFxMixFreq()*0.001f*timeInMs));
}

__forceinline void FxScene::Delay::Reset()
{
	for(float * f = buffer; f < endPrt; f++) *f = 0.0f;
	cur = buffer;
	damp = 0.0f;
}

__forceinline void FxScene::Delay::Set(float v)
{
	*cur = v;
	Next();
}

__forceinline float FxScene::Delay::Predelay(float timeInMs, float g)
{
	dword delta = long(SoundsEngine::GetFxMixFreq()*0.001f*timeInMs);
	if(delta > size) delta = size;
	float * val = cur - delta;
	if(val < buffer) val += endPrt - buffer;
	Assert(val >= buffer && val < endPrt);
	return *val*g;
}

__forceinline float FxScene::Delay::Damp(float v, float kDamp)
{
	damp = v*(1.0f - kDamp) + damp*kDamp;
	return damp;
}

__forceinline float FxScene::Delay::Comb(float v, float g, float kDamp)
{
	damp = damp*kDamp + *cur*(1.0f - kDamp);
	float out = damp*g + v;	
	Set(out);
	return out;
}

__forceinline float FxScene::Delay::AllPass(float v, float g, float kDamp)
{
	damp = damp*kDamp + *cur*(1.0f - kDamp);
	float out = damp - g*v;
	*cur = v + out*g;
	Next();
	return out;
}

__forceinline void FxScene::Delay::Next()
{
	cur++;
	if(cur >= endPrt) cur = buffer;
}

FxScene::FxScene()
{
	FXRegistrationInfo(FxScene, L"Storm engine FxScene", L"Spirenkov Maxim");
	Clear(false);
}

FxScene::~FxScene()
{
}


//Обсчитать полный буффер
void STDMETHODCALLTYPE FxScene::Process(float * __restrict ptr, dword count)
{
	if(flags & flag_isInit)
	{
		accessSync.Enter();
		while(count > 0)
		{
			dword c = coremin(count, c_processblock);
			WaveProcess(ptr, c);
			ptr += c << 1;
			count -= c;
		}
		accessSync.Leave();
	}
}


//Просчитать буфера звука
void FxScene::WaveProcess(float * __restrict buffer, dword samples)
{
	syncro.Enter();
	if(flags & flag_isUpdate)
	{
		//Расчёт новых параметров
		float damp = Clampf(params.damping);			
		float dispersion = Clampf(params.dispersion);
		predelayTime = Clampf(params.predelayTime, 10.0f, (float)c_predelay_max_ms);
		earlyTime = Clampf(params.earlyTime, 10.0f, (float)c_early_max_ms)*normalizeEarlyTime;
		earlyFadeMin = 0.5f*Clampf(params.earlyAttenuation);
		earlyFadeDlt = 1.0f - earlyFadeMin;
		earlyDamp = 0.7f + 0.25f*damp;
		diffuseWeight = 0.2f + 0.7f*dispersion;
		earlyWeight = 1.0f - diffuseWeight;
		wet = params.wet;
		dry = params.dry;
		diffuseAllpassGain =  Clampf(dispersion*0.3f + earlyTime*(0.7f/c_early_max_ms) + 0.2f, 0.1f, 0.85f);
		diffuseCombGain = diffuseAllpassGain*0.8f;
		diffuseCombDamp = damp*0.5f;
		diffuseAllpassDamp = damp*0.8f;
		envDampingKoef = damp*damp*0.99f;
		envDampingKoefRever = 1.0f - envDampingKoef;
		flags &= ~flag_isUpdate;		
	}
	syncro.Leave();
	if(wet < 0.001f && fabsf(dry - 1.0f) < 1e-8f)
	{
		return;
	}	
	dword count = samples << 1;
	for(dword i = 0; i < count; i += 2)
	{		
		//Текущии значения
		float vR = buffer[i + 0];
		float vL = buffer[i + 1];		
		//Предварительная задержка
		predelay.Set((vR + vL)*0.5f);
		float preVal = predelay.Predelay(predelayTime, 1.0f);
		//Вторичные отражения
		float earlyVal = preVal + RRnd(-0.0000001f, 0.0000001f);
		for(dword j = 0; j < ARRSIZE(timeEarly) - 1; j++)
		{
			float v = early.Predelay(timeEarly[j][0]*earlyTime, earlyFadeMin + earlyFadeDlt*timeEarly[j][1]);
			earlyVal += v;
		}
		earlyVal *= normalizeEarlyVolume;		
		float continueVal = earlyVal*earlyFadeMin;
		continueVal = early.Damp(continueVal, earlyDamp);
		early.Set(preVal - continueVal);
		//Рассеивание
		float diffR = envDampingR*0.01f;
		float diffL = envDampingL*0.01f;
		for(dword j = 0; j < ARRSIZE(combR); j++)
		{
			diffR += combR[j].Comb(earlyVal, diffuseCombGain, diffuseCombDamp);
			diffL += combL[j].Comb(earlyVal, diffuseCombGain, diffuseCombDamp);
		}
		diffR = diffR*(0.8f/ARRSIZE(combR)) + diffL*(0.2f/ARRSIZE(combR));
		diffL = diffR*(0.2f/ARRSIZE(combR)) + diffL*(0.8f/ARRSIZE(combR));		
		for(dword j = 0; j < ARRSIZE(allpassR); j++)
		{
			diffR = allpassR[j].AllPass(diffR, diffuseAllpassGain, diffuseAllpassDamp);
			diffL = allpassL[j].AllPass(diffL, diffuseAllpassGain, diffuseAllpassDamp);
		}
		float reverValR = earlyVal*earlyWeight + diffR*diffuseWeight;
		float reverValL = earlyVal*earlyWeight + diffL*diffuseWeight;
		envDampingR = reverValR*envDampingKoefRever + envDampingR*envDampingKoef;
		envDampingL = reverValL*envDampingKoefRever + envDampingL*envDampingKoef;
		//Результирующий сигнал
		buffer[i + 0] = vR*dry + reverValR*wet;
		buffer[i + 1] = vL*dry + reverValL*wet;
	}	
}


//Установить параметры среды
void FxScene::SetParams(const EnvParams & data)
{		
	syncro.Enter();
	if(flags & flag_isInit)
	{				
		params = data;
		flags |= flag_isUpdate;
		syncro.Leave();
	}else{		
		syncro.Leave();
		accessSync.Enter();
		predelay.SetDelayMs(c_predelay_max_ms);
		predelay.Reset();
		early.SetDelayMs(c_early_max_ms);
		early.Reset();
		const float kMin = 0.8f;
		const float kMax = 1.2f;
		for(dword i = 0; i < ARRSIZE(combR); i++)
		{
			float k = i*(1.0f/(ARRSIZE(combR) - 1.0f));
			float scale = powf(2, k) - 1.0f;
			float delayTime = 21.0f + scale*(30.0f + 2.0f*sinf(k*34.0f));
			combL[i].SetDelayMs(delayTime*((i & 1) ? kMax : kMin));
			combL[i].Reset();
			combR[i].SetDelayMs(delayTime*((i & 1) ? kMin + 0.1f : kMax + 0.1f));
			combR[i].Reset();
		}
		for(dword i = 0; i < ARRSIZE(allpassR); i++)
		{
			float k = i*(1.0f/(ARRSIZE(allpassR) - 1.0f));
			float scale = powf(2, 1.0f - k) - 1.0f;
			float delayTime = 15.0f + scale*20.0f;
			allpassL[i].SetDelayMs(delayTime*((i & 1) ? kMin + 0.01f : kMax + 0.01f));
			allpassL[i].Reset();
			allpassR[i].SetDelayMs(delayTime*((i & 1) ? kMax : kMin));
			allpassR[i].Reset();
		}
		accessSync.Leave();
		syncro.Enter();
		params = data;
		flags |= flag_isInit | flag_isUpdate;
		syncro.Leave();
	}
	
}

//Перевести эффект-процессор в исходное состояние
void STDMETHODCALLTYPE FxScene::Reset()
{
	accessSync.Enter();
	if((flags & flag_isInit) != 0)
	{
		predelay.Reset();
		early.Reset();
		for(dword i = 0; i < ARRSIZE(combR); i++)
		{
			combR[i].Reset();
			combL[i].Reset();			
		}
		for(dword i = 0; i < ARRSIZE(allpassR); i++)
		{
			allpassR[i].Reset();
			allpassL[i].Reset();			
		}
	}
	wet = 0.0f;
	dry = 1.0f;
	envDampingR = 0.0f;
	envDampingL = 0.0f;
	accessSync.Leave();
	syncro.Enter();
	memset(&params, 0, sizeof(params));
	params.dry = 1.0f;
	flags &= ~flag_isUpdate;
	syncro.Leave();
}

//Очистить буфера, и перейти в неактивное состояние
void FxScene::Clear(bool isSync)
{
	if(isSync)
	{
		accessSync.Enter();
	}
	flags = flag_init_value;
	predelay.Release();
	early.Release();
	for(dword i = 0; i < ARRSIZE(combR); i++)
	{
		combR[i].Release();
		combL[i].Release();		
	}
	for(dword i = 0; i < ARRSIZE(allpassR); i++)
	{
		allpassR[i].Release();
		allpassL[i].Release();		
	}
	predelayTime = 0.0f;
	earlyTime = 0.0f;
	earlyFadeMin = 0.0f;
	earlyFadeDlt = 0.0f;
	earlyDamp = 0.0f;
	earlyWeight = 0.0f;
	diffuseWeight = 0.0f;
	wet = 0.0f;
	dry = 1.0f;
	diffuseCombGain = 0.0f;
	diffuseCombDamp = 0.0f;
	diffuseAllpassGain = 0.0f;
	diffuseAllpassDamp = 0.0f;
	envDampingKoef = 0.0f;
	envDampingKoefRever = 0.0f;
	envDampingL = 0.0f;
	envDampingR = 0.0f;
	normalizeEarlyTime = 1.0f/timeEarly[ARRSIZE(timeEarly) - 1][0];
	normalizeEarlyVolume = 1.0f;
	for(dword j = 0; j < ARRSIZE(timeEarly); j++)
	{
		normalizeEarlyVolume += fabsf(timeEarly[j][1]);
	}
	normalizeEarlyVolume = 1.0f/normalizeEarlyVolume;
	if(isSync)
	{
		accessSync.Leave();
	}
}