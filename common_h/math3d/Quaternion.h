//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Quaternion
//============================================================================================

#ifndef _Quaternion_h_
#define _Quaternion_h_


#include "Vector.h"
#include "Matrix.h"

//============================================================================================

struct D3DXQUATERNION;

//============================================================================================


///Класс представления кватерниона
class Quaternion
{
public:
	union
	{
		struct
		{
			///Векторная компонента по X
			float x;
			///Векторная компонента по Y
			float y;
			///Векторная компонента по Z
			float z;
			///Скалярная компонента
			float w;
		};
		///Представление в виде массива
		float q[4];
	};

//-----------------------------------------------------------
//Конструкторы
//-----------------------------------------------------------
public:
	///Установить единичный кватернион
	Quaternion();
	///По углам поворота
	Quaternion(float ax, float ay, float az);
	///По углам поворота
	Quaternion(const Vector & ang);
	///По компонентам
	Quaternion(float qx, float qy, float qz, float qw);
	///Из матрицы
	Quaternion(const Matrix & mtx);

//-----------------------------------------------------------
//Операторы
//-----------------------------------------------------------
public:
	///Отрицание
	Quaternion & mathcall operator - ();
	///Приравнять
	Quaternion & mathcall operator = (const Quaternion & q);
	///Сложить
	Quaternion & mathcall operator += (const Quaternion & q);
	///Сложить
	Quaternion & mathcall operator += (float f);
	///Вычесть
	Quaternion & mathcall operator -= (const Quaternion & q);
	///Вычесть
	Quaternion & mathcall operator -= (float f);
	///Умножить
	Quaternion & mathcall operator *= (float f);
	///Умножить
	Quaternion & mathcall operator *= (const Quaternion & q);
	///Разделить
	Quaternion & mathcall operator /= (float f);

//-----------------------------------------------------------
//Заполнение
//-----------------------------------------------------------
public:
	///Установить единичный кватернион
	void mathcall SetIdentity();
	///Сконструировать по углам поворота
	void mathcall Set(float ax, float ay, float az);
	///Сконструировать по углам поворота
	void mathcall Set(const Vector & ang);
	///Сконструировать по компонентам
	void mathcall Set(float qx, float qy, float qz, float qw);
	///Сконструировать из матрицы
	void mathcall Set(const Matrix & mtx);


//-----------------------------------------------------------
//Преобразование
//-----------------------------------------------------------
public:
	///Нормализовать
	Quaternion & mathcall Normalize();
	///Получить компланарный кватернион
	Quaternion & mathcall Conjugate();
	///Инвертировать кватернион
	Quaternion & mathcall Inverse();

	///Сферическая линейная интерполяция
	Quaternion & mathcall SLerp(const Quaternion & from, const Quaternion & to, float kBlend);

//-----------------------------------------------------------
//Утилитные
//-----------------------------------------------------------
public:
	///Получить из кватерниона матрицу
	Matrix & mathcall GetMatrix(Matrix & mtx) const;
	///Получить длинну кватерниона
	float mathcall GetLength() const;
	///Получить квадрат длинны кватерниона
	float mathcall GetLengthSqr() const;

};


//===========================================================
//Конструкторы
//===========================================================

//Установить единичный кватернион
mathinline Quaternion::Quaternion()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 1.0f;
}

//По углам поворота
mathinline Quaternion::Quaternion(float ax, float ay, float az)
{
	Set(ax, ay, az);
}

//По углам поворота
mathinline Quaternion::Quaternion(const Vector & ang)
{
	Set(ang);
}

//По компонентам
mathinline Quaternion::Quaternion(float qx, float qy, float qz, float qw)
{
	Set(qx, qy, qz, qw);
}

//Из матрицы
mathinline Quaternion::Quaternion(const Matrix & mtx)
{
	Set(mtx);
}


//===========================================================
//Операторы
//===========================================================

//Отрицание
mathinline Quaternion & mathcall Quaternion::operator - ()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
	return *this;
}

//Приравнять
mathinline Quaternion & mathcall Quaternion::operator = (const Quaternion & q)
{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
	return *this;
}

//Сложить
mathinline Quaternion & mathcall Quaternion::operator += (const Quaternion & q)
{
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;	
	return *this;
}

//Сложить
mathinline Quaternion & mathcall Quaternion::operator += (float f)
{
	x += f;
	y += f;
	z += f;
	w += f;
	return *this;
}

//Вычесть
mathinline Quaternion & mathcall Quaternion::operator -= (const Quaternion & q)
{
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
	return *this;
}

//Вычесть
mathinline Quaternion & mathcall Quaternion::operator -= (float f)
{
	x -= f;
	y -= f;
	z -= f;
	w -= f;
	return *this;
}

//Умножить
mathinline Quaternion & mathcall Quaternion::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

//Умножить
mathinline Quaternion & mathcall Quaternion::operator *= (const Quaternion & q)
{
	float a = (w + x)*(q.w + q.x);
	float b = (z - y)*(q.y - q.z);
	float c = (x - w)*(q.y - q.z);
	float d = (y + z)*(q.x - q.w);
	float e = (x + z)*(q.x + q.y);
	float f = (x - z)*(q.x - q.y);
	float g = (w + y)*(q.w - q.z);
	float h = (w - y)*(q.w + q.z);
	x = a - ( e + f + g + h)*0.5f; 
	y =-c + ( e - f + g - h)*0.5f;
	z =-d + ( e - f - g + h)*0.5f;
	w = b + (-e - f + g + h)*0.5f;
	return *this;
}

//Разделить
mathinline Quaternion & mathcall Quaternion::operator /= (float f)
{
	double k = 1.0/f;
	x = float(x*k);
	y = float(y*k);
	z = float(z*k);
	w = float(w*k);
	return *this;
}

/*!\relates Quaternion
Сложить
*/
mathinline Quaternion operator + (float f, const Quaternion & q)
{
	Quaternion tq(q);
	tq += f;
	return tq;
}

/*!\relates Quaternion
Сложить
*/
mathinline Quaternion operator + (const Quaternion & q, float f)
{
	Quaternion tq(q);
	tq += f;
	return tq;
}

/*!\relates Quaternion
Сложить
*/
mathinline Quaternion operator + (const Quaternion & q1, const Quaternion & q2)
{
	Quaternion tq(q1);
	tq += q2;
	return tq;
}

/*!\relates Quaternion
Вычесть
*/
mathinline Quaternion operator - (float f, const Quaternion & q)
{
	Quaternion tq(q);
	tq -= f;
	return tq;
}

/*!\relates Quaternion
Вычесть
*/
mathinline Quaternion operator - (const Quaternion & q, float f)
{
	Quaternion tq(q);
	tq -= f;
	return tq;
}

/*!\relates Quaternion
Вычесть
*/
mathinline Quaternion operator - (const Quaternion & q1, const Quaternion & q2)
{
	Quaternion tq(q1);
	tq -= q2;
	return tq;
}

/*!\relates Quaternion
Умножить
*/
mathinline Quaternion operator * (float f, const Quaternion & q)
{
	Quaternion tq(q);
	tq *= f;
	return tq;
}

/*!\relates Quaternion
Умножить
*/
mathinline Quaternion operator * (const Quaternion & q, float f)
{
	Quaternion tq(q);
	tq *= f;
	return tq;
}

/*!\relates Quaternion
Умножить
*/
mathinline Quaternion operator * (const Quaternion & q1, const Quaternion & q2)
{
	Quaternion tq(q1);
	tq *= q2;
	return tq;
}

/*!\relates Quaternion
Разделить
*/
mathinline Quaternion operator / (const Quaternion & q, float f)
{
	Quaternion tq(q);
	tq /= f;
	return tq;
}

//===========================================================
//Заполнение
//===========================================================

//Установить единичный кватернион
mathinline void mathcall Quaternion::SetIdentity()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 1.0f;
}

//Сконструировать по углам поворота
mathinline void mathcall Quaternion::Set(float ax, float ay, float az)
{
	Set(Matrix(ax, ay, az));
}

//Сконструировать по углам поворота
mathinline void mathcall Quaternion::Set(const Vector & ang)
{
	Set(ang.x, ang.y, ang.z);
}

///Сконструировать по компонентам
mathinline void mathcall Quaternion::Set(float qx, float qy, float qz, float qw)
{
	x = qx;
	y = qy;
	z = qz;
	w = qw;	
}

//Сконструировать из матрицы
mathinline void mathcall Quaternion::Set(const Matrix & mtx)
{
	float tr = mtx.m[0][0] + mtx.m[1][1] + mtx.m[2][2];
	if(tr > 0.0f)
	{
		double s = sqrt(tr + 1.0f);
		w = float(s*0.5);
		s = 0.5/s;
		x = float((mtx.m[1][2] - mtx.m[2][1])*s);
		y = float((mtx.m[2][0] - mtx.m[0][2])*s);
		z = float((mtx.m[0][1] - mtx.m[1][0])*s);
	}else{
		long i, j, k;
		if(mtx.m[1][1] > mtx.m[0][0])
		{
			if(mtx.m[2][2] > mtx.m[1][1])
			{
				i = 2;
				j = 0;
				k = 1;
			}else{
				i = 1;
				j = 2;
				k = 0;
			}
		}else
		if(mtx.m[2][2] > mtx.m[0][0])
		{
			i = 2;
			j = 0;
			k = 1;
		}else{
			i = 0;
			j = 1;
			k = 2;
		}
		double s = sqrt((mtx.m[i][i] - (mtx.m[j][j] + mtx.m[k][k])) + 1.0);
		q[i] = float(s*0.5);
		if(s != 0.0) s = 0.5/s;		
		q[j] = float((mtx.m[i][j] + mtx.m[j][i])*s);
		q[k] = float((mtx.m[i][k] + mtx.m[k][i])*s);
		q[3] = float((mtx.m[j][k] - mtx.m[k][j])*s);
	}
}

//===========================================================
//Преобразование
//===========================================================

//Нормализовать
mathinline Quaternion & mathcall Quaternion::Normalize()
{
	double len = double(x)*double(x) + double(y)*double(y) + double(z)*double(z) + double(w)*double(w);
	if(len <= 0.0) return *this;
	len = 1.0/sqrt(len);
	x = float(x*len);
	y = float(y*len);
	z = float(z*len);
	w = float(w*len);
	return *this;
}

//Получить компланарный кватернион
mathinline Quaternion & mathcall Quaternion::Conjugate()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

//Инвертировать кватернион
mathinline Quaternion & mathcall Quaternion::Inverse()
{
	Conjugate();
	Normalize();
	return *this;
}

//Сферическая линейная интерполяция
mathinline Quaternion & mathcall Quaternion::SLerp(const Quaternion & from, const Quaternion & to, float kBlend)
{
	//Угол между кватернионами
	double cosomega = from.x*to.x + from.y*to.y + from.z*to.z + from.w*to.w;
	double k = 1.0f;
	//Ближайшее направление
	if(cosomega < 0.0f)
	{
		cosomega = -cosomega;
		k = -1.0f;
	}
	if(cosomega > 1.0) cosomega = 1.0;
	//Коэфициенты интерполяции
	double k0, k1;
	if(1.0 - cosomega > 0.005)
	{
		//slerp
		double omega = acos(cosomega);
		double sinomega = 1.0/sin(omega);
		k0 = sin(omega*(1.0 - kBlend))*sinomega;
		k1 = sin(omega*kBlend)*sinomega;
	}else{
		//lerp
		k0 = 1.0 - kBlend;
		k1 = kBlend;
	}
	k0 *= k;
	//Интерполируем	
	x = float(from.x*k0 + to.x*k1);
	y = float(from.y*k0 + to.y*k1);
	z = float(from.z*k0 + to.z*k1);
	w = float(from.w*k0 + to.w*k1);
	return *this;
}

//===========================================================
//Утилитные
//===========================================================

//Получить из кватерниона матрицу
mathinline Matrix & mathcall Quaternion::GetMatrix(Matrix & mtx) const
{
	float kLen = x*x + y*y + z*z + w*w;
	if(kLen < 1e-20f)
	{
		mtx.SetZero();
		return mtx;
	}
	kLen = 2.0f/kLen;
	float xx = x*x*kLen, xy = x*y*kLen, xz = x*z*kLen;
	float yy = y*y*kLen, yz = y*z*kLen;
	float zz = z*z*kLen;
	float wx = w*x*kLen, wy = w*y*kLen, wz = w*z*kLen;
	mtx.m[0][0] = 1.0f - (yy + zz);
	mtx.m[0][1] = xy + wz;
	mtx.m[0][2] = xz - wy;
	mtx.m[0][3] = 0.0f;
	mtx.m[1][0] = xy - wz;
	mtx.m[1][1] = 1.0f - (xx + zz);
	mtx.m[1][2] = yz + wx;
	mtx.m[1][3] = 0.0f;
	mtx.m[2][0] = xz + wy;
	mtx.m[2][1] = yz - wx;
	mtx.m[2][2] = 1.0f - (xx + yy);
	mtx.m[2][3] = 0.0f;
	mtx.m[3][0] = 0.0f;
	mtx.m[3][1] = 0.0f;
	mtx.m[3][2] = 0.0f;
	mtx.m[3][3] = 1.0f;
	return mtx;
}

//Получить длинну кватерниона
mathinline float mathcall Quaternion::GetLength() const
{
	return sqrtf(x*x + y*y + z*z + w*w);
}

//Получить квадрат длинны кватерниона
mathinline float mathcall Quaternion::GetLengthSqr() const
{
	return x*x + y*y + z*z + w*w;
}


#endif

