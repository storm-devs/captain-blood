#include "resourceselect.h"
#include "..\..\common_h\fileservice.h"
#include "..\..\common_h\mission.h"

#include "..\NodesPool.h"


#define DEF_WIN_WIDTH 640
#define DEF_WIN_HEIGHT 480

extern string StartDirectory;
extern IRender * pRS; // Рендер


extern TreeNodesPool* globalNodesPool;

string BrowserCurrentSelectedDir  = "";
IGMXService* GMXService = NULL;
extern IMission* miss;

TResourceSelectorWindow::TResourceSelectorWindow () : GUIWindow (null, 0, 0, DEF_WIN_WIDTH, DEF_WIN_HEIGHT),
																										  ExtInfo(_FL_),
																											SubstratedPath(_FL_)
{


	dwWinWidth = pRS->GetScreenInfo3D().dwWidth - 10;
	dwWinHeight = pRS->GetScreenInfo3D().dwHeight - 10;

	SetWidth(dwWinWidth);
	SetHeight(dwWinHeight);


	string PakPath;
#ifndef NO_TOOLS
	miss->EditorGetPackPath(PakPath);
#endif

	if (PakPath.Size() > 0)
	{
		if (PakPath[PakPath.Size()-1] == '\\') PakPath.Delete(PakPath.Size()-1, 1);
		for (dword n = PakPath.Size()-1; n >= 0; n--)
		{
			if (PakPath[n] == '\\')
			{
				PakPath.Delete(0, n+1);
				break;
			}
		}
	}

	//PakPath = string ("Resource\\Missions\\") + PakPath;

	IParticleService* pService = (IParticleService*)api->GetService("ParticleService");
	pParticleManager = pService->CreateManager(PakPath.GetBuffer());

	IAnimationService * as = (IAnimationService *)api->GetService("AnimationService");
	anims = as->CreateScene(_FL_);



	angle = 0.0f;
	bPopupStyle = true;
	previewname = "";
	GMXService = (IGMXService*)api->GetService ("GMXService");
	PreviewTexture = NULL;
	PreviewScene = NULL;
	PreviewSystem = NULL;

	ObjectName = "";
	Caption = "Resource browser";
	pFont->SetName("arialcyrsmall");
	bAlwaysOnTop = true;
	SetScreenCenter();

	TreeView1 = NEW GUITreeView (this, 10, 5, 250, dwWinHeight-45);
	TreeView1->FontColor = 0xFF000000;
	TreeView1->pFont->SetName("arialcyrsmall");
	TreeView1->SetImagesArray ("gtree");
	TreeView1->bDragAndDrop = false;
	TreeView1->OnChange = (CONTROL_EVENT)&TResourceSelectorWindow::OnSelectResource;
	TreeView1->OnDblClick = (CONTROL_EVENT)&TResourceSelectorWindow::OnOpenResource;

  t_OnSelect = NEW GUIEventHandler;  

	Preview = NEW GUIPanel (this, 270, 5, dwWinWidth-280, dwWinHeight-45);
	Preview->OnBeforeDraw = (CONTROL_EVENT)&TResourceSelectorWindow::RenderPreview;

	btnOK = NEW GUIButton (this, 10, dwWinHeight-30, 100, 22);
	btnOK->pFont->SetName("arialcyrsmall");
	btnOK->Caption = "Ok";
	btnOK->Glyph->Load("ok");
	btnOK->OnMousePressed = (CONTROL_EVENT)&TResourceSelectorWindow::OnOpenResource;
	btnOK->FlatButton = true;

	btnCANCEL = NEW GUIButton (this, 120, dwWinHeight-30, 100, 22);
	btnCANCEL->pFont->SetName("arialcyrsmall");
	btnCANCEL->Caption = "Cancel";
	btnCANCEL->Glyph->Load("cancel");
	btnCANCEL->OnMousePressed = (CONTROL_EVENT)&TResourceSelectorWindow::Close;
	btnCANCEL->FlatButton = true;




	ReadExtInfo ();
	ScanForResources ();

	if (!BrowserCurrentSelectedDir.IsEmpty())
	{
		GUITreeNode* my_sel_node = TreeView1->FindItem(BrowserCurrentSelectedDir);
		if (my_sel_node)
		{
			ExpandAndGoToParent (my_sel_node);
			TreeView1->SetSelectedNode(my_sel_node);
		}
	}

}

void TResourceSelectorWindow::ExpandAndGoToParent (GUITreeNode* node)
{
	node->Expanded = true;
	if (node->Parent) ExpandAndGoToParent (node->Parent);
}

TResourceSelectorWindow::~TResourceSelectorWindow ()
{
	
	delete t_OnSelect;

	if (PreviewTexture)
	{
		PreviewTexture->Release();
		PreviewTexture = NULL;
	}
	if (PreviewScene)
	{
		PreviewScene->Release();
		PreviewScene = NULL;
	}

	if (PreviewSystem)
	{
		PreviewSystem->Release();
		PreviewSystem = NULL;
	}

	if (pParticleManager) pParticleManager->Release();
	pParticleManager = NULL;

	if(anims) anims->Release();
	anims = null;
}


bool TResourceSelectorWindow::FuncCompare (GUITreeNode* const &a1, GUITreeNode* const &a2)
{
	if (a1->Tag > a2->Tag) return true;
	if (a1->Tag == a2->Tag)
	{
		if (crt_stricmp(a1->GetText(), a2->GetText()) < 0) return true;
	}
	return false;
}

void TResourceSelectorWindow::AddScanDir (const char* dirName)
{

	GUITreeNode* NewNode = globalNodesPool->CreateNode();
//	NewNode->Text = ;
	NewNode->SetText(dirName);
	TreeView1->Items->Add(NewNode);
	NewNode->Tag = 1;
	NewNode->Image->Load("folder");



	string sDir = "resource\\";
	sDir += dirName;
	sDir += "\\";


	string croped;
	string filename;
	string name, path, ext;

	IFileService* fs = (IFileService*)api->GetService("FileService");
	Assert(fs);
	IFinder* finder = fs->CreateFinder(sDir.c_str(), "*.*", find_all_files_folder | find_no_files_from_packs, _FL_);
	Assert(finder);

	string resourcesFullPath;
	fs->BuildPath("resource\\", resourcesFullPath);

	for (dword n = 0; n < finder->Count(); n++)
	{
		filename = string(finder->FilePath(n)).GetRelativePath (resourcesFullPath);
		
		path.GetFilePath(filename);
		ext.GetFileExt(filename);
		name.GetFileName(filename);

		croped = filename;
		for (dword i = 0; i < croped.Size(); i++)
		{
			if (croped[i] == '\\') 
			{
				croped.Delete (i, croped.Size()-i);
				break;
			}
		}

		GUITreeNodes* nodestoadd = TreeView1->Items;

		//if (crt_stricmp (croped.GetBuffer(), "missions") == 0) continue;


		if (!path.IsEmpty())
		{
			path.Delete(path.Size()-1, 1);
			GUITreeNode* node = TreeView1->FindItem(path.GetBuffer());
			Assert (node != NULL);
			nodestoadd = &node->Childs;
		}

		GUITreeNode* already_exist_node = TreeView1->FindItem(filename);
		if (already_exist_node) continue;

		GUITreeNode* NewNode = globalNodesPool->CreateNode();
		NewNode->SetText(name);
		//NewNode->Text = ;
		nodestoadd->Add(NewNode);

		if (finder->IsMirror(n) && !finder->IsFolder(n))
		{
			NewNode->bUseCustomColor = true;
			NewNode->Color = 0xFF0000FF;

		}

		if (finder->IsFolder(n))
		{
			NewNode->Tag = 1;
			NewNode->Image->Load("folder");
		} else
		{
			if (!ext.IsEmpty())
			{
				ExtensionInfo* info = GetExtInfo (ext);
				if (info)
				{
					NewNode->Image->Load(info->image);
				} else
				{
					NewNode->Image->Load("icon");
				}

			}
			NewNode->Tag = 0;
		}
	}

	finder->Release();
}

void TResourceSelectorWindow::ScanForResources ()
{	
	AddScanDir("Animation");
	AddScanDir("Models");
	AddScanDir("Particles");
	AddScanDir("Textures");

	AddScanDir("Cameras");
	AddScanDir("Videos");

	TreeView1->Sort(FuncCompare);
}

void TResourceSelectorWindow::ReadExtInfo ()
{
	static char TempSection[64];

	IFileService * fs = (IFileService *)api->GetService("FileService");
	IIniFile * EngineIni = fs->SystemIni();

	for (int n = 0; n < 100; n++)
	{
		crt_snprintf (TempSection, 64, "Ext %02d", n);
		const char * tmpExt = EngineIni->GetString("mission_editor", TempSection, null);
		if (!tmpExt) break;

		crt_snprintf (TempSection, 64, "Img %02d", n);
		const char * tmpIcon = EngineIni->GetString("mission_editor", TempSection, "icon");

		ExtensionInfo NewExt;
		NewExt.ext = tmpExt;
		NewExt.image = tmpIcon;
		ExtInfo.Add(NewExt);
	}

	for (n = 0; n < 100; n++)
	{
		crt_snprintf (TempSection, 64, "SubPath %02d", n);
		const char * _spath = EngineIni->GetString("substracted_path", TempSection, null);
		if (!_spath) break;

		string spath = _spath;
		spath.Lower();
		if (spath[spath.Size() - 1] != '\\') spath += "\\";
		SubstratedPath.Add(spath);
	}

	EngineIni->Release(); EngineIni = null;

}

TResourceSelectorWindow::ExtensionInfo* TResourceSelectorWindow::GetExtInfo (const char* ext)
{
	for (int n = 0; n < ExtInfo; n++)
	{
		if (ExtInfo[n].ext == ext) return &ExtInfo[n];
	}

	return NULL;
}


void _cdecl TResourceSelectorWindow::OnSelectResource (GUIControl* sender)
{
	GUITreeNode* sNode = TreeView1->GetSelectedNode();
	if (!sNode)  return;

	ObjectName = sNode->GetFullPath();


	if (sNode->Tag == 0)
	{
		if (ObjectName[ObjectName.Size() - 1] == '\\') ObjectName.Delete(ObjectName.Size() - 1, 1);
	}

	SubstractPath (ObjectName);

	BeginPreview ();


	
}

void TResourceSelectorWindow::SubstractPath (string& fullname)
{
	fullname.Lower();
	for (int n  = 0; n < SubstratedPath; n++)
	{
		long res = fullname.FindSubStr(SubstratedPath[n]);
		if (res == 0)
		{
			fullname.Delete(0, SubstratedPath[n].Size());
		}
	}
}

void _cdecl TResourceSelectorWindow::OnOpenResource (GUIControl* sender)
{
	GUITreeNode* sNode = TreeView1->GetSelectedNode();

	
	if (!sNode)  return;

	BrowserCurrentSelectedDir = sNode->GetFullPath();

	t_OnSelect->Execute(this);
	Close (this);
}

void TResourceSelectorWindow::BeginPreview ()
{
	angle = 0.0f;
	if (previewname == ObjectName) return;

	previewname = ObjectName;
	if (PreviewTexture)
	{
		PreviewTexture->Release();
		PreviewTexture = NULL;
	}
	if (PreviewScene)
	{
		PreviewScene->Release();
		PreviewScene = NULL;
	}

	if (PreviewSystem)
	{
		PreviewSystem->Release();
		PreviewSystem = NULL;
	}

	string ext;
	ext.GetFileExt(ObjectName);
	if (ext == "gmx")
	{
		PreviewScene = GMXService->CreateGMX(ObjectName, anims, pParticleManager, NULL);
	}

	if (ext == "txx")
	{
		PreviewTexture = pRS->CreateTexture(_FL_, ObjectName);
	}

	if (ext == "xps")
	{
		string sParticleName;;
		sParticleName.GetFileName(ObjectName);
		PreviewSystem = pParticleManager->CreateParticleSystemEx(sParticleName.c_str(), _FL_);

		//if (!PreviewSystem) api->Trace("Can't preview psys '%s'", ObjectName.GetBuffer());
	}
}

void _cdecl TResourceSelectorWindow::RenderPreview (GUIControl* sender)
{
	if ((!PreviewScene) && (!PreviewTexture) && (!PreviewSystem)) return;

	angle += 1.6f * api->GetDeltaTime();
	cliper.Push();
	RENDERVIEWPORT saved_vp = pRS->GetViewport();

	GUIRectangle rect = Preview->GetScreenRect();

	
	RENDERVIEWPORT ViewPort;
	ViewPort.X = rect.x;
	ViewPort.Y = rect.y;
	ViewPort.Width = rect.w;
	ViewPort.Height = rect.h;
	ViewPort.MinZ = 0.0f;
	ViewPort.MaxZ = 1.0f;
	pRS->SetViewport(ViewPort);

	pRS->SetPerspective(1.0f, (float)ViewPort.Width, (float)ViewPort.Height);
	pRS->Clear(0, NULL, CLEAR_STENCIL | CLEAR_TARGET |CLEAR_ZBUFFER, 0xFF878787, 1.0f, 0);
	

	if (PreviewScene)
	{
		Vector center = PreviewScene->GetBoundSphere().vCenter;
		float rad = PreviewScene->GetBoundSphere().fRadius;

		float camposx = sin(angle) * rad * 1.5f;
		float camposz = cos(angle) * rad * 1.5f;

		pRS->SetCamera(center-Vector (camposx, -rad, camposz), center, Vector (0.0f, 1.0f, 0.0f));
		PreviewScene->Draw();
	}

	if (PreviewTexture)
	{
		pRS->Clear(0, NULL, CLEAR_STENCIL | CLEAR_TARGET |CLEAR_ZBUFFER, 0x00000030, 1.0f, 0);


		pRS->SetCamera(Vector (2.0f, 0.0f, 0.0f), Vector (0.0f), Vector (0.0f, 1.0f, 0.0f));

		RS_RECT Rect;
		Rect.vPos = Vector (0.0f);
		Rect.fSizeX = 1.0f;
		Rect.fSizeY = 1.0f;
		Rect.fAngle = 0.0f;
		Rect.dwColor = 0xFFFFFFFF;
		Rect.dwSubTexture = 0;
		
		pRS->DrawRects(PreviewTexture,&Rect, 1);



  }

	if (PreviewSystem)
	{
		pRS->SetCamera(Vector (15.0f, 2.0f, 15.0f), Vector (0.0f), Vector (0.0f, 1.0f, 0.0f));
		PreviewSystem->SetTransform(Matrix());
		pParticleManager->Execute(api->GetDeltaTime());
		pRS->SetWorld(Matrix());
		pParticleManager->Execute(api->GetDeltaTime());
		pParticleManager->DrawAllParticles();
		//pRS->DrawSphere(Vector(0.0f), 1.0f);
	}

	
	pRS->SetViewport(saved_vp);
	cliper.Pop();
	
}