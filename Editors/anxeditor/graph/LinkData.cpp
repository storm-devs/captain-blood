//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// LinkData	
//============================================================================================
			

#include "LinkData.h"

//============================================================================================

LinkData::LinkData()
{
	aranges[0] = 0;
	aranges[1] = 100000;
	mranges[0] = 0;
	mranges[1] = 100000;
	blendTime = 0.1f;
	syncPos = 0.0f;
	isSync = false;
	defaultLink = false;
}

LinkData::~LinkData()
{
}

//============================================================================================

//Записать данные в поток
void LinkData::Write(AnxStream & stream)
{
	stream.WriteVersion(1);
	stream << name;
	for(long i = 0; i < 4; i++)
	{
		stream << aranges[0];
		stream << aranges[1];
		stream << mranges[0];
		stream << mranges[1];
	}
	stream << blendTime;
	stream << syncPos;
	stream << isSync;
	stream << defaultLink;
}

//Прочитать данные из потока
void LinkData::Read(AnxStream & stream)
{
	dword ver = stream.ReadVersion();
	if(ver != 1) throw "Invalidate LinkData version";
	stream >> name;
	stream >> aranges[0];
	stream >> aranges[1];
	stream >> mranges[0];
	stream >> mranges[1];
	long tmp;
	for(dword i = 0; i < 3; i++)
	{
		stream >> tmp;
		stream >> tmp;
		stream >> tmp;
		stream >> tmp;
	}
	stream >> blendTime;
	stream >> syncPos;
	stream >> isSync;
	stream >> defaultLink;
}



