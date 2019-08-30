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
	HRESULT hr = S_OK;																												// Zak³adam, ¿e uchwyt jest dobrze zrobiony. Ka¿da zmiana jest sprawdzana pó¿niej, czy coœ siê nie zmieni³o		
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);													//Inicjuje bibliotekê COM do u¿ytku przez w¹ek wywo³uj¹cy, ustawia model wspó³bie¿noœci w¹tku
																																	// Tworzy nowe miejsce dla w¹tku jeœli jest on wymagany

	UINT32 count = 0;
	IMFAttributes *attributes = NULL;																								//Zapewnia ogólny sposób na przechowywanie wartoœci na obiekcie
																																	//Standardowa implementacja tego interfejsu zawiera blokadê w¹tku podczas dodawania, usuwania lub pobierania wartoœci.
	IMFActivate **devices = NULL;																									//To zapewnia mo¿liwoœæ od³o¿enia na póŸniej utworzenie obiektu
																																	//Inne komponenty mog¹ utworzyæ ten sam obiekt

	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }
	
	hr = MFCreateAttributes(&attributes, 1);																						//Tworzenie modu³u wyliczaj¹cego

	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }

	hr = attributes->SetGUID																										//Wymagany atrybut urz¹dzenia które przechwytuje wideo
	(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
	);
	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }
	
	hr = MFEnumDeviceSources(attributes, &devices, &count);																			//Znajduje wszystkie urz¹dzenia o podanych atrybutach oraz wylicza ile ich znalaz³o

	if (FAILED(hr)) { CLEAN_ATTRIBUTES() }

	if (count > 0)																													//Sprawdza, czy zosta³o znalezione jakiekolwiek urz¹dzenie nagrywajace
	{
		/*
		
		W tym miejscu wybierane jest urz¹dzenie którego obraz bêdzie przechwytywany

		*/
		
		SetSourceReader(devices[0]);																								//Tworzony jest uchwyt do urz¹dzenia

		WCHAR *nameString = NULL;																									//Tworzê zmienn¹ na nazwê

		UINT32 cchName;
		hr = devices[0]->GetAllocatedString																							//Pobera ci¹g znaków, który jest nazw¹ skojarzon¹z urz¹dzeniem. Metoda przydziela pamiêæ dla ci¹gu
		(
			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,																					//guidKey - Pobierana nazwa urz¹zenia
			&nameString,																											//je¿eli nazwa jest stringiem to tworzy jego kopiê tutaj
			&cchName																												//Zapamiêtuje liczbê znaków
		);

		if (SUCCEEDED(hr))
		{
			////Alokowanie pamiêci na surowe dane
			bytesPerPixel = abs(stride) / width;																					
			rawData = new BYTE[width*height * bytesPerPixel];
			wcscpy_s(deviceNameString, nameString);
		}
		CoTaskMemFree(nameString);																									//Pamiêæ na string musi byæ zawsze zwalniana na koñcu
	}

	CLEAN_ATTRIBUTES()
}


HRESULT Media::SetSourceReader(IMFActivate *device)
{
	HRESULT hr = S_OK;																												// Zak³adam, ¿e uchwyt jest dobrze zrobiony. Ka¿da zmiana jest sprawdzana pó¿niej, czy coœ siê nie zmieni³o

	IMFMediaSource *source = NULL;																									//ród³¹ multimedów to obiekty generuj¹ce dane multimedialne. Na przyk³ad dane mog¹ pochodziæz pliku wideo 
																																	//strumiena sieciowego lub urz¹dzenia srzêtowego takiego jak kamera. Ka¿de Ÿród³o multimediów zawiera jeden lub wiêcej strumieni
																																	// a ka¿dy strumieñ dostarcza dane jednego typu, takie jak audio i wideo.
	IMFAttributes *attributes = NULL;																								//Zapewnia ogólny sposób na przechowywanie wartoœci na obiekcie
																																	//Standardowa implementacja tego interfejsu zawiera blokadê w¹tku podczas dodawania, usuwania lub pobierania wartoœci.
	IMFMediaType *MediaType = NULL;																									//Reprezentuje opis formatu multimediów.

	EnterCriticalSection(&criticalSection);																							//Czeka na "przyw³aszczenie" specjalnego obiektu sekcji krytycznej. Funkcja zwraca, gdy w¹tek wywo³uj¹cy uzyska w³asnoœæ 

	hr = device->ActivateObject																										// Tworzy obiekt.
	(
		__uuidof(IMFMediaSource),																									//Identyfikator ¿adanego interfacu
		(void**)&source																												//Dostaje wskaŸnik na ¿adany interface. "Wywo³ywacz" musi potem zwolniæ interface. 
	);															

	//Pobieranie symbolicznego linku do urz¹dzenia
	if (SUCCEEDED(hr))
		hr = device->GetAllocatedString																								//Pobera ci¹g znaków, który jest nazw¹ skojarzon¹z urz¹dzeniem. Metoda przydziela pamiêæ dla ci¹gu
		(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,																//Zawiera symboliczne ³¹cze do sterowika przechwytywania wideo.
			&wSymbolicLink,																											//je¿eli nazwa jest stringiem to tworzy jego kopiê tutaj
			&cchSymbolicLink																										//Zapamiêtuje liczbê znaków
		);
	//Tworzenie pustego zbiornika na atrybuty
	if (SUCCEEDED(hr))
		hr = MFCreateAttributes(&attributes, 2);

	//Pobierania atrybutów
	if (SUCCEEDED(hr))
		hr = attributes->SetUINT32																									//Kojarzy wartoœæ UINT32 z kluczem
		(
			MF_READWRITE_DISABLE_CONVERTERS,																						//W³¹cza lub wy³¹cza konwersje formatu przez czytnik Ÿród³owy lub program pisz¹cy
			TRUE																													//Nowa wartoœæ dla klucza
		);
	// Ustawienie wska¿nika wywo³ania zwrotnego
	if (SUCCEEDED(hr))
		hr = attributes->SetUnknown
		(
			MF_SOURCE_READER_ASYNC_CALLBACK,																						// Zawiera wskaŸnik do interfejsu zwrotnego aplikacji dla czytika Ÿród³owego
			this
		);
	//Tworzy czytnik Ÿród³owy
	if (SUCCEEDED(hr))
		hr = MFCreateSourceReaderFromMediaSource
		(
			source,																													//WskaŸnik do IMFMediaSource
			attributes,																												//WskaŸnik na IMFAtributes s³u¿y do konfigurowania Ÿród³a
			&sourceReader																											//WskaŸnik na IMFSouurceReader. "Wywo³ywacz" musi zwolniæ interface.
		);
	// Próba znalezienia odpowiedniego typu wyjœcia
	if (SUCCEEDED(hr))
	{
		for (DWORD i = 0; ; i++)
		{
			hr = sourceReader->GetNativeMediaType																					//Pobiera format obs³ugiwany natywnie przez Ÿród³o multimediów
			(
				(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,																			//Pierwszy strumieñ video
				i, 
				&MediaType																											//Dostaje wskaŸnik na IMFMediaType
			);
			if (FAILED(hr)) { break; }

			hr = IsMediaTypeSupported(MediaType);																					//Pyta, czy obiekt obs³uguje okreœlony typ noœnika
			if (FAILED(hr)) { break; }
			
			MFGetAttributeSize																										//Pobieranie atrybutu którego wartoœci¹ jest rozmiar wyra¿ony jako szerokoœæ i wysokoœæ
			(
				MediaType,									/////																	//WskaŸnik na IMFMediaType
				MF_MT_FRAME_SIZE,																									//Wybranie któr¹ wieloœæ chcemy uzyskaæ
				&width,
				&height
			);
			if (MediaType)
			{
				MediaType->Release(); MediaType = NULL;																				//Zwolnienie pamiêci
			}

			if (SUCCEEDED(hr))																										// Znaleziony typ wyjœcia
				break;
		}
	}
	if (SUCCEEDED(hr))
	{
		hr = sourceReader->ReadSample																								//ProŸba o pierwsz¹ próbkê danych
		(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,																				//Strumieñ z którego s¹ pobierane dane
			0,																														//Jakaœ tam flaga z wyliczenia MF_SOURCE_READER_CONTROL_FLAG 
			NULL,																													//Pobiera od zera liczony index strumienia.
			NULL,																													//Jakaœ tam flaga z wyliczenia MF_SOURCE_READER_FLAG 
			NULL,																													//Pobiera znacznik czasu próbki wskazany przez pdwStreamFlags, cczas podawany w jednostkach 100nanosekundowych
			NULL																													//Pobiera wskaŸnik na IMFSample , je¿eli wartoœæ nie jest zerowa to "wywo³ywacz musi zwolniæ pamiêæ
		);
	}

	if (FAILED(hr))
	{
		// Zamyka zród³o mediów i zwalnia u¿ywane dane
		if (source)
		{
			source->Shutdown();
		}
		Close();
	}
	if (source) { source->Release(); source = NULL; }
	if (attributes) { attributes->Release(); attributes = NULL; }
	if (MediaType) { MediaType->Release(); MediaType = NULL; }

	LeaveCriticalSection(&criticalSection);																							//Zwalnia pamiêæ sekcji krytycznej
	return hr;
}

HRESULT Media::IsMediaTypeSupported(IMFMediaType *pType)
{
	HRESULT hr = S_OK;

	BOOL bFound = FALSE;
	GUID subtype = { 0 };																											//Wartoœci u¿ywane w Windows Media Format SDK i sta³e globalne u¿ywane do ich reprezentowania

	GetDefaultStride(pType, &stride);																								//Pobiera "Defauld Stride"  bazowany na formacie i rozmiarze ramek

	if (FAILED(hr)) { return hr; }
	hr = pType->GetGUID																												//Pobiera wartoœæ okreœlonej kolumny jako unikatowy identyfikator globalny 
	(
		MF_MT_SUBTYPE,
		&subtype
	);																					

	videoFormat = subtype;																											// Uchwyt do zmiennej PUBLIC klasy Media

	if (FAILED(hr)) { return hr; }																									//Sprawdza, czy format jest obs³ugiwany

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

// Zwalnianie wszystkich uchwytów ("Handlers") razem ze zwalnianiem pamiêci
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
	static const QITAB qit[] = { QITABENT(Media, IMFSourceReaderCallback),{ 0 }, };														// Funkcja opisuj¹ca jeden interface
	return QISearch
	(
		this,																															//WskaŸnik do podstawy obiektu COM
		qit,																															//tablica struktur QITAB
		riid,																															//Odwo³anie do IID interfacu do pobrania przez ppv
		ppvObject																														//Gdy funkcja "powróci" pomyœlnie to ten sk³adnik zawiera wskaŸnik wymagany w riid
	);
}

ULONG Media::Release()
{
	ULONG count = InterlockedDecrement(&referenceCount);																				//Zmniejsza o jeden wartoœc okreœlonej zmiennej 32-bitowej jako operacjê atomow¹
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

	//Próba zdobycia "default stride" z media type
	HRESULT hr = type->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&tempStride);															//Zwraca UINT32 (taki intiger) 
	
	if (FAILED(hr))
	{
		//Ustawiaj¹c ten atrybut na NULL mo¿emy uzyskaæ domyœlny krok
		GUID subtype = GUID_NULL;

		UINT32 width = 0;
		UINT32 height = 0;

		//Uzyskiwanie podtypu
		hr = type->GetGUID(MF_MT_SUBTYPE, &subtype);																					//Pobiera okreœlony parametr jako unikatowy identyfikator globalny

		//Uzyskiwanie szerokoœci i wysokoœci obrazu
		if (SUCCEEDED(hr))
			hr = MFGetAttributeSize																										//Zwraca atrybut, ktrórego wartoœæ jest reprezentowana przez wysokoœæ i szerokoœæ
			(
				type,																													//WskaŸnik
				MF_MT_FRAME_SIZE,																										//Okreœla, któr¹ wartoœæ chcemy uzyskaæ
				&width,
				&height
			);

		//Wyliczanie "Stride" bazuj¹c na podtypie i szerokoœæi
		if (SUCCEEDED(hr))
			hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &tempStride);

		//Ustawienie atrybutów do oddczytu
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

	EnterCriticalSection(&criticalSection);																								//Czeka na "przyw³aszczenie" specjalnego obiektu sekcji krytycznej. Funkcja zwraca, gdy w¹tek wywo³uj¹cy uzyska w³asnoœæ 

	if (FAILED(status))
		hr = status;

	if (SUCCEEDED(hr))
	{
		if (sample)
		{
			hr = sample->GetBufferByIndex																								//Pobieranie ramki buffera z próbki
			(
				0,																														//Index buffera 
				&MediaBuffer																											//Otrzymanie wskaŸnika na IMFMediaBuffer interface
			);																				
			
			//Rysowanie ramki
			if (SUCCEEDED(hr))
			{
				BYTE* data;
				MediaBuffer->Lock																										//Pozwala funkcji wywo³uj¹cej na dostêp do pamiêci w bufforze, do odczytu lub zapisu
				(
					&data,																												//Otrzymuje wskaŸnik do pocz¹tku buffora
					NULL,																												//maksymalny rozmiar danych które mog¹ byæ zapisane do buffera
					NULL																												//D³ugoœæ poprawnych danych w bufforze w bajtach
				);																					
				//To jest dobre miejsce aby zmieniæ kolory do rysowania
				//Dla próby w tym momencie ja te dane próbujê po prostu skopiowaæ 
				CopyMemory
				(
					rawData,																											//Gdzie maj¹ byæ kopiowane dane
					data,																												//Sk¹d te dane bêd¹ kopiowane
					width*height * bytesPerPixel																						//Rozmar bloku pamiêci (szerokosæ razy wysokoœæ razy ile bajtów jest przeznaczonych na jeden pixel)
				);

			}
		}
	}
	//ProŸba o kolejn¹ próbkê
	if (SUCCEEDED(hr))
		hr = sourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);

	if (FAILED(hr))
	{
		// Chcia³bym aby zróci³o mi miejsce gdzie jest b³¹d
		printf("Error HRESULT = 0x%d", hr);
		PostMessage(NULL, 1, (WPARAM)hr, 0L);
	}
	if (MediaBuffer) { MediaBuffer->Release(); MediaBuffer = NULL; }

	LeaveCriticalSection(&criticalSection);
	return hr;
}
STDMETHODIMP Media::OnEvent(DWORD, IMFMediaEvent *) { return S_OK; }

STDMETHODIMP Media::OnFlush(DWORD) { return S_OK; }