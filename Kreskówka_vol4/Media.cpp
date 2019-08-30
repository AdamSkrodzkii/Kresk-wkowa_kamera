#include "Media.h"

Media::Media()
{
	InitializeCriticalSection(&criticalSection);
	referenceCount = 1;
	wSymbolicLink = NULL;
	cchSymbolicLink = 0;
	width = 0;
	height = 0;
	sourceReader = NULL;
	rawData = NULL;

}
Media::~Media()
{

	if (wSymbolicLink)
	{
		delete wSymbolicLink;
		wSymbolicLink = NULL;
	}
	EnterCriticalSection(&criticalSection);

	if (sourceReader)
	{
		sourceReader->Release();
		sourceReader = NULL;
	}


	if (rawData)
	{
		delete rawData;
		rawData = NULL;
	}

	CoTaskMemFree(wSymbolicLink);
	wSymbolicLink = NULL;
	cchSymbolicLink = 0;

	LeaveCriticalSection(&criticalSection);
	DeleteCriticalSection(&criticalSection);
}

HRESULT Media::CreateCaptureDevice()
{
	HRESULT hr = S_OK;																												// Zak�adam, �e uchwyt jest dobrze zrobiony. Ka�da zmiana jest sprawdzana p�niej, czy co� si� nie zmieni�o		
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);													//Inicjuje bibliotek� COM do u�ytku przez w�ek wywo�uj�cy, ustawia model wsp�bie�no�ci w�tku
																																	// Tworzy nowe miejsce dla w�tku je�li jest on wymagany

	UINT32 count = 0;
	IMFAttributes *attributes = NULL;																								//Zapewnia og�lny spos�b na przechowywanie warto�ci na obiekcie
																																	//Standardowa implementacja tego interfejsu zawiera blokad� w�tku podczas dodawania, usuwania lub pobierania warto�ci.
	IMFActivate **devices = NULL;																									//To zapewnia mo�liwo�� od�o�enia na p�niej utworzenie obiektu
																																	//Inne komponenty mog� utworzy� ten sam obiekt

	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }
	
	hr = MFCreateAttributes(&attributes, 1);																						//Tworzenie modu�u wyliczaj�cego

	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }

	hr = attributes->SetGUID																										//Wymagany atrybut urz�dzenia kt�re przechwytuje wideo
	(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
	);
	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }
	
	hr = MFEnumDeviceSources(attributes, &devices, &count);																			//Znajduje wszystkie urz�dzenia o podanych atrybutach oraz wylicza ile ich znalaz�o

	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }

	if (count > 0)																													//Sprawdza, czy zosta�o znalezione jakiekolwiek urz�dzenie nagrywajace
	{
		/*
		
		W tym miejscu wybierane jest urz�dzenie kt�rego obraz b�dzie przechwytywany

		*/
		
		SetSourceReader(devices[0]);																								//Tworzony jest uchwyt do urz�dzenia

		WCHAR *nameString = NULL;																									//Tworz� zmienn� na nazw�

		UINT32 cchName;
		hr = devices[0]->GetAllocatedString																							//Pobera ci�g znak�w, kt�ry jest nazw� skojarzon�z urz�dzeniem. Metoda przydziela pami�� dla ci�gu
		(
			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,																					//guidKey - Pobierana nazwa urz�zenia
			&nameString,																											//je�eli nazwa jest stringiem to tworzy jego kopi� tutaj
			&cchName																												//Zapami�tuje liczb� znak�w
		);

		if (SUCCEEDED(hr))
		{
			////Alokowanie pami�ci na surowe dane
			bytesPerPixel = abs(stride) / width;																					
			rawData = new BYTE[width*height * bytesPerPixel];
			wcscpy_s(deviceNameString, nameString);
		}
		CoTaskMemFree(nameString);																									//Pami�� na string musi by� zawsze zwalniana na ko�cu
	}

	CLEAN_ATTRIBUTES()
}


HRESULT Media::SetSourceReader(IMFActivate *device)
{
	HRESULT hr = S_OK;																												// Zak�adam, �e uchwyt jest dobrze zrobiony. Ka�da zmiana jest sprawdzana p�niej, czy co� si� nie zmieni�o

	IMFMediaSource *source = NULL;																									//�r�d�� multimed�w to obiekty generuj�ce dane multimedialne. Na przyk�ad dane mog� pochodzi�z pliku wideo 
																																	//strumiena sieciowego lub urz�dzenia srz�towego takiego jak kamera. Ka�de �r�d�o multimedi�w zawiera jeden lub wi�cej strumieni
																																	// a ka�dy strumie� dostarcza dane jednego typu, takie jak audio i wideo.
	IMFAttributes *attributes = NULL;																								//Zapewnia og�lny spos�b na przechowywanie warto�ci na obiekcie
																																	//Standardowa implementacja tego interfejsu zawiera blokad� w�tku podczas dodawania, usuwania lub pobierania warto�ci.
	IMFMediaType *MediaType = NULL;																									//Reprezentuje opis formatu multimedi�w.

	EnterCriticalSection(&criticalSection);																							//Czeka na "przyw�aszczenie" specjalnego obiektu sekcji krytycznej. Funkcja zwraca, gdy w�tek wywo�uj�cy uzyska w�asno�� 

	hr = device->ActivateObject																										// Tworzy obiekt.
	(
		__uuidof(IMFMediaSource),																									//Identyfikator �adanego interfacu
		(void**)&source																												//Dostaje wska�nik na �adany interface. "Wywo�ywacz" musi potem zwolni� interface. 
	);															

	//Pobieranie symbolicznego linku do urz�dzenia
	if (SUCCEEDED(hr))
		hr = device->GetAllocatedString																								//Pobera ci�g znak�w, kt�ry jest nazw� skojarzon�z urz�dzeniem. Metoda przydziela pami�� dla ci�gu
		(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,																//Zawiera symboliczne ��cze do sterowika przechwytywania wideo.
			&wSymbolicLink,																											//je�eli nazwa jest stringiem to tworzy jego kopi� tutaj
			&cchSymbolicLink																										//Zapami�tuje liczb� znak�w
		);
	//Tworzenie pustego zbiornika na atrybuty
	if (SUCCEEDED(hr))
		hr = MFCreateAttributes(&attributes, 2);

	//Pobierania atrybut�w
	if (SUCCEEDED(hr))
		hr = attributes->SetUINT32																									//Kojarzy warto�� UINT32 z kluczem
		(
			MF_READWRITE_DISABLE_CONVERTERS,																						//W��cza lub wy��cza konwersje formatu przez czytnik �r�d�owy lub program pisz�cy
			TRUE																													//Nowa warto�� dla klucza
		);
	// Ustawienie wska�nika wywo�ania zwrotnego
	if (SUCCEEDED(hr))
		hr = attributes->SetUnknown
		(
			MF_SOURCE_READER_ASYNC_CALLBACK,																						// Zawiera wska�nik do interfejsu zwrotnego aplikacji dla czytika �r�d�owego
			this
		);
	//Tworzy czytnik �r�d�owy
	if (SUCCEEDED(hr))
		hr = MFCreateSourceReaderFromMediaSource
		(
			source,																													//Wska�nik do IMFMediaSource
			attributes,																												//Wska�nik na IMFAtributes s�u�y do konfigurowania �r�d�a
			&sourceReader																											//Wska�nik na IMFSouurceReader. "Wywo�ywacz" musi zwolni� interface.
		);
	// Pr�ba znalezienia odpowiedniego typu wyj�cia
	if (SUCCEEDED(hr))
	{
		for (DWORD i = 0; ; i++)
		{
			hr = sourceReader->GetNativeMediaType																					//Pobiera format obs�ugiwany natywnie przez �r�d�o multimedi�w
			(
				(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,																			//Pierwszy strumie� video
				i, 
				&MediaType																											//Dostaje wska�nik na IMFMediaType
			);
			if (FAILED(hr)) { break; }

			hr = IsMediaTypeSupported(MediaType);																					//Pyta, czy obiekt obs�uguje okre�lony typ no�nika
			if (FAILED(hr)) { break; }
			
			MFGetAttributeSize																										//Pobieranie atrybutu kt�rego warto�ci� jest rozmiar wyra�ony jako szeroko�� i wysoko��
			(
				MediaType,									/////																	//Wska�nik na IMFMediaType
				MF_MT_FRAME_SIZE,																									//Wybranie kt�r� wielo�� chcemy uzyska�
				&width,
				&height
			);
			if (MediaType)
			{
				MediaType->Release(); MediaType = NULL;																				//Zwolnienie pami�ci
			}

			if (SUCCEEDED(hr))																										// Znaleziony typ wyj�cia
				break;
		}
	}
	if (SUCCEEDED(hr))
	{
		hr = sourceReader->ReadSample																								//Pro�ba o pierwsz� pr�bk� danych
		(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,																				//Strumie� z kt�rego s� pobierane dane
			0,																														//Jaka� tam flaga z wyliczenia MF_SOURCE_READER_CONTROL_FLAG 
			NULL,																													//Pobiera od zera liczony index strumienia.
			NULL,																													//Jaka� tam flaga z wyliczenia MF_SOURCE_READER_FLAG 
			NULL,																													//Pobiera znacznik czasu pr�bki wskazany przez pdwStreamFlags, cczas podawany w jednostkach 100nanosekundowych
			NULL																													//Pobiera wska�nik na IMFSample , je�eli warto�� nie jest zerowa to "wywo�ywacz musi zwolni� pami��
		);
	}

	if (FAILED(hr))
	{
		// Zamyka zr�d�o medi�w i zwalnia u�ywane dane
		if (source)
		{
			source->Shutdown();
		}
		Close();
	}
	if (source) { source->Release(); source = NULL; }
	if (attributes) { attributes->Release(); attributes = NULL; }
	if (MediaType) { MediaType->Release(); MediaType = NULL; }

	LeaveCriticalSection(&criticalSection);																							//Zwalnia pami�� sekcji krytycznej
	return hr;
}

HRESULT Media::IsMediaTypeSupported(IMFMediaType *pType)
{
	HRESULT hr = S_OK;

	BOOL bFound = FALSE;
	GUID subtype = { 0 };																											//Warto�ci u�ywane w Windows Media Format SDK i sta�e globalne u�ywane do ich reprezentowania

	GetDefaultStride(pType, &stride);																								//Pobiera "Defauld Stride"  bazowany na formacie i rozmiarze ramek

	if (FAILED(hr)) { return hr; }
	hr = pType->GetGUID																												//Pobiera warto�� okre�lonej kolumny jako unikatowy identyfikator globalny 
	(
		MF_MT_SUBTYPE,
		&subtype
	);																					

	videoFormat = subtype;																											// Uchwyt do zmiennej PUBLIC klasy Media

	if (FAILED(hr)) { return hr; }																									//Sprawdza, czy format jest obs�ugiwany

	if (
		subtype == MFVideoFormat_RGB32 ||
		subtype == MFVideoFormat_RGB24 ||
		subtype == MFVideoFormat_YUY2 ||
		subtype == MFVideoFormat_NV12
		)
		return S_OK;
	else
		return S_FALSE;

	return hr;
}

// Zwalnianie wszystkich uchwyt�w ("Handlers") razem ze zwalnianiem pami�ci
HRESULT Media::Close()
{
	EnterCriticalSection(&criticalSection);
	if (sourceReader)
	{
		sourceReader->Release(); sourceReader = NULL;
	}

	CoTaskMemFree(wSymbolicLink);
	wSymbolicLink = NULL;
	cchSymbolicLink = 0;

	LeaveCriticalSection(&criticalSection);
	return S_OK;
}


STDMETHODIMP Media::QueryInterface(REFIID riid, void** ppvObject)																		//Szablon funkcji pomocniczej
{
	static const QITAB qit[] = { QITABENT(Media, IMFSourceReaderCallback),{ 0 }, };														// Funkcja opisuj�ca jeden interface
	return QISearch
	(
		this,																															//Wska�nik do podstawy obiektu COM
		qit,																															//tablica struktur QITAB
		riid,																															//Odwo�anie do IID interfacu do pobrania przez ppv
		ppvObject																														//Gdy funkcja "powr�ci" pomy�lnie to ten sk�adnik zawiera wska�nik wymagany w riid
	);
}

ULONG Media::Release()
{
	ULONG count = InterlockedDecrement(&referenceCount);																				//Zmniejsza o jeden warto�c okre�lonej zmiennej 32-bitowej jako operacj� atomow�
	if (count == 0)
		delete this;
	return count;
}

ULONG Media::AddRef()
{
	return InterlockedIncrement(&referenceCount);
}


//Pobiera "Defauld Stride"  bazowany na formacie i rozmiarze ramek
HRESULT Media::GetDefaultStride(IMFMediaType *type, LONG *stride)
{
	LONG tempStride = 0;

	//Pr�ba zdobycia "default stride" z media type
	HRESULT hr = type->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&tempStride);															//Zwraca UINT32 (taki intiger) 
	
	if (FAILED(hr))
	{
		//Ustawiaj�c ten atrybut na NULL mo�emy uzyska� domy�lny krok
		GUID subtype = GUID_NULL;

		UINT32 width = 0;
		UINT32 height = 0;

		//Uzyskiwanie podtypu
		hr = type->GetGUID(MF_MT_SUBTYPE, &subtype);																					//Pobiera okre�lony parametr jako unikatowy identyfikator globalny

		//Uzyskiwanie szeroko�ci i wysoko�ci obrazu
		if (SUCCEEDED(hr))
			hr = MFGetAttributeSize																										//Zwraca atrybut, ktr�rego warto�� jest reprezentowana przez wysoko�� i szeroko��
			(
				type,																													//Wska�nik
				MF_MT_FRAME_SIZE,																										//Okre�la, kt�r� warto�� chcemy uzyska�
				&width,
				&height
			);

		//Wyliczanie "Stride" bazuj�c na podtypie i szeroko��i
		if (SUCCEEDED(hr))
			hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &tempStride);

		//Ustawienie atrybut�w do oddczytu
		if (SUCCEEDED(hr))
			(void)type->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(tempStride));
	}

	if (SUCCEEDED(hr))
		*stride = tempStride;
	return hr;
}

HRESULT Media::OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp, IMFSample *sample)
{
	HRESULT hr = S_OK;
	IMFMediaBuffer *MediaBuffer = NULL;

	EnterCriticalSection(&criticalSection);																								//Czeka na "przyw�aszczenie" specjalnego obiektu sekcji krytycznej. Funkcja zwraca, gdy w�tek wywo�uj�cy uzyska w�asno�� 

	if (FAILED(status))
		hr = status;

	if (SUCCEEDED(hr))
	{
		if (sample)
		{
			hr = sample->GetBufferByIndex																								//Pobieranie ramki buffera z pr�bki
			(
				0,																														//Index buffera 
				&MediaBuffer																											//Otrzymanie wska�nika na IMFMediaBuffer interface
			);																				
			
			//Rysowanie ramki
			if (SUCCEEDED(hr))
			{
				BYTE* data;
				MediaBuffer->Lock																										//Pozwala funkcji wywo�uj�cej na dost�p do pami�ci w bufforze, do odczytu lub zapisu
				(
					&data,																												//Otrzymuje wska�nik do pocz�tku buffora
					NULL,																												//maksymalny rozmiar danych kt�re mog� by� zapisane do buffera
					NULL																												//D�ugo�� poprawnych danych w bufforze w bajtach
				);																					
				//To jest dobre miejsce aby zmieni� kolory do rysowania
				//Dla pr�by w tym momencie ja te dane pr�buj� po prostu skopiowa� 
				CopyMemory
				(
					rawData,																											//Gdzie maj� by� kopiowane dane
					data,																												//Sk�d te dane b�d� kopiowane
					width*height * bytesPerPixel																						//Rozmar bloku pami�ci (szerokos� razy wysoko�� razy ile bajt�w jest przeznaczonych na jeden pixel)
				);

			}
		}
	}
	//Pro�ba o kolejn� pr�bk�
	if (SUCCEEDED(hr))
		hr = sourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);

	if (FAILED(hr))
	{
		// Chcia�bym aby zr�ci�o mi miejsce gdzie jest b��d
		printf("Error HRESULT = 0x%d", hr);
		PostMessage(NULL, 1, (WPARAM)hr, 0L);
	}
	if (MediaBuffer) { MediaBuffer->Release(); MediaBuffer = NULL; }

	LeaveCriticalSection(&criticalSection);
	return hr;
}
STDMETHODIMP Media::OnEvent(DWORD, IMFMediaEvent *) { return S_OK; }

STDMETHODIMP Media::OnFlush(DWORD) { return S_OK; }