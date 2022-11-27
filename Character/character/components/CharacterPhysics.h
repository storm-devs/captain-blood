//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharacterPhysics
//===========================================================================================================================

#ifndef _CharacterPhysics_h_
#define _CharacterPhysics_h_

class Character;

class CharacterPhysics : public Object
{
public:

	CharacterPhysics();
	virtual ~CharacterPhysics();

	void SetOwner(Character* owner);
	void Release();

	//Активировать/деактивировать коллижен
	void Activate(bool active, const Vector * ragdollPos = null, bool isRagdoll = false);
	bool IsActive();

	//Обнулить состояние физической части
	void Reset();
	//Установить позицию
	void SetPos(const Vector & p);
	//Получить позицию
	const Vector GetPos(bool count_bone_offset = false);
	//Установить угол
	void SetAy(float ay);
	//Получить угол
	float GetAy();

	//Получить угол
	void SetAz(float new_az);
	//Получить угол
	float GetAz();

	float GetNewAy();

	//Установить скорость поворота (попугаи/сек)
	void SetTurnSpeed(float speed);
	//Получить скорость поворота
	float GetTurnSpeed();
	//Повернуться в заданное направление
	void Turn(float angle, bool isInstantly = false, bool ignoreState = false);
	//Сориентироваться на заданную точку
	void Orient(const Vector & by, bool isInstantly = false, bool ignoreState = false);

	//Установить радиус персонажа
	void SetRadius(float radius);
	//Получить радиус персонажа
	float GetRadius();
	//Установить высоту персонажа
	void SetHeight(float height);
	//Получить высоту
	float GetHeight();

	void ReCalcMatrices();

	void CalcMatrices();
	//Получить матрицу для модели
	Matrix & GetModelMatrix(Matrix & mtx);
	//Получить матрицу для персонажа
	Matrix & GetMatrixWithBoneOffset(Matrix & mtx);
	//Получить матрицу для персонажа
	Matrix & GetCharacterMatrix(Matrix & mtx);
	//Получить матрицу для оболочки
	Matrix & GetColliderMatrix(Matrix & mtx);	
	

	//Обновить угол
	void TurnUpdate(float dltTime);	
	//Обновить положение объектов
	void Move(float dltTime);
	void Move(const Vector & vDir,bool collide_with_characters = true);
	
	//Дать пинок персонажу с заданном направлении (2D)
	void Kick(const Vector & dir);
	//Состояния полёта в пинке
	bool IsKickState();
	//Остановить состояние полёта в пинке
	void StopKick();	

	void SetMovePosition(Vector mv_pos);
	void MoveInPosition(float dltTime);

	void SetAllowMove(bool AllowMove);

	void SetEnableGravity(bool gravity);
	bool IsGravityEnable();	

	void SetUseFakeModelPosition(bool UseFakeModelPosition);

	bool IsUseFakeModelPosition();	
	void SetFakeModelPosition(Vector v_fake_pos);	
	void GetFakeModelPosition(Vector& v_fake_pos) { v_fake_pos = fake_pos; };	
		

//--------------------------------------------------------------------------------------------
//Общие параметры персонажа
//--------------------------------------------------------------------------------------------
public:	

	Vector dltPosSpeed;
	Vector dltPos;	
	Vector vFlyDir;
	Vector vAttackerDir;

private:
	Character* chr;		    //Персонаж которым рулим	
	Vector pos;				//Позиция
	Vector move;			//Вектор перемещения на текущем кадре
	float ay;				//Угол ориентации персонажа
	float az;				//Угол наклона персонажа при повороте
	float radius;			//Текущая длина-ширина персонажа
	float height;			//Высота персонажа	
	float velY;				//Скорость падения персонажа
	float frcY;				//Коэфициент торможения вертикальной сорости	
	Vector kickDir;			//Направление пинка в 2D
	bool kickState;			//Использовать нормальное направление или пиночное	

	int curFrame;
	Vector last_frame_pos;

	IPhysCharacter * phchr;	//Физическое представление персонажа
	bool isActive;          //Флаг активности физической капсулы

	bool bAllowMove;

	bool   bNeedCalcModelMatrix;
	Matrix mModelMatrix; // прекешированная матрица модели 
	Matrix mModelBoneMatrix; // прекешированная матрица модели c учетом положения рутовой кости

	bool   bGravityOn;
	bool   bUseFakeModelPosition;
	Vector fake_pos;	
	
	float newAy;			//Новый угол к которому надо довернуть персонажа
	long  rotDirect;			//Направление поворота
	float turnSpeed;		//Скорость поворота

	float  dist_to_move;
	Vector move_dir;
	Vector move_pos;

	bool isFromRagdoll;
	Vector lastRagdollPos;

	bool CheckRagdollPosition(const Vector & checkPos);
	void InnerReset();
};

inline bool CharacterPhysics::IsActive()
{
	return isActive;
}

//Получить угол
inline void CharacterPhysics::SetAz(float new_az)
{
	az = new_az;
}

//Получить угол
inline float CharacterPhysics::GetAz()
{
	return az;
}

//Установить угол
inline void CharacterPhysics::SetAy(float ay)
{
	this->ay = newAy = ay;
	bNeedCalcModelMatrix = true;
}

//Получить угол
inline float CharacterPhysics::GetAy()
{
	return ay;
}

//Получить угол
inline float CharacterPhysics::GetNewAy()
{
	return newAy;
}

//Установить скорость поворота (попугаи/сек)
inline void CharacterPhysics::SetTurnSpeed(float speed)
{
	turnSpeed = speed;
}

//Получить скорость поворота
inline float CharacterPhysics::GetTurnSpeed()
{
	return turnSpeed;
}

//Получить радиус персонажа
inline float CharacterPhysics::GetRadius()
{
	return radius;
}

//Получить высоту
inline float CharacterPhysics::GetHeight()
{
	return height;
}

//Получить матрицу для модели
inline Matrix & CharacterPhysics::GetModelMatrix(Matrix & mtx)
{
	CalcMatrices();

	mtx=mModelMatrix;
		
	return mtx;
}

inline Matrix & CharacterPhysics::GetMatrixWithBoneOffset(Matrix & mtx)
{
	CalcMatrices();

	mtx=mModelBoneMatrix;

	return mtx;
}

//Получить матрицу для персонажа
inline Matrix & CharacterPhysics::GetCharacterMatrix(Matrix & mtx)
{
	Vector ps = GetPos();

	mtx.Build(0.0f, ay, 0.0f, ps.x, ps.y, ps.z);
	return mtx;
}

//Получить матрицу для оболочки
inline Matrix & CharacterPhysics::GetColliderMatrix(Matrix & mtx)
{
	Vector ps = GetPos();

	mtx.Build(0.0f, ay, 0.0f, ps.x, ps.y + height*0.5f, ps.z);
	return mtx;
}

inline void CharacterPhysics::SetAllowMove(bool AllowMove)
{
	bAllowMove = AllowMove;
};

inline void CharacterPhysics::SetEnableGravity(bool gravity)
{
	bGravityOn = gravity;
};

inline bool CharacterPhysics::IsGravityEnable()
{
	return bGravityOn;
};	

inline void CharacterPhysics::SetUseFakeModelPosition(bool UseFakeModelPosition)
{
	bUseFakeModelPosition = UseFakeModelPosition;

	if (bUseFakeModelPosition) fake_pos = pos;		

	bNeedCalcModelMatrix = true;
};

inline bool CharacterPhysics::IsUseFakeModelPosition()
{
	return bUseFakeModelPosition;
};

inline void CharacterPhysics::SetFakeModelPosition(Vector v_fake_pos)
{
	fake_pos = v_fake_pos;
};


#endif

