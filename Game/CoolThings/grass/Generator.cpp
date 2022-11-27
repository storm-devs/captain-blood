#include "Grass.h"

#if !defined(NO_TOOLS) && !defined(_XBOX)

bool Grass::m_currentGeneratingGrass;

int	Grass::m_lodselector[8][8] = {	{3, 1, 2, 1, 3, 1, 2, 1}, 
									{1, 2, 1, 2, 1, 2, 1, 1}, 
									{2, 1, 3, 1, 2, 1, 3, 1}, 
									{1, 2, 1, 2, 1, 2, 1, 1}, 
									{3, 1, 2, 1, 3, 1, 2, 1}, 
									{1, 2, 1, 2, 1, 2, 1, 1}, 
									{2, 1, 3, 1, 2, 1, 3, 1}, 
									{1, 2, 1, 2, 1, 2, 1, 1} };

bool Grass::EditMode_Export()
{
	LoadDataFile();

	if (!m_loadDataBuffer || !m_loadDataBuffer->Size())
	{
		PrepareGrass(true);

		while (m_startedGeneration)
			ProcessBlock4x4();
	}

	return true;
}

void Grass::PrepareGrass(bool isExport)
{
	if (!m_regenerateGrass || m_patchName.IsEmpty())
		return;

	// если какая-то трава уже генерится, то пока выходим
	if (!isExport && m_currentGeneratingGrass)
		return;

	m_currentGeneratingGrass = true;

	// удаляем файл
	Files().Delete(m_grassPath);

	// удаляем уже загруженный data файл
	RELEASE(m_loadDataBuffer);

	m_regenerateGrass = false;

	m_mainData.Free();
	m_blocks44.Free();

	RELEASE(m_rigidBody);

	IGMXScene * tmpGeo = Geometry().CreateScene(m_patchName.c_str(), &Animation(), &Particles(), &Sound(), _FL_);
	RELEASE(m_geo);
	m_geo = tmpGeo;
	if (!m_geo)
	{
		api->Error("Grass error: Can't create geometry, grass: %s, geo: %s", GetObjectID().c_str(), m_patchName);
		return;
	}

	m_rigidBody = m_geo->CreatePhysicsActor(Physics(), false);
	if (!m_rigidBody)
	{
		m_rigidBody = m_geo->CreatePhysicsActor(Physics(), false);
		api->Error("Grass error: Can't create physics actor, grass: %s, geo: %s", GetObjectID().c_str(), m_patchName);
		return;
	}
	m_rigidBody->SetGroup(phys_grass);
	m_rigidBody->SetUserPointer(this);

	// прокручиваем цикл физики, чтобы прогрузился и поставился патч
	Physics().UpdateEndFrame(0.15f);
	Physics().UpdateBeginFrame(0.15f);
	
	// 1. Вычисляем бокс геометрии
	m_bbox = m_geo->GetBound();
	m_obb.SetFromMinMax(m_bbox.vMin, m_bbox.vMax);

	float sizex = (m_bbox.vMax.x - m_bbox.vMin.x);
	float sizez = (m_bbox.vMax.z - m_bbox.vMin.z);

	m_grassx = dword(sizex * m_density);
	m_grassz = dword(sizez * m_density);

	// 2. Заводим нужное кол-во блоков
	m_blockx = (m_grassx + 15) / 16;
	m_blockz = (m_grassz + 15) / 16;

	m_b44x = (m_blockx + 3) / 4;
	m_b44z = (m_blockz + 3) / 4;

	m_current44 = 0;

	m_numGrasses = 0;

	m_processBlockNumber = 0;
	m_blockNumber = 0;

	m_startedGeneration = true;

	if (m_randomGenerator == 0)
		srand(GetTickCount());
}

void Grass::PackBlock4x4(Block44 & b44)
{
	// создаем непрерывный кусок памяти для этого 4x4 блока
	Vector center = (b44.bbox_min + b44.bbox_max) * 0.5f;
	float radius = (b44.bbox_max - b44.bbox_min).GetLength();

	// добавляем центр блока 4x4 и его квадрат радиуса
	dword temp[6];
	memcpy(temp, &center, 3 * sizeof(dword));
	memcpy(&temp[3], &radius, sizeof(dword));
	for (int i=0; i<4; i++)
		b44.data.Add( temp[i] );

	// Добавляем кол-во блоков 2x2
	b44.data.Add( b44.blocks22.Size() );

	// теперь идем по блокам и добавляем их параметры и их травинки
	// в свои собственные массивы, а смещение к каждому добавляем в b44.data
	dword curBlockOffset = 0;
	for (int i=0; i<b44.blocks22.Len(); i++)
	{
		Block22 & block22 = b44.blocks22[i];

		b44.data.Add(curBlockOffset);

		PackBlock2x2(block22);

		curBlockOffset += block22.data.Len();
	}

	// добавляем каждый block.data в b44.data
	for (int i=0; i<b44.blocks22.Len(); i++)
	{
		Block22 & block22 = b44.blocks22[i];

		for (int j=0; j<block22.data.Len(); j++)
			b44.data.Add( block22.data[j] );

		block22.data.Free();
	}
}

void Grass::PackBlock2x2(Block22 & b22)
{
	short minmax[6];
	for (int i=0; i<3; i++)
	{
		b22.bbox_min.v[i] = floorf(b22.bbox_min.v[i]);
		minmax[i] = short(b22.bbox_min.v[i]) + 16000;

		b22.bbox_max.v[i] = ceilf(b22.bbox_max.v[i]);
		minmax[3 + i] = short(b22.bbox_max.v[i]) + 16000;
	}

	//Assert(b22.bbox_max.x - b22.bbox_min.x > 0.9999f);
	//Assert(b22.bbox_max.y - b22.bbox_min.y > 0.9999f);
	//Assert(b22.bbox_max.z - b22.bbox_min.z > 0.9999f);

	// записываем bbox min/max
	dword temp[6];
	memcpy(&temp[0], &minmax[0], 3 * sizeof(dword));
	for (int k=0; k<3; k++)
		b22.data.Add( temp[k] );

	// Добавляем кол-во блоков
	b22.data.Add( b22.blocks.Size() );

	// теперь идем по блокам и добавляем их параметры и их травинки
	// в свои собственные массивы, а смещение к каждому добавляем в b22.data
	dword curBlockOffset = 0;
	for (int i=0; i<b22.blocks.Len(); i++)
	{
		Block & block = b22.blocks[i];

		b22.data.Add(curBlockOffset);

		PackBlock(block, b22.bbox_min, b22.bbox_max);

		curBlockOffset += block.data.Len();
	}

	// добавляем каждый block.data в b44.data
	for (int i=0; i<b22.blocks.Len(); i++)
	{
		Block & block = b22.blocks[i];

		for (int j=0; j<block.data.Len(); j++)
			b22.data.Add( block.data[j] );

		block.data.Free();
	}
}

void Grass::PackBlock(Block & block, const Vector & b22min, const Vector & b22max)
{
	if (m_randomGenerator)
	{
		srand(m_randomGenerator + 1 + m_blockNumber * 12345);
		m_blockNumber++;
	}

	short offsets[2];

	offsets[0] = block.x_offset;
	offsets[1] = block.z_offset;

	// записываем bbox min/max
	dword temp[6];
	memcpy(&temp[0], &offsets[0], sizeof(dword));
	block.data.Add( temp[0] );

	// записываем кол-во травинок
	LodsNum nums;
	nums.lods[0] = byte(block.lod3.Size());
	nums.lods[1] = byte(block.lod2.Size());
	nums.lods[2] = byte(block.lod1.Size());
	nums.lods[3] = 0;
	block.data.Add(nums.dw_tmp);

	PackLod(block, block.lod3, b22min, b22max);
	PackLod(block, block.lod2, b22min, b22max);
	PackLod(block, block.lod1, b22min, b22max);
}

void Grass::PackLod(Block & block, array<Grasses> & lod, const Vector & b22min, const Vector & b22max)
{
	for (int j=0; j<lod.Len(); j++)
	{
		Vector coeff = (lod[j].pos - b22min) / (b22max - b22min);

		dword red	= dword(coeff.x * 255.0f);
		dword green = dword(coeff.y * 255.0f);
		dword blue  = dword(coeff.z * 255.0f);

		float rnd = Rnd(0.999999f);
		dword texture = 0;
		for (texture=0; texture<3; texture++)
			if (rnd <= m_probabilities[texture])
				break;

		dword tmp = ARGB(/*(lod[j].mtxIndex << 2L) |*/ texture, red, green, blue);

		// записываем травинку
		block.data.Add(tmp);
	}
}

void Grass::ProcessBlock4x4()
{
	if (m_current44 >= m_b44x * m_b44z)
	{
		m_startedGeneration = false;

		if (m_mainData.IsEmpty())
		{
			// добавляем версию
			m_mainData.Add(GRASS_VERSION);

			// создаем куски памяти с блоками
			for (int i=0; i<m_blocks44.Len(); i++)
				PackBlock4x4(m_blocks44[i]);

			dword cur44Offset = 0;

			// записываем кол-во блоков 4x4
			m_mainData.Add( m_blocks44.Size() );

			// записываем смещения блоков 4x4
			for (int i=0; i<m_blocks44.Len(); i++)
			{
				m_mainData.Add( cur44Offset );

				cur44Offset += m_blocks44[i].data.Size();
			}

			// записываем данные блоков 4x4
			for (int i=0; i<m_blocks44.Len(); i++)
			{
				Block44 & b44 = m_blocks44[i];

				for (int j=0; j<b44.data.Len(); j++)
					m_mainData.Add( b44.data[j] );
			}

			// создаем подкаталоги
			string path;
			path.GetFilePath(string(m_grassPath));
			Files().CreateFolder(path);

			// свизлим для xbox
			for (int i=0; i<m_mainData.Len(); i++)
				__RefDataSwizzler( m_mainData[i] );

			// записываем в файл
			IFile * file = Files().OpenFile(m_grassPath, file_create_always, _FL_);
			file->Write(m_mainData.GetBuffer(), m_mainData.GetDataSize());
			RELEASE(file);

			LoadDataFile();

			m_mainData.Free();
			m_blocks44.Free();
		}

		m_currentGeneratingGrass = false;

		return;
	}

	Block44 & b44 = m_blocks44[m_blocks44.Add()];
	
	b44.bbox_min = 1e+6f;
	b44.bbox_max = -1e+6f;

	int cz = m_current44 / m_b44x;
	int cx = m_current44 - cz * m_b44x;

	int sizex = Min(4, m_blockx - cx * 4);
	int sizez = Min(4, m_blockz - cz * 4);

	bool isHit = false;

	for (int z = 0; z < sizez; z++)
		for (int x = 0; x < sizex; x++)
		{
			if (ProcessBlock2x2(b44, cx * 4 + x, cz * 4 + z))
			{
				isHit = true;
				b44.bbox_min.Min(b44.blocks22[ b44.blocks22.Last() ].bbox_min);
				b44.bbox_max.Max(b44.blocks22[ b44.blocks22.Last() ].bbox_max);
			}
		}

	if (isHit)
	{
	}
	else
		m_blocks44.DelIndex( m_blocks44.Last() );

	m_current44++;
}

bool Grass::ProcessBlock2x2(Block44 & b44, int bx, int bz)
{
	int gx = Min(16, m_grassx - bx * 16);
	int gz = Min(16, m_grassz - bz * 16);

	bool isHit = false;

	Block22 & block22 = b44.blocks22[ b44.blocks22.Add() ];

	block22.bbox_min = 1e+6f;
	block22.bbox_max = -1e+6f;

	for (int z = 0; z < (gz+7) / 8; z++)
		for (int x = 0; x < (gx+7) / 8; x++)
		{
			if (ProcessBlock(block22, bx * 16 + x * 8, bz * 16 + z * 8, Min(8, gx - x * 8), Min(8, gz - z * 8), x, z))
			{
				isHit = true;
				block22.bbox_min.Min(block22.blocks[ block22.blocks.Last() ].bbox_min);
				block22.bbox_max.Max(block22.blocks[ block22.blocks.Last() ].bbox_max);
			}
		}

	if (!isHit)
	{
		// 
		b44.blocks22.DelIndex( b44.blocks22.Last() );
	}

	return isHit;
}

bool Grass::ProcessBlock(Block22 & b22, int bx, int bz, int gx, int gz, int x_offset, int z_offset)
{
	if (m_randomGenerator)
	{
		srand(m_randomGenerator + 1 + m_processBlockNumber * 21534);
		m_blockNumber++;
	}

	bool isHit = false;

	Block & block = b22.blocks[ b22.blocks.Add() ];

	block.bbox_min = 1e+6f;
	block.bbox_max = -1e+6f;

	block.x_offset = x_offset;
	block.z_offset = z_offset;

	array<Grasses> * lods[3] = { &block.lod1, &block.lod2, &block.lod3 };

	for (int z = 0; z < gz; z++)
		for (int x = 0; x < gx; x++)
		{
			Vector v1, v2;
			float rndx = 0.3f * RRnd(-m_density * 1.0f, m_density * 1.0f);
			float rndz = 0.3f * RRnd(-m_density * 1.0f, m_density * 1.0f);

			if (m_randomPosition)
			{
				if (rand() < 1000)
					continue;

				v1.x = v2.x = m_bbox.vMin.x + float(bx + Rnd(float(gx))) / m_density + rndx;
				v1.z = v2.z = m_bbox.vMin.z + float(bz + Rnd(float(gz))) / m_density + rndz;
			}
			else
			{
				v1.x = v2.x = m_bbox.vMin.x + float(bx + x) / m_density + rndx;
				v1.z = v2.z = m_bbox.vMin.z + float(bz + z) / m_density + rndz;
			}

			v1.y = m_bbox.vMax.y + 1.0f;
			v2.y = m_bbox.vMin.y - 1.0f;
			IPhysicsScene::RaycastResult detail;
			int max_cnt = 10;
			while (max_cnt--)
			{
				IPhysBase * bg = Physics().Raycast(v1, v2, phys_mask(phys_grass), &detail);
				if (!bg)
					break;

				if (bg->UserPointer() != this)
				{
					if (fabsf(v1.y - v2.y) <= 0.007f)
						break;

					v1.y = detail.position.y + 0.01f;
					continue;
				}

				block.bbox_min.Min(detail.position);
				block.bbox_max.Max(detail.position);

				isHit = true;

				array<Grasses> & lod = *lods[m_lodselector[x][z] - 1];
				Grasses & grasses = lod[ lod.Add() ];
				grasses.pos = detail.position;

				/*float bestDot = -2.0f;
				int best = 0;
				for (int i=0; i<21; i++)
				{
					float dot = GH.vecNormals[i] | detail.normal;
					if (dot > bestDot)
					{
						bestDot = dot;
						best = i;
					}
				}

				grasses.mtxIndex = best;*/
	
				m_numGrasses++;
				break;
			}
		}

	if (!isHit)
	{
		// 
		b22.blocks.DelIndex( b22.blocks.Last() );
	}

	return isHit;
}

#endif