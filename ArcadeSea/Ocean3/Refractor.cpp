#include "Ocean.h"

Ocean3Refractor::Ocean3Refractor()
{
	m_refCount = 0;
	m_refractorIterator = null;
	m_reflectorIterator = null;
	m_executorIterator = null;

	m_reflectAlreadyCleared = false;
	m_refractAlreadyCleared = false;

	m_executor.Reset();
}

Ocean3Refractor::~Ocean3Refractor()
{
	RELEASE(m_refractorIterator);
	RELEASE(m_reflectorIterator);
	RELEASE(m_executorIterator);
}

bool Ocean3Refractor::Create(MOPReader & reader)
{
	m_oceans = (Ocean3Service *)api->GetService("Ocean3Service");
	static const ConstString id_Ocean3Executor("Ocean3Executor");
	Mission().CreateObject(m_executor.GetSPObject(), "Ocean3Executor", id_Ocean3Executor, true);

	m_executorIterator = &Mission().GroupIterator(OCEAN_GROUP3, _FL_);
	m_refractorIterator = &Mission().GroupIterator(MG_SEAREFRACTION, _FL_);
	m_reflectorIterator = &Mission().GroupIterator(MG_SEAREFLECTION, _FL_);

	return true;
}

void Ocean3Refractor::AddRef()
{
	// если это первый океан, то добавляем функции для выполнения
	if (m_refCount == 0)
	{
		//SetUpdate(&Ocean3Refractor::Reflecting, ML_SHADOWCAST - 10);
		SetUpdate(&Ocean3Refractor::Refracting, ML_FILL - 10);
	}

	m_refCount++;
}

void Ocean3Refractor::DecRef()
{
	Assert(m_refCount);

	// если это последний океан, то удаляем функции для выполнения
	if (m_refCount == 1)
	{
		//DelUpdate(&Ocean3Refractor::Reflecting);
		DelUpdate(&Ocean3Refractor::Refracting);
	}

	m_refCount--;
}

void Ocean3Refractor::MakeReflection(IRenderTarget * color, IRenderTargetDepth * depth, float height, bool reflectParticles, bool restoreContext)
{
	float delta = 0.1f;

	dword dwId0 = Render().pixBeginEvent(_FL_, "Ocean2::Reflection");

	Matrix mView = Render().GetView();
	Matrix mProjection = Render().GetProjection();

	Plane p = Plane(Vector(0.0f, -1.0f, 0.0f), Vector(0.0f, (height + delta), 0.0f));
	Matrix mReflect = Matrix().BuildMirror(p.n.x, p.n.y, p.n.z, p.d);

	// устанавливаем зеркальную view матрицу
	Render().SetView(mReflect * mView);

	Matrix transform = mView * mProjection;
	transform.InverseComplette4X4();
	transform.Transposition();

	p = Plane(Vector(0.0f, -1.0f, 0.0f), Vector(0.0f, -(height + delta), 0.0f));
	Plane clipPlane;
	Vector4 r4 = transform * Vector4(p.n.x, p.n.y, p.n.z, p.d);
	clipPlane.n = r4.v;
	clipPlane.d = r4.w;
	clipPlane.Normalize();

	//Завершаем текушую отрисовку
	Render().PushViewport();
	Render().EndScene(null, !restoreContext);
	Render().PushRenderTarget();

	//Ставим свой render target и стартуем отрисовку в текстуру отражения
	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, color, depth);
	Render().BeginScene();

	RENDERVIEWPORT vp;
	vp.Width = m_oceans->rr_size;	vp.Height = m_oceans->rr_size;
	vp.X = 0;						vp.Y = 0;
	vp.MinZ = 0.0f;					vp.MaxZ = 1.0f;
	Render().SetViewport(vp);

#ifdef _XBOX
	Render().SetStreamSource(0, m_oceans->m_halfScreenCopyVB);
	Render().DrawPrimitive(m_oceans->m_clearRTDepthID, PT_QUADLIST, 0, 1);
	//Render().DrawFullScreenQuad(float(m_oceans->rr_size), float(m_oceans->rr_size), m_oceans->m_clearRTDepthID);
#else
	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0f, 0x0);
#endif

	long cullOld = Render().GetStateHack(SH_CULLMODE);
	long clipOld = Render().GetStateHack(SH_CLIPPLANEENABLE);

	Render().SetStateHack(SH_CULLMODE, SHCM_INVERSE);
	Render().SetStateHack(SH_CLIPPLANEENABLE, 1);

	// устанавливаем клип план отсекающий подводную часть объектов
	Render().SetClipPlane (0, clipPlane);

	for (m_reflectorIterator->Reset(); !m_reflectorIterator->IsDone(); m_reflectorIterator->Next())
	{
		MissionObject * mo = m_reflectorIterator->Get();
		
		// FIX-ME - убрать проверку по Is
		MO_IS_IF(tid, "Sky", mo)
		{
			continue;
		}

		m_reflectorIterator->ExecuteEvent();
	}

	// восстанавливаем cullmode перед отрисовкой партиклов
	Render().SetStateHack(SH_CULLMODE, cullOld);

	//
	if (reflectParticles)
		Particles().Draw();

	//Завершаем отрисовку в текстуру отражения
	Render().EndScene(null, false, true);

	//Восстанавливаем старые render target/depth surface
	Render().PopRenderTarget((restoreContext) ? RTO_RESTORE_CONTEXT : RTO_DONTOCH_CONTEXT);

	Render().SetStateHack(SH_CLIPPLANEENABLE, clipOld);

	//Стартуем отрисовку опять
	Render().BeginScene();
	Render().PopViewport();
	Render().SetView(mView);

	Render().pixEndEvent(_FL_, dwId0);
}

// Отрисовываем отражение
void _cdecl Ocean3Refractor::Reflecting(float deltaTime, long level)
{
	if (api->DebugKeyState('O')) return;

	// ищем в текущих активных океанах высоту и включены ли отражения партиклов
	float height = 0.0f;
	bool heightFixed = false;
	bool reflectParticles = false;
	for (m_executorIterator->Reset(); !m_executorIterator->IsDone(); m_executorIterator->Next())
	{
		Ocean2 * ocean = (Ocean2 *)m_executorIterator->Get();

		if (!ocean->IsWorking(true))
			continue;

		if (ocean->IsReflectParticles())
			reflectParticles = true;

		// смотрим высоту отражения
		float curHeight = ocean->GetHeight();
		if (!heightFixed || ocean->IsMainReflection()) 
		{
			if (ocean->IsBigOceanActive() || ocean->IsMainReflection()) 
			{
				height = curHeight;
				heightFixed = true;
			}
			else 
				height = curHeight;
		}
	}

	bool reflectObjects = false;
	for (m_reflectorIterator->Reset(); !m_reflectorIterator->IsDone(); m_reflectorIterator->Next())
	{
		MissionObject * mo = m_reflectorIterator->Get();
		if (mo->IsShow())
		{
			reflectObjects = true;
			break;
		}
	}

	if (!reflectObjects && !reflectParticles)
	{
		if (m_reflectAlreadyCleared)
			return;

		Render().EndScene(null, true);
		Render().PushRenderTarget();
		Render().PushViewport();
		
		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_oceans->pReflectionTexture, null);
		Render().BeginScene();
		Render().Clear(0, null, CLEAR_TARGET, 0x0, 1.0f, 0x0);
		Render().EndScene();
		
		Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
		Render().BeginScene();
		Render().PopViewport();

		m_reflectAlreadyCleared = true;
		return;
	}

	m_reflectAlreadyCleared = false;


	MakeReflection(m_oceans->pReflectionTexture, m_oceans->pReflectionSurfaceDepth, height, reflectParticles, false);
}

bool _cdecl Ocean3Refractor::Refracting(float deltaTime, long level)
{
	if (api->DebugKeyState('O')) return false;

	// проверим, если все активные океаны это зеркала, то рефракшн не нужен
	bool allMirrors = true;
	for (m_executorIterator->Reset(); !m_executorIterator->IsDone(); m_executorIterator->Next())
	{
		Ocean2 * ocean = (Ocean2 *)m_executorIterator->Get();

		if (!ocean->IsWorking(true))
			continue;

		if (!ocean->IsAllMirrors())
		{
			allMirrors = false;
			break;
		}
	}

	if (allMirrors && m_refractAlreadyCleared)
		return false;

	dword dwId0 = Render().pixBeginEvent(_FL_, "Ocean2::Refraction");

	MGIterator * iter = m_refractorIterator;

	Render().EndScene(null, true);
	Render().PushViewport();
	Render().PushRenderTarget();
	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_oceans->pRefractRT, m_oceans->pRefractRTDepth);
	Render().BeginScene();
	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0f, 0);

	RENDERVIEWPORT vp;
	vp.Width = m_oceans->rr_size;	vp.Height = m_oceans->rr_size;
	vp.X = 0;						vp.Y = 0;
	vp.MinZ = 0.0f;					vp.MaxZ = 1.0f;
	Render().SetViewport(vp);

	if (!allMirrors)
	{
		for (m_refractorIterator->Reset(); !m_refractorIterator->IsDone(); m_refractorIterator->Next())
			m_refractorIterator->ExecuteEvent();

		// чистим альфу
		Render().DrawFullScreenQuad((float)m_oceans->rr_size, (float)m_oceans->rr_size, m_oceans->m_shaderClearAlpha);

		// рисуем плоские воды и плоские океаны альфа = 1.0f
		m_executor.Ptr()->Refracting();

		m_refractAlreadyCleared = false;
	}
	else
		m_refractAlreadyCleared = true;

	Render().EndScene(null, false, true);
	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
	Render().BeginScene();
	Render().PopViewport();

	Render().pixEndEvent(_FL_, dwId0);

	return true;
}


MOP_BEGINLIST(Ocean3Refractor, "", '1.00', 101)
MOP_ENDLIST(Ocean3Refractor);
