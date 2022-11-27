#include "..\pch.h"
#include "FakeSailRender.h"

//////////////////////////////////////////////////////////////////////////
// FakeRender
//////////////////////////////////////////////////////////////////////////


FakeRender::FakeRender() :
vb_(NULL),
ib_(NULL),
varDiffuseTex_(NULL),
varGlimpseTex_(NULL),
//varInstanceInfo_(NULL),
varSailFreqTimeAmp_(NULL),
varSailColorAlpha_(NULL),
varSailGlimpseColorScale_(NULL),
//varFaceOrder_(NULL),
diffuseTex_(NULL),
glimpseTex_(NULL),
vertCount_(0),
indCount_(0),
m_nHorzQuantity(0),
m_nVertQuantity(0)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	varDiffuseTex_ = render->GetTechniqueGlobalVariable("sailTexture", __FILE__, __LINE__);
	Assert(varDiffuseTex_);
	varGlimpseTex_ = render->GetTechniqueGlobalVariable("glimpseTexture", __FILE__, __LINE__);
	Assert(varGlimpseTex_);
//	varFaceOrder_ = render->GetTechniqueGlobalVariable("faceOrder", __FILE__, __LINE__);
//	Assert(varFaceOrder_);
	//varInstanceInfo_ = render->GetTechniqueGlobalVariable("SailInfo", __FILE__, __LINE__);
	//Assert(varInstanceInfo_);
	varSailFreqTimeAmp_ = render->GetTechniqueGlobalVariable("sailFreqTimeAmp", __FILE__, __LINE__);
	Assert(varSailFreqTimeAmp_);
	varSailColorAlpha_ = render->GetTechniqueGlobalVariable("sailColorAlpha", __FILE__, __LINE__);
	Assert(varSailColorAlpha_);
	varSailGlimpseColorScale_ = render->GetTechniqueGlobalVariable("sailGlimpseColorScale", __FILE__, __LINE__);
	Assert(varSailGlimpseColorScale_);

	data_.time = 0.f;
	fTimePeriod = 100.f;

	m_bIsTriangle = false;
}

FakeRender::~FakeRender()
{
	if (vb_)
		vb_->Release(), vb_ = NULL;

	if (ib_)
		ib_->Release(), ib_ = NULL;

	if (diffuseTex_)
		diffuseTex_->Release(), diffuseTex_ = NULL;

	if (glimpseTex_)
		glimpseTex_->Release(), glimpseTex_ = NULL;

	varDiffuseTex_ = NULL;
	varGlimpseTex_ = NULL;
//	varFaceOrder_ = NULL;

	//if (varInstanceInfo_)
	//	varInstanceInfo_->Release(), varInstanceInfo_ = NULL;
	varSailFreqTimeAmp_ = NULL;
	varSailColorAlpha_ = NULL;
	varSailGlimpseColorScale_ = NULL;
}

// создать и заполнить буферы
void FakeRender::InitBuffers(const IClothMeshBuilder& mesh)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	//render->GetShaderId("FakeSailCW", FakeSailCW_id);
	render->GetShaderId("FakeSailCCW", FakeSailCCW_id);

	if (vb_)
		vb_->Release(), vb_= NULL;
	if (ib_)
		ib_->Release(), ib_= NULL;

	const IClothMeshBuilder::Vertex* vb = NULL;
	const unsigned short* ib = NULL;

	mesh.GetCombinedBuffers(vb, ib, vertCount_, indCount_);

	vb_ = render->CreateVertexBuffer(vertCount_*sizeof(Vertex), sizeof(Vertex), __FILE__, __LINE__ );
	ib_ = render->CreateIndexBuffer(indCount_*sizeof(unsigned short), __FILE__, __LINE__ );

	Assert(vb_);
	Assert(ib_);

	// ищем минимум/максимум по осям
	float xMin = FLT_MAX, xMax = -FLT_MAX;
	float yMin = FLT_MAX, yMax = -FLT_MAX;
	float zMin = FLT_MAX, zMax = -FLT_MAX;
	for (unsigned int i = 0; i < vertCount_; ++i)
	{
		if ( vb[i].pos.x < xMin) xMin = vb[i].pos.x;
		if ( vb[i].pos.x > xMax) xMax = vb[i].pos.x;
		if ( vb[i].pos.y < yMin) yMin = vb[i].pos.y;
		if ( vb[i].pos.y > yMax) yMax = vb[i].pos.y;
		if ( vb[i].pos.z < zMin) zMin = vb[i].pos.z;
		if ( vb[i].pos.z > zMax) zMax = vb[i].pos.z;
	}
	m_boundBox.center.x = (xMin + xMax) * 0.5f;
	m_boundBox.center.y = (yMin + yMax) * 0.5f;
	m_boundBox.center.z = (zMin + zMax) * 0.5f;
	m_boundBox.size.x = (xMax - xMin);
	m_boundBox.size.y = (yMax - yMin);
	m_boundBox.size.z = (zMax - zMin);

	if ( Vertex * vbData = (Vertex *)vb_->Lock())
	{
		for (unsigned int i = 0; i < vertCount_; ++i)
		{
			vbData[i].pos = vb[i].pos;
			vbData[i].tu = vb[i].tu;
			vbData[i].tv = vb[i].tv;
			vbData[i].blend = sinf(PI*(vb[i].pos.y - yMin)/(yMax-yMin));
			vbData[i].normal = 0.f;
		}

		for (unsigned int i = 0; i < indCount_/3; ++i)
		{
			const Vector& a = vb[ib[i*3 + 0]].pos;
			const Vector& b = vb[ib[i*3 + 1]].pos;
			const Vector& c = vb[ib[i*3 + 2]].pos;

			Vector normal = (b-a) ^ (c-a);
			normal.Normalize();

			vbData[ib[i*3 + 0]].normal += normal;
			vbData[ib[i*3 + 0]].normal.Normalize();
			vbData[ib[i*3 + 1]].normal += normal;
			vbData[ib[i*3 + 1]].normal.Normalize();
			vbData[ib[i*3 + 2]].normal += normal;
			vbData[ib[i*3 + 2]].normal.Normalize();
		}

		vb_->Unlock();
	}

	if ( unsigned short * ibData = (unsigned short *)ib_->Lock())
	{
		memcpy(ibData, ib, indCount_*sizeof(ib[0]));
		ib_->Unlock();
	}
}

// освободить буферы
void FakeRender::FreeBuffers()
{
	if (vb_)
		vb_->Release(), vb_= NULL;
	if (ib_)
		ib_->Release(), ib_= NULL;
}


// отрисовать
void FakeRender::Render(const Matrix& mtx)
{
	// draw
	if( vb_ && ib_ && vertCount_>0 && indCount_>0 )
	{
		IRender* render = (IRender*)api->GetService("DX9Render");
		Assert(render);

		render->SetStreamSource(0, vb_);
		render->SetIndices(ib_);

		if( diffuseTex_ )
			varDiffuseTex_->SetTexture(diffuseTex_);
		else
			varDiffuseTex_->ResetTexture();
		if( glimpseTex_ )
			varGlimpseTex_->SetTexture(glimpseTex_);
		else
			varGlimpseTex_->ResetTexture();

		//varInstanceInfo_->SetValue(&data_, sizeof(data_));
		varSailFreqTimeAmp_->SetVector4(Vector4(data_.xFreq,data_.yFreq,data_.time,data_.amp));
		varSailColorAlpha_->SetVector4(data_.sailColor);
		varSailGlimpseColorScale_->SetVector4(data_.sailGlimpseColor);

		render->SetWorld(mtx);

/*#if defined _XBOX // FIX - #if supports SM30
			render->DrawIndexedPrimitive(PT_TRIANGLELIST, 0, vertCount_, 0, indCount_/3, "FakeSailSM30");
#else*/
//			varFaceOrder_->SetFloat(1.0f);
			render->DrawIndexedPrimitive(FakeSailCCW_id, PT_TRIANGLELIST, 0, vertCount_, 0, indCount_/3);
//			varFaceOrder_->SetFloat(-1.0f);
//			render->DrawIndexedPrimitive(FakeSailCW_id, PT_TRIANGLELIST, 0, vertCount_, 0, indCount_/3);
//#endif
	}
}

// установить текстуру
void FakeRender::SetTexture(const char* texName)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	if (diffuseTex_)
		diffuseTex_->Release(), diffuseTex_ = NULL;

	diffuseTex_ = render->CreateTexture(__FILE__, __LINE__, texName);
}


// установить текстуру просвечивания
void FakeRender::SetGlimpseTexture(const char* texture)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);
	
	if (glimpseTex_)
		glimpseTex_->Release(), glimpseTex_ = NULL;

	if (strlen(texture))
		glimpseTex_ = render->CreateTexture(__FILE__, __LINE__, texture);
}

void FakeRender::AddAnimTime(float fDeltaTime)
{
	data_.time += fDeltaTime;
	if( data_.time > fTimePeriod ) data_.time -= fTimePeriod;
}

void FakeRender::CalculateTimePeriod()
{
	float fMul = data_.xFreq * data_.yFreq;
	if( fMul > 0.0001f )
		fTimePeriod = 1.f / fMul;
	else
	{
		fMul = data_.xFreq > data_.yFreq ? data_.xFreq : data_.yFreq;
		if( fMul > 0.0001f )
			fTimePeriod = 1.f / fMul;
		else
			fTimePeriod = 100.f;
	}

	if( fTimePeriod < 100.f )
		fTimePeriod *= 100.f;
}

bool FakeRender::FakeRaycast(const Matrix& mtxSailTransform, const Vector& vSrc, const Vector& vDst, unsigned int* pVertexID)
{
	if( m_nHorzQuantity==0 || m_nVertQuantity==0 )
		return false;

	// периведем линию трейса к локальным координатам паруса
	Vector v1 = mtxSailTransform.MulVertexByInverse(vSrc);
	Vector v2 = mtxSailTransform.MulVertexByInverse(vDst);

	if( m_bIsTriangle )
	{
		Vector res;
		if( !m_plnTriangle.Intersection(v1,v2, res) )
			return false;
		float fv = (res - m_vTriangle[0]) | m_vTriangleVDir;
		if( fv < 0.f || fv >= m_fTriangleVDist ) return false;
		float fh = (res - m_vTriangle[1]) | m_vTriangleHDir;
		if( fh < 0.f || fh >= m_fTriangleHDist ) return false;

		if( fh > fv ) return false;

		unsigned int nh = (unsigned int)((m_nHorzQuantity-1) * (fh / m_fTriangleHDist) + 0.5f);
		unsigned int nv = (unsigned int)((m_nVertQuantity-1) * (fv / m_fTriangleVDist) + 0.5f);

		if( nh > nv ) return false;

		*pVertexID = nh + nv*(nv+1) / 2;
		return true;
	}

	// точки трейса ориентированные в пространстве паруса
	Vector p1 = m_sailtrace.mtx.MulVertex(v1);
	Vector p2 = m_sailtrace.mtx.MulVertex(v2);

	// выход точек трейса за пределы паруса
	if( p1.x > m_sailtrace.width && p2.x > m_sailtrace.width )
		return false;
	if( p1.x < -m_sailtrace.width && p2.x < -m_sailtrace.width )
		return false;
	if( p1.y > m_sailtrace.height && p2.y > m_sailtrace.height )
		return false;
	if( p1.y < -m_sailtrace.height && p2.y < -m_sailtrace.height )
		return false;

	// ограничиваем трейс габаритами паруса
	float fcut1 = 0.f;
	float fcut2 = 0.f;
	// правая граница
	if( p1.x > m_sailtrace.width ) {
		float ftmp = (p1.x-m_sailtrace.width) / (p1.x-p2.x);
		if( ftmp > fcut1 ) fcut1 = ftmp;
	} else
	if( p2.x > m_sailtrace.width ) {
		float ftmp = (p2.x-m_sailtrace.width) / (p2.x-p1.x);
		if( ftmp > fcut2 ) fcut2 = ftmp;
	}
	// левая граница
	if( p1.x < -m_sailtrace.width ) {
		float ftmp = (m_sailtrace.width+p1.x) / (p1.x-p2.x);
		if( ftmp > fcut1 ) fcut1 = ftmp;
	} else
	if( p2.x < -m_sailtrace.width ) {
		float ftmp = (m_sailtrace.width+p2.x) / (p2.x-p1.x);
		if( ftmp > fcut2 ) fcut2 = ftmp;
	}
	// верхняя граница
	if( p1.y > m_sailtrace.height ) {
		float ftmp = (p1.y-m_sailtrace.height) / (p1.y-p2.y);
		if( ftmp > fcut1 ) fcut1 = ftmp;
	} else
	if( p2.y > m_sailtrace.height ) {
		float ftmp = (p2.y-m_sailtrace.height) / (p2.y-p1.y);
		if( ftmp > fcut2 ) fcut2 = ftmp;
	}
	// нижняя граница
	if( p1.y < -m_sailtrace.height ) {
		float ftmp = (p1.y+m_sailtrace.height) / (p1.y-p2.y);
		if( ftmp > fcut1 ) fcut1 = ftmp;
	} else
	if( p2.y < -m_sailtrace.height ) {
		float ftmp = (p2.y+m_sailtrace.height) / (p2.y-p1.y);
		if( ftmp > fcut2 ) fcut2 = ftmp;
	}

	// если надо резать все, то трейс не попал в парус
	if( fcut1 + fcut2 >= 1.f )
		return false;

	// собственно резка
	Vector vtmp = p1;
	p1 = p2 + (p1-p2) * (1.f-fcut1);
	p2 = vtmp + (p2-vtmp) * (1.f-fcut2);

	// пересечение с основанием паруса
	Vector pnt1,pnt2;
	dword dwIntersection = 0;
	if( m_sailtrace.plane1.Intersection(p1,p2, pnt1) )
		dwIntersection |= 1;
	if( m_sailtrace.plane2.Intersection(p1,p2, pnt2) )
		dwIntersection |= 2;

	if( dwIntersection==0 )
		return false;

	// точка пересечения
	Vector pnt;
	if( dwIntersection==1 )
		pnt = pnt1;
	else if( dwIntersection==2 )
		pnt = pnt2;
	else
		pnt = (pnt1 + pnt2) * 0.5f;

	// выход за пределы?
	if( pnt.x > m_sailtrace.width || pnt.x < -m_sailtrace.width )
		return false;
	if( pnt.y > m_sailtrace.height || pnt.y < -m_sailtrace.height )
		return false;

	// колонка и строка квадрата куда попал трейс
	unsigned int nh = (unsigned int)((m_nHorzQuantity-1) * (0.5f - 0.5f * pnt.x / m_sailtrace.width));
	unsigned int nv = (unsigned int)((m_nVertQuantity-1) * (0.5f - 0.5f * pnt.y / m_sailtrace.height));

	if( nh > m_nHorzQuantity-1 ) nh = m_nHorzQuantity-1;
	if( nv > m_nVertQuantity-1 ) nv = m_nVertQuantity-1;

	// номер квадрата куда попал трейс
	*pVertexID = nh + nv * m_nHorzQuantity;
	return true;
}

void FakeRender::SetSailParams(long nHeightQ, long nVertQ)
{
	m_nHorzQuantity = nHeightQ;
	m_nVertQuantity = nVertQ;
}

void FakeRender::SetTriangleSailParams(const Vector * v)
{
	m_bIsTriangle = true;
	m_plnTriangle.n = (v[1] - v[0]) ^ (v[2] - v[0]);
	m_plnTriangle.d = m_plnTriangle.n | v[0];

	long up = (v[0].y > v[1].y && v[0].y > v[2].y) ? 0 :
		(v[1].y > v[2].y ? 1 : 2);
	long left = up==0 ? (v[1].z > v[2].z ? 1 : 2) :
		(up==1 ? (v[0].z > v[2].z ? 0 : 2) :
		(v[0].z > v[1].z ? 0 : 1));
	long right = (up!=0 && left!=0) ? 0 :
		((up!=1 && left!=1) ? 1 : 2);

	m_vTriangle[0] = v[up];
	m_vTriangle[1] = v[left];
	m_vTriangle[2] = v[right];

	m_vTriangleHDir = v[right] - v[left];
	m_fTriangleHDist = m_vTriangleHDir.Normalize();

	m_vTriangleVDir = v[right] - v[up];
	m_fTriangleVDist = m_vTriangleVDir.Normalize();
}

void FakeRender::SetSquareSailParams(const Vector * v,float fWindCurvature)
{
	m_bIsTriangle = false;

	Vector normal = !((v[1] - v[0]) ^ (v[2] - v[0]));
	Vector pos = 0.25f * (v[0]+v[1]+v[2]+v[3]);

	m_sailtrace.mtx.BuildOriented(pos, pos + normal, v[0] - v[3]);
	m_sailtrace.mtx.Inverse();

	m_sailtrace.width = 0.5f * sqrtf(0.5f * (~(v[1]-v[0]) + ~(v[2]-v[3])));
	m_sailtrace.height = 0.5f * sqrtf(0.5f * (~(v[0]-v[3]) + ~(v[1]-v[2])));

	m_sailtrace.plane1.n = m_sailtrace.plane2.n = Vector(0.f,0.f,1.f);
	m_sailtrace.plane1.d = 0.f;//m_sailtrace.plane1.n | v[0];
	m_sailtrace.plane2.d = fWindCurvature;//m_sailtrace.plane1.d + 4.f;//data_.amp;
}
