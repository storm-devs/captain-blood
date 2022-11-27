//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Matrix
//============================================================================================

#ifndef _Matrix_h_
#define _Matrix_h_

#include "Vector4.h"
#include "Plane.h"

//============================================================================================

struct D3DXMATRIX;


//============================================================================================
/*!
<PRE>

Линейное представление        В виде двухмерного массива
                                vx     vy     vz    pos
    0  4  8 12                [0][0] [1][0] [2][0] [3][0]    x
    1  5  9 13                [0][1] [1][1] [2][1] [3][1]    y
    2  6 10 14                [0][2] [1][2] [2][2] [3][2]    z
    3  7 11 15                [0][3] [1][3] [2][3] [3][3]    w
*/
///Класс матрицы для преобразований в 3D.
//============================================================================================

class Matrix
{
public:
	union
	{
		///Линейный массив
		float matrix[16];
		///Двумерный массив
		float m[4][4];
		struct
		{
			///Направление по X
			Vector vx;
			///Весовое значение по X
			float wx;
			///Направление по Y
			Vector vy;
			///Весовое значение по Y
			float wy;
			///Направление по Z
			Vector vz;
			///Весовое значение по Z
			float wz;
			///Позиция
			Vector pos;
			//Добавляемое весовое значение
			float w;
		};
	};

//-----------------------------------------------------------
//Конструкторы
//-----------------------------------------------------------
public:
	///Сконструировать единичную матрицу
	Matrix();
	///Сконструировать матрицу без заполнения
	Matrix(bool empty);
	///Сконструировать матрицу по углам с позицией
	Matrix(float angX, float angY, float angZ, float x, float y, float z);
	///Сконструировать матрицу по углам
	Matrix(float angX, float angY, float angZ);
	///Сконструировать матрицу по углам с позицией
	Matrix(const Vector & ang, const Vector & pos);
	///Сконструировать матрицу по углам
	Matrix(const Vector & ang);
	///Сконструировать копию матрицы
	Matrix(const Matrix & matrix);
	///Сконструировать результат перемножения матриц this = m1*m2
	Matrix(const Matrix & m1, const Matrix & m2);

//-----------------------------------------------------------
//Операторы
//-----------------------------------------------------------
public:
	//Присвоить матрице другую матрицу
	Matrix & mathcall operator = (const Matrix & mtx);
	//Присвоить позиции матрицы число
	Matrix & mathcall operator = (float f);
	//Присвоить позиции матрицы число
	Matrix & mathcall operator = (double d);
	//Присвоить позиции матрицы вектор
	Matrix & mathcall operator = (const Vector & v);

	//Перемножить матрицы
	Matrix & mathcall operator *= (const Matrix & mtx);

	
//-----------------------------------------------------------
//Заполнение матрицы
//-----------------------------------------------------------
public:
	///Установить единичную матрицу
	Matrix & mathcall SetIdentity();
	///Установить единичную матрицу для поворотов
	Matrix & mathcall SetIdentity3x3();
	///Установить нулевую матрицу
	Matrix & mathcall SetZero();

	///Установить матрицу
	Matrix & mathcall Set(const Matrix & matrix);
	
	///Посчитать матрицу M = rotZ*rotX*rotY*Pos
	Matrix & mathcall Build(float angX, float angY, float angZ, float x, float y, float z);
	///Посчитать матрицу M = rotZ*rotX*rotY
	Matrix & mathcall Build(float angX, float angY, float angZ);
	///Посчитать матрицу M = rotZ*rotX*rotY*Pos
	Matrix & mathcall Build(const Vector & ang, const Vector & pos);
	///Посчитать матрицу M = rotZ*rotX*rotY
	Matrix & mathcall Build(const Vector & ang);

	///Посчитать матрицу M = rotX*rotY*rotZ*Pos
	Matrix & mathcall BuildXYZ(float angX, float angY, float angZ, float x, float y, float z);

	///Посчитать матрицу поворота вокруг X
	Matrix & mathcall BuildRotateX(float ang);
	///Посчитать матрицу поворота вокруг Y
	Matrix & mathcall BuildRotateY(float ang);
	///Посчитать матрицу поворота вокруг Z
	Matrix & mathcall BuildRotateZ(float ang);
	///Посчитать матрицу поворота вокруг заданной оси
	bool mathcall BuildRotate(const Vector & axis, float ang);
	///Посчитать матрицу позиции
	Matrix & mathcall BuildPosition(float x, float y, float z);
	///Посчитать матрицу позиции
	Matrix & mathcall BuildPosition(const Vector & pos);

	///Посчитать матрицу масштабирования
	Matrix & mathcall BuildScale(float scale);
	///Посчитать матрицу масштабирования
	Matrix & mathcall BuildScale(float scaleX, float scaleY, float scaleZ);
	///Посчитать матрицу масштабирования
	Matrix & mathcall BuildScale(const Vector & scale);

	///Посчитать матрицу проекции
	Matrix & mathcall BuildProjection(float viewAngle, float vpWidth, float vpHeight, float zNear, float zFar);
	///Посчитать матрицу камеры
	bool mathcall BuildView(Vector lookFrom, Vector lookTo, Vector upVector);
	///Посчитать матрицу из камеры объекта
	Matrix & mathcall BuildViewFromObject();
	///Посчитать матрицу ориентации объекта имея направление по z и направление вверх
	bool mathcall BuildOrient(Vector zAxisDirection, Vector upVector);
	///Посчитать матрицу объекта имея позицию точку куда направлен объект и направление вверх
	bool mathcall BuildOriented(Vector position, Vector lookTo, Vector upVector);
	///Посчитать матрицу для отзеркаливания геометрии
	Matrix & mathcall BuildMirror(float Nx, float Ny, float Nz, float D);
	///Посчитать ортоганальную матрицу проекции
	Matrix & mathcall BuildOrtoProjection(float vpWidth, float vpHeight, float zNear, float zFar);
	//Посчитать матрицу проекции для тени
	Matrix & mathcall BuildShadowProjection(float viewAngle, float vpWidth, float vpHeight, float zNear, float zFar);

//-----------------------------------------------------------
//Преобразование матрицы
//-----------------------------------------------------------
public:
	///Повернуть вокруг X
	Matrix & mathcall RotateX(float ang);
	///Повернуть вокруг Y
	Matrix & mathcall RotateY(float ang);
	///Повернуть вокруг Z
	Matrix & mathcall RotateZ(float ang);
	///Повернуть вокруг ZXY
	Matrix & mathcall Rotate(float angX, float angY, float angZ);
	///Повернуть вокруг ZXY
	Matrix & mathcall Rotate(const Vector & ang);
	
	///Переместить
	Matrix & mathcall Move(float dX, float dY, float dZ);
	///Переместить
	Matrix & mathcall Move(const Vector & pos);

	///Отмасштабировать
	Matrix & mathcall Scale(float scale);
	///Отмасштабировать матрицу поворота
	Matrix & mathcall Scale3x3(float scale);
	///Отмасштабировать
	Matrix & mathcall Scale(float scaleX, float scaleY, float scaleZ);
	///Отмасштабировать матрицу поворота
	Matrix & mathcall Scale3x3(float scaleX, float scaleY, float scaleZ);
	///Отмасштабировать
	Matrix & mathcall Scale(const Vector & scale);
	///Отмасштабировать матрицу поворота
	Matrix & mathcall Scale3x3(const Vector & scale);
	
	///Расчёт обратной матрицы
	Matrix & mathcall Inverse();
	///Расчёт обратной матрицы из другой
	Matrix & mathcall Inverse(const Matrix & mtx);
	///Полный расчёт обратной матрицы 4x3
	bool mathcall InverseComplette();
	///Полный расчёт обратной матрицы 4x4
	bool mathcall InverseComplette4X4();
	///Транспанирование матрицы
	Matrix & mathcall Transposition();
	///Транспанирование элементов поворота
	Matrix & mathcall Transposition3X3();


//-----------------------------------------------------------
//Утилитные
//-----------------------------------------------------------
public:
	///Считать только вращение
	Matrix & mathcall SetRotate(const Matrix & mtr);
	///Перемножить матрицы и результат поместить в текущую
	Matrix & mathcall EqMultiply(const Matrix & m1, const Matrix & m2);
	///Перемножить матрицы и результат поместить в текущую m1 != this && m2 != this
	Matrix & mathcall EqMultiplyFast(const Matrix & m1, const Matrix & m2);

	///Умножить вершину на матрицу
	Vector mathcall MulVertex(const Vector & v) const;
	///Умножить вершину на матрицу
	Vector4 mathcall MulVertex(const Vector4 & v) const;
	///Умножить нормаль на матрицу
	Vector mathcall MulNormal(const Vector & v) const;
	///Умножить вершину на инверстую матрицу
	Vector mathcall MulVertexByInverse(const Vector & v) const;
	///Умножить нормаль на инверстую матрицу
	Vector mathcall MulNormalByInverse(const Vector & v) const;

	///Получить позицию камеры из матрицы камеры
	Vector mathcall GetCamPos() const;
	///Единичная матрица или нет
	bool mathcall IsIdentity() const;
	///Скалирования матрица или нет
	bool mathcall IsScale() const;
	///Проверить матрицу на инверсное скалирование
	bool mathcall IsParity() const;

	///Споецировать вершину (для матрицы проекции)
	Vector4 mathcall Projection(const Vector & vertex, float vphWidth05 = 1.0f, float vphHeight05 = 1.0f) const;
	///Споецировать массив вершин (для матрицы проекции)
	void mathcall Projection(Vector4 * dstArray, Vector * srcArray, long num, float vphWidth05 = 1.0f, float vphHeight05 = 1.0f, long srcSize = sizeof(Vector), long dstSize = sizeof(Vector4)) const;

	//Получить углы из нескалированной матрицы поворота
	void mathcall GetAngles(float & ax, float & ay, float & az);
	//Получить углы из нескалированной матрицы поворота
	void mathcall GetAngles(Vector & ang);
	//Получить углы из нескалированной матрицы поворота
	Vector mathcall GetAngles();

	//Получить коэфициенты скалирования матрицы
	Vector mathcall GetScale() const;
	//Нормализация (исключение скалирования)
	Matrix & mathcall Normalize();

	//Умножить все элементы на число
	Matrix & mathcall Mul(float k);
	//Сложить прибавить другую матрицу
	Matrix & mathcall Add(const Matrix & m);
	//Добавить другую матрицу умноженую на число
	Matrix & mathcall Mad(const Matrix & m, float k);

	//Доступиться до элементов матрицы через скобки
	float & mathcall operator () (long i, long j);

	///Получить указатель на матрицу D3D
	operator D3DXMATRIX * () const;

	///Получить вектор для расчёта X компоненты
	Vector4 mathcall GetVectorX() const;
	///Получить вектор для расчёта Y компоненты
	Vector4 mathcall GetVectorY() const;
	///Получить вектор для расчёта Z компоненты
	Vector4 mathcall GetVectorZ() const;
	///Получить вектор для расчёта W компоненты
	Vector4 mathcall GetVectorW() const;


//-----------------------------------------------------------
//Вычисления с использыванием SIMD. Указатели должны быть выровнены по 16 байт границе
//-----------------------------------------------------------
	
	///Перемножить матрицы 4х4 с использыванием SIMD. Указатели должны быть выровнены по 16 байт границе.
	static void mathcall MultiplySIMD_4x4(float * res, const float * m1, const float * m2);
	///Перемножить матрицы 4х3 с использыванием SIMD. Указатели должны быть выровнены по 16 байт границе.
	static void mathcall MultiplySIMD_4x3(float * res, const float * m1, const float * m2);
};


//===========================================================
//Конструкторы
//===========================================================

//Сконструировать единичную матрицу
mathinline Matrix::Matrix()
{
	SetIdentity();
}

///Сконструировать матрицу без заполнения
mathinline Matrix::Matrix(bool empty)
{
}

//Сконструировать матрицу по углам с позицией
mathinline Matrix::Matrix(float angX, float angY, float angZ, float x, float y, float z)
{
	Build(angX, angY, angZ, x, y, z);
}

//Сконструировать матрицу по углам
mathinline Matrix::Matrix(float angX, float angY, float angZ)
{
	Build(angX, angY, angZ);
}

//Сконструировать матрицу по углам с позицией
mathinline Matrix::Matrix(const Vector & ang, const Vector & pos)
{
	Build(ang, pos);
}

//Сконструировать матрицу по углам
mathinline Matrix::Matrix(const Vector & ang)
{
	Build(ang);
}

//Сконструировать копию матрицы
mathinline Matrix::Matrix(const Matrix & matrix)
{
	Set(matrix);
}

//Сконструировать результат перемножения матриц this = m1*m2
mathinline Matrix::Matrix(const Matrix & m1, const Matrix & m2)
{
	EqMultiply(m1, m2);
}


//===========================================================
//Операторы
//===========================================================

//Присвоить матрице другую матрицу
mathinline Matrix & mathcall Matrix::operator = (const Matrix & mtx)
{
	Set(mtx);
	return *this;
}

//Присвоить позиции матрицы число
mathinline Matrix & mathcall Matrix::operator = (float f)
{
	pos = f;
	return *this;
}

//Присвоить позиции матрицы число
mathinline Matrix & mathcall Matrix::operator = (double d)
{
	pos = d;
	return *this;
}

//Присвоить позиции матрицы вектор
mathinline Matrix & mathcall Matrix::operator = (const Vector & v)
{
	pos = v;
	return *this;
}

//Перемножить матрицы
mathinline Matrix & mathcall Matrix::operator *= (const Matrix & mtx)
{
	EqMultiply(*this, mtx);
	return *this;
}

/*!\relates Matrix
Перемножить матрицы
*/
mathinline Matrix mathcall operator * (const Matrix & m1, const Matrix & m2)
{
	Matrix m;
	m.EqMultiplyFast(m1, m2);
	return m;
}

/*!\relates Matrix
Умножить вектор на матрицу
*/
mathinline Vector mathcall operator * (const Matrix & mtx, const Vector & v)
{
	return mtx.MulVertex(v);
}

/*!\relates Matrix
Умножить вектор на матрицу
*/
mathinline Vector mathcall operator * (const Vector & v, const Matrix & mtx)
{
	return mtx.MulVertex(v);
}

/*!\relates Matrix
Умножить вектор 4 на матрицу
*/
mathinline Vector4 mathcall operator * (const Matrix & mtx, const Vector4 & v)
{
	return mtx.MulVertex(v);
}

/*!\relates Matrix
Умножить вектор 4 на матрицу
*/
mathinline Vector4 mathcall operator * (const Vector4 & v, const Matrix & mtx)
{
	return mtx.MulVertex(v);
}


/*!\relates Matrix
Умножить плоскость на матрицу
*/
mathinline Plane mathcall operator * (const Matrix & mtx, const Plane & p)
{
	Plane plane;
	Vector pos = p.N*p.D;
	pos = mtx*pos;
	plane.N = mtx.MulNormal(p.N);
	plane.D = pos | plane.N;
	return plane;
}

/*!\relates Matrix
Умножить плоскость на матрицу
*/
mathinline Plane mathcall operator * (const Plane & p, const Matrix & mtx)
{
	return mtx*p;
}
	
//===========================================================
//Заполнение матрицы
//===========================================================

//Установить единичную матрицу
mathinline Matrix & mathcall Matrix::SetIdentity()
{
/*
	_asm
	{
		mov		eax, this
		mov		ecx, 0x3f800000
		xor		ebx, ebx
		mov		[eax + 0*4], ecx
		mov		[eax + 1*4], ebx
		mov		[eax + 2*4], ebx
		mov		[eax + 3*4], ebx
		mov		[eax + 4*4], ebx
		mov		[eax + 5*4], ecx
		mov		[eax + 6*4], ebx
		mov		[eax + 7*4], ebx
		mov		[eax + 8*4], ebx
		mov		[eax + 9*4], ebx
		mov		[eax + 10*4], ecx
		mov		[eax + 11*4], ebx
		mov		[eax + 12*4], ebx
		mov		[eax + 13*4], ebx
		mov		[eax + 14*4], ebx
		mov		[eax + 15*4], ecx
	}
	return *this;
*/

	matrix[0] = 1.0f;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = 1.0f;
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = 1.0f;
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	return *this;

}

//Установить единичную матрицу для поворотов
mathinline Matrix & mathcall Matrix::SetIdentity3x3()
{
/*
	_asm
	{
		mov		eax, this
		mov		ecx, 0x3f800000
		xor		ebx, ebx
		mov		[eax + 0*4], ecx
		mov		[eax + 1*4], ebx
		mov		[eax + 2*4], ebx
		mov		[eax + 4*4], ebx
		mov		[eax + 5*4], ecx
		mov		[eax + 6*4], ebx
		mov		[eax + 8*4], ebx
		mov		[eax + 9*4], ebx
		mov		[eax + 10*4], ecx
	}
	return *this;
*/
	matrix[0] = 1.0f;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = 1.0f;
	matrix[6] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = 1.0f;

	return *this;

}

///Установить нулевую матрицу
mathinline Matrix & mathcall Matrix::SetZero()
{
/*
	_asm
	{
			xor		ebx, ebx
			mov		eax, this			
			mov		[eax + 0*4], ebx
			mov		[eax + 1*4], ebx
			mov		[eax + 2*4], ebx
			mov		[eax + 3*4], ebx
			mov		[eax + 4*4], ebx
			mov		[eax + 5*4], ebx
			mov		[eax + 6*4], ebx
			mov		[eax + 7*4], ebx
			mov		[eax + 8*4], ebx
			mov		[eax + 9*4], ebx
			mov		[eax + 10*4], ebx
			mov		[eax + 11*4], ebx
			mov		[eax + 12*4], ebx
			mov		[eax + 13*4], ebx
			mov		[eax + 14*4], ebx
			mov		[eax + 15*4], ebx
	}
	return *this;
*/
	matrix[0] = 0.0f;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = 0.0f;
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = 0.0f;
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 0.0f;

	return *this;

}

//Установить матрицу
mathinline Matrix & mathcall Matrix::Set(const Matrix & matrix)
{
/*
	_asm
	{
		mov		esi, matrix
		mov		edi, this
		mov		eax, [esi + 0*4]
		mov		ebx, [esi + 1*4]
		mov		ecx, [esi + 2*4]
		mov		edx, [esi + 3*4]
		mov		[edi + 0*4], eax
		mov		[edi + 1*4], ebx
		mov		[edi + 2*4], ecx
		mov		[edi + 3*4], edx
		mov		eax, [esi + 4*4]
		mov		ebx, [esi + 5*4]
		mov		ecx, [esi + 6*4]
		mov		edx, [esi + 7*4]
		mov		[edi + 4*4], eax
		mov		[edi + 5*4], ebx
		mov		[edi + 6*4], ecx
		mov		[edi + 7*4], edx
		mov		eax, [esi + 8*4]
		mov		ebx, [esi + 9*4]
		mov		ecx, [esi + 10*4]
		mov		edx, [esi + 11*4]
		mov		[edi + 8*4], eax
		mov		[edi + 9*4], ebx
		mov		[edi + 10*4], ecx
		mov		[edi + 11*4], edx
		mov		eax, [esi + 12*4]
		mov		ebx, [esi + 13*4]
		mov		ecx, [esi + 14*4]
		mov		edx, [esi + 15*4]
		mov		[edi + 12*4], eax
		mov		[edi + 13*4], ebx
		mov		[edi + 14*4], ecx
		mov		[edi + 15*4], edx
	}
	return *this;
*/
	this->matrix[0] = matrix.matrix[0]; 
	this->matrix[1] = matrix.matrix[1];
	this->matrix[2] = matrix.matrix[2];
	this->matrix[3] = matrix.matrix[3];

	this->matrix[4] = matrix.matrix[4]; 
	this->matrix[5] = matrix.matrix[5];
	this->matrix[6] = matrix.matrix[6];
	this->matrix[7] = matrix.matrix[7];

	this->matrix[8] = matrix.matrix[8];  
	this->matrix[9] = matrix.matrix[9]; 
	this->matrix[10] = matrix.matrix[10];
	this->matrix[11] = matrix.matrix[11];

	this->matrix[12] = matrix.matrix[12]; 
	this->matrix[13] = matrix.matrix[13];
	this->matrix[14] = matrix.matrix[14];
	this->matrix[15] = matrix.matrix[15];

	return *this;

}

//Посчитать матрицу M = rotZ*rotX*rotY*Pos
mathinline Matrix & mathcall Matrix::Build(float angX, float angY, float angZ, float x, float y, float z)
{
	//Синусы и косинусы углов поворота
	float sinAx = sinf(angX);
	float cosAx = cosf(angX);
	float sinAy = sinf(angY);
	float cosAy = cosf(angY);
	float sinAz = sinf(angZ);
	float cosAz = cosf(angZ);	
	//Создаём матрицу с порядком вращений rz*rx*ry
	m[0][0] = cosAz*cosAy + sinAz*sinAx*sinAy;		//vx.x
	m[0][1] = sinAz*cosAx;							//vx.y
	m[0][2] = cosAz*-sinAy + sinAz*sinAx*cosAy;		//vx.z
	m[0][3] = 0.0f;
	m[1][0] = -sinAz*cosAy + cosAz*sinAx*sinAy;		//vy.x
	m[1][1] = cosAz*cosAx;							//vy.y
	m[1][2] = -sinAz*-sinAy + cosAz*sinAx*cosAy;	//vy.z
	m[1][3] = 0.0f;
	m[2][0] = cosAx*sinAy;							//vz.x
	m[2][1] = -sinAx;								//vz.y
	m[2][2] = cosAx*cosAy;							//vz.z
	m[2][3] = 0.0f;
	m[3][0] = x;									//pos.x
	m[3][1] = y;									//pos.y
	m[3][2] = z;									//pos.z
	m[3][3] = 1.0f;
	return *this;
}

//Посчитать матрицу M = rotZ*rotX*rotY
mathinline Matrix & mathcall Matrix::Build(float angX, float angY, float angZ)
{
	Build(angX, angY, angZ, 0.0f, 0.0f, 0.0f);
	return *this;
}

//Посчитать матрицу M = rotZ*rotX*rotY*Pos
mathinline Matrix & mathcall Matrix::Build(const Vector & ang, const Vector & pos)
{
	Build(ang.x, ang.y, ang.z, pos.x, pos.y, pos.z);
	return *this;
}

//Посчитать матрицу M = rotZ*rotX*rotY
mathinline Matrix & mathcall Matrix::Build(const Vector & ang)
{
	Build(ang.x, ang.y, ang.z, 0.0f, 0.0f, 0.0f);
	return *this;
}


//Посчитать матрицу M = rotX*rotY*rotZ*Pos
mathinline Matrix & mathcall Matrix::BuildXYZ(float angX, float angY, float angZ, float x, float y, float z)
{
	//Синусы и косинусы углов поворота
	float sinAx = sinf(angX);
	float cosAx = cosf(angX);
	float sinAy = sinf(angY);
	float cosAy = cosf(angY);
	float sinAz = sinf(angZ);
	float cosAz = cosf(angZ);
	//Создаём матрицу с порядком вращений rx*ry*rz
	m[0][0] = cosAy*cosAz;
	m[0][1] = cosAy*sinAz;
	m[0][2] = -sinAy;
	m[0][3] = 0.0f;
	m[1][0] = sinAx*sinAy*cosAz-cosAx*sinAz;
	m[1][1] = sinAx*sinAy*sinAz+cosAx*cosAz;
	m[1][2] = sinAx*cosAy;
	m[1][3] = 0.0f;
	m[2][0] = cosAx*sinAy*cosAz+sinAx*sinAz;
	m[2][1] = cosAx*sinAy*sinAz-sinAx*cosAz;
	m[2][2] = cosAx*cosAy;
	m[2][3] = 0.0f;
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	m[3][3] = 1.0f;
	return *this;
}

//Посчитать матрицу поворота вокруг X
mathinline Matrix & mathcall Matrix::BuildRotateX(float ang)
{
	SetIdentity();
	m[1][1] = cosf(ang);
	m[1][2] = sinf(ang);
	m[2][1] = -sinf(ang);	
	m[2][2] = cosf(ang);
	return *this;
}

//Посчитать матрицу поворота вокруг Y
mathinline Matrix & mathcall Matrix::BuildRotateY(float ang)
{
	SetIdentity();
	m[0][0] = cosf(ang);	
	m[0][2] = -sinf(ang);
	m[2][0] = sinf(ang);
	m[2][2] = cosf(ang);
	return *this;
}

//Посчитать матрицу поворота вокруг Z
mathinline Matrix & mathcall Matrix::BuildRotateZ(float ang)
{
	SetIdentity();
	m[0][0] = cosf(ang);	
	m[0][1] = sinf(ang);
	m[1][0] = -sinf(ang);
	m[1][1] = cosf(ang);
	return *this;
}

//Посчитать матрицу поворота вокруг заданной оси
mathinline bool mathcall Matrix::BuildRotate(const Vector & axis, float ang)
{	
	Matrix m;
	m.vz = axis;
	if(m.vz.Normalize() < 1e-20f)
	{
		SetIdentity();
		return false;
	}
	m.vy = m.vz ^ Vector(1.0f, 0.0f, 0.0f);
	if(m.vy.Normalize() < 1e-20f)
	{
		m.vy = m.vz ^ Vector(0.0f, 1.0f, 0.0f);
		if(m.vy.Normalize() < 1e-20f)
		{
			m.vy = m.vz ^ Vector(0.0f, 0.0f, 1.0f);
			if(m.vy.Normalize() < 1e-20f)
			{
				//Сюда попасть нереально
				SetIdentity();
				return false;
			}
		}
	}
	m.vx = m.vy ^ m.vz;
	*this = Matrix(m).Inverse()*Matrix().BuildRotateZ(ang)*m;
	return true;
}

//Посчитать матрицу позиции
mathinline Matrix & mathcall Matrix::BuildPosition(float x, float y, float z)
{
	SetIdentity();
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	return *this;
}

//Посчитать матрицу позиции
mathinline Matrix & mathcall Matrix::BuildPosition(const Vector & pos)
{
	BuildPosition(pos.x, pos.y, pos.z);
	return *this;
}

//Посчитать матрицу масштабирования
mathinline Matrix & mathcall Matrix::BuildScale(float scale)
{
	SetIdentity();
	m[0][0] = scale;
	m[1][1] = scale;
	m[2][2] = scale;
	return *this;
}

//Посчитать матрицу масштабирования
mathinline Matrix & mathcall Matrix::BuildScale(float scaleX, float scaleY, float scaleZ)
{
	SetIdentity();
	m[0][0] = scaleX;
	m[1][1] = scaleY;
	m[2][2] = scaleZ;
	return *this;
}

//Посчитать матрицу масштабирования
mathinline Matrix & mathcall Matrix::BuildScale(const Vector & scale)
{
	BuildScale(scale.x, scale.y, scale.z);
	return *this;
}


//Посчитать матрицу проекции
mathinline Matrix & mathcall Matrix::BuildProjection(float viewAngle, float vpWidth, float vpHeight, float zNear, float zFar)
{
/*
	//Обнулим массив
	_asm
	{
		mov		eax, this
		xor		ebx, ebx
		xor		edx, edx
		mov		[eax + 0*4], ebx
		mov		[eax + 1*4], edx
		mov		[eax + 2*4], ebx
		mov		[eax + 3*4], edx
		mov		[eax + 4*4], ebx
		mov		[eax + 5*4], edx
		mov		[eax + 6*4], ebx
		mov		[eax + 7*4], edx
		mov		[eax + 8*4], ebx
		mov		[eax + 9*4], edx
		mov		[eax + 10*4], ebx
		mov		[eax + 11*4], edx
		mov		[eax + 12*4], ebx
		mov		[eax + 13*4], edx
		mov		[eax + 14*4], ebx
		mov		[eax + 15*4], edx
	}
*/
	SetZero();
	//Заполняем матрицу
	double Q = double(zFar)/double(zFar - zNear);
	m[0][0] = float(1.0/tan(viewAngle*0.5));
	m[1][1] = float(1.0/tan((vpHeight/vpWidth)*viewAngle*0.5));
	m[2][2] = float(Q);
	m[2][3] = 1.0f;
	m[3][2] = float(-Q*zNear);
	return *this;
}

///Посчитать ортоганальную матрицу проекции
mathinline Matrix & mathcall Matrix::BuildOrtoProjection(float vpWidth, float vpHeight, float zNear, float zFar)
{
	//Обнулим массив
	/*
	_asm
	{
		mov		eax, this
			xor		ebx, ebx
			xor		edx, edx
			mov		[eax + 0*4], ebx
			mov		[eax + 1*4], edx
			mov		[eax + 2*4], ebx
			mov		[eax + 3*4], edx
			mov		[eax + 4*4], ebx
			mov		[eax + 5*4], edx
			mov		[eax + 6*4], ebx
			mov		[eax + 7*4], edx
			mov		[eax + 8*4], ebx
			mov		[eax + 9*4], edx
			mov		[eax + 10*4], ebx
			mov		[eax + 11*4], edx
			mov		[eax + 12*4], ebx
			mov		[eax + 13*4], edx
			mov		[eax + 14*4], ebx
			mov		[eax + 15*4], edx
	}
	*/
	SetZero();

	//Заполняем матрицу
	double Q = 1.0/double(zFar - zNear);
	m[0][0] = 2.0f/vpWidth;
	m[1][1] = 2.0f/vpHeight;
	m[2][2] = float(Q);
	m[3][2] = float(-Q*zNear);
	m[3][3] = 1.0f;

	return *this;
}

//Посчитать матрицу проекции для тени
mathinline Matrix & mathcall Matrix::BuildShadowProjection(float viewAngle, float vpWidth, float vpHeight, float zNear, float zFar)
{
	//Обнулим массив
	/*
	_asm
	{
		mov		eax, this
			xor		ebx, ebx
			xor		edx, edx
			mov		[eax + 0*4], ebx
			mov		[eax + 1*4], edx
			mov		[eax + 2*4], ebx
			mov		[eax + 3*4], edx
			mov		[eax + 4*4], ebx
			mov		[eax + 5*4], edx
			mov		[eax + 6*4], ebx
			mov		[eax + 7*4], edx
			mov		[eax + 8*4], ebx
			mov		[eax + 9*4], edx
			mov		[eax + 10*4], ebx
			mov		[eax + 11*4], edx
			mov		[eax + 12*4], ebx
			mov		[eax + 13*4], edx
			mov		[eax + 14*4], ebx
			mov		[eax + 15*4], edx
	}
	*/
	SetZero();
	//Заполняем матрицу
	double Q = 1.0/double(zFar - zNear);
	m[0][0] = float(1.0/tan(viewAngle*0.5));
	m[1][1] = float(1.0/tan((vpHeight/vpWidth)*viewAngle*0.5));
	m[2][2] = float(Q);
	m[2][3] = 1.0f;
	m[3][2] = float(-Q*zNear);
	return *this;
}

//Посчитать матрицу камеры
mathinline bool mathcall Matrix::BuildView(Vector lookFrom, Vector lookTo, Vector upVector)
{
	SetIdentity();
	//Нормализуем вектор смотрения
	lookTo -= lookFrom;
	if(lookTo.Normalize() == 0.0f)
	{
		//Ставим позицию для неповёрнутой матрици
		pos = -lookFrom;
		return false;
	}
	//Направляем вектор вверх в нужном направлении
	upVector -= lookTo*(lookTo | upVector);
	//Нормализуем вертор направленный вверх
	if(upVector.Normalize() == 0.0f) upVector.y = 1.0f;
	//Ищем третий вектор базиса
	Vector v = upVector ^ lookTo;
	if(v.Normalize() != 0.0f)
	{
		//Ставим матрицу поворота
		m[0][0] = v.x;
		m[1][0] = v.y;
		m[2][0] = v.z;
		m[0][1] = upVector.x;
		m[1][1] = upVector.y;
		m[2][1] = upVector.z;
		m[0][2] = lookTo.x;
		m[1][2] = lookTo.y;
		m[2][2] = lookTo.z;	
	}else{
		//Ставим позицию для неповёрнутой матрици
		pos = -lookFrom;
		return false;
	}
	//Ставим позицию
	//pos = -MulNormalByInverse(lookFrom);
	pos = -MulNormal(lookFrom);
	return true;
}

//Посчитать матрицу из камеры объекта
mathinline Matrix & mathcall Matrix::BuildViewFromObject()
{
	//pos = -pos;
	Inverse();
	return *this;
}

//Посчитать матрицу ориентации объекта имея направление по z и направление вверх
mathinline bool mathcall Matrix::BuildOrient(Vector zAxisDirection, Vector upVector)
{
	//Нормализуем вектор направления z
	if(zAxisDirection.Normalize() < 1e-37f || upVector.Normalize() < 1e-37f)
	{
		vx = Vector(1.0f, 0.0f , 0.0f);
		vy = Vector(0.0f, 1.0f , 0.0f);
		vz = Vector(0.0f, 0.0f , 1.0f);
		return false;
	}
	//Считаем
	vx = upVector ^ zAxisDirection;
	if(vx.Normalize() == 0.0f)
	{
		vx = Vector(1.0f, 0.0f , 0.0f);
		vy = Vector(0.0f, 1.0f , 0.0f);
		vz = Vector(0.0f, 0.0f , 1.0f);
		return false;
	}
	vy = zAxisDirection ^ vx;
	vz = zAxisDirection;
	return true;
}

//Посчитать матрицу объекта имея позицию точку куда направлен объект и направление вверх
mathinline bool mathcall Matrix::BuildOriented(Vector position, Vector lookTo, Vector upVector)
{
	//Направление
	lookTo -= position;
	//Нормализуем вектор направления z
	if(lookTo.Normalize() == 0.0f || upVector.Normalize() == 0.0f)
	{
		vx = Vector(1.0f, 0.0f , 0.0f); wx = 0.0f;
		vy = Vector(0.0f, 1.0f , 0.0f); wy = 0.0f;
		vz = Vector(0.0f, 0.0f , 1.0f); wz = 0.0f;
		pos = position; w = 1.0f;
		return false;
	}
	//Считаем
	vx = lookTo ^ upVector; wx = 0.0f;
	if(vx.Normalize() == 0.0f)
	{
		vx = Vector(1.0f, 0.0f , 0.0f); wx = 0.0f;
		vy = Vector(0.0f, 1.0f , 0.0f); wy = 0.0f;
		vz = Vector(0.0f, 0.0f , 1.0f); wz = 0.0f;
		pos = position; w = 1.0f;
		return false;		
	}
	vy = lookTo ^ vx; wy = 0.0f;
	vz = lookTo; wz = 0.0f;
	pos = position; w = 1.0f;
	return true;
}

//Посчитать матрицу для отзеркаливания геометрии
mathinline Matrix & mathcall Matrix::BuildMirror(float Nx, float Ny, float Nz, float D)
{
	m[0][0] = -Nx*2.0f*Nx + 1.0f;
	m[0][1] = -Ny*2.0f*Nx;
	m[0][2] = -Nz*2.0f*Nx;
	m[0][3] = 0.0f;
	m[1][0] = -Nx*2.0f*Ny;
	m[1][1] = -Ny*2.0f*Ny + 1.0f;
	m[1][2] = -Nz*2.0f*Ny;
	m[1][3] = 0.0f;
	m[2][0] = -Nx*2.0f*Nz;
	m[2][1] = -Ny*2.0f*Nz;
	m[2][2] = -Nz*2.0f*Nz + 1.0f;
	m[2][3] = 0.0f;
	m[3][0] = -Nx*2.0f*-D;
	m[3][1] = -Ny*2.0f*-D;
	m[3][2] = -Nz*2.0f*-D;	
	m[3][3] = 1.0f;
	return *this;
}

//-----------------------------------------------------------
//Преобразование матрицы
//-----------------------------------------------------------

//Повернуть вокруг X
mathinline Matrix & mathcall Matrix::RotateX(float ang)
{
	Matrix m;
	m.BuildRotateX(ang);
	EqMultiply(Matrix(*this), m);
	return *this;
}

//Повернуть вокруг Y
mathinline Matrix & mathcall Matrix::RotateY(float ang)
{
	Matrix m;
	m.BuildRotateY(ang);
	EqMultiply(Matrix(*this), m);
	return *this;
}

//Повернуть вокруг Z
mathinline Matrix & mathcall Matrix::RotateZ(float ang)
{
	Matrix m;
	m.BuildRotateZ(ang);
	EqMultiply(Matrix(*this), m);
	return *this;
}

//Повернуть вокруг ZXY
mathinline Matrix & mathcall Matrix::Rotate(float angX, float angY, float angZ)
{
	Matrix m;
	m.Build(angX, angY, angZ);
	EqMultiply(Matrix(*this), m);
	return *this;
}

//Повернуть вокруг ZXY
mathinline Matrix & mathcall Matrix::Rotate(const Vector & ang)
{
	Matrix m;
	m.Build(ang.x, ang.y, ang.z);
	EqMultiply(Matrix(*this), m);
	return *this;
}

//Переместить
mathinline Matrix & mathcall Matrix::Move(float dX, float dY, float dZ)
{
	pos.x += dX;
	pos.y += dY;
	pos.z += dZ;
	return *this;
}

//Переместить
mathinline Matrix & mathcall Matrix::Move(const Vector & pos)
{
	this->pos.x += pos.x;
	this->pos.y += pos.y;
	this->pos.z += pos.z;
	return *this;
}

//Отмасштабировать
mathinline Matrix & mathcall Matrix::Scale(float scale)
{
	Scale(scale, scale, scale);
	return *this;
}

//Отмасштабировать матрицу поворота
mathinline Matrix & mathcall Matrix::Scale3x3(float scale)
{
	Scale3x3(scale, scale, scale);
	return *this;
}

//Отмасштабировать
mathinline Matrix & mathcall Matrix::Scale(float scaleX, float scaleY, float scaleZ)
{
	m[0][0] *= scaleX;
	m[1][0] *= scaleX;
	m[2][0] *= scaleX;
	m[3][0] *= scaleX;
	m[0][1] *= scaleY;
	m[1][1] *= scaleY;
	m[2][1] *= scaleY;
	m[3][1] *= scaleY;
	m[0][2] *= scaleZ;
	m[1][2] *= scaleZ;
	m[2][2] *= scaleZ;
	m[3][2] *= scaleZ;
	return *this;
}

//Отмасштабировать матрицу поворота
mathinline Matrix & mathcall Matrix::Scale3x3(float scaleX, float scaleY, float scaleZ)
{
	m[0][0] *= scaleX;
	m[1][0] *= scaleX;
	m[2][0] *= scaleX;
	m[0][1] *= scaleY;
	m[1][1] *= scaleY;
	m[2][1] *= scaleY;
	m[0][2] *= scaleZ;
	m[1][2] *= scaleZ;
	m[2][2] *= scaleZ;
	return *this;
}

//Отмасштабировать
mathinline Matrix & mathcall Matrix::Scale(const Vector & scale)
{
	Scale(scale.x, scale.y, scale.z);
	return *this;
}

//Отмасштабировать поворота
mathinline Matrix & mathcall Matrix::Scale3x3(const Vector & scale)
{
	Scale3x3(scale.x, scale.y, scale.z);
	return *this;
}


//Расчёт обратной матрицы
mathinline Matrix & mathcall Matrix::Inverse()
{
	pos = Vector(-(pos | vx), -(pos | vy), -(pos | vz));
	Transposition3X3();
	return *this;
}

///Расчёт обратной матрицы из другой
mathinline Matrix & mathcall Matrix::Inverse(const Matrix & mtx)
{
	pos = Vector(-(mtx.pos | mtx.vx), -(mtx.pos | mtx.vy), -(mtx.pos | mtx.vz));

	//Транспонируем 3x3 матрицу
	m[0][0] = mtx.m[0][0];
	m[1][0] = mtx.m[0][1];
	m[2][0] = mtx.m[0][2];

	m[0][1] = mtx.m[1][0];
	m[1][1] = mtx.m[1][1];
	m[2][1] = mtx.m[1][2];

	m[0][2] = mtx.m[2][0];
	m[1][2] = mtx.m[2][1];
	m[2][2] = mtx.m[2][2];

	//w заполняем 
	m[0][3] = 0.0f;  
	m[1][3] = 0.0f;  
	m[2][3] = 0.0f;  
	m[3][3] = 1.0f;  

	return *this;

}

///Полный расчёт обратной матрицы 4x3
mathinline bool mathcall Matrix::InverseComplette()
{
	//Матрица поворота
	float mtmp[3][3];
	mtmp[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
	mtmp[0][1] = m[0][2]*m[2][1] - m[0][1]*m[2][2];
	mtmp[0][2] = m[0][1]*m[1][2] - m[0][2]*m[1][1];
	mtmp[1][0] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
	mtmp[1][1] = m[0][0]*m[2][2] - m[0][2]*m[2][0];
	mtmp[1][2] = m[0][2]*m[1][0] - m[0][0]*m[1][2];
	mtmp[2][0] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
	mtmp[2][1] = m[0][1]*m[2][0] - m[0][0]*m[2][1];
	mtmp[2][2] = m[0][0]*m[1][1] - m[0][1]*m[1][0];
	float det = m[0][0]*mtmp[0][0] + m[0][1]*mtmp[1][0] + m[0][2]*mtmp[2][0];
	if(fabsf(det) > 1e-20f)
	{
		det = 1.0f/det;
		m[0][0] = det*mtmp[0][0];
		m[0][1] = det*mtmp[0][1];
		m[0][2] = det*mtmp[0][2];
		m[1][0] = det*mtmp[1][0];
		m[1][1] = det*mtmp[1][1];
		m[1][2] = det*mtmp[1][2];
		m[2][0] = det*mtmp[2][0];
		m[2][1] = det*mtmp[2][1];
		m[2][2] = det*mtmp[2][2];
	}else{
		SetZero();
		return false;
	}
	//Позиция
	pos = -(MulNormal(pos));
	return true;
}

///Полный расчёт обратной матрицы  4x4
mathinline bool mathcall Matrix::InverseComplette4X4()
{
	float k[12];
	k[0] = m[0][0]*m[1][1] - m[0][1]*m[1][0];
	k[1] = m[0][0]*m[1][2] - m[0][2]*m[1][0];
	k[2] = m[0][0]*m[1][3] - m[0][3]*m[1][0];
	k[3] = m[0][1]*m[1][2] - m[0][2]*m[1][1];
	k[4] = m[0][1]*m[1][3] - m[0][3]*m[1][1];
	k[5] = m[0][2]*m[1][3] - m[0][3]*m[1][2];
	k[6] = m[2][0]*m[3][1] - m[2][1]*m[3][0];
	k[7] = m[2][0]*m[3][2] - m[2][2]*m[3][0];
	k[8] = m[2][0]*m[3][3] - m[2][3]*m[3][0];
	k[9] = m[2][1]*m[3][2] - m[2][2]*m[3][1];
	k[10] = m[2][1]*m[3][3] - m[2][3]*m[3][1];
	k[11] = m[2][2]*m[3][3] - m[2][3]*m[3][2];

	float det = k[0]*k[11] - k[1]*k[10] + k[2]*k[9] + k[3]*k[8] - k[4]*k[7] + k[5]*k[6];

	if(fabsf(det) <= 1e-10f)
	{
		SetZero();
		return false;
	}

	det = 1.0f/det;
		
	Matrix result(false);
	result.m[0][0] = (m[1][1]*k[11] - m[1][2]*k[10] + m[1][3]*k[9])*det;
	result.m[1][0] = (m[1][2]*k[8] - m[1][3]*k[7] - m[1][0]*k[11])*det;
	result.m[2][0] = (m[1][0]*k[10] - m[1][1]*k[8] + m[1][3]*k[6])*det;
	result.m[3][0] = (m[1][1]*k[7] - m[1][2]*k[6] - m[1][0]*k[9])*det;
	result.m[0][1] = (m[0][2]*k[10] - m[0][3]*k[9] - m[0][1]*k[11])*det;
	result.m[1][1] = (m[0][0]*k[11] - m[0][2]*k[8] + m[0][3]*k[7])*det;
	result.m[2][1] = (m[0][1]*k[8] - m[0][3]*k[6] - m[0][0]*k[10])*det;
	result.m[3][1] = (m[0][0]*k[9] - m[0][1]*k[7] + m[0][2]*k[6])*det;
	result.m[0][2] = (m[3][1]*k[5] - m[3][2]*k[4] + m[3][3]*k[3])*det;
	result.m[1][2] = (m[3][2]*k[2] - m[3][3]*k[1] - m[3][0]*k[5])*det;
	result.m[2][2] = (m[3][0]*k[4] - m[3][1]*k[2] + m[3][3]*k[0])*det;
	result.m[3][2] = (m[3][1]*k[1] - m[3][2]*k[0] - m[3][0]*k[3])*det;
	result.m[0][3] = (m[2][2]*k[4] - m[2][3]*k[3] - m[2][1]*k[5])*det;
	result.m[1][3] = (m[2][0]*k[5] - m[2][2]*k[2] + m[2][3]*k[1])*det;
	result.m[2][3] = (m[2][1]*k[2] - m[2][3]*k[0] - m[2][0]*k[4])*det;
	result.m[3][3] = (m[2][0]*k[3] - m[2][1]*k[1] + m[2][2]*k[0])*det;
	*this = result;
	return true;
}

//Транспанирование матрицы
mathinline Matrix & mathcall Matrix::Transposition()
{
	float tmp;
	tmp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = tmp;
	tmp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = tmp;
	tmp = m[0][3]; m[0][3] = m[3][0]; m[3][0] = tmp;
	tmp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = tmp;
	tmp = m[1][3]; m[1][3] = m[3][1]; m[3][1] = tmp;
	tmp = m[2][3]; m[2][3] = m[3][2]; m[3][2] = tmp;
	return *this;


}

//Транспанирование элементов поворота
mathinline Matrix & mathcall Matrix::Transposition3X3()
{
	float tmp;
	tmp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = tmp;
	tmp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = tmp;
	tmp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = tmp;
	return *this;

}


//-----------------------------------------------------------
//Утилитные
//-----------------------------------------------------------

//Считать только вращение
mathinline Matrix & mathcall Matrix::SetRotate(const Matrix & mtx)
{
/*
	_asm
	{
		mov		esi, mtx
		mov		edi, this
		mov		ebx, [esi + 0*4]
		mov		ecx, [esi + 1*4]
		mov		edx, [esi + 2*4]
		mov		[edi + 0*4], ebx
		mov		[edi + 1*4], ecx
		mov		[edi + 2*4], edx
		mov		ebx, [esi + 4*4]
		mov		ecx, [esi + 5*4]
		mov		edx, [esi + 6*4]
		mov		[edi + 4*4], ebx
		mov		[edi + 5*4], ecx
		mov		[edi + 6*4], edx
		mov		ebx, [esi + 8*4]
		mov		ecx, [esi + 9*4]
		mov		edx, [esi + 10*4]
		mov		[edi + 8*4], ebx
		mov		[edi + 9*4], ecx
		mov		[edi + 10*4], edx
	}
	return *this;
*/

	m[0][0] = mtx.m[0][0];
	m[0][1] = mtx.m[0][1];
	m[0][2] = mtx.m[0][2];

	m[1][0] = mtx.m[1][0];
	m[1][1] = mtx.m[1][1];
	m[1][2] = mtx.m[1][2];

	m[2][0] = mtx.m[2][0];
	m[2][1] = mtx.m[2][1];
	m[2][2] = mtx.m[2][2];

	return *this;

}


//Перемножить матрицы и результат поместить в текущую
mathinline Matrix & mathcall Matrix::EqMultiply(const Matrix & m1, const Matrix & m2)
{
	Matrix m;
	m.EqMultiplyFast(m1, m2);
	Set(m);
	return *this;
}
	
//Перемножить матрицы и результат поместить в текущую m1 != this && m2 != this
mathinline Matrix & mathcall Matrix::EqMultiplyFast(const Matrix & m1, const Matrix & m2)
{
	m[0][0] = m2.m[0][0]*m1.m[0][0] + m2.m[1][0]*m1.m[0][1] + m2.m[2][0]*m1.m[0][2] + m2.m[3][0]*m1.m[0][3];
	m[0][1] = m2.m[0][1]*m1.m[0][0] + m2.m[1][1]*m1.m[0][1] + m2.m[2][1]*m1.m[0][2] + m2.m[3][1]*m1.m[0][3];
	m[0][2] = m2.m[0][2]*m1.m[0][0] + m2.m[1][2]*m1.m[0][1] + m2.m[2][2]*m1.m[0][2] + m2.m[3][2]*m1.m[0][3];
	m[0][3] = m2.m[0][3]*m1.m[0][0] + m2.m[1][3]*m1.m[0][1] + m2.m[2][3]*m1.m[0][2] + m2.m[3][3]*m1.m[0][3];
	m[1][0] = m2.m[0][0]*m1.m[1][0] + m2.m[1][0]*m1.m[1][1] + m2.m[2][0]*m1.m[1][2] + m2.m[3][0]*m1.m[1][3];
	m[1][1] = m2.m[0][1]*m1.m[1][0] + m2.m[1][1]*m1.m[1][1] + m2.m[2][1]*m1.m[1][2] + m2.m[3][1]*m1.m[1][3];
	m[1][2] = m2.m[0][2]*m1.m[1][0] + m2.m[1][2]*m1.m[1][1] + m2.m[2][2]*m1.m[1][2] + m2.m[3][2]*m1.m[1][3];
	m[1][3] = m2.m[0][3]*m1.m[1][0] + m2.m[1][3]*m1.m[1][1] + m2.m[2][3]*m1.m[1][2] + m2.m[3][3]*m1.m[1][3];
	m[2][0] = m2.m[0][0]*m1.m[2][0] + m2.m[1][0]*m1.m[2][1] + m2.m[2][0]*m1.m[2][2] + m2.m[3][0]*m1.m[2][3];
	m[2][1] = m2.m[0][1]*m1.m[2][0] + m2.m[1][1]*m1.m[2][1] + m2.m[2][1]*m1.m[2][2] + m2.m[3][1]*m1.m[2][3];
	m[2][2] = m2.m[0][2]*m1.m[2][0] + m2.m[1][2]*m1.m[2][1] + m2.m[2][2]*m1.m[2][2] + m2.m[3][2]*m1.m[2][3];
	m[2][3] = m2.m[0][3]*m1.m[2][0] + m2.m[1][3]*m1.m[2][1] + m2.m[2][3]*m1.m[2][2] + m2.m[3][3]*m1.m[2][3];
	m[3][0] = m2.m[0][0]*m1.m[3][0] + m2.m[1][0]*m1.m[3][1] + m2.m[2][0]*m1.m[3][2] + m2.m[3][0]*m1.m[3][3];
	m[3][1] = m2.m[0][1]*m1.m[3][0] + m2.m[1][1]*m1.m[3][1] + m2.m[2][1]*m1.m[3][2] + m2.m[3][1]*m1.m[3][3];
	m[3][2] = m2.m[0][2]*m1.m[3][0] + m2.m[1][2]*m1.m[3][1] + m2.m[2][2]*m1.m[3][2] + m2.m[3][2]*m1.m[3][3];
	m[3][3] = m2.m[0][3]*m1.m[3][0] + m2.m[1][3]*m1.m[3][1] + m2.m[2][3]*m1.m[3][2] + m2.m[3][3]*m1.m[3][3];
	return *this;
}

//Умножить вершину на матрицу
mathinline Vector mathcall Matrix::MulVertex(const Vector & v) const
{
	Vector tv;
	tv.x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0];
	tv.y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1];
	tv.z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2];	
	return tv;
}

///Умножить вершину на матрицу
mathinline Vector4 mathcall Matrix::MulVertex(const Vector4 & v) const
{
	Vector4 tv;
	tv.x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0]*v.w;
	tv.y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1]*v.w;
	tv.z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2]*v.w;
	tv.w = m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3]*v.w;
	return tv;
}

//Умножить нормаль на матрицу
mathinline Vector mathcall Matrix::MulNormal(const Vector & v) const
{
	Vector tv;
	tv.x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z;
	tv.y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z;
	tv.z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z;
	return tv;
}

//Умножить вершину на инверстую матрицу
mathinline Vector mathcall Matrix::MulVertexByInverse(const Vector & v) const
{
	Vector tv;
	tv.x = m[0][0]*(v.x - m[3][0]) + m[0][1]*(v.y - m[3][1]) + m[0][2]*(v.z - m[3][2]);
	tv.y = m[1][0]*(v.x - m[3][0]) + m[1][1]*(v.y - m[3][1]) + m[1][2]*(v.z - m[3][2]);
	tv.z = m[2][0]*(v.x - m[3][0]) + m[2][1]*(v.y - m[3][1]) + m[2][2]*(v.z - m[3][2]);
	return tv;
}

//Умножить нормаль на инверстую матрицу
mathinline Vector mathcall Matrix::MulNormalByInverse(const Vector & v) const
{
	Vector tv;
	tv.x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z;
	tv.y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z;
	tv.z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z;
	return tv;
}

///Получить позицию камеры из матрицы камеры
mathinline Vector mathcall Matrix::GetCamPos() const
{
	return -MulNormalByInverse(pos);
}

//Единичная матрица или нет
mathinline bool mathcall Matrix::IsIdentity() const
{
	const float eps = 1e-4f;
	if(fabs(m[0][0] - 1.0f) > eps) return false;
	if(fabs(m[0][1] - 0.0f) > eps) return false;
	if(fabs(m[0][2] - 0.0f) > eps) return false;
	if(fabs(m[0][3] - 0.0f) > eps) return false;
	if(fabs(m[1][0] - 0.0f) > eps) return false;
	if(fabs(m[1][1] - 1.0f) > eps) return false;
	if(fabs(m[1][2] - 0.0f) > eps) return false;
	if(fabs(m[1][3] - 0.0f) > eps) return false;
	if(fabs(m[2][0] - 0.0f) > eps) return false;
	if(fabs(m[2][1] - 0.0f) > eps) return false;
	if(fabs(m[2][2] - 1.0f) > eps) return false;
	if(fabs(m[2][3] - 0.0f) > eps) return false;
	if(fabs(m[3][0] - 0.0f) > eps) return false;
	if(fabs(m[3][1] - 0.0f) > eps) return false;
	if(fabs(m[3][2] - 0.0f) > eps) return false;
	if(fabs(m[3][3] - 1.0f) > eps) return false;
	return true;
}

//Скалирования матрица или нет
mathinline bool mathcall Matrix::IsScale() const
{
	const float eps = 1e-3f;
	if(fabsf(~vx - 1.0f) > eps) return true;
	if(fabsf(~vy - 1.0f) > eps) return true;
	if(fabsf(~vz - 1.0f) > eps) return true;
	if(fabsf(w - 1.0f) > eps) return true;
	return false;
}

///Проверить матрицу на инверсное скалирование
mathinline bool mathcall Matrix::IsParity() const
{
	return ((vx ^ vy) | vz) >= 0.0f;
}

//Споецировать вершину (для матрицы проекции)
mathinline Vector4 mathcall Matrix::Projection(const Vector & vertex, float vphWidth05, float vphHeight05) const
{
	Vector4 res;
	//Преобразуем вершину
	res.x = m[0][0]*vertex.x + m[1][0]*vertex.y + m[2][0]*vertex.z + m[3][0];
	res.y = m[0][1]*vertex.x + m[1][1]*vertex.y + m[2][1]*vertex.z + m[3][1];
	res.z = m[0][2]*vertex.x + m[1][2]*vertex.y + m[2][2]*vertex.z + m[3][2];
	res.w = m[0][3]*vertex.x + m[1][3]*vertex.y + m[2][3]*vertex.z + m[3][3];
	//Коэфициент нормализации
	float w = 1.0f/res.w;
	//Нормализуем
	res.x = (1.0f + res.x*w)*vphWidth05;
	res.y = (1.0f - res.y*w)*vphHeight05;
	res.z *= w;
	res.w = w;
	return res;
}

//Споецировать массив вершин (для матрицы проекции)
mathinline void mathcall Matrix::Projection(Vector4 * dstArray, Vector * srcArray, long num, float vphWidth05, float vphHeight05, long srcSize, long dstSize) const
{
	for(; num > 0; num--)
	{
		//Преобразуем вершину
		dstArray->x = m[0][0]*srcArray->x + m[1][0]*srcArray->y + m[2][0]*srcArray->z + m[3][0];
		dstArray->y = m[0][1]*srcArray->x + m[1][1]*srcArray->y + m[2][1]*srcArray->z + m[3][1];
		dstArray->z = m[0][2]*srcArray->x + m[1][2]*srcArray->y + m[2][2]*srcArray->z + m[3][2];
		dstArray->w = m[0][3]*srcArray->x + m[1][3]*srcArray->y + m[2][3]*srcArray->z + m[3][3];
		//Коэфициент нормализации
		float w = 1.0f/dstArray->w;
		//Нормализуем
		dstArray->x = (1.0f + dstArray->x*w)*vphWidth05;
		dstArray->y = (1.0f - dstArray->y*w)*vphHeight05;
		dstArray->z *= w;
		dstArray->w = w;
		//Указатели на следующие вершины
		srcArray = (Vector *)((char *)srcArray + srcSize);
		dstArray = (Vector4 *)((char *)dstArray + dstSize);
	}
}

//Получить углы из нескалированной матрицы поворота
mathinline void mathcall Matrix::GetAngles(float & ax, float & ay, float & az)
{	
	if(vz.y < 1.0f)
	{
		if(vz.y > -1.0f)
		{
			ax = (float)asin(-vz.y);
			ay = (float)atan2(vz.x, vz.z);
			az = (float)atan2(vx.y, vy.y);
			return;
		}else{
			ax = 3.141592654f*0.5f;
			ay = 0.0f;
			az = (float)atan2(vx.z, vx.x);
		}
	}else{
		ax = -3.141592654f*0.5f;
		ay = 0.0f;
		az = (float)-atan2(vx.z, vx.x);
	}
}

//Получить коэфициенты скалирования матрицы
mathinline Vector mathcall Matrix::GetScale() const
{
	return Vector(vx.GetLength(), vy.GetLength(), vz.GetLength());
}

//Нормализация (исключение скалирования)
mathinline Matrix & mathcall Matrix::Normalize()
{
	vx.Normalize();
	vy.Normalize();
	vz.Normalize();
	return *this;
}

//Умножить все элементы на число
mathinline Matrix & mathcall Matrix::Mul(float k)
{
	matrix[0] *= k;  matrix[1] *= k;  matrix[2] *= k;  matrix[3] *= k;
	matrix[4] *= k;  matrix[5] *= k;  matrix[6] *= k;  matrix[7] *= k;
	matrix[8] *= k;  matrix[9] *= k;  matrix[10] *= k; matrix[11] *= k;
	matrix[12] *= k; matrix[13] *= k; matrix[14] *= k; matrix[15] *= k;
	return *this;
}

//Сложить прибавить другую матрицу
mathinline Matrix & mathcall Matrix::Add(const Matrix & m)
{
	matrix[0] += m.matrix[0];   matrix[1] += m.matrix[1];   matrix[2] += m.matrix[2];   matrix[3] += m.matrix[3];
	matrix[4] += m.matrix[4];   matrix[5] += m.matrix[5];   matrix[6] += m.matrix[6];   matrix[7] += m.matrix[7];
	matrix[8] += m.matrix[8];   matrix[9] += m.matrix[9];   matrix[10] += m.matrix[10]; matrix[11] += m.matrix[11];
	matrix[12] += m.matrix[12]; matrix[13] += m.matrix[13]; matrix[14] += m.matrix[14]; matrix[15] += m.matrix[15];
	return *this;
}

//Добавить другую матрицу умноженую на число
mathinline Matrix & mathcall Matrix::Mad(const Matrix & m, float k)
{
	matrix[0] += m.matrix[0]*k;   matrix[1] += m.matrix[1]*k;   matrix[2] += m.matrix[2]*k;   matrix[3] += m.matrix[3]*k;
	matrix[4] += m.matrix[4]*k;   matrix[5] += m.matrix[5]*k;   matrix[6] += m.matrix[6]*k;   matrix[7] += m.matrix[7]*k;
	matrix[8] += m.matrix[8]*k;   matrix[9] += m.matrix[9]*k;   matrix[10] += m.matrix[10]*k; matrix[11] += m.matrix[11]*k;
	matrix[12] += m.matrix[12]*k; matrix[13] += m.matrix[13]*k; matrix[14] += m.matrix[14]*k; matrix[15] += m.matrix[15]*k;
	return *this;
}

//Получить углы из нескалированной матрицы поворота
mathinline void mathcall Matrix::GetAngles(Vector & ang)
{
	GetAngles(ang.x, ang.y, ang.z);
}

//Получить углы из нескалированной матрицы поворота
mathinline Vector mathcall Matrix::GetAngles()
{
	Vector tmp;
	GetAngles(tmp);
	return tmp;
}

//Доступиться до элементов матрицы через скобки
mathinline float & mathcall Matrix::operator () (long i, long j)
{
	return m[i][j];
}

//Получить указатель на матрицу D3D
mathinline Matrix::operator D3DXMATRIX * () const
{
	return ((D3DXMATRIX*)matrix);
}

//Получить вектор для расчёта X компоненты
mathinline Vector4 mathcall Matrix::GetVectorX() const
{
	return Vector4(m[0][0], m[1][0], m[2][0], m[3][0]);
}

//Получить вектор для расчёта Y компоненты
mathinline Vector4 mathcall Matrix::GetVectorY() const
{
	return Vector4(m[0][1], m[1][1], m[2][1], m[3][1]);
}

//Получить вектор для расчёта Z компоненты
mathinline Vector4 mathcall Matrix::GetVectorZ() const
{
	return Vector4(m[0][2], m[1][2], m[2][2], m[3][2]);
}

//Получить вектор для расчёта W компоненты
mathinline Vector4 mathcall Matrix::GetVectorW() const
{
	return Vector4(m[0][3], m[1][3], m[2][3], m[3][3]);
}

mathinline void mathcall Matrix::MultiplySIMD_4x4(float * res, const float * m1, const float * m2)
{
#ifdef SIMD_ENABLE
#ifndef _XBOX
	//Загружаем первую матрицу
	__m128 m2_c0 = _mm_load_ps(&m2[0]);
	__m128 m2_c1 = _mm_load_ps(&m2[4]);
	__m128 m2_c2 = _mm_load_ps(&m2[8]);
	__m128 m2_c3 = _mm_load_ps(&m2[12]);
	__m128 tmp, acc;
	//vx
	tmp = _mm_load_ps1(&m1[0]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[1]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[2]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	tmp = _mm_load_ps1(&m1[3]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c3, tmp));
	_mm_store_ps(&res[0], acc);
	//vy
	tmp = _mm_load_ps1(&m1[4]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[5]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[6]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	tmp = _mm_load_ps1(&m1[7]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c3, tmp));
	_mm_store_ps(&res[4], acc);
	//vz
	tmp = _mm_load_ps1(&m1[8]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[9]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[10]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	tmp = _mm_load_ps1(&m1[11]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c3, tmp));
	_mm_store_ps(&res[8], acc);
	//pos
	tmp = _mm_load_ps1(&m1[12]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[13]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[14]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	tmp = _mm_load_ps1(&m1[15]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c3, tmp));
	_mm_store_ps(&res[12], acc);
#else
	*(XMMATRIX *)res = XMMatrixMultiply(*(XMMATRIX *)m1, *(XMMATRIX *)m2);
#endif
#else //!SIMD
	if(res != m1 && res != m2)
	{
		((Matrix *)res)->EqMultiplyFast(*(Matrix *)m1, *(Matrix *)m2);
	}else{
		((Matrix *)res)->EqMultiply(*(Matrix *)m1, *(Matrix *)m2);
	}
#endif
}

mathinline void mathcall Matrix::MultiplySIMD_4x3(float * res, const float * m1, const float * m2)
{
#ifdef SIMD_ENABLE
#ifndef _XBOX
	//Загружаем первую матрицу
	__m128 m2_c0 = _mm_load_ps(&m2[0]);
	__m128 m2_c1 = _mm_load_ps(&m2[4]);
	__m128 m2_c2 = _mm_load_ps(&m2[8]);
	__m128 m2_c3 = _mm_load_ps(&m2[12]);
	__m128 tmp, acc;
	//vx
	tmp = _mm_load_ps1(&m1[0]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[1]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[2]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	_mm_store_ps(&res[0], acc);
	//vy
	tmp = _mm_load_ps1(&m1[4]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[5]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[6]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	_mm_store_ps(&res[4], acc);
	//vz
	tmp = _mm_load_ps1(&m1[8]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[9]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[10]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	_mm_store_ps(&res[8], acc);
	//pos
	tmp = _mm_load_ps1(&m1[12]);
	acc = _mm_mul_ps(m2_c0, tmp);
	tmp = _mm_load_ps1(&m1[13]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c1, tmp));
	tmp = _mm_load_ps1(&m1[14]);
	acc = _mm_add_ps(acc, _mm_mul_ps(m2_c2, tmp));
	acc = _mm_add_ps(acc, m2_c3);
	_mm_store_ps(&res[12], acc);
#else
	*(XMMATRIX *)res = XMMatrixMultiply(*(XMMATRIX *)m1, *(XMMATRIX *)m2);
#endif
#else //!SIMD
	if(res != m1 && res != m2)
	{
		((Matrix *)res)->EqMultiplyFast(*(Matrix *)m1, *(Matrix *)m2);
	}else{
		((Matrix *)res)->EqMultiply(*(Matrix *)m1, *(Matrix *)m2);
	}
#endif
}


#endif
