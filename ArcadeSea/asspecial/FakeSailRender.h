#pragma once

// рендер фейковой ткани
class FakeRender
{
	struct Vertex
	{
		Vector	pos;	// позиция
		Vector	normal;	// нормаль
		float	tu,tv;	// текстурные координаты
		float	blend;	// весовой коэффициент вершины (0 = вершина не анимируется)
	};

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
	unsigned int	vertCount_;
	unsigned int	indCount_;

	//ShaderId FakeSailCW_id;
	ShaderId FakeSailCCW_id;

	IVariable		*varDiffuseTex_;
	IVariable		*varGlimpseTex_;
//	IVariable		*varFaceOrder_;
	//IVariable		*varInstanceInfo_;
	IVariable		*varSailFreqTimeAmp_;
	IVariable		*varSailColorAlpha_;
	IVariable		*varSailGlimpseColorScale_;
	IBaseTexture	*diffuseTex_;
	IBaseTexture	*glimpseTex_;

	unsigned int m_nHorzQuantity;
	unsigned int m_nVertQuantity;

	Box				m_boundBox;

	SailInstanceData	data_;
	float fTimePeriod;

	// параметры треугольного паруса (для трейса)
	bool			m_bIsTriangle;
	Plane			m_plnTriangle;
	Vector			m_vTriangle[3];
	Vector			m_vTriangleHDir;
	float			m_fTriangleHDist;
	Vector			m_vTriangleVDir;
	float			m_fTriangleVDist;
	// параметры прямоугольного паруса (для трейса)
	struct TraceParams
	{
		Matrix	mtx;
		Plane	plane1;
		Plane	plane2;

		float	thick;
		float	width;
		float	height;
	};
	TraceParams		m_sailtrace;

public:
	FakeRender();
	~FakeRender();

	// создать и заполнить буферы
	void InitBuffers(const IClothMeshBuilder& mesh);
	// освободить буферы
	void FreeBuffers();

	// отрисовать
	void Render(const Matrix& mtx);
	// установить текстуру
	void SetTexture(const char* texName);
	// установить цвет
	void SetColor(const Vector& color, float scale = 1.0f) { data_.sailColor.v = color; data_.sailGlimpseColor.w = scale; }
	// установить частоты волн
	void SetFreq(float xFreq, float yFreq) { data_.xFreq = xFreq; data_.yFreq = yFreq; CalculateTimePeriod(); }
	// установить прозрачность паруса
	void SetAlpha(float alpha) { data_.sailColor.w = alpha; }
	// установить текстуру просвечивания
	void SetGlimpseTexture(const char* texture);
	// установить цвет паруса на просвет
	void SetGlimpseColor(const Vector& color) { data_.sailGlimpseColor = color; }
	// установить амплитуду анимации
	void SetFakeAnimAmplitude(float amp) { data_.amp = amp; }
	// инкрементировать время анимации
	void AddAnimTime(float fDeltaTime);
	// получить бокс вокруг паруса
	void GetBoundBox(Box& bb) {bb=m_boundBox;}
	// фейковый трейс луча через парус
	bool FakeRaycast(const Matrix& mtxSailTransform, const Vector& vSrc, const Vector& vDst, unsigned int* pVertexID);
	// установить параметры паруса
	void SetSailParams(long nHeightQ, long nVertQ);
	// установить параметры для треугольного паруса
	void SetTriangleSailParams(const Vector * v);
	// установить параметры для четырехугольного паруса
	void SetSquareSailParams(const Vector * v,float fWindCurvature);

protected:
	void CalculateTimePeriod();
};
