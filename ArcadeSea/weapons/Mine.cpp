#include "..\pch.h"
#include "mine.h"
#include "WeaponPattern.h"
#include "..\ships\Ship.h"

Matrix Mine::FindStartMatrix( const Matrix& mtx, const Vector& offset )
{
	Vector dir = mtx.MulNormal( offset );
	float dist = dir.Normalize();
	float dist2powMax = 4.f * triggerOffRadius_ * triggerOffRadius_;

	Matrix m(mtx);
	m.pos += dir * dist;

	Matrix mtxCur(true);
	for( dword j=0; j<items_.Size()*2+1; j++)
	{
		for (unsigned int i = 0; i < items_.Size(); ++i)
			if ( items_[i].active_ )
			{
				items_[i].mineActor_->GetTransform( mtxCur );
				float dist2pow = ~(mtxCur.pos - m.pos);
				if( dist2pow < dist2powMax )
				{
					m.pos += dir * triggerOffRadius_;
					break;
				}
			}
		// не нашлось мин пересекающихся с этой позицией
		if( i==items_.Size() )
			break;
	}

	return m;
}

Mine::Mine() :
	killRadius_(0.0f),
	triggerOffRadius_(0.0f),
	isOnboard_(false),
	timer_(0.0f),
	useTimer_(false),
	hostDamage_(false),
	reloadTime_(0.0f),
	items_(__FILE__, __LINE__)
{
	Activate(false);
	m_DamageDetector.Reset();
}

Mine::Mine(const Mine& other) :
Weapon(other),
items_(__FILE__, __LINE__)
{
	*this = other;
}

Mine& Mine::operator=(const Mine& other)
{
	killRadius_ = other.killRadius_;
	triggerOffRadius_ = other.triggerOffRadius_;
	isOnboard_ = other.isOnboard_;
	timer_ = other.timer_;
	useTimer_ = other.useTimer_;
	hostDamage_ = other.hostDamage_;
	minesCount_ = other.minesCount_;
	dropInterval_ = other.dropInterval_;

	for (unsigned int i = 0; i < items_.Size(); ++i) {
		RELEASE (items_[i].mineActor_);
	}
	items_.DelAll();
	items_.AddElements(minesCount_);

	reloadTime_ = other.reloadTime_;

	isOnboard_ = other.isOnboard_;
	killRadius_ = other.killRadius_;
	SetTriggerRadius(other.triggerOffRadius_);

	AttractMaxRadius_ = other.AttractMaxRadius_;
	AttractMinRadius_ = other.AttractMinRadius_;
	AttractSpeed_ = other.AttractSpeed_;

	return *this;
}

Mine::~Mine(void)
{
	for (unsigned int i = 0; i < items_.Size(); ++i) {
		RELEASE (items_[i].mineActor_);
	}
	items_.DelAll();
}


bool Mine::Fire(const WeaponTargetZone& wtz, float fDamageMultiply)
{
	if (reloadTime_ > 0.0f)
		return false;
	if ( GetOwner()==null )
		return false;

	for (unsigned int i = 0; i < items_.Size(); ++i)
		if (!items_[i].active_)
		{
			Init(items_[i]);

			Matrix mtx(true);
			if (isOnboard_)
				mtx.BuildPosition(wtz.target);
			else
				mtx = FindStartMatrix( GetOwner()->GetMatrix(Matrix(true)), GetPosition() );

			if( m_DamageDetector.Ptr() )
				items_[i].nMineDamageDetector = m_DamageDetector.Ptr()->AddMine(triggerOffRadius_,mtx.pos);
			else
				items_[i].nMineDamageDetector = -1;

			items_[i].mineActor_->SetTransform(mtx);

			// родить звук бросания мины
			if( GetShootSound() )
				GetOwner()->Sound().Create3D( GetShootSound(), mtx.pos, _FL_ );

			items_[i].mineActor_->Activate(true);
			items_[i].active_ = true;

			reloadTime_ = dropInterval_;

			return true;
		}
	return false;
}

void Mine::SetTriggerRadius(float radius)
{
	triggerOffRadius_ = radius;
}

void Mine::Init(MineItem& item)
{
	item.curTimer_ = timer_;
	if (item.mineActor_)
		return;

	item.mineActor_ = GetOwner()->Physics().CreateSphere(_FL_, triggerOffRadius_, Matrix().SetIdentity());
	if (isOnboard_)
		item.mineActor_->EnableCollision(false);
	item.mineActor_->EnableGravity(false);
	item.mineActor_->SetMass(0.5f); // FIX - 0.5f WAS MASS
	item.mineActor_->SetMotionDamping(1.0f, 10.0f);
	item.mineActor_->SetGroup(phys_nocollision);

	static const ConstString id_MineDamageDetector("MineDamageDetector");
	if( !m_DamageDetector.Ptr() )
		GetOwner()->Mission().CreateObject( m_DamageDetector.GetSPObject(), "MineDamageDetector", id_MineDamageDetector );
}

void Mine::Explode(MineItem& item)
{
	IParticleSystem* ps = GetOwner()->Particles().CreateParticleSystemEx(GetSFX(), __FILE__, __LINE__);
	if (ps)
	{
		Matrix m;
		item.mineActor_->GetTransform(m);
		ps->SetTransform(m);

		if( GetExplosionSound() && GetOwner() )
		{
			GetOwner()->Sound().Create3D( GetExplosionSound(), m.pos, _FL_ );
		}
	}

	item.mineActor_->Activate(false);
	item.active_ = false;

	if( m_DamageDetector.Ptr() )
		m_DamageDetector.Ptr()->DelMine( item.nMineDamageDetector );
	item.nMineDamageDetector = -1;
}

// 
bool Mine::CheckTrigger(MineItem& item)
{
	Assert(GetOwner() != NULL);
	
	// проверить не пора ли взорватся из-за того что время истекло
	if (useTimer_ && item.curTimer_ <= 0.0f)
		return true;

	Matrix m;
	Vector worldPos = GetPosition();
	if (isOnboard_)
		worldPos = GetOwner()->GetMatrix(m).MulVertex(worldPos);
	else
	{
		item.mineActor_->GetTransform(m);
		worldPos = m.pos;// m.MulVertex(worldPos);
	}

	unsigned int count = GetOwner()->QTFindObjects(	MG_DAMAGEACCEPTOR,
													worldPos - Vector(triggerOffRadius_),
													worldPos + Vector(triggerOffRadius_) );

	if ( count == 0 ||
		(count == 1 && (&GetOwner()->QTGetObject(0)->GetMissionObject()) == GetOwner()) )
		return false;

	for (unsigned int i = 0; i < count; ++i)
	{
		MissionObject* mo = &GetOwner()->QTGetObject(i)->GetMissionObject();
		if( CheckAcceptedObjType(mo) )
			return true;
	}

	return false;
}

void Mine::AttractToEnemy(MineItem& item)
{
	Assert(GetOwner()!=NULL && item.mineActor_!=NULL);

	Matrix m(true);
	item.mineActor_->GetTransform( m );
	Vector worldPos = m.pos;

	// отыщем все цели в указанной области
	unsigned int count = GetOwner()->QTFindObjects(	MG_DAMAGEACCEPTOR,
													worldPos - Vector(AttractMaxRadius_),
													worldPos + Vector(AttractMaxRadius_) );

	// выберем из целей ту что ближе всех
	static const float fMaxDist = 10000.f;
	Vector v;
	float fDist = fMaxDist;
	for (unsigned int i = 0; i < count; ++i)
	{
		MissionObject* mo = &GetOwner()->QTGetObject(i)->GetMissionObject();
		if( !CheckAcceptedObjType(mo) )
			continue;

		Matrix et(true);
		mo->GetMatrix(et);
		float d = (et.pos - worldPos).GetLengthXZ();
		if( d<AttractMaxRadius_ && d<fDist )
		{
			v = et.pos;
			fDist = d;
		}
	}
	if( fDist < fMaxDist && fDist > AttractMinRadius_ )
	{
		m.pos += ((Ship*)GetOwner())->GetLastDeltaTime() * AttractSpeed_ * (v - worldPos);
		item.mineActor_->SetTransform( m );
	}
}

void Mine::ApplyDamage(MineItem& item)
{
	Assert(GetOwner() != NULL);

	Matrix m;
	bool weShouldExplode = false;
	Vector worldPos = GetPosition();
	if (isOnboard_)
		worldPos = GetOwner()->GetMatrix(m).MulVertex(worldPos);
	else
	{
		item.mineActor_->GetTransform(m);
		worldPos = m.pos;
	}

	dword count = GetOwner()->QTFindObjects(MG_DAMAGEACCEPTOR,
											worldPos - Vector(killRadius_),
											worldPos + Vector(killRadius_));

	dword hq = 0;
	DamageReceiver* aHitsObj[100];

	for (dword i=0; i<count && hq<100; i++)
	{
		MissionObject * mo = &GetOwner()->QTGetObject(i)->GetMissionObject();

		if( CheckAcceptedObjType(mo) )
			aHitsObj[hq++] = (DamageReceiver*)mo;
	}

	for( i=0; i<hq; i++ )
	{
		weShouldExplode |= aHitsObj[i]->Attack( GetOwner(), DamageReceiver::ds_bomb, GetDamage(), worldPos, killRadius_);
	}

	if (weShouldExplode || item.curTimer_ <= 0.0f)
		Explode(item);
}

// логика работы мины
void Mine::Simulate(float deltaTime)
{
	if( deltaTime<=0.f ) return;
	reloadTime_ -= deltaTime;

	for (unsigned int i = 0; i < items_.Size(); ++i)
		if (items_[i].active_)
		{
			Matrix m;
			if (items_[i].mineActor_ && !isOnboard_)
			{
				// выталкиваем мину из воды в зависимости от погруженности ее в воду
				items_[i].mineActor_->GetTransform(m);
				items_[i].mineActor_->ApplyForce(Vector(0.0f, -m.pos.y*10.0f, 0.0f), 0.0f);
				AttractToEnemy( items_[i] );
				SetTransform(m); // запоминаем текущее положение
			}
			else
			if (isOnboard_ && GetOwner() && items_[i].mineActor_)
			{
				Matrix t(true);
				m = Matrix().BuildPosition(GetPosition()) * GetOwner()->GetMatrix(t);
				items_[i].mineActor_->SetTransform(m);
				SetTransform(m); // запоминаем текущее положение
			}

			bool bExplode = false;
			if( items_[i].nMineDamageDetector>=0 && m_DamageDetector.Ptr() ) {
				m_DamageDetector.Ptr()->ChangeMinePos( items_[i].nMineDamageDetector, m.pos );
				bExplode = m_DamageDetector.Ptr()->IsMineAttacked( items_[i].nMineDamageDetector );
				if( bExplode )
					items_[i].curTimer_ = 0.f; // для того что бы обязательно взрываться, даже если нет рядом целей
			}

			items_[i].curTimer_ -= deltaTime;

			// проверить не надо ли кого-нибудь взорвать
			if ( bExplode || CheckTrigger(items_[i]) )
				ApplyDamage(items_[i]);
		}
}

// отрисовка информации в редакторе
void Mine::DrawEditorFeatures()
{
	IRender& render = GetOwner()->Render();

	render.DrawSphere(GetPosition(), triggerOffRadius_, 0xFFFF0000);
	render.DrawSphere(GetPosition(), killRadius_, 0x4000FF00);
}

// отрисовка
void Mine::Draw(float deltaTime)
{
	if (!GetModel())
		return;

//	Matrix oldview = GetOwner()->Render().GetView();
//	GetOwner()->Render().SetView((GetOwner()->Mission().GetInverseSwingMatrix()*Matrix(oldview).Inverse()).Inverse());

	Matrix m(true);
	for (unsigned int i = 0; i < items_.Size(); ++i)
	{
		if (items_[i].active_)
		{
			items_[i].mineActor_->GetTransform(m);
			GetModel()->SetTransform(m);
			GetModel()->Draw();
		}
	}

//	GetOwner()->Render().SetView(oldview);
}

bool Mine::CheckAcceptedObjType(MissionObject* mo)
{
	if( !mo ) return false;
	// мертв - пропускем
	if ( mo->IsDead() || !mo->IsShow() ) return false;

	if( mo==GetOwner() )
		return hostDamage_;

	const char* pcObjType = mo->GetObjectType();
	if( !pcObjType || pcObjType[0]!='S' || pcObjType[1]!='h' || pcObjType[2]!='i' || pcObjType[3]!='p' )
		return false;

	// для корабля проверяем его отношение к нам
	if( pcObjType[4]==0 ) {
		if ( ((Ship*)GetOwner())->GetParty() == ((Ship*)mo)->GetParty() )  return false;
	}
	else if( string::NotEqual(&pcObjType[4],"DamageDetector") )
		return false;

	return true;
}





MineDamageDetector::MineDamageDetector() :
	m_mines(_FL_)
{
	m_pFinder = NULL;
	m_bUpdateFinder = false;
}

MineDamageDetector::~MineDamageDetector()
{
	RELEASE(m_pFinder);
}

bool MineDamageDetector::Create(MOPReader & reader)
{
	m_pFinder = QTCreateObject(MG_DAMAGEACCEPTOR, _FL_);
	if( m_pFinder )
	{
		m_pFinder->SetBox(-0.1f,0.1f);
		m_pFinder->Activate(true);
	}

	Activate(true);

	return true;
}

void MineDamageDetector::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	MissionObject::Show(isActive);

	if( isActive )
		SetUpdate(&MineDamageDetector::Work, ML_GEOMETRY1);
	else
		DelUpdate(&MineDamageDetector::Work);

	SetUpdate(&MineDamageDetector::DebugDraw, ML_DEBUG);
}

//Воздействовать на объект сферой
bool MineDamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius)
{
	return false;
}

//Воздействовать на объект линией
bool MineDamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to)
{
	if(	m_mines.Size()==0 ) return false;
	Vector vdir = from - to;
	float dist = vdir.Normalize();

	for( long n=0; n<m_mines; n++ )
	{
		if( !m_mines[n].bActive ) continue;

		float d;
		if( Sphere::Intersection(from,vdir,m_mines[n].pos,m_mines[n].r,&d) )
			if( d<=dist )
			{
				if( source!=DamageReceiver::ds_check )
					m_mines[n].bAttacked = true;
				m_transform.BuildPosition( m_mines[n].pos );
				return true;
			}
	}

	return false;
}

//Воздействовать на объект выпуклым чехырёхугольником
bool MineDamageDetector::Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4])
{
	return false;
}

long MineDamageDetector::AddMine(float fMineRadius, const Vector& pos)
{
	for(long n=0; n<m_mines; n++ )
		if( !m_mines[n].bActive )
			break;
	if( n==m_mines )
		m_mines.Add();

	m_mines[n].bActive = true;
	m_mines[n].bAttacked = false;
	m_mines[n].pos = pos;
	m_mines[n].r = fMineRadius;
	return n;
}

void MineDamageDetector::DelMine(long nMine)
{
	if( nMine>=0 && nMine<m_mines )
	{
		m_mines[nMine].bActive = false;
		m_bUpdateFinder = true;
	}
}

void MineDamageDetector::ChangeMinePos(long nMine, const Vector& pos)
{
	if( nMine>=0 && nMine<m_mines )
	{
		m_mines[nMine].pos = pos;
		m_bUpdateFinder = true;
	}
}

bool MineDamageDetector::IsMineAttacked(long nMine)
{
	if( nMine>=0 && nMine<m_mines )
		return m_mines[nMine].bAttacked;
	return false;
}

void _cdecl MineDamageDetector::Work(float dltTime, long level)
{
	if( m_bUpdateFinder )
	{
		if( m_pFinder )
		{
			for( long n=0; n<m_mines; n++ )
				if( m_mines[n].bActive )
					break;
			if( n<m_mines )
			{
				Vector vmin = m_mines[n].pos - m_mines[n].r;
				Vector vmax = m_mines[n].pos + m_mines[n].r;
				for( n++; n<m_mines; n++ )
				{
					if( !m_mines[n].bActive )
						continue;
					vmin.Min( m_mines[n].pos );
					vmax.Max( m_mines[n].pos );
				}
				m_pFinder->SetBox( vmin, vmax );
				if( !m_pFinder->IsActivate() )
					m_pFinder->Activate(true);
			}
			else
				m_pFinder->Activate(false);
		}
		m_bUpdateFinder = false;
	}
}

void _cdecl MineDamageDetector::DebugDraw(float dltTime, long level)
{
/*	for( long n=0; n<m_mines; n++ )
	{
		Render().DrawSphere( m_mines[n].pos, m_mines[n].r );
	}

	Render().DrawBox( m_pFinder->GetBoxCenter() - m_pFinder->GetBoxSize(), m_pFinder->GetBoxCenter() + m_pFinder->GetBoxSize(), m_pFinder->GetMatrix() );
*/
}

MOP_BEGINLIST(MineDamageDetector, "", '1.00', 100);
MOP_ENDLIST(MineDamageDetector)
