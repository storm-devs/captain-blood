//#pragma once
//
//#include "..\..\..\..\common_h\render.h"
//#include "..\..\..\..\common_h\mission.h"
//
//class PostProcessService;
//
//class BlackAndWhite : public IRenderFilter
//{
//	enum DetailLevel {det_off,det_low,det_med,det_hi};
//
//	long detailLevel;
//
//	ShaderId PostProcessShowBW_id;
//
//public:
//
//	BlackAndWhite();
//	virtual ~BlackAndWhite();
//
//	virtual void UserInit();
//
//	virtual void FilterImage(IBaseTexture* source, IRenderTarget* destination); 
//
//	// Получить текстуру в которой будет изображение, которое надо обработать
//	// можно подсунуть GetPostprocessTexture();
//	virtual ITexture* GetInputTexture();
//
///*	void SetParams(float scale)
//	{
//		this->scale = scale;
//	}*/
//	void AddParams(float scale);
//
//	bool turnedOff()
//	{
//		return detailLevel == det_off;
//	}
//
//private:
//
//	IVariable* SourceTex;
//	IVariable* Scale;
//
//	float scale;
//
//	long Users;
//
//	PostProcessService* pServ;
//
//};
//
//class BlackController : public MissionObject
//{
//public:
//
//	 BlackController();
//	~BlackController();
//
//public:
//
//	//Инициализировать объект
//	virtual bool Create			(MOPReader &reader);
//
//	//Инициализировать объект
//	virtual bool EditMode_Create(MOPReader &reader);
//	//Обновить параметры
//	virtual bool EditMode_Update(MOPReader &reader);
//
//	virtual void Restart()
//	{
//		ReCreate();
//	}
//
//	virtual void Command(const char *id, dword numParams, const char **params);
//
//	void Activate(bool isActive);
//
//	void _cdecl FlushPostProcess(float dltTime, long level);
//
//	void UpdateAlpha(float alpha);
//
//	bool IsVisible()
//	{
//		return
//			0.01f < m_alpha &&
//			0.01f < blackFactor;
//	}
//
//private:
//
//	PostProcessService* pServ;
//	BlackAndWhite* pBlack;
//
//	float m_alpha;	// уровень прозрачности для эффекта
//
//	float blackFactor;
//
//	bool bActive;
//
//	bool bEnabled;
//
//	bool turnedOff;
//
//};
