#include "Pictures.h"

RenderingWindow::RenderingWindow(HWND window, int width, int height, int cmd)
{
	factory = NULL;
	d2dBitmap = NULL;
	renderTarget = NULL;

	//Initialize D2D
	if (renderTarget == NULL)
	{
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		D2D1_RENDER_TARGET_PROPERTIES rtProperties = D2D1::RenderTargetProperties();
		rtProperties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProperties.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
		factory->CreateHwndRenderTarget(rtProperties, D2D1::HwndRenderTargetProperties(windowHandle, size), &renderTarget);
		renderTarget->CreateBitmap(size, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&d2dBitmap);
	}
}
RenderingWindow::~RenderingWindow()
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
void RenderingWindow::Draw(BYTE* pixels, int width, int height)
{
	d2dBitmap->CopyFromMemory(NULL, pixels, width * 4);
	renderTarget->BeginDraw();
	renderTarget->DrawBitmap(d2dBitmap);
	renderTarget->EndDraw();
}