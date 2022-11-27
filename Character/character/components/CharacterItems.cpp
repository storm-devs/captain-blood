//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharacterItems
//===========================================================================================================================

#include "CharacterItems.h"
#include "CharacterLogic.h"
#include "CharacterPhysics.h"
#include "CharacterArmor.h"
#include "CharacterAnimationEvents.h"

#define ITEMS_SPLITDIST	0.1f	//Дистанция рассечения шлейфа на дополнительные элементы meters

extern bool PhysRigidBodyActorCheckMtx(const Matrix & mtx);

bool CharacterItems::isCharacterSwordDebug = false;

CharacterItems::CharacterItems(Character * character) : chr(*character),
														items(_FL_, 4),
														locators(_FL_)
#ifndef STOP_DEBUG
														,swDebug(_FL_, 256)
														,swlDebug(_FL_, 256)
#endif
{
	dynamicLighting = false;

	Flare.fTime = 0.0f;
	Flare.Show  = false;

	fMaxFlareTime = 0.75f;

	max_armor_damage_absorb = 0.99f;

	lastactive_effectTable = null;

	isAlwaysArmor = false;

	armor_hp_drop = 50;
	armor_drop_prop = 0.3f;


}

CharacterItems::~CharacterItems()
{
	//Удаляем итемы
	for(long i = 0; i < locators; i++)
	{ 
		if(locators[i].locator.isValid())
		{
			locators[i].scene->Release();
			locators[i].locator.reset();
		}
	}
	for(long i = 0; i < items; i++)
	{
		if (items[i].model)
		{
			items[i].model->Release();
			items[i].model = null;
		}
		if (items[i].model_part1)
		{
			items[i].model_part1->Release();
			items[i].model_part1 = null;
		}
		if (items[i].model_part2)
		{
			items[i].model_part2->Release();
			items[i].model_part2 = null;
		}

		if(items[i].trail)
		{			
			delete items[i].trail;
			items[i].trail = null;
		}
	}
}

//Установить флаг динамического освещения
void CharacterItems::SetDynamicLightingFlag(bool isDynamicLighting)
{
	dynamicLighting = isDynamicLighting;
	for(long i = 0; i < items; i++)
	{
		if(items[i].model)
		{
			items[i].model->SetDynamicLightState(dynamicLighting);
		}
	}
}

void CharacterItems::ReserveElements(dword reserveCount)
{
	if (items.IsEmpty() && reserveCount)
	{
		items.SetAddElements(1);
		items.Reserve(reserveCount);
		items.SetAddElements(4);
	}
}

//Добавить итем
bool CharacterItems::AddItem(CharacterPattern::TItemDescr* descr)
{
	//Банальные проверки
	if((!descr->modelName || !descr->modelName[0]) && descr->weapon_type != wp_armor) return false;
	if((!descr->boneName || !descr->boneName[0]) && descr->weapon_type == wp_armor) return false;

	if(descr->locatorName.IsEmpty()) return false;
	//Ищем локатор
	long index = GetLocatorID(descr->locatorName);
	if(index < 0)
	{
		//Пробуем добавить геометрический локатор
		index = AddLocator(descr->locatorName);
		if(index < 0) return false;
	}	
		
	IGMXScene* model = null;
	
	Vector bMin,bMax;

	if (descr->weapon_type != wp_armor)
	{	
		model = chr.Geometry().CreateScene(descr->modelName, &chr.Animation(), &chr.Particles(), &chr.Sound(), _FL_);	

		if(!model)
		{				
			return false;
		}

		model->SetDynamicLightState(dynamicLighting);
	}
	else
	{
		if (!chr.armor) return false;
		if (!chr.armor->RegisterArmor(descr->boneName,items.Size(),bMin,bMax)) return false;
	}

	//Добавляем запись об итеме
	ItemData & data = items[items.Add()];
	
	data.drp_data.bMin = bMin;
	data.drp_data.bMax = bMax;

	if(descr->id.NotEmpty())
	{
		data.id = descr->id;
	}else{
		data.id.Set("");
	}
	data.weapon_type = descr->weapon_type;
	data.tip_id = descr->tip_id;
	data.model_name = descr->modelName;
	data.model = model;



	data.model_part1 = null;
	data.model_part1_name = null;

	if (descr->broken_part1)
	{
		data.model_part1 = chr.Geometry().CreateGMX(descr->broken_part1, &chr.Animation(), &chr.Particles(), &chr.Sound());	
		data.model_part1_name = descr->broken_part1;
		if (data.model_part1) data.model_part1->SetDynamicLightState(true);
	}

	data.model_part2 = null;
	data.model_part2_name = null;
	
	if (descr->broken_part2)
	{
		data.model_part2 = chr.Geometry().CreateGMX(descr->broken_part2, &chr.Animation(), &chr.Particles(), &chr.Sound());
		data.model_part2_name = descr->broken_part2;
		if (data.model_part2) data.model_part2->SetDynamicLightState(true);
	}

	data.locator = index;
	data.showFlare = descr->showFlare;
	data.attachedobject = descr->attachedobject;
	data.locatortoattache = descr->locatortoattache;
	data.objectoffset = descr->objectoffset;
	data.visible = true;
	data.itemLife = descr->itemLife;	
	data.fProbality = descr->fProbality;

	data.time_lie = descr->time_lie;

	data.armor_damage_absorb = descr->armor_damage_absorb;
	data.drop_priority = descr->drop_priority;

	data.armor_mtl = descr->armor_mtl;

	data.effectTable.Reset();

	if (descr->effectTable.NotEmpty())
	{
		static const ConstString strTypeId("EffectTable");
		data.effectTable.FindObject(&chr.Mission(), descr->effectTable, strTypeId);		
	}

	data.mobj.Reset();
	if (data.attachedobject.NotEmpty())
	{
		chr.FindObject(data.attachedobject, data.mobj);
	}	

	if(descr->useTrail)
	{			
		data.trail = NEW SwordTrail();		
		data.trail->effectTable = null;

		if(!data.trail->Init(&chr, data.model, 0xffffffff, descr->uniqTexture, descr->trailTechnique, descr->trailPower))
		{
			delete data.trail;
			data.trail = null;
		}
		else
		{
			data.trail->effectTable = data.effectTable.Ptr();			
		}
	}
	else
	{
		data.trail = null;
	}

	if (model)
	{	
		//Ищем и добавляем локаторы
		static const ConstString startLocator("start");
		static const ConstString endLocator("end");
		GMXHANDLE bladeStart = model->FindEntityByHashedString(GMXET_LOCATOR, startLocator.c_str(), startLocator.Hash(), startLocator.Len());
		GMXHANDLE bladeEnd = model->FindEntityByHashedString(GMXET_LOCATOR, endLocator.c_str(), endLocator.Hash(), endLocator.Len());

		if(bladeStart.isValid())
		{
			data.bladeStart = model->GetNodeLocalTransform(bladeStart).pos;
		}
		else
		{
			data.bladeStart = 0.0f;
		}
		if(bladeEnd.isValid())
		{
			data.bladeEnd = model->GetNodeLocalTransform(bladeEnd).pos;
		}
		else
		{
			data.bladeEnd = 0.0f;
		}
		data.isSetBladeLocators = bladeStart.isValid() && bladeEnd.isValid();
	}
	else data.isSetBladeLocators = false;

	data.isActive = false;

	data.color = 0xff000000;
	data.color.a = 0.0f;

	return true;
}

//Удалить итем
long CharacterItems::DelItems(const ConstString & id)
{
	long count = 0;
	for(long i = 0; i < items; i++)
	{
		if(items[i].id == id)
		{
			RELEASE(items[i].model);
			RELEASE(items[i].model_part1);
			RELEASE(items[i].model_part2);						

			if(items[i].trail)
			{				
				delete items[i].trail;
				items[i].trail = null;
			}			

			items.ExtractNoShift(i--);
			count++;
		}
	}
	return count;
}

//Добавить логический локатор
void CharacterItems::SetLogicLocator(const ConstString & logicName, const ConstString & locatorName)
{
	//Ищем логический локатор
	if(logicName.IsEmpty()) return;
	for(long i = 0; i < locators; i++)
	{
		if(locators[i].locator.isValid()) continue;
		//if(locators[i].locator != null) continue;
		if(locators[i].name == logicName)
		{
			//Замещаем текущий
			break;
		}
	}
	//Если такого логического не найдено, добавляем
	if(i >= locators)
	{
		Locator & loc = locators[i = locators.Add()];
		loc.name = logicName;
		loc.logicLocator = -1;
		loc.locator.reset();
	}
	//Устанавливаем логическому локатору геометрический
	Locator & loc = locators[i];
	loc.logicLocator = AddLocator(locatorName);
}

//Обновить состояние
void CharacterItems::Update(float dltTime)
{
	bool isNowAttack = (chr.logic->GetState() == CharacterLogic::state_attack);
	
	if(!isNowAttack)
	{
		if(chr.logic->IsAttack())
		{
			ActivateItem(ConstString(), false);
			chr.logic->SetAttackActive(false);
		}
		SetTrailVisible(ConstString(), false);
	}	
}

void CharacterItems::GetWeaponTrail(Vector& start,Vector& end,const ConstString & wp_id)
{	
	static Matrix mtx;

	//Ананализируем попадания
	for(long i = 0; i < items; i++)
	{
		//Смотрим в кого попали
		ItemData & item = items[i];

		if (item.id != wp_id) continue;

		//Получим позицию локатора
		if(!GetLocatorMatrix(item.locator, mtx, true)) continue;		
							
		
		//start = item.bladeStart * mtx;
		start = item.bladeStart * Vector(1.0f,1.0f,1.0f) * mtx;

		//start = mtx * item.bladeStart;
		//start.x=start.z = 0.0f;		
		//end = item.bladeEnd * Vector(1.5f,1.0f,1.5f) * mtx;
		end = item.bladeEnd * mtx;

		break;
	}	
}

void CharacterItems::Reset()
{
	int armor_set = -1;
	

	if (chr.pattern)
	{
		if (chr.pattern->armorsets.Size()>0)
		{
			armor_set = (int)(Rnd(0.999f) * chr.pattern->armorsets.Size());
		}
	}

	for (int i=0; i<(int)items.Size();i++)
	{
		items[i].visible = true;	
		items[i].color = 0xff000000;
		items[i].color.a = 0.0f;

		if (armor_set>=0)
		{
			if (items[i].weapon_type == wp_armor || items[i].weapon_type == wp_bomb) items[i].visible = false;
		}
	}
		
	if (armor_set>=0)
	{
		for (int i=0;i<(int)chr.pattern->armorsets[armor_set].id_armors.Size();i++)
		{
			ShowRandomArmor(chr.pattern->armorsets[armor_set].id_armors[i]);
		}
	}
}

//Нарисовать итемы
void CharacterItems::Draw(const Matrix & toWorld, const Color & userColor)
{
	lastDrawToWorld = toWorld;
	//Мировая матрица с учётом скалирования
	static Matrix mtx(true);

	//Рисуем содержимое инвенторя
	for(long i = 0; i < items; i++)
	{		
		ItemData & item = items[i];
		
		if (!item.visible) continue;
				
		if (chr.logic->IsActor() && item.id.NotEmpty() && item.id.c_str()[0] == 's' && item.id.c_str()[1] == 'e' &&item.id.c_str()[2] == 'c') continue;

		if (item.weapon_type == wp_armor)
		{			
			//GetLocatorMatrix(item.locator, item.drp_data.transform);
			//item.drp_data.transform = item.drp_data.transform;// * toWorld; 
			//item.drp_data.alpha = 1.0f;
		
			continue;
		}

		//Получим позицию локатора
		if(!GetLocatorMatrix(item.locator, mtx)) continue;
		mtx *= toWorld;
		
		IGMXScene* model = item.model;

		if(!model) continue;

		Color color;
		color.a = Clampf(userColor.a + item.color.a);
		color.r = Clampf(userColor.r + item.color.r);
		color.g = Clampf(userColor.g + item.color.g);
		color.b = Clampf(userColor.b + item.color.b);
		
		//color.a = 1.0f;
		//color.r = 1.0f;
		///color.g = 0.5f;
		//color.b = 0.0f;

		model->SetUserColor(color);

		model->SetTransform(mtx);
		model->Draw();

		if(item.attachedobject.NotEmpty())
		{
			if(!item.mobj.Validate())
			{
				chr.FindObject(item.attachedobject,item.mobj);
			}
			
			if (item.mobj.Ptr())
			{
				if(string::NotEmpty(item.locatortoattache))
				{
					GMXHANDLE loc = item.model->FindEntity(GMXET_LOCATOR, item.locatortoattache);

					if (loc.isValid())
						item.model->GetNodeWorldTransform(loc, mtx);
				}
				
				Matrix mat(true);
				chr.GetMatrix(mat);

				Vector offset = mat.MulNormal(item.objectoffset);

				crt_snprintf(params[0], ARRSIZE(params[0]),"%f",mtx.pos.x+offset.x);
				crt_snprintf(params[1], ARRSIZE(params[1]),"%f",mtx.pos.y+offset.y);
				crt_snprintf(params[2], ARRSIZE(params[2]),"%f",mtx.pos.z+offset.z);

				const char * prms[3];
				prms[0] = params[0];
				prms[1] = params[1];
				prms[2] = params[2];
				item.mobj.Ptr()->Command("moveto", 3, prms);
			}			
		}
	}


	//AnimationFrameUpdate();
#ifndef STOP_DEBUG
	if (isCharacterSwordDebug)
	{
		chr.Render().SetWorld(Matrix());
		chr.Render().FlushBufferedLines();
		for(long i = 0; i < swDebug; i++)
		{
	/*		if(!swDebug[i].isBase)
			{
				chr.Render().DrawBufferedLine(swDebug[i].quadrangle[0], 0xffff0000, swDebug[i].quadrangle[1], 0xffffff00);
				chr.Render().DrawBufferedLine(swDebug[i].quadrangle[2], 0xffffff00, swDebug[i].quadrangle[3], 0xffff0000);
			}else{
				chr.Render().DrawBufferedLine(swDebug[i].quadrangle[0], 0xffff0000, swDebug[i].quadrangle[1], 0xff00ffff);
				chr.Render().DrawBufferedLine(swDebug[i].quadrangle[2], 0xffffff00, swDebug[i].quadrangle[3], 0xff00ff00);
			}
			chr.Render().DrawBufferedLine(swDebug[i].quadrangle[1], 0xff0000ff, swDebug[i].quadrangle[2], 0xff0000ff);
			chr.Render().DrawBufferedLine(swDebug[i].quadrangle[0], 0xff0000ff, swDebug[i].quadrangle[3], 0xff0000ff);

	*/
			if(!swDebug[i].isBase)
			{
				chr.Render().DrawLine(swDebug[i].quadrangle[0], 0xffff0000, swDebug[i].quadrangle[1], 0xffffff00);
				chr.Render().DrawLine(swDebug[i].quadrangle[2], 0xffffff00, swDebug[i].quadrangle[3], 0xffff0000);
			}else{
				chr.Render().DrawLine(swDebug[i].quadrangle[0], 0xffff0000, swDebug[i].quadrangle[1], 0xff00ffff);
				chr.Render().DrawLine(swDebug[i].quadrangle[2], 0xffffff00, swDebug[i].quadrangle[3], 0xff00ff00);
			}
			chr.Render().DrawLine(swDebug[i].quadrangle[1], 0xff0000ff, swDebug[i].quadrangle[2], 0xff0000ff);
			chr.Render().DrawLine(swDebug[i].quadrangle[0], 0xff0000ff, swDebug[i].quadrangle[3], 0xff0000ff);

		}
		chr.Render().FlushBufferedLines();
		for(long i = 0; i < swlDebug; i++)
		{
			chr.Render().DrawLine(swlDebug[i].s, 0xffffff00, swlDebug[i].e, 0xffff00ff);
		}	
		chr.Render().FlushBufferedLines();
	}
#endif
}

//Нарнисовать шлейфы за итемами
void CharacterItems::DrawTrails(float dltTime, const Matrix & toWorld)
{
	//Рисуем следы для всех требуемых итемов
	Matrix mtx(true);
	for(long i = 0; i < items; i++)
	{
		ItemData & item = items[i];
		if(item.trail)
		{
			if (item.trail->IsTrailNotEmpty())
			{
				bool isLostPosition = !GetLocatorMatrix(item.locator, mtx);
				item.trail->Draw(toWorld, mtx, dltTime, isLostPosition);
			}
		}
	}
}

bool CharacterItems::FindLocatorinItems(const char* locatorName,Matrix & mtx)
{	
	Matrix mat(true);

	for(long i = 0; i < items; i++)
	{		
		ItemData & item = items[i];

		//Получим позицию локатора
		if(!GetLocatorMatrix(item.locator, mat)) continue;		
				
		if (item.model)
		{
			GMXHANDLE loc = item.model->FindEntity(GMXET_LOCATOR, locatorName);

			if (loc.isValid())
			{				
				item.model->GetNodeWorldTransform(loc, mtx);
	
				return true;
			}
		}
	}

	return false;
}

bool CharacterItems::FindLocatorinItem(int index, const char* locatorName,Matrix & mtx)
{	
	if (index<0 || index>=(int)items.Size()) return false;

	Matrix mat(true);
	
	ItemData & item = items[index];

	//Получим позицию локатора
	if(!GetLocatorMatrix(item.locator, mat)) return false;		

	GMXHANDLE loc = item.model->FindEntity(GMXET_LOCATOR, locatorName);

	if (loc.isValid())
	{				
		item.model->GetNodeWorldTransform(loc, mtx);

		return true;
	}

	return false;
}

int CharacterItems::FindItemIndex(const ConstString & item_id)
{
	for(long i = 0; i < items; i++)
	{				
		if (items[i].id == item_id) return i;
	}

	return -1;
}

float CharacterItems::GetLastWeaponLive()
{
	if (items.Size()==0) return false;

	return items[items.Last()].itemLife;

}

void CharacterItems::DamageWeapon(bool kill, float damage)
{
	for (int i=0;i<(int)items.Size();i++)
	if (items[i].itemLife!=-1)
	{
		if (kill)
		{
			items[i].itemLife = 0;
		}
		else
		{
			if (items[i].itemLife > 0) 
				items[i].itemLife = Max(0.0f, items[i].itemLife - damage);
		}
	}	
}

void CharacterItems::ShowWeapon(bool show,const ConstString & id)
{	
	for(long i = 0; i < items; i++)
	if (items[i].id == id)
	{
		items[i].visible = show;
	}	
}

void CharacterItems::DrawFlares(float dltTime, const Matrix & toWorld)
{
	if (chr.logic->IsDead()) return;

	if (chr.ragdoll) return;

	if (chr.logic->IsPairMode()) return;

	if (chr.logic->GetState()==CharacterLogic::state_aimbomb||
		chr.logic->GetState()==CharacterLogic::state_dropbomb||
		chr.logic->GetState()==CharacterLogic::state_findpair||
		chr.logic->GetState()==CharacterLogic::state_shoot) return;


	Matrix mtx(true);

	for(long i = 0; i < items; i++)
	if (items[i].showFlare)
	{
		Flare.fTime -= dltTime;

		if (Flare.fTime<0)
		{	
			Flare.fTime = 0.0f;

			if (Flare.Show)
			{
				Flare.Show = false;
				Flare.fTime = 3.5f + Rnd(3.0f);
			}
		}

		//Flare.Show = true;
		//Flare.fTime = 0.5f;

		//Flare.Show = true;

		
		ItemData & item = items[i];

		//Получим позицию локатора
		if(!GetLocatorMatrix(item.locator, mtx)) continue;
		mtx *= toWorld;

		//рисуем фларик
		{
			static const ConstString nameFlash("flash");
			static const ConstString nameTraile("traile");
			static const ConstString nameTrails("trails");

			GMXHANDLE loc = item.model->FindEntityByHashedString(GMXET_LOCATOR, nameFlash.c_str(), nameFlash.Hash(), nameFlash.Len());

			if (loc.isValid())
			{
				GMXHANDLE loc1 = item.model->FindEntityByHashedString(GMXET_LOCATOR, nameTraile.c_str(), nameTraile.Hash(), nameTraile.Len());
				GMXHANDLE loc2 = item.model->FindEntityByHashedString(GMXET_LOCATOR, nameTrails.c_str(), nameTrails.Hash(), nameTrails.Len());

				Vector vDir = Vector(1.0f,0.0f,0.0f);

				vDir = mtx.MulNormal(vDir);

				Vector vCamDir = Vector(0.0f,0.0f,1.0f);

				Matrix mView = mView = chr.Render().GetView();
				mView.Inverse();

				vCamDir = mView.MulNormal(vCamDir);

				float scale = vDir.GetAngle(vCamDir)/PI*2.0f;

				if (scale<0.0f) scale = -scale;

				scale = 1.0f - scale;

				if (Flare.Show)
				{			
					scale = fabs(scale);

					if (Flare.fTime<0.35f*fMaxFlareTime)
					{
						scale = Flare.fTime/(0.35f*fMaxFlareTime);
					}
					else
						if (Flare.fTime<0.65f*fMaxFlareTime)
						{						
							scale = 1.0f;
						}
						else
						{						
							scale = 1.0f - (Flare.fTime-0.65f*fMaxFlareTime)/(0.35f*fMaxFlareTime);
						}

						Matrix mtx1, mtx2;

						item.model->GetNodeWorldTransform(loc, mtx);
						item.model->GetNodeWorldTransform(loc1, mtx1);
						item.model->GetNodeWorldTransform(loc2, mtx2);

						mtx = mtx*chr.Render().GetView();

						float ang = mtx.vz.z*2.0f;

						float klerp = 1.0f - (0.5f + 0.5f*mtx.vy.z);

						klerp = Flare.fTime/(fMaxFlareTime);

						mtx.vx = mtx.vz;
						mtx.vx.z = 0.0f;
						float kscale = mtx.vx.Normalize();
						mtx.vz = Vector(0.0f, 0.0f, 1.0f);
						mtx.vy = mtx.vz ^ mtx.vx;
						Matrix mtx11 = mtx;
						mtx.Scale3x3(kscale*0.95f*scale);

						Vector xxxpos = mtx.pos;

						mtx = mtx*Matrix(chr.Render().GetView()).Inverse();

						mtx.pos.Lerp(mtx1.pos,mtx2.pos, klerp );															

						chr.arbiter->DrawFlare(0,mtx,1.0f);

						mtx11.BuildRotateZ(ang);
						mtx11.pos = xxxpos;
						mtx11.Scale3x3(sqrtf(kscale)*0.95f*scale);
						mtx11 *= Matrix(chr.Render().GetView()).Inverse();

						mtx11.pos.Lerp(mtx1.pos,mtx2.pos, klerp );

						chr.arbiter->DrawFlare(1,mtx11,1.0f);
				}
				else
				if (!Flare.Show && Flare.fTime==0)
				{
					if (fabs(scale)>0.75f && Rnd()>0.5f)
					{
						Flare.Show = true;
						Flare.fTime = fMaxFlareTime;
					}
				}
			}			
		}
	}
}

//Получить индекс локатора итема
long CharacterItems::GetLocatorID(const ConstString & locatorName)
{
	//Сначала просматриваем логические локаторы
	for(long i = 0; i < locators; i++)
	{
		if(locators[i].locator.isValid()) continue;
		if(locators[i].name == locatorName)
		{
			return i;
		}
	}
	//Теперь геометрические
	for(long i = 0; i < locators; i++)
	{
		if(!locators[i].locator.isValid()) continue;
		if(locators[i].name == locatorName)
		{
			return i;
		}
	}
	return -1;
}

//Получить матрицу локатора
bool CharacterItems::GetLocatorMatrix(long index, Matrix & mtx, bool updateHierarchy)
{
	//Если нет такого, то никакая матрица
	if(index < 0)
	{
		mtx.SetZero();
		return false;
	}
	//Ищем указатель на локатор
	IGMXScene * scene = chr.model.scene;//locators[index].scene;
	GMXHANDLE locator = locators[index].locator;
	
	if (!locator.isValid())
	{
		index = locators[index].logicLocator;
		if(index < 0)
		{
			mtx.SetZero();
			return false;
		}
		//scene = locators[index].scene;
		locator = locators[index].locator;
		if(!locator.isValid())
		{
			mtx.SetZero();
			return false;
		}
	}
	
	if(updateHierarchy)
	{
		//Ебать, какая длинная %)
		// FIX-ME толи нужно с новой геометрией, толи не нужно
		//locator->TransformationIsSpoiledOnAllParentAnimationLocators();
		scene->SpoilTransformationsCache();
	}

	mtx = chr.model.scene->GetNodeLocalTransform(locator);

	// Вставил Вано, проверка на сколапшенную кость
	if (mtx.vx.GetLength2() < 0.9f)
		return false;

#ifndef STOP_DEBUG
	bool goodMtx = PhysRigidBodyActorCheckMtx(mtx);
	if (!goodMtx)
	{
		api->Trace("ERROR: BAD BAD BAD MATRIX FROM GetLocatorMatrix");
		// FIX-ME
		//api->Trace("locator = %s", locator->GetName());
		api->Trace("vx = (%f, %f, %f; %f); len = %f", mtx.vx.x, mtx.vx.y, mtx.vx.z, mtx.wx, mtx.vx.GetLength());
		api->Trace("vy = (%f, %f, %f; %f); len = %f", mtx.vy.x, mtx.vy.y, mtx.vy.z, mtx.wy, mtx.vy.GetLength());
		api->Trace("vz = (%f, %f, %f; %f); len = %f", mtx.vz.x, mtx.vz.y, mtx.vz.z, mtx.wz, mtx.vz.GetLength());
		api->Trace("pos = (%f, %f, %f; %f)", mtx.pos.x, mtx.pos.y, mtx.pos.z, mtx.w);
	}
#endif
	return true;
}

//Добавить для использования локатор
long CharacterItems::AddLocator(const ConstString & locatorName)
{
	if(!chr.model.scene || locatorName.IsEmpty()) return -1;
	//Если есть, то ничего не заведём
	for(long i = 0; i < locators; i++)
	{
		if (!locators[i].locator.isValid()) continue;
		if (locators[i].name == locatorName)
		{
			return i;
		}
	}
	//Ищем локатор в геометрии
	GMXHANDLE loc = chr.model.scene->FindEntityByHashedString(GMXET_LOCATOR, locatorName.c_str(), locatorName.Hash(), locatorName.Len());
	if (loc.isValid())
	{
		Locator & locator = locators[i = locators.Add()];
		locator.name = locatorName;
		locator.logicLocator = -1;
		locator.scene = chr.model.scene;
		locator.locator = loc;
		locator.scene = chr.model.scene;
		chr.model.scene->AddRef();
		return i;
	}
	return -1;
}

//Запустить партикловую систему привязанную в локаторе итемов
void CharacterItems::StartParticlesOnItems(const ConstString & id, const char * itemLocatorName, const char * particlasName, bool isAttach, const Matrix & toWorld, float scale, bool normalize)
{	
	Matrix mtx(true);
	//Перебераем все итемы
	for(long i = 0; i < items; i++)
	{
		ItemData & item = items[i];
		if(item.id != id) continue;

		if (!item.visible) return;

		//Ищем локатор		
		GMXHANDLE loc = item.model->FindEntity(GMXET_LOCATOR, itemLocatorName);

		if (!loc.isValid()) continue;

		//Получаем позицию локатора
		if(GetLocatorMatrix(item.locator, mtx))
		{						
			Matrix loc_mat = item.model->GetNodeLocalTransform(loc) * mtx * toWorld;

			if (normalize)
			{
				Vector vx = loc_mat.vx;
				Vector vz = loc_mat.vz;

				vx.y = 0.0f;
				vx.Normalize();

				vz.y = 0.0f;
				vz.Normalize();

				loc_mat.vx = vx;
				loc_mat.vy = Vector(0.0f,1.0f,0.0f);
				loc_mat.vz = vz;

				loc_mat.pos.y = chr.physics->GetPos().y + 1.5f;				
			}

			chr.events->CreateParticles(particlasName, item.model, loc, isAttach, loc_mat, scale);
		}
	}
}

//Изменить видимость шлейфа за итемом
void CharacterItems::SetTrailVisible(const ConstString & id, bool isVisible)
{
	for(long i = 0; i < items; i++)
	{
		ItemData & item = items[i];
		if(id.NotEmpty())
		{
			if(item.id == id)
			{
				if(item.trail)
				{
					if(isVisible)
					{
						item.trail->StartTrace();
					}else{
						item.trail->StopTrace();
					}
				}
			}
		}else{
			if(item.trail)
			{
				if(isVisible)
				{
					item.trail->StartTrace();
				}else{
					item.trail->StopTrace();
				}
			}
		}
	}
}


//Изменить активность оружия
void CharacterItems::ActivateItem(const ConstString & id, bool isActive)
{
	bool isAttackNow = false;
	for(long i = 0; i < items; i++)
	{
		ItemData & item = items[i];
		if(id.NotEmpty())
		{
			if(item.id == id)
			{
				if(item.isSetBladeLocators)
				{
					item.isActive = isActive;
					item.collision.Empty();
				}
			}
		}else{
			if(item.isSetBladeLocators)
			{				
				item.isActive = isActive;
				item.collision.Empty();
			}
		}
		isAttackNow |= item.isActive;
	}
	chr.logic->SetAttackActive(isAttackNow);

#ifndef STOP_DEBUG
	if (isCharacterSwordDebug && isAttackNow)
	{
		swDebug.DelAll();
		swlDebug.DelAll();
		api->Trace("isAttackNow");
	}
#endif
}

//Расчёт повреждений от атак
void CharacterItems::AttackProcess(array<Character *> & characters)
{
	if(!chr.logic->IsAttack()) return;
	
	bool isHit = false;

	//Ананализируем попадания
	for(long i = 0; i < items; i++)
	{
		//Смотрим в кого попали
		ItemData & item = items[i];
		if(item.isActive && item.collision > 1)
		{
			lastactive_effectTable = item.effectTable.Ptr();

			for(long i = 1; i < item.collision; i++)
			{
				BladeLine * lines = &item.collision[i - 1];
				static Vector quadrangle[4];
				quadrangle[0] = lines[1].s;
				quadrangle[1] = lines[1].e;
				quadrangle[2] = lines[0].e;
				quadrangle[3] = lines[0].s;

				isHit = isHit | AttackProcessForBlade(characters, quadrangle);								

				quadrangle[0] = lines[1].e;
				quadrangle[1] = lines[1].s;
				quadrangle[2] = lines[0].s;
				quadrangle[3] = lines[0].e;

				isHit = isHit | AttackProcessForBlade(characters, quadrangle);								

/*				RecursiveSplitAttackQuadrangle(characters, quadrangle, 0);
#ifdef CharacterSwordDebug
				SwordDebug & swd = swDebug[swDebug.Add()];
				swd.quadrangle[0] = quadrangle[0];
				swd.quadrangle[1] = quadrangle[1];
				swd.quadrangle[2] = quadrangle[2];
				swd.quadrangle[3] = quadrangle[3];
				swd.isBase = true;
#endif*/
			}
			item.collision[0] = item.collision[item.collision.Last()];
			item.collision.DelRange(1, item.collision.Last());
		}			
	}

	lastactive_effectTable = null;

	if (isHit)
	{
		for(long i = 0; i < items; i++)
		{
			//Смотрим в кого попали
			ItemData & item = items[i];

			if (item.itemLife!=-1)
			{
				item.itemLife--;

				if (item.itemLife<0)
				{
					item.itemLife = 0.0f;
				}
			}
		}
	}
}

//Рекурсивное деление дистанции трейла
void CharacterItems::RecursiveSplitAttackQuadrangle(array<Character *> & characters, Vector quadrangle[4], dword dwLevel)
{	
	//Вектора позиций
	Vector dirs = quadrangle[1] - quadrangle[0];
	Vector dire = quadrangle[2] - quadrangle[3];
	float lenStart = dirs.Normalize();
	float lenEnd = dire.Normalize();
	//Косинус угла между ними
	float cs = dirs | dire;
	if(fabs(cs) > 0.95f || dwLevel >= 8)
	{
		AttackProcessForBlade(characters, quadrangle);
		return;
	}
	//Разделяем дистанцию пополам
	Vector s = (quadrangle[0] + quadrangle[3])*0.5f;
	Vector e = (quadrangle[1] + quadrangle[2])*0.5f;
	Vector n = !(e - s);
	//
	float k = (1.0f - cs)*0.001f;
	s += n*(sqrtf(~(quadrangle[0] + quadrangle[3]))*k);
	//
	float newLen = (lenStart + lenEnd)*0.5f;
	e = s + n*newLen;
	//Продолжаем рекурсию
	Vector q[4];
	q[0] = s;
	q[1] = e;
	q[2] = quadrangle[2];
	q[3] = quadrangle[3];
	RecursiveSplitAttackQuadrangle(characters, q, (dwLevel + 1));
	q[0] = quadrangle[0];
	q[1] = quadrangle[1];
	q[2] = e;
	q[3] = s;
	RecursiveSplitAttackQuadrangle(characters, q, (dwLevel + 1));
}

bool CharacterItems::CheckHit(Character* c, Vector quadrangle[4])
{
	//Построем описывающий бокс
	Vector vmin = quadrangle[0], vmax = quadrangle[0];
	vmin.Min(quadrangle[1]); vmax.Max(quadrangle[1]);
	vmin.Min(quadrangle[2]); vmax.Max(quadrangle[2]);
	vmin.Min(quadrangle[3]); vmax.Max(quadrangle[3]);

	if (c->logic->GetHP()<=0.0f) return false;
	if (c->ragdoll) return false;

	//Параметры персонажа
	const Vector & pos = c->physics->GetPos();
	float r = c->physics->GetRadius();
	//Предварительный тест на пересечение ящиков
	if(vmin.x >= pos.x + r) return false;
	if(vmax.x <= pos.x - r) return false;
	if(vmin.z >= pos.z + r) return false;
	if(vmax.z <= pos.z - r) return false;
	if(vmax.y <= pos.y) return false;
	float h = c->physics->GetHeight();
	if(vmin.y >= pos.y + h) return false;
	//Ящики пересекаются, надо уточнить тест
	Vector scale;
	scale.x = scale.z = 1.0f/r;
	scale.y = 2.0f/h;
	Vector locPos(pos.x, pos.y + h*0.5f, pos.z);
	Vector loc[4];
	loc[0] = (quadrangle[0] - locPos)*scale;
	loc[1] = (quadrangle[1] - locPos)*scale;
	loc[2] = (quadrangle[2] - locPos)*scale;
	loc[3] = (quadrangle[3] - locPos)*scale;
	//Имеем прямоугольник в системе единичной сферы расположенной в 0
	Plane plane;
	plane.N = (loc[1] - loc[0]) ^ (loc[2] - loc[0]);
	if(plane.N.Normalize() > 1e-20f)
	{
		//Надо анализировать четырёх(тре)угольник
		plane.Move(loc[0]);
		float r2 = plane.D*plane.D;
		if(r2 >= 1.0f) return false;
		//Радиус окружности на плоскости
		r = sqrtf(1.0f - r2);
		//Центр окружности
		Vector cent = plane.N*plane.D;
		Plane pln;
		for(long n = 0; n < 4; n++)
		{
			//Строим плоскость, ортоганальную плоскости четырёхугольника и прохлдящую через грань
			Vector v = loc[n] - loc[n - 1 >= 0 ? n - 1 : 3];
			if(v.Normalize() < 1e-20) return false;
			pln.N = v ^ plane.N;
			pln.Move(loc[n]);
			//Проверяем дистанцию до центра круга
			float d = pln.Dist(cent);
			if(d > r) return false;
		}
		if(n < 4) return false;
	}
	else
	{
		//Надо анализировать отрезок
		Sphere sph;
		sph.pos = 0.0f;
		sph.r = 1.0f;
		if(!sph.Intersection(loc[0], loc[1])) return false;
	}

	return true;
}

//Расчёт повреждений от атак
inline bool CharacterItems::AttackProcessForBlade(array<Character *> & characters, Vector quadrangle[4])
{
	bool is_hit = false;

	//Построем описывающий бокс
	Vector vmin = quadrangle[0], vmax = quadrangle[0];
	vmin.Min(quadrangle[1]); vmax.Max(quadrangle[1]);
	vmin.Min(quadrangle[2]); vmax.Max(quadrangle[2]);
	vmin.Min(quadrangle[3]); vmax.Max(quadrangle[3]);
#ifndef STOP_DEBUG
	if (isCharacterSwordDebug)
	{
		SwordDebug & swd = swDebug[swDebug.Add()];
		swd.quadrangle[0] = quadrangle[0];
		swd.quadrangle[1] = quadrangle[1];
		swd.quadrangle[2] = quadrangle[2];
		swd.quadrangle[3] = quadrangle[3];
		swd.isBase = false;
	}
#endif
	//Атакуем персонажей
	for(long i = 0; i < characters; i++)
	{
		//Персонаж
		Character * c = characters[i];
		//Пропустим себя
		if(c == &chr) continue;
		
		//if(!chr.logic->IsEnemy(c)) continue;
		//if (c->logic->IsActor()) continue;

		if (!c->physics->IsActive()) continue;		

		if (!CheckHit(c, quadrangle)) continue;
		
		bool attacked = chr.logic->Attack(c, DamageReceiver::ds_sword);
		
		//Попали
		is_hit = is_hit | attacked;


		if (attacked && chr.IsPlayer() && c->logic->offenderEffectTable)
		{			
			Vector vx = c->physics->GetPos() - (quadrangle[1] + quadrangle[0]) * 0.5f;
			vx.y = 0.0f;
			vx.Normalize();
			
			{
				PhysTriangleMaterialID mtl = pmtlid_other3;

				if (c->items->IfArmorMtrlPresent())
				{
					mtl = pmtlid_other2;
				}

				Vector pos = (quadrangle[1] + quadrangle[0]) * 0.5f;
				pos.x = c->physics->GetPos().x;
				pos.z = c->physics->GetPos().z;

				c->logic->offenderEffectTable->MakeEffect(mtl,c,pos,vx);
			}
		} 
	}
	


	//Атакуем демедж детекторы
	dword count = chr.QTFindObjects(MG_DAMAGEACCEPTOR, vmin, vmax);
	for(dword i = 0; i < count; i++)
	{
		DamageObject& mo = (DamageObject&)chr.QTGetObject(i)->GetMissionObject();
			
		if (chr.arbiter->IsActiveCharacter(&mo)) continue;
		chr.logic->Attack(&mo, quadrangle);
	}

	return is_hit;
}

void CharacterItems::DropItems()
{	
	Matrix mtx(true);

	for(long i = 0; i < items; i++)
	{		
		ItemData & item = items[i];
		
		if (item.weapon_type == wp_armor || item.weapon_type == wp_bomb) continue;

		if (!item.visible) continue;

		//FIX ME!!!!
		item.visible = false;

		if (!chr.arbiter->AllowDropItem(false)) continue;

		Matrix mtx;

		//Мировая позиция
		if(!GetLocatorMatrix(item.locator, mtx)) continue;
		
		Matrix world(mtx, lastDrawToWorld);

#ifndef STOP_DEBUG
		bool goodWorld = PhysRigidBodyActorCheckMtx(lastDrawToWorld);
		bool goodMtx = PhysRigidBodyActorCheckMtx(mtx);
		if (!goodMtx)
		{
			api->Trace("ERROR: BAD MATRIX FROM GetLocatorMatrix");
			api->Trace("Item = %s, model = %s, locator = %d", item.id, item.model_name, item.locator);
			api->Trace("vx = (%f, %f, %f; %f); len = %f", mtx.vx.x, mtx.vx.y, mtx.vx.z, mtx.wx, mtx.vx.GetLength());
			api->Trace("vy = (%f, %f, %f; %f); len = %f", mtx.vy.x, mtx.vy.y, mtx.vy.z, mtx.wy, mtx.vy.GetLength());
			api->Trace("vz = (%f, %f, %f; %f); len = %f", mtx.vz.x, mtx.vz.y, mtx.vz.z, mtx.wz, mtx.vz.GetLength());
			api->Trace("pos = (%f, %f, %f; %f)", mtx.pos.x, mtx.pos.y, mtx.pos.z, mtx.w);
		}
		if (!goodWorld)
		{
			api->Trace("ERROR: BAD MATRIX FROM lastDrawToWorld");
			api->Trace("vx = (%f, %f, %f; %f); len = %f", lastDrawToWorld.vx.x, lastDrawToWorld.vx.y, lastDrawToWorld.vx.z, lastDrawToWorld.wx, lastDrawToWorld.vx.GetLength());
			api->Trace("vy = (%f, %f, %f; %f); len = %f", lastDrawToWorld.vy.x, lastDrawToWorld.vy.y, lastDrawToWorld.vy.z, lastDrawToWorld.wy, lastDrawToWorld.vy.GetLength());
			api->Trace("vz = (%f, %f, %f; %f); len = %f", lastDrawToWorld.vz.x, lastDrawToWorld.vz.y, lastDrawToWorld.vz.z, lastDrawToWorld.wz, lastDrawToWorld.vz.GetLength());
			api->Trace("pos = (%f, %f, %f; %f)", lastDrawToWorld.pos.x, lastDrawToWorld.pos.y, lastDrawToWorld.pos.z, lastDrawToWorld.w);
		}
#endif
		
		bool can_take = true;

		Character* player = chr.arbiter->GetPlayer();

		if (player)
		{
			can_take = player->controller->CanTakeWeapon();			
		}

		chr.arbiter->DropeItem(&item,world, (item.weapon_type == wp_undropable) || (RRnd(0.0001f, 99.99f) > item.fProbality) || !can_take);
	}
}

bool CharacterItems::HideItem(const ConstString & item_id)
{
	for(long i = 0; i < items; i++)
	{		
		ItemData & item = items[i];
		
		if (item.id != item_id) continue;

		item.visible = false;

		return true;
	}

	return false;
}

bool CharacterItems::DropItem(const ConstString & item_id, bool isBroken)
{
	Matrix mtx(true);

	for(long i = 0; i < items; i++)
	{		
		ItemData & item = items[i];
		
		if (item.id != item_id) continue;

		if (!item.visible) continue;
		
		//FIX ME!!!!
		item.visible = false;

		if (item.weapon_type == wp_armor || item.weapon_type == wp_bomb)
		{
			if (!chr.arbiter->AllowDropItem(true)) return false;
		}
		else
		{
			if (!chr.arbiter->AllowDropItem(false)) return false;
		}
						
		Matrix mtx;

		//Мировая позиция
		if(!GetLocatorMatrix(item.locator, mtx)) return false;

#ifndef STOP_DEBUG
		bool goodWorld = PhysRigidBodyActorCheckMtx(lastDrawToWorld);
		bool goodMtx = PhysRigidBodyActorCheckMtx(mtx);
		if (!goodMtx)
		{
			api->Trace("ERROR: BAD MATRIX FROM GetLocatorMatrix");
			api->Trace("Item = %s, model = %s, locator = %d", item.id, item.model_name, item.locator);
			api->Trace("vx = (%f, %f, %f; %f); len = %f", mtx.vx.x, mtx.vx.y, mtx.vx.z, mtx.wx, mtx.vx.GetLength());
			api->Trace("vy = (%f, %f, %f; %f); len = %f", mtx.vy.x, mtx.vy.y, mtx.vy.z, mtx.wy, mtx.vy.GetLength());
			api->Trace("vz = (%f, %f, %f; %f); len = %f", mtx.vz.x, mtx.vz.y, mtx.vz.z, mtx.wz, mtx.vz.GetLength());
			api->Trace("pos = (%f, %f, %f; %f)", mtx.pos.x, mtx.pos.y, mtx.pos.z, mtx.w);
		}
		if (!goodWorld)
		{
			api->Trace("ERROR: BAD MATRIX FROM lastDrawToWorld");
			api->Trace("vx = (%f, %f, %f; %f); len = %f", lastDrawToWorld.vx.x, lastDrawToWorld.vx.y, lastDrawToWorld.vx.z, lastDrawToWorld.wx, lastDrawToWorld.vx.GetLength());
			api->Trace("vy = (%f, %f, %f; %f); len = %f", lastDrawToWorld.vy.x, lastDrawToWorld.vy.y, lastDrawToWorld.vy.z, lastDrawToWorld.wy, lastDrawToWorld.vy.GetLength());
			api->Trace("vz = (%f, %f, %f; %f); len = %f", lastDrawToWorld.vz.x, lastDrawToWorld.vz.y, lastDrawToWorld.vz.z, lastDrawToWorld.wz, lastDrawToWorld.vz.GetLength());
			api->Trace("pos = (%f, %f, %f; %f)", lastDrawToWorld.pos.x, lastDrawToWorld.pos.y, lastDrawToWorld.pos.z, lastDrawToWorld.w);
		}
#endif

		Matrix world(mtx, lastDrawToWorld);

		chr.arbiter->DropeItem(&item,world, isBroken);

		return true;
	}

	return false;
}

void CharacterItems::DropArmor(bool drop_all)
{
	Matrix mtx(true);
	
	int priority = -1;
	int item_to_drop = -1;
	
	for (int index = 0; index<items; index++)
	{		
		if (!chr.arbiter->AllowDropItem(true)) return;

		ItemData & item = items[index];		

		if (!item.visible || (item.weapon_type != wp_armor && item.weapon_type != wp_bomb) ||
			!GetLocatorMatrix(item.locator, mtx))
		{
			continue;
		}				
		
		if (drop_all)
		{
			Matrix world(mtx, lastDrawToWorld);
			chr.arbiter->DropeItem(&item,world,false);
		}
		else
		{						
			if (item_to_drop == -1 ||				
				priority < item.drop_priority ||
				(priority ==  item.drop_priority && Rnd()>0.5))
			{
				priority = item.drop_priority;
				item_to_drop = index;				
			}

		}
	}

	if (item_to_drop >= 0)
	{		
		ItemData & item = items[item_to_drop];

		if (GetLocatorMatrix(item.locator, mtx))
		{
			Matrix world(mtx, lastDrawToWorld);
			chr.arbiter->DropeItem(&item,world,false);
		}
	}
}

void CharacterItems::DropArmor(float dmg, float dmg2)
{
	if (dmg<armor_hp_drop) return;
	if (Rnd()>armor_drop_prop) return;
	if (!chr.arbiter->AllowDropItem(true)) return;

	Matrix mtx(true);

	int count = 0;
	int k = (int)(Rnd() * (items.Size()-1));

	int priority = -1;
	int index = -1;

	while (count<(int)items.Size())
	{		
		ItemData & item = items[k];		
		
		if (!item.visible || (item.weapon_type != wp_armor && item.weapon_type != wp_bomb))
		{			
		}
		else
		{
			if (item.drop_priority>priority)
			{
				priority = item.drop_priority;
				index = k;
			}			
		}

		count++;

		k++;
		if (k>=(int)items.Size()) k = 0;
	}

	if (index!=-1)
	{
		ItemData & item = items[index];
		
		if (GetLocatorMatrix(item.locator, mtx)) 
		{
			Matrix world(mtx, lastDrawToWorld);
			chr.arbiter->DropeItem(&item,world,false);
		}
	}			
}

void CharacterItems::ShowRandomArmor(const ConstString & id)
{
	int count = 0;
	int k = (int)(Rnd() * (items.Size()-1));

	while (count<(int)items.Size())
	{		
		ItemData & item = items[k];		

		if (item.id == id)
		{
			item.visible = true;
			return;
		}

		count++;

		k++;
		if (k>=(int)items.Size()) k = 0;		
	}
}

//Дополнительное обновление следа от оружия (коллижен и визуализация)
void CharacterItems::AnimationFrameUpdate()
{
	static Matrix mtx;
	for(long i = 0; i < items; i++)
	{
		ItemData & item = items[i];
		//Обновим позицию клинка
		if(item.isActive)
		{
			//Получим позицию локатора
			if(!GetLocatorMatrix(item.locator, mtx, true)) continue;
			Matrix world(mtx, lastDrawToWorld);
			//Получаем позицию в мире
			BladeLine & line = item.collision[item.collision.Add()];
			line.s = world*item.bladeStart;
			line.e = world*item.bladeEnd;
#ifndef STOP_DEBUG
	if (isCharacterSwordDebug)
		swlDebug.Add(line);
#endif
		}
		if(item.trail)
		{
			bool isLostPosition = !GetLocatorMatrix(item.locator, mtx);
			item.trail->Update(mtx, -1.0f, isLostPosition);
		}
	}
}

float CharacterItems::CalcDamage(float dmg)
{
	float absrdeb_damage = 0.0f;
	
	for(int i = 0; i < items; i++)
	{
		ItemData & item = items[i];

		if (item.weapon_type == wp_armor && item.visible)
		{
			absrdeb_damage += item.armor_damage_absorb;
		}
	}
	
	absrdeb_damage = coremin(absrdeb_damage,max_armor_damage_absorb);

	if (absrdeb_damage>1.0f) absrdeb_damage = 1.0f;

	return dmg * (1.0f - absrdeb_damage);
}

void CharacterItems::SetAlwaysArmor(bool always_armor)
{
	isAlwaysArmor = always_armor;
}

bool CharacterItems::IfArmorMtrlPresent()
{
	if (isAlwaysArmor) return true;

	for(int i = 0; i < items; i++)
	{
		ItemData & item = items[i];

		if (item.armor_mtl && item.visible)
		{
			return true;
		}
	}

	return false;
}

bool CharacterItems::IfAttackWeapon()
{
	for(long i = 0; i < items; i++)
	{		
		ItemData & item = items[i];
		
		if (!item.visible) continue;
		if (item.isActive) return true;
	}

	return false;
}