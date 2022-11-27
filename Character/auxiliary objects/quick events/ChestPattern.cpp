
#include "ChestPattern.h"

bool ChestPattern::EditMode_Update(MOPReader & reader)
{	
	model_name = reader.String().c_str();
	anim_name = reader.String().c_str();

	loc_pos = reader.Position();
	loc_angles = reader.Angles();
	loc_dropPosition = reader.Position();
	float dropAngle = reader.Float()*(PI/180.0f);
	float dropDeltaAngle = reader.Float()*(0.5f*PI/180.0f);
	dropMinAngle = dropAngle - dropDeltaAngle;
	dropMaxAngle = dropAngle + dropDeltaAngle;
	dropMinVy = reader.Float();
	dropMaxVy = reader.Float();
	dropMinVxz = reader.Float();
	dropMaxVxz = reader.Float();
	friction = reader.Float();

	radius_activation = reader.Float();
	player_node = reader.String();
	button_name = reader.String().c_str();

	player_distance = reader.Float();

	colider.cld_loc_pos = reader.Position();
	colider.cld_loc_angels = reader.Angles();
	colider.cld_size.x = reader.Float() * 0.5f;
	colider.cld_size.y = reader.Float() * 0.5f;
	colider.cld_size.z = reader.Float() * 0.5f;

	particle_name = reader.String();	
	particle_loc_pos = reader.Position();
	particle_loc_angels = reader.Angles();

	button_widget_name = reader.String();

	dyn_light = reader.Bool();
	shadow_cast = reader.Bool();
	shadow_recive = reader.Bool();

	BasePattern::EditMode_Update(reader);

	return true;
}

const char * ChestPattern::comment = "ChestPattern";

MOP_BEGINLISTG(ChestPattern, "ChestPattern", '1.00', 100, "Quick Events")

	MOP_STRING("Model", "")
	MOP_STRING("Animation", "")

	MOP_POSITION("Local Position", Vector(0.0f))
	MOP_ANGLES("Local Angels", Vector(0.0f))

	MOP_POSITION("Drop position", Vector(0.0f))
	MOP_FLOATEX("Drop orient", 0.0f, 0.0f, 360.0f)
	MOP_FLOATEX("Drop sector angle", 120.0f, 0.0f, 360.0f)
	MOP_FLOAT("Drop min vertical speed", 5.0f)
	MOP_FLOAT("Drop max vertical speed", 12.0f)
	MOP_FLOAT("Drop min horisontal speed", 5.0f)
	MOP_FLOAT("Drop max horisontal speed", 15.0f)
	MOP_FLOATEX("Drop horisontal friction", 5.0f, 0.0f, 20.0f)

	MOP_FLOAT("Radius", 3.0f)
	MOP_STRING("Player Start Node", "")
	MOP_STRING("Button Name", "ChrA")	

	MOP_FLOATEX("Player distance", 1.5f,0.1f,1000000.0f)	

	MOP_POSITION("Colider locPos", Vector(0.0f))
	MOP_ANGLES("Colider locAng", Vector(0.0f))
	MOP_FLOAT("Colider Width", 1.0f)
	MOP_FLOAT("Colider Height", 1.0f)
	MOP_FLOAT("Colider Lenght", 1.0f)

	MOP_STRING("Particle Name", "")
	MOP_POSITION("Particle locPos", Vector(0.0f))
	MOP_ANGLES("Particle locAng", Vector(0.0f))

	MOP_STRING("Button Tip", "")

	MOP_BOOL("Dynamic Lighting", true)
	MOP_BOOLC("Shadow Cast", true, "")
	MOP_BOOLC("Shadow Reseive", true, "")
	
MOP_ENDLIST(ChestPattern)