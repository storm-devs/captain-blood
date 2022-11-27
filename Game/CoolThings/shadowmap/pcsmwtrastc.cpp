//#include "pcsmwtrastc.h"
//
//PCSMwTRaSTC::PCSMwTRaSTC() : scoObjects(_FL_)
//{
//	mPrevShadowMap.SetIdentity();
//	m_active = false;
//}
//
//double Halton(double I, double base)
//{
//	double H = 0.0;
//	double half = 1.0 / base;
//
//	while (I > 0)
//	{
//		double digit = fmod(I, base);
//		H = H + digit * half;
//		I =	(I - digit) / base;
//		half = half / base;
//	}
//
//	return H;
//}
//
//PCSMwTRaSTC::~PCSMwTRaSTC()
//{
//}
//
////Создание объекта
//bool PCSMwTRaSTC::Create(MOPReader & reader)
//{
//	m_castRT = Render().CreateRenderTarget(2048, 2048, _FL_, FMT_R32F);
//	m_castRTD = Render().CreateDepthStencil(2048, 2048, _FL_);
//
//	//m_curRT = Render().CreateRenderTarget(512, 512, _FL_, FMT_A8R8G8B8);
//	//m_prevRT = Render().CreateRenderTarget(512, 512, _FL_, FMT_A8R8G8B8);
//	m_curRT = Render().CreateRenderTarget(512, 512, _FL_, FMT_A16B16G16R16F);
//	m_prevRT = Render().CreateRenderTarget(512, 512, _FL_, FMT_A16B16G16R16F);
//	m_recvRTD = Render().CreateDepthStencil(512, 512, _FL_);
//
//	pITestShadowMap = Render().GetTechniqueGlobalVariable("mTestShadowMap", _FL_);
//	pITestPrevShadowMap = Render().GetTechniqueGlobalVariable("mTestPrevShadowMap", _FL_);
//
//	pITempCoord = Render().GetTechniqueGlobalVariable("vTempCrd", _FL_);
//	
//	pITestCoord = Render().GetTechniqueGlobalVariable("test_coord", _FL_);
//	pITestPow = Render().GetTechniqueGlobalVariable("test_pow", _FL_);
//
//	pITestSampler = Render().GetTechniqueGlobalVariable("TestTexture", _FL_);
//	pITestHistoryTex = Render().GetTechniqueGlobalVariable("TestHistoryTexture", _FL_);
//	
//	SetUpdate((MOF_UPDATE)&PCSMwTRaSTC::Realize, ML_SHADOWCAST);
//	SetUpdate((MOF_UPDATE)&PCSMwTRaSTC::DebugRealize, ML_LAST);
//
//	Geometry().GetRenderModeShaderId("TestCast", PSSM_MT_CAST_id);
//	Geometry().GetRenderModeShaderId("TestRecv", PSSM_Proj_Recv_id);
//
//	return EditMode_Update(reader);
//}
//
////Обновление параметров
//bool PCSMwTRaSTC::EditMode_Update(MOPReader & reader)
//{
//	m_lightNear = 0.4f;
//	m_lightFar = 400.0f; 
//	m_lightFarMax = 400.0f;
//	m_lightFOV = 120.0f;
//
//	Vector angles = reader.Angles();
//
//	Vector4 crd;
//	crd.x = reader.Float();
//	crd.y = reader.Float();
//	crd.z = reader.Float();
//	crd.w = reader.Float();
//	float ppow = reader.Float();
//
//	m_active = reader.Bool();
//
//	pITestCoord->SetVector4(crd);
//	pITestPow->SetFloat(ppow);
//
//	m_lightMtx = Matrix(angles);
//	m_lightDir = m_lightMtx.vz;
//
//	return true;
//}
//
//void PCSMwTRaSTC::Cast()
//{
//	Matrix oldView, oldProjection;
//
//	oldView = Render().GetView();
//	oldProjection = Render().GetProjection();
//
//	RENDERVIEWPORT castVP;
//	castVP.X = 0;			castVP.Y = 0;
//	castVP.Width = 2048;	castVP.Height = 2048;
//	castVP.MinZ = 0.0f;		castVP.MaxZ = 1.0f;
//
//	CalcViewProjForSplit(0, 0.0f, 100.0f);
//
//	Render().EndScene(null, true);
//
//	Render().PushViewport();
//	Render().PushRenderTarget();
//
//	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_castRT, m_castRTD);
//	Render().BeginScene();
//	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0x0);
//
//	Render().SetViewport(castVP);
//	Render().SetView(m_lightView);
//	Render().SetProjection(m_lightProj);
//
//	Geometry().SetRenderMode(&PSSM_MT_CAST_id);
//
//	for (int j=0; j<scoObjects.Len(); j++) 
//	{
//		const SCOData & sco = scoObjects[j];
//
//		ExecuteEvent(sco.obj, sco.func);
//	}
//
//	Geometry().SetRenderMode(null);
//
//	Render().EndScene();
//
//	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
//	Render().PopViewport();
//	Render().BeginScene();
//
//	Render().SetView(oldView);
//	Render().SetProjection(oldProjection);
//}
//
//void PCSMwTRaSTC::Receive()
//{
//	static bool m_firstRealize = true;
//
//	if (m_firstRealize)
//	{
//		m_oldLightView = m_lightView;
//		m_oldLightProj = m_lightProj;
//	}
//
//	pITestSampler->SetTexture(m_castRT->AsTexture());
//	pITestHistoryTex->SetTexture(m_prevRT->AsTexture());
//
//	float fTexOffset = 0.5f + (0.5f / float(2048));
//	Matrix mTexScale = Matrix().BuildScale(0.5f, -0.5f, 1.0f) * Matrix().BuildPosition(fTexOffset, fTexOffset, 0.0f);
//
//	Matrix mShadowMap = (m_lightView * m_lightProj) * mTexScale;
//	pITestShadowMap->SetMatrix(mShadowMap);
//
//	Matrix mILView = Matrix(m_lightView).InverseComplette4X4();
//	Matrix mILProj = Matrix(m_lightProj).InverseComplette4X4();
//
//	//pITestPrevShadowMap->SetMatrix((m_oldLightProj * m_oldLightView * mILView * mILProj) * mTexScale);
//	pITestPrevShadowMap->SetMatrix(m_oldLightView * m_oldLightProj);
//
//	m_oldLightView = m_view;
//	m_oldLightProj = m_proj;
//
//	RENDERVIEWPORT recvVP;
//	recvVP.X = 0;			recvVP.Y = 0;
//	recvVP.Width = 512;	recvVP.Height = 512;
//	recvVP.MinZ = 0.0f;		recvVP.MaxZ = 1.0f;
//
//	Render().PushViewport();
//	Render().PushRenderTarget();
//
//	Render().EndScene(null, true);
//
//	if (m_firstRealize)
//	{
//		m_firstRealize = false;
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_prevRT, m_recvRTD);
//		Render().BeginScene();
//		Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0x0, 1.0f, 0x0);
//		Render().EndScene();
//	}
//
//	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_curRT, m_recvRTD);
//	Render().BeginScene();
//	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0x0, 1.0f, 0x0);
//
//	Render().SetViewport(recvVP);
//
//	Geometry().SetRenderMode(&PSSM_Proj_Recv_id);
//
//	MGIterator * pIterSRecv = &Mission().GroupIterator(MG_SHADOWRECEIVE, _FL_);
//
//	for (pIterSRecv->Reset(); !pIterSRecv->IsDone(); pIterSRecv->Next())
//	{
//		pIterSRecv->ExecuteEvent();
//	}
//
//	RELEASE(pIterSRecv);
//
//	Geometry().SetRenderMode(null);
//	Render().EndScene();
//
//	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
//	Render().PopViewport();
//	Render().BeginScene();
//}
//
//void PCSMwTRaSTC::AddShadowCastObject(MissionObject * obj, MOF_EVENT func, const Vector & min, const Vector & max)
//{
//	SCOData & sco = scoObjects[scoObjects.Add()];
//
//	sco.obj = obj;
//	sco.func = func;
//	
//	obj->GetBox(sco.min, sco.max);
//	obj->GetMatrix(sco.mtx);
//}
//
//void _cdecl PCSMwTRaSTC::Realize(float fDeltaTime, long level)
//{
//	if (!m_active)
//		return;
//
//	Vector4 crd;
//	float rad = Rnd(0.5f / 2048.0f);
//	float ang = Rnd(PIm2);
//	crd.x = sinf(ang) * rad;
//	crd.y = cosf(ang) * rad;
//	pITempCoord->SetVector4(crd);
//
//	Swap(mShadowMap, mPrevShadowMap);
//	Swap(m_curRT, m_prevRT);
//
//	scoObjects.Empty();
//	GetShadowCastObjects();
//
//	m_view = Render().GetView();
//	m_invView = Matrix(m_view).Inverse();
//	m_proj = Render().GetProjection();
//
//	m_camPos = m_view.GetCamPos();
//	m_camTarget = m_camPos + m_invView.vz * 1.0f;
//	m_camUp = Vector(0.0f, 1.0f, 0.0f);
//
//	m_renderInfo = Render().GetScreenInfo3D();
//	m_normalViewport = Render().GetViewport();
//
//	m_cameraAspect = float(m_normalViewport.Width) / float(m_normalViewport.Height);
//	m_cameraFOV = (fabsf(m_proj.m[0][0]) > 1e-8f) ? ((180.0f * 2.0f * atanf(1.0f / m_proj.m[0][0])) / PI) : 71.0f;
//
//	Cast();
//	Receive();
//}
//
//void _cdecl PCSMwTRaSTC::DebugRealize(float fDeltaTime, long level)
//{
//	if (!m_active)
//		return;
//
//	RS_SPRITE spr[4];
//	spr[0].vPos = Vector (0.0f, 1.0f, 0.0f);
//	spr[1].vPos = Vector (1.0f, 1.0f, 0.0f);
//	spr[2].vPos = Vector (1.0f, 0.0f, 0.0f);
//	spr[3].vPos = Vector (0.0f, 0.0f, 0.0f);
//
//	spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
//	spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
//	spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
//	spr[3].tu = 0.0f;  spr[3].tv = 1.0f;
//
//	spr[0].dwColor = 0xFFFF0000;		spr[1].dwColor = 0xFFFF0000;
//	spr[2].dwColor = 0xFFFF0000;		spr[3].dwColor = 0xFFFF0000;
//
//	Render().DrawSprites(m_curRT->AsTexture(), spr, 1, "dbgSpritesNoblend");
//}
//
//void PCSMwTRaSTC::CalcViewProjForSplit(long iIndex, float fNear, float fFar)
//{
//	// Calculate corner points of frustum split
//    // To avoid edge problems, scale the frustum so
//    // that it's at least a few pixels larger
//	Vector vCorners[8];
//	CalculateFrustumCorners(vCorners, m_camPos, m_camTarget, m_camUp,
//                            fNear, fFar, m_cameraFOV, m_cameraAspect, 1.0f);	// Here is HACK = 1.2 - FIX ME!!!!
//
//	// Calculate view and projection matrices
//	CalculateLightForFrustum(iIndex, vCorners);
//}
//
//// Создаются стандартные View/Projection матрицы
//void PCSMwTRaSTC::CalculateViewProj(Matrix & mView, Matrix & mProj,
//                       const Vector & vSource, const Vector & vTarget, const Vector & vUpVector,
//                       float fFOV, float fNear, float fFar, float fAspect)
//{
//	// view матрица
//	mView.BuildView(vSource, vTarget, vUpVector);
//
//	// projection матрица
//	mProj.BuildProjection(Deg2Rad(fFOV), float(2048.0f), float(2048.0f), fNear, fFar);
//}
//
//// Computes corner points of a frustum
//void PCSMwTRaSTC::CalculateFrustumCorners(	Vector * pVPoints,
//								const Vector & vCamSource, const Vector & vCamTarget, const Vector & vCamUp,
//								float fNear, float fFar, float fFOV, float fAspect, float fScale)
//{
//	Vector vZ = !(vCamTarget - vCamSource);
//	Vector vX = !(vCamUp ^ vZ);
//	Vector vY = vZ ^ vX;
//
//	float fNearPlaneHeight = tanf(Deg2Rad(fFOV) * 0.5f) * fNear;
//	float fNearPlaneWidth = fNearPlaneHeight * fAspect;
//
//	float fFarPlaneHeight = tanf(Deg2Rad(fFOV) * 0.5f) * fFar;
//	float fFarPlaneWidth = fFarPlaneHeight * fAspect;
//
//	Vector vNearPlaneCenter = vCamSource + vZ * fNear;
//	Vector vFarPlaneCenter = vCamSource + vZ * fFar;
//
//	pVPoints[0] = vNearPlaneCenter - vX * fNearPlaneWidth - vY * fNearPlaneHeight;
//	pVPoints[1] = vNearPlaneCenter - vX * fNearPlaneWidth + vY * fNearPlaneHeight;
//	pVPoints[2] = vNearPlaneCenter + vX * fNearPlaneWidth + vY * fNearPlaneHeight;
//	pVPoints[3] = vNearPlaneCenter + vX * fNearPlaneWidth - vY * fNearPlaneHeight;
//
//	pVPoints[4] = vFarPlaneCenter - vX * fFarPlaneWidth - vY * fFarPlaneHeight;
//	pVPoints[5] = vFarPlaneCenter - vX * fFarPlaneWidth + vY * fFarPlaneHeight;
//	pVPoints[6] = vFarPlaneCenter + vX * fFarPlaneWidth + vY * fFarPlaneHeight;
//	pVPoints[7] = vFarPlaneCenter + vX * fFarPlaneWidth - vY * fFarPlaneHeight;
//
//	float c = fFarPlaneWidth * 0.5f;
//	float b = fNearPlaneWidth * 0.5f;
//	float l = fFar - fNear;
//	float a = (c * c + l * l - b * b) / (2.0f * l);
//	float k = sqrtf(b * b + a * a);
//
//	// равноудаленный центр фрустума
//	m_frustumTarget = vNearPlaneCenter + (vFarPlaneCenter - vNearPlaneCenter) * a / l;
//	m_frustumDiameter = k * 2.0f;
//
//	// считаем центр фрустума
//	Vector vCenter = 0.0f;
//	for (long i=0; i<8; i++) 
//		vCenter += pVPoints[i];
//	vCenter = vCenter / 8.0f;
//
//	// for each point
//	for (long i=0; i<8; i++)
//	{
//		// scale by adding offset from center
//		pVPoints[i] += (pVPoints[i] - vCenter) * (fScale - 1.0f);
//	}
//}
//
//void PCSMwTRaSTC::CalculateLightForFrustum(long iIndex, Vector * pVCorners)
//{
//	Vector vMin = FLT_MAX;
//	Vector vMax = -FLT_MAX;
//
//	for (long i=0; i<8; i++)
//	{
//		if (pVCorners[i].x < vMin.x) vMin.x = pVCorners[i].x;
//		if (pVCorners[i].y < vMin.y) vMin.y = pVCorners[i].y;
//		if (pVCorners[i].z < vMin.z) vMin.z = pVCorners[i].z;
//
//		if (pVCorners[i].x > vMax.x) vMax.x = pVCorners[i].x;
//		if (pVCorners[i].y > vMax.y) vMax.y = pVCorners[i].y;
//		if (pVCorners[i].z > vMax.z) vMax.z = pVCorners[i].z;
//	}
//
//	Vector vCenter = 0.0f;
//	for (long i=0; i<8; i++) 
//		vCenter += pVCorners[i];
//	vCenter = vCenter / 8.0f;
//
//	Vector vBox[8];
//
//	vBox[0] = Vector(vMin.x, vMin.y, vMin.z);
//	vBox[1] = Vector(vMax.x, vMin.y, vMin.z);
//	vBox[2] = Vector(vMin.x, vMin.y, vMax.z);
//	vBox[3] = Vector(vMax.x, vMin.y, vMax.z);
//	vBox[4] = Vector(vMin.x, vMax.y, vMin.z);
//	vBox[5] = Vector(vMax.x, vMax.y, vMin.z);
//	vBox[6] = Vector(vMin.x, vMax.y, vMax.z);
//	vBox[7] = Vector(vMax.x, vMax.y, vMax.z);
//
//	float fMinDistance = FLT_MAX;
//	float fMaxDistance = -FLT_MAX;
//	Vector vBoxCenter = (vMin + vMax) / 2.0f;
//
//	for (long i=0; i<8; i++)
//	{
//		Vector vCube = vBox[i] - vBoxCenter;
//		float fDistance = vCube | m_lightDir;
//
//		if (fDistance < fMinDistance) fMinDistance = fDistance;
//		if (fDistance > fMaxDistance) fMaxDistance = fDistance;
//	}
//
//	Vector lightSource = m_frustumTarget - (m_lightDir * fabsf(fMinDistance * 1.2f));
//	Vector lightTarget = m_frustumTarget;
//
//	//lightSource += Vector(RRnd(-0.49f, 0.49f), RRnd(-0.49f, 0.49f), RRnd(-0.49f, 0.49f));
//
//	// рассчитываем light view и light projection матрицы
//	CalculateViewProj(	m_lightView, m_lightProj,
//						lightSource, lightTarget, Vector(0.0f, 1.0f, 0.0f),
//						m_lightFOV, m_lightNear, m_lightFarMax, 1.0f);
//
//	float r = m_frustumDiameter * 1.35f;
//
//	float radius = float(long((r + 2.0f) / 2.0f) * 2.0f);
//
//	m_lightProj.BuildOrtoProjection(radius, radius, -100.0f, 400.0f);
//	float rn = 0.5f / 2048.0f;
//
//	static double II = 0.0;
//
//	II++;
//
//	float x = (float)Halton(II, 2.0);
//	float y = (float)Halton(II, 3.0);
//
//	m_lightProj *= Matrix().BuildPosition((x - 0.5f) * rn * 2.0f, (y - 0.5f) * rn * 2.0f, 0.0f);
//
//	//m_lightProj *= Matrix().BuildPosition(Rnd(rn * 2) - rn, Rnd(rn * 2) - rn, 0.0f);
//
//	// убираем subpixel jittering
//	/*double resolution = 2048.0 * 0.5;
//
//	Vector origin = Vector(0.0f) * (m_lightView * m_lightProj);
//
//	double texCoordX = origin.x * resolution;
//	double texCoordY = origin.y * resolution;
//
//	double dx = floor(texCoordX + 0.5) - texCoordX;
//	double dy = floor(texCoordY + 0.5) - texCoordY;
//
//	dx /= resolution;
//	dy /= resolution;
//
//	m_lightProj *= Matrix().BuildPosition(float(dx), float(dy), 0.0f);*/
//}
//
//MOP_BEGINLISTG(PCSMwTRaSTC, "ShadowMapa", '1.00', 101, "Lighting");
//	MOP_ANGLESC("Light Dir", Vector(1.0f, 0.0f, 0.0f), "Угол откуда светит источник")
//	MOP_FLOAT("vpos_dx", 0.5f)
//	MOP_FLOAT("vpos_dy", -0.5f)
//	MOP_FLOAT("crd_dx", -0.5f)
//	MOP_FLOAT("crd_dy", -0.5f)
//	MOP_FLOAT("pow", 3.0f)
//	MOP_BOOL("Active", true)
//MOP_ENDLIST(PCSMwTRaSTC)
