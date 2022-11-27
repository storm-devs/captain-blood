#ifndef GMX_SUBSET_INTERFACE
#define GMX_SUBSET_INTERFACE

class IGMXMaterial;


class IPhysTriangleMesh;


//[0][0] , [1][0], [2][0], [3][0]
//[0][1] , [1][1], [2][1], [3][1]
//alpha  , scale,  resevd, [3][2]

//Vector4(mFinal.vx.x, mFinal.vy.x, mFinal.vz.x, mFinal.pos.x)
//Vector4(mFinal.vx.y, mFinal.vy.y, mFinal.vz.y, mFinal.pos.y)
//Vector4(fAlpha, fScale, 0.0f, mFinal.pos.z)

struct gpuBoneData
{

/*
	union
	{
		struct
		{
			float vx_x;
			float vy_x;
			float vz_x;
			float pos_x;

			float vx_y;
			float vy_y;
			float vz_y;
			float pos_y;

			float fAlpha;
			float fScale;
			float unsued_empty;
			float pos_z;
		};

		///Представление в виде массива
		Vector4 data[3];
	};
*/

	Vector4 data[3];

};


//176 байт!!! (48 байт лишних !!!)
struct GMXBone
{
	//!!!MUST BE ALIGNED by 16
	//64 bytes
	Matrix mInitial;    //(!!!!!!!!!!! №1)

	//!!!MUST BE ALIGNED by 16
	//16
	Vector4 bbSphereCenterWorld;  //(!!!!!!!!!!! №5)

	//16
	Vector4 bbMinWorld;//(!!!!!!!!!!! №7)

	//16
	Vector4 bbMaxWorld; //(!!!!!!!!!!! №6)


	//12
	Vector bbMin;  //(!!!!!!!!!!! №3)
	//4
	const char* Name;

	//12
	Vector bbMax;  //(!!!!!!!!!!! №4)
	//4
	unsigned long NameHash;

	//-----------------------------------------
	//12 bytes
	Vector bbSphereCenter; //(!!!!!!!!!!! №2)

	//4 bytes
	float bbSphereRadius;
	

	//!!!MUST BE ALIGNED by 16
	//48 bytes
	gpuBoneData gpuData; //(!!!!!!!!!!! №8)
};




class IGMXSubset
{

public:

	IGMXSubset() {}

	virtual ~IGMXSubset() {};


	//virtual IGMXMaterial* GetMaterial () = 0;

	//virtual bool IsAnimated () = 0;

	//virtual void* LockVB (dword dwOffsetToLock = 0, dword dwSizeToLock = 0, dword Flags = 0) = 0;
	//virtual word* LockIB (dword dwOffsetToLock = 0, dword dwSizeToLock = 0, dword Flags = 0) = 0;
	//virtual void UnlockVB () = 0;
	//virtual void UnlockIB () = 0;


	//virtual dword GetVertexCount() = 0;
	//virtual dword GetTriangleCount() = 0;


	//virtual IPhysTriangleMesh* GetCollider () = 0;


	//получить кол-во костей
	//virtual dword GetBoneCount () = 0;

	//Получить кость по индексу
	//virtual const GMXBone*  GetBoneByIndex (dword dwIndex) = 0;

	//Получить индекс кости родителя, если нет родителя вернет -1
	//virtual long GetBoneParentIndex (dword dwIndex) = 0;

};

/*
__forceinline Vector GMX_UnpackNormal (dword packed_normal)
{
	byte bt_x = (byte)((packed_normal >> 16) & 0xFF);
	byte bt_y = (byte)((packed_normal >> 8) & 0xFF);
	byte bt_z = (byte)((packed_normal) & 0xFF);

	Vector Normal;
	Normal.x = (((float)bt_x / 255.0f) * 2.0f) - 1.0f;
	Normal.y = (((float)bt_y / 255.0f) * 2.0f) - 1.0f;
	Normal.z = (((float)bt_z / 255.0f) * 2.0f) - 1.0f;

	return Normal;
}

__forceinline float GMX_UnpackTexCoord (short packed_u_or_v)
{
	float TexCoord = (packed_u_or_v / 32767.5f) * 12.0f;
	return TexCoord;
}
*/


#endif

