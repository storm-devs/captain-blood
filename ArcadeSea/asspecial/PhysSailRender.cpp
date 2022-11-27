#include "..\pch.h"
#include "PhysSailRender.h"

//////////////////////////////////////////////////////////////////////////
// SailRender
//////////////////////////////////////////////////////////////////////////


SailRender::SailRender() :
vb_(NULL),
ib_(NULL),
varDiffuseTex_(NULL),
varGlimpseTex_(NULL),
//varFaceOrder_(NULL),
//varInstanceInfo_(NULL),
varSailFreqTimeAmp_(NULL),
varSailColorAlpha_(NULL),
varSailGlimpseColorScale_(NULL),
diffuseTex_(NULL),
glimpseTex_(NULL),
m_oldVertexNum(0),
m_oldIndicesNum(0)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	varDiffuseTex_ = render->GetTechniqueGlobalVariable("sailTexture", __FILE__, __LINE__);
	Assert(varDiffuseTex_);
	varGlimpseTex_ = render->GetTechniqueGlobalVariable("glimpseTexture", __FILE__, __LINE__);
	Assert(varGlimpseTex_);
//	varFaceOrder_ = render->GetTechniqueGlobalVariable("faceOrder", __FILE__, __LINE__);
//	Assert(varFaceOrder_);
//	varInstanceInfo_ = render->GetTechniqueGlobalVariable("SailInfo", __FILE__, __LINE__);
//	Assert(varInstanceInfo_);
	varSailFreqTimeAmp_ = render->GetTechniqueGlobalVariable("sailFreqTimeAmp", __FILE__, __LINE__);
	Assert(varSailFreqTimeAmp_);
	varSailColorAlpha_ = render->GetTechniqueGlobalVariable("sailColorAlpha", __FILE__, __LINE__);
	Assert(varSailColorAlpha_);
	varSailGlimpseColorScale_ = render->GetTechniqueGlobalVariable("sailGlimpseColorScale", __FILE__, __LINE__);
	Assert(varSailGlimpseColorScale_);

	m_pSailRenderInfo = NEW SailRenderInfo();
}

SailRender::~SailRender()
{
	if (vb_)
		vb_->Release(), vb_ = NULL;
	if (ib_)
		ib_->Release(), ib_ = NULL;

	varDiffuseTex_ = NULL;

	varGlimpseTex_ = NULL;
	if (diffuseTex_)
		diffuseTex_->Release(), diffuseTex_ = NULL;
	if (glimpseTex_)
		glimpseTex_->Release(), glimpseTex_ = NULL;

//	varFaceOrder_ = NULL;
	//RELEASE (varInstanceInfo_);
	varSailFreqTimeAmp_ = NULL;
	varSailColorAlpha_ = NULL;
	varSailGlimpseColorScale_ = NULL;

	RELEASE (m_pSailRenderInfo);
}

// создать буферы для рендера
void SailRender::CreateBuffers(unsigned int vertCount, unsigned int indCount)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);	

	render->GetShaderId("SailCCW", SailCCW_id);
	//render->GetShaderId("SailCW", SailCW_id);

	

	if (vb_)
		vb_->Release(), vb_ = NULL;
	if (ib_)
		ib_->Release(), ib_ = NULL;

	vb_ = render->CreateVertexBuffer(vertCount*m_pSailRenderInfo->GetSingleVertexSize(), m_pSailRenderInfo->GetSingleVertexSize(), __FILE__, __LINE__, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT );
	ib_ = render->CreateIndexBuffer(indCount*m_pSailRenderInfo->GetSingleIndexSize(), __FILE__, __LINE__, USAGE_WRITEONLY | USAGE_DYNAMIC, FMT_INDEX32, POOL_DEFAULT );

	Assert(vb_);
	Assert(ib_);
}

// закачать данные симуляции из PhysX
void SailRender::UpdateBuffers(const IPhysCloth::SimulationData& data)
{
	Assert(vb_);
	Assert(ib_);

	bool bVB = (data.dirtyFlags & IPhysCloth::SimulationData::VBDirty) != 0;
	bool bPIB = (data.dirtyFlags & IPhysCloth::SimulationData::PIBDirty) != 0;
	bool bIB = (data.dirtyFlags & IPhysCloth::SimulationData::IBDirty) != 0;

	if( m_oldVertexNum != data.vertexCount || m_oldIndicesNum != data.indexCount )
	{
		m_oldVertexNum = data.vertexCount;
		m_oldIndicesNum = data.indexCount;
		bVB = bPIB = bIB = true;
	}

	//if( bVB | bPIB | bIB )
		bVB = bPIB = bIB = true;
	//bVB = true;

	if ( bVB )
		if (SailRenderInfo::Vertex * vbData = (SailRenderInfo::Vertex*)vb_->Lock(0,0,LOCK_DISCARD))
		{
			// пишем текстурные координаты дублированных вершин
			if ( bPIB )
				for ( unsigned int i = 0; i < data.parentIndexCount; ++i )
				{
					((SailRenderInfo::Vertex*)data.posBuffer)[i].tu = ((SailRenderInfo::Vertex*)data.posBuffer)[data.parentIB[i]].tu;
					((SailRenderInfo::Vertex*)data.posBuffer)[i].tv = ((SailRenderInfo::Vertex*)data.posBuffer)[data.parentIB[i]].tv;
				}

#ifndef STOP_DEBUG
				SailRenderInfo::Vertex* pVrtx = (SailRenderInfo::Vertex*)data.posBuffer;
				for( unsigned int idx=0; idx<data.vertexCount; idx++ )
				{
					Assert( !_isnan(pVrtx[idx].pos.x) );
					Assert( !_isnan(pVrtx[idx].pos.y) );
					Assert( !_isnan(pVrtx[idx].pos.z) );
				}
#endif

			memcpy(vbData, data.posBuffer, data.vertexCount*m_pSailRenderInfo->GetSingleVertexSize());
			memset(	((char*)vbData) + data.vertexCount*m_pSailRenderInfo->GetSingleVertexSize(), 0,
				(data.maxVertexCount-data.vertexCount)*m_pSailRenderInfo->GetSingleVertexSize());

			vb_->Unlock();
		}

	if ( bIB )
		if (unsigned short * ibData = (unsigned short*)ib_->Lock())//(0, 0, LOCK_DISCARD))
		{
			memcpy(ibData, data.ib, data.maxIndexCount*m_pSailRenderInfo->GetSingleIndexSize());
			ib_->Unlock();
		}
}

// отрисовать ткань
void SailRender::Render(const IPhysCloth::SimulationData& data)
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

	if (data.vertexCount == 0 || data.indexCount == 0)
		return;

	render->SetWorld(Matrix());

#ifdef _XBOX
	#ifdef _DEBUG
		PIXBeginNamedEvent( 0x0, "SailRender::Render");
	#endif
#endif

/*#if defined _XBOX // FIX - #if supports SM30
	render->DrawIndexedPrimitive(PT_TRIANGLELIST, 0, data.vertexCount, 0, data.indexCount/3, "SailSM30");
#else*/
	//varFaceOrder_->SetFloat(1.0f);
	render->DrawIndexedPrimitive(SailCCW_id, PT_TRIANGLELIST, 0, data.vertexCount, 0, data.indexCount/3);
	//varFaceOrder_->SetFloat(-1.0f);
	//render->DrawIndexedPrimitive(SailCW_id, PT_TRIANGLELIST, 0, data.vertexCount, 0, data.indexCount/3);
//#endif

#ifdef _XBOX
	#ifdef _DEBUG
		PIXEndNamedEvent();
	#endif
#endif

}

// установить текстуру
void SailRender::SetTexture(const char* texName)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	if (diffuseTex_)
		diffuseTex_->Release(), diffuseTex_ = NULL;

	diffuseTex_ = render->CreateTexture(__FILE__, __LINE__, texName);
}

// установить текстуру просвечивания
void SailRender::SetGlimpseTexture(const char* texture)
{
	IRender* render = (IRender*)api->GetService("DX9Render");
	Assert(render);

	if (glimpseTex_)
		glimpseTex_->Release(), glimpseTex_ = NULL;

	if (strlen(texture))
		glimpseTex_ = render->CreateTexture(__FILE__, __LINE__, texture);
}