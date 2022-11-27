#pragma once

#include "..\..\..\Common_h\mission.h"

class FiltersParams
{
public:
	FiltersParams()
	{
		ZERO(m_glowParams);
		ZERO(m_sharpnessParams);
		ZERO(m_radialBlurParams);
		ZERO(m_dofParams);
	}

	struct GlowObject
	{
		ConstString objectName;
		MOSafePointer object;
		float power;
		float threshold;
	};

	struct GlowParams
	{
		bool isActive;
		MissionObject * mo;
		float threshold;
		float power;
		float backFactor, foreFactor;
		float blurriness;
		float realColors;
		array<GlowObject> * objects;
		bool isHighQuality;
	};

	struct SharpnessParams
	{
		MissionObject * mo;
		bool isActive;
		float power;
	};

	struct RadialBlurParams
	{
		MissionObject * mo;
		bool isActive;
		float focus;
		float power;
		Vector blurCenter;
	};

	struct DOFParams
	{
		MissionObject * mo;
		bool isActive;
		float focusPoint, focusDistance;
		float blurFactor, initialBlurFactor;
		bool blurFar, blurNear;
	};

	GlowParams * GetGlowParams() { return &m_glowParams; }
	SharpnessParams * GetSharpnessParams() { return &m_sharpnessParams; }
	RadialBlurParams * GetRadialBlurParams() { return &m_radialBlurParams; }
	DOFParams * GetDOFParams() { return &m_dofParams; }

protected:
	DOFParams			m_dofParams;
	GlowParams			m_glowParams;
	SharpnessParams		m_sharpnessParams;
	RadialBlurParams	m_radialBlurParams;
};

class IPostEffects : public Service
{
public:
	virtual FiltersParams & GetFiltersParams() = 0;

protected:
	virtual ~IPostEffects() {}
};
