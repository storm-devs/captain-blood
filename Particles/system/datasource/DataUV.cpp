#include "DataUV.h"
#include "..\..\icommon\memfile.h"
#include "..\..\..\common_h\core.h"

#include "fieldlist.h"

#include "..\..\TextFile.h"

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "..\..\..\Common_h\data_swizzle.h"

//конструктор/деструктор
DataUV::DataUV (FieldList* pMaster) : Frames (_FL_, 1)
{
	szName = NULL;
	szEditorName = NULL;

	Master = pMaster;
}

DataUV::~DataUV ()
{
}

	//Получить значение   [ x,y = UV1; z,w = UV2 ]
const Vector4 &DataUV::GetValue (DWORD FrameNum)
{
	DWORD TotalFrames = Frames.Size();
	FrameNum = FrameNum % TotalFrames;
	return Frames[FrameNum];
}

	//Установить значения
void DataUV::SetValues (const Vector4* _Frames, DWORD FramesCount)
{
	Frames.DelAll();

	for (DWORD n = 0; n < FramesCount; n++)
	{
		Frames.Add(_Frames[n]);
	}

	if (Master) Master->UpdateCache();
}

	//Получить кол-во кадров
DWORD DataUV::GetFrameCount ()
{
	return Frames.Size();
}

void DataUV::Load (MemFile* File)
{
	dword ElementCount = 0;
	File->ReadType(ElementCount);
	XSwizzleDWord(ElementCount);


	for (DWORD n = 0; n < ElementCount; n++)
	{
		Vector4 rFrame;
		File->ReadType(rFrame.x);
		File->ReadType(rFrame.y);
		File->ReadType(rFrame.z);
		File->ReadType(rFrame.w);

		XSwizzleFloat(rFrame.x);
		XSwizzleFloat(rFrame.y);
		XSwizzleFloat(rFrame.z);
		XSwizzleFloat(rFrame.w);

		Vector4 newFrame;
		newFrame = rFrame;
		newFrame.z += newFrame.x;
		newFrame.w += newFrame.y;
		Frames.Add(newFrame);
	}

	//static char AttribueName[128];
	dword NameLength = 0;
	File->ReadType(NameLength);
	XSwizzleDWord(NameLength);

	Assert (NameLength < 128);
	//File->Read(AttribueName, NameLength);
	const char* AttribueName = File->GetPointerToString(NameLength);

	SetName (AttribueName, "a");

	if (Master) Master->UpdateCache();
}

void DataUV::SetName (const char* szName, const char* szEditorName)
{
	//api->Trace("DataUV::SetName - '%s'", szName);
	this->szName = szName;
	this->szEditorName = szEditorName;
}

const char* DataUV::GetName ()
{
	return szName;
}

void DataUV::Write (MemFile* File)
{
	DWORD ElementCount = GetFrameCount();
	File->WriteType(ElementCount);
	for (DWORD n = 0; n < ElementCount; n++)
	{
		float w = Frames[n].z-Frames[n].x;
		float h = Frames[n].w-Frames[n].y;
		File->WriteType(Frames[n].x);
		File->WriteType(Frames[n].y);
		File->WriteType(w);
		File->WriteType(h);
	}

	//save name
	DWORD NameLength = crt_strlen(szName);
	DWORD NameLengthPlusZero = NameLength+1;
	File->WriteType(NameLengthPlusZero);
	Assert (NameLength < 128);
	File->Write(szName, NameLength);
	File->WriteZeroByte();
}

const char* DataUV::GetEditorName ()
{
	return szEditorName;
}

#ifndef _XBOX
void DataUV::WriteXML (TextFile* xmlFile, dword level)
{
	xmlFile->Write((level+1), "<Name val = \"%s\" />\n", szName);

	xmlFile->Write((level+1), "<Frames>\n");
	DWORD ElementCount = GetFrameCount();
	for (dword n = 0; n < ElementCount; n++)
	{
		xmlFile->Write((level+1), "<Frame>\n");

		float w = Frames[n].z-Frames[n].x;
		float h = Frames[n].w-Frames[n].y;


		xmlFile->Write((level+2), "<x val = \"%f\" />\n", Frames[n].x);
		xmlFile->Write((level+2), "<y val = \"%f\" />\n", Frames[n].y);
		xmlFile->Write((level+2), "<w val = \"%f\" />\n", w);
		xmlFile->Write((level+2), "<h val = \"%f\" />\n", h);

		xmlFile->Write((level+1), "</Frame>\n");
	}
	xmlFile->Write((level+1), "</Frames>\n");

}

void DataUV::LoadXML (TiXmlElement* root)
{
	TiXmlElement* name = root->FirstChildElement("Name");
	if (name)
	{
		SetName (name->Attribute("val"), "a");
	}



	TiXmlElement* framesNode = NULL;
	framesNode = root->FirstChildElement("Frames");

	if (framesNode)
	{
		for(TiXmlElement* child = framesNode->FirstChildElement(); child; child = child->NextSiblingElement())
		{
			string NodeName = child->Value();

			if (NodeName == "Frame")
			{

				Vector4 newFrame;


				TiXmlElement* val_x = child->FirstChildElement("x");
				TiXmlElement* val_y = child->FirstChildElement("y");
				TiXmlElement* val_w = child->FirstChildElement("w");
				TiXmlElement* val_h = child->FirstChildElement("h");


				if (val_x)
				{
					newFrame.x = (float)atof (val_x->Attribute("val"));
				}
				if (val_y)
				{
					newFrame.y = (float)atof (val_y->Attribute("val"));
				}
				if (val_w)
				{
					newFrame.z = (float)atof (val_w->Attribute("val"));
				}
				if (val_h)
				{
					newFrame.w = (float)atof (val_h->Attribute("val"));
				}


				newFrame.z += newFrame.x;
				newFrame.w += newFrame.y;

				Frames.Add(newFrame);

			}
		}
	}


	if (Master) Master->UpdateCache();

}

#endif