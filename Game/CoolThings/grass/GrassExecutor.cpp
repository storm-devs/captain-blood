#include "Grass.h"
#include "..\..\..\Common_h\IShadows.h"
#include "..\..\..\Common_h\SetThreadName.h"


#define GRASS_DYNAMIC		0//USAGE_DYNAMIC

__declspec(align(128)) Plane GrassExecutor::m_frustumPlanes[6];

GrassExecutor::GrassExecutor() :
	drawPackets(_FL_, 1),
	m_grassesToExecute(_FL_, 16)
{
	m_lastGrass = null;
	m_threadStarted = false;

	m_hThread = null;
	m_hStartEvent = null;
	m_hDoneEvent = null;
	m_hExitEvent = null;

	m_iBuffer = null;

	m_vBuffers[0] = null;
	m_vBuffers[1] = null;

	m_curVBuffer = 0;

	m_playerPos = Vector4(1e+6f, 0.0f, 1e+6f, 1.0f);
}

GrassExecutor::~GrassExecutor()
{
	if (m_isMultiThreading)
	{
		// завершаем выполнение отдельного треда
		SetEvent(m_hExitEvent);
		for (int i=0; i<5; i++)
		{
			if ( WaitForSingleObject(m_hThread, 2000) == WAIT_OBJECT_0 ) 
				break;

			api->Trace("GrassExecutor: Killing thread problem!");
		}

		CloseHandle(m_hThread);
		CloseHandle(m_hStartEvent);
		CloseHandle(m_hDoneEvent);
		CloseHandle(m_hExitEvent);
	}

	RELEASE(m_vBuffers[0]);
	RELEASE(m_vBuffers[1]);
	RELEASE(m_iBuffer);
}

bool GrassExecutor::Create(MOPReader & reader)
{
	m_isMultiThreading = (api->GetThreadingInfo() != ICore::mt_none);
	//m_isMultiThreading = false;

	m_shadowService = (IShadowsService *)api->GetService("ShadowsService");

#ifdef _XBOX
	m_iBuffer = null;//Render().CreateIndexBuffer(MAX_GRASSES * 2 * 3 * sizeof(word), _FL_, USAGE_DYNAMIC | USAGE_WRITEONLY, FMT_INDEX16, POOL_DEFAULT);
	m_vBuffers[0] = Render().CreateVertexBuffer(sizeof(GrassHolder::Vertex) * MAX_GRASSES, sizeof(GrassHolder::Vertex), _FL_, GRASS_DYNAMIC, POOL_DEFAULT);
	m_vBuffers[1] = Render().CreateVertexBuffer(sizeof(GrassHolder::Vertex) * MAX_GRASSES, sizeof(GrassHolder::Vertex), _FL_, GRASS_DYNAMIC, POOL_DEFAULT);
#else
	m_iBuffer = Render().CreateIndexBuffer(MAX_INDICES * 2 * 3 * sizeof(word), _FL_);
	m_vBuffers[0] = Render().CreateVertexBuffer(sizeof(GrassHolder::Vertex) * MAX_GRASSES * 4, sizeof(GrassHolder::Vertex), _FL_, USAGE_DYNAMIC | USAGE_WRITEONLY, POOL_DEFAULT);

	word * ww = (word*)m_iBuffer->Lock();
	for (int i=0; i<MAX_INDICES; i++)
	{
		*ww++ = word( i * 4 + 0 );
		*ww++ = word( i * 4 + 1 );
		*ww++ = word( i * 4 + 2 );

		*ww++ = word( i * 4 + 0 );
		*ww++ = word( i * 4 + 2 );
		*ww++ = word( i * 4 + 3 );
	}
	m_iBuffer->Unlock();
#endif

	if (m_isMultiThreading)
	{
		m_hStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL); Assert(m_hStartEvent);
		m_hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL); Assert(m_hDoneEvent);
		m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL); Assert(m_hExitEvent);

		DWORD dwGrassThreadID = 0;
		m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WorkThread, this, CREATE_SUSPENDED, &dwGrassThreadID);	Assert(m_hThread);
#ifdef _XBOX
		::XSetThreadProcessor(m_hThread, 2);
		::SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);
#endif
		XSetThreadName(dwGrassThreadID, "Grass::Work");
		ResumeThread(m_hThread);
	}

	SetUpdate(&GrassExecutor::Synchronize, ML_EXECUTE_END + 1);
	SetUpdate(&GrassExecutor::Execute, ML_EXECUTE1 - 1);

#if !defined(STOP_DEBUG) && !defined(_XBOX)
	SetUpdate(&GrassExecutor::DebugRealize, ML_DEBUG + 1);
#endif

	drawPackets.AddElements(32);
	for (int k=0; k<drawPackets.Len(); k++)
	{
		DrawPacket & packet = drawPackets[k];
	
		packet.used = false;

		/*for (int j=0; j<21; j++)
		{
			packet.posVectors[4 + j * 6 + 0].w = Grass::GH.mtxNormals[j].m[0][0];
			packet.posVectors[4 + j * 6 + 1].w = Grass::GH.mtxNormals[j].m[1][0];
			packet.posVectors[4 + j * 6 + 2].w = Grass::GH.mtxNormals[j].m[2][0];

			packet.posVectors[4 + j * 6 + 3].w = Grass::GH.mtxNormals[j].m[0][2];
			packet.posVectors[4 + j * 6 + 4].w = Grass::GH.mtxNormals[j].m[1][2];
			packet.posVectors[4 + j * 6 + 5].w = Grass::GH.mtxNormals[j].m[2][2];
		}*/
	}

	return true;
}

void GrassExecutor::Add2Execute(Grass * grass)
{
	m_grassesToExecute.Add(grass);
}

dword __stdcall GrassExecutor::WorkThread(LPVOID lpParameter)
{
	GrassExecutor * exec = (GrassExecutor *)lpParameter;

	HANDLE m_hWaitObjects[] = {exec->m_hStartEvent, exec->m_hExitEvent};

	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(sizeof(m_hWaitObjects) / sizeof(m_hWaitObjects[0]), m_hWaitObjects, FALSE, INFINITE);
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		for (long i=0; i<exec->m_grassesToExecute.Len(); i++)
		{
			WorkResult result = exec->m_grassesToExecute[i]->Work(exec->m_vertices, exec->m_camPos, exec->m_frustumPlanes);

			if (result == GRASS_OVERFLOW)
				break;

			exec->m_grassesToExecute[i] = null;
		}

		SetEvent( exec->m_hDoneEvent );
	}

	return 0;
}

// запускаем работу трав в отдельном потоке
void _cdecl GrassExecutor::Execute(float fDeltaTime, long level)
{
	m_lastGrass = null;

	// получаем view матрицу
	m_view = Render().GetView();

	// получаем позицию камеры
	m_camPos = m_view.GetCamPos();

	// получаем и сохраняем frustum planes
	const Plane * frustum = Render().GetFrustum();
	for (dword i=0; i<Render().GetNumFrustumPlanes(); i++)
		m_frustumPlanes[i] = frustum[i];

	for (int i=0; i<drawPackets.Len(); i++)
		drawPackets[i].used = false;

#ifdef _XBOX
	m_curVBuffer ^= 1;
#endif

	// запускаем другой поток
	if (m_isMultiThreading)
	{
		// лочим вертекс буфер
		m_vertices = (GrassHolder::Vertex *)m_vBuffers[m_curVBuffer]->Lock(0, 0, LOCK_DISCARD);

		m_threadStarted = true;
		SetEvent(m_hStartEvent);
	}
}

#if !defined(STOP_DEBUG) && !defined(_XBOX)
void _cdecl GrassExecutor::DebugRealize(float fDeltaTime, long level)
{
	static bool debugDraw = false;

	if (api->DebugKeyState(VK_CONTROL, 'G'))
	{
		debugDraw ^= 1;
		Sleep(100);
	}
	
	if (debugDraw)
	{
		Render().Print(10.0f, 20.0f, 0xFF00FF00, "Grass statistic:");
		Render().Print(10.0f, 40.0f, 0xFF00FF00, "total draws: %d", Grass::m_totalDraws);
		Render().Print(10.0f, 60.0f, 0xFF00FF00, "total blocks: %d", Grass::m_totalBlocks);
		Render().Print(10.0f, 80.0f, 0xFF00FF00, "total grasses: %d", Grass::m_totalGrasses);
	}
}
#endif

void _cdecl GrassExecutor::Synchronize(float fDeltaTime, long level)
{
	dword pixId = Render().pixBeginEvent(_FL_, "GrassExecutor");

	m_lastGrass = null;

	Grass::m_totalBlocks = 0;
	Grass::m_totalGrasses = 0;
	Grass::m_totalDraws = 0;

	// ставим позицию плеера если такой есть
	MissionObject * player = Mission().Player();
	Matrix pmtx(true);
	Vector4 ppos = Vector4(1e+6f, 0.0f, 1e+6f);
	if (player)
		ppos.v = player->GetMatrix(pmtx).pos;

	float dist = (m_playerPos.v - ppos.v).GetLength2();
	if (dist > 1e-4f)
		m_playerPos.w = 0.25f;
	else
		m_playerPos.w *= 0.95f * Max(1.0f / 15.0f, fDeltaTime) / (1.0f / 15.0f);
	//m_playerPos.w += ((dist > 1e-4f) ? 1.0f : -fDeltaTime * 0.0005f) * m_pposAcceleration;
	m_playerPos.w = MinMax(0.0f, 0.25f, m_playerPos.w);
	ppos.w = 0.15f + m_playerPos.w;
	Grass::GH.varGrassPlayer->SetVector4(ppos);

	m_playerPos.v = ppos.v;

	//Render().SetGPRAllocationCount(64, 64);
	// ожидаем окончания выпонения отдельного потока
	if (m_isMultiThreading && m_threadStarted)
	{
		for (int i=0; i<5; i++)
		{
			if ( WaitForSingleObject(m_hDoneEvent, 2000) == WAIT_OBJECT_0 ) 
				break;

			api->Trace("GrassExecutor::Synchronize : m_hDoneEvent waiting timeout!");
		}
		m_threadStarted = false;

		// рисуем те пакеты которые нагенерились в другом потоке
		m_vBuffers[m_curVBuffer]->Unlock();
		//DrawPackets();
	}
	else
	{
		// лочим вертекс буфер
		m_vertices = (GrassHolder::Vertex *)m_vBuffers[m_curVBuffer]->Lock(0, 0, LOCK_DISCARD);

		for (long i=0; i<m_grassesToExecute.Len(); i++)
		{
			WorkResult result = m_grassesToExecute[i]->Work(m_vertices, m_camPos, m_frustumPlanes);

			if (result == GRASS_OVERFLOW)
				break;

			m_grassesToExecute[i] = null;
		}

		m_vBuffers[m_curVBuffer]->Unlock();
		//DrawPackets();
	}
	//Render().SetGPRAllocationCount(32, 96);

	// удаляем все что не нужно
	m_grassesToExecute.Empty();

	Render().pixEndEvent(_FL_, pixId);
}

void GrassExecutor::RealizeGrass(Grass * grass)
{
	Render().SetStreamSource(0, m_vBuffers[m_curVBuffer]);

	bool prepared = false;
	for (int i=0; i<drawPackets.Len(); i++)
	{
		if (!drawPackets[i].used || drawPackets[i].grass != grass)
			continue;

		if (!prepared) 
		{
			Render().SetViewport( Render().GetViewport() );
			Render().SetWorld(Matrix());
			Render().SetupShadowLimiters();

			if (m_shadowService->GetPostProcessTexture())
				Grass::GH.varShadowMap->SetTexture(m_shadowService->GetPostProcessTexture()->AsTexture());
			else
				Grass::GH.varShadowMap->ResetTexture();

			Render().SetGPRAllocationCount(32, 96);
			prepared = true;
			grass->PrepareForRender(m_view);
		}

		grass->DrawPacket(drawPackets[i]);
		
		//drawPackets[i].used = false;
	}

	if (prepared)
		Render().SetGPRAllocationCount(32, 96);

	//Render().Print(10.0f, 20.0f, 0xFF00FF00, "%.3f", m_playerPos.w);
}

MOP_BEGINLIST(GrassExecutor, "", '1.00', 101)
MOP_ENDLIST(GrassExecutor);
