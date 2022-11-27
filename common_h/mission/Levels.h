

#ifndef _MissionLevels_h_
#define _MissionLevels_h_

//Первый используемый
#define ML_FIRST			-10000000
//Исполнение мэнеджеров сервисов
#define ML_SCENE_START		-9500000
//Триггер активации миссии
#define ML_ACTIVATE_TRIGGER	-9300000
//Установить источники света
#define ML_LIGHTS_ON		-9000000
//Поставить звукового слушателя
#define ML_SOUND_LISTENER	-5000000
//Камера
#define ML_CAMERAMOVE_BASE	-1000000
#define ML_CAMERAMOVE		(ML_CAMERAMOVE_BASE + 100)
#define ML_CAMERAMOVE_ANI	(ML_CAMERAMOVE_BASE + 200)
#define ML_CAMERAMOVE_FREE	(ML_CAMERAMOVE_BASE + 300)
//Исполняемые объекты
#define ML_TRIGGERS			-20000
#define ML_EXECUTE1			-19000
#define ML_EXECUTE2			-18000
#define ML_EXECUTE3			-17000
#define ML_EXECUTE4			-16000
#define ML_EXECUTE5			-15000
#define ML_EXECUTE6			-14000
#define ML_EXECUTE7			-13000
#define ML_EXECUTE8			-12000
#define ML_EXECUTE9			-11000
#define ML_EXECUTE_START	-10000
#define ML_EXECUTE_END		-1000
//Рисование теневого буфера
#define ML_SHADOWCAST		-100
//Отбрасывание теней
#define ML_SHADOWRECEIVE	-50

//Заполнение фона
#define ML_FILL				5000
//Типовое рисование статической геометрии
#define ML_GEOMETRY1		10000
#define ML_GEOMETRY2		12000
#define ML_GEOMETRY3		13000
#define ML_GEOMETRY4		14000
#define ML_GEOMETRY5		15000
//Типовое рисование динамических объектов
#define ML_DYNAMIC1			21000
#define ML_DYNAMIC2			22000
#define ML_DYNAMIC3			23000
#define ML_DYNAMIC4			24000
#define ML_DYNAMIC5			25000
//Рисование полупрозрачных объектов
#define ML_ALPHA1			31000
#define ML_ALPHA2			32000
#define ML_ALPHA3			33000
#define ML_ALPHA4			34000
#define ML_ALPHA5			35000
//Рисование партикловых эффектов
#define ML_PARTICLES1		41000
#define ML_PARTICLES2		42000
#define ML_PARTICLES3		43000
#define ML_PARTICLES4		44000
#define ML_PARTICLES5		45000
//Рисование погодных эффектов
#define ML_WEATHER1			61000
#define ML_WEATHER2			62000
#define ML_WEATHER3			63000
#define ML_WEATHER4			64000
#define ML_WEATHER5			65000
//Рисование пост эффектов
#define ML_POSTEFFECTS		90000
//Рисование интерфейсов
#define ML_GUI1				101000
#define ML_GUI2				102000
#define ML_GUI3				103000
#define ML_GUI4				104000
#define ML_GUI5				105000
//Рисование отладочной информации
#define ML_DEBUG			201000
#define ML_DEBUG1			202000
#define ML_DEBUG2			203000
#define ML_DEBUG3			204000
//Установить источники света
#define ML_LIGHTS_OFF		9000000
//Исполнение мэнеджеров сервисов
#define ML_SCENE_END		9500000
//Последний используемый
#define ML_LAST				10000000



#endif



