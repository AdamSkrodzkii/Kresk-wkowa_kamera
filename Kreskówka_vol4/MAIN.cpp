#include <windows.h>
#include <stdio.h>
#include "Media.h"
#include "Camera.h"
#include "Pictures.h"

#include <gdiplus.h>									//Gdi jest bibliotek¹ bazuj¹c¹ na WIndows API

#define no_init_all = NULL;

LRESULT CALLBACK ProceduraOkna(HWND, UINT, UINT, LONG); // deklaracja zapowiadaj¹ca

int WINAPI WinMain (									//Tworzenie (main) okienka aplikacji
	HINSTANCE hInstance,								//Uchwyt (handler) do prywatnych struktur windowsa przez wskaŸnik (to jest taki void * )
	HINSTANCE hPrevInstance,							//Nazwy uchwytów zaczynaj¹ siê od du¿ej literki H
	LPSTR lpszCmdParam,									//Literka P lub LP (poiner lub long poiner) oznacza wskaŸnik str to char * zakoñczony bajtem zerowym.
														//Przechowywane s¹ argumenty wywo³ania tego programu
	int nCmdShow										//Informuje program w jaki sposób powinno pojawiæ siê g³ówne okno aplikacji
	)
{
	Gdiplus::GdiplusStartupInput gdiplusstartupinput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusstartupinput,nullptr);

	
	char Camera_name[2024];
	
	char szClassName[] = "Kreskówkowa kamera";

	HWND hwnd;											//Podaje uchwyt okna, do którego kierowany jest dany komunikat.
	MSG msg;
	Media* m = new Media();
	m->CreateCaptureDevice();
	wsprintf(Camera_name, "Kamera do której siê po³¹czy³em to: %ls o rozdzielczoœci %d x %d", m->deviceNameString, m->width, m->height);

	WNDCLASSEX  wndclass;								//Rejestrujemy klasê okna, po czym wykorzystujemy j¹ do utworzenia i wyœwietlenia na ekranie nowego okna
														//Zawiera niezbêdne do rejestracji dane. Zgodnie z poni¿sz¹ deklaracj¹, posiada ona a¿ 12 pól, które wype³niamy na pocz¹tku funkcji WinMain.

	wndclass.cbSize = sizeof(WNDCLASSEX);				//Rozmiar struktury WNDCLASSEX						{UINT}
	wndclass.style = CS_HREDRAW | CS_VREDRAW;			//Podstawowy styl okienek danej klasy				{UINT}
	wndclass.lpfnWndProc = ProceduraOkna;				//Adres procedury okna								{WNDPROC}
	wndclass.cbClsExtra = 0;							//Iloœæ dodatkowych bajtów przydzielanych klasie	{int}
	wndclass.cbWndExtra = 0;							//Iloœæ dodatk. bajtów przydzielanych ka¿demu oknu	{int}
	wndclass.hInstance = hInstance;						//Uchwyt wyst¹pienia programu						{HANDLE}
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);		//Uchwyt kursora									{HCURSOR}
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//Uchwyt ikony										{HICON}

	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //uchwyt pêdzla u¿ywanego do zamalowywania t³a {HBRUSH}

	wndclass.lpszMenuName = NULL;						//Nazwa menu										{LPCTSTR}
	wndclass.lpszClassName = szClassName;				//nazwa klasy okna									{LPCTSTR}
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); //uchwyt ma³ej ikonki								{HICON}

	RegisterClassEx(&wndclass);							//Rejestracja klasy

	hwnd = CreateWindowEx
	(
		0,												//Dodatkowy styl tworzonego okna									{DWORD}
		szClassName,									//Nazwa zarejestrowanej (lub predefiniowanej) klasy					{LPCTSTR}
		Camera_name,									//Tytu³ okienka. Pojawi siê na pasku tytu³owym okna					{LPCTSTR}
		WS_OVERLAPPEDWINDOW,							//Podstawowy styl okna												{DWORD}
		CW_USEDEFAULT,									//Wspó³rzêdna lewej krawêdzi okna									{INT}
		CW_USEDEFAULT,									//Wspó³rzêdna górnej krawêdzi okna									{INT}
		CW_USEDEFAULT,									//Szerokoœæ okna													{INT}
		CW_USEDEFAULT,									//Wysokoœæ okna														{INT}
		NULL,											//Uchwyt do okna-rodzica lub okna-w³aœciciela						{HWND}
		NULL,											//Uchwyt menu lub identyfikator okna, jeœli tworzymy okno potomne	{HMENU}
		hInstance,										//Uchwyt wyst¹pienia programu										{HINSTANCE}
		NULL											//WskaŸnik do danych u¿ytkownika u¿ywanych do inicjalizacji okna	{LPVOID}
	);
	Pictures* pics = new Pictures(hwnd, m->width, m->height, nCmdShow);

	if (hwnd == 0) return -1;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	/* Pêtla komunikatów: */

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
		HDC hdc = BeginPaint(hwnd, &ps);				//Uchwyt do urz¹dzenia. Funkcja BeginPaint przygotowuje okreœlone okno do malowania i wype³nia strukturê PAINTSTRUCT informacjami o malowaniu.
		GetClientRect(hwnd, &rect);						//Pobiera wspó³rzêdne obszaru roboczego okna.Wspó³rzêdne klienta okreœlaj¹ lewy górny i prawy dolny róg obszaru roboczego.

		Gdiplus::Graphics gf(hdc);
		DrawText(hdc, "jakiœ tekst", -1, &rect,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);

														// @@@@ LAMA zmieñ t¹ œcie¿kê @@@@ //
		Gdiplus::Bitmap bmp(L"C://Users/Adam/Desktop/Projekty_Mechatro/Niskopoziomowe/Kreskówkowa_kamera/Kreskówka_vol4/Kreskówka_vol4/indeks.jpg");
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