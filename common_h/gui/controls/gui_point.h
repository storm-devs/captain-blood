#ifndef _XBOX
#ifndef GUI_POINT
#define GUI_POINT


class GUIPoint
{
public:
	union
	{
		struct
		{
			int X;
			int Y;
		};
		struct
		{
			int x;
			int y;
		};
	};
	
	GUIPoint ();
	GUIPoint (int _X, int _Y);
	~GUIPoint ();
	
	///Присвоить
	GUIPoint & operator = (const GUIPoint & r);
	GUIPoint & operator = (long v);

	GUIPoint & operator += (const GUIPoint & p);
	GUIPoint & operator -= (const GUIPoint & p);
	GUIPoint & operator *= (const GUIPoint & p);
	GUIPoint & operator /= (const GUIPoint & p);
	GUIPoint & operator *= (long v);
	GUIPoint & operator /= (long v);

	GUIPoint operator + (const GUIPoint & p);
	GUIPoint operator - (const GUIPoint & p);
	GUIPoint operator * (const GUIPoint & p);
	GUIPoint operator / (const GUIPoint & p);
	GUIPoint operator * (long v);
	GUIPoint operator / (long v);

};


#endif

#endif