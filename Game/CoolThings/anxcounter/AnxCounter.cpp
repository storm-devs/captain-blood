#include "AnxCounter.h"

#include "..\..\..\Common_h\Animation.h"
#include "..\..\..\Common_h\AnimationNativeAccess.h"

CREATE_CLASS(AnxCounter)

bool AnxCounter::Init()
{
	IFileService *storage = (IFileService *)api->GetService("FileService");
	Assert(storage);

	IAnimationService *service = (IAnimationService *)api->GetService("AnimationService");
	Assert(service);

	IFile *data = storage->OpenFile("animation.txt",file_create_always,_FL_);
	Assert(data)

	char buf[1024];

	////

//	IPackFile *pack = storage->LoadPack("resource\\animation\\animation.pkx",_FL_);

	array<IPackFile *> packs(_FL_);

	IFinder* package = storage->CreateFinder("resource","a*.pkx",
		find_all_files_no_mirrors,_FL_);

	for( dword i = 0 ; i < package->Count() ; i++ )
	{
		strcpy_s(buf,sizeof(buf),package->Path(i));
		strcat_s(buf,sizeof(buf),package->Name(i));

		packs.Add(storage->LoadPack(buf,_FL_));
	}

	////

	IFinder* finder = storage->CreateFinder("resource","*.anx",
		find_all_files_no_mirrors,_FL_);

	IAnimationScene *scene = service->CreateScene(_FL_);
	Assert(scene)

	dword count = finder->Count();

	api->Trace("===================================== AnxCounter::BEGIN\n");

	IFinder* folders = storage->CreateFinder("resource\\missions","animation",
		find_all_files_no_mirrors|find_folders,_FL_);

	array<IMirrorPath *> mirr(_FL_);

	for( dword i = 0 ; i < folders->Count() ; i++ )
	{
		strcpy_s(buf,sizeof(buf),folders->Path(i));
		strcat_s(buf,sizeof(buf),folders->Name(i));

	//	api->Trace(buf);

		mirr.Add(storage->CreateMirrorPath(buf,"resource\\animation",_FL_));
	}

	folders->Release();

	bool r; int len;

	float t_sum = 0.0f;
	dword c_sum = 0;

	dword b_sum = 0;

	dword f_sum = 0;

	len = sprintf_s(buf,sizeof(buf),"%36s ; frames ;  bad ; good ;     time\r\n\r\n","name");
	data->Write(buf,len);

	for( dword i = 0 ; i < count ; i++ )
	{
		const char *name = finder->Name(i);

		IAnimation *ani = scene->Create(name,_FL_);

		if( ani )
		{
		//	api->Trace("    %s",name);

			float total = 0.0f;
			dword clips = 0;

			dword badcn = 0;

			dword frame = 0;

			AGNA_GlobalInfo gi;

			r = ani->GetNativeGraphInfo(gi);
			Assert(r)

		//	len = sprintf_s(buf,sizeof(buf),"%32s: %3d nodes\r\n",name,gi.numNodes);
		//	data->Write(buf,len);

			for( dword node = 0 ; node < gi.numNodes ; node++ )
			{
				AGNA_NodeInfo ni(node);

				r = ani->GetNativeGraphInfo(ni);
				Assert(r)

			//	len = sprintf_s(buf,sizeof(buf),"%32s%d\r\n","",ni.numClips);
			//	data->Write(buf,len);

				for( dword clip = 0 ; clip < ni.numClips ; clip++ )
				{
					AGNA_ClipInfo ci(node,clip);

					r = ani->GetNativeGraphInfo(ci);
					Assert(r)

				//	len = sprintf_s(buf,sizeof(buf),"%36s%2.2f (%5d at %2.2ffps)\r\n","",ci.frames/ci.fps,ci.frames,ci.fps);
				//	data->Write(buf,len);
				//	len = sprintf_s(buf,sizeof(buf),"%36s%3d/%5.2f = %5.2f\r\n","",ci.frames,ci.fps,ci.frames/ci.fps);
				//	data->Write(buf,len);

					if( ci.fps < 1.0f || ci.frames < 5 )
					{
						badcn++;
					}
					else
					{
						total += ci.fps > 0.0f ? ci.frames/ci.fps : 0.0f;
						clips++;
					}

					frame += ci.frames;
				}
			}

		//	len = sprintf_s(buf,sizeof(buf),"%36s: [%4d] %4d %8.2f\r\n",name,badcn,clips,total);
			len = sprintf_s(buf,sizeof(buf),"%36s ; %6d ; %4d ; %4d ; %8.2f\r\n",name,frame,badcn,clips,total);
			data->Write(buf,len);

			t_sum += total;
			c_sum += clips;

			b_sum += badcn;

			f_sum += frame;

			ani->Release();
		}
		else
		{
		//	api->Trace("    [BAD] %s",name);
		}
	}

	dword mm = dword(t_sum/60);
	dword hh = mm/60;

	mm = mm%60;

	float ss = t_sum - floorf(t_sum/60)*60;

//	len = sprintf_s(buf,sizeof(buf),"\r\n%36s  [%4d] %4d %8.2f (%2d:%2d:%5.2f)\r\n","",b_sum,c_sum,t_sum,hh,mm,ss);
	len = sprintf_s(buf,sizeof(buf),"\r\n%36s ; %6d ; %4d ; %4d ; %8.2f (%2d:%2d:%5.2f)\r\n","",f_sum,b_sum,c_sum,t_sum,hh,mm,ss);
	data->Write(buf,len);

	api->Trace("\n===================================== AnxCounter::END");

	data->Release();

	finder->Release();

	package->Release();

	scene->Release();

	////

	for( int i = 0 ; i < mirr ; i++ )
		mirr[i]->Release();

	mirr.DelAll();

	////

//	if( pack )
//		pack->Release();

	for( int i = 0 ; i < packs ; i++ )
		if( packs[i] )
			packs[i]->Release();

	packs.DelAll();

	////

	api->Exit();

	return true;
}
