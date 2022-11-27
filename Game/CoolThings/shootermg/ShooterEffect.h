#ifndef SHOOTEREFFECT_H
#define SHOOTEREFFECT_H

#include "..\..\..\Common_h\graphic\graphic.h"

class IGMXEntity;

class ShooterEffect
{
	float fMaxLiveTime;

	//Effect1
	const char* pcLightLocator;
	ConstString pcLightObjectName;
	Graphic grphLightIntencity;
	//
	GMXHANDLE lightLocator;
	MOSafePointer lightObject;

	//Effect2
	const char* pcModelName;
	const char* pcModelAni;
	const char* pcNodeName;
	Matrix mtxModelLocalTransform;
	Graphic grphModelTransparence;
	//
	IGMXScene* pEffectModel;
	IAnimation* pEffectModelAnimation;
	bool bEffectModelStarted;

	//Effect3
	ConstString pcWidgetName;
	Graphic grphWidgetTransparence;
	//
	MOSafePointer pWidgetObject;

	float fLiveTime;
	IGMXScene* pShooterGMX;
	IMission* mission;

public:
	ShooterEffect();
	~ShooterEffect();

	void ReadMOPs( MOPReader & reader );
	void Init(IMission * pMission, IGMXScene* pShooterModel);
	void PostCreate();
	void Release();

	void Process( float fDeltaTime );
	void Start();
	void Stop();
};

#define MOP_SHOOTEREFFECT( name ) \
	MOP_FLOAT (name##"Live time", 0.2f) \
	/*Effect1*/ \
	MOP_STRINGC(name##"Light Locator", "Shoot","Name of Locator") \
	MOP_STRINGC(name##"Light Name", "", "Name of Mission Point Light") \
	MOP_GRAPHIC(name##"Light Intecity", "Light Intecity") \
	/*Effect2*/ \
	MOP_STRINGC(name##"Model Name", "", "Name of Model used in Effect") \
	MOP_STRINGC(name##"Model Animation", "", "Name of Animation used in Effect") \
	MOP_STRINGC(name##"Model Node Name", "", "Name of Animation Node that used in Effect") \
	MOP_POSITION(name##"Model Pos", Vector(0.f,0.f,0.f)) \
	MOP_ANGLES(name##"Model Angle", Vector(0.f,0.f,0.f)) \
	MOP_GRAPHIC(name##"Model Tracperancy", "Model Tracperancy") \
	/*Effect3*/ \
	MOP_STRINGC(name##"Widget Name", "", "Name of Widget used in Effect") \
	MOP_GRAPHIC(name##"Widget Trancperancy", "Widget Tracperancy")

#endif
