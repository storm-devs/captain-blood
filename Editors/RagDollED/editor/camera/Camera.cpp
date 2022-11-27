#include "Camera.h"
#include "..\..\..\..\common_h\controls.h"

#define ROTOLIMIT 1.52f

TCamera:: TCamera()
{
	Theta = -0.500f;
	Phi	  = -0.785f - PI/2;

	iCurMouseX = 0;
	iCurMouseY = 0;

	SetRotateSensitivity();

	xc = -10.0f*cosf(Theta)*cosf(Phi);
	yc = -10.0f*sinf(Theta);
	zc = -10.0f*cosf(Theta)*sinf(Phi);

	IsLMBPressed = false;

	delta_x = 0;
	delta_y = 0;
}

TCamera::~TCamera ()
{
}

bool TCamera::HandleMessages(unsigned int uMsg, int MouseX, int MouseY)
{
	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		{
		//	if(GetAsyncKeyState(VK_MENU))
			{
				IsLMBPressed = true;

				iCurMouseX = MouseX;
				iCurMouseY = MouseY; 

				delta_x = 0;
				delta_y = 0;
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			if( IsLMBPressed )
				ProcessMouseMove(MouseX, MouseY);
		}
		break;

		case WM_LBUTTONUP:
		{
			IsLMBPressed=false;
		}
		break;		
	}

	return false;
}

void TCamera::ProcessMouseMove(int iMouseX, int iMouseY)
{
	float fDeltaX = (float)(iCurMouseX - iMouseX);
	float fDeltaY = (float)(iCurMouseY - iMouseY);

	iCurMouseX = iMouseX;
	iCurMouseY = iMouseY; 
 
	MoveMouse(fDeltaX,fDeltaY);
}

void TCamera::MoveMouse(float fDeltaX, float fDeltaY)
{
	delta_x = fDeltaX = Lerp(delta_x,fDeltaX,0.5f);
	delta_y = fDeltaY = Lerp(delta_y,fDeltaY,0.5f);

	Phi	  += fDeltaX*RotateSens;
	Theta += fDeltaY*RotateSens;

	if( Theta < -ROTOLIMIT )
		Theta = -ROTOLIMIT;

	if( Theta >  ROTOLIMIT )
		Theta =  ROTOLIMIT;
}

void TCamera::BuildViewMatrix(Matrix &matView, bool canMove, const Vector *to, float k)
{    
	float sin_t = sinf(Theta);
	float cos_t = cosf(Theta);

	float sin_p = sinf(Phi);
	float cos_p = cosf(Phi);

	if( canMove )
	{
		float dxc = 0;
		float dyc = 0;
		float dzc = 0;

		if( GetAsyncKeyState('W') < 0 )
		{
			dyc += sin_t;
			dzc += sin_p*cos_t;
			dxc += cos_p*cos_t;
		}

		if( GetAsyncKeyState('S') < 0 )
		{
			dyc -= sin_t;
			dzc -= sin_p*cos_t;
			dxc -= cos_p*cos_t;
		}

		if( GetAsyncKeyState('A') < 0 )
		{
			dzc += cos_p;
			dxc -= sin_p;
		}

		if( GetAsyncKeyState('D') < 0 )
		{
			dzc -= cos_p;
			dxc += sin_p;
		}

		float dt = api->GetDeltaTime();

		if( GetAsyncKeyState(VK_CONTROL) < 0 )
			dt *= 9.0f;
		if( GetAsyncKeyState(VK_SHIFT)	 < 0 )
			dt *= 3.0f;

		xc += dxc*dt*3;
		yc += dyc*dt*3;
		zc += dzc*dt*3;
	}

    Vector lookFrom = Vector(xc,yc,zc);

    Vector lookTo   = Vector(
		lookFrom.x + cos_p*cos_t,
		lookFrom.y + sin_t,
		lookFrom.z + sin_p*cos_t);

	Vector vecUp(0.0f,1.0f,0.0f);

	if( to )
	{
		lookTo.Lerp(lookTo,*to,k);
	}

	matView.BuildView(lookFrom,lookTo,vecUp);

	if( to )
	{
	/*	Matrix inv(matView);
		inv.Inverse();

		Theta = asinf(inv.vz.y);

		Phi = asinf(inv.vx.z);

		if( inv.vx.x < 0 )
		{
			if( Phi > 0 )
				Phi =  PI - Phi;
			else
				Phi = -PI - Phi;
		}
		
		Phi += PI/2;*/

		Theta = asinf(matView.vy.z);

		Phi = asinf(matView.vz.x);

		if( matView.vx.x < 0 )
		{
			if( Phi > 0 )
				Phi =  PI - Phi;
			else
				Phi = -PI - Phi;
		}
		
		Phi += PI/2;
	}
}

void TCamera::SetTheta(float val)
{
    Theta = val;
}

void TCamera::SetPhi(float val)
{
    Phi = val;
}

void TCamera::SetRotateSensitivity(float val)
{
    RotateSens = val;
}
