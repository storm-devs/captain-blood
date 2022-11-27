#include "Ocean.h"

void Ocean2::CubeMap_GetSideMatrix(long face, Matrix & view)
{
	Vector lookTo, up;

    switch (face)
    {
        case CUBEMAP_FACE_POSITIVE_X:
            lookTo = Vector(1.0f, 0.0f, 0.0f);
            up = Vector(0.0f, 1.0f, 0.0f);
            break;
        case CUBEMAP_FACE_NEGATIVE_X:
            lookTo = Vector(-1.0f, 0.0f, 0.0f);
            up = Vector(0.0f, 1.0f, 0.0f);
            break;
        case CUBEMAP_FACE_POSITIVE_Y:
            lookTo = Vector(0.0f, 1.0f, 0.0f);
            up = Vector(0.0f, 0.0f,-1.0f);
            break;
        case CUBEMAP_FACE_NEGATIVE_Y:
            lookTo = Vector(0.0f,-1.0f, 0.0f);
            up = Vector(0.0f, 0.0f, 1.0f);
            break;
        case CUBEMAP_FACE_POSITIVE_Z:
            lookTo = Vector(0.0f, 0.0f, 1.0f);
            up = Vector(0.0f, 1.0f, 0.0f);
            break;
        case CUBEMAP_FACE_NEGATIVE_Z:
            lookTo = Vector(0.0f, 0.0f, -1.0f);
            up = Vector(0.0f, 1.0f, 0.0f);
            break;
    }

	view.BuildView(0.0f, lookTo, up);
}

void Ocean2::GenerateSphereMap()
{
	struct NetVertex
	{
		Vector pos;
		float tu, tv;
	};

	Matrix m[6];

	m[0].Build(0.0f, 0.0f, -PI/2.0, 0.5f, 0.5f, 0.0f);	// +x
	m[1].Build(0.0f, 0.0f, PI/2.0, -0.5f, 0.5f, 0.0f);	// -x
	m[2].Build(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);		// +y
	m[3].Build(0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f);	// -y
	m[4].Build(-PI/2.0, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f);	// +z
	m[5].Build(PI/2.0, 0.0f, 0.0f, 0.0f, 0.5f, -0.5f);	// -z

	MGIterator & giterator = Mission().GroupIterator(GroupId('S','M','G','n'), _FL_);
	
	Assert(m_oceans->sphereRT);

	Matrix oldView = Render().GetView();
	Matrix oldProj = Render().GetProjection();

	Render().PushRenderTarget();
	Render().EndScene();
	// рендерим и копируем 6 плоскостей кубмапа
	bool isFirstDraw = true;

	RENDERVIEWPORT oldVP = Render().GetViewport();

	RENDERVIEWPORT vp;

	for (long i=0; i<6; i++)
	{
		if (i==3) continue;

		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT, m_oceans->pReflectionTexture, m_oceans->pReflectionSurfaceDepth);
		Render().BeginScene();
		Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0x0, 1.0f, 0x0);

		vp.X = 0;						vp.Y = 0;
		vp.Height = m_oceans->rr_size;	vp.Width = m_oceans->rr_size;
		vp.MinZ = 0.0f;					vp.MaxZ = 1.0f;
		Render().SetViewport(vp);

		// ставим view матрицу
		Matrix view;
		Matrix proj;
		CubeMap_GetSideMatrix(i, view);
		proj.BuildProjection(PI/2.0f, float(m_oceans->rr_size), float(m_oceans->rr_size), 0.1f, 4000.0f);
		Render().SetView(view);
		Render().SetProjection(proj);
		Render().SetWorld(Matrix().SetIdentity());

		// рендерим небо
		for (giterator.Reset(); !giterator.IsDone(); giterator.Next())
		{
			MissionObject * mo = giterator.Get();
			MO_IS_IF(tid, "Sky", mo)
			{
				giterator.ExecuteEvent();
			}
		}
		Render().EndScene();
	
		// рендерим сетку в сферу
		Render().SetRenderTarget(RTO_RESTORE_CONTEXT, m_oceans->sphereRT, null);
		Render().BeginScene();
		if (isFirstDraw)
			Render().Clear(0, null, CLEAR_TARGET, 0x0, 1.0f, 0x0);
		isFirstDraw = false;

		long sizex = 512;
		long sizey = 512;

		vp.X = 0;			vp.Y = 0;
		vp.Width = sizex;	vp.Height = sizey;
		vp.MinZ = 0.0f;		vp.MaxZ = 1.0f;
		Render().SetViewport(vp);

		boolVector32 boolParams;
		boolParams.set(0, i == 1);	// NX
		boolParams.set(1, i == 0);	// PX
		boolParams.set(2, i == 2);	// PY
		boolParams.set(3, i == 5);	// NZ
		boolParams.set(4, i == 4);	// PZ
		m_oceans->varSkyDomeBools->SetBool32(boolParams);

		m_oceans->varSkyDomeTexture->SetTexture(m_oceans->pReflectionTexture->AsTexture());

		Render().DrawFullScreenQuad(float(sizex), float(sizey), m_oceans->SkyDome_id);

		Render().EndScene();
	}
 
	Render().PopRenderTarget(RTO_RESTORE_CONTEXT);
	Render().BeginScene();
	Render().SetViewport(oldVP);
	Render().SetView(oldView);
	Render().SetProjection(oldProj);

	giterator.Release();
}