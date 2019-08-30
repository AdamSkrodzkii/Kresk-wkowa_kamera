#include "Camera.h"



Camera::Camera()
{
}


Camera::~Camera()
{
}

HRESULT Camera::CaptureCamera()
{
	// Wybieranie urz�dzenia

	ICreateDevEnum *pDevEnum;														//Tworzy modu� wyliczaj�cy dla kategorii filtr�w
	HRESULT hr = CoCreateInstance													//Tworzy jedyny niezainicjowany obiekt klasy 
	(
		CLSID_SystemDeviceEnum,														//Dane lub kod kt�ry b�dzie u�yty do stworzenia obiektu									{REFCLSID}
		NULL,																		//Kontekst w kt�rym zostanie uruchomiony kod 
																					//zarz�dzaj�cy nowo utworzonym obiektem. Warto�ci pochodz� z System Device Enumerator	{LPUNKNOWN}
		CLSCTX_INPROC_SERVER,														//Kontekst w kt�rym nowo powsta�y object b�dzie pracowa�								{DWORD}
		IID_PPV_ARGS(&pDevEnum)														//Odwo�anie do identyfikatora interfejsu �ywanego do komunikacji z obiektem				{REFIID}

																					//Jest jeszcze pi�ta wielko��, jest to adres zmiennej wska�nika, kt�ra
																					//odbiera wska�nik interfejsu �adany w REFIID											{LPVOID}
	);

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		//hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}