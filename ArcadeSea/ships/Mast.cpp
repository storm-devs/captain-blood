#include "..\pch.h"
#include ".\mast.h"
#include "..\..\Common_h\Mission.h"
#include "ship.h"
#include "BrokenSystemMaker.h"
#include "Ship.h"

//////////////////////////////////////////////////////////////////////////
// класс Mast
//////////////////////////////////////////////////////////////////////////

// конструктор по умолчанию
Mast::Mast(void) :
parts_(__FILE__, __LINE__)
{

}

// конструтор копий
Mast::Mast(const Mast& other) :
parts_(__FILE__, __LINE__)
{
	params_ = other.params_;
	RecalculateOrientBasis();
}

// конструктор по параметрам
Mast::Mast(const Mast::Params& other) :
parts_(__FILE__, __LINE__)
{
	params_ = other;
	RecalculateOrientBasis();
}

// оператор присваивания
Mast& Mast::operator=(const Mast& other)
{
	parts_.DelAll();

	params_ = other.params_;
	RecalculateOrientBasis();
	for (unsigned int i = 0; i < other.parts_.Size(); ++i)
		parts_.Add(other.parts_[i]);

	return *this;
}



// деструктор
Mast::~Mast(void)
{
	
}

void Mast::Rebuild(const Mast::Params& params)
{
	ShipPart::Rebuild();
	params_ = params;
	RecalculateOrientBasis();
}

// ломает мачту начиная с заданной части
void Mast::BreakAt(MastPart* part)
{
	unsigned int index = parts_.Find(part);
	if ( INVALID_ARRAY_INDEX != index )
	{
		if (part->GetShapes().Size() && GetShipOwner())
		{
			BrokenSystemMaker maker( *GetShipOwner()->GetPhysView(), GetShipOwner()->Physics());
			maker.SetStartNode(part);
			//maker.SetStartNode(this);

			ShipPart * p = this;
			while (p->GetParent())
				p = p->GetParent();

			p->AcceptVisitor(maker);
	
			maker.Make();

			part->SetBrokenSystem(maker.GetSystem());
		}
	}
}

void Mast::Break()
{
	BreakAt(parts_[0]);
}

void SetupOrientation(ShipPart * part, const Matrix& m)
{
	Matrix t;

	if( !part->GetShipOwner() ) return;

/*	for (unsigned int j = 0; j < part->GetShapes().Size(); ++j)
	{
		if( part->IsNotRotable() )
			t = part->GetInitialPoses()[j];
		else
			t = part->GetInitialPoses()[j]*m;
		part->GetShipOwner()->GetPhysView()->SetLocalTransform(part->GetShapes()[j], t);
	}

	for (unsigned int i = 0; i < part->GetChildren().Size(); ++i)
		SetupOrientation(part->GetChildren()[i], m);*/

	for (unsigned int i = 0; i < part->GetChildren().Size(); ++i)
	{
		for (unsigned int j = 0; j < part->GetChildren()[i]->GetShapes().Size(); ++j)
		{
			if( part->GetChildren()[i]->IsNotRotable() )
				t = part->GetChildren()[i]->GetInitialPoses()[j];
			else
				t = part->GetChildren()[i]->GetInitialPoses()[j]*m;
			part->GetShipOwner()->GetPhysView()->SetLocalTransform(part->GetChildren()[i]->GetShapes()[j],	t);
		}
		SetupOrientation(part->GetChildren()[i], m);
	}
}

void Mast::RecalculateOrientBasis()
{
	Matrix m;
	m.vz = params_.rotationAxisEnd_ - params_.rotationAxisBegin_;
	m.vz.Normalize();
	m.vy = m.vz ^ Vector(1.0f, 0.0f, 0.0f);
	m.vy.Normalize();
	m.vx = m.vy ^ m.vz;

	// get basis: x - multiplyer for sin(angle), y - multiply for cos(angle), z - constant
	for( long i=0; i<3; i++ )
		for( long j=0; j<3; j++ )
		{
			if( i!=j ) m_vOrientBasis[i][j].x = m.m[0][i]*m.m[1][j] - m.m[1][i]*m.m[0][j];
			else m_vOrientBasis[i][j].x = 0.f;
			m_vOrientBasis[i][j].y = m.m[0][i]*m.m[0][j] + m.m[1][i]*m.m[1][j];
			m_vOrientBasis[i][j].z = m.m[2][i]*m.m[2][j];
		}
}

Matrix Mast::GetRotationLocal(const Matrix& parent) const
{
	Matrix mres;
	if (!params_.isRotatable_)
		return mres;

	Vector va;
	va.x = -parent.m[0][2] * sin(PI/4); // sin ( angle )
	va.y = sqrt(1.f - va.x*va.x); // cos ( angle )
	va.z = 1.f; // + constant

	// ориентируем мачту по ветру
	mres.m[0][0] = va | m_vOrientBasis[0][0];
	mres.m[0][1] = va | m_vOrientBasis[0][1];
	mres.m[0][2] = va | m_vOrientBasis[0][2];
	mres.m[1][0] = va | m_vOrientBasis[1][0];
	mres.m[1][1] = va | m_vOrientBasis[1][1];
	mres.m[1][2] = va | m_vOrientBasis[1][2];
	mres.m[2][0] = va | m_vOrientBasis[2][0];
	mres.m[2][1] = va | m_vOrientBasis[2][1];
	mres.m[2][2] = va | m_vOrientBasis[2][2];
	return mres;
}

void Mast::DoWorkChildren(const Matrix& mtx)
{
	Matrix m = mtx;

	if (params_.isRotatable_)
	{
		m.BuildPosition(-params_.rotationAxisBegin_);

		m *= GetRotationLocal(mtx);

		m *= Matrix().BuildPosition(params_.rotationAxisBegin_);
		m *= mtx;
	}

	for (unsigned int i = 0;  i < GetChildren().Size(); ++i)
		GetChildren()[i]->Work(mtx);
}

// работа мачты
void Mast::DoWork(const Matrix& mtx)
{
	if ( params_.isRotatable_ )
	{
		Matrix m;
		m.BuildPosition(-params_.rotationAxisBegin_);

		m *= GetRotationLocal(mtx);
		
		m *= Matrix().BuildPosition(params_.rotationAxisBegin_);
		//m *= mtx;

		if (!IsBroken())
			for(long n=0; n<parts_; n++)
			{
				SetupOrientation(parts_[n], m);
			}
	}

	ShipPart::DoWork(mtx);
}


//////////////////////////////////////////////////////////////////////////
// класс MastPart
//////////////////////////////////////////////////////////////////////////

// конструктор
MastPart::MastPart(Mast& mast) :
mast_(mast)
{

}

// конструктор копий
MastPart::MastPart(const MastPart& other) :
mast_(other.mast_)
{
	params_ = other.params_;
}

// конструктор по параметрам
MastPart::MastPart(Mast& mast, const MastPart::Params& params) :
mast_(mast)
{
	params_ = params;
	params_.startHP_ = params_.hp_;
}

// оператор присваивания
MastPart& MastPart::operator=(const MastPart& other)
{
	params_ = other.params_;

	return *this;
}

// деструктор
MastPart::~MastPart(void)
{
}

void MastPart::Rebuild(const MastPart::Params& params)
{
	ShipPart::Rebuild();

	params_ = params;
	params_.startHP_ = params_.hp_;
}

// обработка попаданий
float MastPart::HandleDamage(const Vector& pos, float damage)
{
	if( !GetShipOwner() ) return 0.f;

	params_.hp_ -= damage;
	float fOwnerDamage = damage*mast_.GetShipDamageCoef();
	
	AddHitAnimation(mast_.GetHitSFX(GetShipOwner()->IsLimitedParticles(pos)), 0.75f, pos);
	AddClothBurnSphere( pos );

	if (params_.hp_ < 0.0f)
	{
		if( !IsBroken() )
			fOwnerDamage += GetShipOwner()->GetDamageFromMastBroke();
		mast_.BreakAt(this);
	}

	return fOwnerDamage;
}
