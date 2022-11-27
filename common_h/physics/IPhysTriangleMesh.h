

class IPhysics;

class IPhysTriangleMesh : public Object
{
protected:
	long refCount;

	IPhysTriangleMesh(IPhysics * srv)
	{
		refCount = 1;
		physics = srv;
	};
	virtual ~IPhysTriangleMesh()
	{
		UnregistryPhysTriangleMesh();
	};
	friend class IPhysics;
	void UnregistryPhysTriangleMesh();
	IPhysics * physics;
public:
	void AddRef()
	{
		refCount++;
	}

	void Release()
	{
		refCount--;
		if (refCount <= 0)
			delete this;
	}

};