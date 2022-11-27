#include "OcclusionQuery.h"
#include "..\Render.h"


OcclusionQuery::OcclusionQuery()
{
	Create();
}

OcclusionQuery::~OcclusionQuery()
{
	NGRender::pRS->ReleaseQueryNotify(this);

	RELEASE_D3D(query, 0);
}

void OcclusionQuery::Create()
{
	NGRender::pRS->D3D()->CreateQuery( D3DQUERYTYPE_OCCLUSION, &query);
}

void OcclusionQuery::Release()
{
	delete this;
}

void OcclusionQuery::Begin()
{
	HRESULT r = query->Issue( D3DISSUE_BEGIN );
}

void OcclusionQuery::End()
{
	HRESULT r = query->Issue( D3DISSUE_END );
}

DWORD OcclusionQuery::GetResult(bool bWaitResult)
{
	// Loop until the data becomes available
	HRESULT r = D3D_OK;
	DWORD pixelsVisible = 0;
	for (;;)
	{
		r = query->GetData((void *) &pixelsVisible, sizeof(DWORD), D3DGETDATA_FLUSH);
		if (r != S_FALSE)
		{
			break;
		}
		
		if (bWaitResult == false)
		{
			return 0xFFFFFFFF;
		}

		
	}


	if (r != D3D_OK)
	{
		api->Trace("Histogram: Can't get query results!!");
		return 0xFFFFFFFF;
	}

	return pixelsVisible;

}


void OcclusionQuery::OnLostDevice()
{
	RELEASE(query);
}

void OcclusionQuery::OnResetDevice()
{
	Create();
}














