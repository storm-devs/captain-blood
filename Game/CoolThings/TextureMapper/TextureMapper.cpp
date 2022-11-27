#include "TextureMapper.h"

static IRender *_render = null;

const TextureMapper::EnumElement TextureMapper::enumElements[] =
{
	{ConstString("Single texture"), 1, 1},
	{ConstString("Map 2x1"), 2, 1},
	{ConstString("Map 2x2"), 2, 2},
	{ConstString("Map 4x2"), 4, 2},
	{ConstString("Map 4x4"), 4, 4},
	{ConstString("Map 8x4"), 8, 4},
	{ConstString("Map 8x8"), 8, 8},
	{ConstString("Map 16x8"), 16, 8},
	{ConstString("Map 16x16"), 16, 16},
	{ConstString("Map 32x16"), 32, 16},
	{ConstString("Map 32x32"), 32, 32}
};

struct DebugInfo
{
	struct Info
	{
		  const char *name; bool debug;

		IBaseTexture *diff;
		IBaseTexture *spec;

		 Info() : diff(null),spec(null) {}
		~Info()
		{
			RELEASE(diff)
			RELEASE(spec)
		}

		void update(bool deb, const char *d, const char *s = null)
		{
			debug = deb;

			if( !diff || string::NotEqual(diff->GetName(),d))
			{
				RELEASE(diff) diff = string::NotEmpty(d) ? _render->CreateTexture(_FL_,d) : null;

				if( diff && diff == _render->getWhiteTexture()) diff = null;
			}

			if( !s ) return;

			if( !spec || string::NotEqual(spec->GetName(),s))
			{
				RELEASE(spec) spec = string::NotEmpty(s) ? _render->CreateTexture(_FL_,s) : null;

				if( spec && spec == _render->getWhiteTexture()) spec = null;
			}
		}
	};

	Info info[6];

	enum Item {flys = 0,drops,clouds,trails,water,stains};

	void update(Item item, bool deb, const char *d, const char *s = null)
	{
		info[item].update(deb,d,s);
	}

	ShaderId id; IVariable *tex; IBaseTexture *white;

	bool use; float scale;

	DebugInfo(IRender &render)
	{
		info[flys  ].name = "Flyers";
		info[drops ].name = "Rain";
		info[clouds].name = "Smoke";
		info[trails].name = "Trails";
		info[water ].name = "Water";
		info[stains].name = "Stains";

		render.GetShaderId("Circular",id);

		tex = render.GetTechniqueGlobalVariable("CircularTexture",_FL_);

		white = render.getWhiteTexture();
		white->AddRef();

		_render = &render;
	}

	~DebugInfo()
	{
		tex = null; RELEASE(white)
	}

	void drawBox(float x, float w, float y, float h, int index)
	{
		float buf[] = {
			x + w,y - h,0.0f,0.0f,0.0f,0.4f,
			x	 ,y - h,0.0f,0.0f,0.0f,0.4f,
			x + w,y	   ,0.0f,0.0f,0.0f,0.4f,
			x	 ,y	   ,0.0f,0.0f,0.0f,0.4f};

		if( tex )
			tex->SetTexture(white);

		_render->DrawPrimitiveUP(id,PT_TRIANGLESTRIP,2,buf,6*sizeof(float));

		//////////////////////////////////////////////////

		const RENDERVIEWPORT &vp = _render->GetViewport();

		_render->Print(
			0.5f*(x + 1.0f)*vp.Width ,
			0.5f*(1.0f - y)*vp.Height,0xff606060,"%d",index);
	}

	void drawPlace(int index, float x, float y, float w, float h, const TextureMapper::Place &p)
	{
		drawBox(
			x + w*fmodf(p.t,1.0f),w*p.w,
			y - h*fmodf(p.u,1.0f),h*p.h,index);
	}

	void draw(const TextureMapper::Info &mapping)
	{
		const RENDERVIEWPORT &vp = _render->GetViewport();

		float k = _render->GetWideScreenAspectWidthMultipler();

		float kx = 1.0f/vp.Width;
		float ky = 1.0f/vp.Height;

		float x = 18;
		float y = 17;

		float h = 0;

		for( int i = 0 ; i < 6 ; i++ )
		{
			const Info &item = info[i];

			if( item.diff && item.debug )
			{
				float cx = scale*item.diff->GetWidth ();
				float cy = scale*item.diff->GetHeight();

				if( x > 20 && x + cx > vp.Width )
				{
					y += h + 20; x = 18; h = cy;
				}
				else
				{
					if( h < cy )
						h = cy;
				}

				_render->Print(x,y,-1,item.name);

				y += 20;

				float x_off = -1.0f + 2*x*kx*k;
				float y_off =  1.0f - 2*y*ky;

				float l = 0.0f; float r = l + 1.0f;
				float t = 0.0f; float b = t + 1.0f;

				float d = 2*cx*kx*k;
				float c = 2*cy*ky;

				switch( i )
				{
					case flys:
						drawPlace(0,x_off,y_off,d,c,mapping.Flys  ::place);
						break;

					case drops:
						drawPlace(0,x_off,y_off,d,c,mapping.Drops ::place[0]);
						drawPlace(1,x_off,y_off,d,c,mapping.Drops ::place[1]);
						break;

					case clouds:
						drawPlace(0,x_off,y_off,d,c,mapping.Clouds::place[0]);
						drawPlace(1,x_off,y_off,d,c,mapping.Clouds::place[1]);
						break;

					case trails:
						drawPlace(0,x_off,y_off,d,c,mapping.Trails::place);
						break;

					case water:
						drawPlace(0,x_off,y_off,d,c,mapping.Water ::place);
						break;

					case stains:
					{
						const TextureMapper::Stains &st = mapping;

						TextureMapper::Place p;

						p.w = 1.0f/st.cols;
						p.h = 1.0f/st.rows;

						const TextureMapper::Stains::Loc &bl = st.blood;

						for( int j = 0, k = st.cols*bl.y + bl.x ; j < bl.n ; j++, k++ )
						{
							p.t = p.w*(k%st.cols);
							p.u = p.h*(k/st.cols);

							drawPlace(j,x_off,y_off,d,c,p);
						}

						const TextureMapper::Stains::Loc &so = st.soot;

						for( int j = 0, k = st.cols*so.y + so.x ; j < so.n ; j++, k++ )
						{
							p.t = p.w*(k%st.cols);
							p.u = p.h*(k/st.cols);

							drawPlace(j,x_off,y_off,d,c,p);
						}
					}
				}

				float buf[] = {
					x_off + d,y_off - c,0.0f,r,b,1.0f,
					x_off	 ,y_off - c,0.0f,l,b,1.0f,
					x_off + d,y_off	   ,0.0f,r,t,1.0f,
					x_off	 ,y_off	   ,0.0f,l,t,1.0f};

				if( tex )
					tex->SetTexture(item.diff);

				_render->DrawPrimitiveUP(id,PT_TRIANGLESTRIP,2,buf,6*sizeof(float));

				y -= 20;

				x += cx + 20;
			}
		}
	}
};

TextureMapper::~TextureMapper()
{
	DELETE(info)
}

bool TextureMapper::Create(MOPReader &reader)
{
	if( EditMode_IsOn())
	{
		info = NEW DebugInfo(Render());

		SetUpdate(&TextureMapper::Redraw,ML_LAST + 1000);
	}
	else
		info = null;

	InitParams(reader);

	return true;
}

bool TextureMapper::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void _cdecl TextureMapper::Redraw(float dltTime, long level)
{
	if( EditMode_IsSelect() == false )
		return;

	if( info->use )
		info->draw(mapping);
}

void TextureMapper::GetMapSize(const ConstString & spec, float &cx, float &cy)
{
	for(dword i = 0; i < ARRSIZE(enumElements); i++)
	{
		if(spec == enumElements[i].name)
		{
			cx = enumElements[i].col;
			cy = enumElements[i].row;
			break;
		}
	}
}

void TextureMapper::InitParams(MOPReader &reader)
{
	Flys   &flys   = mapping;
	Drops  &drops  = mapping;
	Clouds &clouds = mapping;
	Trails &trails = mapping;
	Water  &water  = mapping;
	Stains &stains = mapping;

	////////////////////////////

	flys.diff = reader.String().c_str();;

	float cx;
	float cy;

	GetMapSize(reader.Enum(),cx,cy);

	int x = reader.Long();
	int y = reader.Long();
	int w = reader.Long();
	int h = reader.Long();

	flys.place.set(x/cx,y/cy,w/cx,h/cy);

	float beg = reader.Float();
	float end = reader.Float();

	if( beg > end )
	{
		Swap(beg,end);

		LogicDebugError("Invalid fading values.");
	}

	flys.fadeBeg = beg;
	flys.fadeEnd = end;

	flys.size = reader.Float()*0.01f;

	/////////////////////////////

	drops.diff = reader.String().c_str();;
	drops.spec = reader.String().c_str();;

	GetMapSize(reader.Enum(),cx,cy);

	reader.Array();

	x = reader.Long();
	y = reader.Long();
	w = reader.Long();
	h = reader.Long();

	drops.place[0].set(x/cx,y/cy,w/cx,h/cy);

	x = reader.Long();
	y = reader.Long();
	w = reader.Long();
	h = reader.Long();

	drops.place[1].set(x/cx,y/cy,w/cx,h/cy);

	//////////////////////////////

	clouds.diff = reader.String().c_str();;

	GetMapSize(reader.Enum(),cx,cy);

	reader.Array();

	x = reader.Long();
	y = reader.Long();
	w = reader.Long();
	h = reader.Long();

	clouds.place[0].set(x/cx,y/cy,w/cx,h/cy);

	x = reader.Long();
	y = reader.Long();
	w = reader.Long();
	h = reader.Long();

	clouds.place[1].set(x/cx,y/cy,w/cx,h/cy);

	//////////////////////////////

	trails.diff = reader.String().c_str();;
	trails.spec = reader.String().c_str();;

	GetMapSize(reader.Enum(),cx,cy);

	x = reader.Long();
	y = reader.Long();
	w = reader.Long();
	h = reader.Long();

	trails.place.set(x/cx,y/cy,w/cx,h/cy);

	/////////////////////////////

	water.diff = reader.String().c_str();;

	GetMapSize(reader.Enum(),cx,cy);

	x = reader.Long();
	y = reader.Long();
	w = reader.Long();
	h = reader.Long();

	water.place.set(x/cx,y/cy,w/cx,h/cy);

	water.scale = reader.Float();

	//////////////////////////////

	stains.diff = reader.String().c_str();;
	stains.spec = reader.String().c_str();;

	GetMapSize(reader.Enum(),cx,cy);

	stains.cols = (int)cx;
	stains.rows = (int)cy;

	stains.blood.x = reader.Long();
	stains.blood.y = reader.Long();
	stains.blood.n = reader.Long();

	stains.blood.delay = reader.Float();
	stains.blood.in	   = reader.Float();
	stains.blood.res   = reader.Float();
	stains.blood.out   = reader.Float();

	if( stains.blood.in  < 0.1f ) stains.blood.in  = 0.1f;
	if( stains.blood.out < 0.1f ) stains.blood.out = 0.1f;

	stains.blood.SetAlpha(reader.Float());

	stains.blood.power = reader.Float();
	stains.blood.value = reader.Float();

	stains.blood.power = Lerp(1.0f,32.0f,stains.blood.power);

	stains.blood.scale = reader.Float();

	stains.soot.x = reader.Long();
	stains.soot.y = reader.Long();
	stains.soot.n = reader.Long();

	stains.soot.delay = reader.Float();
	stains.soot.in	  = reader.Float();
	stains.soot.res	  = reader.Float();
	stains.soot.out   = reader.Float();

	if( stains.soot.in  < 0.1f ) stains.soot.in  = 0.1f;
	if( stains.soot.out < 0.1f ) stains.soot.out = 0.1f;

	stains.soot.SetAlpha(reader.Float());

	stains.soot.power = reader.Float();
	stains.soot.value = reader.Float();

	stains.soot.power = Lerp(1.0f,32.0f,stains.soot.power);

	stains.soot.scale = reader.Float();

	////////////////////////

	bool deb_flys	= reader.Bool();
	bool deb_drops	= reader.Bool();
	bool deb_clouds = reader.Bool();
	bool deb_trails = reader.Bool();
	bool deb_water	= reader.Bool();
	bool deb_stains = reader.Bool();

	bool debug = reader.Bool();

	float scale = reader.Float();

	Activate(reader.Bool());

	////////////////////////

//	if( EditMode_IsOn())
	{
		MGIterator &it = GroupIterator(GroupId('M','p','p','U'),_FL_);

		while( it.Get())
		{
			it.ExecuteEvent();
			it.Next();
		}

		it.Release();
	}

	///////////////////////

	if( EditMode_IsOn())
	{
		const Flys   &flys   = mapping;
		const Drops  &drops  = mapping;
		const Clouds &clouds = mapping;
		const Trails &trails = mapping;
		const Water  &water  = mapping;
		const Stains &stains = mapping;

		info->update(DebugInfo::flys  ,deb_flys	 ,  flys.diff);
		info->update(DebugInfo::drops ,deb_drops , drops.diff, drops.spec);
		info->update(DebugInfo::clouds,deb_clouds,clouds.diff);
		info->update(DebugInfo::trails,deb_trails,trails.diff,trails.spec);
		info->update(DebugInfo::water ,deb_water , water.diff);
		info->update(DebugInfo::stains,deb_stains,stains.diff,stains.spec);

		info->use	= debug;
		info->scale = scale;
	}
}

void TextureMapper::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	Unregistry(GroupId('T','e','M','r'));

	if( isActive )
	{
		Registry(GroupId('T','e','M','r'));
	}

	LogicDebug(isActive ? "Activate" : "Deactivate");
}

TextureMapper *TextureMapper::GetMapper(IMission &mis)
{
	MGIterator &it = mis.GroupIterator(GroupId('T','e','M','r'),_FL_);

	for( TextureMapper *p ; p = (TextureMapper *)it.Get(); it.Next())
	{
		if( p->IsActive())
		{
			break;
		}
	}

	it.Release();

	return p;
}

MOP_BEGINLISTCG(TextureMapper, "TextureMapper", '1.00', 0, "TextureMapper", "Default")

	MOP_ENUMBEG("MapType")
		for(dword i = 0; i < ARRSIZE(TextureMapper::enumElements); i++)
		{
			MOP_ENUMELEMENT(TextureMapper::enumElements[i].name.c_str())
		}
	MOP_ENUMEND

	MOP_GROUPBEG("Flyers")

		MOP_GROUPBEG("Source")

			MOP_STRING("Diffuse|Flyers|","")

			MOP_ENUM("MapType","Type|Flyers|")

		MOP_GROUPEND()

		MOP_GROUPBEG("Face")

			MOP_LONGEX("Start column|Flyers|",0,0,31)
			MOP_LONGEX("Start row|Flyers|",0,0,31)
			MOP_LONGEX("Width|Flyers|",1,1,32)
			MOP_LONGEX("Height|Flyers|",1,1,32)

		MOP_GROUPEND()

		MOP_GROUPBEG("Fading")

			MOP_FLOATEX("Near distance",14.0f,0.0f,1000.0f)
			MOP_FLOATEX("Far distance",24.0f,0.0f,1000.0f)

		MOP_GROUPEND()

		MOP_FLOATEX("Size (centimeters)|Flyers|",1.3f,0.0f,100.0f)

	MOP_GROUPEND()

	MOP_GROUPBEG("LocalEffect")

		MOP_GROUPBEG("Rain")

			MOP_GROUPBEG("Source")

				MOP_STRING("Diffuse|Rain|","")
				MOP_STRING("Normal|Rain|","")

				MOP_ENUM("MapType","Type|Rain|")

			MOP_GROUPEND()

			MOP_ARRAYBEG("Face|Rain|",2,2)

				MOP_LONGEX("Start column",0,0,31)
				MOP_LONGEX("Start row",0,0,31)
				MOP_LONGEX("Width",1,1,32)
				MOP_LONGEX("Height",1,1,32)

			MOP_ARRAYEND

		MOP_GROUPEND()

		MOP_GROUPBEG("Smoke")

			MOP_GROUPBEG("Source")

				MOP_STRING("Diffuse|Smoke|","")

				MOP_ENUM("MapType","Type|Smoke|")

			MOP_GROUPEND()

			MOP_ARRAYBEG("Face|Smoke|",2,2)

				MOP_LONGEX("Start column",0,0,31)
				MOP_LONGEX("Start row",0,0,31)
				MOP_LONGEX("Width",1,1,32)
				MOP_LONGEX("Height",1,1,32)

			MOP_ARRAYEND

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_GROUPBEG("Trails")

		MOP_GROUPBEG("Source")

			MOP_STRING("Diffuse|Trails|","")
			MOP_STRING("Normal|Trails|","")

			MOP_ENUM("MapType","Type|Trails|")

		MOP_GROUPEND()

		MOP_GROUPBEG("Face")

			MOP_LONGEX("Start column|Trails|",0,0,31)
			MOP_LONGEX("Start row|Trails|",0,0,31)
			MOP_LONGEX("Width|Trails|",1,1,32)
			MOP_LONGEX("Height|Trails|",1,1,32)

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_GROUPBEG("WaterTrack")

		MOP_GROUPBEG("Source")

			MOP_STRING("Diffuse|Water|","")

			MOP_ENUM("MapType","Type|Water|")

		MOP_GROUPEND()

		MOP_GROUPBEG("Face")

			MOP_LONGEX("Start column|Water|",0,0,31)
			MOP_LONGEX("Start row|Water|",0,0,31)
			MOP_LONGEX("Width|Water|",1,1,32)
			MOP_LONGEX("Height|Water|",1,1,32)

		MOP_GROUPEND()

		MOP_FLOATEX("Scale|Water|",1.0f,0.5f,5.0f)

	MOP_GROUPEND()

	MOP_GROUPBEG("Stains")

		MOP_GROUPBEG("Source")

			MOP_STRING("Diffuse|Stains|","")
			MOP_STRING("Normal|Stains|","")

			MOP_ENUM("MapType","Type|Stains|")

		MOP_GROUPEND()

		MOP_GROUPBEG("Blood")

			MOP_GROUPBEG("Face set")

				MOP_LONGEX("Start column|Blood|",0,0,31)
				MOP_LONGEX("Start row|Blood|",0,0,31)

				MOP_LONGEX("Sequence length|Blood|",1,1,1024)

			MOP_GROUPEND()

			MOP_GROUPBEG("Time")

				MOP_FLOATEX("Show delay|Blood|",0.2f,0.0f,5.0f)
				MOP_FLOATEX("Fade in|Blood|",0.25f,0.0f,5.0f)
				MOP_FLOATEX("Live|Blood|",15.0f,5.0f,30.0f)
				MOP_FLOATEX("Fade out|Blood|",1.5f,0.0f,5.0f)

			MOP_GROUPEND()

			MOP_GROUPBEG("Color")

				MOP_FLOATEX("Alpha|Blood|",0.7f,0.3f,1.0f)

			MOP_GROUPEND()

			MOP_GROUPBEG("Specular")

				MOP_FLOATEX("Power|Blood|",1.0f,0.0f,1.0f)
				MOP_FLOATEX("Value|Blood|",1.0f,0.0f,1.0f)

			MOP_GROUPEND()

			MOP_FLOATEX("Scale|Blood|",1.0f,0.5f,5.0f)

		MOP_GROUPEND()

		MOP_GROUPBEG("Soot")

			MOP_GROUPBEG("Face set")

				MOP_LONGEX("Start column|Soot|",0,0,31)
				MOP_LONGEX("Start row|Soot|",0,0,31)

				MOP_LONGEX("Sequence length|Soot|",1,1,1024)

			MOP_GROUPEND()

			MOP_GROUPBEG("Time")

				MOP_FLOATEX("Show delay|Soot|",0.2f,0.0f,5.0f)
				MOP_FLOATEX("Fade in|Soot|",0.6f,0.0f,5.0f)
				MOP_FLOATEX("Live|Soot|",15.0f,5.0f,30.0f)
				MOP_FLOATEX("Fade out|Soot|",1.5f,0.0f,5.0f)

			MOP_GROUPEND()

			MOP_GROUPBEG("Color")

				MOP_FLOATEX("Alpha|Soot|",0.7f,0.3f,1.0f)

			MOP_GROUPEND()

			MOP_GROUPBEG("Specular")

				MOP_FLOATEX("Power|Soot|",1.0f,0.0f,1.0f)
				MOP_FLOATEX("Value|Soot|",1.0f,0.0f,1.0f)

			MOP_GROUPEND()

			MOP_FLOATEX("Scale|Soot|",1.0f,0.5f,5.0f)

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_GROUPBEG("Debug")

		MOP_GROUPBEG("Draw")

			MOP_BOOL("Flyers", true)
			MOP_BOOL("Rain"	 , true)
			MOP_BOOL("Smoke" , true)
			MOP_BOOL("Trails", true)
			MOP_BOOL("Water" , true)
			MOP_BOOL("Stains", true)

		MOP_GROUPEND()

		MOP_BOOLC("Draw", false, "Вывести схему разметки")

		MOP_FLOATEX("Scale",0.5f,0.25f,1.0f)

	MOP_GROUPEND()

	MOP_BOOLC("Active", true, "Использовать таблицу")

MOP_ENDLIST(TextureMapper)
