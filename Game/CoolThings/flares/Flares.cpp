
#include "Flares.h"
#include "FlaresManager.h"



Flares::Flares(): flares(_FL_, 2)
{		
}

Flares::~Flares()
{		
	manager->Activate(this, false);
	manager = null;
}


//Инициализировать объект
bool Flares::EditMode_Create(MOPReader & reader)
{
	Create(reader);

	return true;
}

//Обновить параметры
bool Flares::EditMode_Update(MOPReader & reader)
{
	return EditMode_Create(reader);
}

//Инициализировать объект
bool Flares::Create(MOPReader & reader)
{	
	SetManager();
				
	isFlicker = reader.Bool();
	
	for(long i = 0; i < ARRSIZE(osc); i++)
	{
		osc[i].step = reader.Float();
				
		osc[i].amp = reader.Float();

		if (osc[i].amp > 0.0f)
		{
			osc[i].kamp = osc[i].amp;
		}
		else
		{
			osc[i].amp = 0.0f;
		}
	}		
		
	flicker = reader.Float();

	range = reader.Float();
	size = reader.Float();
	autosize = reader.Bool();
	size_angle = reader.Float();
			
	color = reader.Colors();
	color.Clamp();		

	zOffset = -reader.Float();

	fade_speed = 1.0f / reader.Float();

	int flares_count = reader.Array();

	flares.DelAll();
	flares.AddElements(flares_count);

	for (int i=0; i<flares_count; i++)
	{
		Flare & flare = flares[i];//flares.Add(Flare())];

		flare.pos = reader.Position();
			
		flare.isFlicker = isFlicker;
	
		for(long i = 0; i < ARRSIZE(flare.osc); i++)
		{
			flare.osc[i].step = osc[i].step;
				
			flare.osc[i].amp = osc[i].amp;

			if (flare.osc[i].amp > 0.0f)
			{
				flare.osc[i].kamp = flare.osc[i].amp;
			}
			else
			{
				flare.osc[i].amp = 0.0f;
			}

			flare.osc[i].k = 0.0f;
			
			flare.osc[i].oldv = RRnd(-flare.osc[i].amp, flare.osc[i].amp);
			flare.osc[i].newv = RRnd(-flare.osc[i].amp, flare.osc[i].amp);
		}
		
		flare.flicker = flicker;
		flare.range = range;		
			
		flare.color = color;		
	
		flare.intenc = 1.0f;
		flare.intencVis = 0.0f;

		flare.u = flare.v = -1.0f;


		//----------------------------------------------------
		flare.connectedObjectName = reader.String();
		flare.locatorName = reader.String().c_str();
		flare.alreadyTryToFind = false;
		flare.connectedLocator.reset();
		flare.scene = null;
		flare.connectedObject.Reset();
		
/*		
		FindObject(flare.connectedObjectName, flare.connectedObject);

		flare.connectedLocator = NULL;
		if (flare.connectedObject.Validate())
		{
			MissionObject * mobject = flare.connectedObject.Ptr();
			if (mobject->Is("GeometryObject"))
			{
				GeometryObject* geom = (GeometryObject*)mobject;
				IGMXScene * scene = geom->GetScene();
				if (scene)
				{
					flare.connectedLocator = scene->FindEntity(GMXET_LOCATOR, flare.locatorName);
				}
			} 
		}
*/
		
		
	}
	
	
	
	noSwing = reader.Bool();

	showDebug = reader.Bool();

	Activate(reader.Bool());

	return true;
}

//Активировать/деактивировать объект
void Flares::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	manager->Activate(this,isActive);	
}


void Flares::SetManager()
{
	MOSafePointer obj;
	static const ConstString objectId("FlaresManager");
	Mission().CreateObject(obj,"FlaresManager", objectId);
	manager = (FlaresManager*)obj.Ptr();
	Assert(manager);
}




MOP_BEGINLISTCG(Flares, "Flares", '1.00', 900, "Flares", "Lighting")
		
	MOP_BOOL("Enable flicker", false)
	
	MOP_FLOATEX("Freq1", 0.4f, 0.0f, 100.0f)
	MOP_FLOATEX("Amp1", 0.3f, 0.0f, 1.0f)
	
	MOP_FLOATEX("Freq2", 10.0f, 0.0f, 100.0f)
	MOP_FLOATEX("Amp2", 0.08f, 0.0f, 1.0f)

	MOP_FLOATEX("Flicker", 0.4f, 0.0f, 1.0f)
	
	MOP_FLOATEX("Range", 25.0f, 0.0f, 1000.0f)
	MOP_FLOATEX("Size", 0.2f, 0.01f, 1000.0f)
	MOP_BOOL("AutoSize like sun",false)
	MOP_FLOATEX("Size Angle", 30.0f, 0.01f, 80.0f)
	
	MOP_COLOREX("Сolor", Color(1.0f), Color(0L), Color(0xffffffffL))

	MOP_FLOATEX("zOffset", 0.0f, 0.00f, 10.0f)

	MOP_FLOATEX("Fade time", 0.35f, 0.01f, 10.0f)

	MOP_ARRAYBEGC("Flares", 0, 100, "Flares")
		
		MOP_POSITION("Position", Vector(0.0f))	

		MOP_STRING("Mission object to connect", "")	

		MOP_STRING("Locaor in mission geometry to connect", "")	

	MOP_ARRAYEND

	MOP_BOOLC("No swing", true, "No swing ocean in swing machine")

	MOP_BOOL("Debug", false);

	MOP_BOOL("Active", true)

MOP_ENDLIST(Flares)





