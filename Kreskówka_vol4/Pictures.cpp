#include "Pictures.h"

Pictures::Pictures(HWND window, int width, int height, int cmd)
{
	factory = NULL;
	d2dBitmap = NULL;
	renderTarget = NULL;

	//Ustawianie Direct2D
	if (renderTarget == NULL)
	{
		D2D1CreateFactory																																//Tworzy "factory object", którego mo¿na u¿yæ do utworzenia zasobów Direct2D.
		(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,																											//Model w¹tków "factory' i tworzone przez ni¹ zasoby. Brak synchronizacji w celu uzyskania dostêpu lub zapisu do "factory" lub tworzonych przez ni¹ obiektów.
			&factory																																	//Gdy ta metoda powróci, zawiera adres wskaŸnika do nowej fabryki.
																																						//Mo¿na dodatkowo wyznaczyæ te¿ poziom szczegó³owoœci dostarczany debuggerowi oraz odniesienie do IID ID2D1Factory
		);

		D2D1_SIZE_U size = D2D1::SizeU(width, height);																									//Przechowuje uporz¹dkowan¹ parê liczb ca³kowitych, zwykle szerokoœæ i wysokoœæ prostok¹ta.
		D2D1_RENDER_TARGET_PROPERTIES rtProperties = D2D1::RenderTargetProperties();																	//Zawiera HWND, rozmiar w pikselach i opcje prezentacji dla ID2D1HwndRenderTarget.
		
		rtProperties.pixelFormat = D2D1::PixelFormat																									//pixelFormat (ma³¹ liter¹) to jest rekord struktury d2d1_render(..) a z du¿ej litery PIxelFormat to funkcja tworzy format pixeli. Direct2D powinien wybraæ pixel format i alpha mode za nas.
		(
			DXGI_FORMAT_B8G8R8A8_UNORM,																													//Wartoœæ okreœlaj¹ca rozmiar i rozmieszczenie kana³ów w ka¿dym pikselu.
			D2D1_ALPHA_MODE_IGNORE																														//Wartoœæ, która okreœla, czy kana³ alfa u¿ywa wstêpnie pomno¿onej alfa, czy prostej alfa, czy te¿ nale¿y j¹ zignorowaæ i uznaæ za nieprzezroczyst¹.
		);
		rtProperties.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;																					//Wartoœæ okreœlaj¹ca sposób, w jaki cel renderowania jest zdalnie sterowany i czy powinien on byæ zgodny z GDI.
		
		factory->CreateHwndRenderTarget																													//Tworzy ID2D1HwndRenderTarget, cel renderowania jest renderowany do okna.
		(
			rtProperties,																																//Tryb renderowania, format pikseli, opcje zdalnego sterowania, informacje DPI i minimalna obs³uga DirectX wymagana do renderowania sprzêtowego.
			D2D1::HwndRenderTargetProperties(window, size),																								//Uchwyt do okna
			&renderTarget																																//Gdy ta metoda powraca, zawiera adres wskaŸnika do obiektu ID2D1HwndRenderTarget utworzonego t¹ metod¹.
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
	d2dBitmap->CopyFromMemory																															//Kopiuje okreœlony region z pamiêci do mapy bitowej.
	(
		NULL,																																			//W bitmapie lewy górny róg obszaru, do którego kopiowany jest region okreœlony przez srcRect.
		pixels,																																			//Dane do kopiowania
		width * bytesPerPixel																															//Krok lub wysokoœæ Ÿród³owej bitmapy przechowywanej w srcData. Krokiem jest liczba bajtów linii skanowania (jeden rz¹d pikseli w pamiêci). Krok mo¿na obliczyæ na podstawie nastêpuj¹cego wzoru: szerokoœæ w pikselach * bajtów na piksel + wype³nienie pamiêci.
	);
	renderTarget->BeginDraw();
	renderTarget->DrawBitmap(d2dBitmap);
	renderTarget->EndDraw();
}