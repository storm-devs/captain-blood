// ===========================================================
// Vano
// Advanced ocean
// ===========================================================

#pragma once

#include "..\..\Common_h\Mission.h"
#include "..\..\common_h\ishadows.h"
#include "..\..\common_h\IOceans.h"
#include "..\..\common_h\SetThreadName.h"
#include "fft.h"

#define OCEAN_GROUP3		GroupId('O','C','N','3')

#ifdef _XBOX
	#define DW_F_2W(b, a)	dword(dword(a) | (dword(b) << 16L))
#else
	#define DW_F_2W(a, b)	dword(dword(a) | (dword(b) << 16L))
#endif

struct dcomplex
{
	complex c1, c2;
};

class Ocean2;
class Ocean3Executor;

class Ocean3Service : public IOcean3Service 
{
public:
	Ocean3Service();
	virtual ~Ocean3Service();

	virtual bool Init();

	struct SeaVertex
	{
		Vector	pos;
		dword	nrm;
		//dword	color;
		//float	nx, nz;
		//Vector4	norm;
	};

	IRender * render;
	IFileService * files;

	CritSection		m_CalculateFrameSection;

	ITexture		* pFrenelTexture;
	IBaseTexture	* pFoamTexture;
	IVariable	* pVarOceanRT;
	IVariable	* pVarNMap1, * pVarNMap2;
	IVariable	* pVarBumpMulMove;
	IVariable	* pVarBumpMatrix;
	IVariable	* pVarShifts;
	IVariable	* pVarSkyColor, * pVarWaterColor, * pVarTranslucenceColor;
	IVariable	* pVarTranslucenceAngle, * pVarTranslucenceParams;
	IVariable	* pVarFrenelTex;
	IVariable	* pVarReflTex;
	IVariable	* pVarRefrTex;
	IVariable	* pVarBumpTex;
	IVariable	* pVarBumpPower;
	IVariable	* pVarSphereMapTexture, * pVarFoamTexture;
	IVariable	* pVarDistFade;
	IVariable	* pVarReflTrans;
	IVariable	* pVarRefrParams, * pVarRefrParams2;
	IVariable	* pVarDepthTexture;
	IVariable	* pVarFlatMapping;
	IVariable	* pVarUV05;
	IVariable	* pVarRefrMapping;
	IVariable	* pVarSunRoadParams, * pVarSunRoadColor;
	IVariable	* pVarSunLight, * pVarSunLightParams;
	
	//IVariable	* pVarSunroadEnable, * pVarTranslucenceEnable, * pVarFoamEnable;
	IVariable	* pVarBooleanParams;


	IVariable	* pVarViewport;
	IVariable	* pVarCamPos;
	IVariable	* pVarSunBumpScale;
	IVariable	* pVarBumpDistance;
	IVariable	* pVarDMapTexture;
	IVariable	* pVarFoamParams1;
	IVariable	* pVarFoamParams2;
	IVariable	* pVarFoamColor;
	IVariable	* pVarFoamRotate;
	IVariable	* pVarFogParams, * pVarFogColor;
	IVariable	* pVarFakeHeight;

	IRenderTarget		* pOceanRT;
	IRenderTargetDepth	* pOceanRTDepth;

	IRenderTarget		* pReflectionTexture, * pRefractRT;
	IRenderTarget		* pReflectionTexture2;
	IRenderTarget		* sphereRT;
	//IRenderTargetDepth	* p512RTDepth;
	IRenderTargetDepth	* pReflectionSurfaceDepth, * pRefractRTDepth;

	ShaderId		m_oceanID;
	ShaderId		m_alphaRefractID, m_shaderClearAlpha;
	ShaderId		m_zPrePassID, m_zPrePassSetAlphaID;
	ShaderId		m_oceanShaderID;
	ShaderId		m_waterShaderID;
	ShaderId		m_mirrorShaderID;
	ShaderId		m_badFlatWaterShaderID;
	ShaderId		m_clearRTDepthID;

	ShaderId		m_flatNoZID;

	ShaderId		SkyDome_id;
	IVariable		* varSkyDomeTexture;
	IVariable		* varSkyDomeBools;

	IVBuffer		* m_halfScreenCopyVB;
	IVBuffer		* m_verts2D;
	IIBuffer		* m_indexes2D;

	IVBuffer		* m_vSeaBuffer;
	IIBuffer		* m_iSeaBuffer;

	long			m_oceanRTWidth, m_oceanRTHeight;
	//TEMPO
	IRenderTarget	* faceRT;
	//TEMPO

	// Reflection/Refraction render targets size
	int				rr_size;

	IShadowsService		* pShdServ;

	int			m_fftWavesSize;
	fft			fftWaves, fftBump;

	IMission	* m_sphereMapMission;
	//bool		m_isFirstRealize;
	bool		m_regenerateSphereMap;

	bool		m_isOceansHaveFakeHeight;

	dword		* m_indices;

	virtual void StartFrame(float deltaTime);

	void GenerateBumpMap();
	
	IBaseTexture * GetBumpTexture();

private:
	bool			m_bumpAlreadyGenerated;
	int				m_curBuffer, m_curTexture;
	IVBuffer		* m_vSeaBufferTemp[2];
	IIBuffer		* m_iSeaBufferTemp[2];

	ITexture		* m_normTex2;
	ITexture		* m_normTex2Temp[3];

	void Error(long id, const char * errorEnglish);

	void  __declspec(dllexport) InitProtectData(dword dwVal);
};

class Ocean3Refractor : public MissionObject
{
public:
	Ocean3Refractor();
	virtual ~Ocean3Refractor();

	// Создание объекта
	virtual bool Create(MOPReader & reader);
	// Выполнение объекта для отрисовки отражения
	void _cdecl Reflecting(float deltaTime, long level);
	// Выполнение объекта для отрисовки преломления
	bool _cdecl Refracting(float deltaTime, long level);

	// увеличить счетчик океанов
	void AddRef();
	// уменьшить счетчик океанов
	void DecRef();

	// отрисовать отражение в текстуру
	void MakeReflection(IRenderTarget * color, IRenderTargetDepth * depth, float height, bool reflectParticles, bool restoreContext);

private:
	Ocean3Service * m_oceans;
	MOSafePointerType<Ocean3Executor> m_executor;

	long		m_refCount;
	MGIterator	* m_refractorIterator;
	MGIterator	* m_reflectorIterator;
	MGIterator	* m_executorIterator;
	bool		m_reflectAlreadyCleared;
	bool		m_refractAlreadyCleared;
};

class Ocean3Executor : public MissionObject
{
public:
	Ocean3Executor();
	virtual ~Ocean3Executor();

	//Создание объекта
	virtual bool Create(MOPReader & reader);

	void _cdecl Execute(float fDeltaTime, long level);
	void _cdecl Realize(float fDeltaTime, long level);

	// увеличить счетчик океанов
	void AddRef();
	// уменьшить счетчик океанов
	void DecRef(Ocean2 * ocean);

	// ждет окончания работы треда, если тот был запущен
	void StopWorking();

	// добавляем новый океан на обработку(надо делать каждый кадр)
	void AddToExecute(Ocean2 * ocean);

	Ocean2 * GetBumpOcean() const { return m_bumpOcean; }

	void Refracting();

private:
	Ocean3Service * m_oceans;

	class ThreadInfo
	{
	public:
		HANDLE	thread;
		HANDLE	startEvent, doneEvent, exitEvent;

	public:
		ThreadInfo() 
		{
			thread = null;
			startEvent = null;
			doneEvent = null;
			exitEvent = null;
		}
		~ThreadInfo() 
		{
		}
			
		void Create(void * executor, const char * threadName, LPTHREAD_START_ROUTINE threadProc, dword x360CoreNumber, int x360ThreadPriority = THREAD_PRIORITY_NORMAL)
		{
			DWORD dwThreadID = 0;
			thread = CreateThread(NULL, 0, threadProc, executor, CREATE_SUSPENDED, &dwThreadID); Assert(thread);
			startEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	Assert(startEvent);
			doneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	Assert(doneEvent);
			exitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	Assert(exitEvent);
	
			#ifdef _XBOX
				::XSetThreadProcessor(thread, x360CoreNumber);
				::SetThreadPriority(thread, x360ThreadPriority);
			#endif
			
			XSetThreadName(dwThreadID, threadName);//"Ocean::Execute");
			ResumeThread(thread);
		}

		void Start()
		{
			SetEvent(startEvent);
		}

		bool Synchronize()
		{
			for (int i=0; i<5; i++)
			{
				if ( WaitForSingleObject(doneEvent, 2000) == WAIT_OBJECT_0 ) 
					return true;

				api->Trace("Ocean3Executor::StopWorking : doneEvent waiting timeout!");
			}

			return false;
		}

		void Release()
		{
			SetEvent(exitEvent);
			for (int i=0; i<5; i++)
			{
				if ( WaitForSingleObject(thread, 2000) == WAIT_OBJECT_0 ) 
					break;

				api->Trace("Ocean3Executor: Killing thread problem!");
			}

			CloseHandle(thread);
			CloseHandle(startEvent);
			CloseHandle(doneEvent);
			CloseHandle(exitEvent);
		}
	};

	RENDERVIEWPORT	oldViewport;
	RENDERSCREEN	screenInfo;

	HANDLE		m_startWavesEvent;
	ThreadInfo	m_threadWaves, m_threadBump, m_threadWavesFFT, m_threadBumpFFT;

	bool		firstStart;
	float		m_fftDeltaTime;

	bool		m_threadStarted;
	bool		m_isMultiThreading;

	Ocean2		* m_bumpOcean;

	array<Ocean2*>	m_oceansToExecute;
	
	MOSafePointerType<Ocean3Refractor> m_refractor;

	long		m_refCount;
	MGIterator	* m_executorIterator;
	MGIterator  * m_reflectorIterator;

#ifdef _XBOX
	ShaderId	X360RestoreEDRAM_id, X360RestoreEDRAM_ColorOnly_id;
	IVariable	* pEDRAM_Color, * pEDRAM_Depth;
#else
	ShaderId	PCRestoreColor_id;
	IVariable	* varRestoreColorParams, * varRestoreColorTexture;
#endif

	bool __forceinline IsEditModeOn();

	static dword __stdcall WorkThreadWavesFFT(LPVOID lpParameter);
	static dword __stdcall WorkThreadBumpFFT(LPVOID lpParameter);

	static dword __stdcall WorkThreadWaves(LPVOID lpParameter);
	static dword __stdcall WorkThreadBump(LPVOID lpParameter);

	void ZPrePass(bool setAlpha);
};

class Ocean2 : public MissionObject
{
public:
	static float fGridStep;
	static int dwMaxDim;
	static int dwMinDim;

	//Конструктор - деструктор
	Ocean2();
	virtual ~Ocean2();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Активирование/деактивирование объекта
	virtual void Activate(bool isActive);
	//Включение/выключение объекта
	virtual void Show(bool isShow);
	//
	MO_IS_FUNCTION(Ocean, MissionObject);

	virtual void Command(const char * id, dword numParams, const char * * params);

	void UnlockBuffers();

	bool IsWorking(bool visibleCheck = false) const;
	bool IsOceanOrWatersVisible() const;
	bool IsBigOceanActive() const { return (m_oceanEnable && !m_offBigOcean); }
	bool IsMainFFTSource() const { return m_mainFFTSource; }

	float GetHeight() const { return m_seaHeight; }

	bool IsRealWavesEnable() const { return m_oceanEnableRealWaves; }
	bool IsReflectParticles() const { return m_reflectParticles; }
	bool IsMainReflection() const { return m_mainReflectionHeight; }
	bool IsAllMirrors() const;

	// выполняет рассчет лодов/индексов данного океана - WaveXZ вызывать нельзя тут (старые данные будут)!
	void WorkThreadPreFFT();

	// выполняет рассчет данного океана и его вод
	void WorkThread();

	// рисуем реальные поверхности океана и речек если они есть
	void DrawRealOceanAndParts(float deltaTime, const RENDERVIEWPORT & oldVP);
	// рисуем плоскость океана и плоскости речек если они есть
	void DrawFlatOceanAndParts(bool bNoZ = false, bool alphaRefractPass = false, bool zPrePass = false, bool zPrePassSetAlpha = false);

	// создает бамповую текстуру с мипами из fft 
	void GenerateBumpTexture();

	// Устанавливает параметры данного океана в FFT для волн и бампа
	void SetupFFT();

	float GetWaveSpeed() const { return wave_spd; }
	float GetBumpSpeed() const { return wave_spd2; }

private:
	void GenerateSphereMap();

	void _cdecl FakeFill(float fDeltaTime, long level);
	void _cdecl First(float fDeltaTime, long level);
	//void _cdecl MakeReflection(float fDeltaTime, long level);
	void _cdecl ClearScreen(float fDeltaTime, long level);
	void _cdecl Realize(float fDeltaTime, long level);
	void _cdecl Execute(float fDeltaTime, long level);
#ifndef STOP_DEBUG
	void _cdecl DebugRealize(float fDeltaTime, long level);
#endif

#ifdef _XBOX
	enum WaveXZRegistersSet
	{
		vamp = 64,
		vscale,
		vmove,
		v0011, 
		vxwidth, 
		vmask_y, 
		vfftsize, 
		vxzero,
		vpermControl0127, 
		vpermControl_VXVZ_XZ,
		v_05_4096,
		v_shift_normals,
		vmask1000, 
		vmask1010,
		vx_shift_y_fftSize,
		valpha,

		WaveXZRegistersSet_forcedword = 0x7FFFFFFF
	};

	struct WaveXZIntegerStore
	{
		Vector4 v4;				// vx, vz, x, z
		int		i4[4];			// i1, i2, i3, i4
	};
#else
#endif
	Ocean3Service * m_oceans;

	Color	m_skyColor;
	Color	m_waterColor;

	bool	m_hasOwnReflection;
	bool	m_mainReflectionHeight;
	bool	m_reflectParticles;
	float	m_ReflectionMultiply, m_ReflectionPower;
	float	m_ReflectionMin, m_ReflectionMax;
	float	m_RefractionPower, m_ReflectionEnvPower;
	float	m_TransparencyMin, m_TransparencyMax;
	float	m_ReflectionMinimum, m_ReflectionBlendSkyObj;

	bool	m_oceanEnable;

	float	m_bumpFrame;
	float	m_bumpPower;

	float	m_bumpPosU, m_bumpPosV;
	float	m_bumpAngle, m_bumpMoveSpeed, m_bumpScaleU, m_bumpScaleV;
	float	m_initialBumpScaleU, m_initialBumpScaleV;

	float	m_normalDisplacement;

	MGIterator	* pReflIterator;

	dword	m_valpha;

	struct WaterVector 
	{
		Vector v;
		dword color;
	};

	struct WaterPart
	{
		IVBuffer * vBuffer[2];
		IVBuffer * vBufferFlat;						// плоские треугольники
		IIBuffer * iBuffer;
		int curBuffer;
		WaterVector * verts;
		Ocean3Service::SeaVertex * vsea;				// Locked VBuffer
		long numParts;
		long numVerts;
		long numTrgs;
		long numUPoints;
		Vector boxMin, boxMax;
		float bumpFade;
		ProfileTimer ticks;
		bool isVisible;
		bool isBad;
		bool isMirror;

		WaterPart()
		{
			vBufferFlat = null;
			vBuffer[0] = null;
			vBuffer[1] = null;
			iBuffer = null;
			verts = null;
			vsea = null;
			isVisible = true;
			isMirror = false;
		}

		~WaterPart()
		{
			RELEASE(vBufferFlat);
			RELEASE(vBuffer[0]);
			RELEASE(vBuffer[1]);
			RELEASE(iBuffer);
			DELETE(verts);
		}
	};

	array<WaterPart>		m_waterParts;

	Ocean3Service::SeaVertex	* m_pVSea;

	bool			m_mainFFTSource;

	bool			m_sunRoadEnable;
	Vector4			m_sunRoadParams;	// xyz = angle, w = power
	Color			m_sunRoadColor;
	Vector4			m_sunLight, m_sunLightParams;

	Vector4			m_refractionParams, m_refractionParams2;

	bool			m_translucenceEnable;
	Vector4			m_translucenceLight;
	Vector4			m_translucenceParams;
	Color			m_translucenceColor;

	float			m_oceanAnimate;

	float			m_sunBumpScale;
	long			m_quantity;
	bool			isNoSwing;
	float			m_deltaTime;
	float			m_seaHeight, m_seaFakeHeight;
	float			m_posShift;
	word			* m_pTriangles;
	Matrix			m_View;					// текущая view матрица
	Vector			m_camPos, m_camDir;		// позиция и направление камеры

	bool			m_oceanEnableRealWaves;
	float			m_wavesScale, m_wavesAmplitude;
	float			m_maxWavesDistance;

	Vector			m_wavesMove, m_wavesMoveSpeed;

	long			m_numVerts;

	static bool		m_bDebugDraw;
	static int		m_iDebugDraw;
	static int		m_iNumReflections;

	bool			m_offBigOcean;

	IRenderTarget		* m_depthTex;
	IRenderTargetDepth	* m_depthStencil;

	ProfileTimer	m_oceanTime;

	bool	m_visibleAnyWater;
	bool	m_foamEnable;
	Color	m_foamColor;
	Vector	m_foamAngles;
	float	m_foamStartY, m_foamHeightY, m_foamPowerY;
	float	m_foamUVScale;
	float	m_foamU, m_foamV;
	float	m_foamDisplacement;
	Vector	m_foamMoveAngles;
	float	m_foamMoveSpeed;

	fft::dcomplex	* m_fftCurFrame;

	float	m_fogMultiply, m_fogDistance, m_fogStart;
	Color	m_fogColor;

	bool m_referenceExecRefr;
	MOSafePointerType<Ocean3Executor> m_executor;
	MOSafePointerType<Ocean3Refractor> m_refractor;

	//С кого считывать углы поворота по Y
	MOSafePointer connectToPtr;
	Matrix m_connectMtx;
	ConstString connectToName;

	float wave_len, wave_amp, wind_pwr, wave_drx, wave_drz, wave_frm, wave_spd;
	float wave_len2, wave_amp2, wind_pwr2, wave_drx2, wave_drz2, wave_frm2, wave_spd2;

	//
	struct SeaBlock
	{
		int iX1, iX2, iY1, iY2;			// result rectangle(in units)
		int iSize0;						// 

		int iTX, iTY;
		int iSize;
		int iLOD;
		dword iIStart, VStart;

		bool bInProgress, bDone;

		static bool QSort(const SeaBlock & b1, const SeaBlock & b2) { return (b1.iLOD > b2.iLOD); }
		inline bool operator < (const SeaBlock & B) const { return (B.iLOD < iLOD); };
	};


	array<SeaBlock>	m_blocks;
	array<Vector4>	m_flatBlocks;		// vector4 = { x1, y1, x2, y2 } 
	float fLodScale;
	int m_IStart, m_VStart, m_TStart;
	dword * pTriangles;
	Vector vSeaCenterPos;
	bool isVisibleBBox(const Vector & vSeaCenterPos, const Vector & v1, const Vector & v2);
	float CalcLod(const float & x, const float & y, const float & z);
	void CalculateLOD(const Vector & v1, const Vector & v2, int & iMaxLOD, int & iMinLOD);
	void BuildTree(int iTX, int iTY, int iLev);
	void AddBlock(int iTX, int iTY, int iSize, int iLOD);
	void PrepareIndicesForBlock(int blockIdx);
	void WaveXZBlock_PC(SeaBlock & block);

#ifdef _XBOX
	// рассчитать int координаты вертекса
	void __forceinline CalcWaveXZCoords(__vector4 vin, WaveXZIntegerStore * __restrict res);
	// рассчитать вертекс водной поверхности
	void __forceinline CalcWaveYNormal(__vector4 vin, WaveXZIntegerStore * __restrict data, Ocean3Service::SeaVertex * __restrict result);
	void WaveXZBlock_x360(SeaBlock & block);
#endif
	
	void WaveXZ(const Vector4 & v, Ocean3Service::SeaVertex * __restrict res);

	dword m_numFrustumPlanes;
	Plane m_frustumPlanes[6];

	dword m_totalRDTSC;
	dword m_totalWaveXZRDTSC, m_totalWaveXZVerts;

	bool BoxIsVisible(const Plane * frustum, const Vector & vMin, const Vector & vMax);
	void CubeMap_GetSideMatrix(long face, Matrix & view);
};