#ifndef _AminationBlendLevels_h_
#define _AminationBlendLevels_h_


enum AminationBlendLevel
{
	aminationBlendLevel_betweenAnimations = 10,		//Блэндинг между разными анимациями
	aminationBlendLevel_ragdoll = 500,				//Рэгдол
	aminationBlendLevel_autoEyes = 1000,			//Програмное моргание глаз
	aminationBlendLevel_emoteShapes = 1100,			//Емоции
	aminationBlendLevel_phonemeShapes = 1200,		//Фонемы
	aminationBlendLevel_postProcess = 16000,		//Дополнительная модификация анимации
};


#endif




