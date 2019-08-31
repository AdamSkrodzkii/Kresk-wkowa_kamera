#pragma once
#include <Windows.h>
#include <d2d1.h>											//Nag��wek u�ywany przez Direct2D
#pragma comment(lib,"d2d1")

class Pictures
{
	//D2D sta�e
	ID2D1Factory* factory;									//Tworzy zasoby D2D
	ID2D1HwndRenderTarget* renderTarget;					//Renderuje instrukcje rysowania do okna
	ID2D1Bitmap* d2dBitmap;									//Reprezentuje BITMAP� powi�zan� z renderTarget

public:
	Pictures(HWND window, int width, int height, int cmd);
	~Pictures();
	void Draw(BYTE* pixels, int width, int bytesPerPixel);
};