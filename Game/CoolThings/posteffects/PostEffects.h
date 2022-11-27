#pragma once

#include "Common.h"
#include "UberFilter.h"

class PostEffects : public IPostEffects
{
public:
	PostEffects();
	virtual ~PostEffects();

	virtual bool Init();

#ifdef GAME_RUSSIAN
	ICoreStorageFloat * gameTicks;
#endif

	//Исполнение в начале кадра
	virtual void StartFrame(float dltTime);
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);

	virtual FiltersParams & GetFiltersParams() { return m_uberFilter->GetFiltersParams(); }

private:
	IRender			* m_render;
	IFileService	* m_files;
	UberFilter	* m_uberFilter;

#ifdef GAME_RUSSIAN
	float totalTime;
	float randStartTime;
#endif

	RENDERSCREEN	renderScreen;

	ShaderId	m_gammaShader;
	IVariable	* m_rawImageTexture, * m_gammaCoeff;
	ICoreStorageFloat * m_storageBrightness;

#ifdef GAME_RUSSIAN
	bool isLiveServiceDone;
#endif
};