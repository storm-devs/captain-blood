//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxToolListBox	
//============================================================================================

#ifndef _AnxToolListBox_h_
#define _AnxToolListBox_h_

#include "..\AnxBase.h"

class GraphNodeBase;

class AnxToolListBox : public GUIWindow
{
public:
	class LB : public GUIListBox
	{
	public:
		LB(GUIControl * p, int Width, int Height) : GUIListBox(p, 0, 0, Width, Height){ SelectColor = 0xff2040c0; hintObj = null; setHint = null; };
		virtual void DrawListBox (long nLeft, long nTop, long Width, long Height){};
		virtual void Draw();

		virtual bool BeforeHintShow ()
		{
		//	if(hintObj && setHint) (hintObj->*setHint)();
			return true;
		};
		AnxToolListBox * hintObj;
		void (_cdecl AnxToolListBox::* setHint)();
	};

//--------------------------------------------------------------------------------------------
public:
	AnxToolListBox(int Left, int Top, int Width, int Height);
	virtual ~AnxToolListBox();

//--------------------------------------------------------------------------------------------
public:
	void _cdecl FillNodes(GraphNodeBase * node, bool isRec = true, AnxNodeTypes mask = anxnt_error);
	inline GUIListBox * List(){ return listBox; };

//--------------------------------------------------------------------------------------------
private:
	virtual void Draw();
	virtual void MouseDown(int button, const GUIPoint& pt);
	LB * listBox;
	static bool _cdecl ABSort(const string & s1, const string & s2);

	void _cdecl SetHint();
};

#endif

