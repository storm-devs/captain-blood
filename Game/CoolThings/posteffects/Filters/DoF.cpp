#include "DoF.h"

DepthOfField::DepthOfField()
{
	blurPower = 0.0f;
}

DepthOfField::~DepthOfField()
{
	m_DOFParams->mo = null;
}

bool DepthOfField::Create(MOPReader & reader)
{
	IPostEffects * peService = (IPostEffects *)api->GetService("PostEffects");

	m_DOFParams = peService->GetFiltersParams().GetDOFParams();

	return EditMode_Update(reader);
}

void DepthOfField::PostCreate()
{
}

bool DepthOfField::EditMode_Update(MOPReader & reader)
{
	m_blurFactor = powf(reader.Float(), 1.2f);
	m_focusPoint = reader.Float();
	//m_focusDistance = reader.Float();
	m_blurFar = reader.Bool();
	m_blurNear = reader.Bool();
	long passes = reader.Long();
	fadeInTime = reader.Float();
	fadeOutTime = reader.Float();
	float time = reader.Float();
	m_active = reader.Bool();
	
	Activate(m_active);

	return true;
}

void __cdecl DepthOfField::Execute(float deltaTime, long level)
{
	if (fadeStarted)
	{
		if (fadeTime > 0.0f)
		{
			curTime += deltaTime;
			float k = curTime / Max(fadeTime, 1e-10f);
			blurPower = Lerp(startPower, endPower, Min(1.0f, k));
		}
		else
			blurPower = endPower;

		if (curTime >= fadeTime)
		{
			fadeStarted = false;
			if (fadeOut)
			{
				DelUpdate();
				return;
			}
		}
	} 
	
	m_DOFParams->mo = this;
	m_DOFParams->isActive = true;
	m_DOFParams->blurFar = m_blurFar;
	m_DOFParams->blurNear = m_blurNear;
	m_DOFParams->blurFactor = blurPower;
	m_DOFParams->focusPoint = m_focusPoint;
	m_DOFParams->focusDistance = m_focusDistance;
}

//Включение/выключение объекта
void DepthOfField::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		DelUpdate();
		SetUpdate(&DepthOfField::Execute, ML_EXECUTE5);

		fadeStarted = true;
		fadeOut = false;
		startPower = 0.0f;
		endPower = m_blurFactor;
		fadeTime = EditMode_IsOn() ? 0.0f : fadeInTime;
		curTime = 0.0f;
	}
	else
	{
		fadeStarted = true;
		fadeOut = true;
		startPower = blurPower;
		endPower = 0.0f;
		fadeTime = EditMode_IsOn() ? 0.0f : fadeOutTime;
		curTime = 0.0f;
	}

	if (!EditMode_IsOn())
		LogicDebug("%s", (IsActive()) ? "activated" : "deactivated");
}

MOP_BEGINLISTG(DepthOfField, "DepthOfField", '1.00', 900, "Post effects")
	MOP_FLOATEX("Blur factor", 1.0f, 0.0f, 1.0f);
	MOP_FLOATEX("Focus distance", 15.0f, 0.0f, 10000.0f);
	//MOP_FLOATEX("Focus len", 15.0f, 0.0f, 10000.0f);
	MOP_BOOL("Blur back", true);
	MOP_BOOL("Blur near", true);
	MOP_LONGEX("Blur passes",  0, 0, 20);
	MOP_FLOATEX("FadeIn", 0.0f, 0.0f, 100.0f);
	MOP_FLOATEX("FadeOut", 0.0f, 0.0f, 100.0f);
	MOP_FLOAT("Blend time", 0.0f);
	MOP_BOOL("Active", true)
MOP_ENDLIST(DepthOfField)
