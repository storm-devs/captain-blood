#pragma once

#ifndef _XBOX

	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>

#endif

#include "..\..\common_h\controls.h"
#include "ControlsIniParser.h"

#include "Force.h"

#define MAX_FILTERNAME	40

// структура для хранения разобранного с модификаторами контрола
struct Token
{
	string	name;			//
	bool	isInverted;		//
	bool	isReverse;		//
	float	scale;			//
	bool	addNext;		// == true если следующий токен идет через "+"; == false если через ","
};

class Controls;

// прослойка фильтрации raw-значений в соответствии с модификаторами
class ControlFilter
{
	struct ChainItem
	{
		long  filter;
		float scale;			// ограничитель максимального выдаваемого значения
		bool  isInverted;		// флаг инвертирования значения (value *= -1.0f)
		bool  isSumNext;
	};

	//string	  m_name;			// имя фильтра (используется для создания альясов)
	char		m_fname[MAX_FILTERNAME]; // имя фильтра (используется для создания альясов)
	bool		m_isMouseVorH;		
	bool		m_isMouseDeltaVorH;
	bool		m_isInverted;		// флаг инвертирования значения (value *= -1.0f)
	float		m_scale;			// ограничитель максимального выдаваемого значения

	long	  m_deviceIndex;	// индекс девайса (-1 если неиспольуется)
	long	  m_controlIndex;	// индекс контрола с которого брать raw значение (-1 если неиспольуется)

	array<ChainItem> m_Items;

	float m_curr;				// последнее  считанное значение
	float m_prev;				// предыдущее считанное значение

	int frameUpdated;

protected:
	Controls	* m_ctrl;

	void SetName(const char * name);

public:

	ControlFilter(Controls *ctrl, const char *name, float scale, bool isInverted);
	ControlFilter(Controls *ctrl = null);

	bool operator ==(const ControlFilter &other) const;
	bool operator !=(const ControlFilter &other) const
	{
		return !(*this == other);
	}

	const ControlFilter &operator =(const ControlFilter &);

	const char * GetName() const { return m_fname; }

	float GetValue();		// получить значение фильтра

	float GetValueLast();	// получить значение фильтра с предыдущего кадра

	void SetFilterParams(const char *name, float scale, bool isInverted)
	{
		SetName(name);
		//m_name		 = name;
		m_scale		 = scale;
		m_isInverted = isInverted;
	}

	void SetFilterParams(float scale, bool isInverted)
	{
		m_scale		 = scale;
		m_isInverted = isInverted;
	}
	
	void SetDataSource(long device, long control)
	{
		m_deviceIndex  = device;
		m_controlIndex = control;
	}

	void AddItem(long index, bool isInverted, float scale, bool isSum)
	{
		ChainItem item;
		
		item.filter		= index;

		item.isInverted = isInverted;
		item.scale		= scale;

		item.isSumNext	= isSum;
		
		m_Items.Add(item);
	}

	dword GetItemsCount() const
	{
		return m_Items.Size();
	}

	void SetItemSumFlag(dword index, bool isSumNext)
	{
		if( index < m_Items.Size())
			m_Items[index].isSumNext = isSumNext;
	}

public:

	friend class Controls;

	long m_last;

	ImagePlace m_place;
	ImagePlace m_place_inv;

};

// прослойка логических контролов (выдаются через IControls)
class GameControl : public ControlFilter
{
	friend class Controls;
private:
	int	m_curFrame;					// frame на котором обновлялся контрол

	ControlStateType m_stateType;	// состояние контрола
	float			 m_fValue;		// значение с плавающей точкой

	bool m_isReverse;
	bool m_isUpdated;	// был ли обновлен на текущем кадре
	bool m_enabled;		// включен/выключен
	bool m_locked;

public:

	void Lock()
	{
		ControlStateType st = GetState();

		if( st == CST_ACTIVE ||
			st == CST_ACTIVATED )
		{
			m_locked = true;
		}
	}

	GameControl(Controls *ctrl, const string &name, bool isReverse);
	GameControl();

	ControlStateType GetState()
	{
		if( !IsUpdated())
			Update();

		return m_stateType;
	}

	float GetValue()
	{
		if( !IsUpdated())
			Update();

		return m_fValue;
	}

	void Update();

	__inline void ResetUpdated()
	{
		m_isUpdated = false;
	}

	__inline bool IsUpdated() const;

	__inline void Enable(bool enable)
	{
		m_enabled = enable;
	}

};

struct Item;

class IDevice;
class JoystickManager;

class IRenderTarget;

//////////////////////////////////
// Controls service
//////////////////////////////////

class ControlsInstance;

class Controls : public IControlsService
{
	struct FakeForce
	{
		array<Vector> ls;
		array<Vector> rs;

		FakeForce() :
			ls(_FL_),
			rs(_FL_)
		{
		}
	};

	struct InstanceInfo
	{
		bool used;

		ControlsInstance *p;

		const char *file;
		long		line;
	};

	struct ControlGroup
	{
		ControlGroup() : inds(_FL_)
		{
		}

		string		name;	// имя группы
		bool		used;	// используется

		array<long>	inds;	// индексы GameControl'ов
	};

#ifndef _XBOX

	IDirectInput8 *m_DI;	// DirectInput - только на PC

	friend BOOL __stdcall   DeviceEnumer(LPCDIDEVICEINSTANCE  lpddi, LPVOID pvRef);
	friend BOOL __stdcall JoystickEnumer(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef);

	HWND  m_h;				// хендл главного окна
	POINT m_cursorPos;		// позиция курсора перед захватом мыши

#endif

	// группы контролов
	array<ControlGroup> m_groups;
	HashTable<long,128,64> m_tableGroups;
	// логические контролов с таблицей индексов для быстрого доступа
	mutable array<GameControl> m_gameControls;
	mutable HashTable<long,256,128> m_tableControls;
	// фильтры-модификаторы-альясы с таблицей индексов для быстрого доступа
	array<ControlFilter> m_controlFilters;
	HashTable<long,256,128> m_tableFilters;
	// девайсы
	array<IDevice*> m_devices;

	JoystickManager *m_joyMrg;		// менеджер джойстиков

	array<KeyDescr> m_Keys;			// буфер клавиш
	bool			m_lockMouse;	// лочить ли виндовый мышиный курсор
	ControlsIniParser		m_iniParser;

	bool			m_freeMouse;	// разлочить на время текущего кадра

	bool m_showCursor;				// состояние системного курсора
	bool m_clipCursor;

	bool m_prev_act;				// предыдущее состояние активности
	bool m_preserve;				// не трогать системный курсор

	int m_curFrame;

	////////////////////////////

	ICoreStorageFloat *m_vibrate;	// разрешить вибрацию джойпада

	bool m_forceVibrate; // форсировать вибрацию (в редакторе)

	struct Patcher
	{
		struct Group
		{
			struct Control
			{
				const char *name;
				int index;

				float val;	// оригинальное значение

				Control()
				{
				}

				Control(const char *alias)
				{
					name = alias;
				}
			};

			array<Control> controls;

			const char *name;
			ICoreStorageFloat *p;

			bool isFlag;	// признак переключателя

			Group()
				: controls(_FL_)
			{
			}

			Group(const char *variable, bool flag = false)
				: controls(_FL_)
			{
				name = variable; isFlag = flag;
			}
		};

		array<Group> groups;

		Patcher()
			: groups(_FL_)
		{
		}

		void Init  (Controls *service);
		void Update(Controls *service);

		void Release();
	};

	Patcher m_patcher;				// модификция контролов "на лету"

	///////////////////////////

	virtual bool Init();			// инициализация

	void BuildFilterChain(ControlFilter &aliasFilter, const array<Token> &tokens);

	array<Force> forces;			// действующие силы
	
	Force *GetFreeForce();
	void ApplyForces(float dltTime);

	IRenderTarget *target;			// текстура для отрисовки контролов

	int m_version;					// номер обновления текстуры

	IBaseTexture *images;			// загруженный текстурный атлас

	int m_locked;					// признак блокировки

	bool m_debug;

	array<InstanceInfo> m_inst;

	array<ForceData> m_forceList;
	FakeForce		 m_forceData; ForceData m_fakeData;

	htable<int> m_forceHash;

	string m_tempName;

	long m_mouseH;
	long m_mouseV;

	string m_profile;				// путь к файлу дефолтных профилей

	long CreateGameControl(const char* name, bool isReverse);
	long CreateControlFilter(const char* name, float scale, bool isReverse);
	long CreateControlGroup(const char* name);

public:

	void ReleaseForces(IControls *instance)
	{
		for( int i = 0 ; i < forces ; i++ )
		{
			Force &force = forces[i];

			if( force.busy )
			{
				if( force.instance == instance )
				{
					force.time = FORCE_STOP;

					force.instance = null;
					force.busy = false;
				}
			}
		}
	}

	void Reset()
	{
		for( int i = 0 ; i < m_gameControls ; i++ )
		{
			if( i != m_mouseH && i != m_mouseV )
				m_gameControls[i].Lock();
		}
	}

	Controls();
	virtual ~Controls();

	// вывод локализованной ошибки в лог
	void Error(long id, const char *errorEnglish);

	void CreateForceData();
	
	void ReadIni();

	void SetCurrentProfile(long player, bool single, long group, const char *name);

	void BuildImages();

	__inline int GetCurFrame() { return m_curFrame; }

	void AddAlias(const array<Token> &tokens);
	void AddControl(long group, const array<Token> &tokens);
	long AddGroup(const char *name);
	long AddFilter(const ControlFilter &filter);

	__inline ControlFilter &GetFilter (long index)	{ return m_controlFilters[index]; }

	__inline GameControl   &GetControl(long index)	{ return m_gameControls[index]; }

	long GetFilterNode(long index, bool &inverted);

	bool HandleFilter(int i, array<Item> &map);

	long FindFilterByName(const char *name);

	float GetDeviceValue(long deviceIndex, long controlIndex) const;

	//////////////////////////////////////////////////////////////////////////
	// Service
	//////////////////////////////////////////////////////////////////////////
	//Исполнение в начале кадра
	virtual void StartFrame(float dltTime);
	//Исполнение в конце кадра
	virtual void EndFrame  (float dltTime);
	
	//////////////////////////////////////////////////////////////////////////
	// IControls
	//////////////////////////////////////////////////////////////////////////

	//---------------------------------------------------------------------------
	// Работа с изображениями контролов
	//---------------------------------------------------------------------------
	// получить общую текстуру с изображениями контролов
	virtual IBaseTexture *GetControlsImage();

	virtual int GetControlsImageVersion();

	// получить координаты изображения контрола в общей текстуре
	virtual ImagePlace GetControlImagePlace(const char *control_name);

	//---------------------------------------------------------------------------
	// Управление контролами
	//---------------------------------------------------------------------------
	// найти контрол по его имени
	virtual long FindControlByName(const char *control_name) const;
	virtual long FindGroupByName  (const char *group_name);

	//---------------------------------------------------------------------------
	// управление группами контролов
	//---------------------------------------------------------------------------
	// включить/выключить группу контролов
	virtual void EnableControlGroup(long group_num, bool bEnable = true);
	virtual void EnableControlGroup(const char *group_name, bool bEnable = true);

	//---------------------------------------------------------------------------
	// Опрос и управление состоянием контролов
	//---------------------------------------------------------------------------
	// получить состояние активности контрола
	virtual ControlStateType GetControlStateType(long control_code) /*const*/;
	virtual ControlStateType GetControlStateType(const char *control_name) /*const*/;

	// получить значение контрола в формате с плавающей точкой
	virtual float GetControlStateFloat(long control_code) /*const*/;
	virtual float GetControlStateFloat(const char* control_name) /*const*/;

	// получить значение контрола в формате bool
	virtual bool GetControlStateBool(long control_code) /*const*/;
	virtual bool GetControlStateBool(const char* control_name) /*const*/;

	// разрешить/запретить обработку контролов
	virtual void Lock();
	virtual void Unlock();

	// получить состояние болкировки
	virtual bool Locked();

	virtual long LockCount();

	// получить алиас активного контрола
	virtual const char *GetPressedControl(const char *alias_name);
	virtual const char *GetActiveControl(const char *alias_name);
	virtual const char *GetReleasedControl(const char *alias_name);

	// обновить значение алиаса
	virtual const char *UpdateAlias(const char *alias_name, const char *name, bool buildImages = true);

	// загрузить профайл
	virtual bool LoadProfile(
		long player, bool single, long group, const char *name, bool reset = false, bool buildImages = true);
	
	// получить имя текущего профайла
	virtual const char *CurrentProfile(long player, bool single, long group);

	//----------------------------------------------------------------------------
	// Использование возможности Force feedback
	//----------------------------------------------------------------------------
	// получить интерфейс
	virtual IForce *CreateForce(const IControls *instance, const char *profile,
		bool autoDelete = true, long deviceIndex = 0);
	virtual IForce *CreateForce(array<Vector> &ls, array<Vector> &rs,
		bool autoDelete = true, long deviceIndex = 0);

	// получить количество групп
	virtual long GetControlGroupsQuantity() const;
	// выполнить системно-зависимую команду
	virtual void ExecuteCommand(InputSrvCommand &cmd);

	// работа с буфером клавиш
	void AddKey(const KeyDescr &desc);
	virtual unsigned int GetKeyBufferLength() const;
	virtual const KeyDescr *GetKeyBuffer() const;
	virtual void ClearKeyBuffer();

	// разлочить мышь на один кадр
	virtual void FreeMouse();

	//----------------------------------------------------------------------------
	// ControlsService
	//----------------------------------------------------------------------------

	// создать экземпляр
	virtual IControls *CreateInstance(const char *file, long line);

	////

	void RegInstance(ControlsInstance *p, const char *file, long line);
	void DelInstance(ControlsInstance *p);

};

/////////////////////////////////////////
// Controls instance
/////////////////////////////////////////

class ControlsInstance : public IControls
{
	Controls *m_base;

	bool m_active; bool IsActive() const { return m_active; }

public:

	ControlsInstance(Controls *base, const char *file, long line)
	{
		m_base = base;
		Assert(m_base)

		m_active = true;

		m_base->RegInstance(this,file,line);
	}

	//////////////////////////////////////////////////////////////////////////
	// IControls
	//////////////////////////////////////////////////////////////////////////

	//---------------------------------------------------------------------------
	// Работа с изображениями контролов
	//---------------------------------------------------------------------------
	// получить общую текстуру с изображениями контролов
	virtual IBaseTexture *GetControlsImage()
	{ return m_base->GetControlsImage(); }

	virtual int GetControlsImageVersion()
	{ return m_base->GetControlsImageVersion(); }

	// получить координаты изображения контрола в общей текстуре
	virtual ImagePlace GetControlImagePlace(const char *control_name)
	{ return m_base->GetControlImagePlace(control_name); }

	//---------------------------------------------------------------------------
	// Управление контролами
	//---------------------------------------------------------------------------
	// найти контрол по его имени
	virtual long FindControlByName(const char *control_name) const
	{ return m_base->FindControlByName(control_name); }
	virtual long FindGroupByName  (const char *group_name)	 const
	{ return m_base->FindGroupByName  (group_name); }

	//---------------------------------------------------------------------------
	// управление группами контролов
	//---------------------------------------------------------------------------
	// включить/выключить группу контролов
	virtual void EnableControlGroup(long group_num, bool bEnable = true)
	{ m_base->EnableControlGroup(group_num,bEnable); }
	virtual void EnableControlGroup(const char *group_name, bool bEnable = true)
	{ m_base->EnableControlGroup(group_name,bEnable); }

	//---------------------------------------------------------------------------
	// Опрос и управление состоянием контролов
	//---------------------------------------------------------------------------
	// получить состояние активности контрола
	virtual ControlStateType GetControlStateType(long control_code) /*const*/
	{ return IsActive() ? m_base->GetControlStateType(control_code) : CST_INACTIVE; }
	virtual ControlStateType GetControlStateType(const char *control_name) /*const*/
	{ return IsActive() ? m_base->GetControlStateType(control_name) : CST_INACTIVE; }

	// получить значение контрола в формате с плавающей точкой
	virtual float GetControlStateFloat(long control_code) /*const*/
	{ return IsActive() ? m_base->GetControlStateFloat(control_code) : 0.0f; }
	virtual float GetControlStateFloat(const char* control_name) /*const*/
	{ return IsActive() ? m_base->GetControlStateFloat(control_name) : 0.0f; }

	// получить значение контрола в формате bool
	virtual bool GetControlStateBool(long control_code) /*const*/
	{ return IsActive() ? m_base->GetControlStateBool(control_code) : false; }
	virtual bool GetControlStateBool(const char* control_name) /*const*/
	{ return IsActive() ? m_base->GetControlStateBool(control_name) : false; }

	// разрешить/запретить обработку контролов
	virtual void Lock()
	{ m_base->Lock(); }
	virtual void Unlock()
	{ m_base->Unlock(); }

	// получить состояние болкировки
	virtual bool Locked()
	{ return m_base->Locked(); }

	virtual long LockCount()
	{ return m_base->LockCount(); }

	// получить алиас активного контрола
	virtual const char *GetPressedControl(const char *alias_name)
	{ return m_base->GetPressedControl(alias_name); }
	virtual const char *GetActiveControl(const char *alias_name)
	{ return m_base->GetActiveControl(alias_name); }
	virtual const char *GetReleasedControl(const char *alias_name)
	{ return m_base->GetReleasedControl(alias_name); }

	// обновить значение алиаса
	virtual const char *UpdateAlias(const char *alias_name, const char *name, bool buildImages = true)
	{ return m_base->UpdateAlias(alias_name,name,buildImages); }

	// загрузить профайл
	virtual bool LoadProfile(
		long player, bool single, long group, const char *name, bool reset = false, bool buildImages = true)
	{ return m_base->LoadProfile(player,single,group,name,reset,buildImages); }
	
	// получить имя текущего профайла
	virtual const char *CurrentProfile(long player, bool single, long group)
	{ return m_base->CurrentProfile(player,single,group); }

	//----------------------------------------------------------------------------
	// Использование возможности Force feedback
	//----------------------------------------------------------------------------
	// получить интерфейс
	virtual IForce *CreateForce(const char *profile,
		bool autoDelete = true, long deviceIndex = 0)
	{ return m_base->CreateForce(this,profile,autoDelete,deviceIndex); }
	virtual IForce *CreateForce(array<Vector> &ls, array<Vector> &rs,
		bool autoDelete = true, long deviceIndex = 0)
	{ return m_base->CreateForce(ls,rs,autoDelete,deviceIndex); }

	// получить количество групп
	virtual long GetControlGroupsQuantity() const
	{ return m_base->GetControlGroupsQuantity(); }
	// выполнить системно-зависимую команду
	virtual void ExecuteCommand(InputSrvCommand &cmd)
	{ m_base->ExecuteCommand(cmd); }

	// работа с буфером клавиш
	virtual unsigned int GetKeyBufferLength() const
	{ return m_base->GetKeyBufferLength(); }
	virtual const KeyDescr *GetKeyBuffer() const
	{ return m_base->GetKeyBuffer(); }
	virtual void ClearKeyBuffer()
	{ m_base->ClearKeyBuffer(); }

	// разлочить мышь на один кадр
	virtual void FreeMouse()
	{ m_base->FreeMouse(); }

	//----------------------------------------------------------------------------
	// ControlInstance
	//----------------------------------------------------------------------------
	// изменить состояние активности
	virtual void Activate(bool isActive = true)
	{
		m_active = isActive;
		m_base->Reset();
	}

	virtual bool IsInactive() const
	{
		return IsActive() == false;
	}

	// удалить
	virtual void Release()
	{
		m_base->ReleaseForces(this);
		m_base->DelInstance(this);

		delete this;
	}

};
