#pragma once
#include <Windows.h>
#include <dshow.h>

#pragma comment(lib, "strmiids")
class Camera
{
public:
	HRESULT CaptureCamera();
	Camera();
	~Camera();
};

