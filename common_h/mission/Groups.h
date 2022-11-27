

#ifndef _MissionGroups_h_
#define _MissionGroups_h_

struct GroupId
{
	__forceinline GroupId()
	{
		id = 0;
	};

	__forceinline GroupId(char c1, char c2, char c3, char c4)
	{
		id = (((long)c1) << 0) | (((long)c2) << 8) | (((long)c3) << 16) | (((long)c4) << 24);
	};

	long id;
};

//Ящики, которые закрывают проходы (влияют на поиск пути)
#define MG_ERROR_ID				GroupId()

//Ящики, которые закрывают проходы (влияют на поиск пути)
#define MG_AI_COLLISION			GroupId('a','i','c','l')
//Геометрия с физическими свойствами, надо будет удалить
#define MG_PHYSICGEOMETRY		GroupId('p','h','g','m')
//Персонаж
#define MG_CHARACTER			GroupId('c','h','r','s')
//Корабль
#define MG_SHIP					GroupId('s','h','i','p')
//Объект для воздействия со стороны игрока
#define MG_ACCEPTOR				GroupId('a','c','p','r')
#define MG_ACTIVEACCEPTOR		GroupId('a','a','c','p')
#define MG_DAMAGEACCEPTOR		GroupId('d','m','g','a')
//Камеры
#define MG_CAMERA				GroupId('c','a','m','s')

//Объекты отбрасывающие тень
#define MG_SHADOWCAST			GroupId('s','w','c','t')
//Объекты принимающие тень
#define MG_SHADOWRECEIVE		GroupId('s','w','r','v')
//Объекты принимающие тень
#define MG_SHADOWDONTRECEIVE	GroupId('s','w','n','r')
//Отражение в воде
#define MG_SEAREFLECTION		GroupId('s','r','f','l')
//Преломление в воде
#define MG_SEAREFRACTION		GroupId('s','r','f','r')

//Подбиратель бонусов
#define MG_BONUSPICKUPER		GroupId('b','n','p','r')

//События менеджера об активации-деактивации миссии
#define ACTIVATE_EVENT_GROUP	GroupId('m','m','a','e')
#define DEACTIVATE_EVENT_GROUP	GroupId('m','m','d','e')

//Уровень воды
#define MG_WATERLEVEL			GroupId('w','t','l','v')

//Сюда надо добавляться, если нужно экспортировать что нибудь при експорте в мис-файл
#define MG_EXPORT				GroupId('e','x','r','t')

#endif



