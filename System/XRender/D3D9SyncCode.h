/* 
* Author Sergey Makeev <mail@sergeymakeev.com>, Jan 2009.
*
*
* This software is 100% free
* You may not use this software to directly or indirectly cause harm to others.
* This software is provided as-is and without warrantee. Use at your own risk.
*/

#ifndef _D3D9_SYNCROCODE_H_
#define _D3D9_SYNCROCODE_H_

#include <d3d9.h>

#define GPU_MAX_FRAMES_AHEAD 4

/*! Не дает уйти GPU вперед CPU больше чем, на заданное кол-во кадров.

Пример использования:
\code

Инициализация
...
d3d9Sync.Init(validD3DDevicePtr, gpuFramesAheadCount);
...


Завершение работы
...
d3d9Sync.Release();
...


обновление кадра
...
d3d9Sync.Sync();
validD3DDevicePtr->Present();
...

\endcode

*/
class D3D9Sync
{
	long syncGpuFramesAhead;
	long syncQueryIndex;
	IDirect3DQuery9* syncQueryArray[GPU_MAX_FRAMES_AHEAD]; 
	IDirect3DDevice9* d3dDevice;

public:

	D3D9Sync()
	{
		syncQueryIndex = 0;
		for (long n = 0; n < GPU_MAX_FRAMES_AHEAD; n++)
		{
			syncQueryArray[n] = NULL;
		}

		syncGpuFramesAhead = 1;
		d3dDevice = NULL;
	}

	~D3D9Sync()
	{
		Release();
	}


	/*!
	Инициализация синхронизатора

	device - созданный, валидный d3d9 device
	gpuFramesAheadCount - на сколько кадров вперед можно отпустить GPU, рекомендуемое значение 1

	*/
	__forceinline void Init(IDirect3DDevice9* device, long gpuFramesAheadCount)
	{
		Release();

		syncGpuFramesAhead = gpuFramesAheadCount;
		if (syncGpuFramesAhead > GPU_MAX_FRAMES_AHEAD)
		{
			syncGpuFramesAhead = GPU_MAX_FRAMES_AHEAD; 
		}

		d3dDevice = device;

		Reset();
	}


	/*!
	Деинициализация синхронизатора

	*/
	__forceinline void Release()
	{
		for (long n = 0; n < GPU_MAX_FRAMES_AHEAD; n++)
		{
			if (syncQueryArray[n])
			{
				syncQueryArray[n]->Release();
				syncQueryArray[n] = NULL;
			}
		}
	}

	/*!
	Синхронизировать GPU и CPU, не давать GPU уйти вперед больше чем на syncGpuFramesAhead кадров

	нужно выполнять непосредственно перед device->Present();

	*/
	__forceinline void Sync()
	{
		if (syncGpuFramesAhead >= 0)
		{
			//Flush command buffer 
			long issueIdx = syncQueryIndex % GPU_MAX_FRAMES_AHEAD;
			HRESULT issueRes = syncQueryArray[issueIdx]->Issue(D3DISSUE_END); 
			if (issueRes != D3DERR_DEVICELOST)
			{
				long resultIdx = (syncQueryIndex - syncGpuFramesAhead) % GPU_MAX_FRAMES_AHEAD;

				if (resultIdx >= 0)
				{
					for (;;)
					{
						HRESULT queryRes = syncQueryArray[resultIdx]->GetData( NULL, 0, D3DGETDATA_FLUSH);
						if (queryRes != S_FALSE) break;
						Sleep(0);
					}
				}

			}
			syncQueryIndex++;
		}
	}


	/*!
	Выполнить при потере d3d устройства

	*/
	__forceinline void Lost()
	{
		Release();
		syncQueryIndex = 0;
	}


	/*!
	Выполнить при восстановлении d3d устройства

	*/
	__forceinline void Reset()
	{
		if (!d3dDevice)
		{
			return;
		}

		for (long n = 0; n < GPU_MAX_FRAMES_AHEAD; n++)
		{
			d3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &syncQueryArray[n]);
		}
	}

};

#endif