
#include "SoundParams.h"
#include "..\SndOptions.h"

//=============================================
//SoundBaseParams
//=============================================


SoundBaseParams::SoundBaseParams()
{
	priority = sbpc_priority_init;
	maxCount = sbpc_count_init;
	volume = SndDbToVol(sbpc_db_init);
	selectMethod = sbpc_select_init;
	loopMode = sbpc_loop_disable;
	fx = sbpc_fx_init;
	modifiers = sbpc_mod_init;
}


//Сохранить параметры в ини файле
void SoundBaseParams::SaveToIni(IEditableIniFile * ini)
{
	ini->SetLong("Setup", "priority", priority);
	ini->SetLong("Setup", "count", maxCount);
	ini->SetFloat("Setup", "volume", volume);
	ini->SetLong("Setup", "selector", selectMethod);
	ini->SetLong("Setup", "loop", loopMode);
	ini->SetLong("Setup", "fx", fx);
	ini->SetLong("Setup", "phoneme", (modifiers & sbpc_mod_phoneme) ? 1 : 0);
}

//Загрузить параметры из ини файла
void SoundBaseParams::LoadFromIni(IIniFile * ini)
{
	priority = ini->GetLong("Setup", "priority", sbpc_priority_init);
	priority = MinMax<long>(sbpc_priority_min, sbpc_priority_max, priority);
	maxCount = ini->GetLong("Setup", "count", sbpc_count_init);
	maxCount = MinMax<long>(sbpc_count_inf, sbpc_count_max, maxCount);
	volume = ini->GetFloat("Setup", "volume", SndDbToVol(sbpc_db_init));
	volume = MinMax<float>(0.0f, SndDbToVol(sbpc_db_max), volume);
	selectMethod = ini->GetLong("Setup", "selector", sbpc_select_init);
	selectMethod = MinMax<long>(sbpc_select_min, sbpc_select_max, selectMethod);
	loopMode = ini->GetLong("Setup", "loop", sbpc_loop_disable);
	loopMode = MinMax<long>(sbpc_loop_min, sbpc_loop_max, loopMode);
	fx = ini->GetLong("Setup", "fx", sbpc_fx_init);
	fx = MinMax<long>(sbpc_fx_min, sbpc_fx_max, fx);
	long isPhonemes = (ini->GetLong("Setup", "phoneme", 0) != 0) ?  sbpc_mod_phoneme : 0;
	modifiers = isPhonemes;
}

//Получить текстовое описание режима выбора волн
const char * SoundBaseParams::GetSelTextDesc(long select)
{
	switch(select)
	{
	case sbpc_select_rnd:
		return options->GetString(SndOptions::s_snd_select_rnd);
	case sbpc_select_queue:
		return options->GetString(SndOptions::s_snd_select_queue);
	case sbpc_select_sequence:
		return options->GetString(SndOptions::s_snd_select_sequence);
	}
	Assert(false);
	return null;
}

//Получить текстовое описание режима зацикленного проигрывания
const char * SoundBaseParams::GetLoopTextDesc(long loop)
{
	switch(loop)
	{
	case sbpc_loop_disable:
		return options->GetString(SndOptions::s_snd_loop_disable);
	case sbpc_loop_one_wave:
		return options->GetString(SndOptions::s_snd_loop_one_wave);
	}
	Assert(false);
	return null;
}

//Получить текстовое описание режима эффектов
const char * SoundBaseParams::GetFxTextDesc(long fx)
{
	switch(fx)
	{
	case sbpc_fx_full:
		return options->GetString(SndOptions::s_snd_fx_full);
	case sbpc_fx_premaster:
		return options->GetString(SndOptions::s_snd_fx_premaster);
	case sbpc_fx_master:
		return options->GetString(SndOptions::s_snd_fx_master);
	case sbpc_fx_music:
		return options->GetString(SndOptions::s_snd_fx_music);
	}
	Assert(false);
	return null;
}


//=============================================
//SoundAttGraph
//=============================================

//Диапазон минимальной дистанции
const SoundAttGraph::Range SoundAttGraph::minD = {0.001f, 1000000.0f, 10.0f};
//Диапазон максимальной дистанции
const SoundAttGraph::Range SoundAttGraph::maxD = {0.001f, 1000000.0f, 100.0f};
//Минимальное расстояние между максимальной и минимальной дистанциями
const float SoundAttGraph::mmDist = 1.0f;
//Диапазон коэфициентов кривой
const SoundAttGraph::Range SoundAttGraph::cR[4] = {{-20.0f, 20.0f, 0.0f}, {-40.0f, 40.0f, 0.0f}, {-40.0f, 40.0f, 0.0f}, {-10.0f, 10.0f, 0.0f}};

SoundAttGraph::SoundAttGraph()
{
	minDist = minD.initValue;
	maxDist = maxD.initValue;
	for(long i = 0; i < ARRSIZE(c); i++) c[i] = cR[i].initValue;
}


//Сохранить параметры в ини файле
void SoundAttGraph::SaveToIni(IEditableIniFile * ini)
{
	ini->SetFloat("Attenuation", "mindist", minDist);
	ini->SetFloat("Attenuation", "maxdist", maxDist);
	char buf[16];
	for(long i = 0; i < ARRSIZE(c); i++)
	{
		crt_snprintf(buf, sizeof(buf), "c%i", i);
		ini->SetFloat("Attenuation", buf, c[i]);
	}
}

//Загрузить параметры из ини файла
void SoundAttGraph::LoadFromIni(IIniFile * ini)
{
	minDist = ini->GetFloat("Attenuation", "mindist", minD.initValue);	
	maxDist = ini->GetFloat("Attenuation", "maxdist", maxD.initValue);
	char buf[16];
	for(long i = 0; i < ARRSIZE(c); i++)
	{
		crt_snprintf(buf, sizeof(buf), "c%i", i);
		c[i] = ini->GetFloat("Attenuation", buf, cR[i].initValue);
	}	
	CheckAndFix();
}


//Проверить и исправить значения
void SoundAttGraph::CheckAndFix()
{
	minDist = MinMax(minD.minValue, minD.maxValue, minDist);
	maxDist = MinMax(maxD.minValue, maxD.maxValue, maxDist);
	if(minDist > maxDist - mmDist)
	{
		maxDist = minDist + mmDist;
		if(maxDist > maxD.maxValue)
		{
			maxDist = maxD.maxValue;
			minDist = maxD.maxValue - mmDist;
		}
	}
	for(long i = 0; i < ARRSIZE(c); i++)
	{
		c[i] = MinMax(cR[i].minValue, cR[i].maxValue, c[i]);
	}
}

