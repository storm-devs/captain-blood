#include "DirectionLight.h"



//Конструктор
DirectionLight::DirectionLight()// : Diffuse(_FL_) 
//																	 ,Specular (_FL_)
{
	//pLS = (PPLService*)api->GetService("PPLService");
}

//Деструктор
DirectionLight::~DirectionLight()
{
}


//Создание объекта
bool DirectionLight::Create(MOPReader & reader)
{
	EditMode_Update (reader);
	SetUpdate((MOF_UPDATE)&DirectionLight::Realize, ML_GUI1);
	return true;
}


//Обновление параметров
bool DirectionLight::EditMode_Update(MOPReader & reader)
{

	//vEditorPos = reader.Position();

	//cAmbient = reader.Colors();

	//bShowCubeMap = reader.Bool();


	long DiffuseCount = reader.Array();
//	pLS->SetDiffuseLightsCount(DiffuseCount);

//	Diffuse.DelAll();
//	Diffuse.Reserve(DiffuseCount);
	Assert(DiffuseCount == 1);



	for (long n = 0; n < DiffuseCount; n++)
	{
		PPLLight & l = Diffuse[0];
		l.vPos = reader.Position(); 
		Vector angle = reader.Angles();
		Vector Dir = -(Matrix(angle).vz);
		l.vDir = Dir;
		l.cColor = reader.Colors();
		l.cBackColor = reader.Colors();
		float power = reader.Float();
		float powerBack = reader.Float();
		l.cColor = l.cColor * power;
		l.cBackColor = l.cBackColor * powerBack;
		l.fSize = reader.Float();

		l.bSpecularAttachedToCamera = reader.Bool();
		//Diffuse.Add(l);
	}


/*
	long SpecularCount = reader.Array();
//	pLS->SetSpecularLightsCount(SpecularCount);

	Specular.DelAll();
	Specular.Reserve(SpecularCount);


	for (long n = 0; n < SpecularCount; n++)
	{
		PPLLight l;
		l.vPos = reader.Position();
		Vector angle = reader.Angles();
		Vector Dir = -(Matrix(angle).vz);
		l.vDir = Dir;
		l.cColor = reader.Colors();
		l.fSize = reader.Float();
		Specular.Add(l);
	}


	bSetupVertex = reader.Bool();
	bSetupVertex = true;
*/
	bSetupVertex = true;


	Activate (reader.Bool());

	return true;
}


void _cdecl DirectionLight::Realize(float fDeltaTime, long level)
{
	if (!EditMode_IsOn()) return;

/*
	if (pLS->GetDiffuseLightsCount() > 0)
	{
		Color v =	pLS->GetDiffuseLightColor(0);
		v.a  = 1.0f;
		Render().DrawSphere(vEditorPos, 0.3f, v);
	}
*/

	for (dword n = 0; n < /*Diffuse.Size()*/1; n++)
	{
		Vector vEnd = (Diffuse[n].vPos - Diffuse[n].vDir*6.0f);
		dword dwColor = Diffuse[n].cColor.GetDword();
		Render().DrawSphere(Diffuse[n].vPos, 0.3f, dwColor);
		Render().Print(vEnd, 100000000.0f, 1.0f, dwColor, "Diffuse %d", n);
		Render().Print(vEnd, 100000000.0f, 2.0f, dwColor, "Size %3.2f", Diffuse[n].fSize);
		Render().DrawVector(Diffuse[n].vPos, vEnd, dwColor);
	}


/*	for ( n = 0; n < Specular.Size(); n++)
	{
		Vector vEnd = (Specular[n].vPos-Specular[n].vDir*4.0f);
		dword dwColor = Specular[n].cColor.GetDword();
		Render().DrawSphere(Specular[n].vPos, 0.3f, dwColor);
		Render().Print(vEnd, 100000000.0f, 1.0f, dwColor, "Specular %d", n);
		Render().Print(vEnd, 100000000.0f, 2.0f, dwColor, "Size %3.2f", Specular[n].fSize);
		Render().DrawVector(Specular[n].vPos, vEnd, dwColor);
	}*/


	//if (EditMode_IsOn())
	//{
	//if (bShowCubeMap && pLS) pLS->DrawCubeMap();
	//}

}




void DirectionLight::Activate(bool isActive)
{ 
	MissionObject::Activate(isActive);

	if (IsActive())
	{
		LogicDebug("Activate, direction light is set");
		Console().Trace(COL_ALL, "Direction Light '%s' is set", GetObjectID().c_str());
//		ActivateEvent(null, null);
		Registry(ACTIVATE_EVENT_GROUP, &DirectionLight::ActivateEvent, 0);		
		SetUpdate(&DirectionLight::SetLight, ML_LIGHTS_ON);


		/*
		if (pLS)
		{
			pLS->SetAmbientColor(cAmbient);
		  pLS->SetDiffuseLightsCount(Diffuse.Size());
			
			for (dword i = 0; i < Diffuse.Size(); i++)
			{
				pLS->SetDiffuseColor(i, Diffuse[i].cColor);
				pLS->SetDiffuseLightDir(i, Diffuse[i].vDir);
				pLS->SetDiffuseSize(i, Diffuse[i].fSize);
			}

			pLS->SetSpecularLightsCount(Specular.Size());
			for (dword i = 0; i < Specular.Size(); i++)
			{
				pLS->SetSpecularColor(i, Specular[i].cColor);
				pLS->SetSpecularLightDir(i, Specular[i].vDir);
				pLS->SetSpecularSize(i, Specular[i].fSize);
			}


		}
		*/
		

	}else{
		LogicDebug("Deactivate, direction light skip it");
		Unregistry(ACTIVATE_EVENT_GROUP);
		DelUpdate(&DirectionLight::SetLight);
		/*
		if (b && pLS)
		{
			pLS->SetAmbientColor(Color(0x0L));
			pLS->SetSpecularLightsCount(0);
			pLS->SetDiffuseLightsCount(0);
			Console().Trace(COL_ALL, "Dir Light '%s' disabled", GetObjectID().c_str());
		} else
		{
			Console().Trace(COL_ALL, "Dir Light '%s' already disabled", GetObjectID().c_str());
		}
		*/
	}
}

//Установка источника в основном цикле
void _cdecl DirectionLight::SetLight(float dltTime, long level)
{
	ActivateEvent(null, null);
}


//Выполнить событие активации
void _cdecl DirectionLight::ActivateEvent(const char * group, MissionObject * sender)
{
	Render().SetGlobalLight(Diffuse[0].vDir, Diffuse[0].bSpecularAttachedToCamera, Diffuse[0].cColor, Diffuse[0].cBackColor);
}


MOP_BEGINLISTG(DirectionLight, "DirectionLight", '1.00', 100, "Lighting")

	//MOP_COLOR ("Ambient", Color(0xFF808080L));
	//MOP_BOOL ("Edit_mode_show_cubemap", false)

	MOP_ARRAYBEG("Diffuse", 1, 1)
		MOP_POSITION("EditorPos", Vector(0.0f))
		MOP_ANGLES("Angle", Vector(0.0f))
		MOP_COLOR ("Color", Color(0xFFFFFFFFL));
		MOP_COLOR ("BackColor", Color(0xFFFFFFFFL));
		MOP_FLOAT ("Multipler", 1.0f);
		MOP_FLOAT ("Back Multipler", 1.0f);
		MOP_FLOATEX ("Size", 1.0f, 0.0f, 1.0f);
		MOP_BOOL ("Attach Specular Direction to camera", false)
	MOP_ARRAYEND

/*
	MOP_ARRAYBEG("Specular", 0, 500)
		MOP_POSITION("EditorPos", Vector(0.0f))
		MOP_ANGLES("Angle", Vector(0.0f))
		MOP_COLOR ("Color", Color(0xFFFFFFFFL));
		MOP_FLOATEX ("Size", 0.707f, 0.0f, 1.0f);
	MOP_ARRAYEND
*/
	
	//MOP_BOOL ("SetupVertexLight", true)
	MOP_BOOL ("Active", true)
MOP_ENDLIST(DirectionLight)






