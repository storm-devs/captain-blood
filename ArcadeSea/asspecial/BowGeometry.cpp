#include "..\pch.h"
#include ".\bowgeometry.h"
#include "../../Common_h/gmx.h"
#include "BowWaveEmitter.h"

const float Viscosity = 0.001f;
const float WaterWidth = 25.0f;

BowGeometry::BowGeometry(BowWaveEmitter& main) :
main_(main),
foamTexture_(NULL),
maskTexture_(NULL),
varFoamTexture_(NULL),
varMaskTexture_(NULL),
varTime_(NULL),
varBlend_(NULL),
varHalfWaterWidth_(NULL),
varWVP_(NULL),
prevField_(&firstField_),
nextField_(&secondField_),
tempField_(&thirdField_),
vb_(NULL),
ib_(NULL)
{
	
}

BowGeometry::~BowGeometry(void)
{
	if (foamTexture_)
		foamTexture_->Release();
	if (maskTexture_)
		maskTexture_->Release();

	varBlend_ = NULL;
	varHalfWaterWidth_ = NULL;
	varTime_ = NULL;
	varFoamTexture_ = NULL;
	varMaskTexture_ = NULL;
	varWVP_ = NULL;

	if (vb_)
		vb_->Release();
	if (ib_)
		ib_->Release();
}

void BowGeometry::Init()
{
	time_ = 0.0f;

	if (foamTexture_)
		foamTexture_->Release();
	if (maskTexture_)
		maskTexture_->Release();

	varBlend_ = main_.Render().GetTechniqueGlobalVariable("blend", __FILE__, __LINE__);
	varHalfWaterWidth_ = main_.Render().GetTechniqueGlobalVariable("halfWaterWidth", __FILE__, __LINE__);
	varTime_ = main_.Render().GetTechniqueGlobalVariable("time", __FILE__, __LINE__);
	varFoamTexture_ = main_.Render().GetTechniqueGlobalVariable("foamTexture", __FILE__, __LINE__);
	varMaskTexture_ = main_.Render().GetTechniqueGlobalVariable("maskTexture", __FILE__, __LINE__);
	varWVP_ = main_.Render().GetTechniqueGlobalVariable("matWorldViewProj", __FILE__, __LINE__);

	foamTexture_ = main_.Render().CreateTexture(__FILE__, __LINE__, "foam.txx");
	maskTexture_ = main_.Render().CreateTextureUseD3DX(__FILE__, __LINE__, "bowmask.tga");

	vb_ = main_.Render().CreateVertexBuffer(WaterDensity*WaterDensity*sizeof(Vertex), sizeof(Vertex),__FILE__, __LINE__);
	ib_ = main_.Render().CreateIndexBuffer((WaterDensity-1)*(WaterDensity-1)*sizeof(short)*6, __FILE__, __LINE__);

	Assert(vb_);
	Assert(ib_);

	unsigned short * ib = NULL;
	if ( ib = (unsigned short *)ib_->Lock(0, 0, LOCK_DISCARD) )
	{
		unsigned int k = 0;
		for (unsigned int i = 0; i < WaterDensity-1; ++i)
			for (unsigned int j = 0; j < WaterDensity-1; ++j)
			{
				ib[k + 0] = WaterDensity*i + j;
				ib[k + 1] = WaterDensity*(i+1) + j;
				ib[k + 2] = WaterDensity*(i+1) + j + 1;

				ib[k + 3] = WaterDensity*i + j;
				ib[k + 4] = WaterDensity*(i+1) + j + 1;
				ib[k + 5] = WaterDensity*i + j + 1;

				k += 6;
			}
			ib_->Unlock();
	}

	for (unsigned int i = 0; i < WaterDensity; ++i)
		for (unsigned int j = 0; j < WaterDensity; ++j)
		{
			vertices_[i][j].coo.x = i*WaterWidth/WaterDensity - WaterWidth/2;
			vertices_[i][j].coo.y = 0;
			vertices_[i][j].coo.z = j*WaterWidth/WaterDensity - WaterWidth/2;
		}

	prevPos_ = main_.GetParams().pos;


	main_.Render().GetShaderId("BowWave", BowWave_id);
}


void BowGeometry::Work(float deltaTime, const Vector& hostVelocity)
{
	Matrix m;
	time_ += deltaTime*0.5f;
	varTime_->SetFloat(time_);
	
	static float speedScale = 0.0f;

	speedScale += (hostVelocity.GetLength() - speedScale)*deltaTime*2.0f;
	float speedScaleMax = 3.0f;
	if (speedScale > speedScaleMax)
		speedScale = speedScaleMax;
	
	//varBlend_->SetFloat(speedScale/speedScaleMax);
	varBlend_->SetFloat(1.0f);
	
	static float time = 0.0f;
	static float ptime = 0.0f;

	time += deltaTime;
	if (time - ptime > 0.15f)
	{
		Interact();
		ptime = time;
	}

	Vector pos = main_.GetHost()->GetMatrix(Matrix()).MulVertex(main_.GetParams().pos);

	if ( Slide(nextField_, tempField_, pos - prevPos_) )
	{
		Slide(prevField_, tempField_, pos - prevPos_);
	}

	Step();
	Draw();

	prevPos_ = pos;
}

void BowGeometry::UpdateBuffer()
{
	void * vb = NULL;
	if ( vb = vb_->Lock(0, 0, LOCK_DISCARD) )
	{
		memcpy(vb, vertices_, sizeof(vertices_));
		vb_->Unlock();
	}
}

void BowGeometry::Draw()
{
	UpdateBuffer();

	Matrix m;
	varFoamTexture_->SetTexture(foamTexture_);
	varMaskTexture_->SetTexture(maskTexture_);
	varHalfWaterWidth_->SetFloat(WaterWidth/2);
	
	varWVP_->SetMatrix(	Matrix().BuildPosition(main_.GetHost()->GetMatrix(Matrix()).MulVertex(main_.GetParams().pos))*
						main_.Render().GetView()*
						main_.Render().GetProjection());
	//varWVP_->SetMatrix(	Matrix().BuildPosition(main_.GetParams().pos)*
	//					main_.GetHost()->GetMatrix(Matrix())*
	//					main_.Render().GetView()*
	//					main_.Render().GetProjection());

	main_.Render().SetStreamSource(0, vb_);
	main_.Render().SetIndices(ib_);
	main_.Render().DrawIndexedPrimitive(BowWave_id, PT_TRIANGLELIST, 0, WaterDensity*WaterDensity, 0, (WaterDensity-1)*(WaterDensity-1)*2);
}

void BowGeometry::Step()
{
	int i,j;
	Field *p = prevField_, *n = nextField_;

	for( i = 1; i < WaterDensity-1; ++i)
	{
		for( j = 1; j < WaterDensity-1; ++j)
		{
			vertices_[i][j].coo.y = n->U[i][j];
			vertices_[i][j].nor.x = n->U[i-1][j]-n->U[i+1][j];
			vertices_[i][j].nor.z = n->U[i][j-1]-n->U[i][j+1];

			float laplas=(	n->U[i-1][j] +
				n->U[i+1][j] +
				n->U[i][j+1] +
				n->U[i][j-1] )*0.25f-n->U[i][j];

			p->U[i][j]=((2.0f-Viscosity)*n->U[i][j]-p->U[i][j]*(1.0f-Viscosity)+laplas);
		}
	}
	Field * sw = prevField_;
	prevField_ = nextField_;
	nextField_ = sw;
}

bool BowGeometry::Slide(Field*& f1, Field*& f2, const Vector& dir)
{
	int dz = Max(Min((int)(dir.x*15.0f), WaterDensity-1), -WaterDensity+1);
	int dx = Max(Min((int)(dir.z*15.0f), WaterDensity-1), -WaterDensity+1);

	if (dx == 0 && dz == 0)
		return false;

	char *n = (char *)f1;
	char *p = (char *)f2;
	memset( p, 0, sizeof(Field));


	unsigned int rowSize = (WaterDensity - abs(dx))*sizeof(float);
	unsigned int rowCount = WaterDensity - abs(dz);
	unsigned int rowStride = WaterDensity*sizeof(float);


	unsigned int startRow;
	unsigned int startCol;

	if (dx <= 0) startCol = 0;
	else startCol = dx;

	if (dz <= 0) startRow = 0;
	else startRow = dz;

	char * srcRowAddress = n + startRow*rowStride + startCol*sizeof(float);

	if (dx <= 0) startCol = -dx;
	else startCol = 0;

	if (dz <= 0) startRow = -dz;
	else startRow = 0;

	char * dstRowAddress = p + startRow*rowStride + startCol*sizeof(float);

	for ( unsigned int i = 0; i < rowCount; ++i )
		memcpy(&dstRowAddress[i*rowStride], &srcRowAddress[i*rowStride], rowSize);

	f1 = f2;
	f2 = (Field*)n;


	return true;
}

void BowGeometry::Interact()
{
	int i1,j1,i,j;
	Field *n = nextField_;

	i1=WaterDensity/2;
	j1=WaterDensity/2;

	for(i=-3; i<=3; ++i)
		for(j=-3; j<=3; ++j)
			if (n->U[i+i1][j+j1] < 1.0f)
				n->U[i+i1][j+j1] += 0.1f;
}