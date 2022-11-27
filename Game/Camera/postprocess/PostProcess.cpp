#include "PostProcess.h"

CREATE_SERVICE(PostProcessService, /*100*/120)

static IConsole *console = null;

static ShaderId PostPolygon_id;

struct Vertex
{
	float x,y; dword c;
};

PostProcessService:: PostProcessService() : nodes(_FL_,16)
{
	pRS = null;

	count = 0;

	empty = true;

	trace = false;
	key	  = false;

	dflag = true;
	enabled = true;
}

PostProcessService::~PostProcessService()
{
}

bool PostProcessService::Init()
{
	pRS = (IRender *)api->GetService("DX9Render");

	//Added: by Joker
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * pEngineIni = fs->SystemIni();
	bool bPostProcessEnabled = false;
	if (pEngineIni)
	{
		bPostProcessEnabled = pEngineIni->GetLong("Render", "PostProcess", 1) != 0;
		RELEASE(pEngineIni);
	}
/*
#ifdef _XBOX
	//JOKER - пока фильтры не пофикшенны я их отключил
	//т.к. версия для X360 непоказабельная...
	bPostProcessEnabled = false;
#endif
*/
	if( !bPostProcessEnabled )
	{
		return true;
	}

	console = (IConsole *)api->GetService("Console");




	//filter_motion.Init();
	//filter_motion.UserInit();

	//pRS->AddRenderFilter(&filter_motion,1.0f);
	//filter_motion.Enable(false);

	//filter_effect.Init();
	//filter_effect.UserInit();

	//pRS->AddRenderFilter(&filter_effect,1.0f);
	//filter_effect.Enable(false);

	//filter_blur.Init();
	//filter_blur.UserInit();

	//pRS->AddRenderFilter(&filter_blur,1.0f);
	//filter_blur.Enable(false);

	//filter_bw.Init();
	//filter_bw.UserInit();

	//pRS->AddRenderFilter(&filter_bw,1.0f);
	//filter_bw.Enable(false);

	filter_splash.Init();
	filter_splash.UserInit();

	pRS->AddRenderFilter(&filter_splash,1.0f);
	filter_splash.Enable(true);

	api->SetStartFrameLevel(this,Core_DefaultExecuteLevel);
	api->SetEndFrameLevel(this,Core_DefaultExecuteLevel);

	pRS->GetShaderId("PostPolygon",PostPolygon_id);

	if( console )
	{
		console->RegisterCommand("PostDraw","Draw posteffects."		,this,(CONSOLE_COMMAND)&PostProcessService::Console_EnableDraw);
		console->RegisterCommand("PostList","Draw posteffects list.",this,(CONSOLE_COMMAND)&PostProcessService::Console_EnableList);
	}

	return true;
}

void PostProcessService::StartFrame(float dltTime)
{
	if( !empty )
	{
		for( int i = 0 ; i < nodes ; i++ )
		{
			nodes[i].reset();
		}

		weight = 0.0f;

		count = 0;
		empty = true;
	}

	filter_splash.Update(dltTime);
}

void PostProcessService::EndFrame(float dltTime)
{
//	#ifndef _XBOX

//	if( !console || console->IsHided())
	{
		#ifndef _XBOX

		bool pressed = api->DebugKeyState('P',VK_CONTROL);

		if( pressed && !key )
		{
			trace = !trace;
		}

		key = pressed;

		#endif

		if( pRS && !empty && trace && enabled )
		{
		/*	float y = 10.0f;

			for( int i = 0 ; i < nodes ; i++, y += 15.0f )
			{
				const Node &node = nodes[i];

				if( !!node )
				{
					pRS->Print(10.0f,y,-1,"%s by \"%s\"",node.effect,node.object);
				}
			}*/

			const RENDERVIEWPORT &wp = pRS->GetFullScreenViewPort_3D();

			float cx = (float)wp.Width;
			float cy = (float)wp.Height;

			const float ch = 17.0f;

			float up = 1.0f - (20.0f + count*ch)/cy;

		//	const dword c = 0x80000000;
			const dword c = 0x70000000;
			Vertex v[] = {
				0.0f,  up,c,
				0.0f,1.0f,c,
				0.5f,  up,c,
				0.5f,1.0f,c};

			pRS->DrawPrimitiveUP(PostPolygon_id,PT_TRIANGLESTRIP,2,v,sizeof(Vertex));

		//	const float max_w =  53.0f;
		//	const float sum_w = 116.0f;
			const float max_w =  27.0f;
			const float sum_w =  63.0f;

			float k = weight/sum_w;

			if( k > 1.0f )
				k = 1.0f;

			float y = cy - 10.0f - ch;

			for( int i = 0 ; i < nodes ; i++, y -= ch )
			{
				const Node &node = nodes[i];

				if( !!node )
				{
				//	pRS->Print(10.0f,y,0xff00ff00,"%s by \"%s\"",node.effect,node.object);

					float w = node.weight/max_w*k;

					if( w > 1.0f )
						w = 1.0f;

					Color c(0xff000000);

					if( w < 0.5f )
					{
						c.g = 1.0f;	c.r = w*2.0f;
					}
					else
					{
						c.r = 1.0f; c.g = 1.0f - (w - 0.5f)*2.0f;
					}

					pRS->Print(10.0f,y,c.GetDword(),"%1.2f %s by \"%s\"",node.k,node.effect,node.object.c_str());
				}
			}
		}
	}

//	#endif

	enabled = dflag;
}

void PostProcessService::FilterNotify(const char *effect, const char *object, float weight, float k)
{
	for( int i = 0 ; i < nodes ; i++ )
	{
		Node &cur = nodes[i];

		if( cur.weight == weight )
			break;
	}

	if( i >= nodes )
		this->weight += weight;

	Node *node = null;

	for( int i = 0 ; i < nodes ; i++ )
	{
		Node &cur = nodes[i];

		if( !cur )
		{
			node = &cur;
			break;
		}
	}

	if( !node )
		 node = &nodes[nodes.Add()];

	node->set(effect,object,weight,k);

	count++;
	empty = false;
}


/*IBlur *PostProcessService::GetBlurFilter()
{
	return (IBlur *)&filter_blur;
}

IMotionBlur *PostProcessService::GetMotionBlurFilter()
{
	return (IMotionBlur *)&filter_motion;
}

BlackAndWhite *PostProcessService::GetBlackFilter()
{
	return &filter_bw;
}

MotionEffect *PostProcessService::GetMotionEffectFilter()
{
	return &filter_effect;
}*/

Splash *PostProcessService::GetSplashFilter()
{
	return &filter_splash;
}

void _cdecl PostProcessService::Console_EnableDraw(const ConsoleStack &params)
{
	dflag = !dflag;
}

void _cdecl PostProcessService::Console_EnableList(const ConsoleStack &params)
{
	trace = !trace;
}
