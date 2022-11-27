//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Box
//============================================================================================

#ifndef _Box_h_
#define _Box_h_


#include "Vector.h"

///Класс представления ориентированного по осям ящика
class Box
{
public:
	union
	{
		struct
		{
			///Позиция центра
			Vector p;
		};
		struct
		{
			///Позиция центра
			Vector pos;
		};
		struct
		{
			///Позиция центра
			Vector c;
		};
		struct
		{
			///Позиция центра
			Vector center;
		};
	};
	union
	{
		struct
		{
			///Размер
			Vector s;
		};
		struct
		{
			///Размер
			Vector size;
		};
	};

	//Empty constructor
	Box();
	//Copy constructor
	Box(const Box& b);
	//Assign
	Box& operator=(const Box& b);
	//Проверить пересечение сферы и ящика
	static bool OverlapsBoxSphere(const Matrix & mtx, const Vector & size05, const Vector & center, float radius);
	//Проверить пересечение сферы и линии
	static bool OverlapsBoxLine(const Matrix & mtx, const Vector & size05, const Vector & v1, const Vector & v2);
	//Проверить пересечение четырёхугольника и ящика
	static bool OverlapsBoxPoly(const Matrix & mtx, const Vector & size05, const Vector vp[4]);
	//Получить ABB для OBB
	static void FindABBforOBB(const Matrix & mtx, const Vector & min, const Vector & max, Vector & abbMin, Vector & abbMax, bool isAdd = false);

};


//Empty constructor
__forceinline Box::Box() {}

//Copy constructor
__forceinline Box::Box(const Box& b)
{
	center = b.center;
	size = b.size;
}

//Assign
__forceinline Box& Box::operator=(const Box& b)
{
	center = b.center;
	size = b.size;
	return *this;
}

__forceinline bool Box::OverlapsBoxSphere(const Matrix & mtx, const Vector & size05, const Vector & center, float radius)
{
	Vector c = mtx.MulVertexByInverse(center);
	float dist = 0.0f;
	if(c.x < -size05.x) dist += (c.x - (-size05.x))*(c.x - (-size05.x));
	if(c.x > size05.x) dist += (c.x - size05.x)*(c.x - size05.x);
	if(c.y < -size05.y) dist += (c.y - (-size05.y))*(c.y - (-size05.y));
	if(c.y > size05.y) dist += (c.y - size05.y)*(c.y - size05.y);
	if(c.z < -size05.z) dist += (c.z - (-size05.z))*(c.z - (-size05.z));
	if(c.z > size05.z) dist += (c.z - size05.z)*(c.z - size05.z);
	return dist <= radius*radius;
}

__forceinline bool Box::OverlapsBoxLine(const Matrix & mtx, const Vector & size05, const Vector & v1, const Vector & v2)
{	
	Vector mid = mtx.MulVertexByInverse((v1 + v2)*0.5f);
	Vector dir = mtx.MulNormalByInverse((v2 - v1)*0.5f);
	//Проверяем AABB-AABB (первые 3 оси)
	Vector adir(dir); adir.Abs();
	if(fabs(mid.x) > size05.x + adir.x ||
		fabs(mid.y) > size05.y + adir.y ||
		fabs(mid.z) > size05.z + adir.z) return false;
	//Проверяем X ^ dir
	if (fabs(mid.y*dir.z - mid.z*dir.y) > size05.y*adir.z + size05.z*adir.y) return false;
	//Проверяем Y ^ dir
	if(fabs(mid.z*dir.x - mid.x*dir.z) > size05.x*adir.z + size05.z*adir.x) return false;
	//Проверяем Z ^ dir
	if(fabs(mid.x*dir.y - mid.y*dir.x) > size05.x*adir.y + size05.y*adir.x) return false;
	return true;
}

__forceinline bool Box::OverlapsBoxPoly(const Matrix & mtx, const Vector & size05, const Vector vp[4])
{
	//Преобразуем четырехугольник в локальную систему для AABB теста и найдём AABB четырехугольника
	Vector v[4];
	Vector min, max;
	for(long i = 0; i < 4; i++)
	{
		v[i] = mtx.MulVertexByInverse(vp[i]);
		if(i)
		{
			min.Min(v[i]);
			max.Max(v[i]);
		}else{
			min = max = v[i];
		}
	}
	//Проверяем AABB-AABB (первые 3 оси)
	if(min.x > size05.x || min.y > size05.y || min.z > size05.z) return false;
	if(max.x < -size05.x || max.y < -size05.y || max.z < -size05.z) return false;
	//Проверяем плоскость четырехугольника с AABB
	Vector edge[4];
	edge[0] = v[1] - v[0];
	edge[1] = v[2] - v[1];
	Vector norm = edge[0] ^ edge[1];
	if(fabsf(norm | v[0]) > (norm.Abs() | size05)) return false;
	edge[2] = v[3] - v[2];
	edge[3] = v[0] - v[3];
	//Проверяем коллижен по дополнительным осям
	static Vector basis[3] = {Vector(1.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Vector(0.0f, 0.0f, 1.0f)};	
	for(long i = 0; i < 3; i++)
	{
		for(long j = 0; j < 4; j++)
		{
			Vector axis = basis[i] ^ edge[j];
			float dmin, dmax;
			dmin = dmax = v[0] | axis;
			for(long k = 1; k < 4; k++)
			{
				float dist = v[k] | axis;
				if(dist < dmin) dmin = dist;
				if(dist > dmax) dmax = dist;
			}
			float ext = axis.Abs() | size05;
			if(dmin > ext) return false;
			if(dmax < -ext) return false;
		}
	}
	return true;
}

__forceinline void Box::FindABBforOBB(const Matrix & mtx, const Vector & min, const Vector & max, Vector & abbMin, Vector & abbMax, bool isAdd)
{
	Vector exts = (max - min)*0.5f;

	Vector gexts;
	gexts.x = fabsf(mtx.vx.x*exts.x) + fabsf(mtx.vy.x*exts.y) + fabsf(mtx.vz.x*exts.z);
	gexts.y = fabsf(mtx.vx.y*exts.x) + fabsf(mtx.vy.y*exts.y) + fabsf(mtx.vz.y*exts.z);
	gexts.z = fabsf(mtx.vx.z*exts.x) + fabsf(mtx.vy.z*exts.y) + fabsf(mtx.vz.z*exts.z);

	Vector cnt = mtx*((min + max)*0.5f);

	if(isAdd)
	{
		Vector t = cnt - gexts;
		abbMin.Min(t);
		t = cnt + gexts;
		abbMax.Max(t);
	}else{
		abbMin = cnt - gexts;
		abbMax = cnt + gexts;
	}
}

#endif