#include "ShooterEffect.h"
#include "..\..\..\Common_h\graphic\graphic.cpp"

ShooterEffect::ShooterEffect()
{
	fMaxLiveTime = 0.1f;
	fLiveTime = fMaxLiveTime;

	//Effect1
	pcLightLocator = "";
	//
	lightObject.Reset();

	//Effect2
	pcModelName = "";
	pcModelAni = "";
	pcNodeName = "";
	//
	pEffectModel = null;
	pEffectModelAnimation = null;

	//Effect3
	//
	pWidgetObject.Reset();

	IGMXScene* pShooterGMX = null;
	IMission* mission = null;
}

ShooterEffect::~ShooterEffect()
{
	Release();
}

void ShooterEffect::ReadMOPs( MOPReader & reader )
{
	fMaxLiveTime = reader.Float();
	fLiveTime = fMaxLiveTime;

	//Effect1
	pcLightLocator = reader.String().c_str();
	pcLightObjectName = reader.String();
	grphLightIntencity.ReadParams( reader );

	//Effect2
	pcModelName = reader.String().c_str();
	pcModelAni = reader.String().c_str();
	pcNodeName = reader.String().c_str();
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	mtxModelLocalTransform.Build( ang, pos );
	grphModelTransparence.ReadParams( reader );

	//Effect3
	pcWidgetName = reader.String();
	grphWidgetTransparence.ReadParams( reader );
}

void ShooterEffect::Init(IMission * pMission, IGMXScene* pShooterModel)
{
	mission = pMission;
	pShooterGMX = pShooterModel;

	if( !mission )
	{
		Release();
		return;
	}

	IGMXScene* pOldScene = pEffectModel;
	IAnimation* pOldAni = pEffectModelAnimation;
	pEffectModel = mission->Geometry().CreateGMX(pcModelName, &mission->Animation(), &mission->Particles(), &mission->Sound());
	pEffectModelAnimation = mission->Animation().Create( pcModelAni, _FL_ );
	if( pEffectModel )
	{
		pEffectModel->SetAnimation( pEffectModelAnimation );
		if (pEffectModelAnimation) pEffectModelAnimation->Pause(true);
	}
	RELEASE(pOldAni);
	RELEASE(pOldScene);
}

void ShooterEffect::PostCreate()
{
	if( !mission )
	{
		Release();
		return;
	}

	if( pShooterGMX )
		lightLocator = pShooterGMX->FindEntity(GMXET_LOCATOR, pcLightLocator);
	else
		lightLocator.reset();
	if( !lightLocator.isValid() && !string::IsEmpty(pcLightLocator) )
	{
		api->Trace("ShooterEffect: Can`t find locator \"%s\" for light",pcLightLocator);
	}
	mission->FindObject( pcLightObjectName, lightObject );

	mission->FindObject( pcWidgetName, pWidgetObject );
}

void ShooterEffect::Release()
{
	fLiveTime = 0.f;

	//Effect1
	lightLocator.reset();
	lightObject.Reset();
	grphLightIntencity.Release();

	//Effect2
	RELEASE(pEffectModel);
	RELEASE(pEffectModelAnimation);
	grphModelTransparence.Release();

	//Effect3
	pWidgetObject.Reset();

	pShooterGMX = null;
	mission = null;
}

void ShooterEffect::Process( float fDeltaTime )
{
	if( fLiveTime < fMaxLiveTime )
	{
		fLiveTime += fDeltaTime;
		if( fLiveTime >= fMaxLiveTime )
		{
			Stop();
			return;
		}

		// свет
		if (lightObject.Ptr() && lightLocator.isValid() && pEffectModel)
		{
			Matrix mtx;
			pEffectModel->GetNodeWorldTransform(lightLocator,mtx);

			char params[3][32];
			const char * prms[3];
			prms[0] = params[0];
			prms[1] = params[1];
			prms[2] = params[2];

			// ставим источник света в позицию
			crt_snprintf(params[0], ARRSIZE(params[0]),"%f",mtx.pos.x);
			crt_snprintf(params[1], ARRSIZE(params[1]),"%f",mtx.pos.y);
			crt_snprintf(params[2], ARRSIZE(params[2]),"%f",mtx.pos.z);
			lightObject.Ptr()->Command("moveto", 3, prms);

			//
			crt_snprintf(params[0], ARRSIZE(params[0]),"%f", grphLightIntencity.ReadGraphic(fLiveTime));
			lightObject.Ptr()->Command("SetMultiply", 1, prms);
		}

		// моделька
		if ( pEffectModel )
		{
			if( pEffectModelAnimation && !bEffectModelStarted )
			{
				if ( string::IsEqual(pEffectModelAnimation->CurrentNode(),pcNodeName) )
				{
					bEffectModelStarted = true;
				}
			}

			if( bEffectModelStarted )
			{
				Color color;
				color.a = grphModelTransparence.ReadGraphic(fLiveTime);

				pEffectModel->SetUserColor(color);

				pEffectModel->SetTransform( mtxModelLocalTransform * pShooterGMX->GetTransform() );
				pEffectModel->Draw();
			}
		}

		// виджет
		if( pWidgetObject.Ptr() )
		{
			const char* params[2];
			static char str[32];
			crt_snprintf(str, sizeof(str),"%4.3f",Clampf(grphWidgetTransparence.ReadGraphic(fLiveTime)));
			params[0] = str;
			pWidgetObject.Ptr()->Command("setfxa",1,params);
		}
	}
}

void ShooterEffect::Start()
{
		fLiveTime = 0.0f;

		// свет
		if (lightObject.Ptr()) lightObject.Ptr()->Activate(true);

		// моделька
		bEffectModelStarted = string::IsEmpty(pcNodeName) || pEffectModelAnimation==null;
		if (pEffectModelAnimation)
		{
			pEffectModelAnimation->Goto(pcNodeName,0.0f);
			pEffectModelAnimation->Pause(false);
		}

		// виджет
		if (pWidgetObject.Ptr())
		{
			pWidgetObject.Ptr()->Show(true);
		}

		Process( 0.f );
}

void ShooterEffect::Stop()
{
	fLiveTime = fMaxLiveTime;

	if (lightObject.Ptr()) lightObject.Ptr()->Activate(false);

	if (pEffectModelAnimation)
	{
		pEffectModelAnimation->Start();
		//pEffectModelAnimation->Pause(true);
	}

	if (pWidgetObject.Ptr()) pWidgetObject.Ptr()->Show(false);
}
