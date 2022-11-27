#include "RadialBlur.h"

RadialBlurController::RadialBlurController()
{
	power = 0.0f;
}

RadialBlurController::~RadialBlurController()
{
	radialBlurParams->mo = null;
}

bool RadialBlurController::Create(MOPReader & reader)
{
	IPostEffects * peService = (IPostEffects *)api->GetService("PostEffects");

	radialBlurParams = peService->GetFiltersParams().GetRadialBlurParams();

	return EditMode_Update(reader);
}

bool RadialBlurController::EditMode_Update(MOPReader & reader)
{
	focus = reader.Float();
	//power = reader.Float();

	passes = float(reader.Float()) / 6.0f;
	blurFactor = reader.Float();
	blurScale = reader.Float() / 10.0f;

	fadeInTime = reader.Float();
	fadeOutTime = reader.Float();

	targetName = reader.String();
	targetOffset = reader.Position();

	bool t0 = reader.Bool();
	float f1 = reader.Float();
	float f2 = reader.Float();
	float f3 = reader.Float();

	active = reader.Bool();

	Activate(active);

	targetPtr.Reset();

	return true;
}

void RadialBlurController::PostCreate()
{
	FindObject(targetName, targetPtr);
}

void __cdecl RadialBlurController::Realize(float deltaTime, long level)
{
	//Render().Print(10.0f, 40.0f, 0xFFFFFFFF, "%.2f, %.2f", blurCenter.x, blurCenter.y);
	//Render().Print(Vector(0.0f, 5.199999f, 0.0f), 1000.0f, 0.0f, 0xFFFFFFFF, "fasd");
	//Render().DrawSphere(blurPosition, 0.5f);
}

void __cdecl RadialBlurController::Execute(float deltaTime, long level)
{
	if (fadeStarted)
	{
		if (fadeTime > 0.0f)
		{
			curTime += deltaTime;
			float k = curTime / Max(fadeTime, 1e-10f);
			power = Lerp(startPower, endPower, Min(1.0f, k));
		}
		else
			power = endPower;

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

	if (targetName.NotEmpty())
	{
		FindObject(targetName, targetPtr);
	}
	
	if (targetPtr.Validate())
	{
		Matrix mtx(true);	
		
		mtx = targetPtr.Ptr()->GetMatrix(mtx);

		Matrix vp(Render().GetView(), Render().GetProjection());
		blurPosition = mtx.MulVertex(targetOffset);
		blurCenter = vp.Projection(blurPosition, 0.5f, 0.5f).v;//, Render().GetViewport().Width * 0.5f, Render().GetViewport().Height * 0.5f).v;
	}
	else
	{
		blurCenter = Vector(0.5f, 0.5f, 0.0f);
	}

	radialBlurParams->mo = this;
	radialBlurParams->isActive = true;
	radialBlurParams->focus = focus;
	radialBlurParams->power = power;
	radialBlurParams->blurCenter = blurCenter;
}

void RadialBlurController::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		DelUpdate();
		SetUpdate(&RadialBlurController::Execute, ML_EXECUTE5);
		SetUpdate(&RadialBlurController::Realize, ML_GUI5);

		fadeStarted = true;
		fadeOut = false;
		startPower = 0.0f;
		endPower = passes * blurFactor * blurScale * 24.0f;
		fadeTime = fadeInTime;
		curTime = 0.0f;
	}
	else
	{
		fadeStarted = true;
		fadeOut = true;
		startPower = power;
		endPower = 0.0f;
		fadeTime = fadeOutTime;
		curTime = 0.0f;
	}

	if (!EditMode_IsOn())
		LogicDebug("%s", (IsActive()) ? "activated" : "deactivated");
}

MOP_BEGINLISTG(RadialBlurController, "RadialBlur controller", '1.00', 900, "Post effects")
	MOP_FLOAT("Focus", 0.3f)
	
	MOP_FLOATEX("Blur passes", 6, 1, 20);

	MOP_FLOATEX("Blur factor", 1.0f, 0.0f, 1.0f);
	MOP_FLOATEX("Blur scale", 10.0f, 0.0f,10.0f);

	MOP_FLOATEX("Fade in  time", 0.0f, 0.0f, 100.0f);
	MOP_FLOATEX("Fade out time", 0.0f, 0.0f, 100.0f);

	MOP_STRING("Target", "Player");
	MOP_POSITION("Offset", Vector(0.0f));

	MOP_BOOL("Linear", true);

	MOP_FLOATEX("Focus dir", 270.0f, 0.0f, 359.9f);
	MOP_FLOATEX("Focus rad", 1.0f, 0.0f, 2.0f);
	MOP_FLOATEX("Focus asp", 0.0f, 0.0f, 1.0f);

	MOP_BOOL("Active", true);
MOP_ENDLIST(RadialBlurController)
