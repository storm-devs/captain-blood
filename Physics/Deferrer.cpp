// Пул памяти для отложенных вызовов

#include "common.h"
#include "Deferrer.h"

MemPool * MemPool::ptr = null;

MemPool::MemPool()
{
#ifndef STOP_DEBUG
	numBlocks = 196 * (MAX_MCALLS + 1);
#else
	numBlocks = 196 * 6;
#endif
	chunkSize = 80;
	
	buffer = NEW char[numBlocks * chunkSize];
	list = NEW word[numBlocks];

	list[0] = 0xFFFF;
	for (dword i=1; i<numBlocks; i++) list[i] = i - 1;
	lastIndex = numBlocks - 1;
}

MemPool::~MemPool()
{
	DELETE(buffer);
	DELETE(list);
}

void MemPool::Reset()
{
}

void * MemPool::Alloc(dword allocSize)
{
	// отдаем память из пулла если есть еще свободные блоки
	if (lastIndex != 0xFFFF && allocSize <= chunkSize)
	{
		void * ptr = (void*)&buffer[lastIndex * chunkSize];
		lastIndex = list[lastIndex];
		return ptr;
	}

	// Если память кончилась или размер большой то выделяем через api
	return api->Reallocate(null, allocSize, _FL_);
}

void MemPool::Free(void * ptr)
{
	// Если поинтер был выделен с помощью api, освобождаем его
	if ((char*)ptr < buffer || (char*)ptr >= (buffer + numBlocks * chunkSize))
	{
		api->Free(ptr, _FL_);
		return;
	}

	// освобождаем блок
	dword idx = dword((char*)ptr - buffer) / chunkSize;
	list[idx] = lastIndex;
	lastIndex = word(idx);
}

#ifndef STOP_DEBUG
void AddFloat(string & str, float value)
{
	char res[MAX_PATH];
	sprintf_s(res, sizeof(res), "%f", value);
	// удаляем лишние нули
	dword len = strlen(res) - 1;
	while (true)
	{
		if (len < 4) break;
		if (res[len - 1] == '.') break;
		if (res[len] != '0') break;
		len--;
	}
	res[len] = 0;

	str += res;
};

void DeferrerTrace(string & str, dword value)
{
	str += value;
}

void DeferrerTrace(string & str, long value)
{
	str += value;
}

void DeferrerTrace(string & str, int value)
{
	str += long(value);
}

void DeferrerTrace(string & str, float value)
{
	AddFloat(str, value);
}

void DeferrerTrace(string & str, const Vector & value)
{
	str += "Vector(";
	AddFloat(str, value.x);	str += ", ";	
	AddFloat(str, value.y);	str += ", ";	
	AddFloat(str, value.z);	str += ")";	
}

void DeferrerTrace(string & str, const Matrix & mtx)
{
	str += "Matrix(";
	string tmp;
	tmp.Format("vx = (%f, %f, %f; %f), ", mtx.vx.x, mtx.vx.y, mtx.vx.z, mtx.wx);
	str += tmp;
	tmp.Format("vy = (%f, %f, %f; %f), ", mtx.vy.x, mtx.vy.y, mtx.vy.z, mtx.wy);
	str += tmp;
	tmp.Format("vz = (%f, %f, %f; %f), ", mtx.vz.x, mtx.vz.y, mtx.vz.z, mtx.wz);
	str += tmp;
	tmp.Format("pos = (%f, %f, %f; %f)", mtx.pos.x, mtx.pos.y, mtx.pos.z, mtx.w);
	str += tmp;	str += ")";	
}

void DeferrerTrace(string & str, const Vector4 & value)
{
	str += "Vector4(";
	AddFloat(str, value.x);	str += ", ";	
	AddFloat(str, value.y);	str += ", ";	
	AddFloat(str, value.z);	str += ", ";	
	AddFloat(str, value.w);	str += ")";	
}

void DeferrerTrace(string & str, bool value)
{
	str += (value) ? "true" : "false";
}

void DeferrerTrace(string & str, const Plane & value)
{
	str += "Plane(";
	AddFloat(str, value.n.x); str += ", ";	
	AddFloat(str, value.n.y); str += ", ";	
	AddFloat(str, value.n.z); str += ", ";
	AddFloat(str, value.d); str += ")";
}

void DeferrerTrace(string & str, PhysicsCollisionGroup value)
{
	str += dword(value);
}

void DeferrerTrace(string & str, IPhysMaterial * material)
{
	str += "IPhysMaterial";
}

void DeferrerTrace(string & str, IAnimation * animation)
{
	str += "IAnimation";
}

void DeferrerTrace(string & str, IPhysRigidBody & body)
{
	str += "IPhysRigidBody";
}
#endif