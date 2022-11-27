#include "Glow.h"

GlowController::GlowController() : 
	glowObjects(_FL_, 4)
{
	postCreateDone = false;
}

GlowController::~GlowController()
{
	if (glowParams->mo == this)
	{
		glowParams->mo = null;
		glowParams->objects = null;
	}
}

bool GlowController::Create(MOPReader & reader)
{
	IPostEffects * peService = (IPostEffects *)api->GetService("PostEffects");

	glowParams = peService->GetFiltersParams().GetGlowParams();

	return EditMode_Update(reader);
}

bool GlowController::EditMode_Update(MOPReader & reader)
{
	threshold = reader.Float();
	initialPower = reader.Float();	

	backFactor = reader.Float();
	foreFactor = reader.Float();

	blurriness = float(reader.Float()) / 6.0f * 1.5f;
	float passes = reader.Float();

	//realColors = 1.0f;
	realColors = 1.0f - reader.Float();

	fadeInTime = reader.Float();
	fadeOutTime = reader.Float();

	highQuality = reader.Bool();
	//highQuality = false;

	int nodesNum = reader.Array();
	glowObjects.DelAll();
	glowObjects.AddElements(nodesNum);
	for (int i=0; i<nodesNum; i++)
	{
		glowObjects[i].objectName = reader.String();
		//glowObjects[i].threshold = reader.Float();
		glowObjects[i].power = reader.Float();
	}

	bool isActive = reader.Bool();

	Activate(isActive);

	postCreateDone = false;

	return true;
}

void GlowController::PostCreate()
{
	for (int i=0; i<glowObjects.Len(); i++)
	{
		if (!FindObject(glowObjects[i].objectName, glowObjects[i].object))
		{
			glowObjects.DelIndex(i);
			i--;
			continue;
		}
	}

	postCreateDone = true;
}

void GlowController::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		DelUpdate();
		SetUpdate(&GlowController::Execute, ML_EXECUTE5);

		fadeStarted = true;
		fadeOut = false;
		startPower = 0.0f;
		endPower = initialPower;
		fadeTime = EditMode_IsOn() ? 0.0f : fadeInTime;
		curTime = 0.0f;
	}
	else
	{
		fadeStarted = true;
		fadeOut = true;
		startPower = glowPower;
		endPower = 0.0f;
		fadeTime = EditMode_IsOn() ? 0.0f : fadeOutTime;
		curTime = 0.0f;
	}

	if (!EditMode_IsOn())
		LogicDebug("%s", (IsActive()) ? "activated" : "deactivated");
}

void __cdecl GlowController::Execute(float deltaTime, long level)
{
	if (!postCreateDone)
		PostCreate();

	if (fadeStarted)
	{
		if (fadeTime > 0.0f)
		{
			curTime += deltaTime;
			float k = curTime / Max(fadeTime, 1e-10f);
			glowPower = Lerp(startPower, endPower, Min(1.0f, k));
		}
		else
			glowPower = endPower;

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

	glowParams->isActive = true;
	glowParams->mo = this;
	glowParams->power = glowPower;
	glowParams->foreFactor = foreFactor;
	glowParams->backFactor = backFactor;
	glowParams->threshold = threshold;
	glowParams->blurriness = blurriness;
	glowParams->realColors = realColors;
	glowParams->objects = &glowObjects;
	glowParams->isHighQuality = highQuality;
}

MOP_BEGINLISTG(GlowController, "Glow controller", '1.00', 900, "Post effects")
	MOP_FLOAT("Threshlod", 0.5f);
	MOP_FLOAT("Brightes" , 0.5f);
	MOP_FLOATEX("Back", 0.0f, 0.0f, 1.0f);
	MOP_FLOATEX("Fore", 1.0f, 0.0f, 1.0f);
	MOP_FLOATEX("Blur passes", 6.0f, 0.0f, 20.0f);
	MOP_FLOATEX("Blur factor", 1.0f, 0.0f, 1.0f);
	MOP_FLOATEX("Real colors", 0.0f, 0.0f, 1.0f);
	MOP_FLOATEX("FadeIn", 0.0f, 0.0f, 100.0f);
	MOP_FLOATEX("FadeOut", 0.0f, 0.0f, 100.0f);
	MOP_BOOLC("High quality", false, "Убирает мерцание при движении, fps может упасть при этом.")
	MOP_ARRAYBEG("Nodes", 0, 1000)
		MOP_STRING("Name", "")
		MOP_FLOATEX("Brightness", 1.0f, 0.0f, 4.0f)
	MOP_ARRAYEND
	MOP_BOOL("Active", true);
MOP_ENDLIST(GlowController)