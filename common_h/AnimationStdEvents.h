//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnimationStdEvents
//============================================================================================

#ifndef _AnimationStdEvents_h_
#define _AnimationStdEvents_h_

#include "Animation.h"
#include "gmx.h"
#include "Sound.h"
#include "particles.h"

#include "IExplosionPatch.h"

#include "AnimationBlendLevels.h"


#ifndef _ANI_EVENTS_NO_MISSION_
#include "mission.h"
#else
class IMission;
#endif


class AnimationStdEvents : public IAnimationListener
{
//--------------------------------------------------------------------------------------------
public:
	AnimationStdEvents() : sounds(_FL_),
							particles(_FL_)
	{
		sound = null;
		particle = null;
		geometry = null;
		mission = null;
		voice = null;
		activeBlendShape = -1;
		blendShapesCount = 0;
	};

	virtual ~AnimationStdEvents()
	{
		Stop();
		if(geometry)
		{
			geometry->Release();
		}
	}

	void Init(ISoundScene * s, IParticleManager * p, IMission * mis)
	{
		sound = s;
		particle = p;
		mission = mis;
	}

	//Установить обработчики анимации для всей сцены
	virtual void SetScene(IGMXScene * _geometry, const Matrix & mtx)
	{		
		if(geometry)
		{
			geometry->Release();
		}
		geometry = _geometry;
		if(!_geometry) return;
		geometry->AddRef();
		if(geometry->IsAnimated())
		{
			IAnimation * a = (IAnimation *)geometry->GetAnimation();
			if(a)
			{
				if(a->Is(anitype_animation))
				{
					AddAnimation(a);
				}				
				a->Release();
			}
		}
		matrix = mtx;
	};

	//Установить обработчики анимации на заданную анимацию
	void AddAnimation(IAnimation * anx)
	{
		if(anx)
		{
			for(long i = 0; i < ani; i++)
			{
				if(ani[i] == anx) return;
			}
			collapser.Unregistry(ani.GetBuffer(), ani.Size());
			ani.Add(anx);
			AddHandlers(anx);
			collapser.Registry(ani.GetBuffer(), ani.Size());
		}
	}

	void DelAnimation(IAnimation * anx)
	{
		if(anx)
		{
			collapser.Unregistry(ani.GetBuffer(), ani.Size());
			ani.Del(anx);
			anx->DelAllEventHandlers(this);
		}
	}

protected:
	virtual void AddHandlers(IAnimation * anx)
	{
		IAnimationListener::AddHandlers(anx);
		//Обработчик 3D звука на локаторе
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::PlaySound, "Snd");
		//Завести партикловую систему в локаторе		
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::PlayParticles, "PlyPrt");
		//Завести партикловую систему привязанную к локатору
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::LocParticles, "LocPrt");
		//Сжать кость в 0
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::BoneCollapse, "BoneCollapse");
		//Удалить выборочно партикловую систему
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::DelLocParticles, "DelLocPrt");
		//Удалить все партикловые системы
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::DelAllParticles, "DelAllPrt");
		//Активировать-деактивировать объект миссии
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::ChangeActiveObject, "ActivateObject");
		//Создать взрыв поверхности в локаторе или в центре геометрии
		anx->SetEventHandler(this, (AniEvent)&AnimationStdEvents::PatchExplosion, "PatchExplosion");
	}

	virtual void DelHandlers(IAnimation * anx)
	{
		IAnimationListener::DelHandlers(anx);
		//Обработчик 3D звука на локаторе
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::PlaySound, "Snd");
		//Завести партикловую систему в локаторе		
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::PlayParticles, "PlyPrt");
		//Завести партикловую систему привязанную к локатору
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::LocParticles, "LocPrt");
		//Сжать кость в 0
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::BoneCollapse, "BoneCollapse");
		//Удалить выборочно партикловую систему
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::DelLocParticles, "DelLocPrt");
		//Удалить все партикловые системы
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::DelAllParticles, "DelAllPrt");
		//Активировать-деактивировать объект миссии
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::ChangeActiveObject, "ActivateObject");
		//Создать взрыв поверхности в локаторе или в центре геометрии
		anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::PatchExplosion, "PatchExplosion");
	}

public:

	//Остановить партиклы
	void ResetParticles()
	{
		for(long i = 0; i < particles; i++)
		{
			LocalParticles & prt = particles[i];
			RELEASE(prt.prt);
			ReleaseLocator(prt.locator);
		}
		particles.Empty();
	}

	//Остановить звуки
	void ResetSounds()
	{
		StopVoice();
		for(long i = 0; i < sounds; i++)
		{
			LocalSound & snd = sounds[i];
			ReleaseLocator(snd.locator);
			snd.snd->Release();
			snd.snd = null;
		}
		sounds.Empty();
	}

	//Остановить голосовую анимацию
	void StopVoice()
	{
		if(voice)
		{
			voice->Stop();
			voice = null;
			blendShapesCount = 0;
		}
		if(geometry)
		{
			dword needCount = geometry->GetBlendShapesCount();
			for(dword i = 0; i < needCount; i++)
			{
				geometry->SetBlendShapeMorphTargetWeight(i, 0.0f);
			}
		}
		activeBlendShape = -1;
	}

	//Закончить коллапс костей
	void ResetCollapser()
	{
		collapser.Reset(ani.GetBuffer(), ani.Size());
	}

	//Полностью остановить работу
	virtual void Stop()
	{
		ResetParticles();
		ResetSounds();
		RemoveAllHandlers();
	}

	//Отписаться от обработки
	virtual void RemoveAllHandlers()
	{
		for(long i = 0; i < ani; i++)
		{
			ani[i]->DelAllEventHandlers(this);
		}
		collapser.Unregistry(ani.GetBuffer(), ani.Size());
		ani.Empty();
	}

	//Обновить состояние обработчика
	virtual void Update(const Matrix & mtx, float dltTime)
	{
		matrix = mtx;
		for(long i = 0; i < sounds; i++)
		{
			LocalSound & ls = sounds[i];
			if(ls.nodeName && ls.nodeName != ls.ani->CurrentNode())
			{
				ls.snd->FadeOut(ls.fadeoutTime);
				ls.nodeName = null;
			}
			if(ls.snd->IsPlay())
			{
				//Обновляем позицию
				if(ls.locator.isValid())
				{
					Assert(geometry);
					ls.snd3D->SetPosition(matrix*geometry->GetNodeLocalTransform(ls.locator).pos);
				}
			}else{
				//Удаляем звук из списка
				if(voice == ls.snd)
				{
					StopVoice();
				}
				ReleaseLocator(ls.locator);
				ls.snd->Release();
				ls.snd = null;
				sounds.Extract(i);
				i--;
			}
		}
		for(long i = 0; i < particles; i++)
		{
			LocalParticles & lp = particles[i];
			if(!lp.prt->IsAlive() || lp.prt->IsLooped())
			{
				if(lp.prt->IsLooped())
				{
					api->Trace("AnimationStdEvents : Can't support looped particles. Stop particles %s", lp.prt->GetName());
				}
				//Удаляем систему из списка
				RELEASE(particles[i].prt);
				ReleaseLocator(particles[i].locator);
				particles.Extract(i);
				i--;
			}
		}
		//Фонемы
/*		if(api->DebugKeyState('1'))
		{
			int i = 0;
		}
*/
		if(voice && geometry)
		{
			long id = voice->GetPhonemeId();
			if(id >= 1)
			{
				id = id - 1;
				for(long i = 0; i < blendShapesCount; i++)
				{
					if(blendShapes[i].id == id)
					{
						activeBlendShape = i;
						break;
					}
				}
				if(i >= blendShapesCount)
				{
					if(blendShapesCount < ARRSIZE(blendShapes))
					{
						activeBlendShape = blendShapesCount;
						blendShapesCount++;
					}else{
						long indexForReplace = 0;
						float weightForReplace = 10.0f;
						for(long i = 0; i < blendShapesCount; i++)
						{
							if(blendShapes[i].k < weightForReplace)
							{
								indexForReplace = i;
								weightForReplace = blendShapes[i].k;
							}
						}
						activeBlendShape = indexForReplace;
					}
					blendShapes[activeBlendShape].k = 0.0f;
				}			
				blendShapes[activeBlendShape].id = id;
			}else{
				activeBlendShape = -1;
			}
		}
		if(blendShapesCount > 0)
		{
			Assert(geometry);
			const float blendInTimeInSec = 0.08f;
			const float blendOutTimeInSec = 0.15f;
			for(long i = 0; i < blendShapesCount; i++)
			{
				float & k = blendShapes[i].k;
				if(i != activeBlendShape)
				{
					k -= dltTime*(1.0f/blendOutTimeInSec);
					if(k <= 0.0f)
					{
						geometry->SetBlendShapeMorphTargetWeight(blendShapes[i].id, 0.0f);
						if(blendShapesCount > 0)
						{
							blendShapesCount--;
							blendShapes[i] = blendShapes[blendShapesCount];
							if(activeBlendShape == blendShapesCount)
							{
								activeBlendShape = i;
							}
							i--;
							continue;
						}else{
							blendShapesCount = 0;
							activeBlendShape = -1;
							break;
						}
					}
				}else{
					k += dltTime*(1.0f/blendInTimeInSec);
					if(k > 1.0f) k = 1.0f;
				}
				geometry->SetBlendShapeMorphTargetWeight(blendShapes[i].id, k);
			}
		}
	}

//--------------------------------------------------------------------------------------------
protected:
	void RedirectSoundEvent(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
#ifndef _ANI_EVENTS_NO_MISSION_
		//Пытаемся найти миссионый объект
		if(!mission) return;
		MOSafePointer ptr;
		if(!mission->FindObject(ConstString(params[2]), ptr))
		{
			api->Trace("Animation event: Can't redirect play sound event, mission object \"%s\" not found, animation %s, node %s", params[2], ani->GetName(), ani->CurrentNode());
			return;
		}
		//Перебрасываем команду объекту, чтобы он инициировал проигрывание звука
		const char * prms[6];
		prms[0] = params[0];
		prms[1] = params[1];
		prms[2] = "";
		prms[3] = params[3];
		prms[4] = params[4];
		prms[5] = params[5];
		ptr.Ptr()->Command("RedirectSoundEvent", 6, prms);
#endif
	}

public:
	//Проиграть звук
	void _cdecl PlaySound(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
		if(!sound || numParams < 6) return;


		//api->Trace("Animation event \"Snd\": sound: %s, animation \"%s\", node \"%s\"", params[0], geometry->GetFileName(), ani->GetName(), ani->CurrentNode());


		LocalSound ls;
		ls.ani = ani;
		ls.locator.reset();
		if(string::NotEmpty(params[1]) && string::NotEmpty(params[2]))
		{
			RedirectSoundEvent(ani, name, params, numParams);
			return;
		}
		ls.locator = FindLocator(params[1]);
		if(params[3][0] == 'y' || params[3][0] == 'Y')
		{
			ls.nodeName = ani->CurrentNode();
			char * pos = null;
			ls.fadeoutTime = (float)strtod(params[4], &pos);
			ls.fadeoutTime = Clampf(ls.fadeoutTime, 0.001f, 100.0f);
		}else{
			ls.nodeName = null;
		}
		if(ls.locator.isValid())
		{
			Assert(geometry);
			ls.snd3D = sound->Create3D(params[0], matrix*geometry->GetNodeLocalTransform(ls.locator).pos, _FL_, false, false);
		}else{			
			if(string::IsEmpty(params[1]))
			{
				ls.snd = sound->Create(params[0], _FL_, false, false);
			}else{
				api->Trace("Animation event \"Snd\": Can't play sound, locator \"%s\" not found, animation %s, node %s", params[1], ani->GetName(), ani->CurrentNode());
				return;
			}
		}
		Assert(ls.snd == ls.snd3D);
		if(ls.snd)
		{
			if(ls.snd->IsLoop() && ls.nodeName == null)
			{
				api->Trace("Animation event \"Snd\": Can't support looped unbinded sounds. Stop sound %s, animation %s, node %s", ls.snd->GetName(), ani->GetName(), ani->CurrentNode());
				ls.snd->Release();
				return;
			}
			if(params[5][0] == 'y' || params[5][0] == 'Y')
			{
				StopVoice();
				ISound::ErrorCode ecode;
				ls.snd->GetPhonemeId(&ecode);
				bool isError = false;
				switch(ecode)
				{
				case ISound::ec_ok:
				case ISound::ec_phonemes_sound_not_play:
					break;
				case ISound::ec_phonemes_no_data:
					api->Trace("Animation event \"Snd\": No start voice animation, because sound \"%s\" have no phonemes data, animation \"%s\", node \"%s\"", ls.snd->GetName(), ani->GetName(), ani->CurrentNode());
					isError = true;
					break;
				case ISound::ec_phonemes_data_is_empty:
					api->Trace("Animation event \"Snd\": No start voice animation, because sound \"%s\" have empty phonemes data (no install xphonemes when export bank), animation %s, node %s", ls.snd->GetName(), ani->GetName(), ani->CurrentNode());
					isError = true;
					break;
				default:
					api->Trace("Animation event \"Snd\": No start voice animation, because sound \"%s\" have not desciption error: %i, animation \"%s\", node \"%s\"", ls.snd->GetName(), (long)ecode, ani->GetName(), ani->CurrentNode());
					isError = true;
				}
				if(!geometry)
				{
					api->Trace("Animation event \"Snd\": No start voice animation, because not set model for that, animation \"%s\", node \"%s\"", ls.snd->GetName(), ani->GetName(), ani->CurrentNode());
					isError = true;
				}else{
					const dword needBlandshapesCount = 17;
					dword needCount = geometry->GetBlendShapesCount();
					if(needCount < needBlandshapesCount)
					{
						api->Trace("Animation event \"Snd\": No start voice animation, because model \"%s\" have not enough blendshapes (%i of %i), animation \"%s\", node \"%s\"", geometry->GetFileName(), needCount, needBlandshapesCount, ani->GetName(), ani->CurrentNode());
						isError = true;
					} else
					{
						if (needCount > needBlandshapesCount)
						{
							api->Trace("Animation event \"Snd\": Warning !! Model \"%s\" have too many blendshapes (%i of %i), animation \"%s\", node \"%s\"", geometry->GetFileName(), needCount, needBlandshapesCount, ani->GetName(), ani->CurrentNode());
						}
					}
				}
				if(isError)
				{
					ls.snd->Release();
					ls.snd = null;
					return;
				}
				voice = ls.snd;
			}
			if(ls.locator.isValid())
			{
				Assert(geometry);
				geometry->AddRef();
			}
			sounds.Add(ls);
			ls.snd->Play();
		}
	}

	//Завести партикловую систему в локаторе
	void _cdecl PlayParticles(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
		if(!particle || numParams < 2) return;
		GMXHANDLE loc = FindLocator(params[0]);
		if(loc.isValid())
		{
			IParticleSystem * prt = particle->CreateParticleSystem(params[1]);
			if(prt)
			{
				Assert(geometry);
				Matrix m(geometry->GetNodeLocalTransform(loc),matrix);
				float kx = m.vx.Normalize();
				float ky = m.vy.Normalize();
				float kz = m.vz.Normalize();
				float scale = (kx + ky + kz)*(1.0f/3.0f);
				prt->Teleport(m);
				LocalParticles & lp = particles[particles.Add()];
				if(numParams >= 3 && params[2][0] != 0)
				{
					char * pos = null;
					float s = (float)strtod(params[2], &pos);
					scale *= s;
				}
				prt->SetScale(scale);
				prt->AutoDelete(false);
				prt->Restart(rand());
				lp.locator.reset();
				lp.prt = prt;
			}else{
				api->Trace("AnimationStdEvents: Particels \"%s\" not created", params[1]);
			}
		}else{
			api->Trace("Particles created in locator : Locator \"%s\" not found, animation %s, node %s", params[0], ani->GetName(), ani->CurrentNode());
		}
	}

	//Завести партикловую систему привязанную к локатору
	void _cdecl LocParticles(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
		if(!particle || numParams < 2) return;
		GMXHANDLE loc = FindLocator(params[0]);
		if(loc.isValid())
		{
			Assert(geometry);
			IParticleSystem * prt = particle->CreateParticleSystem(params[1]);
			if(prt)
			{
				prt->AutoDelete(false);
				prt->AttachTo(geometry, loc, false);
				prt->Restart(rand());
				LocalParticles & lp = particles[particles.Add()];
				if(numParams >= 3 && params[2][0] != 0)
				{
					char * pos = null;
					float scale = (float)strtod(params[2], &pos);
					prt->SetScale(scale);
				}
				lp.locator = loc;
				if(lp.locator.isValid())
				{
					geometry->AddRef();
				}
				lp.prt = prt;
			}else{
				api->Trace("AnimationStdEvents: Particels \"%s\" not created", params[1]);
			}
		}else{
			api->Trace("Particles connected to locator : Locator \"%s\" not found, animation %s, node %s", params[0], ani->GetName(), ani->CurrentNode());
		}
	}

	//Удалить партикловую систему привязанную к локатору
	void _cdecl DelLocParticles(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
		if(numParams < 2) return;
		bool isStopEmmision = false;
		if(numParams >= 3 && params[2] != null)
		{
			if(params[2][0] == 's' || params[2][0] == 'S')
			{
				isStopEmmision = true;
			}
		}
		if(params[0] != null && params[0][0] != 0)
		{
			GMXHANDLE loc = FindLocator(params[0]);
			if(loc.isValid())
			{
				//Удалить с локатора
				for(long i = 0; i < particles; i++)
				{
					if(particles[i].locator == loc)
					{
						const char * name = particles[i].prt->GetName();
						if(string::IsEqual(name, params[1]))
						{
							if(isStopEmmision)
							{
								particles[i].prt->PauseEmission(true);
							}else{
								RELEASE(particles[i].prt);
								ReleaseLocator(particles[i].locator);
								particles.Extract(i);
								i--;
							}
						}
					}
				}
			}
		}else{
			//Удалить по имени
			for(long i = 0; i < particles; i++)
			{
				const char * name = particles[i].prt->GetName();
				if(string::IsEqual(name, params[1]))
				{
					if(isStopEmmision)
					{
						particles[i].prt->PauseEmission(true);
					}else{
						RELEASE(particles[i].prt);
						ReleaseLocator(particles[i].locator);
						particles.Extract(i);
						i--;
					}
				}
			}
		}
	}

	//Удалить все партикловые систем
	void _cdecl DelAllParticles(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
		bool isStopEmmision = false;
		if(numParams >= 1 && params[0] != null)
		{
			if(params[0][0] == 's' || params[0][0] == 'S')
			{
				isStopEmmision = true;
			}
		}
		if(isStopEmmision)
		{
			for(long i = 0; i < particles; i++)
			{
				particles[i].prt->PauseEmission(true);
			}
		}else{
			for(long i = 0; i < particles; i++)
			{
				RELEASE(particles[i].prt);
				ReleaseLocator(particles[i].locator);
			}
			particles.Empty();
		}
	}

	//Сжать кость в 0
	void _cdecl BoneCollapse(IAnimation * _ani, const char * name, const char ** params, dword numParams)
	{
		if(numParams < 2) return;
		long i = _ani->FindBone(params[1], true);
		if(i >= 0)
		{
			if(params[0][0] == 'c' || params[0][0] == 'C')
			{
				collapser.Collapse(i, ani.GetBuffer(), ani.Size());
			}else{
				collapser.Restore(i, ani.GetBuffer(), ani.Size());
			}
		}
	}

	//Деактивировать-активировать объект миссии
	void _cdecl ChangeActiveObject(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
#ifndef _ANI_EVENTS_NO_MISSION_
		if(!mission)
		{
			api->Trace("Animation: object %s can't be activated because can not set mission", params[0]);
			return;
		}
		if(numParams < 2 || !params[0] || !params[1])
		{
			mission->LogicDebugError("Animation: object can't be activated because invalidate event params");
			return;
		}
		MOSafePointer mo;
		mission->FindObject(ConstString(params[0]), mo);
		if(mo.Validate())
		{
			bool isActive = false;
			if(params[1][0] == 'a' || params[1][0] == 'A')
			{
				isActive = true;
			}
			mission->LogicDebug("Animation: %s object %s from animation %s", isActive ? "activate" : "deactivate", params[0], ani->GetName());
			mo.Ptr()->Activate(isActive);
		}else{
			mission->LogicDebugError("Animation: object %s can't be activated because has not been found", params[0]);
		}
#endif
	}

	//Создать взрыв поверхности в локаторе или в центре геометрии
	void _cdecl PatchExplosion(IAnimation * ani, const char * name, const char ** params, dword numParams)
	{
#ifndef _ANI_EVENTS_NO_MISSION_
		if (!geometry || !mission) 
			return;

		if (numParams < 1) 
		{
			api->Trace("PatchExplosion with zero parameters: ani = %s, name = %s", ani->GetName(), name);
			return;
		}

		// тип взрыва патча
		if (params[0][0] == 0)
		{
			api->Trace("PatchExplosion with zero explosion type, ani = %s, name = %s", ani->GetName(), name);
			return;
		}

		Matrix mtx(true);
		Vector pos;

		ConstString type; 
		type.Set(params[0]);

		// имя локатора если есть, иначе возьмем позицию персонажа
		if (numParams >= 2 && params[1][0] != 0)
		{
			GMXHANDLE locHandle = FindLocator(params[1]);
			if (!locHandle.isValid())
			{
				api->Trace("Can't find locator %s, ani = %s, name = %s", params[1], ani->GetName(), name);
				return;
			}
			pos = geometry->GetNodeWorldTransform(locHandle, mtx).pos;
		}
		else
			pos = geometry->GetTransform().pos;

		IExplosionPatch * explosionPatch = IExplosionPatch::GetExplosionPatch(*mission);
		if (explosionPatch)
			explosionPatch->MakeExplosion(pos, type);
#endif
	}
	
	inline GMXHANDLE FindLocator(const char * name)
	{
		if(!geometry || !name) return GMXHANDLE();

		return geometry->FindEntity(GMXET_LOCATOR, name);
	}

	inline void ReleaseLocator(GMXHANDLE & locator)
	{
		if(locator.isValid())
		{
			Assert(geometry);
			geometry->Release();
			locator.reset();
		}
	}


protected:

	//Модель
	IGMXScene * geometry;
	Matrix matrix;

	//Службы
	ISoundScene * sound;
	IParticleManager * particle;

	//Массив заведёных звуков
	struct LocalSound
	{
		GMXHANDLE locator;
		union
		{
			ISound * snd;
			ISound3D * snd3D;
		};
		IAnimation * ani;
		const char * nodeName;
		float fadeoutTime;
	};
	array<LocalSound> sounds;


	//Текущие параметры лицевой анимации
	struct ActiveBlendShapes
	{
		long id;
		float k;
	};
	
	ISound * voice;
	long activeBlendShape;
	long blendShapesCount;
	ActiveBlendShapes blendShapes[8];


	//Массив заведёных партиклов
	struct LocalParticles
	{
		GMXHANDLE locator;
		IParticleSystem * prt;
	};

	array<LocalParticles> particles;

	IMission * mission;

	//Коллапс костей
	class BoneCollapser : public IAniBlendStage
	{
	public:
		BoneCollapser()
		{
			for(long i = 0; i < ARRSIZE(collapsedBone); i++)
			{
				collapsedBone[i] = 0;
			}
			workCount = 0;
		}

		virtual ~BoneCollapser()
		{
		}
		
		//Получить коэфициент блендинга для интересующей кости
		virtual float GetBoneBlend(long boneIndex)
		{
			boneIndex &= 255;
			long index = boneIndex >> 5;
			long mask = 1 << (boneIndex & 31);
			if(collapsedBone[index] & mask)
			{
				return 1.0f;
			}
			return 0.0f;
		}

		//Получить трансформацию кости
		virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
		{
			rotation.SetIdentity();
			position = 0.0f;
			scale = 1e-10f;
		}

		//Сколапсить кость
		void Collapse(long boneIndex, IAnimation ** animations, dword animationsCount)
		{
			//Отмечаем сколапшеную кость
			boneIndex &= 255;
			long index = boneIndex >> 5;
			long mask = 1 << (boneIndex & 31);
			collapsedBone[index] |= mask;
			//Регистрируемся
			if(!workCount)
			{
				for(dword i = 0; i < animationsCount; i++)
				{
					animations[i]->RegistryBlendStage(this, aminationBlendLevel_postProcess);
				}
			}
			workCount++;
		}

		//Развернуть кость
		void Restore(long boneIndex, IAnimation ** animations, dword animationsCount)
		{
			//Не делаем лишнего
			if(!workCount)
			{
				return;
			}
			//Отмечаем развернувшиюся кость
			boneIndex &= 255;
			long index = boneIndex >> 5;
			long mask = 1 << (boneIndex & 31);
			collapsedBone[index] &= ~mask;
			workCount--;
			if(workCount <= 0)
			{
				Assert(workCount == 0);
				for(dword i = 0; i < animationsCount; i++)
				{
					animations[i]->UnregistryBlendStage(this);
				}
			}
		}

		//Зарегистрировать в новом списке анимаций
		void Registry(IAnimation ** animations, dword animationsCount)
		{
			if(workCount)
			{
				for(dword i = 0; i < animationsCount; i++)
				{
					animations[i]->RegistryBlendStage(this, aminationBlendLevel_postProcess);
				}
			}
		}

		//Отписаться из списка анимаций
		void Unregistry(IAnimation ** animations, dword animationsCount)
		{
			if(workCount)
			{
				for(dword i = 0; i < animationsCount; i++)
				{
					animations[i]->UnregistryBlendStage(this);
				}
			}
		}

		void Reset(IAnimation ** animations, dword animationsCount)
		{
			for(long i = 0; i < ARRSIZE(collapsedBone); i++)
			{
				collapsedBone[i] = 0;
			}
			workCount = 0;
			for(dword i = 0; i < animationsCount; i++)
			{
				animations[i]->UnregistryBlendStage(this);
			}
		}

	private:
		dword collapsedBone[8];		//Массив сколапшеных костей
		long workCount;				//Состояние 
	};
	BoneCollapser collapser;

};

#endif

