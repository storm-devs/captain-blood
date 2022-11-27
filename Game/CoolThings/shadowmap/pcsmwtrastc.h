//#pragma once
//
//#include "..\..\..\Common_h\Mission.h"
//#include "..\..\..\Common_h\ishadows.h"
//
//class PCSMwTRaSTC : public MissionShadowCaster
//{
//public:
//	PCSMwTRaSTC();
//	virtual ~PCSMwTRaSTC();
//
//	// Создание объекта
//	bool Create(MOPReader & reader);
//	// Обновление параметров
//	bool EditMode_Update(MOPReader & reader);
//	// Cast/Recive/Блур у шадоумапа
//	void _cdecl Realize(float fDeltaTime, long level);
//	// отрисовка дебажной информации
//	void _cdecl DebugRealize(float fDeltaTime, long level);
//
//	void AddShadowCastObject(MissionObject * obj, MOF_EVENT func, const Vector & min, const Vector & max);
//
//private:
//	struct SCOData
//	{
//		MissionObject	* obj;					// 4
//		MOF_EVENT		func;					// 4
//
//		Matrix	mtx;							// 64
//		Vector	min, max;						// 24
//	};
//
//	IGMXService::RenderModeShaderId	PSSM_MT_CAST_id;
//	IGMXService::RenderModeShaderId	PSSM_Proj_Recv_id;
//
//	array<SCOData> scoObjects;
//
//	Matrix mShadowMap, mPrevShadowMap;
//
//	IVariable * pITestHistoryTex;
//	IVariable * pITestShadowMap, * pITestPrevShadowMap;
//	IVariable * pITestSampler;
//	IVariable * pITempCoord;
//	IVariable * pITestCoord, * pITestPow;
//
//	IRenderTarget * m_castRT;
//	IRenderTarget * m_curRT;
//	IRenderTarget * m_prevRT;
//	IRenderTargetDepth * m_castRTD;
//	IRenderTargetDepth * m_recvRTD;
//
//	Vector m_frustumTarget;
//	float m_frustumDiameter;
//
//	Matrix m_lightView, m_lightProj, m_oldLightView, m_oldLightProj;
//	Matrix m_view, m_invView, m_proj;
//	Vector m_camPos, m_camTarget, m_camUp;
//
//	Matrix m_lightMtx;
//	Vector m_lightDir;
//	float m_lightNear, m_lightFar, m_lightFarMax, m_lightFOV;
//
//	RENDERSCREEN m_renderInfo;
//	RENDERVIEWPORT m_normalViewport;
//	float m_cameraAspect, m_cameraFOV;
//
//	bool m_active;
//
//	// отрисовать cast проход
//	void Cast();
//	// принять тени
//	void Receive();
//
//	void CalculateLightForFrustum(long iIndex, Vector * pVCorners);
//
//	// Computes corner points of a frustum
//	void CalculateFrustumCorners(	Vector * pVPoints, const Vector & vCamSource, 
//		const Vector & vCamTarget, const Vector & vCamUp, float fNear, float fFar, 
//		float fFOV, float fAspect, float fScale);
//
//	// Создаются стандартные View/Projection матрицы
//	void CalculateViewProj(Matrix & mView, Matrix & mProj, const Vector & vSource, 
//		const Vector & vTarget, const Vector & vUpVector, float fFOV, float fNear, 
//		float fFar, float fAspect);
//
//	void CalcViewProjForSplit(long iIndex, float fNear, float fFar);
//};