#include "..\CharacterAI.h"

#include "..\..\Character\Components\CharacterAnimationEvents.h"

class CharacterAI_Doodle : public CharacterController
{
public:
	CharacterAI_Doodle(Character & ch, const char* szName) : CharacterController(ch, szName)
	{
		int a = 0;
	};

	virtual bool AllowToBeMoved() { return false; };
};

DeclareCharacterController(CharacterAI_Doodle, "Doodle", 0)

class CharacterAI_DoodleGuard : public CharacterController
{
public:
	Character * chr;

	CharacterAI_DoodleGuard(Character & ch, const char* szName) : CharacterController(ch, szName)
	{		
		chr = &ch;
	};

	virtual bool AllowToBeMoved() { return false; };

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset(){ chr->physics->SetAllowMove(false); };
	
};

DeclareCharacterController(CharacterAI_DoodleGuard, "DoodleGuard", 0)

#ifndef NO_TOOLS

class IPreviewService : public Service
{
public:

	virtual void SetCharacter(Character *p) = 0;

	virtual IAnimation *SetAnimation(const char *name) = 0;

	virtual IGMXScene *GetModel() = 0;

	virtual void Begin() = 0;

	virtual void Rotate(float ay) = 0;

	virtual void Reset() = 0;

	virtual void LockEvents(bool lock) = 0;

	virtual bool AddName(const char *xmlName, const char *xmlPath, const char *verPath) = 0;

};

class PreviewService : public IPreviewService
{
	Character *chr; IAnimation *animation;

	Matrix mat;

	IFileService *storage;

	struct Item
	{
		char name[32]; FILETIME t;

		dword a;		// библиотека/миссия не найдена
		dword f;		// необходим переэкспорт (для миссии)
	};

	array<Item> xmlzs;

	IFile *data;

	bool init;			// инициализация при первом обращении

private:

	bool Check(array<Item> &items, const char *path, const char *mask, bool reset = true)
	{
		bool update = false;

		IFinder *finder = storage->CreateFinder(path,mask,
			find_all_files_no_mirrors|find_no_recursive|find_no_files_from_packs,_FL_);

		dword count = finder->Count();

		for( dword i = 0 ; i < count ; i++ )
		{
			HANDLE file = CreateFile(finder->FilePath(i),
				GENERIC_READ,FILE_SHARE_WRITE,null,
				OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,null);

			FILETIME t;

			GetFileTime(file,null,null,&t);
			CloseHandle(file);

			const char *name = finder->Title(i);

			for( int j = 0 ; j < items ; j++ )
			{
				Item &item = items[j];

				if( string::IsEqual(name,item.name))
					break;
			}

			if( j < items )
			{
				Item &item = items[j];

				if( reset )
				{
					item.f = 0;
				}

				if( CompareFileTime(&item.t,&t)) // изменилась версия
				{
					item.t = t;
					item.f = 1;

					update = true;
				}

				item.a = 0;
			}
			else
			{
				Item &item = items[items.Add()];

				ZeroMemory(&item,sizeof(item));

				strcpy_s(item.name,sizeof(item.name),name);

				item.t = t;
				item.f = 1;

				item.a = 0;
			}
		}

		RELEASE(finder)

		return
			update ||
			count < items.Size(); // были удалены некоторые файлы
	}

	void SaveData()
	{
		data->SetPos(0); data->Truncate();

		for( int j = 0 ; j < xmlzs ; j++ )
		{
			Item &item = xmlzs[j];

			if( item.a == 0 )
				data->Write(&item,sizeof(item));
		}
	}

public:

	PreviewService() : xmlzs(_FL_,32)
	{
		chr = null;
		data = null;
		init = true;
	}

	virtual bool Init()
	{
		chr = null; animation = null;

		return true;
	}

	void InitService(const char *xmlPath, const char *verPath)
	{
		string fileName;

	//	api->Trace("    INIT PREVIEW_SERVICE\n");

		storage = (IFileService *)api->GetService("FileService");
		Assert(storage)

		///////////////////////////

		fileName = verPath;	fileName += "xmlz.ver";

		data = storage->OpenFile(fileName,file_open_always,_FL_);
		Assert(data)

		dword count = data->Size()/sizeof(Item);

		for( dword i = 0 ; i < count ; i++ )
		{
			Item &item = xmlzs[xmlzs.Add()];

			data->Read(&item,sizeof(item));

			item.a = 1;
		}

		Check(xmlzs,xmlPath,"*.xmlz",false);

		///////////////////////////

		array<Item> items(_FL_,32);

		fileName = verPath;	fileName += "mod.ver";

		IFile *libs = storage->OpenFile(fileName,file_open_always,_FL_);
		Assert(libs)

		count = libs->Size()/sizeof(Item);

		for( dword i = 0 ; i < count ; i++ )
		{
			Item &item = items[items.Add()];

			libs->Read(&item,sizeof(item));

			item.a = 1;
		}

		if( Check(items,"modules","*.dll")) // если изменились библиотеки
		{
			for( int j = 0 ; j < xmlzs ; j++ )
			{
				Item &item = xmlzs[j];

				item.f = 1; // переэкспортить миссию
			}
		}

		libs->SetPos(0); libs->Truncate();

		for( int j = 0 ; j < items ; j++ )
		{
			Item &item = items[j];

			if( item.a == 0 )
				libs->Write(&item,sizeof(item));
		}

		RELEASE(libs)

		//////////////////////////////////

		SaveData();

		//////////////////////////////////

		init = false;
	}

	virtual void PreRelease()
	{
		RELEASE(data)
	}

//	virtual void StartFrame(float dltTime) {}
//	virtual void   EndFrame(float dltTime) {}

	virtual void SetCharacter(Character *p)
	{	
		chr = p;
		if(!chr)
		{
			animation = null;
		}
	}

	virtual IAnimation *SetAnimation(const char *name)
	{
		Assert(name)

		if( chr )
		{
			IAnimation *ani = chr->animation->AddAnimation(name,name);

			if( ani )
			{
				chr->animation->SetAnimation(name);

				animation = chr->GetAnimation();

				return animation;
			}
		}

		return null;
	}

	virtual IGMXScene *GetModel()
	{
		if( chr )
		{
			return chr->GetModel();
		}

		return null;
	}

	virtual void Begin()
	{
		if( chr )
		{
			chr->physics->GetCharacterMatrix(mat);

			//////// Disable sound events ////////

			IAnimation *ani = chr->GetAnimation();
			AnimationStdEvents *events = chr->events;

			if (ani)
			{
				ani->DelEventHandler(events,(AniEvent)&AnimationStdEvents::PlaySound, "Snd");

				//// Ragdoll must die

				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::AniToRagdoll,"AniToRgd");
				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::RagdollToAni,"RgdToAni");
		
				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::ArmRagdoll,"ArmRagdoll");

				//// не надо мусорить

				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::FlyBodyPart,"FlyBodyPart");
				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::DropItems,"DropItems");
				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::DropItem,"DropItem");

				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::DropBonus,"DropBonus");

				//// как страшно жить

				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::Kick,"Kick");

				////

				ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::CharacterCommand,"ChrCommand");
			}
		}
	}

	virtual void Rotate(float ay)
	{
		if( chr )
		{
			Matrix rot(true);
				   rot.BuildRotateY(ay);

			mat = rot*mat;

			chr->physics->SetAy(mat.vz.GetAY(chr->physics->GetAy()));
		}
	}

	virtual void Reset()
	{
		if( chr )
		{
			chr->animation->SetAnimation(animation,0.0f);

			chr->physics->SetPos(mat.pos);
			chr->physics->SetAy(mat.vz.GetAY(chr->physics->GetAy()));
		}
	}

	virtual void LockEvents(bool lock)
	{
		if( chr )
		{
			IAnimation *ani = chr->GetAnimation();

			Assert(animation)
			if( ani != animation )
			{
				chr->animation->SetAnimation(ani = animation,0.0f);
			}

			AnimationStdEvents *events = chr->events;

			if( lock )
			{
				chr->events->_DelHandlers(ani);

				chr->events->ResetCollapser();
				chr->events->ResetParticles();
			}
			else
			{
				if (ani)
				{
					chr->events->_AddHandlers(ani);

					////

					ani->DelEventHandler(events,(AniEvent)&AnimationStdEvents::PlaySound,"Snd");

					//// Ragdoll must die

					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::AniToRagdoll,"AniToRgd");
					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::RagdollToAni,"RgdToAni");
			
					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::ArmRagdoll,"ArmRagdoll");

					//// не надо мусорить

					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::FlyBodyPart,"FlyBodyPart");
					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::DropItems,"DropItems");
					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::DropItem,"DropItem");

					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::DropBonus,"DropBonus");

					//// как страшно жить

					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::Kick,"Kick");

					////

					ani->DelEventHandler(chr->events,(AniEvent)&CharacterAnimationEvents::CharacterCommand,"ChrCommand");
				}
			}
		}
	}

	virtual bool AddName(const char *xmlName, const char *xmlPath, const char *verPath)
	{
		if( init )
		{
			InitService(xmlPath,verPath);
		}

		for( int i = 0 ; i < xmlzs ; i++ )
		{
			Item &item = xmlzs[i];

			if( string::IsEqual(xmlName,item.name))
			{
				if( item.a == 0 )
				{
					if( item.f ) // нужно экспортить
					{
						item.f = 0;

						SaveData();

						return true;
					}
					else
						return false;
				}
				else // миссия была удалена
					return true;
			}
		}

		return true;
	}

};

CREATE_SERVICE(PreviewService, 100)

class CharacterAI_Preview : public CharacterController
{
public:

	CharacterAI_Preview(Character &ch, const char *name) : CharacterController(ch,name)
	{
		IPreviewService *service = (IPreviewService *)api->GetService("PreviewService");
		Assert(service)

		service->SetCharacter(&ch);
	}

	virtual ~CharacterAI_Preview()
	{
		IPreviewService *service = (IPreviewService *)api->GetService("PreviewService");
		Assert(service)

			service->SetCharacter(null);
	}

	virtual bool AllowToBeMoved() { return false; }

};

DeclareCharacterController(CharacterAI_Preview, "Preview", 255)

#endif
