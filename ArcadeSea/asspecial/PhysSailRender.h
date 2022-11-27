#pragma once

class SailRenderInfo : public IClothRenderInfo
{
public:
	struct Vertex
	{
		Vector	pos;
		Vector	normal;
		float	tu,tv;
	};

	// дать размер вершины в байтах
	virtual unsigned int GetSingleVertexSize() const { return sizeof(Vertex); }
	// дать смещение нормали внутри вершины
	virtual unsigned int GetNormalOffset() const { return sizeof(Vector); }
	// дать смещение UV внутри вершины
	virtual unsigned int GetTexCoordOffset() const { return 2*sizeof(Vector); }
	// дать размер индекса в байтах
	virtual unsigned int GetSingleIndexSize() const { return sizeof(DWORD); }
	// сказать нужны ли нормали
	virtual bool NeedNormals() const { return true; }
	// сказать нужны ли текстурные координаты
	virtual bool NeedTexCoords() const { return true; }
};

// рендер физической ткани
class SailRender
{
	struct SailInstanceData
	{
		float		xFreq;
		float		yFreq;
		float		time;
		float		amp;

		Vector4		sailColor;
		//float		sailAlpha;

		Vector4		sailGlimpseColor;
		//float		colorScale;
	};

	IVBuffer		*vb_;
	IIBuffer		*ib_;
//	IVariable		*varFaceOrder_;
	IVariable		*varDiffuseTex_;
	IVariable		*varGlimpseTex_;
	//IVariable		*varInstanceInfo_;
	IVariable		*varSailFreqTimeAmp_;
	IVariable		*varSailColorAlpha_;
	IVariable		*varSailGlimpseColorScale_;
	
	IBaseTexture	*diffuseTex_;
	IBaseTexture	*glimpseTex_;
	
	SailInstanceData data_;

	unsigned int m_oldVertexNum;
	unsigned int m_oldIndicesNum;

	SailRenderInfo *m_pSailRenderInfo;


	ShaderId SailCCW_id;
	//ShaderId SailCW_id;

public:
	SailRender();
	virtual ~SailRender();

	// создать буферы для рендера
	void CreateBuffers(unsigned int vertCount, unsigned int indCount);
	// закачать данные симуляции из PhysX
	void UpdateBuffers(const IPhysCloth::SimulationData& data);
	// отрисовать ткань
	void Render(const IPhysCloth::SimulationData& data);
	// установить текстуру
	void SetTexture(const char* texName);
	// установить цвет
	void SetColor(const Vector& color, float colorScale) { data_.sailColor.v = color; data_.sailGlimpseColor.w = colorScale; }
	// установить прозрачность паруса
	void SetAlpha(float alpha) { data_.sailColor.w = alpha; }
	// установить текстуру просвечивания
	void SetGlimpseTexture(const char* texture);
	// установить цвет паруса на просвет
	void SetGlimpseColor(const Vector& color) { data_.sailGlimpseColor.v = color; }

	IClothRenderInfo& GetRenderInfo() {return *m_pSailRenderInfo;}
};