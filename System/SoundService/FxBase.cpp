

#include "FxBase.h"


FxBase::FxBase()
{
	referenceCount = 1;
	isLocked = 0;
	registrationProperties = null;
}

FxBase::~FxBase()
{
}

void STDMETHODCALLTYPE FxBase::SystemSetFromConstructor(const XAPO_REGISTRATION_PROPERTIES * pRegistrationProperties)
{
	Assert(pRegistrationProperties != null);
	Assert(registrationProperties == null);
	registrationProperties = pRegistrationProperties;
}

HRESULT STDMETHODCALLTYPE FxBase::QueryInterface(REFIID riid, __deref_out void** ppInterface)
{
	HRESULT hr = S_OK;
	if(riid == __uuidof(IXAPO))
	{
		*ppInterface = static_cast<IXAPO*>(this);
		AddRef();
	}else
	if(riid == __uuidof(IUnknown))
	{
		*ppInterface = static_cast<IUnknown*>(this);
		AddRef();
	}else{
		*ppInterface = NULL;
		hr = E_NOINTERFACE;
	}
	return hr;
}

ULONG STDMETHODCALLTYPE FxBase::AddRef()
{
	return (ULONG)InterlockedIncrement(&referenceCount);
}

ULONG STDMETHODCALLTYPE FxBase::Release()
{
	ULONG uTmpReferenceCount = (ULONG)InterlockedDecrement(&referenceCount);
	if(uTmpReferenceCount == 0)
	{
		Assert(false);
//		delete this;
	}
	return uTmpReferenceCount;

}

HRESULT STDMETHODCALLTYPE FxBase::GetRegistrationProperties(__deref_out XAPO_REGISTRATION_PROPERTIES ** ppRegistrationProperties)
{
	if(!ppRegistrationProperties) return E_POINTER;
	*ppRegistrationProperties = (XAPO_REGISTRATION_PROPERTIES *)XAPOAlloc(sizeof(XAPO_REGISTRATION_PROPERTIES));
	memcpy(*ppRegistrationProperties, registrationProperties, sizeof(XAPO_REGISTRATION_PROPERTIES));	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE FxBase::IsInputFormatSupported(const WAVEFORMATEX* pOutputFormat, const WAVEFORMATEX* pRequestedInputFormat, __deref_opt_out WAVEFORMATEX** ppSupportedInputFormat)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE FxBase::IsOutputFormatSupported(const WAVEFORMATEX* pInputFormat, const WAVEFORMATEX* pRequestedOutputFormat, __deref_opt_out WAVEFORMATEX** ppSupportedOutputFormat)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE FxBase::Initialize(__in_bcount_opt(DataByteSize) const void*, UINT32 DataByteSize)
{
	UNREFERENCED_PARAMETER(DataByteSize);
	return S_OK;
}

void STDMETHODCALLTYPE  FxBase::Reset()
{
}

HRESULT STDMETHODCALLTYPE FxBase::LockForProcess(UINT32 InputLockedParameterCount, __in_ecount_opt(InputLockedParameterCount) const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters, UINT32 OutputLockedParameterCount, __in_ecount_opt(OutputLockedParameterCount) const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters)
{
	Assert(isLocked == 0);
	Assert(InputLockedParameterCount == 1);
	Assert(OutputLockedParameterCount == 1);
	Assert(pInputLockedParameters != null);
	Assert(pOutputLockedParameters != null);
	Assert(pInputLockedParameters[0].pFormat != null);
	Assert(pOutputLockedParameters[0].pFormat != null);	
	Assert(pInputLockedParameters[0].pFormat->nChannels == pOutputLockedParameters[0].pFormat->nChannels);
	Assert(pOutputLockedParameters[0].pFormat->nChannels == 2);
	Assert(pInputLockedParameters[0].pFormat->wBitsPerSample == 32);
	Assert(pOutputLockedParameters[0].pFormat->wBitsPerSample == 32);
	Reset();
	isLocked = 1;
	return S_OK;
}

void STDMETHODCALLTYPE FxBase::Process(UINT32 iCount, const XAPO_PROCESS_BUFFER_PARAMETERS * iParams, UINT32 oCount, XAPO_PROCESS_BUFFER_PARAMETERS *oParams, BOOL isEnabled)
{
	Assert(isLocked != 0);
	Assert(iCount == 1);
	Assert(oCount == 1);
	Assert(iParams);
	Assert(oParams);


	XAPO_BUFFER_FLAGS inFlags = iParams[0].BufferFlags;
	XAPO_BUFFER_FLAGS outFlags = oParams[0].BufferFlags;

	Assert(inFlags == XAPO_BUFFER_VALID || inFlags == XAPO_BUFFER_SILENT);
	Assert(outFlags == XAPO_BUFFER_VALID || outFlags == XAPO_BUFFER_SILENT);

	switch (inFlags)
	{
	case XAPO_BUFFER_VALID:
		Assert(iParams[0].pBuffer != null);
		Assert(iParams[0].pBuffer == oParams[0].pBuffer);
		Process((float * __restrict)iParams[0].pBuffer, iParams[0].ValidFrameCount);
		break;
	case XAPO_BUFFER_SILENT:
		break;
	}
	oParams[0].ValidFrameCount = iParams[0].ValidFrameCount;
	oParams[0].BufferFlags = iParams[0].BufferFlags;
}

void STDMETHODCALLTYPE FxBase::UnlockForProcess()
{
	Reset();
	isLocked = 0;
}

UINT32 STDMETHODCALLTYPE FxBase::CalcInputFrames(UINT32 OutputFrameCount)
{
	return OutputFrameCount;
}

UINT32 STDMETHODCALLTYPE FxBase::CalcOutputFrames(UINT32 InputFrameCount)
{
	return InputFrameCount;
}

