//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*  description: 
//*
//****************************************************************

#include "..\..\..\..\Common_h\InputSrvCmds.h"

#include "gui_viewport.h"
#include "..\Camera\Camera.h"
#include "..\..\gizmo\gizmo.h"
#include "..\EditorMain.h"



GUIViewPort::GUIViewPort (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	pFont = NEW GUIFont ("SansSerif");
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;

	t_OnRender = NEW GUIEventHandler;

	IControlsService *service = (IControlsService *)api->GetService("ControlsService");

	controls = service->CreateInstance(_FL_);

	move_x = controls->FindControlByName("GUICursor_MoveX");
	move_y = controls->FindControlByName("GUICursor_MoveY");

	m_look = false;

	show_cursor = 0;
}

GUIViewPort::~GUIViewPort ()
{
	DELETE(t_OnRender)
	DELETE(pFont)

	RELEASE(controls)

	GUIControl::~GUIControl();
}

void GUIViewPort::Draw ()
{
	if (Visible == false) return;
	GUIControl::Draw ();

	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	
	GUIRectangle rect = GetDrawRect ();
	this->ClientToScreen (rect);

//	GUIHelper::DrawDownBorder(rect.x, rect.y , Width, Height);

	dword shad = 0x20000000;

	GUIHelper::DrawHorizLine(rect.x + 3,rect.w - 5 + 1,rect.x + rect.h - 3,shad);
	GUIHelper::DrawVertLine(rect.y + 3,rect.h - 5,rect.x + rect.w - 1,shad);

	cliper.Push();
	GUIRectangle r;
	r = rect;
	r.x += 2;
	r.y += 2;
	r.w = Width - 4;
	r.h = Height - 4;
	cliper.SetRectangle(r);
	t_OnRender->Execute(this);
	cliper.Pop();

	if( m_look )
	{
		float x = controls->GetControlStateFloat(move_x);
		float y = controls->GetControlStateFloat(move_y);

		pEditor->ArcBall()->MoveMouse(
			-x*0.08f,
			-y*0.08f);
	}

	if( show_cursor > 0 )
	{
		if( !--show_cursor )
			GetMainControl()->Application->ShowCursor(true);
	}
}


bool GUIViewPort::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{	
	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();
	GUIPoint LocalMousePos = MousePos;

	ScreenToClient(LocalMousePos);

	switch( message )
	{
		case GUIMSG_LMB_DOWN:
		{
			//pEditor->ArcBall()->HandleMessages(WM_LBUTTONDOWN, MousePos.x, MousePos.y);

			pEditor->GetGizmo()->HandleMessages(WM_LBUTTONDOWN, LocalMousePos.x, LocalMousePos.y);
		}
		break;

		case GUIMSG_RMB_DOWN:
		{			
			pEditor->ArcBall()->HandleMessages(WM_LBUTTONDOWN, MousePos.x, MousePos.y);		

			pEditor->GetGizmo()->HandleMessages(WM_RBUTTONDOWN, LocalMousePos.x, LocalMousePos.y);

			GetMainControl()->Application->ShowCursor(false);

			controls->ExecuteCommand(InputSrvLockMouse(true));

			m_look = true;

			show_cursor = 0;

			pEditor->GetGizmo()->HighlightAxis(-1000.0f,-1000.0f);
		}
		break;

		case GUIMSG_RMB_UP:
		{
			controls->ExecuteCommand(InputSrvLockMouse(false));

		//	GetMainControl()->Application->ShowCursor(true);
			show_cursor = 2;

			m_look = false;

			pEditor->GetGizmo()->HighlightAxis(
				(float)MousePos.x,
				(float)MousePos.y);
		}
		break;

		case GUIMSG_MMB_DOWN:
		{			
			pEditor->GetGizmo()->HandleMessages(WM_MBUTTONDOWN, LocalMousePos.x, LocalMousePos.y);
		}
		break;

		case GUIMSG_LMB_UP:
		{			
			pEditor->bDragRagDollBone=false;
		}
		break;
	}

	return GUIControl::ProcessMessages(message,lparam,hparam);
}

void GUIViewPort::MouseUp (int button, const GUIPoint& pt)
{	
	//if (pEditor->ArcBall()->GetMode() != ABM_DISABLED)
	{
		//GetMainControl()->Application->GetCursor()->Pop();
	}
	
	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();

	pEditor->ArcBall()->HandleMessages(WM_LBUTTONUP, MousePos.x, MousePos.y);

	GUIPoint LocalMousePos = MousePos;
	ScreenToClient(LocalMousePos);

	pEditor->GetGizmo()->HandleMessages(WM_LBUTTONUP, LocalMousePos.x, LocalMousePos.y);
	pEditor->GetGizmo()->HandleMessages(WM_RBUTTONUP, LocalMousePos.x, LocalMousePos.y);
	pEditor->GetGizmo()->HandleMessages(WM_MBUTTONUP, LocalMousePos.x, LocalMousePos.y);

	//if (button==5) pEditor->bDragRagDollBone=false;
	pEditor->bDragRagDollBone=false;
}

void GUIViewPort::MouseMove (int button, const GUIPoint& pt)
{
	if( m_look )
		return;

	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();

		
	pEditor->ArcBall()->HandleMessages(WM_MOUSEMOVE, MousePos.x, MousePos.y);

	GUIPoint LocalMousePos = MousePos;
	ScreenToClient(LocalMousePos);

	if (LocalMousePos.x<0||LocalMousePos.x>DrawRect.Width) return;
	if (LocalMousePos.y<0||LocalMousePos.y>DrawRect.Height) return;

	pEditor->GetGizmo()->HandleMessages (WM_MOUSEMOVE, LocalMousePos.x, LocalMousePos.y);		
}

void GUIViewPort::MouseDown (int button, const GUIPoint& pt)
{	
	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();

	GUIPoint LocalMousePos = MousePos;
	ScreenToClient(LocalMousePos);

	if (LocalMousePos.x<0||LocalMousePos.x>DrawRect.Width) return;
	if (LocalMousePos.y<0||LocalMousePos.y>DrawRect.Height) return;
	

	if (button==5)
	{				
		if (pEditor->bSimulating)
		{	
			if (pEditor->BoneID!=0)
			{
				IPhysEditableRagdoll::IBone* pRagDollBone=pEditor->pPhysRagdoll->GetBone(pEditor->BoneID);

				if (pRagDollBone!=NULL)
				{			
					pRagDollBone->ApplyForce(Vector(0,10,0),pEditor->BoneLocalPoint);

					pEditor->bDragRagDollBone=true;
				}
			}
		}		
		else
		if (pEditor->bEditEnv&&!pEditor->GetGizmo()->bHighlighted&&!pEditor->bAttachBoneMode)
		{
			if (pEditor->PreSelEnvObject!=-1)
			{
				pEditor->SelEnvObject=pEditor->PreSelEnvObject;

				pEditor->GetGizmo()->SetTransform(pEditor->EnvObjects[pEditor->SelEnvObject].mTransform);

				pEditor->GetGizmo()->Enable(true);

				pEditor->SelBone=NULL;
			}		
			else
			if (pEditor->PreSelBone!=NULL&&!pEditor->GetGizmo()->bHighlighted&&!pEditor->bSimulating)
			{
				pEditor->SelEnvObject=-1;

				pEditor->SelBone=pEditor->PreSelBone;

				pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mWorldGlobal);

				pEditor->GetGizmo()->CalcGizmoScale();

				pEditor->UpdateScrolls();

				pEditor->GetGizmo()->Enable(true);
			}
		}
		else
		{
			if (pEditor->SelBone!=NULL&&pEditor->ModelSelBone)
			{
				Matrix mat=pEditor->BoneMatrix;

				if (pEditor->SelBone->parent!=NULL)
				{
					Matrix inv_mat=pEditor->SelBone->parent->mGlobal;										
					
					inv_mat.Inverse();					

					mat=mat*inv_mat;

					pEditor->SelBone->mLocal=mat;
				}
				else
				{
					pEditor->SelBone->mLocal=mat;
				}				

				if (pEditor->SelBone->parent!=NULL)
				{
					pEditor->SelBone->mGlobal = pEditor->SelBone->mLocal * pEditor->SelBone->parent->mGlobal;		
				}
				else
				{
					pEditor->SelBone->mGlobal=pEditor->SelBone->mLocal;		
				}

				pEditor->SelBone->AssignedBoneName=pEditor->BoneName;

				///////////////

				if(!pEditor->bAttachBoneMode )
					pEditor->SetLocalMatrixAsWorld(&pEditor->RootBone);

				pEditor->CalcLocalChildMatrix(pEditor->SelBone);

				///////////////

				pEditor->GetGizmo()->SetTransform(pEditor->SelBone->mGlobal);

				pEditor->GetGizmo()->CalcGizmoScale();

				pEditor->UpdateScrolls();
			}
		}
	}
/*	else
	if (button==6&&pEditor->fLenght>0)
	{
		if (pEditor->SelBone!=NULL&&pEditor->ModelSelBone)
		{
			pEditor->SelBone->fHeight=pEditor->fLenght;

			pEditor->UpdateScrolls();
		}
	}*/
}
