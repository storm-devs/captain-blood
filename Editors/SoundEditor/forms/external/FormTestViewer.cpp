#include "FormTestViewer.h"
#include "FormTestGizmo.h"

#include "..\..\..\..\Common_h\InputSrvCmds.h"
#include "..\Lists\FormPolygonList.h"

static const float delay_total = 9.5f;
static const float delay_fader = 1.0f;

FormTestViewer::FormTestViewer(GUIControl *parent, const GUIRectangle &rect, FormPolygonList &list)
	: FormControlBase(parent,rect),m_list(list)
{
	m_lookFixed = true;

	m_view.BuildView(Vector(0.0f,0.0f,-20.0f),Vector(0.0f),Vector(0.0f,1.0f,0.0f));

	if( m_lookFixed )
	{
		Vector p = m_view.GetCamPos();

		m_flat.BuildView(p + Vector(0.0f,1000.0f,0.0f),p,Vector(0.0f,0.0f,1.0f));
	}
	else
	{
		Vector p = m_view.GetCamPos();

		Vector z = m_view.vz; z.x = -z.x;

		m_flat.BuildView(p + Vector(0.0f,1000.0f,0.0f),p,z);
	}

	m_right = false;
	m_look	= false;

	ISoundService *p = (ISoundService *)api->GetService("SoundService");
	Assert(p)

	m_scene = p->CreateScene(SoundSceneName, _FL_);

	if( m_scene )
		m_scene->SetListenerMatrix(Matrix(m_view).Inverse());

	tabDown = false;

	m_scale = 0.3f;
	m_time	= 0.0f;

	m_delay = delay_total;

	loading = false;

	m_rot_x = 0.0f;
	m_rot_y = 0.0f;

	m_dec = false;
	m_inc = false;

	options->controls->EnableControlGroup("SoundCamera");

	////

	m_map.r.w = 200;
	m_map.r.h = 200;

	m_map.r.x = 0;
	m_map.r.y = 0;

	////

	dragMap = false;

	m_gizmo = NEW TransformGizmo(TransformGizmo::GT_MOVE,TransformGizmo::GM_LOCAL,Matrix());

	m_overGizmo = false;

	dragGizmo = false;

	dragObject = null; dragObjectIndex = -1;

	m_gizmo->SetTransform(Matrix());
	m_gizmo->CalcGizmoScale();

	m_filePath = options->pathProjectPolygonOptions;

	//////////////////////////

	m_file = project->FileOpenBin(m_filePath, SndProject::poff_noflags, null, _FL_, 109);
	if(!m_file)
	{
		m_file = project->FileOpenBin(m_filePath, SndProject::poff_isCreateNewFile, null, _FL_, 109);
	}
	Assert(m_file)

	if( m_file->Size())
	{
		LoadHeader();
	}

	if( m_list.GetElementsCount() < 1 )
	{
		AddSound("fire_wood_major"     ,Vector(   0.0f,0.0f,   0.0f),0xffff0000,0.5f, 15.0f, 75.0f);
		AddSound("ambient_river_medium",Vector(-300.0f,0.0f,-100.0f),0xffff0000,0.8f,100.0f,350.0f);

		AddModel("taran.gmx",Vector(-150.0,0.0f,-50.0f),0.0f);
	}

	if( dragObjectIndex < 0 && m_list.GetElementsCount() > 0 )
	{
		dragObjectIndex = 0;
	}

	if( dragObject )
	{
		Matrix m; m.pos = dragObject->pos + dragObject->off;

		m_gizmo->SetTransform(m);
		m_gizmo->CalcGizmoScale();
	}

	////

	m_map.inc.r.w = 16;
	m_map.inc.r.h = 16;

	m_map.inc.r.x = m_map.r.x + m_map.r.w - m_map.inc.r.w;
	m_map.inc.r.y = m_map.r.y + m_map.r.h - m_map.inc.r.h;

	m_map.dec.r.w = m_map.inc.r.w;
	m_map.dec.r.h = m_map.inc.r.h;

	m_map.dec.r.x = m_map.inc.r.x - m_map.dec.r.w;
	m_map.dec.r.y = m_map.inc.r.y;

	////

	SaveData();

	////

	m_list.InitList();

	if( dragObject )
		m_list.SetFocusIndex(dragObjectIndex);

	m_list.onCreateSound.SetHandler(this,(CONTROL_EVENT)&FormTestViewer::OnCreateSound);
	m_list.onCreateModel.SetHandler(this,(CONTROL_EVENT)&FormTestViewer::OnCreateModel);

	m_list.onDeleteItems.SetHandler(this,(CONTROL_EVENT)&FormTestViewer::OnDeleteItems);
}

void FormTestViewer::LoadHeader()
{
	FileHeader info;

	m_file->SetPos(0); m_file->Read(&info,sizeof(info));

	//// Map

	m_lookFixed = info.map.fixed;

	m_scale = info.map.scale;

	m_map.r.x = info.map.x;
	m_map.r.y = info.map.y;

	//// Camera

	m_view.BuildView(info.cam.pos,info.cam.pos + Vector(0.0f,0.0f,1.0f),Vector(0.0f,1.0f,0.0f));

	m_rot_x = info.cam.ang.x;
	m_rot_y = info.cam.ang.y;

	//// Data

	LoadData(info.itemsCount);

	dragObjectIndex = info.gizmoIndex;

	if( dragObjectIndex >= info.itemsCount )
		dragObjectIndex  = -1;

	if( dragObjectIndex >= 0 )
	{
		dragObject = (Entity *)m_list.GetElementPtr(dragObjectIndex);
	}
}

void FormTestViewer::SaveHeader()
{
	FileHeader info;

	memset(&info,0xcc,sizeof(info));

	//// Map

	info.map.fixed = m_lookFixed;

	info.map.scale = m_scale;

	info.map.x = m_map.r.x;
	info.map.y = m_map.r.y;

	//// Camera

	info.cam.pos = m_view.GetCamPos();

	info.cam.ang.x = m_rot_x;
	info.cam.ang.y = m_rot_y;

	info.cam.ang.z = 0.0f;

	//// Data

	info.itemsCount = m_list.GetElementsCount();

	info.gizmoIndex = dragObjectIndex;

	////

	m_file->SetPos(0); m_file->Write(&info,sizeof(info));
}

FormTestViewer::~FormTestViewer()
{
	////

	project->FileCloseBin(m_file, 109);

	////

	RELEASE(m_scene)

	DELETE(m_gizmo)
}

void FormTestViewer::Cleanup()
{
	dword count = m_list.GetElementsCount();

	for( dword i = 0 ; i < count ; i++ )
	{
		Entity *p = (Entity *)m_list.GetElementPtr(i);

		delete p;
	}
}

void FormTestViewer::AddSound(const char *name, const Vector &pos, dword color, float vol, float r1, float r2)
{
	Source &s = *NEW Source;

	dragObject = null;
	dragObjectIndex = -1;

	s.pos = pos; s.color = color;

/*	s.vol = vol;

	s.r1 = r1;
	s.r2 = r2;*/

	s.vol = 0.0f;

	s.r1 = 0.0f;
	s.r2 = 0.0f;

	s.sound = null;

	if( m_scene )
	{
		s.sound = m_scene->Create3D(name,s.pos,_FL_,false,false);
	}

	if( s.sound )
	{
		const array<ProjectSound *> &sounds = project->SoundArray();

		for( int i = 0 ; i < sounds ; i++ )
		{
			ProjectSound *p = sounds[i];

			if( string::IsEqual(name,p->GetName().str))
			{
				const SoundBaseParams * par = p->GetCurrentBaseParams();
				const SoundAttGraph * att = p->GetCurrentAttenuation();

				if( par )
				{
					s.vol = par->volume;
				}

				if( att )
				{
					s.r1 = att->minDist;
					s.r2 = att->maxDist;
				}

				break;
			}
		}
	}

	if( s.sound )
		s.sound->Play();

	s.name = name;

	s.off = 0.0f;

	m_list.Add(s.name,FormPolygonElement::et_sound,&s);

	UpdateBox();

	//////////////////////

	if( loading == false )
		SaveData();
}

void FormTestViewer::AddModel(const char *name, const Vector &pos, const Vector &ang)
{
	Geometry &g = *NEW Geometry;

	dragObject = null;
	dragObjectIndex = -1;

	g.pos = pos;
	g.ang = ang;

	g.scene = null;

	IGMXService *service = (IGMXService *)api->GetService("GMXService");

	if( service )
	{
		g.scene = service->CreateScene(name,null,null,null,_FL_);
	}

	g.name = name;

	Vector p; float r; g.GetBoundSphere(p,r);

	g.off = p/* - g.pos*/;

	if( !g.scene )
		 g.off -= g.pos;

	string capt;
		   capt.GetFileTitle(name);

	m_list.Add(capt,FormPolygonElement::et_geometry,&g);

	UpdateBox();

	//////////////////////

	if( loading == false )
		SaveData();
}

void FormTestViewer::NextObject()
{
	int count = (int)m_list.GetElementsCount();

	if( dragObject )
	{
		dragObjectIndex++;

		if( dragObjectIndex >= count )
			dragObjectIndex  = 0;

		dragObject = (Entity *)m_list.GetElementPtr(dragObjectIndex);

		SaveHeader();

		UpdateFocus();
	}
	else
	{
		if( count > 0 )
		{
			dragObjectIndex = 0;

			dragObject = (Entity *)m_list.GetElementPtr(dragObjectIndex);

			SaveHeader();

			UpdateFocus();
		}
	}
}

void FormTestViewer::UpdateFocus()
{
	/////////////////////////////

	if( dragObject )
	{
		Matrix m; m.pos = dragObject->pos + dragObject->off;

		m_gizmo->SetTransform(m);
		m_gizmo->CalcGizmoScale();
	}

	/////////////////////////////

	m_list.SetFocusIndex(dragObjectIndex);
}

void FormTestViewer::UpdateBox()
{
	m_min = 0.0f;
	m_max = 0.0f;

	if( dword count = m_list.GetElementsCount())
	{
		const Entity *p = (Entity *)m_list.GetElementPtr(0);

		p->GetBoundBox(m_min,m_max);

		Vector a,b;

		for( dword i = 1 ; i < count ; i++ )
		{
			p = (Entity *)m_list.GetElementPtr(i);

			p->GetBoundBox(a,b);

			m_min.Min(a);
			m_max.Max(b);
		}
	}
}

void FormTestViewer::DrawScene()
{
	if( dword count = m_list.GetElementsCount())
	{
		options->render->DrawBox(m_min,m_max);

		for( dword i = 0 ; i < count ; i++ )
		{
			const Entity *p = (Entity *)m_list.GetElementPtr(i);

			p->Draw(m_time);
		}
	}

//	if( m_scene )
//		m_scene->DebugDraw();
}

//Рисование
void FormTestViewer::OnDraw(const GUIRectangle &rect)
{
	int listIndex = m_list.GetFocusIndex();

	if( dragObjectIndex != listIndex )
	{
		if( listIndex < 0 )
		{
			dragObject = null;
			dragObjectIndex = -1;
		}
		else
		{
			dragObjectIndex = listIndex;

			dragObject = (Entity *)m_list.GetElementPtr(dragObjectIndex);

			dragObject->Click();
		}

		SaveHeader();

		UpdateFocus();
	}

	////

	GUIHelper::Draw2DRect(rect.x,rect.y,rect.w,rect.h,options->black2Bkg[6]);

	////

	Matrix m_view(this->m_view);

	if( m_look )
	{
		float x = options->controls->GetControlStateFloat("GUICursor_MoveX");
		float y = options->controls->GetControlStateFloat("GUICursor_MoveY");

		m_rot_x -= y*0.0003f;
		m_rot_y -= x*0.0003f;
	}

	Vector vx;
	Vector vz;

	{
		float x = options->controls->GetControlStateFloat("SoundCamera_Turn_H");
		float y = options->controls->GetControlStateFloat("SoundCamera_Turn_V");

		m_rot_x += y*0.0003f;
		m_rot_y -= x*0.0003f;

		if( m_rot_x >  PI*0.49f )
			m_rot_x =  PI*0.49f;
		else
		if( m_rot_x < -PI*0.49f )
			m_rot_x = -PI*0.49f;

		Matrix m(true);

		m.BuildRotateY(m_rot_y);

		m_view *= m;

		vx = m_view.vx;
		vz = m_view.vz;

		m.BuildRotateX(m_rot_x);

		m_view *= m;
	}

	float dt = api->GetDeltaTime();

	m_time += dt*5.0f;

	if( m_time > 2.0f*PI )
		m_time = fmodf(m_time,2.0f*PI);

	if( m_dec )
		m_scale /= 1.0f + dt;
	else
	if( m_inc )
		m_scale *= 1.0f + dt;

	dt = dt*10.0f;

	if( GetAsyncKeyState(VK_CONTROL) < 0 )
		dt *= 15.0f;
	if( GetAsyncKeyState(VK_SHIFT)	 < 0 )
		dt *=  5.0f;

	if( m_look )
	{
		if( GetAsyncKeyState('A') < 0 ) m_view.pos.x -= dt;
		if( GetAsyncKeyState('D') < 0 ) m_view.pos.x += dt;
		if( GetAsyncKeyState('W') < 0 ) m_view.pos.z += dt;
		if( GetAsyncKeyState('S') < 0 ) m_view.pos.z -= dt;
	}

/*	if( GetAsyncKeyState(VK_TAB) < 0 )
	{
		if( tabDown == false )
			NextObject();

		tabDown = true;
	}
	else
	{
		tabDown = false;
	}*/

	m_view.pos.x += dt*options->controls->GetControlStateFloat("SoundCamera_Move_H");
	m_view.pos.z += dt*options->controls->GetControlStateFloat("SoundCamera_Move_V");

	if( m_lookFixed )
	{
		Vector p = m_view.GetCamPos();

		m_flat.BuildView(p + Vector(0.0f,1000.0f,0.0f),p,Vector(0.0f,0.0f,1.0f));
	}
	else
	{
		Vector p = m_view.GetCamPos();

		Vector z = vz; z.x = -z.x;

		m_flat.BuildView(p + Vector(0.0f,1000.0f,0.0f),p,z);
	}

	//// Scene

	RENDERVIEWPORT pp = options->render->GetViewport();
	RENDERVIEWPORT vp;

	vp.X = rect.x; vp.Width  = rect.w;
	vp.Y = rect.y; vp.Height = rect.h;

	vp.MinZ = 0.1f;
	vp.MaxZ = 1.0f;

	options->render->SetViewport(vp);

	options->render->SetView(m_view);

	Matrix pj; pj.BuildProjection(1.0f,(float)rect.w,(float)rect.h,0.1f,4000.0f);

	options->render->SetProjection(pj);

	DrawScene();

	///////////////////////////////////

	GUIPoint pt = GetMainControl()->Application->GetCursor()->GetPosition();

	if( dragObject )
	{
		if( !dragGizmo && !m_look )
		{
			m_gizmo->SetView(m_view);
			m_gizmo->SetProjection(pj);
			m_gizmo->SetViewPort(vp);

			m_overGizmo = m_gizmo->MouseMove(pt.x - rect.x,pt.y - rect.y);

			m_mouseX = pt.x - rect.x;
			m_mouseY = pt.y - rect.y;
		}

		m_gizmo->Draw();
	}

	///////////////////////////////////

	options->render->SetViewport(pp);
	options->render->SetView(m_flat);

	////

	currObject = null;
	currObjectIndex = -1;

	DrawElem cur(dragObject,m_view,pj,rect);

	cur.index = dragObjectIndex;

	if( cur.e && cur.in )
		cur.Draw();

	GUIPoint qq(pt.x - rect.x,pt.y - rect.y);

	if( m_map.r.Inside(qq) == false )
	{
		if( cur.in && cur.rt.Inside(pt))
		{
			currObject		= dragObject;
			currObjectIndex = dragObjectIndex;
		}
		else
			cur.e = null;

		dword count = m_list.GetElementsCount();

		for( dword i = 0 ; i < count ; i++ )
		{
			const Entity *e = (Entity *)m_list.GetElementPtr(i);

			DrawElem elem(e,m_view,pj,rect);

		/*	if( elem.in )
			{
				GUIHelper::Draw2DRect(elem.rt.x + elem.rt.w,elem.rt.y,10,-10,0xffffffff);
			}*/

			if( e == dragObject )
				continue;

			if( m_look == false )
			{
				elem.index = i;

				if( elem.in )
				{
					if( elem.rt.Inside(pt))
					{
						if( !cur.e || cur.z > elem.z )
						{
							 cur = elem;
						}
					}
				}
			}
		}

		if( m_look == false )
		{
			if( cur.e && cur.in && cur.e != dragObject )
			{
				cur.Draw();

				currObject = (Entity *)cur.e;
				currObjectIndex = cur.index;
			}
		}
	}

	////

	GUIHelper::Draw2DRect(rect.x + m_map.r.x,rect.y + m_map.r.y,m_map.r.w,m_map.r.h,options->black2Bkg[7]);

	//// Map

	Matrix proj = options->render->GetProjection();

	Matrix m(true); m.BuildOrtoProjection(2.0f,2.0f,0.1f,4000.0f);

	m.m[0][0] *= 0.2f*m_scale;
	m.m[1][1] *= 0.2f*m_scale;

	options->render->SetProjection(m);

	vp.X = rect.x + m_map.r.x; vp.Width  = m_map.r.w;
	vp.Y = rect.y + m_map.r.y; vp.Height = m_map.r.h;

	vp.MinZ = 0.1f;
	vp.MaxZ = 1.0f;

	options->render->SetViewport(vp);

	DrawScene();

	float sk = 1.0f/m_scale;

	if( m_lookFixed )
	{
		Vector pos = m_view.GetCamPos();

		pos.y += 900.0f;

		Vector v[3] = {pos,pos,pos};

		Vector x = vx; x.x = -x.x;
		Vector z = vz; z.x = -z.x;

		x *= sk*0.25f;
		z *= sk*0.35f;

		v[0] += z;

		v[1] -= x;
		v[1] -= z;

		v[2] += x;
		v[2] -= z;

		options->render->DrawPolygon(v,3,options->black2Bkg[1]);
	}
	else
	{
		Vector pos = m_view.GetCamPos();

		pos.z +=   5.0f*sk;
		pos.y += 900.0f;

		Vector v[3] = {pos,pos,pos};

		v[1].x += 0.40f*sk;
		v[1].z -= 0.35f*sk;

		v[2].x -= 0.40f*sk;
		v[2].z -= 0.35f*sk;

		options->render->DrawPolygon(v,3,options->black2Bkg[1]);
	}

	options->render->SetProjection(proj);

	options->render->SetViewport(pp);

	//// Buttons

	dword fcol = options->black2Bkg[1];

	int x = rect.x + m_map.dec.r.x; int w = m_map.dec.r.w;
	int y = rect.y + m_map.dec.r.y; int h = m_map.dec.r.h;

	GUIHelper::Draw2DRect(x    ,y,w,h,m_dec ? options->black2Bkg[7] : options->bkg);
	GUIHelper::Draw2DRect(x + w,y,w,h,m_inc ? options->black2Bkg[7] : options->bkg);

	GUIHelper::DrawWireRect(x,y,w + w,h,fcol);
	GUIHelper::DrawVertLine(y,h,x + w,fcol);

	GUIHelper::DrawHorizLine(x     + 5,7,y + 8,fcol);
	GUIHelper::DrawHorizLine(x + w + 5,7,y + 8,fcol);

	GUIHelper::DrawVertLine (y + 5,7,x + w + 8,fcol);

	////

	GUIHelper::DrawWireRect(rect.x + m_map.r.x,rect.y + m_map.r.y,m_map.r.w,m_map.r.h,options->black2Bkg[1]);

	if( m_lookFixed )
	{
	}
	else
	{
		int x = rect.x + m_map.r.x + m_map.r.w/2;
		int y = rect.y + m_map.r.y + m_map.r.h/2;

		GUIHelper::DrawHorizLine(x - 3,7,y,options->black2Bkg[1]);
		GUIHelper::DrawVertLine (y - 3,7,x,options->black2Bkg[1]);
	}

	if( m_delay > 0.0f )
	{
		Color back(options->bkg);
		Color text(options->colorTextLo);

		if( m_delay < delay_fader )
		{
			float k = m_delay/delay_fader;

			back.a *= k;
			text.a *= k;
		}

		int x = rect.x + m_map.r.w + 1;
		int y = rect.y;

		GUIHelper::Draw2DRect(x,y,rect.w - m_map.r.w - 1,100,back);

		x += 5;
		y += 5;

		options->render->Print((float)x,(float)y,text,
			"При нажатой клавише мыши Вы находитесь в режиме перемещения по");

		y += 20;

		options->render->Print((float)x,(float)y,text,
			"сцене. Используйте мышь для поворота камеры и клавиши WASD для");

		y += 20;

		options->render->Print((float)x,(float)y,text,
			"перемещения. Клавиши Shift и Ctrl - ускоренное перемещение.");

		y += 28;

		options->render->Print((float)x,(float)y,text,
			"В обычном режиме мышь используется для выбора объектов.");

		m_delay -= api->GetDeltaTime();
	}

	GUIHelper::DrawWireRect(rect.x,rect.y,rect.w,rect.h,options->black2Bkg[1]);

	////

	if( m_scene )
		m_scene->SetListenerMatrix(Matrix(m_view).Inverse());

	////

	this->m_view.pos += this->m_view.MulNormalByInverse(m_view.GetCamPos() - this->m_view.GetCamPos());

	////

	if( m_right && GetAsyncKeyState(VK_RBUTTON) >= 0 )
	{
		OnMouseRightUp();
	}
}

//Начало перемещения, возвранить true, если начинать тащить. elementPivot графический центр элемента
bool FormTestViewer::OnDragBegin(const GUIPoint &mousePos, GUIPoint &elementPivot)
{
	if( m_map.r.Inside(mousePos))
	{
		if( m_map.dec.r.Inside(mousePos))
		{
			m_dec = true;

			return true;
		}
		if( m_map.inc.r.Inside(mousePos))
		{
			m_inc = true;

			return true;
		}

		dragMap = true;

		elementPivot = m_map.r.pos;

		return true;
	}
	else
	if( m_overGizmo )
	{
		dragGizmo = true;

		elementPivot = 0;

		m_gizmo->MousePressedPoint = Vector((float)m_mouseX,(float)m_mouseY,0.0f);

		m_startX = m_mouseX;
		m_startY = m_mouseY;

		return true;
	}
	else
	if( currObject )
	{
		if( currObject != dragObject )
		{
			dragObject		= currObject;
			dragObjectIndex = currObjectIndex;

			SaveHeader();

			UpdateFocus();
		}

		dragObject->Click();

		return false;
	}
	else
	{
		if( m_right == false )
		{
			m_look = true;

			GetMainControl()->Application->ShowCursor(false);

			options->controls->ExecuteCommand(InputSrvLockMouse(true));

			return true;
		}

		return false;
	}
}

//Перемещение текущего элемента в данную точку
void FormTestViewer::OnDragMoveTo(const GUIPoint &elementPosition)
{
	if( dragMap )
	{
		const GUIRectangle &rect = GetClientRect();

		GUIPoint pos = elementPosition;

		if( pos.x < 0 ) pos.x = 0; if( pos.x > rect.w - m_map.r.w ) pos.x = rect.w - m_map.r.w;
		if( pos.y < 0 ) pos.y = 0; if( pos.y > rect.h - m_map.r.h ) pos.y = rect.h - m_map.r.h;

		m_map.dec.r.pos += pos - m_map.r.pos;
		m_map.inc.r.pos += pos - m_map.r.pos;

		m_map.r.pos = pos;
	}
	else
	if( dragGizmo )
	{
		int x = m_startX + elementPosition.x;
		int y = m_startY + elementPosition.y;

		m_gizmo->MouseDrag(x,y);

		if( dragObject )
		{
			dragObject->pos = m_gizmo->GetTransform().pos - dragObject->off;

			UpdateBox();
		}
	}
}

//Прекратить операцию перетаскивания
void FormTestViewer::OnDragEnd()
{
	for( ;; )
	{
		if( m_dec )
		{
			m_dec = false; break;
		}
		if( m_inc )
		{
			m_inc = false; break;
		}

		if( dragMap )
		{
			dragMap = false;

			break;
		}

		if( dragGizmo )
		{
			SaveItem(dragObjectIndex,*dragObject);

			dragGizmo = false;

			break;
		}

		m_look = false;

		options->controls->ExecuteCommand(InputSrvLockMouse(false));

		GetMainControl()->Application->ShowCursor(true);

		break;
	}

	SaveHeader();
}

//Щелчёк левой кнопкой мыши
void FormTestViewer::OnMouseLeftClick(const GUIPoint &mousePos)
{
}

//Щелчёк правой кнопкой мыши
void FormTestViewer::OnMouseRightClick(const GUIPoint &mousePos)
{
	if( m_map.r.Inside(mousePos))
	{
		if( m_map.dec.r.Inside(mousePos)) return;
		if( m_map.inc.r.Inside(mousePos)) return;

		m_lookFixed = !m_lookFixed;

		SaveHeader();
	}
	else
	{
		if( m_look == false )
		{
			m_right = true;

			GetMainControl()->Application->ShowCursor(false);

			options->controls->ExecuteCommand(InputSrvLockMouse(true));

			m_look = true;
		}
	}
}

void FormTestViewer::OnMouseRightUp()
{
	m_right = false;

	m_look = false;

	options->controls->ExecuteCommand(InputSrvLockMouse(false));

	GetMainControl()->Application->ShowCursor(true);

	SaveHeader();
}

//Дельта от колеса
void FormTestViewer::OnMouseWeel(const GUIPoint &mousePos, long delta)
{
}

/////////////////////////////////////////////

void _cdecl FormTestViewer::OnCreateSound(GUIControl *sender)
{
	const char *name = m_list.GetAddSoundName();

	if( string::IsEmpty(name))
		return;

	AddSound(name,Vector(0.0f,0.0f,0.0f),0xffff0000,0.5f,5.0f,10.0f);

	m_list.InitList();

	m_list.SetFocusIndex(m_list.GetElementsCount() - 1);
}

void _cdecl FormTestViewer::OnCreateModel(GUIControl *sender)
{
	const char *name = m_list.GetAddModelPath();

	if( string::IsEmpty(name))
		return;

	AddModel(name,Vector(0.0,0.0f,0.0f),0.0f);

	m_list.InitList();

	m_list.SetFocusIndex(m_list.GetElementsCount() - 1);
}

/////////////////////////////////////////////

void _cdecl FormTestViewer::OnDeleteItems(GUIControl *sender)
{
	int index = m_list.GetFocusIndex();

	if( index >= 0 )
	{
		int focus = m_list.GetFocus();

		int count = (int)m_list.GetElementsCount();

		for( int i = count - 1 ; i >= 0 ; i-- )
		{
			if( m_list.GetElementSelect(i))
			{
				Entity *p = (Entity *)m_list.GetElementPtr(i);

				delete p;

				m_list.Del(i);
			}
		}

		UpdateBox();

		dragObject = null;
		dragObjectIndex = -1;

		m_list.InitList();

		count = (int)m_list.GetElementsCount();

		if( focus >= count )
			focus--;

		if( focus >= 0 )
		{
			m_list.SetFocus(focus);

			dragObjectIndex = m_list.GetFocusIndex();

			dragObject = (Entity *)m_list.GetElementPtr(dragObjectIndex);
		}

		SaveData();

		UpdateFocus();
	}
}

/////////////////////////////////////////////

void FormTestViewer::LoadData(int itemsCount)
{
//	m_file->SetPos(sizeof(FileHeader));

	loading = true;

	FileItem item;

	for( int i = 0 ; i < itemsCount ; i++ )
	{
		m_file->Read(&item,sizeof(item));

		switch( item.type )
		{
			case FileItem::sound: AddSound(item.name,item.pos,item.color,item.vol,item.r1,item.r2);
				break;

			case FileItem::model: AddModel(item.name,item.pos,item.ang);
				break;
		}
	}

	loading = false;
}

void FormTestViewer::SaveData()
{
	m_file->SetPos(0); m_file->Truncate();

	SaveHeader();

//	m_file->SetPos(sizeof(FileHeader));

	FileItem item;

	dword count = m_list.GetElementsCount();

	for( dword i = 0 ; i < count ; i++ )
	{
		memset(&item,0xcc,sizeof(item));

		const Entity *p = (Entity *)m_list.GetElementPtr(i);

		p->SaveTo(item);

		m_file->Write(&item,sizeof(item));
	}
}

void FormTestViewer::SaveItem(int index, const Entity &e)
{
	m_file->SetPos(sizeof(FileHeader) + index*sizeof(FileItem));

	FileItem item;

	e.SaveTo(item);

	m_file->Write(&item,sizeof(item));
}

/////////////////////////////////////////////////////////

FormTestViewer::DrawElem::DrawElem(const Entity *enti, const Matrix &view, const Matrix &proj, const GUIRectangle &_rect)
	: e(enti),in(false)
{
	if( e )
	{
		rect = _rect;

		Vector pos; float R;

		e->GetBoundSphere(pos,R);

		Vector pp = view.MulVertex(pos);

		z = pp.z;

		if( pp.z > 0.0f )
		{
			p = proj.Projection(pp,rect.w*0.5f,rect.h*0.5f);

			float r = R*p.w*rect.w;

			int x = round(p.x - r);
			int y = round(p.y - r);

			int w = round(p.x + r) - x;
			int h = round(p.y + r) - y;

			x += rect.x;
			y += rect.y;

			const int max_sel = 400;

			if( w > max_sel )
			{
				x += (w - max_sel)/2;
				w  = max_sel;
			}
			if( h > max_sel )
			{
				y += (h - max_sel)/2;
				h  = max_sel;
			}

			rt = GUIRectangle(x,y,w,h);

			in = true;
		}
	}
}

void FormTestViewer::DrawElem::Draw()
{
	const dword c = 0xffffffff;

	int d = round(1.0f*p.w*rect.w);

	if( d < 3 )
		d = 3;

	const int max_d = 60;

	if( d > max_d )
		d = max_d;

	const int &x = rt.x;
	const int &y = rt.y;

	const int &w = rt.w;
	const int &h = rt.h;

	int l = rect.x; int r = rect.x + rect.w;
	int t = rect.y; int b = rect.y + rect.h;

	bool ll = x > l; bool lr = x + d < r; bool rl = x + w - d > l; bool rr = x + w < r;
	bool tt = y > t; bool tb = y + d < b; bool bt = y + h - d > t; bool bb = y + h < b;

	if( ll && lr )
	{
		if( tt && tb )
		{
			GUIHelper::DrawHorizLine(x,d,y,c);
			GUIHelper::DrawVertLine (y,d,x,c);
		}
		if( bt && bb )
		{
			GUIHelper::DrawHorizLine(x,d,y + h,c);
			GUIHelper::DrawVertLine (y + h - d + 1,d,x,c);
		}
	}
	if( rl && rr )
	{
		if( tt && tb )
		{
			GUIHelper::DrawHorizLine(x + w - d + 1,d,y,c);
			GUIHelper::DrawVertLine (y,d,x + w,c);
		}
		if( bt && bb )
		{
			GUIHelper::DrawHorizLine(x + w - d + 1,d,y + h,c);
			GUIHelper::DrawVertLine (y + h - d + 1,d,x + w,c);
		}
	}
}
