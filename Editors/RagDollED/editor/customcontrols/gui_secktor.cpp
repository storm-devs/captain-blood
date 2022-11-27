	
#include "gui_secktor.h"


GUISecktor::GUISecktor (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	pFont = NEW GUIFont ("SansSerif");
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;

	fStartAngle=0.0f;
	fEndAngle=360.0f;

	IsDragged=false;

	Type=secktor_circle;

	NeedDrag=false;

	dwSecktorColor=0xff0000ff;

	Range = GUIHelper::GetRender()->GetTechniqueGlobalVariable("SectorRange",_FL_);
}

GUISecktor::~GUISecktor ()
{
	DELETE(pFont)

	Range = null;
}

void GUISecktor::Draw ()
{
	if (Visible == false) return;	

	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	
	GUIRectangle rect = GetDrawRect ();
	this->ClientToScreen (rect);

	const GUIRectangle &DrawRect = rect;
	

//	int index=0;
//	int iNumSectors=36;

/*	Points[0].x= DrawRect.x+(int)(DrawRect.Width*0.5f);
	Points[0].y= DrawRect.y+(int)(DrawRect.Height*0.5f);

	for (int i=0;i<=iNumSectors;i++)
	{
		Points[index*2+0+1].x=(int)( sinf((float)(i)*360.0f/(float)iNumSectors*PI/180.0f)*DrawRect.Width*0.5f+DrawRect.x+DrawRect.Width*0.5f);
		Points[index*2+0+1].y=(int)(-cosf((float)(i)*360.0f/(float)iNumSectors*PI/180.0f)*DrawRect.Height*0.5f+DrawRect.y+DrawRect.Height*0.5f);		

		Points[index*2+1+1].x=(int)( sin((float)(i+1)*360.0f/(float)iNumSectors*PI/180)*DrawRect.Width*0.5f+DrawRect.x+DrawRect.Width*0.5f);
		Points[index*2+1+1].y=(int)(-cos((float)(i+1)*360.0f/(float)iNumSectors*PI/180)*DrawRect.Height*0.5f+DrawRect.y+DrawRect.Height*0.5f);

		index++;
	}		

	GUIHelper::DrawPolygon(Points , index*2, 0xffeeeeee);*/

	const RENDERVIEWPORT &vp = GUIHelper::GetRender()->GetViewport();

	float kx = 2.0f/vp.Width;
	float ky = 2.0f/vp.Height;

	float l = kx*rect.x - 1.0f; float r = l + kx*rect.w;
	float t = 1.0f - ky*rect.y; float b = t - ky*rect.h;

	Vector v[4];
	
	v[0] = Vector(l,t,0.0f);
	v[1] = Vector(r,t,0.1f);
	v[2] = Vector(r,b,0.3f);
	v[3] = Vector(l,b,0.2f);

	if( Range )
		Range->SetVector4(Vector4(0,PI*2,0,0));

	GUIHelper::GetRender()->DrawPolygon(v,4,0xffeeeeee,Matrix(),"RagdollSector");


/*	index=0;
		
	float fTmpAngle=fStartAngle;

	if (fTmpAngle>fEndAngle) fTmpAngle-=360;

	int k=(int)(fTmpAngle/(360.0f/(float)iNumSectors));
	
	int k2=(int)(fEndAngle/(360.0f/(float)iNumSectors));

	Points[0].x=(int)(DrawRect.x+DrawRect.Width*0.5f);
	Points[0].y=(int)(DrawRect.y+DrawRect.Height*0.5f);

	for (int i=k;i<=k2;i++)
	{
		if (i==k)
		{
			Points[index*2+0+1].x=(int)( sin(fStartAngle*PI/180)*DrawRect.Width*0.5f+DrawRect.x+DrawRect.Width*0.5f);
			Points[index*2+0+1].y=(int)(-cos(fStartAngle*PI/180)*DrawRect.Height*0.5f+DrawRect.y+DrawRect.Height*0.5f);
		}
		else
		{
			Points[index*2+0+1].x=(int)( sin((float)(i)*360.0f/(float)iNumSectors*PI/180)*DrawRect.Width*0.5f+DrawRect.x+DrawRect.Width*0.5f);
			Points[index*2+0+1].y=(int)(-cos((float)(i)*360.0f/(float)iNumSectors*PI/180)*DrawRect.Height*0.5f+DrawRect.y+DrawRect.Height*0.5f);
		}
				
		if (i+1==k2)
		{
			Points[index*2+1+1].x=(int)( sin(fEndAngle*PI/180)*DrawRect.Width*0.5f+DrawRect.x+DrawRect.Width*0.5f);
			Points[index*2+1+1].y=(int)(-cos(fEndAngle*PI/180)*DrawRect.Height*0.5f+DrawRect.y+DrawRect.Height*0.5f);			
		}
		else
		{
			Points[index*2+1+1].x=(int)( sin((float)(i+1)*360.0f/(float)iNumSectors*PI/180)*DrawRect.Width*0.5f+DrawRect.x+DrawRect.Width*0.5f);
			Points[index*2+1+1].y=(int)(-cos((float)(i+1)*360.0f/(float)iNumSectors*PI/180)*DrawRect.Height*0.5f+DrawRect.y+DrawRect.Height*0.5f);
		}

		index++;
	}

	GUIHelper::DrawPolygon(Points,index*2,dwSecktorColor);*/

//	float fTmpAngle=fStartAngle;

//	if (fTmpAngle>fEndAngle) fTmpAngle-=360;

	if( Range )
		Range->SetVector4(Vector4(fStartAngle*PI/180,fEndAngle*PI/180,0,0));

	GUIHelper::GetRender()->DrawPolygon(v,4,dwSecktorColor,Matrix(),"RagdollSector");

	//////////////////////////////////////////////////////////////////////

	rect.x += (int)(rect.Width*0.5f) - 2;
	rect.Width = 4;
	rect.Height = (int)(rect.Height*0.525f);

	GUIHelper::Draw2DRect(rect.x + 1,rect.y,rect.Width - 1,rect.Height,0xff00a0ff);
}


bool GUISecktor::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{		

	return GUIControl::ProcessMessages (message, lparam, hparam);
}

void GUISecktor::MouseUp (int button, const GUIPoint& pt)
{	
	IsDragged=false;	
}

void GUISecktor::MouseMove (int button, const GUIPoint& pt)
{
	if (IsDragged==true)
	{
		GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();

		GUIPoint LocalMousePos = MousePos;
		ScreenToClient(LocalMousePos);
		
		LocalMousePos.x-=(int)(DrawRect.Width*0.5f);
		LocalMousePos.y-=(int)(DrawRect.Height*0.5f);

		Vector v=Vector((float)LocalMousePos.x,0,(float)LocalMousePos.y);

		v.Normalize();
		
		Vector up=Vector(0,0,1.0f);

		float fAngle=v.GetAngleXZ(up)/(PI/180)+180;

		if (Type==secktor_circle)
		{
			if (NeedDrag)
			{
				if (SelAngle==0)
				{
					fStartAngle=fAngle;					
				}
			}
			else
			{
				fStartAngle=fAngle;
			}			
		}
		else
		if (Type==secktor_simetric)
		{
			if (NeedDrag)
			{
				if (SelAngle!=-1)
				{
					if (v.x>0)
					{
						SelAngle=0;
					}
					else
					{
						SelAngle=1;
					}

					if (SelAngle==0)
					{
						fStartAngle=fAngle;
						fEndAngle=360-fAngle;
					}
					else
					if (SelAngle==1)
					{
						fStartAngle=360-fAngle;
						fEndAngle=fAngle;
					}
				}
			}
			else
			{		
				if (v.x>0)
				{
					fStartAngle=fAngle;
					fEndAngle=360-fAngle;
				}
				else
				{
					fStartAngle=360-fAngle;
					fEndAngle=fAngle;
				}
			}	
		}
		else
		if (Type==secktor_assimetric)
		{
			if (NeedDrag)
			{
				if (SelAngle==0)
				{
					fStartAngle=fAngle;						
				}
				else
				if (SelAngle==1)
				{						
					fEndAngle=fAngle;
				}				
			}
			else
			{			
				if (v.x>0)
				{
					fStartAngle=fAngle;
				}
				else
				{
					fEndAngle=fAngle;
				}	
			}	
		}
	}
}

void GUISecktor::MouseDown (int button, const GUIPoint& pt)
{	
	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();

	GUIPoint LocalMousePos = MousePos;
	ScreenToClient(LocalMousePos);

	if (LocalMousePos.x<0||LocalMousePos.x>DrawRect.Width) return;
	if (LocalMousePos.y<0||LocalMousePos.y>DrawRect.Height) return;

	LocalMousePos.x-=(int)(DrawRect.Width*0.5f);
	LocalMousePos.y-=(int)(DrawRect.Height*0.5f);
	

	Vector v=Vector((float)LocalMousePos.x,0,(float)LocalMousePos.y);

	v.Normalize();
	
	Vector up=Vector(0,0,1.0f);
	
	float fAngle=v.GetAngleXZ(up)/(PI/180)+180;

	enum ESecktorType
	{
		secktor_circle=0,
		secktor_simetric,
		secktor_assimmetric
	};

	if (Type==secktor_circle)
	{
		if (NeedDrag)
		{
			if (fStartAngle-10<fAngle&&fAngle<fStartAngle+10)
			{
				SelAngle=0;
			}
			else
			{
				SelAngle=-1;
			}

		}
		else
		{
			SelAngle=0;

			fStartAngle=fAngle;
		}		
	}
	else
	if (Type==secktor_simetric)
	{
		if (NeedDrag)
		{
			if (fStartAngle-10<fAngle&&fAngle<fStartAngle+10)
			{
				SelAngle=0;
			}
			else
			if (fEndAngle-10<fAngle&&fAngle<fEndAngle+10)
			{
				SelAngle=1;
			}
			else
			{
				SelAngle=-1;
			}
		}
		else
		{
			if (v.x>0)
			{
				fStartAngle=fAngle;
				fEndAngle=360-fAngle;
			}
			else
			{
				fStartAngle=360-fAngle;
				fEndAngle=fAngle;
			}
		}
	}
	else
	if (Type==secktor_assimetric)
	{
		if (NeedDrag)
		{
			if (fStartAngle-10<fAngle&&fAngle<fStartAngle+10)
			{
				SelAngle=0;
			}
			else
			if (fEndAngle-10<fAngle&&fAngle<fEndAngle+10)
			{
				SelAngle=1;
			}
			else
			{
				SelAngle=-1;
			}
		}
		else
		{
			if (v.x>0)
			{
				fStartAngle=fAngle;
			}
			else
			{
				fEndAngle=fAngle;
			}	
		}
	}	

	IsDragged=true;	
}

void GUISecktor::SetType(int _Type)
{
	switch (_Type)
	{
		case 0:
		{
			Type=secktor_circle;

		}
		break;
		case 1:
		{
			Type=secktor_simetric;
		}
		break;
		case 2:
		{
			Type=secktor_assimetric;
		}
		break;
	}

}

void GUISecktor::SetNeedDrag(bool _Need)
{
	NeedDrag=_Need;
}

void GUISecktor::SetSecktorColor(dword dwColor)
{
	dwSecktorColor=dwColor;
}
