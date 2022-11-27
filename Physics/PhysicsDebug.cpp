#ifndef STOP_DEBUG

#include "PhysicsScene.h"
#include "PhysicsService.h"
#include "..\Common_h\Render.h"
#include "..\Common_h\Mission.h"

// Moller-Trumbore: ray intersect with triangle(only hit check)
bool IntersectTriangle(const Vector & r0, const Vector & dir, const Vector & v0, const Vector & v1, const Vector & v2, Vector & vRes)
{
	Vector edge1, edge2, tvec, pvec, qvec;
	float det, inv_det;

	/* find vectors for two edges sharing vert0 */
	edge1 = v1 - v0;
	edge2 = v2 - v0;

	/* begin calculating determinant - also used to calculate U parameter */
	pvec = dir ^ edge2;

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1 | pvec;

	if (det > -0.000001 && det < 0.000001) return false;

	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	tvec = r0 - v0;

	Vector n = edge1 ^ edge2;
    float a = -(n | tvec);
    float b = (n | dir);
    if (fabs(b) < 0.000001) return false;
    float r = a / b;
    if (r < 0.0) return false;
    vRes = r0 + r * dir;           // intersect point of ray and plane

	float u, v, t;
	/* calculate U parameter and test bounds */
	u = (tvec | pvec) * inv_det;
	if (u < 0.0 || u > 1.0) return false;

	/* prepare to test V parameter */
	qvec = tvec ^ edge1;

	/* calculate V parameter and test bounds */
	v = (dir | qvec) * inv_det;
	if (v < 0.0 || u + v > 1.0) return false;

	/* calculate t, ray intersects triangle */
	t = (edge2 | qvec) * inv_det;
	return true;
}

#ifndef STOP_DEBUG
void PhysicsScene::Debug_DrawMaterialsPatches()
{
	long iMaterialShow = m_service->m_gMaterialIndex;

	array<Vector> aTrgs(_FL_, 8192);
	array<PhysTriangleMaterialID> aMats(_FL_, 32);
	OverlapSphere(0.0, 10000.0f, 0xFFFFFFFF, true, true, aTrgs, &aMats);

	IRender * render = (IRender *)api->GetService("DX9Render");
	IVariable * pVColor = render->GetTechniqueGlobalVariable("Phys_v4Color", _FL_);

	Matrix mView = render->GetView();
	Matrix mIView = Matrix(mView).Inverse();
	Vector vCamPos = mView.GetCamPos();
	Vector r0 = vCamPos;
	Vector r1 = vCamPos + mIView.MulNormal(Vector(0.0f, 0.0f, 1000.0f));

	struct temp_t
	{
		temp_t() : aTrgs(_FL_, 8192) 
		{
			bInited = false;
			dwColor = 0xFFFFFFFF;
		};
		
		array<Vector>	aTrgs;
		dword			dwColor;
		string			sType;
		bool			bInited;
	};

	array<temp_t> aTemp(_FL_, 32);
	dword dwIntersectMat = -1, dwIntersectTri = -1;
	float fMinDistance = 1e+10f;

#define MAX_MATERIALS	64

	aTemp.Empty();
	for (long j=0; j<=MAX_MATERIALS; j++) aTemp.Add();

	for (long i=0; i<aTrgs.Len() / 3; i++)
	{
		long mat = aMats[i];
		if (mat > MAX_MATERIALS) continue;	// Мелкий хак, все равно если материалов больше 64 - значит кто-то зажрался!

		temp_t & t = aTemp[mat];
		if (!t.bInited)
		{
			t.bInited = true;
			t.dwColor = ARGB(255, 25, 25, 25);
			t.sType = "Unknown: " + mat;

			switch ((PhysTriangleMaterialID)mat)
			{
				case pmtlid_air:		t.dwColor = ARGB(255, 0, 210, 255); t.sType = "Air"; break;
				case pmtlid_ground:		t.dwColor = ARGB(255, 110, 148, 92); t.sType = "Ground"; break;
				case pmtlid_stone:		t.dwColor = ARGB(255, 206, 206, 206); t.sType = "Stone"; break;
				case pmtlid_sand:		t.dwColor = ARGB(255, 237, 255, 211); t.sType = "Sand"; break;
				case pmtlid_wood:		t.dwColor = ARGB(255, 184, 140, 87); t.sType = "Wood"; break;
				case pmtlid_grass:		t.dwColor = ARGB(255, 47, 214, 55); t.sType = "Grass"; break;
				case pmtlid_water:		t.dwColor = ARGB(255, 0, 180, 255); t.sType = "Water"; break;
				case pmtlid_iron:		t.dwColor = ARGB(255, 200, 200, 200); t.sType = "Iron"; break;
				case pmtlid_fabrics:	t.dwColor = ARGB(255, 228, 228, 228); t.sType = "Fabrics"; break;
				case pmtlid_other1:		t.dwColor = ARGB(255, 80, 80, 80); t.sType = "Other1"; break;
				case pmtlid_other2:		t.dwColor = ARGB(255, 80, 80, 80); t.sType = "Other2"; break;
				case pmtlid_other3:		t.dwColor = ARGB(255, 80, 80, 80); t.sType = "Other3"; break;
				case pmtlid_other4:		t.dwColor = ARGB(255, 80, 80, 80); t.sType = "Other4"; break;
			}
		}
	
		t.aTrgs.Add(aTrgs[i * 3 + 0]);
		t.aTrgs.Add(aTrgs[i * 3 + 1]);
		t.aTrgs.Add(aTrgs[i * 3 + 2]);

		Vector vRes;
		/*if (IntersectTriangle(r0, (r1 - r0), aTrgs[i * 3 + 0], aTrgs[i * 3 + 1], aTrgs[i * 3 + 2], vRes))
		{
			float fDistance = ~(vRes - vCamPos);
			if (fDistance < fMinDistance)
			{
				fMinDistance = fDistance;
				dwIntersectMat = mat;
				dwIntersectTri = t.aTrgs.Last();
			}
		}*/
	}

	if (iMaterialShow >= 0 && iMaterialShow > (long)aTemp.Last()) iMaterialShow = (long)aTemp.Last();

	render->SetWorld(Matrix());
	render->FlushBufferedLines();
	for (long i=0, k=0; i<aTemp.Len(); i++)
	{
		temp_t & t = aTemp[i];
		if (t.aTrgs.IsEmpty()) continue;
		if (iMaterialShow >= 0 && iMaterialShow != k) 
		{
			k++;
			continue;
		}

		dword dwColor = t.dwColor;
		Color c(dwColor);

		// рисуем треугольники
		pVColor->SetVector4((c * 0.8f).v4);

		ShaderId id;
		render->GetShaderId("dbgPhysPolygons", id);
		render->DrawPrimitiveUP(id, PT_TRIANGLELIST, t.aTrgs.Size() / 3, t.aTrgs.GetBuffer(), sizeof(Vector));

		// добавляем линии сверху
		for (long j=0; j<t.aTrgs.Len() / 3; j++)
		{
			render->DrawBufferedLine(t.aTrgs[j * 3 + 0], dwColor, t.aTrgs[j * 3 + 1], dwColor, false, "dbgPhysLine");
			render->DrawBufferedLine(t.aTrgs[j * 3 + 1], dwColor, t.aTrgs[j * 3 + 2], dwColor, false, "dbgPhysLine");
			render->DrawBufferedLine(t.aTrgs[j * 3 + 2], dwColor, t.aTrgs[j * 3 + 0], dwColor, false, "dbgPhysLine");
		}
		k++;
	}
	render->FlushBufferedLines(true, "dbgPhysLine");

	render->Print(20.0f, 30.0f, 0xFFFFFFFF, "Current material: %s", (dwIntersectMat != -1 && dwIntersectTri != -1) ? aTemp[dwIntersectMat].sType.c_str() : "None");
	//render->Print(20.0f, 50.0f, 0xFFFFFFFF, "Use '+' and '-' keys to select single material");
	render->Print(20.0f, 75.0f, 0xFFFFFFFF, "Materials found:");
	for (long i=0, k=0; i<aTemp.Len(); i++)
	{
		temp_t & t = aTemp[i];
		if (t.aTrgs.IsEmpty()) continue;
		dword dwColor = 0xFFFFFFFF;
		if (iMaterialShow >= 0 && iMaterialShow != k) dwColor = 0xFF6F6F6F;
		render->Print(38.0f, 95.0f + k * 20.0f, dwColor, "%d. %s : %d triangles.", k, t.sType.c_str(), t.aTrgs.Size() / 3);

		dwColor = t.dwColor;
		RS_SPRITE spr[4];
		float dx = 1024.0f;
		float dy = 768.0f;
		float x1 = (2.0f * 23.0f / dx) - 1.0f;
		float x2 = (2.0f * 35.0f / dx) - 1.0f;
		float y1 = 1.0f - (2.0f * (98.0f + k * 20.0f) / dy);
		float y2 = 1.0f - (2.0f * (12.0f + 98.0f + k * 20.0f) / dy);
		spr[0].vPos = Vector (x1, y1, 0.0f);
		spr[1].vPos = Vector (x2, y1, 0.0f);
		spr[2].vPos = Vector (x2, y2, 0.0f);
		spr[3].vPos = Vector (x1, y2, 0.0f);

		spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
		spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
		spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
		spr[3].tu = 0.0f;  spr[3].tv = 1.0f;

		spr[0].dwColor = dwColor;
		spr[1].dwColor = dwColor;
		spr[2].dwColor = dwColor;
		spr[3].dwColor = dwColor;

		render->DrawSprites(null, &spr[0], 1);

		k++;
	}

	RS_RECT r;
	r.dwColor = 0xFFFFFFFF;
	r.dwSubTexture = 0;
	r.fAngle = 0.0f;
	r.fSizeX = 0.0005f; r.fSizeY = 0.0005f;
	r.vPos = mIView.MulVertex(Vector(0.0f, 0.0f, 0.2f));
	render->SetWorld(Matrix());
	render->DrawRects(null, &r, 1);
	if (dwIntersectMat != -1 && dwIntersectTri != -1)
		render->Print(mIView.MulVertex(Vector(0.0f, 0.0f, 7.0f)), 10.0f, -0.9f, 0xFFFFFFFF, aTemp[dwIntersectMat].sType.c_str());

	pVColor = NULL;
}

void PhysicsScene::DebugFrame()
{
	IRender * render = (IRender *)api->GetService("DX9Render");

	for (int i=0; i<m_spheres.Len(); i++)
		render->DrawSphere(m_spheres[i], 0.2f, 0xFF00FF00);

	if (api->DebugKeyState('I'))
		m_spheres.Empty();

	if (m_service->m_gShowColliders)
		Debug_DrawMaterialsPatches();

	if (m_service->m_gShowPhysBoxes)
	{
		array<Object*> objs(_FL_);
		api->FindObject("CameraController", objs);

		for (long i=0; i<objs.Len(); i++)
		{
			MissionObject * camCon = (MissionObject *)objs[i];
			dword numPhysObjs = camCon->QTFindObjects(MG_AI_COLLISION, -Vector(10000.0f), Vector(10000.0f));
			for (dword j=0; j<numPhysObjs; j++)
			{
				IMissionQTObject * fo = camCon->QTGetObject(j);
				if (!fo) continue;
				const Vector & boxCenter = fo->GetBoxCenter();
				Vector boxSize05 = fo->GetBoxSize()*0.5f;
				Matrix mtx(fo->GetMatrix());
				mtx.pos = mtx*boxCenter;
				
				render->DrawBox(-boxSize05, boxSize05, mtx);
			}
		}
	}

	if (m_service->m_gShowWorldPoint)
	{
		static float coeff = 0.0f;

		const Vector & pos = m_service->m_gWorldPoint;

		render->DrawSphere(pos, 0.1f);
		float fDeltaTime = api->GetDeltaTime();

		coeff += fDeltaTime;
		coeff -= 2.0f * long(coeff / 2.0f);
		float dist0 = 0.2f + ((coeff < 1.0f) ? coeff : 2.0f - coeff);
		float dist1 = dist0 + 2.5f;
		
		render->DrawVector(pos + Vector(0.0f, dist1, 0.0f), pos + Vector(0.0f, dist0, 0.0f), 0xFF00FF00);
		render->DrawVector(pos + Vector(0.0f, -dist1, 0.0f), pos + Vector(0.0f, -dist0, 0.0f), 0xFF00FF00);

		render->DrawVector(pos + Vector(dist1, 0.0f, 0.0f), pos + Vector(dist0, 0.0f, 0.0f), 0xFF00FF00);
		render->DrawVector(pos + Vector(-dist1, 0.0f, 0.0f), pos + Vector(-dist0, 0.0f, 0.0f), 0xFF00FF00);

		render->DrawVector(pos + Vector(0.0f, 0.0f, dist1), pos + Vector(0.0f, 0.0f, dist0), 0xFF00FF00);
		render->DrawVector(pos + Vector(0.0f, 0.0f, -dist1), pos + Vector(0.0f, 0.0f, -dist0), 0xFF00FF00);
	}
}
#endif

#endif
