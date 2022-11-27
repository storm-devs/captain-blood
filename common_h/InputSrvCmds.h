#pragma once

enum CommandID
{
	LockMouseCursorPos,
	QueryNeedPause,
	QueryJoypadConnect,
	QueryJoypadAssignIndex
};

struct InputSrvCommand
{
	CommandID	id;
};


struct InputSrvLockMouse : public InputSrvCommand
{
	bool isLock;
	InputSrvLockMouse(bool isLock) : isLock(isLock)
	{
		id = LockMouseCursorPos;
	}
};


struct InputSrvQueryNeedPause : public InputSrvCommand
{
	bool needPause;
	InputSrvQueryNeedPause()
	{
		id = QueryNeedPause;
	}
};

struct InputSrvQueryJoypadConnect : public InputSrvCommand
{
	long joypadIndex;
	bool isConnect;
	InputSrvQueryJoypadConnect(long idx) : joypadIndex(idx)
	{
		id = QueryJoypadConnect;
	}
};

struct InputSrvQueryAssignIndex : public InputSrvCommand
{
	long joypadIndex;
	long assignIndex;
	InputSrvQueryAssignIndex(long idx) : joypadIndex(idx)
	{
		id = QueryJoypadAssignIndex;
	}
};
