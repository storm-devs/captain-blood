//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// AnxAnimationViewer
//============================================================================================

#include "AnxAnimationViewer.h"
#include "..\AnxProject.h"
//#include "..\..\AnxAnimation\AnxData.h"
#include "AnxToolButton.h"
#include "AnxToolEdit.h"
#include "..\..\..\Common_h\corecmds.h"
#include "..\..\..\Common_h\InputSrvCmds.h"
#include "..\..\..\Common_h\AnimationNativeAccess.h"


//============================================================================================

dword AnxAnimationViewer::createCounter = 0;

AnxAnimationViewer::AnxAnimationViewer(AnxOptions & options, GUIControl * parent, long x, long y, long w, long h) : GUIControl(parent), opt(options)
{
	GUIRectangle rect(x, y, w, h);
	SetDrawRect(rect);
	SetClientRect(rect);
	particles = null;
	sounds = null;
	aniScene = null;
	isCameraView = false;
	model = null;
	loadAnimation = null;
	animationData = null;
	angle = 0.0f;
	position = 0.0f;
	timeLineRect.pos = timeLineRect.size = 0;
	playButton = null;
	eFrame = null;
	isPlay = false;
	isMovement = false;
	movementButton = null;
	targetingButton = null;
	isTargeting = false;
	modelButton = false;
	isModel = true;
	eventsButton = null;
	clipSize = 0;
	clipSRate = 1.0f;
	clipFrames = 0;
	asignedBones = 0;
	totalBones = 0;
	globalPosBones = 0;
	isEvents = false;
	SetAnimation(null, 0);
	IParticleService * psrv = (IParticleService *)api->GetService("ParticleService");
	if(psrv) particles = psrv->CreateManager(null);
	ISoundService * ssrv = (ISoundService *)api->GetService("SoundService");
	if(ssrv) sounds = ssrv->CreateScene("AnimationViewer", _FL_);
	events.Init(sounds, particles, null);	
	if(opt.animationService)
	{
		aniScene = (IAnimationScene *)opt.animationService->CreateScene(_FL_);
	}
	iesa = (IAnxEditorServiceAccess *)api->GetService("AnxEditorServiceAccess");
	Assert(iesa);
}

AnxAnimationViewer::~AnxAnimationViewer()
{
	api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));
	events.Stop();
	events.SetScene(null, Matrix());
	if(model)
	{
		model->Release();
		model = null;
	}
	if(loadAnimation)
	{
		loadAnimation->Release();
		loadAnimation = null;
	}
	if(particles)
	{
		particles->Release();
	}
	if(sounds)
	{
		sounds->Release();
	}
	if(aniScene)
	{
		aniScene->Release();
	}
	if(animationData)
	{
		delete animationData;
	}
}

//Создать панель управления
void AnxAnimationViewer::ShowControlPanel()
{
	AnxToolButton * b;
	GUILabel * lb;
	GUIRectangle rect = GetDrawRect();
	GUIRectangle newRect = rect;
	newRect = rect; newRect.h += 100;
	SetDrawRect(newRect);
	//Управление проигрыванием
	long bx = 10;
	long by = rect.h + 10;
	//На начало
	b = NEW AnxToolButton(null, this, bx, by, 32, 32);
	b->Glyph->Load("AnxEditor\\AnxViewerStart");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::GoToStart;
	b->Hint = "Go to first frame";
	bx += b->GetDrawRect().w + 8;
	//Предыдущий кадр
	b = NEW AnxToolButton(null, this, bx, by, 32, 32);
	b->Glyph->Load("AnxEditor\\AnxViewerPrev");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::GoToPrevFrame;
	b->Hint = "Go to previous frame";
	bx += b->GetDrawRect().w + 8;
	//Играть-остановить
	playButton = NEW AnxToolButton(null, this, bx, by, 32, 32);
	imagePlay.Load("AnxEditor\\AnxViewerPlay");
	imageStop.Load("AnxEditor\\AnxViewerStop");
	*playButton->Glyph = imageStop;
	isPlay = false;
	playButton->Align = GUIAL_Center;
	playButton->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::PlayStop;
	playButton->Hint = "Play/stop animation";
	bx += playButton->GetDrawRect().w + 8;
	//Следующий кадр
	b = NEW AnxToolButton(null, this, bx, by, 32, 32);
	b->Glyph->Load("AnxEditor\\AnxViewerNext");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::GoToNextFrame;
	b->Hint = "Go to next frame";
	bx += b->GetDrawRect().w + 8;
	//В конец
	b = NEW AnxToolButton(null, this, bx, by, 32, 32);
	b->Glyph->Load("AnxEditor\\AnxViewerEnd");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::GoToEnd;
	b->Hint = "Go to last frame";
	bx += b->GetDrawRect().w + 8;
	//Текущий кадр
	bx += 8;
	lb = NEW GUILabel(this, bx, by + 2, 50, 28);
	lb->Caption = "Frame:";
	lb->Layout = GUILABELLAYOUT_Left;
	bx += lb->GetDrawRect().w + 8;
	eFrame = NEW AnxToolEdit(this, bx, by + 6, 80, 20);
	eFrame->Text = "0";
	eFrame->OnEditUpdate = (CONTROL_EVENT)&AnxAnimationViewer::SetNewFrame;
	eFrame->Hint = "View/set current frame";
	bx += eFrame->GetDrawRect().w + 8;
	//Кнопки справа
	bx = DrawRect.w - 10;
	//Разрешить-запретить смещения
	bx -= 32;
	movementButton = NEW AnxToolButton(null, this, bx, by, 32, 32);
	movementEnable.Load("AnxEditor\\AnxViewerGoMvm");
	movementDisable.Load("AnxEditor\\AnxViewerStopMvm");
	*movementButton->Glyph = movementDisable;
	movementButton->Align = GUIAL_Center;
	movementButton->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::EnableMovement;
	isMovement = false;
	EnableMovement(movementButton);
	bx -= 8;
	//Перемещение в начало координат
	bx -= 32;
	b = NEW AnxToolButton(null, this, bx, by, 32, 32);
	b->Glyph->Load("AnxEditor\\AnxViewerToZero");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::ResetCamPosition;
	b->Hint = "Reset camera's position";
	bx -= 8;
	//Перемещение в начало координат
	bx -= 32;
	b = NEW AnxToolButton(null, this, bx, by, 32, 32);
	b->Glyph->Load("AnxEditor\\AnxViewerManToZero");
	b->Align = GUIAL_Center;
	b->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::ResetManPosition;
	b->Hint = "Reset object movement position";
	bx -= 8;
	//Слежение камеры за рутовой костью
	bx -= 32;
	targetingButton = NEW AnxToolButton(null, this, bx, by, 32, 32);	
	targetingEnable.Load("AnxEditor\\AnxViewerLockEn");
	targetingDisable.Load("AnxEditor\\AnxViewerLockDs");
	*targetingButton->Glyph = targetingEnable;
	targetingButton->Align = GUIAL_Center;
	targetingButton->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::EnableTargeting;
	isTargeting = true;
	EnableTargeting(targetingButton);
	bx -= 8;
	//Модель-скелет
	bx -= 32;
	modelButton = NEW AnxToolButton(null, this, bx, by, 32, 32);	
	showModel.Load("AnxEditor\\AnxViewerShowModel");
	showSkeleton.Load("AnxEditor\\AnxViewerShowSkeleton");
	*modelButton->Glyph = targetingEnable;
	modelButton->Align = GUIAL_Center;
	modelButton->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::SwitchModel;
	isModel = false;
	SwitchModel(modelButton);
	bx -= 8;
	//Включение отключение обработчика эвентов
	bx -= 32;
	eventsButton = NEW AnxToolButton(null, this, bx, by, 32, 32);
	eventsEnable.Load("AnxEditor\\AnxViewerEvtEn");
	eventsDisable.Load("AnxEditor\\AnxViewerEvtDs");
	*eventsButton->Glyph = eventsDisable;
	eventsButton->Align = GUIAL_Center;
	eventsButton->OnMousePressed = (CONTROL_EVENT)&AnxAnimationViewer::EnableEvents;
	isEvents = true;
	EnableEvents(eventsButton);
	bx -= 8;
	//Слайдер кадров
	timeLineRect = GetDrawRect();
	timeLineRect.x += 10;
	timeLineRect.y += timeLineRect.h - 45;
	timeLineRect.w -= 20;
	timeLineRect.h = 14;
}


//============================================================================================

//Установить анимацию
bool AnxAnimationViewer::SetAnimation(void * data, dword size)
{
#ifndef NO_TOOLS
	events.Stop();
	//Состояния зависящии от анимации
	bool oldPlay = isPlay;
	bool oldEvents = isEvents;
	isPlay = true;
	isEvents = true;
	PlayStop(null);
	EnableEvents(null);
	//Освобождаем ресурсы
	angle = 0.0f;
	position = 0.0f;
	curAniFileName.Empty();
	clipSize = 0;
	if(model)
	{
		model->Release();
		model = null;
	}
	if(loadAnimation)
	{
		loadAnimation->Release();
		loadAnimation = null;
	}
	if(animationData)
	{
		delete animationData;
		animationData = null;
	}
	//Создаём новую модельку
	if(opt.project->preveiwModel.Len())
	{
		//Ставим принудительный файл для загрузки
		model = opt.geometryService->CreateGMX(opt.project->preveiwModel, null, particles, sounds);
	}
	//Загружаем анимацию
	if(!opt.animationService) return false;
	if(!opt.geometryService) return false;
	if(!aniScene) return false;
	if(!data) return false;	
	loadAnimation = iesa->CreateAnimation(aniScene, AlignPtr(data), size);
	if(loadAnimation)
	{
		if(model)
		{
			model->SetAnimation(loadAnimation);
		}
	}
	//Настраиваем состояния зависящии от анимации
	isPlay = !oldPlay;
	PlayStop(null);
	isEvents = !oldEvents;
	GoToStart(this);
	EnableEvents(null);
	return true;
#else
	return false;
#endif
}

//============================================================================================


//Нарисовать
void AnxAnimationViewer::Draw()
{
	if(!opt.render) return;
	//Прямоугольник окна
	GUIRectangle rect = GetClientRect();
	ClientToScreen(rect);
	//Текущий режим
	bool isActive = false;
	if(parent)
	{
		isActive = (parent->IsActive != 0);
	}else{
		isActive = (IsActive != 0);
	}
	if(isActive/* && !isTargeting*/)
	{
		if(opt.controls->GetControlStateType("AnxSwitchCamera") == CST_ACTIVATED)
		{
			isCameraView = !isCameraView;
			if(isCameraView)
			{
				//oldPosition = opt.gui_manager->GetCursor()->GetPosition();
			}else{
				//opt.gui_manager->GetCursor()->SetPosition(oldPosition);
				opt.gui_manager->ShowCursor(false);
			}
		}
	}else{
		isCameraView = false;
	}
	opt.controls->ExecuteCommand(InputSrvLockMouse(isCameraView));
	opt.gui_manager->ShowCursor(!isCameraView);
	if(isCameraView)
	{
		api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(false));
		opt.gui_manager->ShowCursor(false);
		opt.gui_manager->GetCursor()->SetPosition(rect.pos + rect.size/2);
		opt.gui_manager->SetKeyboardFocus(null);
		opt.project->camAng.y += opt.controls->GetControlStateFloat("AnxGoRotHor");
		opt.project->camAng.x -= opt.controls->GetControlStateFloat("AnxGoRotVrt");
		if(opt.project->camAng.x > 0.5f*PI) opt.project->camAng.x = 0.5f*PI;
		if(opt.project->camAng.x < -0.5f*PI) opt.project->camAng.x = -0.5f*PI;
		opt.project->camAng.z = 0;
		Matrix mtx(opt.project->camAng);
		float speed = 2.0f*api->GetDeltaTime();
		if(opt.controls->GetControlStateType("AnxGoSpeed10x") == CST_ACTIVE) speed *= 10.0f;
		if(opt.controls->GetControlStateType("AnxGoSpeed01x") == CST_ACTIVE) speed *= 0.1f;
		if(opt.controls->GetControlStateType("AnxGoForward") == CST_ACTIVE)
		{
			opt.project->camPos += mtx.vz*speed;
		}
		if(opt.controls->GetControlStateType("AnxGoBack") == CST_ACTIVE)
		{
			opt.project->camPos -= mtx.vz*speed;
		}
		if(opt.controls->GetControlStateType("AnxGoLeft") == CST_ACTIVE)
		{
			opt.project->camPos -= mtx.vx*speed;
		}
		if(opt.controls->GetControlStateType("AnxGoRight") == CST_ACTIVE)
		{
			opt.project->camPos += mtx.vx*speed;
		}
	}else{
		api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));
	}
	//Фон
	GUIHelper::Draw2DRect(rect.x, rect.y, rect.w, rect.h, 0x00c0c0c0);
	//Вьюпорт
	cliper.Push();
	cliper.SetRectangle(rect);	
	//Матрицы
	opt.render->SetGlobalLight(!Vector(1.0f, 1.0f, 1.0f), false, Color(3.0f), 0.8f);
	Matrix mtx;
	opt.render->SetWorld(mtx);
	mtx.Build(opt.project->camAng);
	mtx.Inverse();
	if(isTargeting)
	{
		Vector camPos = opt.project->camPos + position;
		if(loadAnimation)
		{
			camPos += loadAnimation->GetBoneMatrix(0).pos;
		}
		mtx.pos = mtx*-camPos;
	}else{
		mtx.pos = mtx*-opt.project->camPos;
	}
	
	opt.render->SetView(mtx);
	Matrix prsp;
	opt.render->SetPerspective(1.25f, float(rect.w), float(rect.h));
	//Сетка
	for(long i = 0; i <= 40; i++)
	{
		dword color = 0xff808080;
		if((i % 4) == 0) color = 0xff000000;
		opt.render->DrawBufferedLine(Vector((i - 20)*0.25f, 0.0f, -5.0f), color, Vector((i - 20)*0.25f, 0.0f, 5.0f), color);
		opt.render->DrawBufferedLine(Vector(-5.0f, 0.0f, (i - 20)*0.25f), color, Vector(5.0f, 0.0f, (i - 20)*0.25f), color);
	}
	opt.render->FlushBufferedLines();
	//Моделька или скелет
	if(loadAnimation)
	{
		Vector mv;
		if(isPlay && isMovement)
		{
			loadAnimation->GetMovement(mv);
			position += mv.Rotate(angle);
			//position += mv;
		}else{
			loadAnimation->GetMovement(mv);
		}
	}
	Matrix transform(0.0f, angle, 0.0f, position.x, position.y, position.z);
	events.Update(transform, api->GetDeltaTime());
	if(model && isModel)
	{
		//if (model->IsLoaded())
		{
			model->SetTransform(transform);
			model->Draw();
		}
	}else{
		opt.render->FlushBufferedLines();
		const Matrix * m = null;
		dword cnt = 0;
		if(loadAnimation)
		{
			m = loadAnimation->GetBoneMatrices();
			cnt = loadAnimation->GetNumBones();
		}else{
			cnt = opt.project->skeleton->bones;
			for(dword i = 0; i < cnt; i++)
			{
				MasterSkeleton::Bone * bone = opt.project->skeleton->bones[i];
				bone->q.GetMatrix(bone->mtx);
				bone->mtx.pos = bone->p;
				if(bone->parent >= 0)
				{
					bone->mtx = bone->mtx*opt.project->skeleton->bones[bone->parent]->mtx;
				}
			}
		}
		Vector from, to;
		for(dword i = 0; i < cnt; i++)
		{			
			const Matrix * mtx = null;
			const char * boneName = null;
			if(loadAnimation)
			{			
				//Куда идёт кость
				to = m[i].pos;
				//Откуда идёт кость
				long pidx = loadAnimation->GetBoneParent(i);
				//if(pidx >= 0) from = m[pidx].pos; else from = to;
				if(pidx >= 0) from = m[pidx].pos; else from = 0.0f;
				//
				mtx = &m[i];
				boneName = loadAnimation->GetBoneName(i);
			}else{
				long pidx = opt.project->skeleton->bones[i]->parent;
				if(pidx >= 0)
				{					
					from = opt.project->skeleton->bones[pidx]->mtx.pos;
				}else{
					from = 0.0f;
					continue;
				}
				to = opt.project->skeleton->bones[i]->mtx.pos;
				mtx = &opt.project->skeleton->bones[i]->mtx;
				boneName = opt.project->skeleton->bones[i]->name;
			}
			//Рисуем кость
			dword colorFrom = 0xff8f008f;
			dword colorTo = 0xff008f8f;
			if(loadAnimation)
			{
				if(loadAnimation->GetNativeGraphInfo(AGNA_IsBoneUseGlobalPos(i)))
				{
					colorFrom = 0xff8f008f;
					colorTo = 0xffffff00;
				}
			}
			opt.render->DrawBufferedLine(from*transform, colorFrom, to*transform, colorTo);
			if(!api->DebugKeyState(VK_SHIFT))
			{
				if(mtx)
				{
					opt.render->DrawMatrix(Matrix(*mtx, transform), 0.08f);
				}
				if(boneName)
				{
					opt.render->Print(to, 0.3f, 0.0f, 0xff008f8f, boneName);
				}
			}
		}
		opt.render->FlushBufferedLines();
	}
	if(particles)
	{
		particles->Execute(api->GetDeltaTime());
	}
	if(sounds)
	{
		sounds->SetListenerMatrix(Matrix(opt.render->GetView()).Inverse());
	}
	if(aniScene)
	{
		opt.animationService->Update(aniScene, api->GetDeltaTime());		
	}
	//Стрелка направления
	opt.render->DrawVector(position, position + Vector(sinf(angle), 0.0f, cosf(angle)), 0xffff0000);
	//Оси
	Matrix drawSystem;
	drawSystem.m[1][1] = float(rect.w)/float(rect.h);
	drawSystem.m[2][2] = 0.0f;
	drawSystem.pos = Vector(-0.89f, -0.85f, 1.0f);
	opt.render->SetProjection(drawSystem);
	drawSystem.SetIdentity();
	drawSystem.pos = opt.render->GetView().MulVertexByInverse(Vector(0.0f, 0.0f, 2.0f));
	opt.render->DrawMatrix(drawSystem, 0.1f);
	cliper.Pop();
	//Информация
	if(clipSize)
	{
		opt.node.fontLabel->Print(rect.x + 4, rect.y + 4, 0xffffffff, "Clip size: %u bytes (compressed by %.2f)", clipSize, clipSRate);
		opt.node.fontLabel->Print(rect.x + 4, rect.y + 4 + opt.node.fontLabel->GetHeight() + 2, 0xffffffff, "Clip frames: %u", clipFrames);
		opt.node.fontLabel->Print(rect.x + 4, rect.y + 4 + (opt.node.fontLabel->GetHeight() + 2)*2, 0xffffffff, "Clip assigned bones %u of %u", asignedBones, totalBones);
		opt.node.fontLabel->Print(rect.x + 4, rect.y + 4 + (opt.node.fontLabel->GetHeight() + 2)*3, 0xffffffff, "Bones used global position %u", globalPosBones);
		
	}
	//Полоса с кадрами
	DrawTimeLine();
	//Считаем текущий кадр
	char buf[32];
	if(eFrame)
	{
		if(loadAnimation)
		{
			if(eFrame->isAccept)
			{
				float frame = AnimationGetCurrentFrame();
				crt_snprintf(buf, 32, "%i", long(frame));
				eFrame->Text = buf;
			}
		}else{
			eFrame->Text = "0";
		}
	}
	//Оконтовка
	GUIHelper::DrawLinesBox(rect.x, rect.y, rect.w, rect.h, 0xff000000);
	if(isCameraView)
	{
		GUIHelper::DrawLinesBox(rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2, 0xffffff80);
		GUIHelper::DrawLinesBox(rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4, 0xffffff80);
	}
	//Рисуем детей
	GUIControl::Draw();
}

//Сообщения
bool AnxAnimationViewer::ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam)
{
	GUIPoint cursor_pos;
	GUIHelper::ExtractCursorPos(message, lparam, hparam, cursor_pos);
	switch(message)
	{
	case GUIMSG_LMB_DOWN:
		if(CheckTimeLineClick(cursor_pos)) return true;
		break;
	}
	return GUIControl::ProcessMessages(message, lparam, hparam);
}

//Установить новую скорость проигрывания анимации
void AnxAnimationViewer::SetNewFPS(float fps)
{
	if(loadAnimation)
	{
		loadAnimation->GetNativeGraphInfo(AGNA_SetFPS(fps));
	}
}

//============================================================================================
//Кнопки проигрывания
//============================================================================================

void _cdecl AnxAnimationViewer::GoToStart(GUIControl * sender)
{
	if(loadAnimation)
	{
		AnimationSetCurrentFrame(0.0f);
	}	
}

void _cdecl AnxAnimationViewer::GoToPrevFrame(GUIControl * sender)
{
	if(!loadAnimation) return;
	float frame = AnimationGetCurrentFrame();
	AnimationSetCurrentFrame(long(frame) - 1.0f);
}

void _cdecl AnxAnimationViewer::PlayStop(GUIControl * sender)
{
	if(!playButton) return;
	if(loadAnimation)
	{
		if(isPlay)
		{
			*playButton->Glyph = imagePlay;
			isPlay = false;
			loadAnimation->Pause(true);
		}else{
			*playButton->Glyph = imageStop;
			isPlay = true;
			AnimationSetCurrentFrame(AnimationGetCurrentFrame());
			loadAnimation->Pause(false);
		}
	}else{
		*playButton->Glyph = imageStop;
		isPlay = false;
	}	
}

void _cdecl AnxAnimationViewer::GoToNextFrame(GUIControl * sender)
{
	if(!loadAnimation) return;
	float frame = AnimationGetCurrentFrame();
	AnimationSetCurrentFrame(long(frame) + 1.0f);
}

void _cdecl AnxAnimationViewer::GoToEnd(GUIControl * sender)
{
	if(!loadAnimation) return;
	AnimationSetCurrentFrame((float)AnimationGetNumberOfFrames());
}


//============================================================================================
//Разрешить-запретить перемещения персонажа
//============================================================================================

void _cdecl AnxAnimationViewer::EnableMovement(GUIControl * sender)
{
	if(!movementButton) return;
	if(isMovement)
	{
		*movementButton->Glyph = movementEnable;
		isMovement = false;
		movementButton->Hint = "Enable movement object by read from root bone";
	}else{
		*movementButton->Glyph = movementDisable;
		isMovement = true;
		movementButton->Hint = "Disable movement object by read from root bone";
	}
}

//============================================================================================
//Разрешить-запретить слежение за персонажем
//============================================================================================

void _cdecl AnxAnimationViewer::EnableTargeting(GUIControl * sender)
{
	if(!targetingButton) return;
	if(isTargeting)
	{
		*targetingButton->Glyph = targetingEnable;
		isTargeting = false;
		targetingButton->Hint = "Move camera with object";
	}else{
		*targetingButton->Glyph = targetingDisable;
		isTargeting = true;
		targetingButton->Hint = "Full free move camera";
	}
}

//============================================================================================
//Рисовать модельку или скилет
//============================================================================================

void _cdecl AnxAnimationViewer::SwitchModel(GUIControl * sender)
{
	if(!modelButton) return;
	if(!model) isModel = true;
	if(isModel)
	{
		*modelButton->Glyph = showModel;
		isModel = false;
		modelButton->Hint = "Show model";
	}else{
		*modelButton->Glyph = showSkeleton;
		isModel = true;
		modelButton->Hint = "Show skeleton";
	}
}

//============================================================================================
//Включить-выключить события
//============================================================================================

void _cdecl AnxAnimationViewer::EnableEvents(GUIControl * sender)
{
	if(!eventsButton) return;
	if(!isEvents && loadAnimation)
	{
		*eventsButton->Glyph = eventsEnable;
		isEvents = true;		
		eventsButton->Hint = "Disable standart animation events";
		if(loadAnimation)
		{
			Matrix transform(0.0f, angle, 0.0f, position.x, position.y, position.z);
			if(model) events.SetScene(model, transform);
			events.AddAnimation(loadAnimation);			
		}
	}else{
		*eventsButton->Glyph = eventsDisable;
		isEvents = false;		
		eventsButton->Hint = "Enable standart animation events";
		events.Stop();
	}
}

//============================================================================================
//Полоса с кадрами
//============================================================================================

void AnxAnimationViewer::DrawTimeLine()
{
	if(!timeLineRect.w) return;
	//Анимация
	if(!loadAnimation) return;
	//Прямоугольник полосы в экранных координатах
	GUIRectangle rect = timeLineRect;
	ClientToScreen(rect);
	//Рисуем бегунок
	float frame = AnimationGetCurrentFrame();
	dword frames = AnimationGetNumberOfFrames() - 1;
	if(!frames) return;
	//Бегунок
	long pos = long(rect.w*(frame/float(frames)) + 0.5f);
	if(pos > rect.w - 1) pos = rect.w - 1;
	GUIHelper::Draw2DRectAlpha(pos + rect.x - 2, rect.y + 1, 5, rect.h - 1, 0xcf8080ff);
	//Разметка
	float step = rect.w/float(frames);
	long fstep = 1;
	Assert(step > 0.0f);
	while(step < 30.0f)
	{
		fstep *= 10;
		step *= 10.0f;
	}
	float markpos = float(rect.x);
	long y = rect.y + rect.h + 4;
	for(dword i = 0; i <= frames; i += fstep)
	{
		long x = long(markpos);
		GUIHelper::Draw2DLine(x, y, x, y + 8, 0xff000000);
		markpos += step;
		if(opt.fontViewer)
		{
			char buf[32];
			crt_snprintf(buf, 32, "%u", i);
			dword fx = x - opt.fontViewer->GetWidth(buf)/2;
			opt.fontViewer->Print(fx, y + 10, 0xff000000, buf);
		}
	}
	//Полоса прокрутки
	GUIHelper::DrawLinesBox(rect.x - 2, rect.y, rect.w + 5, rect.h, 0xff000080);
}

void _cdecl AnxAnimationViewer::SetNewFrame(GUIControl * sender)
{
	if(!loadAnimation) return;
	long frame = atol(eFrame->Text);
	AnimationSetCurrentFrame(float(frame));
}

bool AnxAnimationViewer::CheckTimeLineClick(const GUIPoint & point)
{
	if(timeLineRect.w < 2) return false;
	if(isPlay) return false;
	if(!loadAnimation) return false;
	GUIPoint p = point; p += DrawRect.pos;
	ScreenToClient(p);
	GUIRectangle rect = timeLineRect;
	rect.x -= 2; rect.w += 5;
	if(!rect.Inside(p)) return false;
	if(p.x < timeLineRect.x) p.x = timeLineRect.x;
	if(p.x > timeLineRect.x + timeLineRect.w - 1) p.x = timeLineRect.x + timeLineRect.w - 1;
	float frame = float(p.x - timeLineRect.x)/float(timeLineRect.w - 1);
	frame *= (AnimationGetNumberOfFrames() - 1.0f);
	if(frame < 0.0f) frame = 0.0f;
	AnimationSetCurrentFrame(frame);
	return true;
}

//Получить позицию кадра на временной полосе в экранных координатах
long AnxAnimationViewer::GetFramePosition(long frame, long & y, long & h, bool isClamp)
{
	y = h = 0;
	if(timeLineRect.w < 2) return 0;
	if(!loadAnimation) return 0;
	long frames = AnimationGetNumberOfFrames();
	if(frames < 2) return 0;
	if(frame < 0)
	{
		if(!isClamp) return 0;
		frame = 0;
	}
	if(frame >= frames)
	{
		if(!isClamp) return 0;
		frame = frames - 1;
	}
	//Прямоугольник полосы в экранных координатах
	GUIRectangle rect = timeLineRect;
	ClientToScreen(rect);
	//Координаты по высоте
	y = rect.y;
	h = rect.h;
	//Координата по горизонтали	
	float x = float(frame)/float(frames - 1);
	return long(rect.x + x*rect.w + 0.5f);
}


//============================================================================================
//Доступ к анимации
//============================================================================================

//Установить текущий кадр анимации
void AnxAnimationViewer::AnimationSetCurrentFrame(float frame)
{
	if(!loadAnimation) return;
	loadAnimation->GetNativeGraphInfo(AGNA_SetCurrentFrame(frame));
}

//Получить текущий кадр анимации
float AnxAnimationViewer::AnimationGetCurrentFrame()
{
	if(!loadAnimation) return 0.0f;
	AGNA_GetCurrentFrame frame;
	if(loadAnimation->GetNativeGraphInfo(frame))
	{
		return frame.currentFrame;
	}
	return 0.0f;
}

//Получить количество кадров в анимации
dword AnxAnimationViewer::AnimationGetNumberOfFrames()
{
	if(!loadAnimation) return 0;
	AGNA_GetNumberOfFrames gnof;
	if(loadAnimation->GetNativeGraphInfo(gnof))
	{
		return gnof.frames;
	}
	return 0;
}

//============================================================================================
//Разное
//============================================================================================

void _cdecl AnxAnimationViewer::ResetCamPosition(GUIControl * sender)
{
	opt.project->ResetCameraPosition();
}

void _cdecl AnxAnimationViewer::ResetManPosition(GUIControl * sender)
{
	position = 0.0f;
	angle = 0.0f;
}


