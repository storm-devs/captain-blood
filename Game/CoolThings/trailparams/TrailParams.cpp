#include "TrailParams.h"

bool TrailParams::Create		 (MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool TrailParams::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void TrailParams::InitParams	 (MOPReader &reader)
{
	updated = Rnd();

	/////////////////////////

	use = reader.Bool();

	radBegin = reader.Float();
	radEnd   = reader.Float();

	fadeMin = reader.Float();
	fadeMax = reader.Float();

	color = reader.Colors();

	offStrength = reader.Float();

	windDir = Matrix(reader.Angles()).vz;
	windVel = reader.Float();

	forceValue = reader.Float();

	staticLive = reader.Bool();

	/////////////////

	ballRot	= reader.Bool();
	ballRotSpeed = reader.Float();
}

MOP_BEGINLISTCG(TrailParams, "Trail params", '1.00', 50, "", "Default")

	MOP_BOOL ("Use trail", false);

	MOP_FLOAT("Beg radius", 0.55f);
	MOP_FLOAT("End radius", 2.50f);

	MOP_FLOATC("Min fade delay", 2.0f, "Минимальное время рассеивания");
	MOP_FLOATC("Max fade delay", 3.0f, "Максимальное время рассеивания");

	MOP_COLOR("Color", Color((dword)-1));

	MOP_FLOATC("Offset strength", 1.0f, "Сила разбрасывания хвоста");

	MOP_GROUPBEG("Wind params")

		MOP_ANGLESC("Dir", Vector(-PI*0.5f,0.0f,0.0f), "Направление ветра")
		MOP_FLOATC ("Vel", 0.0f, "Скорость ветра")

	MOP_GROUPEND()

	MOP_FLOATC("Force value", 0.0f, "Скорость выбрасывания дыма");

	MOP_BOOLC("Enable static live", false, "Генерить дым при остановке");

	///////////////

	MOP_BOOL ("Ball rotation", true);
	MOP_FLOAT("Ball rotation speed", 1.0f)

MOP_ENDLIST(TrailParams)
