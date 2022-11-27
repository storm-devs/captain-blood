#pragma once

#include "..\FormControlBase.h"

class PreviewAnimation;

class FormTimeLine : public FormControlBase
{
	enum Consts
	{
		c_node_h = 40,
		c_evnt_h = 16,
		c_cursor = 15,
		c_meter  = 20,
		c_scroll = 20
	};

	struct Node
	{
		ExtNameStr name;

		int index;		// индекс анимационного нода
		int isEnable;	// можно ли расставлять евенты

		int frames;

		float fps;
		float frameLen;

		float t;
		float w;

		float width;

		int prev;
		int next;

		bool valid;		// ссылаемся на существующий анимационный нод
	};

	struct Events
	{
		GUIRectangle r;
	};

	struct Cursor
	{
		GUIRectangle r;
	};

	struct Meter
	{
		GUIRectangle r;
	};

	struct Scroller
	{
		GUIRectangle r;
		GUIRectangle t;

		float page;
		float size;

		float scale;

		float pos;

		void update()
		{
			if( size <= page )
			{
				t.w = r.w;
				t.h = c_scroll;

				t.x = r.x;
				t.y = r.y;
			}
			else
			{
				if( size == 0 )
				{
					t.w = r.w;
					t.h = c_scroll;

					t.x = r.x;
					t.y = r.y;
				}
				else
				{
					t.w = int(r.w*(page/size) + 0.5);
					t.h = c_scroll;

					t.x = r.x + int((r.w - t.w)*pos/(size - page) + 0.5f);
					t.y = r.y;
				}
			}
		}

		void alignPos()
		{
			float r = 0.5f/scale;

			pos -= fmodf(pos + r,r + r);
		}

		float scroll(float delta)
		{
			if( size <= page )
				return 0.0f;

			float prev = pos;

			pos -= delta;

			alignPos();

			if( pos < 0.0f )
				pos = 0.0f;

			if( pos > size - page )
				pos = size - page;

			update();

			return pos - prev;
		}

		float setSize(float val)
		{
			float prev = pos;

			size = val;

			if( pos + page > size )
				pos = size - page;

			if( pos < 0.0f )
				pos = 0.0f;

			update();

			return prev - pos;
		}

		void setPage(float val)
		{
			page = val;

			if( pos + page > size )
				pos = size - page;

			if( pos < 0.0f )
				pos = 0.0f;

			update();
		}

		void setPos(float val)
		{
			pos = val;

			alignPos();

			if( pos < 0.0f )
				pos = 0.0f;

			if( pos + page > size )
				pos = size - page;

			update();
		}
	};

	template <class Key, class Val> class _map
	{
		array<pair<Key,Val>> items;

	public:

		_map() : items(_FL_)
		{
		}

		void DelAll()
		{
			items.DelAll();
		}

		int Find(const Key &key) const
		{
			for( int i = 0 ; i < items ; i++ )
			{
				if( key == items[i].first)
					return i;
			}

			return -1;
		}

		Val &operator [](const Key &key)
		{
			int index = Find(key);

			if( index < 0 )
				index = items.Add(pair<Key,Val>(key,Val()));

			return items[index].second;
		}

		Key &GetKey(int index)
		{
			Assert(index >= 0)
			Assert(index < items)

			return items[index].first;
		}

		Val &GetVal(int index)
		{
			Assert(index >= 0)
			Assert(index < items)

			return items[index].second;
		}

		void Del(int index)
		{
			Assert(index >= 0)
			Assert(index < items)

			int last = items - 1;

			if( index < last )
			{
				items[index] = items[last];
			}

			items.DelIndex(last);
		}

		operator int() const
		{
			return items;
		}

	};

public:

	FormTimeLine(GUIControl *parent, const GUIRectangle &rect, PreviewAnimation *preview);
	virtual ~FormTimeLine();

protected:
	//Рисование
	virtual void OnDraw(const GUIRectangle &screenRect);
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

protected:

	void _cdecl OnIgnoreElement(GUIControl *sender);
	void _cdecl OnDeleteElement(GUIControl *sender);

	int indexNode;
	int indexE;

	int round(float x)
	{
		return int(x + 0.5f);
	}

private:

	void UpdateFrame();

	void UpdateOffset();

	void UpdatePlay();

private:

	void PrepareForSoundPlay();

	void UpdateButtons();

private:

	dword GetNodeFrames(const Node &node)
	{
		dword frames = node.frames;

		if( node.next >= 0 )
		{
			const Node &next = nodes[node.next];

			float df = node.frameLen;

			int count = int((next.t - node.t + df*0.5f)/df);

			if( count > node.frames )
				count = node.frames;

			frames = count;
		}

		return frames;
	}

	bool Goto(const Node &node, float blend = 0.0f);

	void GotoIdle(float blend = 0.0f);

	void SetFrame(float frame);

	void ani_Pause(bool enable);

private:

	void CheckEvents(const Node &node, float prevFrame, float currFrame);

	void  StopEvents(const Node &node)
	{
		const array<ProjectAnimation::Event> &events = anime->GetNode(node.index).events;

		for( int i = 0 ; i < events ; i++ )
		{
			anime->EventSoundStop(node.index,i);
		}
	}

	void UpdateEvents();

	void SelectEvent(const ProjectAnimation::Event *e, int node = -1, int index = -1);

	void UpdateSize(Node &node);

	void ScrollToCursor();

	void SetFocus(const Node *node, int index, bool reset = true);

	void  MoveCursor(float t, const Node &node, float &cursFrame, int &cursNode, bool align = false);

	void CheckCursor();

	void Trim(const Node &node);

	void DeleteNode(int index);

	int GetFreeNode();

	void Expand();
	void Shift(int index, float off);

	void StepNodeL();
	void StepNodeR();

	void InsertNode(Node &node, int index, int after = -1);
	void RemoveNode(Node &node, int index);

	void ClearNodes();

	float DrawNode(const GUIRectangle &rect, const Node &node, bool sel = false);

	struct Place
	{
		int index; bool in;
		int e;
	};

	bool FindElement(float t, int y, Place &place);

	void UpdatePrevNext(float t, bool left);

	void ExportNodes(); // обновить список нодов в проекте

private:

	PreviewAnimation *preview;

	bool editNodes;
	bool editEvents;

	Node *dragNode; int dragIndex; int dragNodeNext; float dragNodeNextOffset; float nodeBase;

	int mouseOff;

	array<Node> nodes;

	int nodesHead;
	int nodesTail;

	int nodesFree;

	_map<int,int> nodesMap;

	Scroller *dragThumb;

	Scroller scr;

	bool autoScroll;

	float offset; float dragOffset;

	bool scroll_l;
	bool scroll_r;

	const ProjectAnimation::Event *dragEvent; int dragEventIndex; Node *curNode; float evenBase;

	Events eve;
	Cursor cur;

	Meter *dragMeter; float metBase; int metPos;

	Meter met;

	Node *selNode;

	const ProjectAnimation::Event *selE; int selEIndex;

	int focusEventNode;
	int focusEventIndex;

	const ProjectAnimation::Event *focusEvent;

	GUIFont *smallFont;

	float cursor; float cursorBeg;

	Node *prevNode;
	Node *nextNode;

	UniqId animeId;
	int movieIndex;

	const Node *focus; int focusIndex;

	ProjectAnimation *anime;

	bool loading;

	int cursorNode;
	float cursorFrame;

	bool dragCursor;

	float cursorBase;

	const Node *animNode;

	float animTime;
	float animStep;

	float animPrev;		// время на предыдущем кадре

	IAnimation *ani;

	float blendTime;

	int mustSetFrame;	// установить позицию анимации с задеркой в несклько кадров

	bool animPause;		// находимя в режиме паузы
	bool animLocal;		// режим проигрывания одного выделенного нода

	bool initAnim;		// установка нода и кадра на старте при смене ролика

	bool initPlay;		// только что начали проигрывание

	bool moveLeft;		// сдвинулись на кадр влево

	bool moveCursor;	// перемещаем курсор вместе с нодом

	float cursorOffset;	// смещение курсора относительно смещаемого нода

	bool lockEvents;	// запретить отработку событий на время установки фрейма анимации

	////////////////

	bool ldown;
	bool rdown;

	bool ctl_c;

	////////////////

	int play_delay; bool play_local;

	////////////////

	UniqId lastID;		// евент, указанный в качестве шаблона

public:

	void _cdecl Begin(GUIControl *sender);
	void _cdecl End  (GUIControl *sender);

	void _cdecl NodeBegin(GUIControl *sender);
	void _cdecl NodeEnd  (GUIControl *sender);

	void _cdecl Prev(GUIControl *sender);
	void _cdecl Next(GUIControl *sender);

	void _cdecl PlayNode(GUIControl *sender);
	void _cdecl Play	(GUIControl *sender);

	void _cdecl Pause(GUIControl *sender);
	void _cdecl Stop (GUIControl *sender);
	
	void _cdecl UpdateSounds(GUIControl *sender);

public:

	GUIEventHandler onSelectEvent;

public:

	void EnableEditNodes (bool enable) { editNodes  = enable; }
	void EnableEditEvents(bool enable) { editEvents = enable; }

	void SetMovie(const UniqId & animationId, int indexMovie);

	void AddNode (int indexNode, bool isEnable = true, float offset = 0.0f);

	bool GetSelectedEvent(int &anxNode, int &index)
	{
		if( focusEvent )
		{
			anxNode = focus->index;
			index	= focusEventIndex;

			return true;
		}
		else
		{
			anxNode = -1;
			index	= -1;

			return false;
		}
	}

	void PlayEvent();
	void StopEvent();

};
