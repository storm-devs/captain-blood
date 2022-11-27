
#ifndef _IAnxEditorServiceAccess_h_
#define _IAnxEditorServiceAccess_h_


class IAnimation;
class IAnimationScene;


class IAnxEditorServiceAccess : public Service
{
public:
	//Запретить потоки
	virtual void DisableThreads() = 0;
	//Создать анимацию из памяти
	virtual IAnimation * CreateAnimation(IAnimationScene * scene, const void * data, unsigned long size) = 0;
};


#endif
