#pragma once
//Media foundation headers
#include <Windows.h>
#include <mfidl.h> 
#include <Mfapi.h> 
#include <Mfreadwrite.h>
#include <Shlwapi.h>

//include and lib dependencies for Media Foundation
#pragma comment(lib,"Mfplat.lib")
#pragma comment(lib,"Mf.lib")
#pragma comment(lib,"Mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")
#pragma comment(lib,"shlwapi.lib")

#include <stdio.h>

#define CLEAN_ATTRIBUTES() if (attributes) { attributes->Release(); attributes = NULL; }for (DWORD i = 0; i < count; i++){if (&devices[i]) { devices[i]->Release(); devices[i] = NULL; }}CoTaskMemFree(devices);return hr;


class Media : public IMFSourceReaderCallback //klasa dziedziczy po IMFSourceReaderCallback
{
	CRITICAL_SECTION criticalSection;																												// Obiekt zapewnia synchronizacjê, lecz moo¿ê byæ u¿ywana tylko przez w¹tki jednego procesu.
																																					// Takie obiekty nie mog¹byæ wspó³u¿ytkowane przez procesy. Pozwala na szybsze dzia³anie. 
	long referenceCount;
	WCHAR                   *wSymbolicLink;																											//symboliczne ³¹cze do sterowika przechwytywania wideo.
	UINT32                  cchSymbolicLink;
	IMFSourceReader* sourceReader;


public:
	LONG stride;
	int bytesPerPixel;
	GUID videoFormat;																																//U¿ywany przy funkcji ISMediaTypeSuported jako magazyn GUID dla parametrów do reprezentowania Windows Formad SDK
	UINT height;
	UINT width;
	WCHAR deviceNameString[2048];
	BYTE* rawData;

	HRESULT CreateCaptureDevice();
	HRESULT SetSourceReader(IMFActivate *device);
	HRESULT IsMediaTypeSupported(IMFMediaType* type);
	HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride);
	HRESULT Close();
	Media();
	~Media();

	// STDMETHODIMP gwarantuje, ¿e konwencja jest zgodna z COM (COM to jakiœ binarny interface, standard dla Microsoftu od 1993 roku)
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp, IMFSample *sample);
	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *);
	STDMETHODIMP OnFlush(DWORD);

};
