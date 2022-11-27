#pragma once

#include "..\..\..\Common_h\PostProcess\Filters\IBlur.h"

//class BlurController;

//class Blur : public IBlur
//{
//	virtual void UserInit() {}
///
//	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination) {}
//
//	virtual ITexture *GetInputTexture() { return null; }
//
//	virtual void SetParams(long BlurPasses, float blurFactor, bool radial) {}
//	virtual void AddParams(long BlurPasses, float blurFactor, bool radial, float k) {}
//
//	virtual bool turnedOff() { return true; }
//
//	virtual void enable(bool en, IMission &mis) {}
//	virtual void update(IMission &mis) {}
//	enum DetailLevel {det_off,det_low,det_med,det_hi};
//
////	dword passCount;
//	dword divider;
//
//	long detailLevel;
//
////	ShaderId IterativeRadialBlur_id;
//
////	ShaderId IterativeRadialBlurH_First_Low_id;
//
//	ShaderId ResizeRectSimple_id;
//
//	ShaderId IterativeRadialBlurH_id;
//	ShaderId IterativeRadialBlurV_id;
//
//	ShaderId ShowGlow_id;
//	ShaderId BlurPaste_Low_Rad_id;
//
//public:
//
//	Blur();
//	virtual ~Blur();
//
//	virtual void UserInit();
//
//	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination);
//
//	virtual void SetParams(long BlurPasses, float blurFactor, bool radial);
//	virtual void AddParams(long BlurPasses, float blurFactor, bool radial, float k);
//
//	virtual ITexture *GetInputTexture();
//
////	virtual void SetViewPort(RENDERVIEWPORT viewPort, Matrix mProj, Matrix mView);
////	virtual void SetActiveMissionObject(BlurController *ActiveMissionObject);
//
//	virtual bool turnedOff()
//	{
//		return detailLevel == det_off;
//	}
//
//private:
//
////	BlurController *ActiveMissionObject;
//
////	IGMXService *pGeometry;
//
////	ITexture *Filter;
//
//	IVariable *SourceTex;
////	IVariable *FilterTex;
//
//	RENDERVIEWPORT texViewPort;
//
//	//IRenderTarget *pSourceTexture;	
//	//IRenderTarget *pZTexture;	
//
//	IRenderTarget *pChainTex[2];
//
//	IVariable *PixelSize;
//
//	IVariable *DOFOriginal;
//	IVariable *DOFBlured;
//	IVariable *DOFBlurFactor;
//
//	IVariable *OneDivScreenWidth;
//	IVariable *OneDivScreenHeight;
//
//	IVariable *GlowBlurPass;
//
//	IVariable *GlowThreshold;
//	IVariable *GlowPower;
//
////	IVariable *BlurFactor;
//
//	IVariable *InnerFactor;
//	IVariable *OuterFactor;
//
//	IVariable *DOF_Params;
//
////	IVariable *BlurA;
////	IVariable *BlurB;
//	IVariable *BlurC;
//	IVariable *BlurD;
//
////	long BlurPasses;
//
////	RENDERVIEWPORT vp;
//
////	bool radial;
//
////	Matrix m_Proj;
////	Matrix m_View;
//
////	float blurFactor;
//
//	float innerFactor;
//	float outerFactor;
//
//	struct Counter
//	{
//		IMission *m;
//		int counter;
//	};
//
//	array<Counter> counters;
//
//	int mis_index;
//
//	float count_k;
//
//	PostProcessService* pServ;
//
//public:
//
//	void enable(bool en, IMission &mis);
//	void update(IMission &mis);
//
//};
//
//class BlurController : public MissionObject
//{
//public:
//
//	 BlurController();
//	~BlurController();
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
//	void Command(const char *id, dword numParams, const char **params);
//
//	void Activate(bool isActive);
//
////	void _cdecl UpdateViewPort	(float dltTime, long level);
//	void _cdecl FlushPostProcess(float dltTime, long level);
//
//	void UpdateAlpha(float alpha);
//
//	bool IsVisible()
//	{
//		return
//			0.01f < m_alpha &&
//			0.01f < blurFactor;
//	}
//
//private:
//
//	PostProcessService* pServ;
//	IBlur *pBlur;
//
//	float m_alpha;	// уровень прозрачности для эффекта
//
//	bool bActive;
//	bool bFirst;
//
//	RENDERVIEWPORT viewPort;
//
//	long BlurPasses;
//
//	float fadeIn;
//	float fadeOut;
//
//	float blurFactor;
//
//	float time;
//
//	bool fadingIn;
//	bool fadingOut;
//
//	bool radial;
//
//	bool bEnabled;
//
//	bool turnedOff;
//
//	bool activated;
//
//};
