#ifndef AIPATH_CLASS_H
#define AIPATH_CLASS_H

#include "..\Mission.h"

class IPath
{
protected:
	IPath() {};
	virtual ~IPath() {};

public:
	// релизит путь
	virtual bool Release() = null;

	// возвратить начальную точку отрезка пути
	virtual const Vector & Pnt0() const = null;
	// возвратить конечную точку отрезка пути
	virtual const Vector & Pnt1() const = null;
	// возвратить текущую точку где находится(если двигаемся по пути с помошью Move())
	virtual const Vector & Curr() const = null;
	// возвратить точку откуда начали искать путь
	virtual const Vector & From() const = null;
	// возвратить точку до куда начали искать путь
	virtual const Vector & To() const = null;

	// true если путь закончился
	virtual bool IsDone() const = null;
	// возвращает true при первом вызове после того как путь стартовал
	// потом сбрасывает флажок и возвращает false
	virtual bool IsStarted() = null;
	// возвращает true если путь стартовал и готов к использованию
	virtual bool IsReady() const = null;
	// возвращает true если если мы находимся на последнем отрезке
	virtual bool IsLast() const = null;
	// возвращает true если путь непроходимый
	virtual bool IsImpassable() const = null; 

	// перейти к следующему отрезку, если есть такая возможность
	// pnt1 становится pnt0, выбирается новый pnt1
	// возвращает true - если новый отрезок нашелся, false - если тупик или конец пути
	// эта функция сбрасывает указатель текущего положения в начальную точку отрезка
	virtual bool Next() = null;

	// Перемещает указатель по пути на расстояние fDistance
	// возвращает true - если прошел дистанцию и есть еще куда идти, false - если дошел до конца или уперся в тупик
	virtual bool Move(float fDistance) = null;

	// возвращает кол-во точек в пути
	virtual int GetNumPoints() const = null;
};


//***************************************************************************************************
//
// Путь возвращаемый системой поиска путей по графу
// фактически просто массив из векторов
//
//***************************************************************************************************

class Path
{
	array<Vector> Points;
public:

	Path(): Points(_FL_) {};
	~Path() { Points.DelAll(); };

	void clear ()
	{
		Points.DelAll();
	}

	dword size () const
	{
		return Points.Size();
	};

	dword last () const
	{
		if (Points.Size() == 0) return 0;

		return Points.Size()-1;
	};

	const Vector& getPoint (int index) const
	{
		return Points[index];
	};

	void add_to_end (const Vector& v)
	{
		Points.Add(v);
	}

	void add_to_begin (const Vector& v)
	{
		Points.Insert(v, 0);
	}


	void remove_from_begin ()
	{
		Points.DelIndex(0);
	}

	void remove_from_end ()
	{
		Points.DelIndex((Points.Size()-1));
	}

	void remove (dword dwIndex)
	{
		Points.DelIndex(dwIndex);
	};


	const Vector& operator [] (dword dwIndex)
	{
		return getPoint(dwIndex);
	};
};


#endif