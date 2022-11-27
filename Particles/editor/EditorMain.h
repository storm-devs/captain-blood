#ifndef _PARTICLE_EDITOR_H_
#define _PARTICLE_EDITOR_H_

#include "..\..\common_h\render.h"
#include "..\..\common_h\Controls.h"
#include "..\..\common_h\gui.h"
#include "..\..\common_h\particles.h"
#include "arcball\arcball.h"

#define PARTICLES_PLACE "resource\\particles"

class ParticleManager;
class ParticleEditor;
class TMainWindow;

extern ParticleEditor* pEditor;

class ParticleSystem;
class TransformGizmo;

class ParticleEditor : public RegObject
{
	string StartDirectory;
	STORM3_ArcBall ST3ArcBall;

	DWORD BackgroundColor;
	DWORD GridColor;
	DWORD GridZeroLinesColor;
	IParticleService* pPS;
	ParticleManager* pParticleManager;
  IFileService* pFS;
	IRender* pRS;
	IControls* pCtrl;

	TransformGizmo* Gizmo;
	
public:

	TMainWindow* FormMain;

	
	virtual ~ParticleEditor();
	ParticleEditor();
	bool    Init();
	void    Realize(dword Delta_Time);
	void    Execute(dword Delta_Time);


	IRender* Render();
	IParticleManager* Manager ();

	DWORD GetBackgroundColor ();
	void SetBackgroundColor (DWORD Color);

	DWORD GetGridColor ();
	void SetGridColor (DWORD Color);

	DWORD GetGridZeroColor ();
	void SetGridZeroColor (DWORD Color);


	STORM3_ArcBall* ArcBall ();

	const char* GetStartDir ();


	void _cdecl OnCloseManager (GUIControl* sender);


	IParticleSystem* GetSystem ();

	IFileService* Files ();

	TransformGizmo* GetGizmo ();

};

#endif