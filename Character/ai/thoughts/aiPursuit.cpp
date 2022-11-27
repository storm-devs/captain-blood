#include "aiPursuit.h"
#include "../Brains/aiBrain.h"
#include "../../character/components/CharacterPhysics.h"
#include "../../character/components/CharacterLogic.h"
#include "../../../Physics/PhysicsService.h"
#include "../../../common_h/IMissionPhysObject.h"
#include <cmath>

aiThoughtPursuit::aiThoughtPursuit(aiBrain* Brain) : 
	aiThought (Brain)
{	
	pFinder = NULL;
	pPath = null;
	
	check_trg_dist = true;

	crt_strcpy(act_link,sizeof(act_link),"run");
}

void aiThoughtPursuit::Activate(const Vector& destination, const ConstString & WayPointsName, float fMinDistance, bool chck_trg_dist, float skidDistance)
{
	RELEASE(pPath);
	fDist = fMinDistance;
	
	MOSafePointerTypeEx<IPathFinder> mo;
	static const ConstString strTypeId("PathFinder");
	mo.FindObject(&pBrain->GetBody()->Mission(),WayPointsName,strTypeId);

	pFinder = mo.Ptr();	

	TaskState = state_need_go;

	vDest = destination;

	AttractionPointIndex = -1;

	vDestination = 0.0f;
	vDestination1 = 0.0f;
	vDestination2 = 0.0f;

	// переменные для рассчета пробуксовки
	minSkidDistance = (skidDistance >= 0.0f) ? skidDistance : 1.2f;
	skidDistance2 = 0.0f;
	vSkidDistance2 = 0.0f;
	skidTime = 0.0f;
	tryCount = 0;

	check_trg_dist = chck_trg_dist;

	GetBrain()->GetBody()->animation->ActivateLink("idle", true);

	aiThought::Activate();	
}

void aiThoughtPursuit::SetActLink(const char* act)
{
	crt_strcpy(act_link,sizeof(act_link),act);
}

void aiThoughtPursuit::ChangeParams (const Vector& destination, float fMinDistance)
{	
	if (bPushMode) return;

	if (~(destination - vDest) < Sqr(0.4f)) return;

	//if (TaskState!= state_move)
	{
		TaskState = state_need_go;

		AttractionPointIndex = -1;		
	}	

	vDest = destination;	
	fDist = fMinDistance;

	vDestination = 0.0f;
	vDestination1 = 0.0f;
	vDestination2 = 0.0f;

	// переменные для рассчета пробуксовки
	skidDistance2 = 0.0f;
	vSkidDistance2 = 0.0f;
	skidTime = 0.0f;
	tryCount = 0;
}

aiThoughtPursuit::~aiThoughtPursuit()
{
	RELEASE(pPath);
}

//const char * _chr_id;

bool aiThoughtPursuit::Process (float fDeltaTime)
{
	//ProfileTimer timer;
	//timer.AddToCounter("Char Move");	

	//_chr_id = GetBrain()->GetBody()->GetObjectID();

	if (!pFinder)
	{
		GetBrain()->GetBody()->LogicDebug("%s: Can't find way_point system !!!",GetBrain()->GetBody()->GetObjectID().c_str());
		return false;
	}	

	//Никуда ходить не надо...
	if (TaskState == state_stoped)
	{
		Stop();
		return false;
	}



	Vector vCurPos = GetBrain()->GetBody()->physics->GetPos();

	if (~(vDest - vCurPos) < Sqr(fDist))
	{
		//Близко от финальной точки...
		TaskState = state_stoped;

		return false;
	}

	if (check_trg_dist)
	{
		Character * target = GetBrain()->GetChrTarget();
		if (target && fabs(target->physics->GetPos().y - vCurPos.y) < 0.6f)
		{	
			if ( (target->physics->GetPos() - vCurPos).GetLength2() < Sqr(Max(fDist , 1.75f) * 0.95f) )
			{
				//Близко от финальной точки...
				TaskState = state_stoped;

				return false;
			}
		}
	}
	

	fTimeToCheckObstancles -= fDeltaTime;
	
	//Сглаживаем его...
	/*
	if (TaskState == state_path_created)
	{
	MakeSmoothPath(astar_path, path);
	TaskState = state_move;

	AttractionPointIndex = 1;
	return true;
	}
	*/

	if (bPushMode)
	{
		fPushTime -= fDeltaTime;

		if (fPushTime<0.0f)
		{
			fPushTime = 0.0f;
			bPushMode = false;
		}

		/*GetBrain()->GetBody()->arbiter->Find(*GetBrain()->GetBody(), 0.2f, 2.0f, false, -PI*0.5f, PI*0.5f, 0);

		if(GetBrain()->GetBody()->arbiter->find > 0)
		{			
			array<CharacterFind> & find = GetBrain()->GetBody()->arbiter->find;

			for(long i = 0; i < find; i++)
				if (find[i].chr->controller->AllowToBeMoved())
				{			
					Vector vPos1 = find[i].chr->physics->GetPos();

					Vector vPos2 = GetBrain()->GetBody()->physics->GetPos();

					Vector vDir = vPos1 - vPos2;

					float Dist = vDir.Normalize();
					vDir*=fDeltaTime*3.5f*(2.0f-Dist);

					find[i].chr->physics->Move(vDir);				
				}
		}*/
	}
	
	{
 		float dist = (vLastPos - GetBrain()->GetBody()->physics->GetPos()).GetLength()/fDeltaTime;
		
		if (dist<0.4f && GetBrain()->GetBody()->logic->GetState()==CharacterLogic::state_run)
		{
			fStuckTime += fDeltaTime;

			if (fStuckTime>1.1f)
			{
				fStuckTime = 0.0f;

				bPushMode = true;
				fPushTime = 3.0f;

				GetBrain()->SectorSearch(GetBrain()->GetBody()->physics->GetPos(),
										 GetBrain()->GetBody()->physics->GetAy(), 0.5f, 2.0f, -PI/3.0f, PI/3.0f,MG_CHARACTER);

				Character* pCharacter=NULL;

				//if (GetBrain()->SectorSearchResult.IsEmpty())
				{
					TaskState = state_need_go;

					AttractionPointIndex = -1;		
					
					Vector delta = Vector(0,0.0f,1.0f).Rotate((PI/180.0f * (RRnd(-90.0f,90.0f)+180)+GetBrain()->GetBody()->physics->GetAy())) * RRnd(3.5f,5.5f);
									
					vDest = GetBrain()->GetBody()->physics->GetPos()+delta;//-Vector(0.0f, 0.8f, 0.0f);	

					vDestination = 0.0f;
					vDestination1 = 0.0f;
					vDestination2 = 0.0f;
				}

				{				
					for (dword i=0;i<GetBrain()->searchResult.Size();i++)
					{							
						pCharacter = GetBrain()->searchResult[i].chr;

						if (pCharacter == GetBrain()->GetBody()) continue;

						if (pCharacter->controller)
						{
							Vector delta = Vector(0,0.0f,1.0f).Rotate(PI/180.0f * Rnd(360.0f)) * RRnd(1.5f,2.5f);
							
							pCharacter->controller->MoveAway(pCharacter->physics->GetPos()+delta);
						}
					}
				}
			}
		}
		else
		{
			fStuckTime = 0.0f;
		}
 
		vLastPos = GetBrain()->GetBody()->physics->GetPos();
	}



	//Находим путь по системе waypoints...
	if (TaskState == state_need_go)
	{
		RELEASE(pPath);
		CharacterLogic::Side side = GetBrain()->GetBody()->logic->GetSide();
		bool ally = (side == CharacterLogic::s_ally) || (side == CharacterLogic::s_ally_frfire);
		pPath = pFinder->GetPath(GetBrain()->GetBody()->physics->GetPos(), vDest, Vector(0.0f, 0.5f, 0.0f), phys_mask(phys_character), ally);
		TaskState = state_move;
		//fTimeToCheckObstancles = 2.0f;
	}

	// если путь стартовал(IsStarted срабатывает только 1 раз)
	if (pPath && pPath->IsStarted())
	{
		// делаем первые шаги - тут же идет первая проверка на проходимость части пути
		Move(0.0f);

		// если путь не проходим - то пробуем подойти к точке чуть поодаль от конечной
		if (!pPath || pPath->IsImpassable()) 
		{
			if (tryCount < 4)
			{
				tryCount++;

				RELEASE(pPath);
				TaskState = state_need_go;
				AttractionPointIndex = -1;		
				Vector dir = !(vDest - vCurPos);
				vDest -= (0.7f * dir);
				return true;
			}			

			return false;
		}

		// переменные для рассчета пробуксовки
		oldCurPos = GetBrain()->GetBody()->physics->GetPos();
		skidDistance2 = 0.0f;
		vSkidDistance2 = 0.0f;
		skidTime = 0.0f;
	}

	// надо активировать на каждом кадре, иначе анимацию могут перебить на другую
	if (pPath && pPath->IsReady())
	{
		if (GetBrain()->GetBody()->logic->GetState() != CharacterLogic::state_run)
		{
			if (!GetBrain()->GetBody()->animation->ActivateLink(act_link, true))
			{
				GetBrain()->GetBody()->animation->ActivateLink("run", true);
			}
		}
	}

	// цикл движения по пути
	if (pPath && GetBrain()->GetBody()->logic->GetState() == CharacterLogic::state_run) Move(fDeltaTime);

	return true;
}

/*void aiThoughtPursuit::MakeSmoothPath (Path &src_path, Path& dest_path)
{
	dest_path.clear();

	if (src_path.size() <= 2)
	{
		dest_path = src_path;
		return;
	}


	dest_path.add_to_begin(src_path[0]);


	float fPathSizeToSmooth = 1.4f;


	for (dword i = 0; i < (src_path.size()-2); i++)
	{
		Vector begin = src_path[i];
		Vector center = src_path[i+1];
		Vector end = src_path[i+2];

		Vector v1 = begin - center;
		Vector v2 = end - center;

		float fAngle = (v1.GetAngleXZ(v2) * 180 / PI);;


		//Если не нужно сглаживать...
		float fAngleThreshold = 45;
		if ((fAngle > 180-fAngleThreshold) && fAngle < (180+fAngleThreshold))
		{
			dest_path.add_to_end(center);
			continue;
		}

		//Если слишком короткие отрезки тоже не сглаживаем...

		if (v1.GetLength() < fPathSizeToSmooth)
		{
			dest_path.add_to_end(center);
			continue;
		}

		if (v2.GetLength() < fPathSizeToSmooth)
		{
			dest_path.add_to_end(center);
			continue;
		}

		//Создаем первый отрезок
		Vector s1 = center;
		Vector s2 = center;
		s1.MoveByStep(begin, fPathSizeToSmooth*0.2f);
		s2.MoveByStep(end, fPathSizeToSmooth*0.2f);

		//находим его середину...
		Vector temp = (s1 + s2) / 2.0f;


		s1 = center;
		s2 = center;
		s1.MoveByStep(begin, fPathSizeToSmooth*0.4f);
		s2.MoveByStep(end, fPathSizeToSmooth*0.4f);

		Vector c1 = (s1 + temp) / 2.0f;
		Vector c2 = (s2 + temp) / 2.0f;

		s1 = center;
		s2 = center;
		s1.MoveByStep(begin, fPathSizeToSmooth*0.6f);
		s2.MoveByStep(end, fPathSizeToSmooth*0.6f);


		dest_path.add_to_end(s1);
		dest_path.add_to_end(c1);
		dest_path.add_to_end(c2);
		dest_path.add_to_end(s2);
		//Если нужно сглаживать, сглаживаем путь...
	}


	dest_path.add_to_end(src_path[src_path.last()]);

}*/


void aiThoughtPursuit::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	Vector vPos = GetBrain()->GetBody()->physics->GetPos();
	if (GetBrain()->GetChrTarget())	
	{
		Matrix mat;

		GetBrain()->GetChrTarget()->GetMatrix(mat);

		pRS->DrawLine(mat.pos, 0xff00ffff, vPos, 0xff00ffff);
	}

	/*if (path.size() > 0)
	{
		Vector vPrevious = path[0];
		for (dword i = 1; i < path.size(); i++)
		{
			pRS->DrawLine(vPrevious+Vector (0.0f, 1.0f, 0.0f), 0xFF000000, path[i]+Vector (0.0f, 1.0f, 0.0f), 0xFFFFFF00);
			vPrevious = path[i];

			if (i ==AttractionPointIndex)
			{
				pRS->DrawSphere (path[i], 0.65f, 0xFFFF0000);
			}
		}
	}*/

	if (pPath && pPath->IsReady() && !pPath->IsDone())
	{
		Vector h = Vector(0.0f, 0.5f, 0.0f);

		const Vector & v0 = pPath->Pnt0();
		const Vector & v1 = pPath->Pnt1();

		const Vector & ve = pPath->To();
		const Vector & vf = pPath->From();

		pRS->DrawLine(vPos + h, 0x4F505050, vf + h, 0xFF202020);
		pRS->DrawVector(vPos + h, v0 + h, 0xFFFFFFFF);
		pRS->DrawLine(vPos + h, 0x4F505050, v1 + h, 0xFF00FF00);
		pRS->DrawLine(vPos + h, 0xFF505050, ve + h, 0xFFFF0000);

		pRS->DrawSphere(ve + h, 0.4f, 0xFFFF0000);
	}

	Vector vCurPos = GetBrain()->GetBody()->physics->GetPos();
	GetBrain()->GetBody()->Render().Print(vCurPos + Vector(0.0f, 3.2f, 0.0f), 40.0f, 0.0f, 0xFF00FF00, "%.2f, %.2f", skidTime, sqrtf(skidDistance2));

	/*
	if (TaskState == state_move)
	{
	pRS->DrawSphere(vDestination, 0.5f);
	pRS->DrawSphere(vDestination1, 0.4f);
	pRS->DrawSphere(vDestination2, 0.4f);
	}
	*/

}

void aiThoughtPursuit::Move(float dltTime)
{
	if (!pPath || !pPath->IsReady()) return;
	
	if (pPath->IsDone())
	{
		RELEASE(pPath);
		TaskState = state_stoped;
		return;
	}

	Vector vCurPos = GetBrain()->GetBody()->physics->GetPos();

	//Тут нужно бегать от точки к точке...
	vDestination1 = pPath->Pnt0();
	vDestination2 = pPath->Pnt1();

	//Если к последней точке бежим...
	if (pPath->IsLast())
		vDestination1 = vDestination2;

	float fDistToTarget = Vector(vCurPos - vDestination1).GetLengthXZ();
	float fBlendK = 1.0f - Clampf(fDistToTarget / 0.9f);
	fBlendK *= fBlendK;

	vDestination.Lerp(vDestination1, vDestination2, fBlendK);

	Vector vZAxis = Vector (0.0f, 0.0f, 1.0f);
	Vector vLookTo = !Vector(vDestination.x - vCurPos.x, 0.0f, vDestination.z - vCurPos.z);

	//GetBrain()->GetBody()->Render().DrawVector(vCurPos, vCurPos + (1.5f * vLookTo), 0xFF00FF00);

	float fCurAngle = GetBrain()->GetBody()->physics->GetAy();
	float fNewAngle = vZAxis.GetAngleXZ(vLookTo);

	float fAngleOffset=0;

	//if (fTimeToCheckObstancles <= 0.0f)
	{
		vAntiDir = 0.0f;
		fTimeToCheckObstancles += 1.0f;

		GetBrain()->CircleSearch(GetBrain()->GetBody()->physics->GetPos(), 2.5f, MG_CHARACTER);

		Character* pCharacter=NULL;	

		for (dword i=0;i<GetBrain()->searchResult.Size();i++)
		{		
			pCharacter = (Character*)GetBrain()->searchResult[i].chr;
	
			if (pCharacter==GetBrain()->GetBody()) continue;
			if (!pCharacter->IsShow()) continue;
			if (pCharacter->logic->IsActor()) continue;
			if (pCharacter->logic->IsDead()) continue;
			// если этот чарактер - наш таргет - скипаем
			if (GetBrain()->GetChrTarget() == pCharacter) 
				continue;

			Vector dir = pCharacter->physics->GetPos() - vCurPos;
			float dist = dir.Normalize();
			float dot = dir | vLookTo;
			if (dot < 0.85) continue;

			vAntiDir += (!(vLookTo - dir));

			//GetBrain()->GetBody()->Render().DrawVector(vCurPos, vCurPos + vAntiDir, 0xFFFF0000);

			/*if (0.5f < dist && dist < 2.0f)
			{		
				float fCharAngle = vZAxis.GetAngleXZ(vOrient);

				if (fCharAngle> PI) fCharAngle -= 2 * PI;
				if (fCharAngle<-PI) fCharAngle += 2 * PI;

				if (fNewAngle> PI) fNewAngle -= 2 * PI;
				if (fNewAngle<-PI) fNewAngle += 2 * PI;

				if (fNewAngle - fCharAngle> PI) fNewAngle -= 2 * PI;

				float fDelta = fNewAngle - fCharAngle;

				if (fDelta> PI) fDelta-= 2 * PI;
				if (fDelta<-PI) fDelta+= 2 * PI;
		
				float fDist = (1-vOrient.GetLength()/2.0f)*4.5f;

				if (fabs(fDelta)< PI * 0.333f)
				{			
					fAngleOffset = SIGN(fDelta) * (PI * 0.333f-fabs(fDelta)) * fDist;
				}
			}*/
		}	

		/* Никита - дамаг ресиверы не должны мешать чарактеру - нужна другая система для обегания мелких объектов (если вообще надо это)
		dword count =  GetBrain()->GetBody()->QTFindObjects(MG_DAMAGEACCEPTOR, vCurPos - Vector(2.0f, 0.0f, 2.0f), vCurPos + Vector(2.0f, 0.0f, 2.0f));		
	
		for(dword i = 0; i < count; i++)
		{
			MissionObject& mo = (MissionObject&)GetBrain()->GetBody()->QTGetObject(i)->GetMissionObject();
			if (mo.Is("Character")) continue;
			
			Matrix mat;
			Vector min, max;

			const char* type = mo.GetObjectType();

			if (mo.Is("IMissionPhysObject"))
			{	
				IMissionPhysObject* box = (IMissionPhysObject*)&mo;
										
				box->GetPartMatrix(0,mat);
				box->GetBox(min, max);
			}
			else
			{
				mo.GetBox(min, max);
				mo.GetMatrix(mat);				
			}

			Vector h = Vector(0.0f, 0.5f, 0.0f);
			bool bTest = Box::OverlapsBoxLine(mat, (max - min) * 0.5f, vCurPos + h, vCurPos + (vLookTo * 0.85f) + h);
			//GetBrain()->GetBody()->Render().DrawBox(min, max, Matrix(), (bTest) ? 0xFF00FF00 : 0xFF00007F);
			//Vector vOrient = mat.pos - vCurPos;

			if (bTest)
				vAntiDir += Vector(vCurPos.x - mat.pos.x, 0.0f, vCurPos.z - mat.pos.z);
		}*/
	}

	if (~vAntiDir > 1e-5f)
		vLookTo = vLookTo + (!vAntiDir);
	
	fNewAngle = vZAxis.GetAngleXZ(vLookTo);

	//GetBrain()->GetBody()->Render().Print(10, 20, 0xFF00FF00, "f = %.3f", fAngleOffset);

	fAngleOffset = Min(fAngleOffset, PI / 16.0f);
	fAngleOffset = Max(fAngleOffset, -PI / 16.0f);

	Vector vCurDir = Vector(0.0f, 0.0f, 1.0f).Rotate(fNewAngle);
	///GetBrain()->GetBody()->Render().DrawVector(vCurPos, vCurPos + vCurDir, 0xFF804FFF);
	

	if (GetBrain()->GetBody()->logic->AllowOrient()) GetBrain()->GetBody()->physics->Turn(fNewAngle+fAngleOffset);

	float fNowDistanceToTarget = Vector (vDestination1 - vCurPos).GetLengthXZ();	
	
	double fDelta = (double)fLastDistanceToTarget - (double)fNowDistanceToTarget;
	float fDistToPoint2 = Vector(vCurPos - vDestination1).GetLengthXZ2();

	// анализируем буксовку на месте
	if (dltTime > 0.0001f)
	{
		vSkidDistance2 += (vCurPos - oldCurPos);
		skidTime += dltTime;

		skidDistance2 = vSkidDistance2.GetLength2();

		if (skidDistance2 > Sqr(minSkidDistance))
		{
			vSkidDistance2 = 0.0f;
			skidDistance2 = 0.0f;
			skidTime = 0.0f;
		}
		oldCurPos = vCurPos;
	}

	bool bSkidDone = (skidTime > 1.0f && skidDistance2 < Sqr(0.6f) && fDistToPoint2 < Sqr(1.0f) + skidTime * 0.6f);

	// если продолжили буксовать больше 6 секунд - то заканчиваем этот путь
	if (skidTime > 2.0f && skidDistance2 < Sqr(1.2f))
	{
		TaskState = state_stoped;
		RELEASE(pPath);
		return;
	}

	// FIX-ME ?
	bool hDiff = abs(vCurPos.y - vDestination1.y) < 1.8f;
	if (( (fDistToPoint2 < Sqr(1.0f) /*&& hDiff*/) && fDelta < 0.0f) || (fDistToPoint2 < Sqr(1.0f) && hDiff) || bSkidDone)
	{
		if (bSkidDone)
			bSkidDone = true;

		while (fDistToPoint2 < Sqr(1.0f))// && hDiff)
		{
			pPath->Next();
			if (pPath->IsDone()) break;
			
			fDistToPoint2 = Vector(vCurPos - pPath->Pnt0()).GetLengthXZ2();
			hDiff = abs(vCurPos.y - pPath->Pnt0().y) < 1.8f;
		}

		if (pPath->IsDone() || pPath->IsImpassable())
		{
			TaskState = state_stoped;
			RELEASE(pPath);
		}
		else
		{
			fLastDistanceToTarget = Vector (pPath->Pnt1() - vCurPos).GetLengthXZ();
		}
	}
	else
	{
		fLastDistanceToTarget = fNowDistanceToTarget;
	}
}

void aiThoughtPursuit::Stop ()
{
	RELEASE(pPath);

	bPushMode = false;
	fPushTime = 0.0f;

	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle",true);

	GetBrain()->OrinentToTarget();
}

int aiThoughtPursuit::GetNumPathPoints()
{
	if (!pPath)
		return 0;

	return pPath->GetNumPoints();
}
