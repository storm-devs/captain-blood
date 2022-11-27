#pragma once

class IPhysicsScene;

class IPhysBase : public Object
{
	friend class PhysicsScene;
protected:
	IPhysBase(IPhysicsScene * _scene)
	{
		scene = _scene;
		userPointer = 0;
		userNumber = 0;
		iRefCount = 1;
		m_fileName = null;
		m_fileLine = -1;
		m_physRealObject = null;
		m_proxyObject = null;

		contactReportFlags = -1;
	};

	virtual ~IPhysBase()
	{
		UnregistryPhysObject();
	};

public:
	__forceinline void * UserPointer() { return userPointer; };
	__forceinline void SetUserPointer(void * ptr) 
	{ 
		if (m_physRealObject)
			m_physRealObject->SetUserPointer(ptr);
		userPointer = ptr; 
	};
	__forceinline long UserNumber() { return userNumber; };
	__forceinline void SetUserNumber(long v) 
	{ 
		if (m_physRealObject)
			m_physRealObject->SetUserNumber(v);
		userNumber = v; 
	};	
	
	__forceinline const char * GetFileName() { return m_fileName; }
	__forceinline long GetFileLine() { return m_fileLine; }

private:
	dword contactReportFlags;
	Vector contactReportPoint;
	Vector contactReportForce;

public:
	// Если было коллижен в этом кадре, то возвращает номер материала об который ударились
	// Если не было столкновения за этот кадр, то возвращает -1
	__forceinline dword GetContactReport() const { return contactReportFlags; }
	// Возвращает точку контакта для последнего контакта
	__forceinline const Vector & GetContactPoint() const { return contactReportPoint; }
	// Возвращает силу которая была приложена к объектам что не допустить
	__forceinline const Vector & GetContactForce() const { return contactReportForce; }

	__forceinline void SetContactReport(dword flags) { contactReportFlags = flags; }
	__forceinline void SetContactReportPoint(const Vector & point) { contactReportPoint = point; }
	__forceinline void SetContactReportForce(const Vector & force) { contactReportForce = force; }

public:
	const char * m_fileName;
	long m_fileLine;

	virtual void AddRef() { iRefCount++; }
	virtual bool DecRef() 
	{
		iRefCount--;

		if (iRefCount <= 0)
		{
			ForceRelease();
			return true;
		}
		return false;
	}

	virtual bool Release() 
	{
		return DecRef();
	}

	// 
	__forceinline void SetFileLine(const char * filename, long fileline)
	{
		if (m_physRealObject)
			m_physRealObject->SetFileLine(filename, fileline);

		m_fileName = filename;
		m_fileLine = fileline;
	}

	__forceinline IPhysBase * GetProxyObject() { return m_proxyObject; }
	__forceinline IPhysBase * GetRealObject() { return m_physRealObject; }

protected:
	IPhysBase * m_physRealObject;	// если this == proxy, то тут будет находиться реальный объект
	IPhysBase * m_proxyObject;		// если this == realPhysXObject, то тут будет находиться поинтер на прокси объект
	IPhysicsScene * scene;
	void * userPointer;
	long userNumber;
	long iRefCount;
	
	// удалить объект
	virtual void ForceRelease() { delete this; }

	// Нормализовать физический объект, чтобы матрицы не искажались
	// возвращает 1 если нормализация была проведена, 0 если не была
	virtual int Normalize() { return 0; };

	// Установить связь данного прокси объекта с реальным phys объектом
	void SetRealPhysObject(IPhysBase * physRealObject)
	{
		Assert(physRealObject);

		m_physRealObject = physRealObject;
		
		m_physRealObject->SetUserNumber(UserNumber());
		m_physRealObject->SetUserPointer(UserPointer());
		m_physRealObject->SetFileLine(m_fileName, m_fileLine);
	}

	void SetProxyObject(IPhysBase * proxyObject) { m_proxyObject = proxyObject;	}

private:
	void UnregistryPhysObject();
};