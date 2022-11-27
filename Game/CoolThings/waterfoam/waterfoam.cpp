/* устарело - теперь есть шейдер gmx_foam
#include "waterfoam.h"


WaterFoam::WaterFoam() : foam (_FL_)
{
	SeaTrack_id = NULL;

	bShowHelper = false;

	fRunSpeed = 0.0f;
	fRunPosition = 0.0f;

	fAnimSpeed = 0.0f;
	frame = 0.0f;

	for (dword n = 0; n < 4; n++)
	{
		pFoamTextures[n] = NULL;
	}

	trackTex1 = NULL;
	trackTex2 = NULL;
	trackTexBlend = NULL;

	master.Reset();
}

WaterFoam::~WaterFoam()
{
	for (dword n = 0; n < 4; n++)
	{
		RELEASE(pFoamTextures[n]);
	}


	trackTex1 = NULL;
	trackTex2 = NULL;
	trackTexBlend = NULL;
}


void WaterFoam::PostCreate()
{
	if( masterName[0] && !master.Ptr())
		FindObject(masterName,master);
}


bool WaterFoam::Create(MOPReader & reader)
{
	Render().GetShaderId("SeaTrack", SeaTrack_id);

	trackTex1 = Render().GetTechniqueGlobalVariable("TrackTexture1", _FL_);
	trackTex2 = Render().GetTechniqueGlobalVariable("TrackTexture2", _FL_);
	trackTexBlend = Render().GetTechniqueGlobalVariable("TrackBlend", _FL_);

	EditMode_Update (reader);
	return true;
}


bool WaterFoam::EditMode_Create(MOPReader & reader)
{
	Render().GetShaderId("SeaTrack", SeaTrack_id);

	trackTex1 = Render().GetTechniqueGlobalVariable("TrackTexture1", _FL_);
	trackTex2 = Render().GetTechniqueGlobalVariable("TrackTexture2", _FL_);
	trackTexBlend = Render().GetTechniqueGlobalVariable("TrackBlend", _FL_);

	EditMode_Update (reader);
	return true;
}


bool WaterFoam::EditMode_Update(MOPReader & reader)
{
	IBaseTexture* tex = NULL;

	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	foamMatrix.Build(ang, pos);



	for (dword n = 0; n < 4; n++)
	{
		const char * textureName = reader.String();
		tex = Render().CreateTexture(_FL_, textureName);
		RELEASE(pFoamTextures[n]);
		pFoamTextures[n] = tex;
	}





	fAnimSpeed = reader.Float();
	fRunSpeed = reader.Float();
	fScale = reader.Float();


	long arraySize = reader.Array();

	float maping1 = 0.0f;
	float maping2 = 0.0f;
	Vector v1;
	Vector v2;


	float fInnerDist = 0.0f;
	float fOuterDist = 0.0f;

	foam.DelAll();
	for (long n = 0; n < arraySize; n++)
	{
		v1 = reader.Position();
		v2 = reader.Position();


		if (n > 0)
		{
			vMin.Min(v1);
			vMax.Max(v1);

			vMin.Min(v2);
			vMax.Max(v2);

			dword dwLast = foam.Last();
			Vector & prevV1 = foam[dwLast].v1.pos;
			Vector & prevV2 = foam[dwLast].v2.pos;

			v1 = v1 * foamMatrix;
			v2 = v2 * foamMatrix;

			fInnerDist += (prevV1 - v1).GetLength();
			maping1 = fInnerDist;

			fOuterDist += (prevV2 - v2).GetLength();
			maping2 = fOuterDist;
		} else
		{
			vMin = v1;
			vMax = v1;

			vMin.Min(v2);
			vMax.Max(v2);

			v1 = v1 * foamMatrix;
			v2 = v2 * foamMatrix;
		}

		FoamTrack* track = &foam[foam.Add()];

		track->v1.pos = v1;
		track->v1.color = 0xFFFFFFFF;
		track->v1.u = 0.0f;
		track->v1.v = maping1;


		track->v2.pos = v2;
		track->v2.color = 0xFFFFFFFF;
		track->v2.u = 1.0f;
		track->v2.v = maping2;
	}


	float fInnerInMeter = (1.0f * fScale) / fInnerDist;
	float fOuterInMeter = (1.0f * fScale) / fOuterDist;

	maping1 = 0.0f;
	maping2 = 0.0f;


	for (long i = 1; i < (long)foam.Size(); i++)
	{
		foam[i].v1.v = fInnerInMeter / (1.0f / foam[i].v1.v);
		foam[i].v2.v = fOuterInMeter / (1.0f / foam[i].v2.v);
	}

	alpha = reader.Float() * 0.01f;

	Activate(reader.Bool());

	bShowHelper = reader.Bool();


	noSwing = reader.Bool();

	masterName = reader.String();

	FindObject(masterName,master);

	return true;
}


void _cdecl WaterFoam::Work(float fDeltaTime, long level)
{
	if( EditMode_IsOn() && masterName[0] )
	{
		if( !master.Validate())
			 FindObject(masterName,master);
	}

	Matrix view = Render().GetView();

	if( !noSwing )
	{
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
	}

	Matrix mat;

	if( master.Ptr())
	{
		master.Ptr()->GetMatrix(mat);
	}

	Render().SetWorld(mat);

	if (bShowHelper)
	{
		Render().FlushBufferedLines();
		if (foam.Size() > 0)
		{
			Render().DrawBufferedLine(foam[0].v1.pos, 0xFFFF00FF, foam[0].v2.pos, 0xFFFF00FF, true);
		}
		for (int n = 0; n < (int)foam.Size() - 1; n++)
		{
			Render().DrawBufferedLine(foam[n].v1.pos, 0xFFFFFFFF, foam[n+1].v1.pos, 0xFFFFFFFF, true);
			Render().DrawBufferedLine(foam[n].v2.pos, 0xFFFFFFFF, foam[n+1].v2.pos, 0xFFFFFFFF, true);
			Render().DrawBufferedLine(foam[n+1].v1.pos, 0xFFFF00FF, foam[n+1].v2.pos, 0xFFFF00FF, true);
		}
		Render().FlushBufferedLines();
	}


	long FrameIndexLong = fftol(frame);
	float FrameBlendK = (frame - FrameIndexLong);

	if (trackTex1)
	{
		trackTex1->SetTexture(pFoamTextures[FrameIndexLong & 3]);
	}

	if (trackTex2)
	{
		trackTex2->SetTexture(pFoamTextures[(FrameIndexLong+1) & 3]);
	}
	
	if (trackTexBlend)
	{
		trackTexBlend->SetFloat(0.0f);
	}

	Color c;
	float pulsar = 0.0f;
	float inten = 0.0f;
	for (dword n = 0; n < foam.Size(); n++)
	{
		pulsar = (sin(fRunPosition * 0.01f) + 1.0f) * 0.5f;  // normalized 0..1

		inten = (((1.0f - pulsar) * 0.5f) + 0.5f);

		inten = inten * alpha;

		c.r = inten;
		c.g = inten;
		c.b = inten;
		c.a = inten;
		
		foam[n].v1.color = c.GetDword();
		foam[n].v2.color = c.GetDword();
		foam[n].v2.u = 1.0f + (pulsar * 0.20f);
	}

	Render().DrawPrimitiveUP(SeaTrack_id, PT_TRIANGLESTRIP, (foam.Size()*2)-2, foam.GetBuffer(), sizeof(FoamVertex));

	frame += fAnimSpeed * fDeltaTime * 60.0f;
	fRunPosition += fRunSpeed * fDeltaTime * 60.0f;

	if( !noSwing )
	{
		Render().SetView(view);
	}
}

void WaterFoam::Activate(bool bIsActive)
{
	MissionObject::Activate(bIsActive);

	if (bIsActive)
	{
		SetUpdate((MOF_UPDATE)&WaterFoam::Work, ML_ALPHA4);
	} else
	{
		DelUpdate((MOF_UPDATE)&WaterFoam::Work);
	}

}

MOP_BEGINLIST(WaterFoam, "WaterFoam", '1.00', 999999)

  MOP_POSITION ("Position", Vector(0.0f));
	MOP_ANGLES("Rotation", Vector(0.0f));

	MOP_STRINGC("Foam Texture1", "foam.txx", "Текстура пены");
	MOP_STRINGC("Foam Texture2", "foam.txx", "Текстура пены");
	MOP_STRINGC("Foam Texture3", "foam.txx", "Текстура пены");
	MOP_STRINGC("Foam Texture4", "foam.txx", "Текстура пены");

	MOP_FLOAT("Foam animation speed", 0.1f);
	MOP_FLOAT("Run speed", 0.1f);
	MOP_FLOAT("Foam scale", 1.0f);

	MOP_ARRAYBEG("Foam Track", 2, 512);
		MOP_POSITION ("LandPoint", Vector(0.0f));              
		MOP_POSITION ("SeaPoint", Vector(0.0f));
	MOP_ARRAYEND    


	MOP_FLOATEX("Alpha", 100, 0, 100);
	MOP_BOOLC("Active", true, "Активность пены");
	MOP_BOOLC("Show helper", false, "Показать контуры пены");

	MOP_BOOLC("No swing", true, "No swing model in swing machine")

	MOP_STRINGC("Master object", "", "Get global position from this object")

MOP_ENDLIST(WaterFoam)




*/