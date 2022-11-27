
#include "TaskPointer.h"

//Вершины для рендера картинок
Vector4 TaskPointer::vertices[4];

TaskPointer::TaskPointer()
{
	back = null;
	mask = null;
	texVar = null;
	scene = null;

	modelPos = fov = w = h = 0.0f;

	vertices[0] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	vertices[1] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	vertices[2] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[3] = Vector4(1.0f, 1.0f, 0.0f, 1.0f);

	observer.Reset();
	target  .Reset();

	showPosition = 0.0f;
}

TaskPointer::~TaskPointer()
{
	if(texVar)
	{
		texVar->SetTexture(null);
		texVar = null;
	}
	RELEASE(back);
	RELEASE(mask);
	RELEASE(scene);
}

//Инициализировать объект
bool TaskPointer::Create(MOPReader & reader)
{
	Render().GetShaderId("Interface_TaskpointerPicture", Interface_TaskpointerPicture_id);
	Render().GetShaderId("Interface_TaskpointerPictureMask", Interface_TaskpointerPictureMask_id);

	
	

	if(!texVar)
	{
		texVar = Render().GetTechniqueGlobalVariable("TaskPointer_Texture", _FL_);
	}
	RELEASE(back);
	RELEASE(mask);
	RELEASE(scene);

	observer.Reset();
	target  .Reset();

	showPosition = 1.0f;
	current.SetIdentity();
	string s = reader.String().c_str();
	if(!s.IsEmpty())
	{
	//	back = Render().CreateTexture(_FL_, s);
		back = Render().CreateTextureFullQuality(_FL_, s);
	}
	s = reader.String().c_str();
	if(!s.IsEmpty())
	{
	//	mask = Render().CreateTexture(_FL_, s);
		mask = Render().CreateTextureFullQuality(_FL_, s);
	}
	s = reader.String().c_str();
	if(!s.IsEmpty())
	{
		scene = Geometry().CreateGMX(s.GetBuffer(), &Animation(), &Particles(), &Sound());
		if(scene)
		{
			scene->SetDynamicLightState(false);
		}
	}
	w = reader.Float();
	h = reader.Float();
	fov = reader.Float()*PI/180.0f;
	camera.Build(reader.Angles());
	modelPos = reader.Position();
	modelRot = Matrix(reader.Angles());	
	modelRot.pos = reader.Position();
	float scale = reader.Float();
	modelRot.Scale(scale);
	observerId = reader.String().c_str();
	targetId = reader.String().c_str();
	showPosition = 0.0f;
	drawPriority = reader.Long();
	Show(reader.Bool());
	Activate(reader.Bool());
	return true;
}

//Инициализировать объект
bool TaskPointer::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обновить параметры
bool TaskPointer::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обработчик команд для объекта
void TaskPointer::Command(const char * id, dword numParams, const char ** params)
{
	if( numParams < 1 )
		return;

	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"Observer"))
	{
		observerId = params[0];

		observer.Reset();
	}
	else
	if( string::IsEqual(id,"Target"))
	{
		targetId = params[0];

		target.Reset();
	}		
}

//Показать/скрыть объект
void TaskPointer::Show(bool isShow)
{
	if(EditMode_IsOn())
	{
		if(EditMode_IsSelect())
		{
			isShow = true;
		}
	}
	MissionObject::Show(isShow);
	if(isShow)
	{
		LogicDebug("Show");
		Activate(IsActive());
	}else{
		showPosition = 0.0f;
		LogicDebug("Hide");
		DelUpdate(&TaskPointer::Draw);
	}
}

//Активировать
void TaskPointer::Activate(bool isActive)
{
	if(EditMode_IsOn())
	{
		if(EditMode_IsSelect())
		{
			isActive = true;
		}
	}
	bool isChange = isActive != IsActive();
	MissionObject::Activate(isActive);
	if(!IsShow())
	{
		LogicDebug("Save active state, but not applyed, becose element not visible");
		return;
	}
	if(isActive)
	{
		if(back || scene || mask)
		{
			SetUpdate(&TaskPointer::Draw, ML_GUI4 + drawPriority);
		}
	}
}

#ifndef MIS_STOP_EDIT_FUNCS
//Выделить объект
void TaskPointer::EditMode_Select(bool isSelect)
{
	MissionObject::EditMode_Select(isSelect);
	Show(IsShow());
	Activate(IsActive());
}
#endif

//Нарисовать модельку
void _cdecl TaskPointer::Draw(float dltTime, long level)
{	
	if(InterfaceUtils::IsHide()) return;
	//Анализируем смещение
	const float moveSpeed = 2.0f;
	if(IsActive())
	{
		showPosition += dltTime*moveSpeed;
		if(showPosition > 1.0f)
		{
			showPosition = 1.0f;
		}
	}else{
		showPosition -= dltTime*moveSpeed;
		if(showPosition <= 0.0f)
		{
			showPosition = 0.0f;
			if(!EditMode_IsOn())
			{
				DelUpdate(&TaskPointer::Draw);
			}
			return;
		}
	}
	if(EditMode_IsOn())
	{
		if(!EditMode_IsSelect()) return;
		showPosition = 1.0f;
	}
	//Сохраняем текущие параметры
	RENDERVIEWPORT savedVP = Render().GetViewport();
	Matrix savedView = Render().GetView();
	Matrix savedPrj = Render().GetProjection();
	//Новый вьюпорт
//	dword width = Render().GetScreenInfo().dwWidth;
//	dword height = Render().GetScreenInfo().dwHeight;
	const RENDERVIEWPORT &nat = Render().GetViewport();
	dword width  = nat.Width;
	dword height = nat.Height;
	RENDERVIEWPORT vp;
	vp.Width = dword(w*width);
	vp.Height = dword(h*height*InterfaceUtils::AspectRatio(Render()));
	vp.X = nat.X + width  - vp.Width  - 1 - InterfaceUtils::HideFieldSize(Render());
	vp.Y = nat.Y + height - vp.Height - 1 - InterfaceUtils::HideFieldSize(Render());
	if(vp.Width < 16) vp.Width = 16;
	if(vp.Width > width) vp.Width = width;
	if(vp.Height < 16) vp.Height = 16;
	if(vp.Height > height) vp.Height = height;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	Render().SetViewport(vp);
	//Матрица проекции
	Matrix prj;
	prj.BuildProjection(fov, (float)vp.Width, (float)vp.Height, 0.01f, 10.0f);
	prj.vz.x = modelPos.x - (showPosition - 1.0f)*2.0f;
	prj.vz.y = modelPos.y;
	Render().SetProjection(prj);
	//Матрица камеры
	Matrix view;
	view.BuildView(Vector(0.0f, 0.0f, -modelPos.z), Vector(0.0f), Vector(0.0f, 1.0f, 0.0f));
	Render().SetView(view);
	//Рисуем элементы
	if(back)
	{
		vertices[0].z = showPosition;
		vertices[1].z = showPosition;
		vertices[2].z = showPosition;
		vertices[3].z = showPosition;
		texVar->SetTexture(back);
		Render().DrawPrimitiveUP(Interface_TaskpointerPicture_id, PT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
	}
	if(scene)
	{
		Matrix orient;
		if(!EditMode_IsOn())
		{
			if(UpdatePointer(observerId, observer))
			{
				if(UpdatePointer(targetId, target))
				{
					Vector from = observer.Ptr()->GetMatrix(Matrix()).pos;
					Vector to = target.Ptr()->GetMatrix(Matrix()).pos;
					Vector dir = (savedView.MulNormal(to - from)).GetXZ();
					if(dir.NormalizeXZ() > 1e-10f)
					{
						orient.vy = Vector(0.0f, 1.0f, 0.0f);
						orient.vz = dir;
						orient.vx = orient.vy ^ orient.vz;
						float kBlend = Clampf(dltTime*10.0f);
						current.SLerp(current, Quaternion(orient), kBlend);
					}
				}
			}			
		}else{
			orient.Build(0.0f, dltTime, 0.0f);
			current = current*Quaternion(orient);
		}
		current.GetMatrix(orient);
		RENDERRECT rect;		
		rect.x1 = vp.X;
		rect.y1 = vp.Y;
		rect.x2 = vp.X + vp.Width - 1;
		rect.y2 = vp.Y + vp.Height - 1;
		Render().Clear(1, &rect, CLEAR_ZBUFFER, 0, 1.0f, 0);
		if(mask)
		{
			vertices[0].z = showPosition;
			vertices[1].z = showPosition;
			vertices[2].z = showPosition;
			vertices[3].z = showPosition;
			texVar->SetTexture(mask);
			Render().DrawPrimitiveUP(Interface_TaskpointerPictureMask_id, PT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
		}

		//Geometry().SetRenderMode("GMX_SingleTextureWOSh");
		
		scene->SetTransform(modelRot*orient*camera);
		scene->Draw();

		//Geometry().SetRenderMode();
	}
	//Востанавливаем параметры
	Render().SetViewport(savedVP);
	Render().SetView(savedView);
	Render().SetProjection(savedPrj);
}

//Обновить указатель на объект
bool TaskPointer::UpdatePointer(const string &str, MOSafePointer &ptr)
{
	if( ptr.Validate())
	{
		return true;
	}
	else
	{
		return FindObject(ConstString(str),ptr);
	}
}

const char * TaskPointer::comment = 
"UI element for show task place direction.\n"
"Commands:\n"
"   Observer [new observer id]\n"
"   Target [new target id]\n"
" ";

MOP_BEGINLISTCG(TaskPointer, "Task pointer", '1.00', 100000000, TaskPointer::comment, "Interface")
	MOP_STRING("Background", "")
	MOP_STRING("Model mask", "")
	MOP_STRING("Model", "")	
	MOP_FLOATEX("Width", 0.125f, 0.0f,1.0f)
	MOP_FLOATEX("Height", 0.125f, 0.0f,1.0f)
	MOP_FLOATEX("FOV", 30.0f, 0.0f, 140.0f)
	MOP_ANGLES("Camera angle", 0.0f)
	MOP_POSITIONEX("Model position", Vector(0.0f, 0.0f, 1.0f), Vector(-2.0f, -2.0f, 0.0f), Vector(2.0f, 2.0f, 1000.0f))
	MOP_ANGLES("Model angle", 0.0f)
	MOP_POSITION("Model offset", 0.0f)
	MOP_FLOATEX("Model scale", 1.0f, 0.0001f, 10000.0f)
	MOP_STRING("Observer", "Player")
	MOP_STRING("Target", "")
	MOP_LONG("Draw priority", 0)
	MOP_BOOLC("Show", true, "Show or hide instantly")
	MOP_BOOLC("Active", true, "Show or hide with animation")
MOP_ENDLIST(TaskPointer)

