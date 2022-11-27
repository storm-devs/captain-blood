#pragma once

#include "common.h"

class UberFilter : public IRenderFilter 
{
public:
	UberFilter(IRender * render, IFileService * files);
	virtual ~UberFilter();

	virtual void Init();

#ifdef GAME_RUSSIAN
	ICoreStorageFloat * gameTicks;
#endif

	virtual void FilterImage(IBaseTexture * source, IRenderTarget * destination); 

	virtual void SetSharpness(float sharpness);

	FiltersParams & GetFiltersParams() { return m_params; }

	void StartFrame(float dltTime);
	void DebugDraw();

#ifdef GAME_RUSSIAN
	float protValue;
	bool _dofBlurNear;
	ICoreStorageFloat * timeInSecFromStart;

	virtual void SetProtValue(float value)
	{
		if (timeInSecFromStart && timeInSecFromStart->Get(0.0f) > 1999.0f)
			protValue = 2.5f + value;
		else
			protValue = 2.5f;
	}
#endif

private:
	bool isEnableDOF;
	bool isEnableGlow;
	bool isEnableSharpness;

	FiltersParams	m_params;
	FiltersParams	m_prevParams;

	enum
	{
		BLUR_SIZE = 25
	};

#ifdef GAME_RUSSIAN
	float protect1, protect2, protect3, protect4, protect5;

public:
	virtual void SetProtValue2(float value)
	{
			protValue = 2.5f + value * Min(1.0f, float(int(gameTicks->Get(0.0f) / 1754.3f)));
	}
private:
#endif

	IRender			* m_render;
	IFileService	* m_files;

	IGMXService		* m_geometry;
	IGMXService::RenderModeShaderId		m_RMSGlowObjectID;

#ifdef GAME_RUSSIAN
	float _glowPower, _glowStart, _sharpPower, _dofPower, _dofFocus;

	void ChangePCoeffs();
#endif

	RENDERMULTISAMPLE_TYPE	multiSample;

	IVariable			* m_varBlurSource, * m_varBlurWeights;
	IRenderTarget		* m_quarterRT[2], * m_copyRT[2], * m_copyScreen;
	IRenderTargetDepth	* m_copyDepth;
	ShaderId			m_shaderResize4x4Glow, m_shaderResize4x4GlowHQ;
	ShaderId			m_shaderBlurGauss5H, m_shaderBlurGauss5V;

	ShaderId			m_shaderDOFScale;
	ShaderId			m_shaderBlurGauss5HDOF, m_shaderBlurGauss5VDOF;

	ShaderId			m_shaderProcessGlow;
	ShaderId			m_shaderSharpness;
	ShaderId			m_shaderGlow;
	ShaderId			m_shaderRadialBlur;
	ShaderId			m_shaderDOF;
	ShaderId			m_shaderFinal;

	ShaderId			m_shaderSharpnessWithZ, m_shaderFinalWithZ;

	//ShaderId			m_combinerShaders[16];
	IVariable			* m_rawImageTexture;

	IVariable			* m_varOriginalScreen;
	IVariable			* varScreenSize2;
	IVariable			* m_varSharpnessWeights;
	IVariable			* m_varGlowParams, * m_varGlowParams2, * m_varGlowObjParams;
	IVariable			* m_varRadialBlurParams;

	IVariable			* varDOFUV;
	IVariable			* m_varDOFParams;
	IVariable			* varDepthTexture;
	ShaderId			m_shaderDOFZNear, m_shaderDOFZFar;
	ShaderId			shaderDOFCalculateAlpha;

	// 
	Vector4			m_vectorsGBlur[25];
	Vector4			m_sharpnessWeight;

	RENDERVIEWPORT	debugVP;

	void		RecalcGlow();

#if !defined(_XBOX) && !defined(NO_TOOLS)
	ShaderId	shaderToolsWindow;
	IVariable	* varToolsParams;
#endif

#ifdef _XBOX
	IVariable	* varDOFParams2;

	// Восстановление Z-Buffer'a на X360
	IVariable	* varRestoreDepthTexture;
	ShaderId	x360RestoreDepthId;

	void	X360_RestoreDepth(float width, float height, IRenderTargetDepth * depth);
#endif
};
