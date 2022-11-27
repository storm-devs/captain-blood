//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Vector
//============================================================================================

#ifndef _Vector_h_
#define _Vector_h_

#include <stdlib.h>
#include <math.h>


///Класс представления вектора в 3D пространстве
class Vector
{
public:
	union
	{
		struct
		{
			///Компонента по X
			float x;
			///Компонента по Y
			float y;
			///Компонента по Z
			float z;
		};
		///Представление в виде массива
		float v[3];
	};

//-----------------------------------------------------------
//Конструкторы
//-----------------------------------------------------------
public:
	///Пустой конструктор
	Vector();
	///Заполнить числом
	Vector(float f);
	///Заполнить числом
	Vector(double d);
	///Заполнить все компоненты
	Vector(float x, float y, float z);
	///Заполнить все компоненты
	Vector(const float f[3]);
	///Заполнить все компоненты
	Vector(const double d[3]);
	///Конструктор копирования
	Vector(const Vector & v);
	
//-----------------------------------------------------------
//Операторы
//-----------------------------------------------------------
public:
	///Найти квадрат длинны вектора
	float mathcall operator ~ () const;
	///Вернуть нормализованный вектор
	Vector mathcall operator ! () const;

	///Получить отрицательный вектор
	Vector mathcall operator - () const;

	///Присвоить
	Vector & mathcall operator = (float f);
	///Присвоить
	Vector & mathcall operator = (double d);
	///Присвоить
	Vector & mathcall operator = (const Vector & v);
	///Покомпонентное сложение с присваиванием
	Vector & mathcall operator += (float f);
	///Покомпонентное сложение с присваиванием
	Vector & mathcall operator += (double d);
	///Покомпонентное сложение с присваиванием
	Vector & mathcall operator += (const Vector & v);
	///Покомпонентное вычитание с присваиванием
	Vector & mathcall operator -= (float f);
	///Покомпонентное вычитание с присваиванием
	Vector & mathcall operator -= (double d);
	///Покомпонентное вычитание с присваиванием
	Vector & mathcall operator -= (const Vector & v);
	///Покомпонентное умножение с присваиванием
	Vector & mathcall operator *= (float f);
	///Покомпонентное умножение с присваиванием
	Vector & mathcall operator *= (double d);
	///Покомпонентное умножение с присваиванием
	Vector & mathcall operator *= (const Vector & v);
	///Покомпонентное деление с присваиванием
	Vector & mathcall operator /= (float f);
	///Покомпонентное деление с присваиванием
	Vector & mathcall operator /= (double d);
	///Покомпонентное деление с присваиванием
	Vector & mathcall operator /= (const Vector & v);
	
	///Скалярное перемножение, результат копируется во все компоненты
	Vector & mathcall operator |= (const Vector & v);
	///Векторное перемножение
	Vector & mathcall operator ^= (const Vector & v);

//-----------------------------------------------------------
//Преобразование
//-----------------------------------------------------------
public:
	///Нормализовать вектор, и вернуть его бывшую длинну
	float mathcall Normalize();
	///Спроецировать на плоскость XZ и нормализовать
	float mathcall NormalizeXZ();

	///Ограничить длинну вектора, и вернуть текущую длинну
	float mathcall ClampLength(float clampValue);
	
	///Ограничить компоненту X диапазоном
	Vector & mathcall ClampX(float min, float max);
	///Ограничить компоненту Y диапазоном
	Vector & mathcall ClampY(float min, float max);
	///Ограничить компоненту Z диапазоном
	Vector & mathcall ClampZ(float min, float max);
	///Ограничить компоненты диапазоном
	Vector & mathcall Clamp(float min, float max);

	///Сохранить в векторе минимальные компаненты
	Vector & mathcall Min(const Vector & v);
	///Сохранить в векторе максимальные компаненты
	Vector & mathcall Max(const Vector & v);
	///Сохранить в векторе минимальные компаненты
	Vector & mathcall Min(const Vector & v1, const Vector & v2);
	///Сохранить в векторе максимальные компаненты
	Vector & mathcall Max(const Vector & v1, const Vector & v2);

	///Взять все компоненты по модулю
	Vector & mathcall Abs();

//-----------------------------------------------------------
//Утилитные
//-----------------------------------------------------------
public:
	///Установить новые значения
	Vector & mathcall Set(float x, float y, float z);
	///Установить новые значения
	Vector & mathcall Set(const Vector & v);

	///Получить угол между векторами
	float mathcall GetAngle(const Vector & v) const;
	///Получить знаковый угол между векторами в плоскости XZ
	float mathcall GetAngleXZ(const Vector & v) const;
	///Получить угол поворота вектора вокруг оси Y
	float mathcall GetAY(float defAngle = 0.0f) const;
	
	///Получить синус между 2D векторами в плоскости XZ
	float mathcall SinXZ(const Vector & v) const;
	///Получить косинус между 2D векторами в плоскости XZ
	float mathcall CosXZ(const Vector & v) const;
	///Получить векторное произведение векторов в плоскости XZ
	float mathcall CrossXZ(const Vector & v) const;

	///Получить вектор в плоскости XZ
	Vector mathcall GetXZ() const;
	///Получить вектор в плоскости XZ
	Vector mathcall GetXZ(float y) const;
	///Сформировать вектор в плоскости XZ по углу
	Vector & mathcall MakeXZ(float ay);


	///Получить длинну вектора
	float mathcall GetLength() const;
	///Получить квадрат длинны вектора
	float mathcall GetLength2() const;
	///Получить длинну вектора в 2D
	float mathcall GetLengthXZ() const;
	///Получить квадрат длинны вектора в 2D
	float mathcall GetLengthXZ2() const;
	
	///Повернуть вектор в плоскости XZ на угол
	Vector & mathcall Rotate(float angle);
	///Повернуть вектор в плоскости XZ на угол заданный cos, sin
	Vector & mathcall Rotate(float vcos, float vsin);
	///Повернуть вектор в плоскости XY на угол заданный cos, sin
	Vector & mathcall RotateXY(float vcos, float vsin);
	///Повернуть вектор по часовой стрелке в плоскости XZ на угол PI/2
	Vector & mathcall Rotate_PI2_CW();
	///Повернуть вектор против часовой стрелке в плоскости XZ на угол PI/2
	Vector & mathcall Rotate_PI2_CCW();

	///Расчитать линейно интерполированное значение
	Vector & mathcall Lerp(const Vector & from, const Vector & to, float kBlend);

	///Получить коэфициент затухания 1.0f - (this.Length2() - minDist)*(kLen = 1.0f/(maxDist - minDist))
	float mathcall GetAttenuation(float minDist, float kLen);
	///pow(this.GetAttenuation(), 2)
	float mathcall GetAttenuation2(float minDist, float kLen);
	///pow(this.GetAttenuation(), 3)
	float mathcall GetAttenuation3(float minDist, float kLen);
	///pow(this.GetAttenuation(), 4)
	float mathcall GetAttenuation4(float minDist, float kLen);
	///Получить коэфициент затухания 1.0f - (this.Length2D2() - minDist)*(kLen = 1.0f/(maxDist - minDist))
	float mathcall GetAttenuationXZ(float minDist, float kLen);
	///pow(this.GetAttenuation(), 2)
	float mathcall GetAttenuation2XZ(float minDist, float kLen);
	///pow(this.GetAttenuation(), 3)
	float mathcall GetAttenuation3XZ(float minDist, float kLen);
	///pow(this.GetAttenuation(), 4)
	float mathcall GetAttenuation4XZ(float minDist, float kLen);

	///Расчитать отражённый вектор
	Vector & mathcall Reflection(const Vector & normal);

	///Заполнить единичным вектором со случайным направлением
	Vector & mathcall Rand();
	///Заполнить единичным вектором со случайным направлением в XZ
	Vector & mathcall RandXZ();
	///Заполнить случайными значениями в заданном ABB
	Vector & mathcall Rand(const Vector & min, const Vector & max);
	///Заполнить случайными значениями в заданной сфере
	Vector & mathcall Rand(const Vector & pos, float radius);

	///Переместить текущий вектор к заданной точке на заданный шаг
	bool mathcall MoveByStep(const Vector & to, float step);
	///Переместить текущий вектор к заданной точке на заданный шаг в 2D
	bool mathcall MoveByStepXZ(const Vector & to, float step);

	///Точка находиться в ящике
	bool mathcall InBox(const Vector & min, const Vector & max) const;
	///Точка находиться в шаре
	bool mathcall InSphere(const Vector & pos, float rad) const;

	///Торможение, v -= v^power*k; максимально вычитается v
	Vector & mathcall Friction(float k, float power);
	///Торможение, v -= v^power*k; максимально вычитается v для компонент XZ
	Vector & mathcall FrictionXZ(float k, float power);

};


//===========================================================
//Конструкторы
//===========================================================

//Пустой конструктор
mathinline Vector::Vector(){}

//Заполнить числом
mathinline Vector::Vector(float f)
{
	x = f;
	y = f;
	z = f;
}

//Заполнить числом
mathinline Vector::Vector(double d)
{
	x = float(d);
	y = float(d);
	z = float(d);
}

//Заполнить все компоненты
mathinline Vector::Vector(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

//Заполнить все компоненты
mathinline Vector::Vector(const float f[3])
{
	x = f[0];
	y = f[1];
	z = f[2];
}

//Заполнить все компоненты
mathinline Vector::Vector(const double d[3])
{
	x = float(d[0]);
	y = float(d[1]);
	z = float(d[2]);
}

//Конструктор копирования
mathinline Vector::Vector(const Vector & v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}


//===========================================================
//Операторы
//===========================================================

//Найти квадрат длинны вектора
mathinline float mathcall Vector::operator ~ () const
{
	return x*x + y*y + z*z;
}

//Вернуть нормализованный вектор
mathinline Vector mathcall Vector::operator ! () const
{
	Vector v(*this);
	v.Normalize();
	return v;
}

mathinline Vector mathcall Vector::operator - () const
{
	Vector v(*this);
	v.x = -v.x;
	v.y = -v.y;
	v.z = -v.z;
	return v;
}

//Присвоить
mathinline Vector & mathcall Vector::operator = (float f)
{
	x = f;
	y = f;
	z = f;
	return *this;
}

//Присвоить
mathinline Vector & mathcall Vector::operator = (double d)
{
	x = float(d);
	y = float(d);
	z = float(d);
	return *this;
}

//Присвоить
mathinline Vector & mathcall Vector::operator = (const Vector & v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

//Покомпонентное сложение с присваиванием
mathinline Vector & mathcall Vector::operator += (float f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}

//Покомпонентное сложение с присваиванием
mathinline Vector & mathcall Vector::operator += (double d)
{
	x += float(d);
	y += float(d);
	z += float(d);
	return *this;
}

//Покомпонентное сложение с присваиванием
mathinline Vector & mathcall Vector::operator += (const Vector & v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

//Покомпонентное вычитание с присваиванием
mathinline Vector & mathcall Vector::operator -= (float f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}

//Покомпонентное вычитание с присваиванием
mathinline Vector & mathcall Vector::operator -= (double d)
{
	x -= float(d);
	y -= float(d);
	z -= float(d);
	return *this;
}

//Покомпонентное вычитание с присваиванием
mathinline Vector & mathcall Vector::operator -= (const Vector & v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

//Покомпонентное умножение с присваиванием
mathinline Vector & mathcall Vector::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

//Покомпонентное умножение с присваиванием
mathinline Vector & mathcall Vector::operator *= (double d)
{
	x *= float(d);
	y *= float(d);
	z *= float(d);
	return *this;
}

//Покомпонентное умножение с присваиванием
mathinline Vector & mathcall Vector::operator *= (const Vector & v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

//Покомпонентное деление с присваиванием
mathinline Vector & mathcall Vector::operator /= (float f)
{
	double d = 1.0/f;
	x = float(x*d);
	y = float(y*d);
	z = float(z*d);
	return *this;
}

//Покомпонентное деление с присваиванием
mathinline Vector & mathcall Vector::operator /= (double d)
{
	d = 1.0/d;
	x = float(x*d);
	y = float(y*d);
	z = float(z*d);
	return *this;
}

//Покомпонентное деление с присваиванием
mathinline Vector & mathcall Vector::operator /= (const Vector & v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}


//Скалярное перемножение, результат копируется во все компоненты
mathinline Vector & mathcall Vector::operator |= (const Vector & v)
{
	x = y = z = v.x*x + v.y*y + v.z*z;
	return *this;
}

//Векторное перемножение
mathinline Vector & mathcall Vector::operator ^= (const Vector & v)
{
	float tx = y * v.z - z * v.y;
	float ty = z * v.x - x * v.z;
	float tz = x * v.y - y * v.x;
	x = tx;
	y = ty;
	z = tz;
	return *this;
}

/*!\relates Vector
Сложить
*/
mathinline Vector mathcall operator + (const Vector & v, float f)
{
	Vector tv(v);
	tv += f;
	return tv;
}

/*!\relates Vector
Сложить
*/
mathinline Vector mathcall operator + (float f, const Vector & v)
{
	Vector tv(v);
	tv += f;
	return tv;
}

/*!\relates Vector
Сложить
*/
mathinline Vector mathcall operator + (const Vector & v, double d)
{
	Vector tv(v);
	tv += d;
	return tv;
}

/*!\relates Vector
Сложить
*/
mathinline Vector mathcall operator + (double d, const Vector & v)
{
	Vector tv(v);
	tv += d;
	return tv;
}

/*!\relates Vector
Сложить
*/
mathinline Vector mathcall operator + (const Vector & v1, const Vector & v2)
{
	Vector tv(v1);
	tv += v2;
	return tv;
}

/*!\relates Vector
Вычесть
*/
mathinline Vector mathcall operator - (const Vector & v, float f)
{
	Vector tv(v);
	tv -= f;
	return tv;
}

/*!\relates Vector
Вычесть
*/
mathinline Vector mathcall operator - (float f, const Vector & v)
{
	Vector tv(v);
	tv -= f;
	return tv;
}

/*!\relates Vector
Вычесть
*/
mathinline Vector mathcall operator - (const Vector & v, double d)
{
	Vector tv(v);
	tv -= d;
	return tv;
}

/*!\relates Vector
Вычесть
*/
mathinline Vector mathcall operator - (double d, const Vector & v)
{
	Vector tv(v);
	tv -= d;
	return tv;
}

/*!\relates Vector
Вычесть
*/
mathinline Vector mathcall operator - (const Vector & v1, const Vector & v2)
{
	Vector tv(v1);
	tv -= v2;
	return tv;
}

/*!\relates Vector
Умножить
*/
mathinline Vector mathcall operator * (const Vector & v, float f)
{
	Vector tv(v);
	tv *= f;
	return tv;
}

/*!\relates Vector
Умножить
*/
mathinline Vector mathcall operator * (float f, const Vector & v)
{
	Vector tv(v);
	tv *= f;
	return tv;
}

/*!\relates Vector
Умножить
*/
mathinline Vector mathcall operator * (const Vector & v, double d)
{
	Vector tv(v);
	tv *= d;
	return tv;
}

/*!\relates Vector
Умножить
*/
mathinline Vector mathcall operator * (double d, const Vector & v)
{
	Vector tv(v);
	tv *= d;
	return tv;
}

/*!\relates Vector
Умножить
*/
mathinline Vector mathcall operator * (const Vector & v1, const Vector & v2)
{
	Vector tv(v1);
	tv *= v2;
	return tv;
}

/*!\relates Vector
Разделить
*/
mathinline Vector mathcall operator / (const Vector & v, float f)
{
	Vector tv(v);
	tv /= f;
	return tv;
}

/*!\relates Vector
Разделить
*/
mathinline Vector mathcall operator / (float f, const Vector & v)
{
	Vector tv(f);
	tv /= v;
	return tv;
}

/*!\relates Vector
Разделить
*/
mathinline Vector mathcall operator / (const Vector & v, double d)
{
	Vector tv(v);
	tv /= d;
	return tv;
}

/*!\relates Vector
Разделить
*/
mathinline Vector mathcall operator / (double d, const Vector & v)
{
	Vector tv(d);
	tv /= v;
	return tv;
}

/*!\relates Vector
Разделить
*/
mathinline Vector mathcall operator / (const Vector & v1, const Vector & v2)
{
	Vector tv(v1);
	tv /= v2;
	return tv;
}

/*!\relates Vector
Скалярное перемножение
*/
mathinline float mathcall operator | (const Vector & v1, const Vector & v2)
{	
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

/*!\relates Vector
Векторное перемножение
*/
mathinline Vector mathcall operator ^ (const Vector & v1, const Vector & v2)
{
	Vector v;
	v.x = v1.y*v2.z - v1.z*v2.y;
	v.y = v1.z*v2.x - v1.x*v2.z;
	v.z = v1.x*v2.y - v1.y*v2.x;
	return v;
}

/*!\relates Vector
Сравнить покомпонентно
*/
mathinline bool mathcall operator > (const Vector & v1, const Vector & v2)
{
	if(v1.x > v2.x && v1.y > v2.y && v1.z > v2.z) return true;	
	return false;
}

/*!\relates Vector
Сравнить покомпонентно
*/
mathinline bool mathcall operator >= (const Vector & v1, const Vector & v2)
{
	if(v1.x >= v2.x && v1.y >= v2.y && v1.z >= v2.z) return true;	
	return false;
}

/*!\relates Vector
Сравнить покомпонентно
*/
mathinline bool mathcall operator < (const Vector & v1, const Vector & v2)
{
	if(v1.x < v2.x && v1.y < v2.y && v1.z < v2.z) return true;	
	return false;
}

/*!\relates Vector
Сравнить покомпонентно
*/
mathinline bool mathcall operator <= (const Vector & v1, const Vector & v2)
{
	if(v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z) return true;	
	return false;
}

/*!\relates Vector
Сравнить покомпонентно
*/
mathinline bool mathcall operator == (const Vector & v1, const Vector & v2)
{
	if(fabsf(v1.x - v2.x) < 1e-34f && fabsf(v1.y - v2.y) < 1e-34f && fabsf(v1.z - v2.z) < 1e-34f) return true;
	return false;
}

/*!\relates Vector
Сравнить покомпонентно
*/
mathinline bool mathcall operator != (const Vector & v1, const Vector & v2)
{	
	return !(v1 == v2);
}

//===========================================================
//Преобразование
//===========================================================

//Нормализовать вектор, и вернуть его бывшую длинну
mathinline float mathcall Vector::Normalize()
{
	float len = sqrtf(x*x + y*y + z*z);
	if(len > 0.0)
	{
		float k = 1.0f/len;
		x = float(x*k);
		y = float(y*k);
		z = float(z*k);
	}
	return float(len);
}

//Спроецировать на плоскость XZ и нормализовать
mathinline float mathcall Vector::NormalizeXZ()
{
	y = 0.0f;
	float len = sqrtf(x*x + z*z);
	if(len > 0.0)
	{
		float k = 1.0f/len;
		x = float(x*k);
		z = float(z*k);
	}
	return float(len);
}

//Ограничить длинну вектора, и вернуть текущую длинну
mathinline float mathcall Vector::ClampLength(float clampValue)
{
	if(clampValue <= 0.0f)
	{
		x = y = z = 0.0f;
		return 0.0f;
	}
	float len = x*x + y*y + z*z;
	if(len > clampValue*clampValue)
	{
		float k = clampValue/sqrtf(len);
		x = float(x*k);
		y = float(y*k);
		z = float(z*k);
		return clampValue;
	}
	return float(len);
}

//Ограничить компоненту X диапазоном
mathinline Vector & mathcall Vector::ClampX(float min, float max)
{
	if(x < min) x = min;
	if(x > max) x = max;
	return *this;
}

//Ограничить компоненту Y диапазоном
mathinline Vector & mathcall Vector::ClampY(float min, float max)
{
	if(y < min) y = min;
	if(y > max) y = max;
	return *this;
}

//Ограничить компоненту Z диапазоном
mathinline Vector & mathcall Vector::ClampZ(float min, float max)
{
	if(z < min) z = min;
	if(z > max) z = max;
	return *this;
}

//Ограничить компоненты диапазоном
mathinline Vector & mathcall Vector::Clamp(float min, float max)
{
	if(x < min) x = min;
	if(x > max) x = max;
	if(y < min) y = min;
	if(y > max) y = max;
	if(z < min) z = min;
	if(z > max) z = max;
	return *this;
}

//Сохранить в векторе минимальные компаненты
mathinline Vector & mathcall Vector::Min(const Vector & v)
{
	if(x > v.x) x = v.x;
	if(y > v.y) y = v.y;
	if(z > v.z) z = v.z;
	return *this;
}

//Сохранить в векторе максимальные компаненты
mathinline Vector & mathcall Vector::Max(const Vector & v)
{
	if(x < v.x) x = v.x;
	if(y < v.y) y = v.y;
	if(z < v.z) z = v.z;
	return *this;
}

///Сохранить в векторе минимальные компаненты
mathinline Vector & mathcall Vector::Min(const Vector & v1, const Vector & v2)
{
	x = (v1.x < v2.x) ? v1.x : v2.x;
	y = (v1.y < v2.y) ? v1.y : v2.y;
	z = (v1.z < v2.z) ? v1.z : v2.z;
	return *this;
}

///Сохранить в векторе максимальные компаненты
mathinline Vector & mathcall Vector::Max(const Vector & v1, const Vector & v2)
{
	x = (v1.x > v2.x) ? v1.x : v2.x;
	y = (v1.y > v2.y) ? v1.y : v2.y;
	z = (v1.z > v2.z) ? v1.z : v2.z;
	return *this;
}

///Взять все компоненты по модулю
mathinline Vector & mathcall Vector::Abs()
{
	x = fabsf(x);
	y = fabsf(y);
	z = fabsf(z);
	return *this;
}

//===========================================================
//Утилитные
//===========================================================

//Установить новые значения
mathinline Vector & mathcall Vector::Set(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
	return *this;
}

//Установить новые значения
mathinline Vector & mathcall Vector::Set(const Vector & v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

//Получить угол между векторами
mathinline float mathcall Vector::GetAngle(const Vector & v) const
{
	double len = double(x)*double(x) + double(y)*double(y) + double(z)*double(z);
	len *= double(x)*double(x) + double(y)*double(y) + double(z)*double(z);
	if(len <= 0.0) return 0.0f;
	double cs = (x*v.x + y*v.y + z*v.z)/sqrt(len);
	if(cs > 1.0) cs = 1.0;
	if(cs < -1.0) cs = -1.0;
	return float(acos(cs));
}

///Получить знаковый угол между векторами в плоскости XZ
mathinline float mathcall Vector::GetAngleXZ(const Vector & v) const
{
	double len = double(x)*double(x) + double(z)*double(z);
	len *= double(v.x)*double(v.x) + double(v.z)*double(v.z);
	if(len <= 0.0) return 0.0f;
	len = (x*v.x + z*v.z)/sqrt(len);
	if(len > 1.0) len = 1.0;
	if(len < -1.0) len = -1.0;
	len = acos(len);
	if(z*v.x - x*v.z < 0) len = -len;
	return float(len);
}

//Получить угол поворота вектора вокруг оси Y
mathinline float mathcall Vector::GetAY(float defAngle) const
{
	double len = double(x)*double(x) + double(z)*double(z);
	if(len > 0.00000000001)
	{
		len = z/sqrt(len);
		if(len > 1.0) len = 1.0;
		if(len < -1.0) len = -1.0;
		len = acos(len);
	}else return defAngle;
	if(x < 0) len = -len;
	return float(len);
}

//Получить синус между 2D векторами в плоскости XZ
mathinline float mathcall Vector::SinXZ(const Vector & v) const
{
	double len = double(x)*double(x) + double(z)*double(z);
	len *= double(v.x)*double(v.x) + double(v.z)*double(v.z);
	if(len <= 0.0) return 0.0f;
	len = (z*v.x - x*v.z)/sqrt(len);
	return float(len);
}

//Получить косинус между 2D векторами в плоскости XZ
mathinline float mathcall Vector::CosXZ(const Vector & v) const
{
	double len = double(x)*double(x) + double(z)*double(z);
	len *= double(v.x)*double(v.x) + double(v.z)*double(v.z);
	if(len <= 0.0) return 1.0f;
	len = (x*v.x + z*v.z)/sqrt(len);
	return float(len);
}

//Получить векторное произведение векторов в плоскости XZ
mathinline float mathcall Vector::CrossXZ(const Vector & v) const
{
	return z*v.x - x*v.z;
}

//Получить вектор в плоскости XZ
mathinline Vector mathcall Vector::GetXZ() const
{
	return Vector(x, 0.0f, z);
}

///Получить вектор в плоскости XZ
mathinline Vector mathcall Vector::GetXZ(float y) const
{
	return Vector(x, y, z);
}

//Сформировать вектор в плоскости XZ по угло
mathinline Vector & mathcall Vector::MakeXZ(float ay)
{
	x = sinf(ay);
	y = 0.0f;
	z = cosf(ay);
	return *this;
}

//Получить длинну вектора
mathinline float mathcall Vector::GetLength() const
{
	return float(sqrtf(x*x + y*y + z*z));
}

///Получить квадрат длинны вектора
mathinline float mathcall Vector::GetLength2() const
{
	return x*x + y*y + z*z;
}

//Получить длинну вектора в 2D
mathinline float mathcall Vector::GetLengthXZ() const
{
	return float(sqrtf(x*x + z*z));
}

///Получить квадрат длинны вектора в 2D
mathinline float mathcall Vector::GetLengthXZ2() const
{
	return x*x + z*z;
}

//Повернуть вектор в плоскости XZ на угол
mathinline Vector & mathcall Vector::Rotate(float angle)
{
	return Rotate(cosf(angle), sinf(angle));
}

//Повернуть вектор в плоскости XZ на угол заданный cos, sin
mathinline Vector & mathcall Vector::Rotate(float vcos, float vsin)
{
	float tx = x*vcos + z*vsin;
	float tz = z*vcos - x*vsin;
	x = tx; z = tz;
	return *this;
}

///Повернуть вектор в плоскости XY на угол заданный cos, sin
mathinline Vector & mathcall Vector::RotateXY(float vcos, float vsin)
{
	float tx = x*vcos + y*vsin;
	float ty = y*vcos - x*vsin;
	x = tx; y = ty;
	return *this;
}

//Повернуть вектор по часовой стрелке в плоскости XZ на угол PI/2
mathinline Vector & mathcall Vector::Rotate_PI2_CW()
{
	float t = x;
	x = z;
	z = -t;
	return *this;
}

//Повернуть вектор против часовой стрелке в плоскости XZ на угол PI/2
mathinline Vector & mathcall Vector::Rotate_PI2_CCW()
{
	float t = x;
	x = -z;
	z = t;
	return *this;
}

//Расчитать линейноинтерпалированное значение
mathinline Vector & mathcall Vector::Lerp(const Vector & from, const Vector & to, float kBlend)
{
	x = from.x + (to.x - from.x)*kBlend;
	y = from.y + (to.y - from.y)*kBlend;
	z = from.z + (to.z - from.z)*kBlend;
	return *this;
}

///Получить коэфициент затухания 1.0f - (this.Length2() - minDist)*(kLen = 1.0f/(maxDist - minDist))
mathinline float mathcall Vector::GetAttenuation(float minDist, float kLen)
{
	float v = x*x + y*y + z*z;
	v = 1.0f - (v - minDist)*kLen;	
	if(v < 0.0f) v = 0.0f;
	if(v > 1.0f) v = 1.0f;
	return v;
}

///pow(this.GetAttenuation(), 2)
mathinline float mathcall Vector::GetAttenuation2(float minDist, float kLen)
{
	float v = GetAttenuation(minDist, kLen);
	return v*v;
}

///pow(this.GetAttenuation(), 3)
mathinline float mathcall Vector::GetAttenuation3(float minDist, float kLen)
{
	float v = GetAttenuation(minDist, kLen);
	return v*v*v;
}

///pow(this.GetAttenuation(), 4)
mathinline float mathcall Vector::GetAttenuation4(float minDist, float kLen)
{
	float v = GetAttenuation(minDist, kLen);
	v *= v;
	return v*v;
}

///Получить коэфициент затухания 1.0f - (this.Length2D2() - minDist)*(kLen = 1.0f/(maxDist - minDist))
mathinline float mathcall Vector::GetAttenuationXZ(float minDist, float kLen)
{
	float v = x*x + z*z;
	return (v - minDist)*kLen;		
}

///pow(this.GetAttenuation(), 2)
mathinline float mathcall Vector::GetAttenuation2XZ(float minDist, float kLen)
{
	float v = GetAttenuationXZ(minDist, kLen);
	return v*v;
}

///pow(this.GetAttenuation(), 3)
mathinline float mathcall Vector::GetAttenuation3XZ(float minDist, float kLen)
{
	float v = GetAttenuationXZ(minDist, kLen);
	return v*v*v;
}

///pow(this.GetAttenuation(), 4)
mathinline float mathcall Vector::GetAttenuation4XZ(float minDist, float kLen)
{
	float v = GetAttenuationXZ(minDist, kLen);
	v *= v;
	return v*v;
}

//Расчитать отражённый вектор
mathinline Vector & mathcall Vector::Reflection(const Vector & normal)
{
	float k = -2.0f*(x*normal.x + y*normal.y + z*normal.z);
	x += normal.x*k;
	y += normal.y*k;
	z += normal.z*k;
	return *this;
}

//Заполнить единичным вектором со случайным направлением
mathinline Vector & mathcall Vector::Rand()
{
	MakeXZ(rand()*((2.0f*3.141592654f)/RAND_MAX));
	y = z; z = 0.0f;
	return Rotate(rand()*((2.0f*3.141592654f)/RAND_MAX));
}

//Заполнить единичным вектором со случайным направлением в XZ
mathinline Vector & mathcall Vector::RandXZ()
{
	return MakeXZ(rand()*((2.0f*3.141592654f)/RAND_MAX));
}

///Заполнить случайными значениями в заданном ABB
mathinline Vector & mathcall Vector::Rand(const Vector & min, const Vector & max)
{
	x = min.x + rand()*((max.x - min.x)*(1.0f/RAND_MAX));
	y = min.y + rand()*((max.y - min.y)*(1.0f/RAND_MAX));
	z = min.z + rand()*((max.z - min.z)*(1.0f/RAND_MAX));
	return *this;
}

///Заполнить случайными значениями в заданной сфере
mathinline Vector & mathcall Vector::Rand(const Vector & pos, float radius)
{
	Rand();
	*this *= rand()*(radius*(1.0f/RAND_MAX));
	return *this;
}

//Переместить текущий вектор к заданной точке на заданный шаг
mathinline bool mathcall Vector::MoveByStep(const Vector & to, float step)
{
	if(step <= 0.0f) return false;
	//Дистанция до точки
	float dx = to.x - x;
	float dy = to.y - y;
	float dz = to.z - z;
	double dist = dx*dx + dy*dy + dz*dz;
	if(dist < 1e-30)
	{
		x = to.x;
		y = to.y;
		z = to.z;
		return false;
	}
	dist = sqrt(dist);
	//Движение к точке
	if(step >= dist)
	{
		x = to.x;
		y = to.y;
		z = to.z;
		return true;
	}
	double k = step/dist;
	x += float(dx*k);
	y += float(dy*k);
	z += float(dz*k);
	return true;
}

//Переместить текущий вектор к заданной точке на заданный шаг в 2D
mathinline bool mathcall Vector::MoveByStepXZ(const Vector & to, float step)
{
	if(step <= 0.0f) return false;
	//Дистанция до точки
	float dx = to.x - x;
	float dz = to.z - z;
	double dist = dx*dx + dz*dz;
	if(dist < 1e-30)
	{
		x = to.x;
		z = to.z;
		return false;
	}
	dist = sqrt(dist);
	//Движение к точке
	if(step >= dist)
	{
		x = to.x;
		z = to.z;
		return true;
	}
	double k = step/dist;
	x += float(dx*k);
	z += float(dz*k);
	return true;
}

//Точка находиться в ящике
mathinline bool mathcall Vector::InBox(const Vector & min, const Vector & max) const
{
	if(y < min.y || y > max.y) return false;
	if(x < min.x || x > max.x) return false;
	if(z < min.z || z > max.z) return false;
	return true;
}

//Точка находиться в шаре
mathinline bool mathcall Vector::InSphere(const Vector & pos, float rad) const
{
	double d = (x - pos.x)*(x - pos.x);
	d += (y - pos.y)*(y - pos.y);
	d += (z - pos.z)*(z - pos.z);
	return d < double(rad)*double(rad);
}

//Торможение, v -= v^power*k; максимально вычитается v
mathinline Vector & mathcall Vector::Friction(float k, float power)
{
	//Получим скорость в абсолютных велечинах и знаки
	Vector sign;
	Vector v;
	if(x >= 0.0f)
	{
		sign.x = 1.0f;
		v.x = x;
	}else{
		sign.x = -1.0f;
		v.x = -x;
	}
	if(y >= 0.0f)
	{
		sign.y = 1.0f;
		v.y = y;
	}else{
		sign.y = -1.0f;
		v.y = -y;
	}
	if(z >= 0.0f)
	{
		sign.z = 1.0f;
		v.z = z;
	}else{
		sign.z = -1.0f;
		v.z = -z;
	}
	//Расчитаем в абсолютных велечинах
	if(power < 1e-10f)
	{
		power = 1e-10f;
	}
	x = v.x - powf(v.x, power)*k;
	y = v.y - powf(v.y, power)*k;
	z = v.z - powf(v.z, power)*k;
	if(x < 0.0f) x = 0.0f;
	if(y < 0.0f) y = 0.0f;
	if(z < 0.0f) z = 0.0f;
	x *= sign.x;
	y *= sign.y;
	z *= sign.z;
	return *this;
}

//Торможение, v -= v^power*k; максимально вычитается v для компонент XZ
mathinline Vector & mathcall Vector::FrictionXZ(float k, float power)
{
	//Получим скорость в абсолютных велечинах и знаки
	Vector sign;
	Vector v;
	if(x >= 0.0f)
	{
		sign.x = 1.0f;
		v.x = x;
	}else{
		sign.x = -1.0f;
		v.x = -x;
	}
	v.y = 0.0f;
	sign.y = 0.0f;
	if(z >= 0.0f)
	{
		sign.z = 1.0f;
		v.z = z;
	}else{
		sign.z = -1.0f;
		v.z = -z;
	}
	//Расчитаем в абсолютных велечинах
	if(power < 1e-10f)
	{
		power = 1e-10f;
	}
	x = v.x - powf(v.x, power)*k;
	z = v.z - powf(v.z, power)*k;
	if(x < 0.0f) x = 0.0f;
	if(z < 0.0f) z = 0.0f;
	x *= sign.x;
	z *= sign.z;
	return *this;
}


#endif
