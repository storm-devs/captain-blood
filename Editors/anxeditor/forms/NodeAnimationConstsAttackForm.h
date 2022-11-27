//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationConstsAttackForm
//============================================================================================

#ifndef _NodeAnimationConstsAttackForm_h_
#define _NodeAnimationConstsAttackForm_h_

#include "..\AnxBase.h"

class NodeAnimationConstsAttackForm : public GUIControl
{
	struct Vertex
	{
		Vector p;
		dword c;
	};
	
	struct VertexUV
	{
		Vector p;
		float u, v;
	};

	enum DragState
	{
		ds_none,
		ds_sarc,
		ds_earc,
		ds_sside,
		ds_eside,
	};

//--------------------------------------------------------------------------------------------
public:
	NodeAnimationConstsAttackForm(AnxOptions & options, GUIControl * parent, const GUIPoint & position, long w);
	~NodeAnimationConstsAttackForm();

//--------------------------------------------------------------------------------------------
public:
	virtual void Draw();

private:
	bool OnArc(const Vector & p, float rad);
	bool OnSide(const Vector & p, float ang);
	void DragArc(const Vector & p, float * rad);
	void DragSide(const Vector & p, float * ang);


//--------------------------------------------------------------------------------------------
public:
	float * angleStart;
	float * angleEnd;
	float * radiusStart;
	float * radiusEnd;

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	float scale;
	Vertex sector[1024][2];
	GUIImage man;
	float dragDlt;
	DragState drag;
	IVariable * texture;
};

#endif

