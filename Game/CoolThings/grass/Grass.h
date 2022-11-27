#pragma once

#include "..\..\..\Common_h\Mission.h"

#define GRASS_VERSION	0x04015247

#define MAX_GRASSES		29000
#define MAX_INDICES		10000
#define MAX_BLOCKS		68				// 65 минимум, потому что в .w сохраняется 126 параметров

// структура grs файла
// размер		описание
//	4			версия
//	4=n			кол-во блоков 4x4
//	4*n			смещения блоков 4x4
//	[ начинается массив блоков 4x4
//	12			позиция центра 4x4
//  4			радиус блока 4x4
//  4=k			кол-во блоков 2x2
//  4*k			смещения блоков 2x2
//		[ начинается массив блоков 2x2
//	12			bbox_min, bbox_max
//  4=z			кол-во травяных блоков 
//  4*z			с+мещения травяных блоков 
//		[ начинается массив травяных блоков 
//	4			два short - x_offset, z_offset (0, 1)
//	4=j			кол-во травинок dword(lod3, lod2, lod1, 0)
//	4*j			травинки: альфа(6 бит номер матрицы поворота травинки, 2 бита текстура), остальные 3 байта - x,y,z

class Grass;
class IShadowsService;

class GrassHolder
{
public:
	GrassHolder();
	~GrassHolder();

	void AddRef();
	void Release();

	struct Vertex
	{
		dword pos;
		dword color;
	};

	ShaderLightingId	shaderGrassLighted, shaderGrassLightedNS;
	ShaderId			shaderGrass, shaderGrassNS, shaderShadowReceive;

	IVariable	* varGrassTemp;
	IVariable	* varGrassTexPatches;
	IVariable	* varGrassPatchesParams;
	IVariable	* varPosVectors;
	IVariable	* varWindParams1, * varWindParams2;
	IVariable	* varGrassColor;
	IVariable	* varGrassDistances;
	IVariable	* varShadowMap;
	IVariable	* varGrassAngles;
	IVariable	* varGrassPlayer;
	IVariable	* varShadowReceive;

	IVariable	* varGrassTexture;

	Matrix		mtxNormals[21];
#if !defined(NO_TOOLS) && !defined(_XBOX)
	Vector		vecNormals[21];
#endif
private:
	int m_ref;

	void Init();
};

class GrassExecutor : public MissionObject
{
public:
	enum WorkResult
	{
		GRASS_OK = 1,
		GRASS_OVERFLOW = 200,
		GRASS_FORCEDWORD = 0x7FFFFFFF
	};

	struct DrawPacket
	{
		Grass	* grass;
		dword	curBlock;
		dword	startVertex, numRects;
		Vector	obbMin, obbMax;
		Vector4	posVectors[MAX_BLOCKS * 2];
		bool	used;
	};
	
	array<DrawPacket>	drawPackets;

	GrassExecutor();
	virtual ~GrassExecutor();

	virtual bool Create(MOPReader & reader);

	void Add2Execute(Grass * grass);
	void RealizeGrass(Grass * grass);

	int m_curVBuffer;
	IVBuffer * m_vBuffers[2];
	IIBuffer * m_iBuffer;
	IShadowsService	* m_shadowService;

private:
	GrassHolder::Vertex * m_vertices;
	Grass * m_lastGrass;
	Matrix m_view;
	Vector m_camPos;
	Vector4 m_playerPos;
	static Plane m_frustumPlanes[6];
	array<Grass*> m_grassesToExecute;
	bool m_threadStarted;
	bool m_isMultiThreading;
	HANDLE m_hThread;
	HANDLE m_hStartEvent, m_hDoneEvent, m_hExitEvent;

	static dword __stdcall WorkThread(LPVOID lpParameter);
	void _cdecl Execute(float fDeltaTime, long level);
	void _cdecl Synchronize(float fDeltaTime, long level);
	void _cdecl DebugRealize(float fDeltaTime, long level);
};

class Grass : public MissionObject
{
public:
	static GrassHolder	GH;
	static int m_totalBlocks, m_totalDraws, m_totalGrasses;

	Grass();
	virtual ~Grass();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void Show(bool isShow);

	GrassExecutor::WorkResult _cdecl Work(GrassHolder::Vertex * __restrict vertices, const Vector & camPos, const Plane * frustum);

	void DrawPacket(GrassExecutor::DrawPacket & packet);

	void PrepareForRender(const Matrix & view);

	void _cdecl Execute(float fDeltaTime, long level);
	void _cdecl Realize(float fDeltaTime, long level);
	void _cdecl ShadowReceive(const char * group, MissionObject * sender);
	void _cdecl SeaRefraction(const char * group, MissionObject * sender);

	virtual void GetBox(Vector & min, Vector & max);

private:
	enum 
	{
		vperm0167 = 64,
		vabs,
		vfrustum1,
		vfrustum2,
		vfrustum3,
		vfrustum4,
		vforce_dword = 0x7FFFFFFF
	};

	struct tb44
	{
		Vector center;
		float radius;
		dword numBlocks;
		dword offsets[16];
	};

	struct tb22
	{
		union {
			dword dw_minmax[3];	// shorts minx, miny, minz, maxx, maxy, maxz
			short sh_minmax[6];	
		};
		dword numBlocks;
		dword offsets[4];
	};

	struct LodsNum
	{
		union 
		{
			dword	dw_tmp;
			byte	lods[4];
		};
	};

	struct tb
	{
		union {
			dword dw_shifts;	// x offset, z offset (0, 1)
			short sh_shifts[2];	
		};
		union {
			dword	numGrasses;
			byte	lods[4];
		};
		dword grasses[64];
	};

	char			m_grassPath[MAX_PATH];
	ILoadBuffer		* m_loadDataBuffer;

	MOSafePointerType<GrassExecutor> m_executor;

	dword		i_last, j_last, e_last;

	IVBuffer	* m_vBParameters;

	int			m_prevTotalGrasses;

	bool		m_dynamicLighting, m_shadowCast, m_shadowReceive, m_noSwing;
	bool		m_seaReflection, m_seaRefraction;
	bool		m_drawPatch;
	bool		m_randomPosition;
	bool		m_startedGeneration;
	bool		m_regenerateGrass;
	bool		m_shadowRecieveMode;
	float		m_density;
	float		m_grassSize, m_grassRandSize;
	float		m_useNormals;
	int			m_randomGenerator;
	static int	m_grassDebugLine;
	GMXBoundBox m_bbox;
	Vector4		m_grassAngles;
	Vector4		m_texPatches[4];
	Vector4		m_texPatchesParams[4];
	float		m_windPowerMultiplicator[4];
	Color		m_grassColor;
	int			m_level;

	Vector		m_totalObbMin, m_totalObbMax;

	float		m_grassMaxDistance, m_grassTransparencyDistance;

	Vector		m_windAngles;
	float		m_windPower;
	float		m_windColorAmbient, m_windColorPower;
	float		m_time;

	IBaseTexture	* m_grassTexture;

	static Vector	m_camPos;
	static dword	m_numB44;
	static const dword *	m_b44Offset;

	void LoadDataFile();

	bool SphereIsVisibleSquare(const Plane * frustum, const Vector4 & SphereCenter, float SphereRadius);
	dword __forceinline FastRand(dword & seed);
	
	GrassExecutor::DrawPacket * GetDrawPacket();
	void AddDrawPacket(dword i, dword j, dword e, GrassExecutor::DrawPacket * packet);

	static bool QSortBlocks44(const dword & v1, const dword & v2);

#if !defined(NO_TOOLS) && !defined(_XBOX)
	struct Grasses
	{
		Vector pos;
		int	mtxIndex;
	};

 	struct Block
	{
		array<Grasses>	lod1, lod2, lod3;
		array<dword>	data;
		Vector			bbox_min, bbox_max;
		int				x_offset, z_offset;

		Block() : lod1(_FL_, 48), lod2(_FL_, 12), lod3(_FL_, 4), data(_FL_, 1024)
		{
		}
	};
	IGMXScene		* m_geo;
	IPhysRigidBody	* m_rigidBody;
	array<dword>	m_mainData;
	string			m_patchName;
	GMXOBB			m_obb;
	static int		m_lodselector[8][8];
	int				m_b44x, m_b44z;
	int				m_blockx, m_blockz;
	int				m_grassx, m_grassz;
	int				m_current44;
	int				m_numGrasses;
	int				m_blockNumber, m_processBlockNumber;
	float			m_probabilities[4];

	static bool		m_currentGeneratingGrass;

	struct Block22
	{
		array<Block>	blocks;
		array<dword>	data;
		Vector			bbox_min, bbox_max;

		Block22() : blocks(_FL_, 16), data(_FL_, 1024)
		{
		}
	};

	struct Block44
	{
		array<Block22>	blocks22;
		array<dword>	data;
		Vector			bbox_min, bbox_max;

		Block44() : blocks22(_FL_, 16), data(_FL_, 1024)
		{
		}
	};

	array<Block44>	m_blocks44;

	virtual bool EditMode_Export();

	void PrepareGrass(bool isExport = false);
	void ProcessBlock4x4();
	bool ProcessBlock2x2(Block44 & b44, int bx, int bz);
	bool ProcessBlock(Block22 & b22, int bx, int bz, int gx, int gz, int x_offset, int z_offset);
	void PackBlock4x4(Block44 & b44);
	void PackBlock2x2(Block22 & block22);
	void PackBlock(Block & block, const Vector & b22min, const Vector & b22max);
	void PackLod(Block & block, array<Grasses> & lod, const Vector & b22min, const Vector & b22max);

#endif

};

