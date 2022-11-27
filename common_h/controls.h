//#ifndef _CONTROLS_INTERFACE_H_
//#define _CONTROLS_INTERFACE_H_

#pragma once

#include "core.h"

// интерфейс для Force feedback
#include "IForce.h"

#define INVALID_CODE (-1)

// вид состояния контрола
enum ControlStateType
{
	CST_INACTIVE,		// выключен (выключен в текущем и в предыдущем кадре)
	CST_ACTIVE,		// включен (включен в текущем и в предыдущем кадре)
	CST_INACTIVATED,	// выключен (выключен в текущем и был включен в предыдущем кадре)
	CST_ACTIVATED		// включен (включен в текущем и был выключен в предыдущем кадре)
};

struct KeyDescr
{
	dword ucVKey;
	bool   bSystem;
};

struct ImagePlace
{
	float u; float w;
	float v; float h;
};

struct InputSrvCommand;
class IBaseTexture;

// интерфейс для работы с инстансами контролов

class IControls
{
public:

	virtual ~IControls()
	{
	}

	//---------------------------------------------------------------------------
	// Работа с изображениями контролов
	//---------------------------------------------------------------------------
	// получить общую текстуру с изображениями контролов
	virtual IBaseTexture *GetControlsImage() = 0;

	// получить номер обновления текстуры
	virtual int GetControlsImageVersion() = 0;

	// получить координаты изображения контрола в общей текстуре
	virtual ImagePlace GetControlImagePlace(const char *control_name) = 0;

	//---------------------------------------------------------------------------
	// Управление контролами
	//---------------------------------------------------------------------------
	// найти контрол по его имени
	virtual long FindControlByName(const char *control_name) const = 0;
	virtual long FindGroupByName  (const char *group_name)	 const = 0;

	//---------------------------------------------------------------------------
	// Управление группами контролов
	//---------------------------------------------------------------------------
	// включить/выключить группу контролов
	virtual void EnableControlGroup(long group_num, bool bEnable=true) = 0;
	virtual void EnableControlGroup(const char *group_name, bool bEnable=true) = 0;

	//---------------------------------------------------------------------------
	// Опрос и управление состоянием контролов
	//---------------------------------------------------------------------------
	// получить состояние активности контрола
	virtual ControlStateType GetControlStateType(long control_code) /*const*/ = 0;
	virtual ControlStateType GetControlStateType(const char *control_name) /*const*/ = 0;

	// получить значение контрола в формате с плавающей точкой
	virtual float GetControlStateFloat(long control_code) /*const*/ = 0;
	virtual float GetControlStateFloat(const char *control_name) /*const*/ = 0;

	// получить значение контрола в формате bool
	virtual bool GetControlStateBool(long control_code) /*const*/ = 0;
	virtual bool GetControlStateBool(const char *control_name) /*const*/ = 0;

	// разрешить/запретить обработку контролов
	virtual void Lock() = 0;
	virtual void Unlock() = 0;

	// получить состояние болкировки
	virtual bool Locked() = 0;

	virtual long LockCount() = 0;

	// получить алиас активного контрола
	virtual const char *GetPressedControl(const char *alias_name) = 0;
	virtual const char *GetActiveControl(const char *alias_name) = 0;
	virtual const char *GetReleasedControl(const char *alias_name) = 0;

	// обновить значение алиаса
	virtual const char *UpdateAlias(const char *alias_name, const char *name, bool buildImages = true) = 0;

	// загрузить профайл
	virtual bool LoadProfile(
		long player, bool single, long group, const char *name, bool reset = false, bool buildImages = true) = 0;

	// получить имя текущего профайла
	virtual const char *CurrentProfile(long player, bool single, long group) = 0;

	//----------------------------------------------------------------------------
	// Использование возможности Force feedback
	//----------------------------------------------------------------------------
	// получить интерфейс
	virtual IForce *CreateForce(const char *profile,
		bool autoDelete = true, long deviceIndex = 0) = 0;
	virtual IForce *CreateForce(array<Vector> &ls, array<Vector> &rs,
		bool autoDelete = true, long deviceIndex = 0) = 0;

	// получить количество групп
	virtual long GetControlGroupsQuantity() const = 0;

	// выполнить системно-зависимую команду
	virtual void ExecuteCommand(InputSrvCommand &cmd) = 0;

	virtual unsigned int GetKeyBufferLength() const = 0;
	virtual const KeyDescr *GetKeyBuffer() const = 0;
	virtual void ClearKeyBuffer() = 0;

	// разлочить мышь на один кадр
	virtual void FreeMouse() = 0;

	//----------------------------------------------------------------------------
	// ControlInstance
	//----------------------------------------------------------------------------
	// изменить состояние активности
	virtual void Activate(bool isActive = true) = 0;

	virtual bool IsInactive() const = 0;

	// удалить
	virtual void Release() = 0;

};

// интерфейс для работы с сервисом контролов

class IControlsService : public Service
{
public:

	// создать экземпляр
	virtual IControls *CreateInstance(const char *file, long line) = 0;

	//---------------------------------------------------------------------------
	// Работа с изображениями контролов
	//---------------------------------------------------------------------------
	// получить общую текстуру с изображениями контролов
	virtual IBaseTexture *GetControlsImage() = 0;

	virtual int GetControlsImageVersion() = 0;

	// получить координаты изображения контрола в общей текстуре
	virtual ImagePlace GetControlImagePlace(const char *control_name) = 0;

	//---------------------------------------------------------------------------
	// Управление контролами
	//---------------------------------------------------------------------------
	// найти контрол по его имени
	virtual long FindControlByName(const char *control_name) const = 0;
	virtual long FindGroupByName  (const char *group_name) = 0;

	//---------------------------------------------------------------------------
	// Управление группами контролов
	//---------------------------------------------------------------------------
	// включить/выключить группу контролов
	virtual void EnableControlGroup(long group_num, bool bEnable=true) = 0;
	virtual void EnableControlGroup(const char *group_name, bool bEnable=true) = 0;

//	//---------------------------------------------------------------------------
//	// Опрос и управление состоянием контролов
//	//---------------------------------------------------------------------------
//	// получить состояние активности контрола
//	virtual ControlStateType GetControlStateType(long control_code) /*const*/ = 0;
//	virtual ControlStateType GetControlStateType(const char *control_name) /*const*/ = 0;

//	// получить значение контрола в формате с плавающей точкой
//	virtual float GetControlStateFloat(long control_code) /*const*/ = 0;
//	virtual float GetControlStateFloat(const char *control_name) /*const*/ = 0;

//	// получить значение контрола в формате bool
//	virtual bool GetControlStateBool(long control_code) /*const*/ = 0;
//	virtual bool GetControlStateBool(const char *control_name) /*const*/ = 0;

	// разрешить/запретить обработку контролов
	virtual void Lock() = 0;
	virtual void Unlock() = 0;

	// получить состояние болкировки
	virtual bool Locked() = 0;

	virtual long LockCount() = 0;

//	// получить алиас активного контрола
//	virtual const char *GetPressedControl(const char *alias_name) = 0;
//	virtual const char *GetActiveControl(const char *alias_name) = 0;
//	virtual const char *GetReleasedControl(const char *alias_name) = 0;

	// обновить значение алиаса
	virtual const char *UpdateAlias(const char *alias_name, const char *name, bool buildImages = true) = 0;

	// загрузить профайл
	virtual bool LoadProfile(
		long player, bool single, long group, const char *name, bool reset = false, bool buildImages = true) = 0;

	// получить имя текущего профайла
	virtual const char *CurrentProfile(long player, bool single, long group) = 0;

	//----------------------------------------------------------------------------
	// Использование возможности Force feedback
	//----------------------------------------------------------------------------
	// получить интерфейс
	virtual IForce *CreateForce(const IControls *instance, const char *profile,
		bool autoDelete = true, long deviceIndex = 0) = 0;
	virtual IForce *CreateForce(array<Vector> &ls, array<Vector> &rs,
		bool autoDelete = true, long deviceIndex = 0) = 0;

	// получить количество групп
	virtual long GetControlGroupsQuantity() const = 0;

	// выполнить системно-зависимую команду
	virtual void ExecuteCommand(InputSrvCommand &cmd) = 0;

	virtual unsigned int GetKeyBufferLength() const = 0;
	virtual const KeyDescr *GetKeyBuffer() const = 0;
	virtual void ClearKeyBuffer() = 0;

	// разлочить мышь на один кадр
	virtual void FreeMouse() = 0;

};

//#endif
