#include "HairParams.h"

bool HairParams::Create			(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool HairParams::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void HairParams::InitParams(MOPReader &reader)
{
	hair.lines.DelAll();
	coll.list .DelAll();

//	m_k = reader.Float();
//	m_m = reader.Float();

	int ln = reader.Array();

	for( int i = 0 ; i < ln ; i++ )
	{
		HairLine &line = hair.lines[hair.lines.Add()];

		line.k = reader.Float();
		line.m = reader.Float();

		int bn = reader.Array();

		line.bones.Reserve(line.bones.Size() + bn);
		for( int j = 0 ; j < bn ; j++ )
			line.bones.Add(reader.String());

		line.n = reader.Float();
	}

	int cn = reader.Array();

	for( int i = 0 ; i < cn ; i++ )
	{
		CollData &data = coll.list[coll.list.Add()];

		data.a = reader.String();
		data.b = reader.String();

		data.x = reader.Float();
		data.r = reader.Float();

		data.p = reader.Position();

		data.c = reader.Colors();
	}

	updated = true;
}

const char *HairParams::comment =
"Character hair parameters\n\n"
"В разделе Lines указываются цепочки костей для отдельных волос\n"
"(необходимо указать как минимум 3 начальные кости)\n\n"
"    Lines\n"
"        Bones\n"
"            Name - имя кости\n\n"
"В разделе Colls указываются сферы для расчета коллизии волос с телом\n"
"(центр сферы задается опорной костью и смещением относительно ее,\n"
"либо двумя костями, с указанием относительного расстояния между ними)\n\n"
"    Colls\n"
"        Bone A - первая опорная кость\n"
"        Bone B - вторая опорная кость\n"
"        Blend  - относительное расположение между костями\n"
"        Rad    - радиус сферы\n"
"        Offset - смещение относительно кости\n"
"        Color  - цвет сферы (для отрисовки в редакторе)";

MOP_BEGINLISTCG(HairParams, "Hair params", '1.00', 10, HairParams::comment, "Character")

//	MOP_FLOATC("k", 100.0f, "Коэффициент жесткости волос (пока лучше не менять)");
//	MOP_FLOATC("m",   1.0f, "Коэффициент массы волос (пока лучше не менять)");

	MOP_ARRAYBEG("Lines", 0, 100)

		MOP_FLOATC("k", 100.0f, "Коэффициент жесткости волос (пока лучше не менять)");
		MOP_FLOATC("m",	  1.0f, "Коэффициент массы волос (пока лучше не менять)");

		MOP_ARRAYBEG("Bones", 3, 100)

			MOP_STRINGC("Name", "", "Имя кости");

		MOP_ARRAYEND

		MOP_FLOATEXC("n",0.0f,0.0f,1.0f,"Коэффициент сохранения оригинальной формы волоса");

	MOP_ARRAYEND

	MOP_ARRAYBEG("Colls", 0, 10)

		MOP_STRINGC("Bone A", "", "Первая опорная кость для сферы коллизии");
		MOP_STRINGC("Bone B", "", "Вторая опорная кость для сферы коллизии");

		MOP_FLOATC("Blend", 0.0f, "Расположение сферы коллизии между опорными костями (0.0 - в первой, 1.0 - во второй)");
		MOP_FLOATC("Rad"  , 0.0f, "Радиус сферы коллизии");

		MOP_POSITIONC("Offset", 0.0f, "Смещение сферы относительно опорной кости");

		MOP_COLORC("Color", 0xffff0000, "Цвет сферы коллизии (для отрисовки в редакторе)");

	MOP_ARRAYEND

MOP_ENDLIST(HairParams)
