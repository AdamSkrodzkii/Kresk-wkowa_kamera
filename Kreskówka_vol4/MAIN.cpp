#include <windows.h>
#include <stdio.h>
#include "Media.h"
#include "Camera.h"
#include "Pictures.h"

#include <gdiplus.h>									//Gdi jest bibliotek� bazuj�c� na WIndows API

#define no_init_all = NULL;

LRESULT CALLBACK ProceduraOkna(HWND, UINT, UINT, LONG); // deklaracja zapowiadaj�ca

int WINAPI WinMain (									//Tworzenie (main) okienka aplikacji
	HINSTANCE hInstance,								//Uchwyt (handler) do prywatnych struktur windowsa przez wska�nik (to jest taki void * )
	HINSTANCE hPrevInstance,							//Nazwy uchwyt�w zaczynaj� si� od du�ej literki H
	LPSTR lpszCmdParam,									//Literka P lub LP (poiner lub long poiner) oznacza wska�nik str to char * zako�czony bajtem zerowym.
														//Przechowywane s� argumenty wywo�ania tego programu
	int nCmdShow										//Informuje program w jaki spos�b powinno pojawi� si� g��wne okno aplikacji
	)
{
	Gdiplus::GdiplusStartupInput gdiplusstartupinput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusstartupinput,nullptr);

	
	char Camera_name[2024];
	
	char szClassName[] = "Kresk�wkowa kamera";

	HWND hwnd;											//Podaje uchwyt okna, do kt�rego kierowany jest dany komunikat.
	MSG msg;
	Media* m = new Media();
	m->CreateCaptureDevice();
	wsprintf(Camera_name, "Kamera do kt�rej si� po��czy�em to: %ls o rozdzielczo�ci %d x %d", m->deviceNameString, m->width, m->height);

	WNDCLASSEX  wndclass;								//Rejestrujemy klas� okna, po czym wykorzystujemy j� do utworzenia i wy�wietlenia na ekranie nowego okna
														//Zawiera niezb�dne do rejestracji dane. Zgodnie z poni�sz� deklaracj�, posiada ona a� 12 p�l, kt�re wype�niamy na pocz�tku funkcji WinMain.

	wndclass.cbSize = sizeof(WNDCLASSEX);				//Rozmiar struktury WNDCLASSEX						{UINT}
	wndclass.style = CS_HREDRAW | CS_VREDRAW;			//Podstawowy styl okienek danej klasy				{UINT}
	wndclass.lpfnWndProc = ProceduraOkna;				//Adres procedury okna								{WNDPROC}
	wndclass.cbClsExtra = 0;							//Ilo�� dodatkowych bajt�w przydzielanych klasie	{int}
	wndclass.cbWndExtra = 0;							//Ilo�� dodatk. bajt�w przydzielanych ka�demu oknu	{int}
	wndclass.hInstance = hInstance;						//Uchwyt wyst�pienia programu						{HANDLE}
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);		//Uchwyt kursora									{HCURSOR}
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//Uchwyt ikony										{HICON}

	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //uchwyt p�dzla u�ywanego do zamalowywania t�a {HBRUSH}

	wndclass.lpszMenuName = NULL;						//Nazwa menu										{LPCTSTR}
	wndclass.lpszClassName = szClassName;				//nazwa klasy okna									{LPCTSTR}
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); //uchwyt ma�ej ikonki								{HICON}

	RegisterClassEx(&wndclass);							//Rejestracja klasy

	hwnd = CreateWindowEx
	(
		0,												//Dodatkowy styl tworzonego okna									{DWORD}
		szClassName,									//Nazwa zarejestrowanej (lub predefiniowanej) klasy					{LPCTSTR}
		Camera_name,									//Tytu� okienka. Pojawi si� na pasku tytu�owym okna					{LPCTSTR}
		WS_OVERLAPPEDWINDOW,							//Podstawowy styl okna												{DWORD}
		CW_USEDEFAULT,									//Wsp�rz�dna lewej kraw�dzi okna									{INT}
		CW_USEDEFAULT,									//Wsp�rz�dna g�rnej kraw�dzi okna									{INT}
		CW_USEDEFAULT,									//Szeroko�� okna													{INT}
		CW_USEDEFAULT,									//Wysoko�� okna														{INT}
		NULL,											//Uchwyt do okna-rodzica lub okna-w�a�ciciela						{HWND}
		NULL,											//Uchwyt menu lub identyfikator okna, je�li tworzymy okno potomne	{HMENU}
		hInstance,										//Uchwyt wyst�pienia programu										{HINSTANCE}
		NULL											//Wska�nik do danych u�ytkownika u�ywanych do inicjalizacji okna	{LPVOID}
	);
	Pictures* pics = new Pictures(hwnd, m->width, m->height, nCmdShow);

	if (hwnd == 0) return -1;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	/* P�tla komunikat�w: */

	int result;
	while ((result = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (result == -1) return -1;
		pics->Draw(m->rawData, m->width,m->bytesPerPixel);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

/***   PROCEDURA OKNA   ***/

LRESULT CALLBACK ProceduraOkna(HWND hwnd, UINT message,
	UINT wParam, LONG lParam)

{
	switch (message)
	{
	case WM_PAINT:
	{
		
		PAINTSTRUCT ps;
		RECT        rect;
		HDC hdc = BeginPaint(hwnd, &ps);				//Uchwyt do urz�dzenia. Funkcja BeginPaint przygotowuje okre�lone okno do malowania i wype�nia struktur� PAINTSTRUCT informacjami o malowaniu.
		GetClientRect(hwnd, &rect);						//Pobiera wsp�rz�dne obszaru roboczego okna.Wsp�rz�dne klienta okre�laj� lewy g�rny i prawy dolny r�g obszaru roboczego.

		Gdiplus::Graphics gf(hdc);
		DrawText(hdc, "jaki� tekst", -1, &rect,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);

														// @@@@ LAMA zmie� t� �cie�k� @@@@ //
		Gdiplus::Bitmap bmp(L"C://Users/Adam/Desktop/Projekty_Mechatro/Niskopoziomowe/Kresk�wkowa_kamera/Kresk�wka_vol4/Kresk�wka_vol4/indeks.jpg");
		gf.DrawImage(&bmp, 1000, 400);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}