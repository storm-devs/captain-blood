#pragma once

#include "..\..\..\Common_h\PostProcess\Filters\IMotionBlur.h"

//class MotionBlur : public IMotionBlur
//{
//	enum DetailLevel {det_off,det_low,det_med,det_hi};
//
//	long detailLevel;
//
//	ShaderId MotionBlurShow_id;
//	ShaderId MotionBlurCopy_id;
//
//public:
//
//	MotionBlur();
//	virtual ~MotionBlur();
//
//	virtual void UserInit();
//
//	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination);
//
//	// Получить текстуру в которой будет изображение, которое надо обработать
//	// можно подсунуть GetPostprocessTexture();
//	virtual ITexture* GetInputTexture();
//
//	virtual void SetParams(float  fOriginal, float  fAfterImage);
//	virtual void GetParams(float &fOriginal, float &fAfterImage);
//
///*	virtual void Update(float dltTime)
//	{
//	//	if( dltTime > 0.1f )
//	//		dltTime = 0.1f;
//
//		fTime += dltTime;
//	}*/
//
///*	virtual void SetType(bool blur)
//	{
//		motionBlur = blur;
//	}*/
//
//	virtual bool turnedOff()
//	{
//		return detailLevel == det_off;
//	}
//
//private:
//
//	bool bFirstTime;
//
//	IVariable *SourceTex;
//
//	IVariable *varAccumulator;
//	IVariable *varTexture;
//
////	IVariable *BlurOriginalK;
////	IVariable *AfterImageK;
//
////	IVariable *BlendType;
//	IVariable *BlendFactor;
//
//	IRenderTarget *pAccumulator;
//	IRenderTarget *pTexture;
//
//	RENDERVIEWPORT texViewPort;
//
//	float fPassedTime;
//	float GrabTime;
//
//	float fOriginal;
//	float fAfterImage;
//
////	bool motionBlur;
//
////	float fTime;
//
//};
//
//class RealMotionBlurController : public MissionObject
//{
//public:
//
//	 RealMotionBlurController();
//	~RealMotionBlurController();
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
//	virtual void Restart();
//
//	void Activate(bool isActive);
//
//	void _cdecl FlushPostProcess(float dltTime, long level);
//
//private:
//
//	float fOriginal;
//	float fAfterImage;
//
////	bool blur;
//
//	bool bActive;
//
//	bool turnedOff;
//
//};
