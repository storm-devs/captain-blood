#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\Render.h"

class PostProcessService;

class Splash : public IRenderFilter
{
	struct Vertex
	{
		Vector p;

		float u1,v1;
		float u2,v2;

		float a;
		float b;

		Color c;
	};

	struct Blood
	{
		Vector p[4];

		float a;
		float time;

		int type; float playTime;

		MissionObject *owner;

		bool busy;

		Blood()
		{
			a	 = 0.0f;
			time = 0.0f;
		}
	};

	struct Pos
	{
		float u,v;
		float w,h;
	};

	struct Seq
	{
		array<Pos> poses;

		Seq() : poses(_FL_)
		{
		}
	};

	//enum DetailLevel {det_off,det_low,det_med,det_hi};

	//long detailLevel;


	ShaderId SimpleQuad_id;
	ShaderId SplashFilter_id;
	

public:

	Splash();
	virtual ~Splash();

	virtual void UserInit();
	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination);

	void Update(float dltTime);
	
	ITexture* GetInputTexture();

	bool turnedOff()
	{
		return false;//detailLevel == det_off;
	}

public:

	void AddBloodPuff(MissionObject *owner = null);
	void Update		 (MissionObject *owner, float dltTime);
	void Release	 (MissionObject *owner);

private:

	void CreateBuffers();

	Blood *GetFreeBlood();

private:

	IVariable *NatMap;

	IVariable *NorMap; IBaseTexture *pNor;
	IVariable *DifMap; IBaseTexture *pDif;

	IVariable *Screen;

	array<Blood> bloods;

	IVBuffer *pVBuffer;
	IIBuffer *pIBuffer;

	float bloodResp;

//	IRenderTarget *diffMap;
//	IRenderTarget *normMap;

	array<Seq> seqs;

	int count; // количество активных пятен на экране

	float m_aspect;

	PostProcessService* pServ;

};
