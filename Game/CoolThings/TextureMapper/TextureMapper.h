#pragma once

#include "..\..\..\Common_h\Mission.h"

class TextureMapper : public MissionObject
{
public:

	struct EnumElement
	{
		ConstString name;
		float col;
		float row;
	};

	struct Place
	{
		float t,u;
		float w,h;

		void set(float t, float u, float w, float h)
		{
			this->t = t;
			this->u = u;
			this->w = w;
			this->h = h;
		}

		void reset()
		{
			t = 0.0f;
			u = 0.0f;
			w = 0.0f;
			h = 0.0f;
		}
	};

	struct Flys
	{
		const char *diff;

		Place place;

		float fadeBeg;
		float fadeEnd;

		float size;

		void reset()
		{
			diff = null;

			place.reset();

			fadeBeg = 14.0f;
			fadeEnd = 24.0f;

			size = 0.013f;
		}
	};

	struct Drops
	{
		const char *diff;
		const char *spec;

		Place place[2];

		void reset()
		{
			diff = null;
			spec = null;

			place[0].reset();
			place[1].reset();
		}
	};

	struct Clouds
	{
		const char *diff;

		Place place[2];

		void reset()
		{
			diff = null;

			place[0].reset();
			place[1].reset();
		}
	};

	struct Trails
	{
		const char *diff;
		const char *spec;

		Place place;

		void reset()
		{
			diff = null;
			spec = null;

			place.reset();
		}
	};

	struct Water
	{
		const char *diff;

		Place place; float scale;

		void reset()
		{
			diff = null;

			place.reset(); scale = 1.0f;
		}
	};

	struct Stains
	{
		const char *diff;
		const char *spec;

		int cols;
		int rows;

		struct Loc
		{
			int x,y;
			int n;

			float scale;

			float delay;
			float in;
			float res;
			float out;

			float alpha;
			dword rgba;

			float power;
			float value;

			void SetAlpha(float a)
			{
				alpha = a;
				
				Color c(0xffffffff);
				c.a = alpha;
				rgba = c;
			}

			void reset()
			{
				x = 0;
				y = 0; n = 1;

				scale = 1.0f;

				delay =  0.2f;
				in	  =  0.6f;
				res	  = 15.0f;
				out	  =  1.5f;

				SetAlpha(0.7f);

				power = 32.0f;
				value =  1.0f;
			}
		};

		Loc blood;
		Loc soot;

		void reset()
		{
			diff = null;
			spec = null;

			cols = 1;
			rows = 1;

			blood.reset();
			soot .reset();

			blood.in = 0.25f;
		}
	};

	struct Info :

		public Flys,
		public Drops,
		public Clouds,
		public Trails,
		public Water,
		public Stains {};

public:

	 TextureMapper() {}
	~TextureMapper();

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

private:

	void InitParams		(MOPReader &reader);

private:

	void GetMapSize(const ConstString & spec, float &cx, float &cy);

public:

	void Activate(bool isActive);

	MO_IS_FUNCTION(TextureMapper, MissionObject);

	void _cdecl Redraw(float dltTime, long level);

public:

	template <class T>
	static bool GetMapping(IMission &mis, T &info)
	{
		if( TextureMapper *p = GetMapper(mis))
		{
			info = p->mapping;

			return true;
		}
		else
		{
			info.reset();

			return false;
		}
	}

private:

	static TextureMapper *GetMapper(IMission &mis);

private:

	Info mapping;

	struct DebugInfo *info;

public:
	static const EnumElement enumElements[];
};
