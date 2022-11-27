#include "Ocean.h"

Ocean3Executor::Ocean3Executor() :
	m_oceansToExecute(_FL_)
{
	m_refCount = 0;
	m_executorIterator = null;

	m_bumpOcean = null;

	m_threadStarted = false;

	m_isMultiThreading = (api->GetThreadingInfo() != ICore::mt_none);
	//m_isMultiThreading = false;

	if (m_isMultiThreading)
	{
		m_threadWavesFFT.Create(this, "fft::waves", (LPTHREAD_START_ROUTINE)&WorkThreadWavesFFT, 1, THREAD_PRIORITY_BELOW_NORMAL);
		m_threadBumpFFT.Create(this, "fft::bump", (LPTHREAD_START_ROUTINE)&WorkThreadBumpFFT, 2, THREAD_PRIORITY_BELOW_NORMAL);

		m_threadWaves.Create(this, "ocean::waves", (LPTHREAD_START_ROUTINE)&WorkThreadWaves, 3);
		m_threadBump.Create(this, "ocean::bump", (LPTHREAD_START_ROUTINE)&WorkThreadBump, 4);
	}
}

Ocean3Executor::~Ocean3Executor()
{
	if (m_isMultiThreading)
	{
		m_threadWaves.Release();
		m_threadBump.Release();
		m_threadWavesFFT.Release();
		m_threadBumpFFT.Release();
	}

	RELEASE(m_executorIterator);
	RELEASE(m_reflectorIterator);
}

bool Ocean3Executor::Create(MOPReader & reader)
{
	m_oceans = (Ocean3Service *)api->GetService("Ocean3Service");

	m_executorIterator = &Mission().GroupIterator(OCEAN_GROUP3, _FL_);
	m_reflectorIterator = &Mission().GroupIterator(MG_SEAREFLECTION, _FL_);

	static const ConstString id_Ocean3Refractor("Ocean3Refractor");
	Mission().CreateObject(m_refractor.GetSPObject(), "Ocean3Refractor", id_Ocean3Refractor, true);

#ifdef _XBOX
	pEDRAM_Color = Render().GetTechniqueGlobalVariable("EDRAM_Color", _FL_);
	pEDRAM_Depth = Render().GetTechniqueGlobalVariable("EDRAM_Depth", _FL_);
	Render().GetShaderId("X360RestoreEDRAM", X360RestoreEDRAM_id);
	Render().GetShaderId("X360RestoreEDRAM_ColorOnly", X360RestoreEDRAM_ColorOnly_id);
#else
	varRestoreColorParams = Render().GetTechniqueGlobalVariable("Ocean2RestoreColorParams", _FL_);
	varRestoreColorTexture = Render().GetTechniqueGlobalVariable("Ocean2ScreenCopyTexture", _FL_);
	Render().GetShaderId("Ocean2ScreenCopy", PCRestoreColor_id);
#endif

	//m_oceans->fftWaves.Setup(512.0f, 210.0e-10f, 30.0f, 1.0f, 0.0f, 3.0f);
	//m_oceans->fftBump.Setup(2048.0f, 22.4e-10f, 57.0f, 1.0f, 0.0f, 0.0f);

	return true;
}

dword __stdcall Ocean3Executor::WorkThreadWavesFFT(LPVOID lpParameter)
{
	Ocean3Executor * exec = (Ocean3Executor *)lpParameter;

	HANDLE m_hWaitObjects[] = {exec->m_threadWavesFFT.startEvent, exec->m_threadWavesFFT.exitEvent};
	
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(sizeof(m_hWaitObjects) / sizeof(m_hWaitObjects[0]), m_hWaitObjects, FALSE, INFINITE);
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		float wave_speed = (exec->m_bumpOcean) ? exec->m_bumpOcean->GetWaveSpeed() : 1.0f;
		exec->m_oceans->fftWaves.Execute(exec->m_fftDeltaTime * wave_speed);

		SetEvent( exec->m_threadWavesFFT.doneEvent );
	}

	return 0;
}

dword __stdcall Ocean3Executor::WorkThreadBumpFFT(LPVOID lpParameter)
{
	Ocean3Executor * exec = (Ocean3Executor *)lpParameter;

	HANDLE m_hWaitObjects[] = {exec->m_threadBumpFFT.startEvent, exec->m_threadBumpFFT.exitEvent};
	
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(sizeof(m_hWaitObjects) / sizeof(m_hWaitObjects[0]), m_hWaitObjects, FALSE, INFINITE);
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		float bump_speed = (exec->m_bumpOcean) ? exec->m_bumpOcean->GetBumpSpeed() : 1.0f;
		exec->m_oceans->fftBump.Execute(exec->m_fftDeltaTime * bump_speed);

		SetEvent( exec->m_threadBumpFFT.doneEvent );
	}

	return 0;
}

dword __stdcall Ocean3Executor::WorkThreadWaves(LPVOID lpParameter)
{
	Ocean3Executor * exec = (Ocean3Executor *)lpParameter;

	HANDLE m_hWaitObjects[] = {exec->m_threadWaves.startEvent, exec->m_threadWaves.exitEvent};
	
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(sizeof(m_hWaitObjects) / sizeof(m_hWaitObjects[0]), m_hWaitObjects, FALSE, INFINITE);
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		// выполняем 
		for (long i=0; i<exec->m_oceansToExecute.Len(); i++)
			exec->m_oceansToExecute[i]->WorkThreadPreFFT();

		for (long i=0; i<exec->m_oceansToExecute.Len(); i++)
			exec->m_oceansToExecute[i]->WorkThread();

		SetEvent( exec->m_threadWaves.doneEvent );
	}

	return 0;
}

dword __stdcall Ocean3Executor::WorkThreadBump(LPVOID lpParameter)
{
	Ocean3Executor * exec = (Ocean3Executor *)lpParameter;

	HANDLE m_hWaitObjects[] = {exec->m_threadBump.startEvent, exec->m_threadBump.exitEvent};
	
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(sizeof(m_hWaitObjects) / sizeof(m_hWaitObjects[0]), m_hWaitObjects, FALSE, INFINITE);
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		if (exec->m_bumpOcean)
			exec->m_bumpOcean->GenerateBumpTexture();

		SetEvent( exec->m_threadBump.doneEvent );
	}

	return 0;
}

void Ocean3Executor::AddRef()
{
	// если это первый океан, то добавляем функцию для выполнения
	if (m_refCount == 0)
	{
		firstStart = true;

		SetUpdate(&Ocean3Executor::Realize, ML_ALPHA3 - 1);
		//SetUpdate(&Ocean3Executor::First, ML_FIRST);			// запускаем выполнение FFT для волн и бампа
		SetUpdate(&Ocean3Executor::Execute, ML_EXECUTE1 - 1);	// обязательно чтобы был позже execute у Ocean3
	}

	m_refCount++;
}

void Ocean3Executor::DecRef(Ocean2 * ocean)
{
	Assert(m_refCount);

	// если это последний океан, то удаляем функцию для выполнения
	if (m_refCount == 1)
	{
		// останавливаем работу потока
		StopWorking();

		DelUpdate(&Ocean3Executor::Execute);
		DelUpdate(&Ocean3Executor::Realize);
	}

	if (ocean)
		m_oceansToExecute.Del(ocean);

	m_refCount--;
}

void Ocean3Executor::StopWorking()
{
	if (!m_isMultiThreading)
		return;

	// если поток был запущен, ждем окончания его выполнения
	if (!m_threadStarted)
		return;

	//m_threadWavesFFT.Synchronize();
	//m_threadBumpFFT.Synchronize();

	m_threadWaves.Synchronize();
	m_threadBump.Synchronize();

	m_threadStarted = false;
}

void Ocean3Executor::AddToExecute(Ocean2 * ocean)
{
	if (m_isMultiThreading)
		Assert(!m_threadStarted);

	if (m_oceansToExecute.Find(ocean) != INVALID_ARRAY_INDEX)
	{
		api->Trace("ERROR: Ocean: adding duplicate ocean to execute, \"%s\"", ocean->GetObjectID().c_str());
		return;
	}

	m_oceansToExecute.Add(ocean);
}

void _cdecl Ocean3Executor::Execute(float fDeltaTime, long level)
{
	// ищем наиболее подходящую воду для работы бампа и волн
	// если есть активный объект с m_mainFFTSource = true, он будет главным
	// иначе если есть активный океан, он будет главным, иначе будет какая-нить вода
	Ocean2 * oldBumpOcean = m_bumpOcean;
	m_bumpOcean = null;
	bool fftSource = false;
	for (int i=0; i<m_oceansToExecute.Len(); i++)
	{
		Ocean2 * o = m_oceansToExecute[i];

		if (o->IsShow() && o->IsMainFFTSource())
		{
			fftSource = true;
			m_bumpOcean = o;
		}

		if (!o->IsWorking(true)) continue;

		if (!fftSource)
			m_bumpOcean = o;

		if (o->IsBigOceanActive())
		{
			m_bumpOcean = o;
			break;
		}
	}

	if (m_bumpOcean)
	{
		if (oldBumpOcean != m_bumpOcean)
		{
			m_bumpOcean->SetupFFT();
		}

		if (m_isMultiThreading)
		{
			if (firstStart)
			{
				// стартуем выполнение FFT, если это первый запуск
				firstStart = false;
				m_fftDeltaTime = 0.0f;
				m_threadWavesFFT.Start();
				m_threadBumpFFT.Start();
			}

			// ждем окончания выполнения FFT с предыдущего кадра
			m_threadWavesFFT.Synchronize(); 
			m_threadBumpFFT.Synchronize();
 
			m_oceans->fftWaves.ChangeCurFrame();
			m_oceans->fftBump.ChangeCurFrame();

			// стартуем выполнение рассчета волн и генерации текстуры бампа
			m_threadWaves.Start();
			m_threadBump.Start();

			// стартуем рассчет FFT для следующего кадра
			m_fftDeltaTime = fDeltaTime;
			m_threadWavesFFT.Start();
			m_threadBumpFFT.Start();

			m_threadStarted = true;
		}
	}
}

bool __forceinline Ocean3Executor::IsEditModeOn()
{
#if defined(STOP_TOOLS) || defined(_XBOX)
	return false;
#else
	if (EditMode_IsOn())
		return true;

	if (oldViewport.X || oldViewport.Y || oldViewport.Width != screenInfo.dwWidth || oldViewport.Height != screenInfo.dwHeight)
		return true;

	return false;
#endif
}

void Ocean3Executor::ZPrePass(bool setAlpha)
{
	MGIterator * iter = m_executorIterator;

	for (iter->Reset(); !iter->IsDone(); iter->Next())
	{
		Ocean2 * ocean = (Ocean2*)iter->Get();
		if (!ocean->IsWorking(true)) continue;

		if (!ocean->IsRealWavesEnable())
			ocean->DrawFlatOceanAndParts(false, false, true, setAlpha);
	}
}

void _cdecl Ocean3Executor::Realize(float fDeltaTime, long level)
{
	//LARGE_INTEGER TicksPerSecond;
    //QueryPerformanceFrequency( &TicksPerSecond );	
	//float fTicksPerMillisecond = (DOUBLE)TicksPerSecond.QuadPart * 0.001;
	
	// ожидаем окончания выполнения отдельного потока
	//dword t1, t2, t3, t4, t5, t6;
	if (!m_isMultiThreading)
	{
		if (m_bumpOcean)
		{
			float wave_speed = m_bumpOcean->GetWaveSpeed();
			float bump_speed = m_bumpOcean->GetBumpSpeed();

			m_oceans->fftWaves.Execute(fDeltaTime * wave_speed);
			m_oceans->fftBump.Execute(fDeltaTime * bump_speed);

			m_oceans->fftWaves.ChangeCurFrame();
			m_oceans->fftBump.ChangeCurFrame();

			m_bumpOcean->GenerateBumpTexture();

			for (long i=0; i<m_oceansToExecute.Len(); i++)
				m_oceansToExecute[i]->WorkThreadPreFFT();

			for (long i=0; i<m_oceansToExecute.Len(); i++)
				m_oceansToExecute[i]->WorkThread();
		}
	}
	else
	{
		StopWorking();
	}

	m_oceansToExecute.Empty();

	// считаем кол-во реальных и плоских вод/океанов
	MGIterator * iter = m_executorIterator;
	bool flatOceanExist = false;
	long numFlat = 0, numRealWaves = 0;
	for (iter->Reset(); !iter->IsDone(); iter->Next())
	{
		Ocean2 * o = (Ocean2*)iter->Get();

		o->UnlockBuffers();

		if (!o->IsWorking(true)) continue;

		numFlat += (o->IsRealWavesEnable()) ? 0 : 1;
		numRealWaves += (o->IsRealWavesEnable()) ? 1 : 0;
		if (o->IsBigOceanActive() && !o->IsRealWavesEnable())
			flatOceanExist = true;
	}

	if (api->DebugKeyState('O')) return;
	if (api->DebugKeyState('8')) return;

	dword dwId0 = Render().pixBeginEvent(_FL_, "Ocean3::Executor");

	IRenderTarget * curRT = Render().GetRenderTarget(_FL_);
	IRenderTargetDepth * curRTDepth = Render().GetDepthStencil(_FL_);

	oldViewport = Render().GetViewport();
	screenInfo = Render().GetScreenInfo3D();

	bool reflectionExist = false;

	// подготавливаем плоскость для преломления
	/*if (m_refractor.Ptr()->Refracting(fDeltaTime, level))
	{
		// рисуем плоские воды и океаны
		for (iter->Reset(); !iter->IsDone(); iter->Next())
		{
			Ocean2 * ocean = (Ocean2*)iter->Get();
			if (!ocean->IsWorking(true)) continue;

			ocean->DrawFlatOceanAndParts(false, true);
		}
	}
	Render().ResolveColor(m_oceans->pOceanRT->AsTexture());
	m_oceans->pVarRefrTex->SetTexture(m_oceans->pOceanRT->AsTexture());*/

	// заполняем фейковыми морями/поверхностями с предыдущего кадра, чтобы антиалиасинг более-менее работал

	if (numFlat)
	{
		Render().PushViewport();

		RENDERVIEWPORT vp;
		vp.Width = m_oceans->m_oceanRTWidth;	vp.Height = m_oceans->m_oceanRTHeight;
		vp.X = 0;								vp.Y = 0;
		vp.MinZ = 0.0f;							vp.MaxZ = 1.0f;

#ifdef _XBOX
		Render().EndScene(null, false);		
		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
		Render().BeginScene();
		Render().SetViewport(vp);

		// FIX-ME: восстанавливаем контекст, это оптимизация, чтобы не было лишних ResolveDepth и EndScene(xxx)
		/*if (pEDRAM_Color && pEDRAM_Depth)
		{
			pEDRAM_Color->SetTexture(curRT->AsTexture());
			pEDRAM_Depth->SetTexture(curRTDepth->AsTexture());
			Render().DrawFullScreenQuad(m_oceans->m_oceanRTWidth, m_oceans->m_oceanRTHeight, X360RestoreEDRAM_id);
		}*/
		Render().DisableHyperZ(true);
#else
		Render().PushRenderTarget();
		//Render().EndScene((IsEditModeOn()) ? null : m_oceans->pOceanRT->AsTexture());
		Render().EndScene(null);
		//Render().EndScene(m_oceans->pOceanRT->AsTexture());

		if (!IsEditModeOn())
			Render().ResolveDepth(m_oceans->pOceanRTDepth);

		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_oceans->pOceanRT, m_oceans->pOceanRTDepth);
		Render().BeginScene();
		Render().SetViewport(vp);

		bool isClearZ = (IsEditModeOn() || m_oceans->m_isOceansHaveFakeHeight);

		if (!isClearZ && flatOceanExist)
		{
			// чистим альфу
			Render().DrawFullScreenQuad(float(m_oceans->m_oceanRTWidth), float(m_oceans->m_oceanRTHeight), m_oceans->m_shaderClearAlpha);

			// рисуем в альфу плоский океан
			ZPrePass(true);

			Render().EndScene();
			Render().BeginScene();

			// рисуется плоский океан в ZPrePass, где Z ставится в 1.0 если в данной точке океан виден
			// нужно чтобы океан не ездил вверх вниз, а всегда был на одной высоте
			m_oceans->pVarDepthTexture->SetTexture(m_oceans->pOceanRT->AsTexture());
			ZPrePass(false);
		}

		// FIX-ME: восстанавливаем прямоугольник color target'a если находимся в режиме редактора
		//if (IsEditModeOn())
		//{
			float sx = oldViewport.X / float(screenInfo.dwWidth);
			float sy = oldViewport.Y / float(screenInfo.dwHeight);
			float dx = oldViewport.Width / float(screenInfo.dwWidth);
			float dy = oldViewport.Height / float(screenInfo.dwHeight);
			varRestoreColorTexture->SetTexture(curRT->AsTexture());
			varRestoreColorParams->SetVector4(Vector4(sx, sy, dx, dy));
			Render().DrawFullScreenQuad(float(m_oceans->m_oceanRTWidth), float(m_oceans->m_oceanRTHeight), PCRestoreColor_id);
		//}
		//else
		//{
			//Render().Clear(0, null, CLEAR_TARGET, 0x0, 1.0f, 0x0);
		//}

#endif

		m_refractor.Ptr()->Reflecting(fDeltaTime, level);
		reflectionExist = true;

#ifdef _XBOX
		Render().DisableHyperZ(false);
		//Первый раз после запуска/остановки океанов - чистим TARGET  
		/*if (m_oceans->m_isOceansHaveFakeHeight)
			Render().Clear(0, null, CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0f, 0x0);*/

		// восстанавливаем COLOR в рендер таргете, из-за рефлектора
		if (pEDRAM_Color)
		{
			pEDRAM_Color->SetTexture(curRT->AsTexture());
			Render().DrawFullScreenQuad(m_oceans->m_oceanRTWidth, m_oceans->m_oceanRTHeight, X360RestoreEDRAM_ColorOnly_id);
		}

		// рисуется плоский океан в ZPrePass, где Z ставится в 1.0 если в данной точке океан виден
		// нужно чтобы океан не ездил вверх вниз, а всегда был на одной высоте
		m_oceans->pVarDepthTexture->SetTexture(curRTDepth->AsTexture());
		ZPrePass(false);
		Render().FlushHiZStencil(false); 

#else
		if (isClearZ)
			Render().Clear(0, null, CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0f, 0x0);
		//Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0f, 0x0);

		// рисуем небо
		for (m_reflectorIterator->Reset(); !m_reflectorIterator->IsDone(); m_reflectorIterator->Next())
		{
			MissionObject * mo = m_reflectorIterator->Get();
			
			// FIX-ME - убрать проверку по Is
			MO_IS_IF_NOT(tid, "Sky", mo)
			{
				continue;
			}

			m_reflectorIterator->ExecuteEvent();
			break;
		}
#endif

		// рисуем океан и куски вод для плоского отображения
		for (iter->Reset(); !iter->IsDone(); iter->Next())
		{
			Ocean2 * ocean = (Ocean2*)iter->Get();
			if (!ocean->IsWorking(true)) continue;

			if (!ocean->IsRealWavesEnable())
				ocean->DrawRealOceanAndParts(fDeltaTime, oldViewport);
		}

		// заканчиваем отрисовку и восстанавливаем старые рендертаргеты
#ifdef _XBOX
		Render().ResolveColor(m_oceans->pOceanRT->AsTexture());
		//Render().EndScene(null, true, true);
		/*if (m_oceans->m_isOceansHaveFakeHeight)
		{
			if (pEDRAM_Color && pEDRAM_Depth)
			{
				pEDRAM_Color->SetTexture(curRT->AsTexture());
				pEDRAM_Depth->SetTexture(curRTDepth->AsTexture());
				Render().DrawFullScreenQuad(m_oceans->m_oceanRTWidth, m_oceans->m_oceanRTHeight, X360RestoreEDRAM_id);
			}
		}
		else*/
		{
			// восстанавливаем COLOR в рендер таргете, z-buffer должен быть без изменений
			if (pEDRAM_Color && pEDRAM_Depth)
			{
				pEDRAM_Color->SetTexture(curRT->AsTexture());
				pEDRAM_Depth->SetTexture(curRTDepth->AsTexture());
				Render().DrawFullScreenQuad(m_oceans->m_oceanRTWidth, m_oceans->m_oceanRTHeight, X360RestoreEDRAM_id);
				Render().FlushHiZStencil(true);
			}
			/*if (pEDRAM_Color)
			{
				pEDRAM_Color->SetTexture(curRT->AsTexture());
				Render().DrawFullScreenQuad(m_oceans->m_oceanRTWidth, m_oceans->m_oceanRTHeight, X360RestoreEDRAM_ColorOnly_id);
			}*/
		}
#else
		Render().EndScene(null, false, true);

		// восстанавливаем рендертаргет/вьюпорт
		Render().PopRenderTarget(RTO_RESTORE_CONTEXT);
		Render().BeginScene();
#endif
		Render().PopViewport();
	}

	// рисуем плоские поверхности, и реальные если они есть
	if (numFlat || numRealWaves)
	{
		if (!reflectionExist)
		{
			// сохраняем COLOR
			Render().EndScene(null, false, true);

			// восстанавливаем основной рендертаргет
			Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
			Render().BeginScene();

			#ifdef _XBOX
				Render().DisableHyperZ(true);
			#endif

			// выполняем рефлектор, который портит EDRAM в области COLOR у главного рендер таргета
			// и портит HI-Z 
			m_refractor.Ptr()->Reflecting(fDeltaTime, level);
			reflectionExist = true;

			#ifdef _XBOX
				Render().DisableHyperZ(false);
				// восстанавливаем COLOR в рендер таргете, из-за рефлектора
				if (pEDRAM_Color)
				{
					pEDRAM_Color->SetTexture(curRT->AsTexture());
					Render().DrawFullScreenQuad(m_oceans->m_oceanRTWidth, m_oceans->m_oceanRTHeight, X360RestoreEDRAM_ColorOnly_id);
				}
			#endif
		}

		Render().SetViewport(oldViewport);

		for (iter->Reset(); !iter->IsDone(); iter->Next())
		{
			Ocean2 * ocean = (Ocean2*)iter->Get();
			if (!ocean->IsWorking(true)) continue;

			if (ocean->IsRealWavesEnable())
				ocean->DrawRealOceanAndParts(fDeltaTime, oldViewport);
			else
				ocean->DrawFlatOceanAndParts();
		}
	}

#ifdef _XBOX
		Render().ResolveColor(m_oceans->pOceanRT->AsTexture());
#endif

	/*Render().Print(5, 105, 0xFFFFFFFF, "t1 = %d, %.1f ms", t1, float(t1) / fTicksPerMillisecond);
	Render().Print(5, 125, 0xFFFFFFFF, "t2 = %d, %.1f ms", t2, float(t2) / fTicksPerMillisecond);
	Render().Print(5, 145, 0xFFFFFFFF, "t3 = %d, %.1f ms", t3, float(t3) / fTicksPerMillisecond);
	Render().Print(5, 165, 0xFFFFFFFF, "t4 = %d, %.1f ms", t4, float(t4) / fTicksPerMillisecond);
	Render().Print(5, 185, 0xFFFFFFFF, "t5 = %d, %.1f ms", t5, float(t5) / fTicksPerMillisecond);
	Render().Print(5, 205, 0xFFFFFFFF, "t6 = %d, %.1f ms", t6, float(t6) / fTicksPerMillisecond);*/

	Render().pixEndEvent(_FL_, dwId0);
}


void Ocean3Executor::Refracting()
{
	MGIterator * iter = m_executorIterator;
	// рисуем плоские воды и океаны
	for (iter->Reset(); !iter->IsDone(); iter->Next())
	{
		Ocean2 * ocean = (Ocean2*)iter->Get();
		if (!ocean->IsWorking(true)) continue;

		ocean->DrawFlatOceanAndParts(false, true);
	}
}

/*float sine(float x)
{
    const float B = 4/pi;
    const float C = -4/(pi*pi);

    float y = B * x + C * x * abs(x);

    #ifdef EXTRA_PRECISION
    //  const float Q = 0.775;
        const float P = 0.225;

        y = P * (y * abs(y) - y) + y;   // Q * y + P * y * abs(y)
    #endif
}*/

MOP_BEGINLIST(Ocean3Executor, "", '1.00', 101)
MOP_ENDLIST(Ocean3Executor);
