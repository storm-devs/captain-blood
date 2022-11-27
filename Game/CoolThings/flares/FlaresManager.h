
#ifndef _FlaresMamager_h_
#define _FlaresMamager_h_

#include "..\..\..\Common_h/Mission.h"

class Flares;

class FlaresManager : public MissionObject
{
	IVariable	  *pVarTex;	
	IVariable	  *pVarMask;	
	IVariable	  *pVarPowSecMask;
	IVariable	  *pFlareColor;
	IVariable	  *pFlareTexSzU;
	IVariable	  *pFlareTexSzV;	

	ShaderId   flareNoZ_id;
	ShaderId   flareMask_id;
	ShaderId   flare9_id;

	ShaderId   flare2D9Mask_id, XBFastFlare2D9Mask_id;	

	float scr_width;
	float scr_height;
	IRenderTarget* pMask;
	IRenderTargetDepth* pZMask;
	
	IRenderTarget* pPowMask;
	IRenderTarget* pPowSecMask;	
	
	IBaseTexture* texture;
	
	array<Flares*> flares;

	struct FLARE_VERTEX
	{
		Vector	vPos;
		dword	dwColor;
		float	tu, tv;
		float	tu2, tv2;
		Vector	vMaskPos;
	};
	
	IVBuffer* pFlareVBuffer;
	IIBuffer* pFlareIBuffer;
	IVBuffer* pFlare2DVBuffer;

	bool first_time;


	Matrix mr;
	Matrix mrNoSwing;

	Matrix mView;
	Matrix mViewNoSwing;

	Matrix mInvView;
	Matrix mInvViewNoSwing;

	const Plane * frustum;
	const Plane * frustumNoSwing;	

public:

	struct FLARE2D_VERTEX
	{
		Vector	vPos;		
		Vector  wpos;
		float	tu, tv, tw;
		float	tu2, tv2;
	};
		
	FlaresManager();
	virtual ~FlaresManager();

	void Activate(Flares* flare,bool activate);

	MO_IS_FUNCTION(FlaresMamager, MissionObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	
	void DrawFlares(FLARE_VERTEX * pFlareVerts, int poly_count, float fDelta);

	void _cdecl Flicker(float dltTime, long level);	
	void _cdecl Draw(float dltTime, long level);


	void ConnectLocator (Flares::Flare* flare);
	

	void CalcFlare(Flares::Flare* flare,float dltTime, Matrix& flare_mat);
	bool InFrustrum(const Plane * frustum,Vector pos,float radius);

	void Release();	
};

#endif

