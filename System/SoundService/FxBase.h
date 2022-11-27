
#ifndef _FxBase_h_
#define _FxBase_h_

#include "..\..\common_h\core.h"
#include <Xaudio2.h>
#include <XAPO.h>


#define FXRegistrationInfo(ClassId, Desc, Autor) static const XAPO_REGISTRATION_PROPERTIES regProperties = {__uuidof(ClassId), Desc, Autor, 1, 0, XAPO_FLAG_INPLACE_REQUIRED | XAPO_FLAG_CHANNELS_MUST_MATCH | XAPO_FLAG_FRAMERATE_MUST_MATCH | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH | XAPO_FLAG_INPLACE_SUPPORTED, 1, 1, 1, 1}; FxBase::SystemSetFromConstructor(&regProperties);

#pragma pack(push, 8)

class __declspec(novtable) FxBase: public IXAPO
{
protected:
	FxBase();
	virtual ~FxBase();

protected:	
	void STDMETHODCALLTYPE SystemSetFromConstructor(const XAPO_REGISTRATION_PROPERTIES * pRegistrationProperties);
private:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __deref_out void** ppInterface);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE GetRegistrationProperties(__deref_out XAPO_REGISTRATION_PROPERTIES** ppRegistrationProperties);
	virtual HRESULT STDMETHODCALLTYPE IsInputFormatSupported(const WAVEFORMATEX* pOutputFormat, const WAVEFORMATEX* pRequestedInputFormat, __deref_opt_out WAVEFORMATEX** ppSupportedInputFormat);
	virtual HRESULT STDMETHODCALLTYPE IsOutputFormatSupported(const WAVEFORMATEX* pInputFormat, const WAVEFORMATEX* pRequestedOutputFormat, __deref_opt_out WAVEFORMATEX** ppSupportedOutputFormat);
	virtual HRESULT STDMETHODCALLTYPE Initialize(__in_bcount_opt(DataByteSize) const void *, UINT32 DataByteSize);
	virtual HRESULT STDMETHODCALLTYPE LockForProcess(UINT32 InputLockedParameterCount, __in_ecount_opt(InputLockedParameterCount) const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters, UINT32 OutputLockedParameterCount, __in_ecount_opt(OutputLockedParameterCount) const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters);
	virtual void STDMETHODCALLTYPE Process(UINT32 iCount, const XAPO_PROCESS_BUFFER_PARAMETERS * iParams, UINT32 oCount, XAPO_PROCESS_BUFFER_PARAMETERS *oParams, BOOL isEnabled);
	virtual void STDMETHODCALLTYPE UnlockForProcess();
	virtual UINT32 STDMETHODCALLTYPE CalcInputFrames(UINT32 OutputFrameCount);
	virtual UINT32 STDMETHODCALLTYPE CalcOutputFrames(UINT32 InputFrameCount);

public:
	virtual void STDMETHODCALLTYPE Reset();
	virtual void STDMETHODCALLTYPE Process(float * __restrict ptr, dword count);

private:
	long referenceCount;
	long isLocked;
	const XAPO_REGISTRATION_PROPERTIES * registrationProperties;
};

#pragma pack(pop)

#endif
