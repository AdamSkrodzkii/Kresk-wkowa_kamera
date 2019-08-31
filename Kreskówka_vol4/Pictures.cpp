#include "Pictures.h"

Pictures::Pictures(HWND window, int width, int height, int cmd)
{
	factory = NULL;
	d2dBitmap = NULL;
	renderTarget = NULL;

	//Ustawianie Direct2D
	if (renderTarget == NULL)
	{
		D2D1CreateFactory																																//Tworzy "factory object", kt�rego mo�na u�y� do utworzenia zasob�w Direct2D.
		(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,																											//Model w�tk�w "factory' i tworzone przez ni� zasoby. Brak synchronizacji w celu uzyskania dost�pu lub zapisu do "factory" lub tworzonych przez ni� obiekt�w.
			&factory																																	//Gdy ta metoda powr�ci, zawiera adres wska�nika do nowej fabryki.
																																						//Mo�na dodatkowo wyznaczy� te� poziom szczeg�owo�ci dostarczany debuggerowi oraz odniesienie do IID ID2D1Factory
		);

		D2D1_SIZE_U size = D2D1::SizeU(width, height);																									//Przechowuje uporz�dkowan� par� liczb ca�kowitych, zwykle szeroko�� i wysoko�� prostok�ta.
		D2D1_RENDER_TARGET_PROPERTIES rtProperties = D2D1::RenderTargetProperties();																	//Zawiera HWND, rozmiar w pikselach i opcje prezentacji dla ID2D1HwndRenderTarget.
		
		rtProperties.pixelFormat = D2D1::PixelFormat																									//pixelFormat (ma�� liter�) to jest rekord struktury d2d1_render(..) a z du�ej litery PIxelFormat to funkcja tworzy format pixeli. Direct2D powinien wybra� pixel format i alpha mode za nas.
		(
			DXGI_FORMAT_B8G8R8A8_UNORM,																													//Warto�� okre�laj�ca rozmiar i rozmieszczenie kana��w w ka�dym pikselu.
			D2D1_ALPHA_MODE_IGNORE																														//Warto��, kt�ra okre�la, czy kana� alfa u�ywa wst�pnie pomno�onej alfa, czy prostej alfa, czy te� nale�y j� zignorowa� i uzna� za nieprzezroczyst�.
		);
		rtProperties.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;																					//Warto�� okre�laj�ca spos�b, w jaki cel renderowania jest zdalnie sterowany i czy powinien on by� zgodny z GDI.
		
		factory->CreateHwndRenderTarget																													//Tworzy ID2D1HwndRenderTarget, cel renderowania jest renderowany do okna.
		(
			rtProperties,																																//Tryb renderowania, format pikseli, opcje zdalnego sterowania, informacje DPI i minimalna obs�uga DirectX wymagana do renderowania sprz�towego.
			D2D1::HwndRenderTargetProperties(window, size),																								//Uchwyt do okna
			&renderTarget																																//Gdy ta metoda powraca, zawiera adres wska�nika do obiektu ID2D1HwndRenderTarget utworzonego t� metod�.
		);

		renderTarget->CreateBitmap																														//Tworzenie bit mapy
		(
			size,
			D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&d2dBitmap
		);
	}
}
Pictures::~Pictures()
{
	if (factory)
	{
		factory->Release();
		factory = NULL;
	}
	if (renderTarget)
	{
		renderTarget->Release();
		renderTarget = NULL;
	}
	if (d2dBitmap)
	{
		d2dBitmap->Release();
		d2dBitmap = NULL;
	}

}
void Pictures::Draw(BYTE* pixels, int width, int bytesPerPixel)
{
	d2dBitmap->CopyFromMemory																															//Kopiuje okre�lony region z pami�ci do mapy bitowej.
	(
		NULL,																																			//W bitmapie lewy g�rny r�g obszaru, do kt�rego kopiowany jest region okre�lony przez srcRect.
		pixels,																																			//Dane do kopiowania
		width * bytesPerPixel																															//Krok lub wysoko�� �r�d�owej bitmapy przechowywanej w srcData. Krokiem jest liczba bajt�w linii skanowania (jeden rz�d pikseli w pami�ci). Krok mo�na obliczy� na podstawie nast�puj�cego wzoru: szeroko�� w pikselach * bajt�w na piksel + wype�nienie pami�ci.
	);
	renderTarget->BeginDraw();
	renderTarget->DrawBitmap(d2dBitmap);
	renderTarget->EndDraw();
}