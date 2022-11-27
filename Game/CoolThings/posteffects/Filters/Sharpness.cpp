#include "Sharpness.h"

SharpnessController::SharpnessController()
{
}

SharpnessController::~SharpnessController()
{
	m_sharpnessParams->mo = null;
}

bool SharpnessController::Create(MOPReader & reader)
{
	IPostEffects * peService = (IPostEffects *)api->GetService("PostEffects");

	m_sharpnessParams = peService->GetFiltersParams().GetSharpnessParams();

	return EditMode_Update(reader);
}

bool SharpnessController::EditMode_Update(MOPReader & reader)
{
	m_power = reader.Float();
	bool isActive = reader.Bool();

	Activate(isActive);

	return true;
}

void __cdecl SharpnessController::Execute(float deltaTime, long level)
{
	m_sharpnessParams->mo = this;
	m_sharpnessParams->isActive = true;
	m_sharpnessParams->power = m_power;
}

void SharpnessController::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	DelUpdate();

	if (IsActive())
	{
		SetUpdate(&SharpnessController::Execute, ML_EXECUTE5);
	}

	if (!EditMode_IsOn())
		LogicDebug("%s", (IsActive()) ? "activated" : "deactivated");
}

MOP_BEGINLISTG(SharpnessController, "Sharpness", '1.00', 900, "Post effects")
	MOP_FLOATEX("Scale", 1.0f, 0.0f, 10.0f)
	MOP_BOOL("Active", true)
MOP_ENDLIST(SharpnessController)
