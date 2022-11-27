#include "DamageEffect.h"

#include "..\..\..\Common_h\PostProcess\IPostProcess.h"
#include "..\..\..\Common_h\PostProcess\IPostProcess.h"

DamageEffect:: DamageEffect()
{
	pFace = null;
	pBack = null;

	Texture = null;
	Alpha	= null;

	fore = 0.0f;
	back = 1.5f;

	pVBuffer = null;

	nextHP = 0.0f;

	time = 0.0f;
	blen = 0.0f;

//	beat = OnDeadBeatMin*2 + OnDeadBeatMax;
	beat = 0;

	OnDead = false;
	IsDead = false;

//	pAfterImage = null;
	//pBlur		= null;

	backHP = 0.0f;

	beatSound = null;

	force		= null;
	forceDevice = 0;

	firstTime = true;

	rageTime = 0.0f;

	prevDead = true;

	bEnabled = false;
}

DamageEffect::~DamageEffect()
{
	//if( pBlur && bEnabled )
	//	pBlur->enable(bEnabled = false,Mission());

//	if( pAfterImage )
//		pAfterImage->Enable(false);

	if( beatSound )
		beatSound->Release();

	if( pFace )
		pFace->Release();

	if( pBack )
		pBack->Release();

	Texture = NULL;
	Alpha = NULL;

	if( force )
		force->Release();

	if( pVBuffer )
		pVBuffer->Release();
}

bool DamageEffect::Create		 (MOPReader &reader)
{
	Render().GetShaderId("DamageEffect", DamageEffect_id);

	Texture = Render().GetTechniqueGlobalVariable("DamageTexture",_FL_);
	Alpha	= Render().GetTechniqueGlobalVariable("Alpha"		 ,_FL_);

	CreateBuffer();

	IPostProcessService *p = (IPostProcessService *)api->GetService("PostProcessService");

	if( p )
	{
//		pAfterImage = p->GetAfterImageFilter();
		//pBlur		= p->GetBlurFilter();
	}

	InitParams(reader);

	return true;
}

bool DamageEffect::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void _cdecl DamageEffect::Draw(float dltTime, long level)
{
//	if( dltTime <= 0.0f )
//		return;

	if( dltTime > 0.0f )
	{
		//if( pBlur && IsActive())
		//	pBlur->update(Mission());

		if( EditMode_IsOn())
		{
			if( !character.Validate())
			{
				 FindObject(CharacterName,character);

			//	 if( character )
			//		 ValidatePointer(character,characterHash);
			}
		}
		else
		{
			if( !character.Ptr() && CharacterName.NotEmpty() )
			{
				 FindObject(CharacterName,character);

				 if( !character.Ptr())
					  CharacterName.Empty();
			}
		}

	//	if( !character && time == 0.0f && blen == 0.0f )
	//		return;

		if( character.Ptr())
		{
			time -= dltTime;

			if( time < 0.0f )
			{
				time = 0.0f; fore = 0.0f;
			}

			blen -= dltTime;

			if( blen < 0.0f )
			{
				blen = 0.0f;
			}
		}

		if( character.Ptr())
		{
			beat -= dltTime;

			if( beat < 0.0f )
			{
				beat = OnDeadBeatMin*2 + OnDeadBeatMax;

				if( beatSound && OnDead )
				{
					beatSound->Stop();
					beatSound->Play();
				}

				if( force && OnDead )
				{
					force->Stop();
					force->Play();
				}
			}
		}

		if( character.Ptr()/*&& !IsDead*/ )
		{
			float charHP = character.Ptr()->GetHP();

			if( firstTime )
			{
				nextHP = charHP; firstTime = false;
			}

			if( charHP != nextHP )
			{
				if( charHP < nextHP )
				{
					TriggerOnHit.Activate(Mission(),false);

					if( time > 0.0f )
					{
						if( time > OnHitHideTime )
						{
							//
						}
						else
						{
							time = OnHitHideTime + OnHitShowTime - time/OnHitHideTime*OnHitShowTime;
						}
					}
					else
					{
						time = OnHitHideTime + OnHitShowTime;
					}
				}

				float currHP;

				if( blen > 0.0f )
					currHP = Lerp(nextHP,prevHP,blen/blenTime);
				else
					currHP = nextHP;

				if( charHP != currHP )
				{
					prevHP = currHP;
					nextHP = charHP;

					blen = blenTime = fabsf(nextHP - prevHP)/character.Ptr()->GetMaxHP()*OnDeadBlendTime;
				}
				else
				{
					nextHP = currHP; blen = 0.0f;
				}
			}

		//	backHP = character->GetMaxHP()*OnDeadStartHP*0.01f;
			backHP = OnDeadStartHP;

			if( charHP <= 0.0f )
			{
				if( !IsDead )
				{
					IsDead = true;
				
					OnDeathTrigger.Activate(Mission(),false);
				}
			}
			else
			{
				if( IsDead )
					IsDead = false;

				if( charHP <= backHP && !OnDead )
				{
					if( beatSound )
						beatSound->Play();

					beat = OnDeadBeatMin*2 + OnDeadBeatMax;

					OnDead = true;
				
					OnDeadTriggerON .Activate(Mission(),false);
				}

				if( charHP >  backHP &&  OnDead )
				{
					if( beatSound )
					//	beatSound->FadeOut(3.5f);
						beatSound->Stop();

					OnDead = false;
				
					OnDeadTriggerOFF.Activate(Mission(),false);
				}
			}
		}
		else
			nextHP = 0.0f;

		if( character.Ptr())
		{
			if( time > 0.0f )
			{
				if( time > OnHitHideTime )
					fore = (OnHitHideTime + OnHitShowTime - time)/OnHitShowTime;
				else
					fore = time/OnHitHideTime;
			}

			if( backHP > 0.0f )
			{
				if( blen > 0.0f )
				{
					back = Lerp(nextHP,prevHP,blen/blenTime)/backHP;
				}
				else
					back = nextHP/backHP;
			}
			else
				back = 1.5f;
		}
		else
		{
			if( rageTime > 0.0f )
			{
				rageTime -= dltTime;

				if( rageTime < 0.0f )
					rageTime = 0.0f;

				float k = rageTime/rageTo;

				if( rageEnd > rageBeg )
				{
					fore = 1.0f - k;
					back = k;
				}
				else
				{
					fore = k;
					back = 1.0f - k;
				}
			}
			else
			{
				if( IsActive())
				{
					fore = 1.0f;
					back = 0.0f;
				}
				else
				{
					fore = 0.0f;
					back = 1.5f;

				}
			}
		}
	}

	float beat_k = 0.0f;

/*	if( beat > OnDeadBeatMin*2 )
	{
		beat_k = 0.0f;
	}
	else
	{
		beat_k = beat;

		if( beat_k  > OnDeadBeatMin )
			beat_k -= OnDeadBeatMin;

		beat_k = 1.0f - 2*fabsf(beat_k/OnDeadBeatMin - 0.5f);
	}*/

	if( character.Ptr())
	{
		if( beat < OnDeadBeatMax )
		{
			beat_k = 0.0f;
		}
		else
		{
			beat_k = beat - OnDeadBeatMax;

		/*	if( beat_k  > OnDeadBeatMin )
				beat_k -= OnDeadBeatMin;

			beat_k = 1.0f - 2*fabsf(beat_k/OnDeadBeatMin - 0.5f);*/

			if( beat_k < 1.5f*OnDeadBeatMin &&
				beat_k > 0.5f*OnDeadBeatMin )
			{
				beat_k = fabsf(beat_k/OnDeadBeatMin - 1.0f) + 0.5f;
			}
			else
			{
				beat_k = 2.0f - 2*fabsf(beat_k/OnDeadBeatMin - 1.0f);
			}
		}
	}

	if( EditMode_IsOn() && EditMode_IsSelect())
		fore = 1.0f;

	if( EditMode_IsOn() && EditMode_IsSelect())
	{
		Vertex b[4];

		b[0].x = 0.0f;
		b[0].y = 0.0f;

		b[1].x = 0.0f;
		b[1].y = 1.0f;

		b[2].x = 0.5f;
		b[2].y = 0.0f;

		b[3].x = 0.5f;
		b[3].y = 1.0f;

		Color c = OnDeadColorStart;

		b[0].c = c;
		b[1].c = c;
		b[2].c = c;
		b[3].c = c;

	//	if( Alpha )
	//		Alpha->SetFloat(-1.0f);

		if( Texture && pBack )
			Texture->SetTexture(pBack);

		if( Alpha )
			Alpha->SetFloat(pBack ? 1.0f : -1.0f);

		Render().DrawPrimitiveUP(DamageEffect_id, PT_TRIANGLESTRIP,2,b,sizeof(Vertex));

		b[0].x = 0.5f;
		b[0].y = 0.0f;

		b[1].x = 0.5f;
		b[1].y = 1.0f;

		b[2].x = 1.0f;
		b[2].y = 0.0f;

		b[3].x = 1.0f;
		b[3].y = 1.0f;

		c = OnDeadColorEnd;

		b[0].c = c;
		b[1].c = c;
		b[2].c = c;
		b[3].c = c;

		Render().DrawPrimitiveUP(DamageEffect_id, PT_TRIANGLESTRIP,2,b,sizeof(Vertex));
	}
	else
	if( back <= 1.0f )
	{
		Vertex b[4];

		b[0].x = 0.0f;
		b[0].y = 0.0f;

		b[1].x = 0.0f;
		b[1].y = 1.0f;

		b[2].x = 1.0f;
		b[2].y = 0.0f;

		b[3].x = 1.0f;
		b[3].y = 1.0f;

		Color cl;

		if( IsDead )
		{
			cl = OnDeadColorEnd;
		}
		else
		{
			if( character.Ptr())
			{
				Color ct; ct.LerpA(OnDeadColorEnd,OnDeadColorStart,back);
				Color cb; cb.LerpA(ct,OnDeadColorStart,OnDeadBeatAmp*0.01f);

				cl.LerpA(cb,ct,beat_k);
			}
			else
				cl.LerpA(OnDeadColorEnd,OnDeadColorStart,back);
		}

		b[0].c = cl;
		b[1].c = cl;
		b[2].c = cl;
		b[3].c = cl;

	//	if( Alpha )
	//		Alpha->SetFloat(-1.0f);

		if( Texture && pBack )
			Texture->SetTexture(pBack);

		if( Alpha )
			Alpha->SetFloat(pBack ? 1.0f : -1.0f);

		Render().DrawPrimitiveUP(DamageEffect_id, PT_TRIANGLESTRIP,2,b,sizeof(Vertex));
	}

	if( fore > 0.0f )
	{
		Render().SetStreamSource(0,pVBuffer);

		if( Texture && pFace )
			Texture->SetTexture(pFace);

		if( Alpha )
			Alpha->SetFloat(fore*(pFace ? 1.0f : -1.0f));

		Render().DrawPrimitive(DamageEffect_id, PT_TRIANGLESTRIP,0,8);
	}

	if( character.Ptr())
	{
		if( !IsDead )
		{
			if( back <= 1.0f )
			{
				float af = Lerp(OnDeadAfterApplyForceEnd,OnDeadAfterApplyForceStart,back);
				float pf = Lerp(OnDeadAfterPostFactorEnd,OnDeadAfterPostFactorStart,back);

/*				if( af < 1.0f && pf > 0.0f )
				{
					pAfterImage->Enable(true);
					pAfterImage->SetParams(af,pf);
				}
				else
					pAfterImage->Enable(false);
  */
				float bf = Lerp(OnDeadBlurEnd,OnDeadBlurStart,back);

				if( bf > 0.0f )
				{
					bf = bf*(1.0f + beat_k*OnDeadBeatBlurMul);

					if( bf > 1.0f )
						bf = 1.0f;

					//if( !bEnabled )
					//	pBlur->enable(bEnabled = true,Mission());

					//pBlur->AddParams(OnDeadBlurPasses,bf,OnDeadBlurRadial,bf);
				}
				else
				{
					//if( bEnabled )
					//	pBlur->enable(bEnabled = false,Mission());
				}

				if( beatSound )
					beatSound->SetVolume(Lerp(OnDeadSoundVolumeEnd,OnDeadSoundVolumeStart,back));

				prevDead = true;
			}
			else
			{
				if( prevDead )
				{
//					pAfterImage->Enable(false);

					//if( bEnabled )
					//	pBlur->enable(bEnabled = false,Mission());

					prevDead = false;
				}
			}
		}
		else
		{
			if( beatSound )
			//	beatSound->FadeOut(3.5f);
				beatSound->Stop();

			if( force )
				force->Stop();

			if( IsActive())
			{
				float bf = OnDeadBlurEnd;

				//pBlur->AddParams(OnDeadBlurPasses,bf,OnDeadBlurRadial,bf);
			}
		}
	}
	else
	{
		if( back <= 1.0f )
		{
			float af = Lerp(OnDeadAfterApplyForceEnd,OnDeadAfterApplyForceStart,back);
			float pf = Lerp(OnDeadAfterPostFactorEnd,OnDeadAfterPostFactorStart,back);

/*			if( af < 1.0f && pf > 0.0f )
			{
				pAfterImage->Enable(true);
				pAfterImage->SetParams(af,pf);
			}
			else
				pAfterImage->Enable(false);
  */
			float bf = Lerp(OnDeadBlurEnd,OnDeadBlurStart,back);

			if( bf > 0.0f )
			{
				//if( !bEnabled )
				//	pBlur->enable(bEnabled = true,Mission());

				//pBlur->AddParams(OnDeadBlurPasses,bf,OnDeadBlurRadial,bf);
			}
			else
			{
				//if( bEnabled )
				//	pBlur->enable(bEnabled = false,Mission());
			}

			prevDead = true;
		}
		else
		{
			if( prevDead )
			{
				//pAfterImage->Enable(false);

				//if( bEnabled )
				//	pBlur->enable(bEnabled = false,Mission());

				prevDead = false;
			}
		}
	}
}

void DamageEffect::CreateBuffer()
{
	if( pVBuffer )
		pVBuffer->Release();

	pVBuffer = Render().CreateVertexBuffer(
		sizeof(Vertex)*10,
		sizeof(Vertex),
		_FL_);
	Assert(pVBuffer)
}

void DamageEffect::UpdateBuffer()
{
	
	float aspect = Render().GetWideScreenAspectWidthMultipler();

	Vertex *p = (Vertex *)pVBuffer->Lock();
	Assert(p)

	Color ic = OnHitColor;
	Color oc = OnHitColor;

	float rx = OnHitBorderWidth *0.01f/aspect;
	float ry = OnHitBorderHeight*0.01f;

	float il = rx;
	float ir = 1.0f - rx;

	float it = ry;
	float ib = 1.0f - ry;

	float ol = 0.0f;
	float or = 1.0f;

	float ot = 0.0f;
	float ob = 1.0f;

	p[0].x = il; p[0].y = ib; p[0].c = ic;
	p[1].x = ol; p[1].y = ob; p[1].c = oc;
	p[2].x = ir; p[2].y = ib; p[2].c = ic;
	p[3].x = or; p[3].y = ob; p[3].c = oc;
	p[4].x = ir; p[4].y = it; p[4].c = ic;
	p[5].x = or; p[5].y = ot; p[5].c = oc;
	p[6].x = il; p[6].y = it; p[6].c = ic;
	p[7].x = ol; p[7].y = ot; p[7].c = oc;
	p[8].x = il; p[8].y = ib; p[8].c = ic;
	p[9].x = ol; p[9].y = ob; p[9].c = oc;

	pVBuffer->Unlock();
}

void DamageEffect::Activate(bool isActive)
{
//	if( pBlur )
//		pBlur->update(Mission());

	bool act = IsActive();

	MissionObject::Activate(isActive);

	if( !character.Ptr() && act != isActive )
	{
		if( isActive )
		{
			if( rageTime > 0.0f && rageBeg > rageEnd )
			{
				rageTo	 = OnHitShowTime;
				rageTime = (1.0f - rageTime/OnHitHideTime)*rageTo;
			}
			else
			{
				rageTo	 = OnHitShowTime;
				rageTime = rageTo;
			}

			rageBeg = 0.0f;
			rageEnd = 1.0f;

//			if( pAfterImage )
//				pAfterImage->Reset();

		//	if( !bEnabled )
		//		pBlur->enable(bEnabled = true,Mission());
		}
		else
		{
			if( rageTime > 0.0f && rageBeg < rageEnd )
			{
				rageTo	 = OnHitHideTime;
				rageTime = (1.0f - rageTime/OnHitShowTime)*rageTo;
			}
			else
			{
				rageTo	 = OnHitHideTime;
				rageTime = rageTo;
			}

			rageBeg = 1.0f;
			rageEnd = 0.0f;
		}
	}
}

void DamageEffect::Show(bool isShow)
{
	//if( pBlur )
	//	pBlur->update(Mission());

	if( IsShow() && !isShow )
	{
//		if( pAfterImage )
//			pAfterImage->Enable(false);

	//	if( pBlur && bEnabled )
	//		pBlur->enable(bEnabled = false,Mission());

		if( beatSound )
			beatSound->Stop();
	}

/*	if( isShow && !IsShow())
	{
		if( pBlur && !bEnabled )
			pBlur->enable(bEnabled = true,Mission());
	}*/

	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&DamageEffect::Draw,ML_GUI1 - 1);
	else
		DelUpdate(&DamageEffect::Draw);

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}

void DamageEffect::Command(const char *id, dword numParams, const char **params)
{
	//
}

void DamageEffect::Restart()
{
	//if( pBlur && bEnabled )
	//	pBlur->enable(bEnabled = false,Mission());

//	if( pAfterImage )
//		pAfterImage->Enable(false);

	bEnabled = false;

//	if( pAfterImage )
//		pAfterImage->Reset();

	////

	OnDead = false;
	IsDead = false;

	backHP = 0.0f;

	time = 0.0f;
	blen = 0.0f;

	if( beatSound )
		beatSound->Stop();

	firstTime = true;

	prevDead = true;

	beat = OnDeadBeatMin*2 + OnDeadBeatMax;

	////

	Show	(m_show);
	Activate(m_active);
}

void DamageEffect::InitParams(MOPReader &reader)
{
	CharacterName = reader.String();

	TriggerOnHit.Init(reader);

	OnHitTexture = reader.String().c_str();
	OnHitColor	 = reader.Colors();

	OnHitShowTime = reader.Float();
	OnHitHideTime = reader.Float();

	OnHitBorderWidth  = reader.Float();
	OnHitBorderHeight = reader.Float();

	OnDeadStartHP = reader.Float();

	OnDeadTriggerON .Init(reader);
	OnDeadTriggerOFF.Init(reader);

	OnDeadBlendTime = reader.Float();

	OnDeadColorStart = reader.Colors();
	OnDeadColorEnd	 = reader.Colors();

	OnDeadTexture = reader.String().c_str();

//	OnDeadTimeScaleObject = reader.String();

//	OnDeadTimeScaleStart = reader.Float();
//	OnDeadTimeScaleEnd	 = reader.Float();

	OnDeadSoundName = reader.String();

	OnDeadSoundVolumeStart = reader.Float();
	OnDeadSoundVolumeEnd   = reader.Float();

	OnDeadBlurPasses = reader.Long();

	OnDeadBlurStart = reader.Float();
	OnDeadBlurEnd	= reader.Float();

	OnDeadBlurRadial = reader.Bool();

	OnDeadAfterApplyForceStart = reader.Float();
	OnDeadAfterApplyForceEnd   = reader.Float();

	OnDeadAfterPostFactorStart = reader.Float();
	OnDeadAfterPostFactorEnd   = reader.Float();

	OnDeadBeatMin = reader.Float();
	OnDeadBeatMax = reader.Float();

	OnDeadBeatAmp	  = reader.Float();
	OnDeadBeatBlurMul = reader.Float();

	const char *forceProfile = reader.String().c_str();
	long device = reader.Long();

	OnDeathTrigger.Init(reader);

//	rageTime = 0.0f;

//	if( pAfterImage )
//		pAfterImage->Reset();

	Show	(m_show	  = reader.Bool());
	Activate(m_active = reader.Bool());

	////

	FindObject(CharacterName,character);

	if( pFace )
		pFace->Release();

	pFace = Render().CreateTexture(_FL_,OnHitTexture);

	if( pBack )
		pBack->Release();

	pBack = Render().CreateTexture(_FL_,OnDeadTexture);

	UpdateBuffer();

	OnDead = false;
	IsDead = false;

	backHP = 0.0f;

	time = 0.0f;
	blen = 0.0f;

	if( beatSound )
		beatSound->Release();

	beatSound = Sound().Create(OnDeadSoundName,_FL_,false,false);

	firstTime = true;

	prevDead = true;

	beat = OnDeadBeatMin*2 + OnDeadBeatMax;

	if( OnDeadForce != forceProfile || forceDevice != device )
	{
		if( force )
			force->Release();

		force = Controls().CreateForce(forceProfile,false,device);

		OnDeadForce = forceProfile;
		forceDevice = device;
	}
}

MOP_BEGINLISTCG(DamageEffect, "DamageEffect", '1.00', 1000, "DamageEffect", "Character")

	MOP_STRING("CharacterName", "Player");		// имя перса за которым следить

	MOP_MISSIONTRIGGER("TriggerOnHit");			// триггер при демедже

	MOP_STRING("OnHitTexture", "");				// текстура рамки околоэкранной при демедже
	MOP_COLOR("OnHitColor", Color(0xffff0000));	// цвет добавляемый к текстуре на околоэкрканной рамке

	MOP_FLOAT("OnHitShowTime", 0.1f);			// за сколько появится рамка из прозрачность
	MOP_FLOAT("OnHitHideTime", 0.4f);			// за сколько появится рамка из прозрачность

	MOP_FLOAT("OnHitBorderWidth" ,  9);			// ширина рамки
	MOP_FLOAT("OnHitBorderHeight", 10);			// высота рамки

	MOP_FLOAT("OnDeadStartHP", 25);				// сколько должно быть жизни, что бы эффект смерти заработал

	MOP_MISSIONTRIGGER("OnDeadTriggerON" );		// тригер на включение  эффекта смерти
	MOP_MISSIONTRIGGER("OnDeadTriggerOFF");		// тригер на выключение эффекта смерти

	MOP_FLOAT("OnDeadBlendTime", 2.0f);

	MOP_COLOR("OnDeadColorStart", (dword)0x00ff0000);	// начальные цвет и прозрачность эффекта смерти
	MOP_COLOR("OnDeadColorEnd",	  (dword)0x80ff0000);	// конечные  цвет и прозрачность эффекта смерти

	MOP_STRING("OnDeadTexture", "");

//	MOP_STRING("OnDeadTimeScaleObject", "");	// имя объекта который изменяет время в миссии

//	MOP_FLOAT("OnDeadTimeScaleStart", 1.0f);	// начальный масштаб времени для эффекта смерти
//	MOP_FLOAT("OnDeadTimeScaleEnd",	  0.5f);	// начальный масштаб времени для эффекта смерти

	MOP_STRING("OnDeadSoundName", "");			// имя звука для эффекта

	MOP_FLOAT("OnDeadSoundVolumeStart", 0.5f);	// начальная громкость звука смерти
	MOP_FLOAT("OnDeadSoundVolumeEnd",	1.0f);	// начальная громкость звука смерти

	MOP_LONGEX("OnDeadBlurPasses", 12, 1, 20);

	MOP_FLOAT("OnDeadBlurStart", 0.0f);			// сила блюра при начале    эффекта смерти
	MOP_FLOAT("OnDeadBlurEnd",	 0.1f);			// сила блюра при окончании эффекта смерти

	MOP_BOOL("OnDeadBlurRadial", false);

	MOP_FLOAT("OnDeadAfterApplyForceStart", 0.75f)
	MOP_FLOAT("OnDeadAfterApplyForceEnd",	0.30f)

	MOP_FLOAT("OnDeadAfterPostFactorStart", 0.5f)
	MOP_FLOAT("OnDeadAfterPostFactorEnd",	1.0f)

	MOP_FLOATEX("OnDeadBeatMin", 0.33f, 0.1f, 1.0f);
	MOP_FLOATEX("OnDeadBeatMax", 0.32f, 0.1f, 2.0f);

	MOP_FLOATEX("OnDeadBeatAmp", 30, 0, 100);
	MOP_FLOATEX("OnDeadBeatBlurMul", 2.5f, 1.0f, 10.0f);

	MOP_STRING("OnDeadForce", "");
	MOP_LONG  ("OnDeadDevice", 0);

	MOP_MISSIONTRIGGER("OnDeathTrigger");		// тригер на смерть

	MOP_BOOL("Show"  , true);
	MOP_BOOL("Active", true);

MOP_ENDLIST(DamageEffect)
