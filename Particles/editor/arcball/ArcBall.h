#ifndef DX9_ARCBALL
#define DX9_ARCBALL

#include "..\..\..\common_h\math3d.h"

enum ArcBallMode
{
 ABM_DISABLED = 0,
 ABM_ROTATE,
 ABM_MOVE,
 ABM_SCALE
};


class STORM3_ArcBall
{

 ArcBallMode Mode;
 
 float Theta;  //Up-down angle
 float Phi;    //Left-right angle
 
 Vector LookTo; // LookTo point
 float Scale;        // Distance from look to
 
 int iCurMouseX;      // Saved mouse position
 int iCurMouseY;
 
 
 float RotateSens; // RotateSensitivity
 float ScaleSens; //ScaleSensitivity
 float MoveSens; //MoveSensitivity
 
 
 void ProcessMouseMove (int iMouseX, int iMouseY);
 

public:
 
  STORM3_ArcBall ();
  ~STORM3_ArcBall ();

  bool HandleMessages( unsigned int uMsg, int MouseX, int MouseY);
  
  ArcBallMode GetMode ();
  
  void BuildViewMatrix (Matrix& matView);
  
  void SetScale (float _Scale);
  void SetTheta (float _Theta);
  void SetPhi (float _Phi);
  
  void RotateSensitivity (float value = 0.005f);
  void ScaleSensitivity (float value = 150.0f);
  void MoveSensitivity (float value = 0.0015f);
  
  

};

#endif