#ifndef _XBOX

#include "ArcBall.h"
#include "..\..\..\common_h\controls.h"

#define ROTOLIMIT 1.52f
 
STORM3_ArcBall::STORM3_ArcBall ()
{
 Mode = ABM_DISABLED;
 Theta = 0.5f;
 Phi = 0.785f;
 LookTo = Vector(0.0f, 0.0f, 0.0f); 
 Scale = 20.0f;
 
 iCurMouseX = 0;
 iCurMouseY = 0;

 RotateSensitivity ();
 ScaleSensitivity ();
 MoveSensitivity ();
}

STORM3_ArcBall::~STORM3_ArcBall ()
{
}

bool STORM3_ArcBall::HandleMessages( unsigned int uMsg, int MouseX, int MouseY )
{
 // Current screen mouse position
 int iMouseX = MouseX;
 int iMouseY = MouseY;

 if (!GetAsyncKeyState (VK_MENU) && uMsg == WM_LBUTTONDOWN) return false;


 switch (uMsg)
 {
  case WM_LBUTTONDOWN:
    Mode = ABM_ROTATE;
    iCurMouseX = iMouseX;
    iCurMouseY = iMouseY;
    return false;

  case WM_RBUTTONDOWN:
    Mode = ABM_SCALE;
    iCurMouseX = iMouseX;
    iCurMouseY = iMouseY;
    return false;

  case WM_MBUTTONDOWN:
    Mode = ABM_MOVE;
    iCurMouseX = iMouseX;
    iCurMouseY = iMouseY;
    return false;

  case WM_LBUTTONUP:
    Mode = ABM_DISABLED;
    return false;

  case WM_RBUTTONUP:
    Mode = ABM_DISABLED;
    return false;

  case WM_MBUTTONUP:
    Mode = ABM_DISABLED;
    return false;
    
  case WM_MOUSEMOVE:
  {
    if (Mode != ABM_DISABLED)
    {
     ProcessMouseMove (iMouseX, iMouseY);
     iCurMouseX = iMouseX;
     iCurMouseY = iMouseY;
     return true;
    }
    return false;
  }    
 }
 return false;
}
  
void STORM3_ArcBall::ProcessMouseMove (int iMouseX, int iMouseY)
{
 if (Mode == ABM_DISABLED) return;
 
 float fDeltaX = (float)( iCurMouseX-iMouseX );
 float fDeltaY = (float)( iCurMouseY-iMouseY );

 if (Mode == ABM_ROTATE)
 { 
  Phi -= (fDeltaX * RotateSens);
  Theta -= (fDeltaY * RotateSens);
  if (Theta < -ROTOLIMIT) Theta = -ROTOLIMIT;
  if (Theta > ROTOLIMIT) Theta = ROTOLIMIT;
 }

 if (Mode == ABM_MOVE)
 {
  float StrafeSpeed = fabs (fDeltaX) * 0.0015f * Scale;
  float ElevationSpeed = fabs (fDeltaY) * 0.0015f * Scale;
  Vector Strafe(0.0f, 0.0f, 0.0f);
  Strafe.x = cos(Phi)*StrafeSpeed;
  Strafe.z = -sin(Phi)*StrafeSpeed;
  if (fDeltaX > 0) Strafe = -Strafe;
  if (fDeltaY > 0) ElevationSpeed = -ElevationSpeed;
  Strafe.y = ElevationSpeed;
  LookTo += Strafe;
 }
 
 if (Mode == ABM_SCALE)
 {
   float ScaleFactor = fDeltaY + fDeltaX;
   ScaleFactor /= ScaleSens;
   float Sign = ScaleFactor;
   ScaleFactor = 1.0f + fabs(ScaleFactor);
   if (Sign < 0) ScaleFactor = 1.0f / ScaleFactor;
   Scale *= ScaleFactor;  
 }
}
  
  
ArcBallMode STORM3_ArcBall::GetMode ()
{
 return Mode;
}

void STORM3_ArcBall::BuildViewMatrix (Matrix& matView)
{
  float deltaX = sin(Phi) * Scale;
  float deltaZ = cos(Phi) * Scale;
  
  float deltaY = sin(Theta) * Scale;

  float NormalK = cos (Theta);
  deltaX *= NormalK; 
  deltaZ *= NormalK;

  Vector LookFrom = LookTo;
  LookFrom += Vector (deltaX, deltaY, deltaZ);
  
  Vector vecUp(0, 1, 0);
  
	matView.BuildView(LookFrom, LookTo, vecUp);
}

void STORM3_ArcBall::SetScale (float _Scale)
{
 Scale = _Scale;
}

void STORM3_ArcBall::SetTheta (float _Theta)
{
 Theta = _Theta;
}

void STORM3_ArcBall::SetPhi (float _Phi)
{
 Phi = _Phi;
}

void STORM3_ArcBall::RotateSensitivity (float value)
{
 RotateSens = value;
}

void STORM3_ArcBall::ScaleSensitivity (float value)
{
 ScaleSens = value;
}

void STORM3_ArcBall::MoveSensitivity (float value)
{
 MoveSens = value;
}


#endif