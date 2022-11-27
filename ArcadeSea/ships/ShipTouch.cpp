#include "ShipTouch.h"
#include "Ship.h"
#include "PartsClassificator.h"

ShipTouch::ShipTouch() :
	m_aTouch(_FL_),
	m_aShip(_FL_)
{
}

ShipTouch::~ShipTouch()
{
	DelUpdate(&ShipTouch::Work);
}

bool ShipTouch::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	DelUpdate(&ShipTouch::Work);
	SetUpdate(&ShipTouch::Work, ML_DYNAMIC5+30);

	return true;
}

void _cdecl ShipTouch::Work(float fDeltaTime, long level)
{
	// установим флаг отсутсвия столкновения для всех столкновений
	// ибо уже устарели для этого кадра
	for( long n=0; n<m_aTouch; n++ )
	{
		m_aTouch[n].bUse = false;
		m_aTouch[n].bIsFirstTime = false;
	}


	// пробегаем по всем парам кораблей и проверяем их на столкновение
	for( long i=0; i<m_aShip; i++ )
		for( long j=i+1; j<m_aShip; j++ )
		{
			Touch t;
			if( CheckTouch( m_aShip[i], m_aShip[j], t ) )
				AddTouch( t );
		}

	// удалим уже не действующие столкновения
	for( n=0; n<m_aTouch; n++ )
		if( !m_aTouch[n].bUse )
			m_aTouch.DelIndex(n--);

	// удалим подписанные корабли (на следущем кадре те кто участвует в столкновениях снова добавятся)
	m_aShip.DelAll();

	// отрисовка дебажной инфы
	//DebugDraw();
}

bool ShipTouch::CheckShip(IShip* pShip, TouchData& td)
{
	// подпишем корабль на столкновение
	dword n = m_aShip.Find(pShip);
	if( n == INVALID_ARRAY_INDEX )
		m_aShip.Add(pShip);

	// отыщем есть ли в зарегестрированных столкновениях этот корабль?
	for( long i=0; i<m_aTouch; i++ )
		if( m_aTouch[i].pShip1 == pShip )
		{
			td.part = m_aTouch[i].pPart1;
			td.shape = m_aTouch[i].nShape1;
			td.knock = m_aTouch[i].bIsFirstTime;
			td.fDamage = m_params.fDamage;
			return true;
		}
		else if( m_aTouch[i].pShip2 == pShip )
		{
			td.part = m_aTouch[i].pPart2;
			td.shape = m_aTouch[i].nShape2;
			td.knock = m_aTouch[i].bIsFirstTime;
			td.fDamage = m_params.fDamage;
			return true;
		}

	// корабль ни с кем не сталкивался
	return false;
}

void ShipTouch::ReadMOPs(MOPReader & reader)
{
	m_params.fDamage = reader.Float();
	m_params.fDirectFactor = reader.Float();
}

bool ShipTouch::CheckTouch(IShip* pShip1, IShip* pShip2, Touch& touch)
{
	if( !pShip1 || !pShip2 )
		return false;
	if( !((Ship*)pShip1)->GetPhysView() || !((Ship*)pShip2)->GetPhysView() )
		return false;

	Matrix m1,m2;
	Vector vMin1, vMax1, vMin2, vMax2;

	pShip1->GetMatrix( m1 );
	pShip2->GetMatrix( m2 );

	pShip1->GetBox( vMin1, vMax1 );
	pShip2->GetBox( vMin2, vMax2 );

	// сравнение на дистанцию между кораблями
	if( (m1.pos-m2.pos).GetLength() > sqrtf(vMax1.x*vMax1.x + vMax1.z*vMax1.z) + sqrtf(vMax2.x*vMax2.x + vMax2.z*vMax2.z) )
		return false;

	// получим спсики частей для обоих кораблей
	PartsClassificator cls1;
	cls1.Visit( *(Ship*)pShip1 );
	PartsClassificator cls2;
	cls2.Visit( *(Ship*)pShip2 );

	// предполагаемые позиции кораблей через малый промежуток времени
	const float fPredicateTime = 0.02f;
	Matrix mPred1;
	mPred1.Build( 0.f, ((Ship*)pShip1)->GetCurAngleSpeed() * fPredicateTime, 0.f );
	mPred1 *= m1;
	mPred1.pos += ((Ship*)pShip1)->GetPhysView()->GetLinearVelocity() * fPredicateTime;
	//
	Matrix mPred2;
	mPred2.Build( 0.f, ((Ship*)pShip2)->GetCurAngleSpeed() * fPredicateTime, 0.f );
	mPred2 *= m2;
	mPred2.pos += ((Ship*)pShip2)->GetPhysView()->GetLinearVelocity() * fPredicateTime;

	// бежим по частям корпуса корабля и проверяем их на колижн друг с другом
	for( long n1=0; n1<cls1.GetHullParts(); n1++ )
	{
		for( long ns1=0; ns1<cls1.GetHullParts()[n1]->GetShapes(); ns1++ )
		{
			long nShape1 = cls1.GetHullParts()[n1]->GetShapes()[ns1];
			Matrix mloc1;
			((Ship*)pShip1)->GetPhysView()->GetLocalTransform( nShape1, mloc1 );
			Matrix mtx1 = mloc1 * m1;
			Vector vsize1;
			((Ship*)pShip1)->GetPhysView()->GetBox( nShape1, vsize1 );

			for( long n2=0; n2<cls2.GetHullParts(); n2++ )
			{
				for( long ns2=0; ns2<cls2.GetHullParts()[n2]->GetShapes(); ns2++ )
				{
					long nShape2 = cls2.GetHullParts()[n2]->GetShapes()[ns2];
					Matrix mloc2;
					((Ship*)pShip2)->GetPhysView()->GetLocalTransform( nShape2, mloc2 );
					Matrix mtx2 = mloc2 * mPred2;
					Vector vsize2;
					((Ship*)pShip2)->GetPhysView()->GetBox( nShape2, vsize2 );

					Vector v[4];
					v[0] = mtx2.MulVertex( Vector(vsize2.x, 0.f, vsize2.z) );
					v[1] = mtx2.MulVertex( Vector(-vsize2.x, 0.f, vsize2.z) );
					v[2] = mtx2.MulVertex( Vector(vsize2.x, 0.f, -vsize2.z) );
					v[3] = mtx2.MulVertex( Vector(-vsize2.x, 0.f, -vsize2.z) );
					if( Box::OverlapsBoxPoly( mtx1, vsize1, v ) )
					{
						touch.pShip1 = pShip1;
						touch.pShip2 = pShip2;
						touch.pPart1 = cls1.GetHullParts()[n1];
						touch.pPart2 = cls2.GetHullParts()[n2];
						touch.nShape1 = nShape1;
						touch.nShape2 = nShape2;
						return true;
					}
				}
			}
		}
	}

	return false;
}

void ShipTouch::AddTouch(Touch& touch)
{
	for( long n=0; n<m_aTouch; n++ )
	{
		// если уже было такое столкновение, то просто обновляем его
		if( (m_aTouch[n].pShip1 == touch.pShip1 && m_aTouch[n].pShip2 == touch.pShip2) ||
			(m_aTouch[n].pShip1 == touch.pShip2 && m_aTouch[n].pShip2 == touch.pShip1) )
		{
			m_aTouch[n].bUse = true;
			if( m_aTouch[n].pShip1 == touch.pShip1 )
			{
				m_aTouch[n].pPart1 = touch.pPart1;
				m_aTouch[n].pPart2 = touch.pPart2;
				m_aTouch[n].nShape1 = touch.nShape1;
				m_aTouch[n].nShape2 = touch.nShape2;
			}
			else
			{
				m_aTouch[n].pPart1 = touch.pPart2;
				m_aTouch[n].pPart2 = touch.pPart1;
				m_aTouch[n].nShape1 = touch.nShape2;
				m_aTouch[n].nShape2 = touch.nShape1;
			}
			return;
		}
	}

	// если это только появившееся столкновение, то добавляем новое
	touch.bIsFirstTime = true;
	m_aTouch.Add(touch);
}

void ShipTouch::DebugDraw()
{
	for( long n=0; n<m_aTouch; n++ )
	{
		Vector vsize;
		Matrix m, mloc;

		m_aTouch[n].pShip1->GetMatrix(m);
		((Ship*)m_aTouch[n].pShip1)->GetPhysView()->GetBox( m_aTouch[n].nShape1, vsize );
		((Ship*)m_aTouch[n].pShip1)->GetPhysView()->GetLocalTransform( m_aTouch[n].nShape1, mloc );

		Render().DrawBox( -vsize, vsize, mloc*m, 0xFFFF0000 );

		m_aTouch[n].pShip2->GetMatrix(m);
		((Ship*)m_aTouch[n].pShip2)->GetPhysView()->GetBox( m_aTouch[n].nShape2, vsize );
		((Ship*)m_aTouch[n].pShip2)->GetPhysView()->GetLocalTransform( m_aTouch[n].nShape2, mloc );

		Render().DrawBox( -vsize, vsize, mloc*m, 0xFFFF0000 );
	}
}

MOP_BEGINLISTCG(ShipTouch, "ShipTouch", '1.00', 100, "Ship touch parameters", "Arcade Sea");
	MOP_FLOAT("Touch damage",1.f)
	MOP_FLOATEXC("Touch direction factor",0.f,0.f,1.f,"часть дамага которая получается при лобовом таране")
MOP_ENDLIST(ShipTouch)
