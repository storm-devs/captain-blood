#pragma once
#include "../../Common_h/Render.h"

class BowWaveEmitter;
class BowGeometry
{
	enum {WaterDensity=150};

	struct Vertex
	{
		Vector coo;
		Vector nor;
	};

	struct Field
	{
		float U[WaterDensity][WaterDensity];
	} firstField_, secondField_, thirdField_;

	Vertex vertices_[WaterDensity][WaterDensity];
	Field *prevField_, *nextField_, *tempField_;

	IBaseTexture		*foamTexture_;				//	текстура пены
	IBaseTexture		*maskTexture_;				//	текстура пены
	IVariable			*varFoamTexture_;			//	--//--
	IVariable			*varMaskTexture_;			//	--//--
	IVariable			*varTime_;					//	время
	IVariable			*varBlend_;					//	бленд
	IVariable			*varHalfWaterWidth_;
	IVariable			*varWVP_;					//	матрица 

	IVBuffer			*vb_;
	IIBuffer			*ib_;

	float				time_;

	BowWaveEmitter&		main_;

	Vector				prevPos_;


	ShaderId BowWave_id;

	bool Slide(Field*& f1, Field*& f2, const Vector&);
	void Interact();
	void UpdateBuffer();
	void Draw();
	void Step();

public:

	void Init();
	void Work(float deltaTime, const Vector& hostVelocity);

	BowGeometry(BowWaveEmitter& main);
	~BowGeometry(void);
};
