//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationConstsAttackForm
//============================================================================================

#include "NodeAnimationConstsAttackForm.h"
#include "..\..\..\common_h\controls.h"

//============================================================================================

NodeAnimationConstsAttackForm::NodeAnimationConstsAttackForm(AnxOptions & options, GUIControl * parent, const GUIPoint & position, long size) : GUIControl(parent), opt(options)
{
	GUIRectangle rect(position, GUIPoint(size, size));
	SetDrawRect(rect);
	SetClientRect(rect);
	angleStart = null;
	angleEnd = null;
	radiusStart = null;
	radiusEnd = null;
	scale = 2.0f;
	dragDlt = 0.0f;
	drag = ds_none;
	man.Load("AnxEditor\\AnxManUp");
	texture->SetTexture(man.GetTexture());
	texture = opt.render->GetTechniqueGlobalVariable("AnxEditorDrawTexture", _FL_);
}

NodeAnimationConstsAttackForm::~NodeAnimationConstsAttackForm()
{
	angleStart = null;
	angleEnd = null;
	radiusStart = null;
	radiusEnd = null;
}

//============================================================================================

//Рисование
void NodeAnimationConstsAttackForm::Draw()
{
	opt.render->FlushBufferedLines();
	//Масштабируем
	float dScale = opt.controls->GetControlStateFloat("AnxScale2DH");
	dScale -= opt.controls->GetControlStateFloat("AnxScale2DV");
	Clampfr(dScale, -0.5f, 0.5f);
	scale *= 1.0f + dScale;
	Clampfr(scale, 0.1f, 16.0f);
	//Прямоугольник окна
	GUIRectangle rect = GetDrawRect();
	ClientToScreen(rect);
	//Рисуем панель
	cliper.Push();
	cliper.SetInsRectangle(rect);
	//Рисуем масштабную сетку
	Vector from(0.0f), to(0.0f);
	Matrix mtx;
	opt.render->SetWorld(mtx);
	Matrix veiw = opt.render->GetView();
	opt.render->SetView(mtx);
	Matrix projection = opt.render->GetProjection();
	mtx.m[0][0] = scale;
	mtx.m[1][1] = scale;
	opt.render->SetProjection(mtx);
	//Шаг сетки в зависимости от масштаба
	long step;
	if(scale >= 1.3f){ step = 1; }else if(scale >= 0.6f){ step = 2; }else
		if(scale >= 0.2f){ step = 5; }else if(scale >= 0.1f){ step = 10; }else
			if(scale >= 0.05f){ step = 20; }else{ step = 50; }
	//Сетка
	for(long i = 0; i <= 100; i++)
	{
		if(i > 0 && i < 100)
		{
			if(i % step) continue;
		}		
		dword color = 0xff909090;
		if(!(i % 10)) color = 0xff000000;
		from.x = -0.9f; to.x = 0.9f;
		from.y = to.y = (i*0.01f - 0.5f)*1.8f;
		opt.render->DrawBufferedLine(from, color, to, color);
		from.y = -0.9f; to.y = 0.9f;
		from.x = to.x = (i*0.01f - 0.5f)*1.8f;
		opt.render->DrawBufferedLine(from, color, to, color);
	}
	opt.render->FlushBufferedLines();
	//Коэфициент масштабирования
	const float mst = 0.9f/5.0f;
	//Рисуем человека
	VertexUV vrt[4];
	float manSize = 0.35f*mst;
	vrt[0].p = Vector(-manSize, manSize, 0);
	vrt[0].u = 0.0f; vrt[0].v = 0.0f;
	vrt[1].p = Vector(-manSize, -manSize, 0);
	vrt[1].u = 0.0f; vrt[1].v = 1.0f;	
	vrt[2].p = Vector(manSize, manSize, 0);
	vrt[2].u = 1.0f; vrt[2].v = 0.0f;	
	vrt[3].p = Vector(manSize, -manSize, 0);
	vrt[3].u = 1.0f; vrt[3].v = 1.0f;	

	ShaderId id;
	opt.render->GetShaderId("AnxScreenDrawTexture", id);
	opt.render->DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 2, vrt, sizeof(VertexUV));
	//Углы
	float tmp;
	float & as = angleStart ? *angleStart : tmp;
	float & ae = angleEnd ? *angleEnd : tmp;
	Clampfr(as, -360.0f, 360.0f);
	Clampfr(ae, -360.0f, 360.0f);
	if(as > ae){ tmp = as; as = ae; ae = tmp; }
	//Радиусы
	float & rs = radiusStart ? *radiusStart : tmp;
	float & re = radiusEnd ? *radiusEnd : tmp;
	Clampfr(rs, 0.1f, 100.0f);
	Clampfr(re, 0.1f, 100.0f);
	if(rs > re){ tmp = rs; rs = re; re = tmp; }
	//Заполняем сектор
	float ang = as*PI/180.0f;
	float astp = (ae - as)*(PI/180.0f)*(1.0f/1023.0f);
	for(i = 0; i < 1024; i++, ang += astp)
	{
		float dx = sinf(ang)*0.9f/5.0f;
		float dy = cosf(ang)*0.9f/5.0f;
		sector[i][0].p = Vector(rs*dx, rs*dy, 0.0f);
		sector[i][0].c = 0x90cf0000;
		sector[i][1].p = Vector(re*dx, re*dy, 0.0f);
		sector[i][1].c = 0x90cf0000;
	}

	
	opt.render->GetShaderId("AnxScreenDrawFill", id);
	opt.render->DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 2047, sector, sizeof(Vertex));
	//Позиция курсора в позиции вьюпорта
	GUIPoint mpos = opt.gui_manager->GetCursor()->GetPosition();
	ScreenToClient(mpos);
	Vector mp;
	mp.x = mpos.x - GetDrawRect().w*0.5f;
	mp.y = GetDrawRect().h*0.5f - mpos.y;
	mp.z = 0.0f;
	mp.x /= 0.5f*GetDrawRect().w*scale*0.9f/5.0f;
	mp.y /= 0.5f*GetDrawRect().h*scale*0.9f/5.0f;
	//Смотрим попадание мыши
	dword selRE = 0xff0000ff;
	dword selRS = 0xff0000ff;
	dword selAS = 0xff0000ff;
	dword selAE = 0xff0000ff;
	if(drag == ds_none)
	{
		if(OnArc(mp, *radiusEnd))
		{
			selRE = 0xffffff00;
			if(opt.controls->GetControlStateType("AnxDragMode") == CST_ACTIVE) drag = ds_earc;
		}else	
		if(OnArc(mp, *radiusStart))
		{
			selRS = 0xffffff00;
			if(opt.controls->GetControlStateType("AnxDragMode") == CST_ACTIVE) drag = ds_sarc;
		}else
		if(OnSide(mp, *angleEnd))
		{
			selAE = 0xffffff00;
			if(opt.controls->GetControlStateType("AnxDragMode") == CST_ACTIVE) drag = ds_eside;
		}else
		if(OnSide(mp, *angleStart))
		{
			selAS = 0xffffff00;
			if(opt.controls->GetControlStateType("AnxDragMode") == CST_ACTIVE) drag = ds_sside;
		}
	}else{
		switch(drag)
		{
		case ds_earc:
			DragArc(mp, radiusEnd);
			break;
		case ds_sarc:
			DragArc(mp, radiusStart);
			break;
		case ds_eside:
			DragSide(mp, angleEnd);
			break;
		case ds_sside:
			DragSide(mp, angleStart);
			break;
		default:
			drag = ds_none;
		}
		
	}
	//Рисуем элементы
	opt.render->DrawBufferedLine(sector[0][0].p, selAS, sector[0][1].p, selAS);
	opt.render->DrawBufferedLine(sector[1023][0].p, selAE, sector[1023][1].p, selAE);
	for(i = 0; i < 1023; i++)
	{
		opt.render->DrawBufferedLine(sector[i][0].p, selRS, sector[i + 1][0].p, selRS);
		opt.render->DrawBufferedLine(sector[i][1].p, selRE, sector[i + 1][1].p, selRE);
	}
	opt.render->FlushBufferedLines();
	//Остатки
	cliper.Pop();
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000040);
	GUIControl::Draw();
}

bool NodeAnimationConstsAttackForm::OnArc(const Vector & p, float rad)
{
	if(!angleStart || !angleEnd) return false;
	float r = p.GetLength();
	dragDlt = rad - r;
	if(fabs(dragDlt) < 0.1f/scale)
	{
		float ang = Vector(p.x, 0.0f, p.y).GetAY()*(180.0f/PI);
		if(*angleStart <= ang && ang <= *angleEnd) return true;
	}
	return false;
}

void NodeAnimationConstsAttackForm::DragArc(const Vector & p, float * rad)
{
	if(rad) *rad = p.GetLength() + dragDlt; else drag = ds_none;
	if(opt.controls->GetControlStateType("AnxDragMode") != CST_ACTIVE)
	{
		drag = ds_none;
	}
}

bool NodeAnimationConstsAttackForm::OnSide(const Vector & p, float ang)
{
	if(!radiusStart || !radiusEnd) return false;
	float r = p.GetLength();
	if(r < 1e-10) return false;
	if(r < *radiusStart || r > *radiusEnd) return false;
	Vector dir(sinf(ang*PI/180.0f), cosf(ang*PI/180.0f), 0.0f);
	float cs = (dir | p)/r;
	if(cs <= 0.0f) return false;
	dragDlt = sqrtf(Clampf(1.0f - cs*cs));
	float dist = dragDlt*r;
	if(dist < 0.1f/scale)
	{
		dragDlt = (float)-asin(dragDlt);
		return true;
	}
	return false;
}

void NodeAnimationConstsAttackForm::DragSide(const Vector & p, float * ang)
{
	if(opt.controls->GetControlStateType("AnxDragMode") != CST_ACTIVE)
	{
		drag = ds_none;
	}else
	if(ang)
	{
		float dang = (Vector(p.x, 0.0f, p.y).GetAY() + dragDlt)*180.0f/PI - *ang;
		if(fabs(dang) >= 180)
		{
			dang -= long(dang/360.0f + dang >= 0.0f ? 1.0f : -1.0f)*360.0f;
		}		
		*ang += dang;
	}else{
		drag = ds_none;
	}
}

