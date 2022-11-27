#ifndef GMX_MESH_INTERFACE
#define GMX_MESH_INTERFACE

class IGMXSubset;


enum GMXVERTEXFORMAT
{
	GMXVF_STATIC = 0,
	GMXVF_ANIMATED,

	GMXVF_FORCE_DWORD = 0x7fffffff
};

class IGMXMesh
{

protected:

	virtual ~IGMXMesh() {};

public:

	IGMXMesh() {};


	//virtual void Draw (dword dwSubsetID) = 0;

	//virtual bool Release () = 0;
	//virtual void AddRef () = 0;

	//virtual dword GetSubsetsCount() = 0;
	//virtual IGMXSubset* GetSubset(dword dwIndex) = 0;

	//virtual GMXVERTEXFORMAT GetVertexFormat () = 0;

	//virtual dword GetNumBytesPerVertex() = 0;
	//virtual dword Stride() = 0;


};

#endif

