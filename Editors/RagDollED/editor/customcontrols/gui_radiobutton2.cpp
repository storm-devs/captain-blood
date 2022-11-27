#include "gui_radiobutton2.h"

GUIRadioButton2::GUIRadioButton2(GUIControl *parent, int x, int y, int w, int h, bool tabbed)
	: GUIControl(parent)
{
	this->tabbed = tabbed;

	DrawRect.Left = x;
	DrawRect.Top = y;
	DrawRect.Width = w;
	DrawRect.Height = h;
	
	ImageNormal = NEW GUIImage ();
	ImageChecked = NEW GUIImage ();
	
	pFont = NEW GUIFont ("SansSerif");
	
	ClientRect = DrawRect;
	ClientRect.Left += 2;
	ClientRect.Top += 2;
	ClientRect.Width -= 4;
	ClientRect.Height -= 4;
	
    FontColor = 0xFF000000;
	
	Checked = false;
	
	MD_Time = 0.0f;

	GroupID=0;	

	t_OnClick = NEW GUIEventHandler;
}

GUIRadioButton2::~GUIRadioButton2 ()
{
	delete t_OnClick;
	delete ImageNormal;
	delete ImageChecked;
	
	delete pFont;
	
	//GUIControl::~GUIControl ();
}


void GUIRadioButton2::Draw ()
{
	if( Visible == false )
		return;

	MD_Time += api->GetDeltaTime();

//	cliper.Push ();
	
	int Width = DrawRect.Width;
	int Height = DrawRect.Height;

	GUIRectangle rect = GetClientRect();
	this->ClientToScreen(rect);

//	cliper.SetRectangle(rect);

	GUIPoint np;

	np.X = DrawRect.Left;
	np.Y = DrawRect.Top;

	ClientToScreen(np);
	
	int nLeft = np.X;
	int nTop = np.Y;

	int width = 0; int y = np.Y;

	if( tabbed )
	{
		int w = pFont->GetWidth(Caption) + 12;

		Color col(0xffd4d0c8);

		if( Checked )
		{
			col.r *= 1.15f;
			col.g *= 1.15f;
			col.b *= 1.15f;
		}
		else
		{
			col.r *= 1.03f;
			col.g *= 1.03f;
			col.b *= 1.03f;
		}

		GUIHelper::Draw2DRect(rect.x,rect.y,w,rect.h,col);

		y += (Height - pFont->GetHeight())/2 - 1;
	}
	else
	{
		width = ImageNormal->GetWidth();

		GUIImage *image = Checked ? ImageChecked : ImageNormal;
	
		GUIHelper::DrawSprite(nLeft + 3,np.Y + 3,width,image->GetHeight(),image);
	}

	pFont->Print(nLeft + width + 6,y,FontColor,Caption);

//	cliper.Pop();

	GUIControl::Draw();
}


bool GUIRadioButton2::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
 	if (Visible == false) return false;

	bool Handled = false;
	
	Handled = GUIControl::ProcessMessages (message, lparam, hparam);
	if (Handled) return Handled;
	
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos (message, lparam, hparam, cursor_pos);
	
	
	if (message == GUIMSG_LMB_DOWN) 
	{
		Handled = true;
		OnMDown (message, cursor_pos);
	}
/*	
	if (message == GUIMSG_LMB_UP) 
	{
		Handled = true;
	}
	
	if (message == GUIMSG_MOUSEENTER) 
	{
		Handled = true;
	}
	
	if (message == GUIMSG_MOUSELEAVE) 
	{
		Handled = true;
	}
*/
	if (message == GUIMSG_DISABLERADIOBUTTON)
	{
		Checked = false;
		MD_Time = 0.0f;
	}
	
	return Handled;
}


void GUIRadioButton2::OnMDown (int MouseKey, const GUIPoint& pt)
{
	if (MD_Time < 0.15f ) return;
	
	GUIRectangle sRect = GetScreenRect ();
	//sRect.Width = 20;
	if (GUIHelper::PointInRect(pt, sRect))
	{
		GUIControl* parent = this->parent;
		if (parent != NULL)
		{
			for (unsigned long n =0; n < parent->Childs.Size(); n++)
			{
				GUIControl* pChild = parent->Childs[n];

				if (pChild->Is("GUIRadioButton2"))
				{
					GUIRadioButton2* rbutton=(GUIRadioButton2*)pChild;

					if (rbutton->GroupID==GroupID)
					{
						pChild->SendMessage (GUIMSG_DISABLERADIOBUTTON);
					}					
				}				
			}
		}

		Checked = true;
		MD_Time = 0.0f;

		t_OnClick->Execute(this);
	}
	
	
}
