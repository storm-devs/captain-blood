#include "Grass.h"
#include "..\..\..\Common_h\IShadows.h"

GrassHolder	Grass::GH;
int Grass::m_grassDebugLine = 0;

int Grass::m_totalBlocks = 0;
int Grass::m_totalGrasses = 0;
int Grass::m_totalDraws = 0;

Vector	Grass::m_camPos;
dword	Grass::m_numB44 = 0;
const dword * Grass::m_b44Offset = null;

GrassHolder::GrassHolder()
{
	m_ref = 0;
}

GrassHolder::~GrassHolder()
{
}

void GrassHolder::Init()
{
	IRender * rs = (IRender *)api->GetService("DX9Render");

	rs->GetShaderLightingId("Grass2", shaderGrassLighted);
	rs->GetShaderId("Grass2", shaderGrass);

	rs->GetShaderLightingId("Grass2NoShadow", shaderGrassLightedNS);
	rs->GetShaderId("Grass2NoShadow", shaderGrassNS);
	rs->GetShaderId("Grass2ShadowRecv", shaderShadowReceive);
	
	varGrassTemp = rs->GetTechniqueGlobalVariable("grass2Temp", _FL_);
	
	varShadowMap = rs->GetTechniqueGlobalVariable("grass2ShadowTexture", _FL_);
	varGrassPlayer = rs->GetTechniqueGlobalVariable("grass2Player", _FL_);

	varGrassAngles = rs->GetTechniqueGlobalVariable("grass2Angles", _FL_);

	varWindParams1 = rs->GetTechniqueGlobalVariable("grass2WindParams", _FL_);
	varWindParams2 = rs->GetTechniqueGlobalVariable("grass2WindParams2", _FL_);
	varGrassColor = rs->GetTechniqueGlobalVariable("grass2Color", _FL_);
	
	//varShadowReceive = rs->GetTechniqueGlobalVariable("grass2ShadowReceive", _FL_);

	varGrassDistances = rs->GetTechniqueGlobalVariable("grass2Distances", _FL_);
		
	varGrassTexture = rs->GetTechniqueGlobalVariable("grass2Texture", _FL_);

	varGrassTexPatches = rs->GetTechniqueGlobalVariable("grass2TexPatches", _FL_);
	varGrassPatchesParams = rs->GetTechniqueGlobalVariable("grass2PatchesParams", _FL_);
	varPosVectors = rs->GetTechniqueGlobalVariable("grass2PosVectors", _FL_);

	mtxNormals[0].SetIdentity();
#if !defined(NO_TOOLS) && !defined(_XBOX)
	vecNormals[0] = Vector(0.0f, 1.0f, 0.0f);
#endif

	/*int curMtx = 1;
	for (int x=0; x<4; x++)
	{
		float ax = float(x + 1) * (1.0f / 4.0f);
		for (int y=0; y<5; y++)
		{
			float ay = float(y) * (1.0f / 5.0f) * PIm2;

			mtxNormals[curMtx].Build(ax, ay, 0.0f, 0.0f, 0.0f, 0.0f);

			Vector vy = mtxNormals[curMtx].MulVertex(Vector(0.0f, 1.0f, 0.0f));
			Vector vx = Vector(0.0f, 0.0f, 1.0f) ^ vy;
			Vector vz = vx ^ vy;
			vx = vy ^ vz;

#if !defined(NO_TOOLS) && !defined(_XBOX)
			vecNormals[curMtx] = vy;
#endif
			mtxNormals[curMtx].m[0][0] = vx.x;
			mtxNormals[curMtx].m[1][0] = vx.y;
			mtxNormals[curMtx].m[2][0] = vx.z;

			mtxNormals[curMtx].m[0][1] = vy.x;
			mtxNormals[curMtx].m[1][1] = vy.y;
			mtxNormals[curMtx].m[2][1] = vy.z;

			mtxNormals[curMtx].m[0][2] = vz.x;
			mtxNormals[curMtx].m[1][2] = vz.y;
			mtxNormals[curMtx].m[2][2] = vz.z;

			curMtx++;
		}
	}*/
}

void GrassHolder::AddRef()
{
	if (m_ref == 0)
		Init();

	m_ref++;
}

void GrassHolder::Release()
{
	Assert(m_ref);

	m_ref--;
	if (m_ref == 0)
	{
	}
}

__forceinline void VectorMin(Vector & res, const Vector & compare)
{
	res.x = Min(res.x, compare.x);
	res.y = Min(res.y, compare.y);
	res.z = Min(res.z, compare.z);
}

__forceinline void VectorMax(Vector & res, const Vector & compare)
{
	res.x = Max(res.x, compare.x);
	res.y = Max(res.y, compare.y);
	res.z = Max(res.z, compare.z);
}

Grass::Grass() 
#if !defined(NO_TOOLS) && !defined(_XBOX)
	: m_blocks44(_FL_, 1024),
	m_mainData(_FL_, 8192)
#endif
{
	m_loadDataBuffer = null;
	m_grassTexture = null;

	m_vBParameters = null;

	m_time = 0.0f;

	m_executor.Reset();

	m_regenerateGrass = false;
	m_startedGeneration = false;

	m_shadowRecieveMode = false;

	#if !defined(NO_TOOLS) && !defined(_XBOX)
		m_rigidBody = null;
		m_geo = null;

	#endif
}

Grass::~Grass()
{
	GH.Release();

	RELEASE(m_vBParameters);
	RELEASE(m_loadDataBuffer);
	
	#if !defined(NO_TOOLS) && !defined(_XBOX)
		RELEASE(m_rigidBody);
		RELEASE(m_geo);
	#endif

	RELEASE(m_grassTexture);
}

bool Grass::Create(MOPReader & reader)
{
	GH.AddRef();

	static const ConstString objectName("GrassExecutor");
	Mission().CreateObject(m_executor.GetSPObject(), "GrassExecutor", objectName, true);

	EditMode_Update(reader);

	sprintf_s(m_grassPath, sizeof(m_grassPath), "resource\\missions\\%s\\data\\%s_%s.grs", Mission().GetMissionName(), Mission().GetMissionName(), GetObjectID().c_str());

	LoadDataFile();

	#if !defined(NO_TOOLS) && !defined(_XBOX)
		if( EditMode_IsOn() )
			Registry(MG_EXPORT);
	#endif

	return true;
}

bool Grass::EditMode_Update(MOPReader & reader)
{
	const char * patchName = reader.String().c_str();
	float density = reader.Float();
	m_grassSize = reader.Float();
	m_grassRandSize = reader.Float();

	bool randomPosition = reader.Bool();
	m_useNormals = (reader.Bool()) ? 1.0f : 0.0f;

	m_grassMaxDistance = reader.Float();
	m_grassTransparencyDistance = reader.Float();

	m_grassAngles.x = reader.Float() * PIm2 / 360.0f;
	m_grassAngles.y = reader.Float();
	m_grassAngles.y = 1.0f;

	int seed = reader.Long();

	m_grassColor = reader.Colors();

	bool bVisible = reader.Bool();

	m_windAngles = reader.Angles();
	m_windPower = reader.Float();
	m_windColorAmbient = reader.Float();
	m_windColorPower = reader.Float();

	m_level = reader.Long();
	m_dynamicLighting = reader.Bool();
	m_shadowCast = reader.Bool();
	m_shadowReceive = reader.Bool();
	m_seaReflection = reader.Bool();
	m_seaRefraction = reader.Bool();
	m_drawPatch = reader.Bool();
	m_noSwing = reader.Bool();

	const char * texName = reader.String().c_str();

	Unregistry(MG_SHADOWRECEIVE);
	if (m_shadowReceive)
		Registry(MG_SHADOWRECEIVE, &Grass::ShadowReceive, 10000000);

	Unregistry(MG_SEAREFLECTION);
	if (m_seaRefraction)
		Registry(MG_SEAREFLECTION, &Grass::SeaRefraction, 10000000);

	Unregistry(MG_SEAREFRACTION);
	if (m_seaRefraction)
		Registry(MG_SEAREFRACTION, &Grass::SeaRefraction, 10000000);

	IBaseTexture * texture = Render().CreateTexture(_FL_, texName);
	RELEASE(m_grassTexture);
	m_grassTexture = texture;

	double probabilities[4];

	for (int i=0; i<4; i++)
	{
		float x = reader.Float();
		float y = reader.Float();
		float width = reader.Float();
		float height = reader.Float();
		m_texPatchesParams[i].x = reader.Float();
		m_texPatchesParams[i].y = -reader.Float();
		probabilities[i] = reader.Float();
		
		// wind power multiplicator
		m_windPowerMultiplicator[i] = reader.Float() * 0.1f;
		
		m_texPatches[i] = Vector4(x, y + height, width, -height);
	}

	#if !defined(NO_TOOLS) && !defined(_XBOX)
		if (!string::IsEqual(m_patchName.c_str(), patchName) || fabsf(m_density - density) > 1e-6f)
			m_regenerateGrass = true;

		if (seed != m_randomGenerator)
			m_regenerateGrass = true;

		if (m_randomPosition != randomPosition)
			m_regenerateGrass = true;

		m_patchName = patchName;

		// считаем вероятность появления травинок
		double fullProbability = 0.0f;
		for (int i=0; i<4; i++)
			fullProbability += probabilities[i];

		for (int i=0; i<4; i++)
		{
			probabilities[i] /= fullProbability;
			
			if (i > 0)
				probabilities[i] += probabilities[i - 1];
		}

		// сохраняем вероятность и смотрим, может надо перегенерить траву
		for (int i=0; i<4; i++)
		{
			if (fabs(m_probabilities[i] - probabilities[i]) > 1e-6)
				m_regenerateGrass = true;

			m_probabilities[i] = float(probabilities[i]);
		}
	#endif

	m_randomPosition = randomPosition;
	m_randomGenerator = seed;
	m_density = density;

	Show(bVisible);

	return true;
}

void Grass::Show(bool isShow)
{
	MissionObject::Show(isShow);

	DelUpdate();
	if (IsShow())
	{
		SetUpdate(&Grass::Execute, ML_EXECUTE1 - 2);
		SetUpdate(&Grass::Realize, ML_ALPHA1 + (ML_ALPHA3 - ML_ALPHA1) * m_level / 100);
	}
	else
	{
		DelUpdate(&Grass::Execute);
		DelUpdate(&Grass::Realize);
	}

	if (!EditMode_IsOn())
	{
		Console().Trace(COL_ALL, "Grass '%s' %s", GetObjectID().c_str(), (IsShow()) ? "enabled" : "disabled");
		LogicDebug("Grass '%s' %s", GetObjectID().c_str(), (IsShow()) ? "enabled" : "disabled");
	}
}

void Grass::LoadDataFile()
{
	RELEASE(m_loadDataBuffer);

	m_loadDataBuffer = Files().LoadData(m_grassPath, _FL_);

#ifndef _XBOX
	if (m_loadDataBuffer)
	{
		// свизлим файл если запускаем на PC
		dword size = m_loadDataBuffer->Size();
		for (dword i=0; i<size / sizeof(dword); i++) 
			__RefDataSwizzler( ((dword*)m_loadDataBuffer->Buffer())[i] );
	}
#endif

	// проверяем версию файла
	if (m_loadDataBuffer)
	{
		dword * data = (dword*)m_loadDataBuffer->Buffer();
		if (data[0] != GRASS_VERSION)
		{
			api->Trace("Grass error: grass data(object \"%s\") version wrong.", GetObjectID().c_str());
			RELEASE(m_loadDataBuffer);
		}
	}
}

//! Проверка видимости сферы в frustum'e камеры
__forceinline bool Grass::SphereIsVisibleSquare(const Plane * frustum, const Vector4 & SphereCenter, float SphereRadius)
{
#ifndef _XBOX
	for (int p = 1; p < 5; p++)
	{
		float dist = frustum[p].Dist(SphereCenter.v);
		if (dist < -SphereRadius) 
			return false;
	}
#else
	__vector4 vradius = XMVectorReplicate(-SphereRadius);
	__vector4 vcenter = __lvlx(&SphereCenter, 0);

	__vector4 v1 = __vmsum4fp(__VMXGetReg(vfrustum1), vcenter);
	__vector4 v2 = __vmsum4fp(__VMXGetReg(vfrustum2), vcenter);
	__vector4 v3 = __vmsum4fp(__VMXGetReg(vfrustum3), vcenter);
	__vector4 v4 = __vmsum4fp(__VMXGetReg(vfrustum4), vcenter);
	
	__vector4 v12 = __vmrglw(v1, v2);
	__vector4 v34 = __vmrghw(v3, v4);

	__vector4 v1234 = __vperm(v12, v34, __VMXGetReg(vperm0167));

	unsigned int compareResult = 0;
	__vcmpgtfpR(vradius, v1234, &compareResult);

	 if (!(compareResult & (1 << 5)))
		return false;
#endif

	return true;
}

dword __forceinline Grass::FastRand(dword & seed) 
{ 
	//seed = (214013 * seed + 2531011); 
	seed += 2531011; 
	//return (seed << 8L) & 0xFF000000; 
	return (seed & 0xFFFF); 
	//return (seed >> 16L); 
} 

void _cdecl Grass::Execute(float fDeltaTime, long level)
{
	if (EditMode_IsOn() && !EditMode_IsVisible()) return;

	/*if (api->DebugKeyState(ICore::xb_y))
		return;*/

	m_prevTotalGrasses = m_totalGrasses;

	m_totalObbMin = 0.0f;
	m_totalObbMax = 0.0f;

	i_last = 0;
	j_last = 0;
	e_last = 0;

	m_time += fDeltaTime;

#if !defined(NO_TOOLS) && !defined(_XBOX)
	// в режиме редактирования генерим травку постепенно
	m_grassDebugLine = 0;

	if (EditMode_IsOn())
	{
		for (int i=0; i<4; i++)
			if (m_startedGeneration) ProcessBlock4x4();

		PrepareGrass();
	}
#endif

	if (!m_startedGeneration) 
		m_executor.Ptr()->Add2Execute(this);
}

void Grass::GetBox(Vector & min, Vector & max)
{
	min = m_totalObbMin;
	max = m_totalObbMax;
}

void Grass::AddDrawPacket(dword i, dword j, dword e, GrassExecutor::DrawPacket * packet)
{
	i_last = i;
	j_last = j;
	e_last = e;

	VectorMin(m_totalObbMin, packet->obbMin);
	VectorMax(m_totalObbMax, packet->obbMax);
	
	if (packet && packet->numRects)
		packet->used = true;
}

GrassExecutor::DrawPacket * Grass::GetDrawPacket()
{
	GrassExecutor::DrawPacket * packet = null;
	GrassExecutor::DrawPacket * last = null;

	// FIX-ME переделать на queue или стэк, или еще чего
	for (int i=0; i<m_executor.Ptr()->drawPackets.Len(); i++)
	{
		packet = &m_executor.Ptr()->drawPackets[i];

		if (!packet->used)
		{
			if (i !=0 )
				last = &m_executor.Ptr()->drawPackets[i - 1];

			break;
		}
	}

	if (!packet)
		return null;

	packet->grass = this;
	packet->curBlock = 0;
	packet->startVertex = 0;
	packet->numRects = 0;
	packet->obbMin = 1e+6f;
	packet->obbMax = -1e+6f;

	if (last)
	{
		packet->startVertex = last->startVertex + last->numRects * 4; 
	}

	return packet;
}

bool Grass::QSortBlocks44(const dword & v1, const dword & v2)
{
	const tb44 * b1 = (const tb44 *)(m_b44Offset + m_numB44 + v1);
	const tb44 * b2 = (const tb44 *)(m_b44Offset + m_numB44 + v2);

	float r1 = (b1->center - m_camPos).GetLength2();
	float r2 = (b2->center - m_camPos).GetLength2();

	return (r1 < r2);
}

GrassExecutor::WorkResult _cdecl Grass::Work(GrassHolder::Vertex * __restrict vertices, const Vector & camPos, const Plane * frustum)
{
	GrassExecutor::DrawPacket * packet;

	m_camPos = camPos;

	packet = GetDrawPacket();
	if (!packet)
		return GrassExecutor::GRASS_OVERFLOW;

	int numRects = packet->numRects;

	GrassHolder::Vertex * __restrict verts = vertices + packet->startVertex / IS_XBOX(4, 1);

	__declspec(align(16)) Vector4 b44center, b22center;
	b44center.w = -1.0f;
	b22center.w = -1.0f;

#ifdef _XBOX
	__declspec(align(16)) byte byteControl0167[16] = {0,1,2,3, 4,5,6,7, 24,25,26,27,  28,29,30,31};
	__VMXSetReg(vfrustum1, __lvlx(frustum, 16));
	__VMXSetReg(vfrustum2, __lvlx(frustum, 32));
	__VMXSetReg(vfrustum3, __lvlx(frustum, 48));
	__VMXSetReg(vfrustum4, __lvlx(frustum, 64));
	__VMXSetReg(vperm0167, __lvlx(&byteControl0167, 0));
	__VMXSetReg(vabs, XMVectorReplicateInt(0x7FFFFFFF));
#endif

	if (m_loadDataBuffer && m_loadDataBuffer->Size())
	{
		const dword * m_loadedData = (const dword *)m_loadDataBuffer->Buffer();

		m_numB44 = m_loadedData[1];
		m_b44Offset = &m_loadedData[2];

		// Сортируем смещения от удаленности до камеры
		if (i_last == 0 && m_prevTotalGrasses > 8 * (MAX_GRASSES / 10))
		{
			//ProfileTimer tt;
			array<dword>::QSort(QSortBlocks44, (dword*)m_b44Offset, long(m_numB44));
			//tt.Stop();
			//api->Trace("tt = %d", tt.GetTime32());
		}

		// FIX-ME: сначала рисовать блоки которые рядом с камерой
		for (dword i=i_last; i<m_numB44; i++)
		{
			const dword * addr = m_b44Offset + m_numB44 + m_b44Offset[i];
			
			const tb44 * b44 = (const tb44 *)addr;

			float dist = (b44->center - camPos).GetLength();
			if (dist - b44->radius >= m_grassMaxDistance)
				continue;

			b44center.v = b44->center;
			if (!SphereIsVisibleSquare(frustum, b44center, b44->radius))
				continue;

			for (dword j=j_last; j<b44->numBlocks; j++)
			{
				if (packet->curBlock >= MAX_BLOCKS || numRects >= MAX_INDICES - 256)
				{
					packet->numRects = numRects;
					AddDrawPacket(i, j, 0, packet);

					packet = GetDrawPacket();
					if (!packet)
						return GrassExecutor::GRASS_OVERFLOW;
					numRects = 0;
				}

				// если кол-во GH.posVectors
				if ((packet->startVertex / 4 + numRects) >= MAX_GRASSES - 512)
				{
					packet->numRects = numRects;
					AddDrawPacket(i, j, 0, packet);
					return GrassExecutor::GRASS_OVERFLOW;
				}

				const dword * dwb22 = (addr + sizeof(tb44) / sizeof(dword) - (16 - b44->numBlocks) + b44->offsets[j]);
				const tb22 * b22 = (const tb22 *)dwb22;

				Vector b22min, b22max;

				b22min.x = float(b22->dw_minmax[0] & 0xFFFF) - 16000.0f;
				b22min.y = float(b22->dw_minmax[0] >> 16L) - 16000.0f;
				b22min.z = float(b22->dw_minmax[1] & 0xFFFF) - 16000.0f;
				b22max.x = float(b22->dw_minmax[1] >> 16L) - 16000.0f;
				b22max.y = float(b22->dw_minmax[2] & 0xFFFF) - 16000.0f;
				b22max.z = float(b22->dw_minmax[2] >> 16L) - 16000.0f;

				VectorMin(packet->obbMin, b22min);
				VectorMax(packet->obbMax, b22max);

				Vector b22halfsize = (b22max - b22min) * 0.5f;

				packet->posVectors[packet->curBlock * 2 + 0].v = b22min;
				packet->posVectors[packet->curBlock * 2 + 1].v = b22max - b22min;

				b22center.v = (b22max + b22min) * 0.5f;
				float b22radius = (b22max - b22min).GetLength();

				if (!SphereIsVisibleSquare(frustum, b22center, b22radius))
					continue;

				float dist = (b22center.v - camPos).GetLength();
				if (dist - b22radius >= m_grassMaxDistance)
					continue;

				dwb22 += sizeof(tb22) / sizeof(dword) - (4 - b22->numBlocks);

				for (dword e=e_last; e<b22->numBlocks; e++)
				{
					const tb * block = (const tb *)(dwb22 + b22->offsets[e]);

#ifdef _XBOX
					__dcbt(0, &block->grasses[0]);
					__dcbt(128, &block->grasses[0]);
#endif

					Vector bbmin, bbmax;

					float x_offset = float(block->dw_shifts & 0xFFFF);
					float z_offset = float(block->dw_shifts >> 16L);

					__declspec(align(16)) Vector4 center;

					center.x = b22min.x + (0.5f + x_offset) * b22halfsize.x;
					center.y = (b22min.y + b22max.y) * 0.5f;
					center.z = b22min.z + (0.5f + z_offset) * b22halfsize.z;
					center.w = -1.0f;

					float radius = b22radius * 0.5f;
					
					if (!SphereIsVisibleSquare(frustum, center, radius))
						continue;

					dword seed = (*dwb22) * 1257383 + e * 145631;

					dword cur_block = (packet->curBlock << 8L);

					float dist = (center.v - camPos).GetLength();
					int maxlod = 0;
					if (dist - radius <= 35.0f)
						maxlod = 2;
					else 
						if (dist - radius <= 50.0f)
							maxlod = 1;

					const dword * __restrict grassPtr = &block->grasses[0];
					int LodsGrasses = block->numGrasses;
					//int curGrass = 0;
					for (int lod=0; lod < maxlod + 1; lod++)
					{
						int numGrasses = LodsGrasses & 0xFF;//(block->numGrasses >> (8L * lod)) & 0xFF;
						LodsGrasses >>= 8L;
						for (int k=0; k<numGrasses; k++)
						{
							dword random = FastRand(seed);
							const dword & grass = *grassPtr++; //block->grasses[curGrass];

							dword rotate = (random << 16L) & 0xFF000000;
							dword texture_and_slope = (grass & 0xFF000000) >> 8L;
							dword result = rotate | texture_and_slope | cur_block;
							result |= (dword(lod) << 2L);
						
							dword pos = grass | (random << 24L);

#ifdef _XBOX
							verts->pos = pos;		verts->color = result;
							verts += 1;
#else
							verts[0].pos = pos;		verts[0].color = result | 0;
							verts[1].pos = pos;		verts[1].color = result | 1;
							verts[2].pos = pos;		verts[2].color = result | 2;
							verts[3].pos = pos;		verts[3].color = result | 3;
							verts += 4;
#endif
							numRects++;
							//curGrass++;
						}
					}
				}

				packet->curBlock++;
				e_last = 0;
			}

			j_last = 0;
		}
		i_last = 0;
	}
	
	packet->numRects = numRects;

	if (packet->numRects)
		AddDrawPacket(0, 0, 0, packet);

	return GrassExecutor::GRASS_OK;
}

void Grass::PrepareForRender(const Matrix & view)
{
	if (!m_noSwing)
		Render().SetView((Mission().GetInverseSwingMatrix() * Matrix(view).Inverse()).Inverse());
	else
		Render().SetView(view);

	Vector4 temp[4];
	temp[0] = Vector4(-0.5f, 0.0f, 0.0f, m_time);
	temp[1] = Vector4(-0.5f, 1.0f, 0.0f, m_grassSize);
	temp[2] = Vector4( 0.5f, 1.0f, 0.0f, m_grassRandSize);
	temp[3] = Vector4( 0.5f, 0.0f, 0.0f, m_useNormals);

	m_texPatchesParams[0].z = 1e+7f;	m_texPatchesParams[0].w = 1.0f;
	m_texPatchesParams[1].z = 50.0f;	m_texPatchesParams[1].w = 1.0f / 5.0f;
	m_texPatchesParams[2].z = 35.0f;	m_texPatchesParams[2].w = 1.0f / 5.0f;

	GH.varGrassTemp->SetVector4Array(temp, 4);

#ifndef _XBOX
	GH.varGrassTexPatches->SetVector4Array(m_texPatches, 4);
	GH.varGrassPatchesParams->SetVector4Array(m_texPatchesParams, 4);
#endif

	GH.varWindParams1->SetVector4(Vector4(sinf(m_windAngles.y), cosf(m_windAngles.y), m_windPower, 0.0f));
	GH.varWindParams2->SetVector4(Vector4(m_windColorAmbient, m_windColorPower, 0.0f, 0.0f));
	GH.varGrassTexture->SetTexture(m_grassTexture);
	GH.varGrassColor->SetVector4(m_grassColor.v4);
	GH.varGrassAngles->SetVector4(m_grassAngles);

#ifdef _XBOX
	if (!m_vBParameters)
	{
		struct Stream2
		{
			Vector4 texPatch;
			Vector	temp;
			float	patchParamsX, patchParamsY;
			float	lodParamsX, lodParamsY;
		};

		Stream2 s2[4];

		for (int i=0; i<4; i++)
		{
			s2[i].texPatch = m_texPatches[i];
			s2[i].temp = Vector(m_texPatchesParams[i].x, m_texPatchesParams[i].y, m_windPowerMultiplicator[i]);
			s2[i].patchParamsX = temp[i].x;
			s2[i].patchParamsY = temp[i].y;
			s2[i].lodParamsX = m_texPatchesParams[i].z;
			s2[i].lodParamsY = m_texPatchesParams[i].w;
		}

		m_vBParameters = Render().CreateVertexBuffer(sizeof(Stream2) * 4, sizeof(Stream2), _FL_);
		m_vBParameters->Copy(s2, sizeof(s2));
	}

	Render().SetStreamSource(1, m_vBParameters);
#endif

	//GH.varShadowReceive->SetBool(m_shadowReceive);

	float startDist = m_grassMaxDistance * m_grassTransparencyDistance;
	float transDist = 1.0f / Sqr(m_grassMaxDistance - startDist);
	GH.varGrassDistances->SetVector4(Vector4(Sqr(startDist), transDist, 0.0f, 0.0f));
}

void _cdecl Grass::Realize(float fDeltaTime, long level)
{
#if !defined(NO_TOOLS) && !defined(_XBOX)
	if (m_startedGeneration)
	{
		Render().Print(10.0f, 40.0f + m_grassDebugLine * 20, 0xFF00FF00, "Generating grass(\"%s\"), blocks = %d / %d", GetObjectID().c_str(), m_current44, m_b44x * m_b44z);
		m_grassDebugLine++;
	}

	if (m_drawPatch && m_geo)
	{
		Render().SetWorld(Matrix());
		m_geo->Draw();
	}
#endif

	m_executor.Ptr()->RealizeGrass(this);
}

void _cdecl Grass::SeaRefraction(const char * group, MissionObject * sender)
{
	m_executor.Ptr()->RealizeGrass(this);
}

void _cdecl Grass::ShadowReceive(const char * group, MissionObject * sender)
{
	IGMXService::RenderModeShaderId * oldRMS = Geometry().GetRenderMode();
	IGMXService::RenderModeShaderId * oldRMSAlpha = Geometry().GetRenderModeNoAlpha();

	Geometry().SetRenderMode(null);
	m_shadowRecieveMode = true;
	m_executor.Ptr()->RealizeGrass(this);
	m_shadowRecieveMode = false;
	Geometry().SetRenderMode(oldRMS, oldRMSAlpha);
}

void Grass::DrawPacket(GrassExecutor::DrawPacket & packet)
{
	m_totalBlocks += packet.curBlock;
	m_totalGrasses += packet.numRects;
	m_totalDraws++;

	for (int i=0; i<4; i++)
	{
		packet.posVectors[i].w = m_windPowerMultiplicator[i];
	}

	GH.varPosVectors->SetVector4Array(&packet.posVectors[0], MAX_BLOCKS * 2);

	int startIndex = (packet.startVertex / 4) * 2 * 3;

	if (m_dynamicLighting && !m_shadowRecieveMode)
	{
		GMXOBB obb;
		obb.SetFromMinMax(packet.obbMin, packet.obbMax);

		ShaderLightingId & shader = (m_shadowReceive) ? GH.shaderGrassLighted : GH.shaderGrassLightedNS;

#ifdef _XBOX
		//dword dwId0 = Render().pixBeginEvent(_FL_, "grs: start: %d, stride: %d", packet.startVertex, sizeof(GrassHolder::Vertex));
		
		Render().DrawPrimitiveLighted(shader, PT_QUADLIST, packet.startVertex, packet.numRects, obb);
		
		//Render().pixEndEvent(_FL_, dwId0);
#else
		Render().SetIndices(m_executor.Ptr()->m_iBuffer, packet.startVertex);
		Render().DrawIndexedPrimitiveLighted(shader, PT_TRIANGLELIST, 0, packet.numRects * 4, 0, packet.numRects * 2, obb);
#endif
	}
	else
	{
		ShaderId * shader = (m_shadowReceive) ? &GH.shaderGrass : &GH.shaderGrassNS;
		if (m_shadowRecieveMode)
			shader = &GH.shaderShadowReceive;

#ifdef _XBOX
		Render().DrawPrimitive(*shader, PT_QUADLIST, packet.startVertex, packet.numRects);
#else
		Render().SetIndices(m_executor.Ptr()->m_iBuffer, packet.startVertex);
		Render().DrawIndexedPrimitive(*shader, PT_TRIANGLELIST, 0, packet.numRects * 4, 0, packet.numRects * 2);
#endif
	}
}

MOP_BEGINLIST(Grass, "Grass", '1.00', 900)
	MOP_STRINGC("Patch geometry", "", "Геометрия в которой есть патчи с травой")
	MOP_FLOATEXC("Density", 5.0f, 0.01f, 20.0f, "Плотность травы на метр^2")
	MOP_FLOATEXC("Grass size", 1.0f, 0.01f, 1000.0f, "Размер травы в метрах")
	MOP_FLOATEXC("Rand size", 0.0f, 0.0f, 1000.0f, "Рандомный размер травы относительно Grass size, +-")
	MOP_BOOLC("Random position", false, "true - трава будет размещаться слегка рандомнее, false - поравномернее")
	MOP_BOOLC("Use normals", false, "Трава ориентируется по нормалям патча, false - растет вверх, true - растет по нормали")
	MOP_FLOATEXC("Hide distance", 100.0f, 0.0f, 10000.0f, "Расстояние на котором трава пропадает")
	MOP_FLOATEXC("Transparency distance", 0.5f, 0.0f, 1.0f, "Расстояние на котором трава начинает пропадать")
	MOP_FLOATEXC("Grass angle", 0.0f, 0.0f, 360.0f, "Угол травы")
	MOP_FLOATEXC("Grass angle range", 1.0f, 0.0f, 1.0f, "Уровень разброса углов травы")
	MOP_LONGEXC("Random seed", 1, 1, 1000000, "При разных значениях генерит по разному траву")
	MOP_COLOR("Grass color", Color(1.0f))
	MOP_BOOL("Visible", true)
	MOP_GROUPBEG("Wind params")
		MOP_ANGLESC("Wind direction", 0.0f, "Направление ветра, используется только Y");
		MOP_FLOATEX("Wind power", 1.0f, 0.0f, 100.0f)
		MOP_FLOATEX("Wind color ambient", 0.75f, 0.0f, 1.0f)
		MOP_FLOATEX("Wind color power", 0.4f, -10.0f, 10.0f)
	MOP_GROUPEND()
	MOP_GROUPBEG("Render params")
		MOP_LONGEXC("Level", 0, 0, 100, "Левел отрисовки травы");
		MOP_BOOL("Dynamic lighting", false)
		MOP_BOOLC("Shadow cast", false, "true - трава бросает тени, жутко медленно")
		MOP_BOOLC("Shadow receive | v2 |", true, "true - трава принимает тени")
		MOP_BOOLC("Sea reflection", false, "true - трава рисуется в отражение")
		MOP_BOOLC("Sea refraction", false, "true - трава рисуется в преломление")
		MOP_BOOLC("Draw patch", false, "true - рисует патч на котором растет трава")
		MOP_BOOLC("No swing", false, "No swing geometry in swing machine")
	MOP_GROUPEND()
	MOP_GROUPBEG("Texture patches")
		MOP_STRINGC("Texture", "grass.txx", "Текстура содержащая разные травинки")
		MOP_GROUPBEG("Patch 0")
			MOP_FLOAT("X | P0 |", 0.0f)
			MOP_FLOAT("Y | P0 |", 0.0f)
			MOP_FLOAT("Width | P0 |", 0.5f)
			MOP_FLOAT("Height | P0 |", 0.5f)
			MOP_FLOAT("Width multiplier | P0 |", 1.0f)
			MOP_FLOAT("Height multiplier | P0 |", 1.0f)
			MOP_FLOAT("Probability | P0 |", 100.0f)
			MOP_FLOATEX("Wind power | P0 |", 1.0f, 0.0f, 1.0f)
		MOP_GROUPEND()
		MOP_GROUPBEG("Patch 1")
			MOP_FLOAT("X | P1 |", 0.5f)
			MOP_FLOAT("Y | P1 |", 0.0f)
			MOP_FLOAT("Width | P1 |", 0.5f)
			MOP_FLOAT("Height | P1 |", 0.5f)
			MOP_FLOAT("Width multiplier | P1 |", 1.0f)
			MOP_FLOAT("Height multiplier | P1 |", 1.0f)
			MOP_FLOAT("Probability | P1 |", 0.0f)
			MOP_FLOATEX("Wind power | P1 |", 1.0f, 0.0f, 1.0f)
		MOP_GROUPEND()
		MOP_GROUPBEG("Patch 2")
			MOP_FLOAT("X | P2 |", 0.0f)
			MOP_FLOAT("Y | P2 |", 0.5f)
			MOP_FLOAT("Width | P2 |", 0.5f)
			MOP_FLOAT("Height | P2 |", 0.5f)
			MOP_FLOAT("Width multiplier | P2 |", 1.0f)
			MOP_FLOAT("Height multiplier | P2 |", 1.0f)
			MOP_FLOAT("Probability | P2 |", 0.0f)
			MOP_FLOATEX("Wind power | P2 |", 1.0f, 0.0f, 1.0f)
		MOP_GROUPEND()
		MOP_GROUPBEG("Patch 3")
			MOP_FLOAT("X | P3 |", 0.5f)
			MOP_FLOAT("Y | P3 |", 0.5f)
			MOP_FLOAT("Width | P3 |", 0.5f)
			MOP_FLOAT("Height | P3 |", 0.5f)
			MOP_FLOAT("Width multiplier | P3 |", 1.0f)
			MOP_FLOAT("Height multiplier | P3 |", 1.0f)
			MOP_FLOAT("Probability | P3 |", 0.0f)
			MOP_FLOATEX("Wind power | P3 |", 1.0f, 0.0f, 1.0f)
		MOP_GROUPEND()
	MOP_GROUPEND()
MOP_ENDLIST(Grass)
