#pragma once

#include "..\..\..\..\common_h\math3d.h"

class TCamera
{
public:

	float Theta;		// Up-down angle
	float Phi;			// Left-right angle

	float xc,yc,zc;		// Camera Position
	float CamSpeed;		// Camera Speed

    int iCurMouseX;		// Saved mouse position
    int iCurMouseY;

    float RotateSens;	// RotateSensitivity

	bool IsLMBPressed;

	float delta_x;
	float delta_y;

	 TCamera();
	~TCamera();

	bool HandleMessages(unsigned int uMsg, int MouseX, int MouseY);

    void ProcessMouseMove(int iMouseX, int iMouseY);

	void MoveMouse(float fDeltaX, float fDeltaY);

    void BuildViewMatrix(Matrix &matView, bool canMove, const Vector *to = null, float k = 0.0f);

    void SetScale(float val);
    void SetTheta(float val);
    void SetPhi	 (float val);

    void SetRotateSensitivity(float val = 0.005f);
	void SetCamSpeed		 (float val = 1.000f);

};
