//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Capsule
//============================================================================================

#ifndef _Capsule_h_
#define _Capsule_h_

//Вертикально расположенная капсула, находящеяся в 0
class Capsule
{
public:
	Capsule();
	Capsule(float s, float r);

	//Установить радиус сфер и цилиндра
	void mathcall SetRadius(float r);
	//Получить радиус сфер и цилиндра
	float mathcall GetRadius();
	//Установить растояние между сферами
	void mathcall SetSize(float s);
	//Получить растояние между сферами
	float mathcall GetSize();
	//Получить размер описывающего ящика
	Vector mathcall GetBoxExtends();
	//Получить параметры выталкивания из треугольника, возвращает количество контактов 0, 1, 2
	long mathcall Extraction(const Triangle & t, Vector extPoint[2], Vector extVector[2]);

protected:
	float radius;
	float size05;
	Vector up, down;
};

mathinline Capsule::Capsule(){};

mathinline Capsule::Capsule(float s, float r)
{
	SetSize(s);
	SetRadius(r);
};

//Установить радиус сфер и цилиндра
mathinline void mathcall Capsule::SetRadius(float r)
{
	radius = r;
}

//Получить радиус сфер и цилиндра
mathinline float mathcall Capsule::GetRadius()
{
	return radius;
}

//Установить растояние между сферами
mathinline void mathcall Capsule::SetSize(float s)
{
	size05 = s*0.5f;
	up = Vector(0.0f, size05, 0.0f);
	down = Vector(0.0f, -size05, 0.0f);
}

//Получить растояние между сферами
mathinline float mathcall Capsule::GetSize()
{
	return size05*2.0f;
}

//Получить размер описывающего ящика
mathinline Vector mathcall Capsule::GetBoxExtends()
{
	return Vector(radius, size05 + radius, radius);
}

//Получить параметры выталкивания из треугольника, возвращает количество контактов 0, 1, 2
mathinline long mathcall Capsule::Extraction(const Triangle & t, Vector extPoint[2], Vector extVector[2])
{
	//Определим пересечение с плоскостью
	Plane plane = t.GetPlane();
	if(plane.d > 0.0f)
	{
		//Центр капсулы позади плоскости
		return 0;
	}
	float dUp = plane.n.y*size05 - plane.d;
	float dDown = plane.n.y*-size05 - plane.d;
	if(dUp >= radius && dDown >= radius)
	{
		//Капсула не пересекается с плоскостью
		return 0;
	}
	//Определим самую нижнию точку пересечения
	Vector pointOnPlane;
	const Vector * nearest;
	float k;
	if(fabsf(dUp - dDown) < 1e-4f)
	{
		//Цилиндр паралелен плоскости
		long count = 0;
		//Для верхней точки
		pointOnPlane = plane.n*-dUp;
		pointOnPlane.y += size05;		
		k = radius - dUp;
		if(k >= 0.0f)
		{		
			if(t.FindClosestPoint(plane.n, pointOnPlane))
			{
				extVector[count] = plane.n*k;
				extPoint[count] = pointOnPlane - extVector[count];
				count++;
			}
		}
		//Для нижней точки
		pointOnPlane = plane.n*-dDown;
		pointOnPlane.y -= size05;
		k = radius - dDown;
		if(k >= 0.0f)
		{		
			if(t.FindClosestPoint(plane.n, pointOnPlane))
			{
				extVector[count] = plane.n*k;
				extPoint[count] = pointOnPlane - extVector[count];
				count++;
			}
		}
		return count;
	}else
	if(dUp < dDown)
	{
		pointOnPlane = plane.n*-dUp;
		pointOnPlane.y += size05;
		nearest = &up;
		k = radius - dUp;
	}else{
		pointOnPlane = plane.n*-dDown;
		pointOnPlane.y -= size05;
		nearest = &down;
		k = radius - dDown;
	}
	//Ищим ближайшую точку в треугольнике
	if(t.FindClosestPoint(plane.n, pointOnPlane))
	{
		//Точка пересечения лежит внутри треугольника
		extVector[0] = plane.n*k;
		extPoint[0] = pointOnPlane - extVector[0];
		return 1;
	}
	//Смещаем точку на максимальную глубину + 1
	const float safeDist = 1.0f + (size05 + radius)*2.0f;
	Vector rayOrigin = pointOnPlane - plane.n*(k + safeDist);
	//Пускаем луч из найденой точки в направлении капсулы и ищем пересечение
	//Для начала решаем задачу для бесконечного вертикального цилиндра в 0
	//Направление на цилиндр
	Vector dir2D = plane.n.GetXZ();
	double in2D = ~dir2D;
	if(in2D > 1e-20f)
	{
		//Нормализуем;
		dir2D *= (in2D = 1.0f/sqrt(in2D));
		//Луч наклонный
		float distToOrtoPlane = -(rayOrigin | dir2D);
		float distToEdge2 = radius*radius - (rayOrigin.GetLengthXZ2() - distToOrtoPlane*distToOrtoPlane);
		if(distToEdge2 <= 0.0f)
		{
			//Капсула не пересекается с треугольником
			return 0;
		}
		float distToCylinder = distToOrtoPlane - sqrtf(distToEdge2);
		//Зная расстояние до цилиндра найдём высоту пересечения
		float y;
		if(distToCylinder > 0.0f)
		{
			//Точка внутри цилиндра
			y = 0.0f;
		}else{
			y = float(plane.n.y*in2D*distToCylinder);
		}
		//Определимся с попаданием
		if(rayOrigin.y + y > size05)
		{
			//Тестим с верхней сферой
			float dist;
			if(Sphere::Intersection(rayOrigin, plane.n, up, radius, &dist))
			{
				k = k + safeDist - dist;
			}else{
				return 0;
			}
		}else
		if(rayOrigin.y + y < -size05)
		{
			//Тестим с нижней сферой
			float dist;
			if(Sphere::Intersection(rayOrigin, plane.n, down, radius, &dist))
			{
				k = k + safeDist - dist;
			}else{
				return 0;
			}
		}else{		
			//Пересеклись с цилиндром
			k = k + safeDist - sqrtf(distToCylinder*distToCylinder + y*y);
		}
	}else{
		//Луч вертикальный
		float dist;
		if(Sphere::Intersection(rayOrigin, plane.n, *nearest, radius, &dist))
		{
			k = k + safeDist - dist;
		}else{
			return 0;
		}
	}
	if(k <= 0.0f)
	{
		return 0;
	}
	extVector[0] = plane.n*k;
	extPoint[0] = pointOnPlane - extVector[0];
	return 1;
}

#endif

