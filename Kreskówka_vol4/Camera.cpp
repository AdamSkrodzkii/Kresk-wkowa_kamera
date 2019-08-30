#include "Camera.h"



Camera::Camera()
{
}


Camera::~Camera()
{
}

HRESULT Camera::CaptureCamera()
{
	// Wybieranie urz¹dzenia

	ICreateDevEnum *pDevEnum;														//Tworzy modu³ wyliczaj¹cy dla kategorii filtrów
	HRESULT hr = CoCreateInstance													//Tworzy jedyny niezainicjowany obiekt klasy 
	(
		CLSID_SystemDeviceEnum,														//Dane lub kod który bêdzie u¿yty do stworzenia obiektu									{REFCLSID}
		NULL,																		//Kontekst w którym zostanie uruchomiony kod 
																					//zarz¹dzaj¹cy nowo utworzonym obiektem. Wartoœci pochodz¹ z System Device Enumerator	{LPUNKNOWN}
		CLSCTX_INPROC_SERVER,														//Kontekst w którym nowo powsta³y object bêdzie pracowa³								{DWORD}
		IID_PPV_ARGS(&pDevEnum)														//Odwo³anie do identyfikatora interfejsu ¿ywanego do komunikacji z obiektem				{REFIID}

																					//Jest jeszcze pi¹ta wielkoœæ, jest to adres zmiennej wskaŸnika, która
																					//odbiera wskaŸnik interfejsu ¿adany w REFIID											{LPVOID}
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