#pragma once

#include "..\..\..\..\Common_h\Render.h"
#include "..\..\..\..\Common_h\Mission.h"

//class PostProcessService;
//
//class MotionEffect : public IRenderFilter
//{
//	struct Vertex
//	{
//		Vector4 p;
//
//		float u,v; float a;
//	};
//
//	enum DetailLevel {det_off,det_low,det_med,det_hi};
//
//	ShaderId MotionBlurCopy_Z_Dir_id;
//	ShaderId MotionEffectPaste_id;
//	ShaderId MotionBlurCopy_id;
//	ShaderId CalculateDir_id;
//	ShaderId IterativeBlurDir_id;
//	
//
//public:
//
//	MotionEffect();
//	virtual ~MotionEffect();
//
//	virtual void UserInit();
//
//	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination);
//
//	virtual void SetParams(dword blurPasses, float blurFactor, float motionFactor, float motionScale,
//		float cameraFactor);
//
//	virtual void Reset()
//	{
//		started = true;
//	}
//
//	virtual ITexture *GetInputTexture();
//
//	bool turnedOff()
//	{
//		return detailLevel == det_off;
//	}
//
//private:
//
//	Matrix prevMat;
//
//	bool started;
//
//	RENDERVIEWPORT texViewPort;
//
//	IRenderTarget *pChainTex[2];
//	IRenderTarget *pChain;
//
//	IRenderTargetDepth *depth_target;
//
//	IRenderTarget *pScreen;
//
//	IVariable *MotionFactor;
//	IVariable *MotionMatrix;
//
//	IVariable *FirstMotion;
//	IVariable *MotionScale;
//
//	IVariable *SourceTex;
//
//	IVariable *Screen;
//	IVariable *GlowBlurPass;
//
//	IVariable *BlurFactor;
//
//	IVariable *OneDivScreenWidth;
//	IVariable *OneDivScreenHeight;
//
//	dword blurPasses;
//	float blurFactor;
//
//	float motionFactor;
//	float motionScale;
//
//	float cameraFactor;
//
//	dword passCount;
//	dword divider;
//
//	long detailLevel;
//
//	PostProcessService* pServ;
//
//};
//
//class MotionEffectController : public MissionObject
//{
//public:
//
//	 MotionEffectController();
//	~MotionEffectController();
//
//public:
//
//	bool Create			(MOPReader &reader);
//	bool EditMode_Update(MOPReader &reader);
//
//	void Restart();
//
//	void Activate(bool isActive);
//
//	void Command(const char *id, dword numParams, const char **params);
//
//private:
//
//	void _cdecl Draw(float dltTime, long level);
//
//	void InitParams		(MOPReader &reader);
//
//	void UpdateAlpha(float alpha);
//
//	bool IsVisible()
//	{
//		return
//			0.01f < m_alpha &&
//			0.01f < blurFactor &&
//			0.01f < motionFactor &&
//			0.01f < motionScale;
//	}
//
//private:
//
//	PostProcessService *pServ;
//	MotionEffect *pEffect;
//
//	float m_alpha;	// уровень прозрачности для эффекта
//
//	bool bActive;
//
//	dword blurPasses;
//	float blurFactor;
//
//	float motionFactor;
//	float motionScale;
//
//	float cameraFactor;
//
//	bool bEnabled;
//
//	bool turnedOff;
//
//};
