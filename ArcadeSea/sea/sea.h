// ************************************************************************
// *  Author : Vano, 1861; Description : Sea/Ocean/Water/Lake/River/pool  *
// ************************************************************************
#ifndef NEW_SEA_HPP
#define NEW_SEA_HPP

#include "..\..\common_h\mission.h"

#include "Intel.h"

class SEA : public MissionObject
{
	/*
private:

	IVariable* fs_foamTexture;
	IVariable* fs_bumpTexture;
	IVariable* fs_reflTexture;
	IVariable* fs_refrTexture;
	IVariable* fs_bumpFrame;
	IVariable* fFlatSeaBumpScale;

	IVariable* fs_vClearColor;
	IVariable* fs_vColor;
	IVariable* fs_vSubsurfaceColor;
	IVariable* fs_SpecularPower;
	IVariable* fs_vUnderWaterColor;
	IVariable* fs_vSpecularColor;
	IVariable* fs_FoamScale;
	IVariable* fs_flatSeaTexture;

	IVariable* fs_TextureProjMatrix;
	
	
	
	float fFlatSeaLength;

	bool bFlatSea;
	


	ITexture* pReflection;
	ISurface* pReflectionSurface;
	ISurface* pReflectionSurfaceDepth;
	ITexture* pRefraction;
	ISurface* pRefractionSurface;
	IAnimTexture* pBumpTexture;
	IBaseTexture* pFoamTexture;



	float fFoamScale;
	float fSpecularPower;
	Color vUnderWaterColor;
	Color vSeaColor;
	Color vClearColor;
	Color vSeaSubsurfaceColor;
	Color vSeaSpecularColor;





  struct SeaVertex
  {
    Vector   vPos;
    dword   dwPackedNormal;
  };

  struct SeaBlock
  {
    long iX1, iX2, iY1, iY2;      // result rectangle(in units)
    long iSize0;            // 

    long iTX, iTY;
    long iSize;
    long iLOD;
    long iIStart;
    long iIFirst, iILast;

    bool bInProgress, bDone;

    static bool QSort(const SeaBlock & b1, const SeaBlock & b2) { return (b1.iLOD > b2.iLOD); }
    inline bool operator < (const SeaBlock & B) const { return (B.iLOD < iLOD); };
  };

  static SEA    * pSea;

  dword     dwMaxDim, dwMinDim;
  float     fMaxSeaHeight;
  float     fGridStep;
  float     fLodScale;
  float     fBumpScale, fBumpSpeed;
  Vector     vMoveSpeed1, vMoveSpeed2;

  Vector     vCamPos, vSeaCenterPos;

  static bool   bIntel, bSSE, bDisableSSE;

  IVBuffer *pVSeaBuffer;
	IIBuffer* pISeaBuffer;

	IVolumeTexture* pVolumeTexture;

  SeaVertex * pVSea;
  word    * pTriangles;
  dword   * pIndices;
  long    iVStart, iTStart, iIStart;

  Plane   * pFrustumPlanes;
  dword   dwNumFrustumPlanes;

  array<dword*>   aNormals;
  array<byte*>    aBumps;
  array<SeaBlock>   aBlocks;

  Vector4    v4SeaColor, v4SkyColor, v4SeaParameters;

  float * pSeaFrame1, * pSeaFrame2, * pSeaNormalsFrame1, * pSeaNormalsFrame2;

  Vector vMove1, vMove2;
  float fFrame1, fFrame2;
  float fScale1, fScale2;
  float fAmp1, fAmp2;
  float _fAmp1, _fAmp2;
  float fAnimSpeed1, fAnimSpeed2;
  float fPosShift;
  float fFrenel;

  long iB1, iB2;

  float fFogSeaDensity, fFogStartDistance;
  Vector vFogColor;
  bool  bFogEnable;

  bool  bTempFullMode;
  float fTempGridStep, fTempLodScale;

#ifndef _XBOX 
  void        SSE_WaveXZ(SeaVertex * * pArray);
#endif



	float __fastcall  WaveXZ(float x, float z, Vector * pNormal = null);


  void      AddBlock(long iTX, long iTY, long iSize, long iLOD);
  void      BuildTree(long iTX, long iTY, long iLev);
  void      SetBlock(dword dwBlockIndex);

  bool      isVisibleBBox(const Vector & vCenter, const Vector & v1, const Vector & v2);
  void      CalculateLOD(const Vector & v1, const Vector & v2, long & iMaxLOD, long & iMinLOD);
  inline float  CalcLod(const float & x, const float & y, const float & z);
  void      WaveXZBlock(SeaBlock * pB);

#ifndef _XBOX 
	void      SSE_WaveXZBlock(SeaBlock * pB);
#endif




  SeaBlock *    GetUndoneBlock();
  void      PrepareIndicesForBlock(dword dwBlockIndex);

  long      VisCode(const Vector & vP);

  void      CalculateHeightMap(float fFrame, float fAmplitude, float * pfOut, array<byte*> & aFrames);
  void      CalculateNormalMap(float fFrame, float fAmplitude, float * pfOut, array<dword*> & aFrames);


  // HyperThreading 
  Intel       intel;
  HANDLE        hEventCalcMaps;
  array<HANDLE>   aEventCalcBlock;
  array<long>     aThreadsTest;
  array<HANDLE>   aThreads;
  bool        bHyperThreading;
  CRITICAL_SECTION  cs, cs1;
  long        iBlocksDoneNum;

  static dword  ThreadExecute(long iThreadIndex);

	bool bRefraction;
	bool bShowTexture;


	float fSmallBumpScale;
	float fSeaHeight;

	void SetBumpTexture (const char* szTextureName);
	void SetFoamTexture (const char* szTextureName);
	

	static dword _fastcall SEA::PackNormal (const Vector& n);
	static float _fastcall SEA::UnpackNormalX (DWORD packed_normal);
	static float _fastcall SEA::UnpackNormalY (DWORD packed_normal);
	static float _fastcall SEA::UnpackNormalZ (DWORD packed_normal);


public:

  SEA();
  virtual ~SEA();

  bool Create(MOPReader & reader);
  void _cdecl Realize(float fDeltaTime, long level);

  float Trace(const Vector & vSrc, const Vector & vDst);


  void InitParams();

	void BuildEnvironmentTextureMaps ();

	bool EditMode_Update(MOPReader & reader);


  //bool      Init();
  //void      Realize(dword Delta_Time);
  //void      Execute(dword Delta_Time);
  //dword _cdecl  ProcessMessage(MESSAGE & message);
  */
public:
	SEA(){};
	virtual ~SEA(){};
};

#endif