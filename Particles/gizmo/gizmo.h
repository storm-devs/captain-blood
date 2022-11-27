#ifndef EDITOR_TRANSFORM_GIZMO
#define EDITOR_TRANSFORM_GIZMO



#include "..\..\common_h\render.h"
#include "..\..\common_h\iconsole.h"
#include "..\..\common_h\math3d.h"
#include "..\..\common_h\templates.h"




class TransformGizmo
{
public:

	enum GizmoType
	{
		GT_MOVE,
		GT_ROTATE,

		GT_FORCE_DWORD = 0x7fffff
	};

	enum GizmoMode
	{
		GM_WORLD,
		GM_LOCAL,

		GM_FORCE_DWORD = 0x7fffffff
	};

private:

	enum GizmoAxis
	{
		GA_AXIS_NONE,
		GA_AXIS_X,
		GA_AXIS_Y,
		GA_AXIS_Z,

		GA_FORCE_DWORD = 0x7fffffff

	};

	void DrawMoveGizmo ();
	void DrawRotateGizmo ();

	struct Segment
	{
		Vector vStart;
		Vector vEnd;

		Segment()
		{
		}

		Segment(const Vector &v1, const Vector &v2)
		{
			vStart = v1;
			vEnd = v2;
		}
	};


	IRender* pRS;
	IConsole* pConsole;
	GizmoType mType;
	GizmoMode mMode;
	Matrix mTransform;

	array<Segment> XAxis;
	array<Segment> YAxis;
	array<Segment> ZAxis;

	bool bMousePressed;

	GizmoAxis HighlightedAxis;

	//Матрицы проецирования и камеры, для извлечения луча из курсора мыши
	Matrix matGizmoView;
	Matrix matGizmoViewInv;
	Matrix matGizmoProjection;
	RENDERVIEWPORT GizmoviewPort;


	float fDeltaMove;


	void PushLine (GizmoAxis mode, const Vector& v1, const Vector& v2);


	void GetPickupRay (const Matrix& matProjection, const Matrix& matView, const RENDERVIEWPORT& ViewPort, float sX, float sY, Vector& rayStart, Vector& rayEnd);
	float GetDistanceBetweenPointAndLine (const Vector& line_start, const Vector& line_end, const Vector& point, bool bSegmentCollision);
	bool LineLineIntersection (const Vector& line1_start, const Vector& line1_end, const Vector& line2_start, const Vector& line2_end, float Epsilon, Vector& intersection_point, bool bSegmentCollision);


	bool HighlightAxis(float fMouseX, float fMouseY);

	GizmoAxis SelectedAxis;

	bool NeedDrawRotateGizmoLine (const Vector &v1, const Vector &v2, const Vector& vCamDir);



	bool GizmoChangeTransform (float fMouseX, float fMouseY);

	bool GizmoChangeTransformMove (float fMouseX, float fMouseY);
	bool GizmoChangeTransformRotate (float fMouseX, float fMouseY);


	Vector PointOfAxisSelect;

	Vector MousePressedPoint;

	Vector vMouseLine_Start;
	Vector vMouseLine_End;
	Vector vMouseLine_Dir;
	float fMouseLine_Len;

	bool bEnabled;


	void CreateMouseLine(const Vector& v1, const Vector& v2);

	float GizmoScale;

	void CalcGizmoScale ();

  
public:

	TransformGizmo(GizmoType _type, GizmoMode _mode, const Matrix& _matTransform);
	~TransformGizmo();


	void Draw ();

	bool HandleMessages(unsigned int uMsg, int LocalMouseX, int LocalMouseY);


	void SetView (const Matrix& mView);
	void SetProjection  (const Matrix& mProjection);
	void SetViewPort (const RENDERVIEWPORT& viewport);

	const Matrix& GetTransform ();


	void SetType (GizmoType _type);
	void SetMode (GizmoMode _mode);
	GizmoType GetType ();
	GizmoMode GetMode ();

	void SetTransform (const Matrix& m);

	//Gizmo в "активном" режиме, юзер тащит объект...
	bool IsActiveMode();


	//Включено GIZMO или нет
	//когда отключено, ничего не делает...
	void Enable (bool _bEnabled);
	bool IsEnabled ();


};

#endif