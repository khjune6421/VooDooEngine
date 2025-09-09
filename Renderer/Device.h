#pragma once

// Base header files
#include <Windows.h>
#include <vector>
#include <wrl/client.h>

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

// I usually don't use 'using' or #define macro in header files but I'll make this one an exception
using Microsoft::WRL::ComPtr;

extern HWND g_hWnd;

namespace VDD
{
	// Structure for hardware information such as GPU and monitor
	struct HardwareInfo
	{
		UINT adapterIndex = 0;
		DXGI_ADAPTER_DESC1 adapterDesc = {};
		std::vector<std::pair<UINT, DXGI_OUTPUT_DESC>> outputDescs = {};
	};

	// Device information structure containing Direct3D device, context, hardware, etc
	struct DeviceInfo
	{
		ComPtr<ID3D11Device> device = nullptr;
		ComPtr<ID3D11DeviceContext> context = nullptr;
		ComPtr<IDXGISwapChain> swapChain = nullptr;
		ComPtr<ID3D11RenderTargetView> renderTargetView = nullptr;
		ComPtr<ID3D11Texture2D> depthStencilBuffer = nullptr;
		ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;

		bool isFullscreen = false;
		bool isVSync = false;
		DWORD antiAliasingLevel = 4;

		DXGI_MODE_DESC displayMode = { 1920, 1080, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;

		std::vector<HardwareInfo> hardwareInfos = {};
	};

	extern DeviceInfo g_deviceInfo;

	void Initialize();
	void Release();

	template <typename T = float>
	T GetdeltaTime()
	{
		static ULONGLONG previousTime = GetTickCount64();
		ULONGLONG currentTime = GetTickCount64();

		double deltaTime = static_cast<double>(currentTime - previousTime) / 1000.0;
		previousTime = currentTime;

		return static_cast<T>(deltaTime);
	}

	void ShowFrameRate();
	
	// Clear the back buffer without depth stencil buffer
	inline void ClearBackBuffer(DirectX::XMFLOAT4 color) { g_deviceInfo.context->ClearRenderTargetView(g_deviceInfo.renderTargetView.Get(), reinterpret_cast<const float*>(&color)); }
	// Clar the back buffer and depth stencil buffer
	void ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth = 1.0f, UINT8 stencil = 0);
	inline void PresentBackBuffer() { g_deviceInfo.swapChain->Present(g_deviceInfo.isVSync, 0); }

	void CreateInputLayout(ComPtr<ID3D11Device> device, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements, ComPtr<ID3DBlob> shaderCode, _Out_ ComPtr<ID3D11InputLayout>* inputLayout);
	
	// Create Vertex and index buffer
	void CreateVertexBuffer(ComPtr<ID3D11Device> device, UINT size, _Out_ ComPtr<ID3D11Buffer>* buffer, const void* initData, UINT stride);
	void CreateConstBuffer(ComPtr<ID3D11Device> device, UINT size, _Out_ ComPtr<ID3D11Buffer>* buffer);

	inline DeviceInfo* GetDeviceInfo() { return &g_deviceInfo; }
	void DisplayDeviceInfo();

	void LoadFont();
	void DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), float scale = 1.0f, const wchar_t* font = L"Gugi");
}