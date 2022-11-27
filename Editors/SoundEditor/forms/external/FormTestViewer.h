#pragma once

#include "..\FormControlBase.h"
#include "..\..\..\..\common_h\Sound.h"

class FormPolygonList;

class FormTestViewer : public FormControlBase
{
	struct FileHeader
	{
		union
		{
			struct
			{
				struct Camera
				{
					Vector pos;
					Vector ang;
				};

				Camera cam;

				struct Map
				{
					int x;
					int y;

					float scale; bool fixed;
				};

				Map map;

				int itemsCount;

				int gizmoIndex;
			};

			char data[64];
		};
		//Empty constructor
		FileHeader() {}
	};

	struct FileItem
	{
		enum {sound = 0,model}; enum {name_len = 128};

		union
		{
			struct
			{
				Vector pos;
				Vector ang;

				char name[name_len];

				dword type;

				////////////

				dword color;

				float vol;

				float r1,r2;
			};

			char data[192];
		};
		//Empty constructor
		FileItem() {}
	};

	/////////////

	struct Entity
	{
		string name;

		Vector pos;
		Vector off;

		virtual void GetBoundBox(Vector &a, Vector &b) const = 0;
		virtual void GetBoundSphere(Vector &p, float &r) const = 0;

		virtual void Draw(float time) const = 0;

		virtual void Click() const = 0;

		virtual void SaveTo(FileItem &item) const = 0;

		virtual ~Entity()
		{
		}
	};

	struct Source : public Entity
	{
		dword color;

		float vol;

		float r1,r2;

		ISound3D *sound;

		void GetBoundBox(Vector &a, Vector &b) const
		{
			a = pos - r2 - 5.0f;
			b = pos + r2 + 5.0f;
		}

		void GetBoundSphere(Vector &p, float &r) const
		{
			p = pos; r = 3.5f;
		}

		void Draw(float time) const
		{
			const float r = 0.3f;
			const float R = 3.5f;

			options->render->DrawSphere(pos,r,color);

			options->render->DrawSphereGizmo(pos,R,0xffff0000,0);

			if( sound )
			{
				float dr = sinf(time*5.0f)*0.04f;

				if( sound->IsPlay() == false )
					dr = 0.0f;

				options->render->DrawSphereGizmo(pos,r + vol*(R - r) + dr,-1,0);

				options->render->DrawSphereGizmo(pos,r1,0xffffff00,0);
				options->render->DrawSphereGizmo(pos,r2,0xff00ff00,0);

				sound->SetPosition(pos);
			}
		}

		void Click() const
		{
			if( sound /*&& sound->IsPlay() == false*/ )
			{
				sound->Play();
			}
		}

		void SaveTo(FileItem &item) const
		{
			item.pos = pos;
			item.ang = 0.0f;

			strcpy_s(item.name,FileItem::name_len,name);

			item.type = FileItem::sound;

			item.color = color;

			item.vol = vol;

			item.r1 = r1;
			item.r2 = r2;
		}

		~Source()
		{
			RELEASE(sound)
		}
	};

	struct Geometry : public Entity
	{
		Vector ang;

		IGMXScene *scene;

		void GetBoundBox(Vector &a, Vector &b) const
		{
			Vector p; float r;

			GetBoundSphere(p,r);

			a = p - r;
			b = p + r;
		}

		void GetBoundSphere(Vector &p, float &r) const
		{
			if( scene )
			{
				const GMXBoundBox &b = scene->GetBound();

				p = 0.5f*(b.vMin + b.vMax);

				r = p.x - b.vMin.x;

				if( r < p.y - b.vMin.y )
					r = p.y - b.vMin.y;

				if( r < p.z - b.vMin.z )
					r = p.z - b.vMin.z;
			}
			else
			{
				p = pos; r = 3.5f;
			}
		}

		void Draw(float time) const
		{
			if( scene )
			{
				Matrix m; m.pos = pos;

				scene->SetTransform(m);
				scene->Draw();
			}
			else
			{
				options->render->DrawSphere(pos,3.5f,0xff0000ff);
			}
		}

		void Click() const
		{
		}

		void SaveTo(FileItem &item) const
		{
			item.pos = pos;
			item.ang = ang;

			strcpy_s(item.name,FileItem::name_len,name);

			item.type = FileItem::model;
		}

		~Geometry()
		{
			RELEASE(scene)
		}
	};

	struct Map
	{
		GUIRectangle r;

		struct Button
		{
			GUIRectangle r;
		};

		Button dec,inc;
	};

	Map m_map; bool dragMap;

	////////////////////////

	struct DrawElem
	{
		const Entity *e; int index; bool in;

		GUIRectangle rect;

		float z; Vector4 p;

		GUIRectangle rt;

		DrawElem()
			: e(null),in(false)
		{
		}

		DrawElem(const Entity *enti, const Matrix &view, const Matrix &proj, const GUIRectangle &_rect);

		void Draw();

		static int round(float x)
		{
			if( x > 0 )
				return int(x + 0.5f);
			else
				return int(x - 0.5f);
		}
	};

public:

	FormTestViewer(GUIControl *parent, const GUIRectangle &rect, FormPolygonList &list);
	virtual ~FormTestViewer();

public:

	void OnMouseRightUp();

	//Рисование
	virtual void OnDraw(const GUIRectangle &rect);
	//Начало перемещения, возвранить true, если начинать тащить. elementPivot графический центр элемента
	virtual bool OnDragBegin(const GUIPoint &mousePos, GUIPoint &elementPivot);
	//Перемещение текущего элемента в данную точку
	virtual void OnDragMoveTo(const GUIPoint &elementPosition);
	//Прекратить операцию перетаскивания
	virtual void OnDragEnd();
	//Щелчёк левой кнопкой мыши
	virtual void OnMouseLeftClick(const GUIPoint &mousePos);
	//Щелчёк правой кнопкой мыши
	virtual void OnMouseRightClick(const GUIPoint &mousePos);
	//Дельта от колеса
	virtual void OnMouseWeel(const GUIPoint &mousePos, long delta);

public:

	void _cdecl OnCreateSound(GUIControl *sender);
	void _cdecl OnCreateModel(GUIControl *sender);

	void _cdecl OnDeleteItems(GUIControl *sender);

private:

	void LoadHeader();
	void SaveHeader();

	void LoadData(int itemsCount);
	void SaveData();

	void SaveItem(int index, const Entity &e);

private:

	void DrawScene();

	void UpdateBox();
	void UpdateFocus();

private:

	void NextObject();

private:

	FormPolygonList &m_list;

	Vector m_min;
	Vector m_max;

	Matrix m_view;
	Matrix m_flat;

	bool m_look;
	bool m_lookFixed;

	bool m_dec;
	bool m_inc;

	bool m_right;

	float m_rot_x;
	float m_rot_y;

	float m_scale;
	float m_time;

	float m_delay;

	int m_mouseX;
	int m_mouseY;

	int m_startX;
	int m_startY;

	ISoundScene *m_scene;

	///////////////

	class TransformGizmo *m_gizmo;

	bool m_overGizmo;

	bool dragGizmo;

	Entity *dragObject; int dragObjectIndex;
	Entity *currObject; int currObjectIndex;

	///////////////

	IFile *m_file;

	string m_filePath;

	bool loading;

	/////////////

	bool tabDown;

public:

	void AddSound(const char *name, const Vector &pos, dword color, float vol, float r1, float r2);

	void AddModel(const char *name, const Vector &pos,
									const Vector &ang);

	void Cleanup(); // почистить все, что связано со списком

};
