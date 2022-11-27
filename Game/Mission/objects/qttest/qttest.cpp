
#include "QTTest.h"


const GroupId QTTest::group = GroupId('q', 't', 'e', 's');
const Vector QTTest::minRange = -200.0f;
const Vector QTTest::maxRange = 200.0f;


QTTest::QTTest() : objects(_FL_, 256)
{
	minAbb = Vector(-15.0f, 0.0f, -15.0f);
	maxAbb = Vector(15.0f, 0.0f, 15.0f);
	isSelfTest = false;
	isTreeDraw = false;
	levelScale = 2.0f;
	qtGroupName.id = 0;
}

QTTest::~QTTest()
{

}

//Инициализировать объект
bool QTTest::Create(MOPReader & reader)
{
	EditMode_Update(reader);
	SetUpdate(&QTTest::Work, ML_DYNAMIC4);
	return true;
}

//Обработчик команд для объекта
void QTTest::Command(const char * id, dword numParams, const char ** params)
{
	isSelfTest = false;
	isTreeDraw = false;
	char c[4];
	c[0] = c[1] = c[2] = c[3] = ' ';
	for(dword i = 0; i < 4; i++)
	{
		if(!id[i]) break;
		c[i] = id[i];
	}
	qtGroupName = GroupId(c[0], c[1], c[2], c[3]);
	SetUpdate(&QTTest::Work, ML_DYNAMIC4);
}

//Инициализировать объект
bool QTTest::EditMode_Create(MOPReader & reader)
{
	EditMode_Update(reader);
	SetUpdate(&QTTest::Work, ML_DYNAMIC4);
	//Удалим то что есть
	for(long i = 0; i < objects; i++)
	{
		objects[i].qt->Release();
	}
	objects.Empty();
	//Генерим локальные объекты
	for(long i = 0; i < 100; i++)
	{
		Obj obj;
		//Ставим позицию
		obj.ang = Vector().Rand(Vector(0.0f), Vector(2.0f*PI));		
		obj.pos = Vector().Rand(Vector(minRange), Vector(maxRange));
		obj.pos.y = 0.0f;
		if(rand() & 0x40)
		{
			obj.vp = 0.0f;
			obj.va = 0.0f;
			obj.color = 0xffffffff;
		}else{
			obj.va = Vector().Rand(Vector(-3.0f), Vector(3.0f));
			obj.vp.Rand(Vector(0.1f), 30.0f);
			obj.color = 0xffffff00;
		}
		//Создаём объект, ставим размеры и начальную позицию
		obj.qt = QTCreateObject(group, _FL_);
		obj.qt->SetBoxCenter(Vector().Rand(Vector(-15.0f), Vector(15.0f)));
		obj.qt->SetBoxSize(Vector().Rand(Vector(0.3f), Vector(20.0f)));
		obj.qt->SetMatrix(Matrix(obj.ang, obj.pos));
		//Имитируем разные ситуации
		if(rand() & 15)
		{
			if((rand() & 63) == 5)
			{
				obj.pos = 10e+15f;
				obj.vp = 0.0f;
			}
			obj.qt->Activate(true);			
		}
		//Добавляем в список
		objects.Add(obj);
	}
	return true;
}

//Обновить параметры
bool QTTest::EditMode_Update(MOPReader & reader)
{
	isSelfTest = reader.Bool();
	isTreeDraw = reader.Bool();
	const char * id = reader.String().c_str();
	char c[4];
	c[0] = c[1] = c[2] = c[3] = ' ';
	for(dword i = 0; i < 4; i++)
	{
		if(!id[i]) break;
		c[i] = id[i];
	}
	qtGroupName = GroupId(c[0], c[1], c[2], c[3]);
	levelScale = reader.Float();
	return true;
}

//Нарисовать модельку
void _cdecl QTTest::Work(float dltTime, long level)
{
	if(!isSelfTest)
	{
		if(qtGroupName.id != 0)
		{
			QTDraw(qtGroupName, levelScale);
		}
		return;
	}
	//Двигаем объекты
	for(long i = 0; i < objects; i++)
	{
		Obj & obj = objects[i];
		obj.ang += obj.va*dltTime;
		obj.pos += obj.vp*dltTime;
		if(obj.pos.x < minRange && obj.vp.x < 0.0f) obj.vp.x = -obj.vp.x;
		if(obj.pos.x > maxRange && obj.vp.x > 0.0f) obj.vp.x = -obj.vp.x;
		if(obj.pos.z < minRange && obj.vp.z < 0.0f) obj.vp.z = -obj.vp.z;
		if(obj.pos.z > maxRange && obj.vp.z > 0.0f) obj.vp.z = -obj.vp.z;
		obj.pos.y = 0.0f;
		obj.qt->SetMatrix(Matrix(obj.ang, obj.pos));
	}
	//Отрисовка
	if(isTreeDraw)
	{
		QTDraw(group, levelScale);
		return;
	}
	for(long i = 0; i < objects; i++)
	{
		Obj & obj = objects[i];
		//Рисуем мировой ящик объекта
		Vector vmin = obj.qt->GetBoxCenter() - obj.qt->GetBoxSize()*0.5f;
		Vector vmax = obj.qt->GetBoxCenter() + obj.qt->GetBoxSize()*0.5f;
		Render().DrawBox(vmin, vmax, obj.qt->GetMatrix(), obj.color);
		//Рисуем квадрат на дереве
		if(obj.qt->GetQTAbb(vmin, vmax))
		{
			Render().DrawBox(vmin, vmax, Matrix(), obj.color & 0xff7f7f7f);
		}else{
			Render().DrawBox(vmin*1.1f, vmax*1.1f, obj.qt->GetMatrix(), 0xffff0000);
		}
	}
	//Ставим точки области поиска
	if(api->DebugKeyState('1'))
	{
		minAbb = Render().GetView().GetCamPos();
		minAbb.y = 0.0f;
	}
	if(api->DebugKeyState('2'))
	{
		maxAbb = Render().GetView().GetCamPos();
		maxAbb.y = 0.0f;
	}
	Render().DrawBox(minAbb, maxAbb, Matrix(), 0xff00ff00);
	//Найдём тех кто попадает в область
	dword count = QTFindObjects(group, minAbb, maxAbb);
	for(dword i = 0; i < count; i++)
	{
		IMissionQTObject * qobj = QTGetObject(i);
		Vector vmin = qobj->GetBoxCenter() - qobj->GetBoxSize()*0.5f;
		Vector vmax = qobj->GetBoxCenter() + qobj->GetBoxSize()*0.5f;
		Render().DrawBox(vmin*1.1f, vmax*1.1f, qobj->GetMatrix(), 0xff00ff00);
	}
}

#ifndef STOP_DEBUG

MOP_BEGINLISTC(QTTest, "Quad tree test", '1.00', 0, "1,2 test frame range, for view QT in mission\nsend command with name like group name for draw\nwithout parameters")
	MOP_BOOL("Self test", false)
	MOP_BOOL("Self tree", false)
	MOP_STRINGC("Group id for QT draw", "", "Group name, when QT need draw (off self test before)")
	MOP_FLOAT("Levels step", 2.0f)
MOP_ENDLIST(QTTest)

#endif





