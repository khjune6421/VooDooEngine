#pragma once

// Base header files
#include <Windows.h>
#include <vector>

// DirectX header files
#include <d3d11.h>
#include <DirectXMath.h>

// DirectXTK header files
#include <SimpleMath.h>
#include <SpriteFont.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

// DirectX libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// DirectXTK libraries
#pragma comment(lib, "DirectXTK.lib")

extern HWND g_hWnd;

namespace VDR
{
	void Initialize();
	void Release();

	double GetdeltaTime();
	void ShowFrameRate();

	// Structure for hardware information such as GPU and monitor
	struct HardwareInfo
	{
		UINT adapterIndex = 0;
		DXGI_ADAPTER_DESC1 adapterDesc = {};
		std::vector<std::pair<UINT, DXGI_OUTPUT_DESC>> outputDescs = {};
	};

	// Device information structure containing Direct3D device, context, hardware, etc.
	struct DeviceInfo
	{
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		IDXGISwapChain* swapChain = nullptr;
		ID3D11RenderTargetView* renderTargetView = nullptr;
		ID3D11Texture2D* depthStencilBuffer = nullptr;
		ID3D11DepthStencilView* depthStencilView = nullptr;

		bool isFullscreen = false;
		bool isVSync = false;
		DWORD antiAliasingLevel = 4;

		DXGI_MODE_DESC displayMode = { 1920, 1080, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;

		std::vector<HardwareInfo> hardwareInfos = {};
	};

	extern DeviceInfo g_DeviceInfo;

	inline void ClearBackBuffer(DirectX::XMFLOAT4 color) { g_DeviceInfo.context->ClearRenderTargetView(g_DeviceInfo.renderTargetView, reinterpret_cast<const float*>(&color)); }
	void ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth = 1.0f, UINT8 stencil = 0);

	inline void PresentBackBuffer() { g_DeviceInfo.swapChain->Present(g_DeviceInfo.isVSync, 0); }

	void CreateBuffer(ID3D11Device* device, UINT size, _Out_ ID3D11Buffer** buffer, const void* initData = nullptr, UINT stride = 0);

	inline DeviceInfo* GetDeviceInfo() { return &g_DeviceInfo; }
	void DisplayDeviceInfo();

	void LoadFont();
	void DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), float scale = 1.0f, const wchar_t* font = L"Gugi");
}