
#include "RagDollAnimation.h"

//============================================================================================

RagDollAnimation::RagDollAnimation() : Bones(_FL_)
{
	matrices=NULL;
}

RagDollAnimation::~RagDollAnimation()
{
}

//============================================================================================
//Доступ к костям анимации
//============================================================================================

//Получить количество костей в анимации
long RagDollAnimation::GetNumBones()
{
	return Bones.Size();	
}

//Получить имя кости
const char * RagDollAnimation::GetBoneName(long index)
{
	const char * BoneName=Bones[index].Name.c_str();

	return BoneName;
}

string RagDollAnimation::GetParentName(const char* BoneName)
{
	string TempName;
	TempName = BoneName;
	dword len = TempName.Size();
	if (len == 0) return string();

	len--;

	for (dword n = len; n > 0; n--)
	{
		if (TempName[n] == '|')
		{
			TempName.Delete(n, len-n+1);
			//Trace("Bone '%s', parent '%s'\n", GetName(), TempName.GetBuffer());
			return TempName;
		}
	}

	return string();
}


//Получить индекс родительской кости
string RagDollAnimation::GetParentName(const char* BoneName, int Depth)
{	
	string TempName;
	TempName = BoneName;
	dword len = TempName.Size();
	if (len == 0) return string();

	len--;

	for (dword n = len; n > 0; n--)
	{
		if (TempName[n] == '|')
		{
			TempName.Delete(n, len-n+1);

			len = TempName.Size();

			Depth--;

			if (Depth<0) return TempName;
		}
	}

	return "-1";
}

long RagDollAnimation::GetBoneParent(long index)
{

	const char* szName = Bones[index].Name.c_str();					

	int Depth=0;

	string ParentName="";

	long parent=-1;

	while (strcmp (ParentName.c_str(), "-1") != 0 && parent==-1)
	{
		ParentName = GetParentName(szName, Depth);

		for (dword j = 0; j < Bones.Size(); j++)
		{
			if (j == index) continue;			

			const char* szParentName = Bones[j].Name.c_str();

			if (strcmp (ParentName.c_str(), szParentName) == 0)
			{
				parent=j;

				break;
			}
		}

		if (parent==-1)
		{
			Depth++;
		}
	}

	/*const char* szName = Bones[index].Name.c_str();					

	string ParentName = GetParentName(szName);

	long parent=-1;

	for (dword j = 0; j < Bones.Size(); j++)
	{
		if (j == index) continue;

		const char* szParentName = Bones[j].Name.c_str();

		if (crt_stricmp (ParentName.c_str(), szParentName) == 0)
		{
			parent=j;

			break;
		}
	}*/

	return parent;

}