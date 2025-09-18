#pragma once

// Base header files
#include <Windows.h>
#include <vector>
#include <wrl/client.h>
#include <memory>
#include <unordered_map>
#include <filesystem>

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

// Other header files
#include "Camera.h"

// I usually don't use 'using' or #define macro in header files but I'll make this one an exception
#define comPtr Microsoft::WRL::ComPtr

extern Camera g_camera;

enum class Shapes
{
	Triangle,
	Square,
	Plane,

	Tetrahedron,
	Cube,

	Tree,
	WindmillWing
};

class Object;

namespace VDGM
{
	extern std::vector<Object*> g_objects;

	extern float g_deltaTimeF;
	extern double g_deltaTimeD;
}

class Render
{
	// Structs and Enums

	// Device
	struct HardwareInfo
	{
		UINT adapterIndex = 0;
		DXGI_ADAPTER_DESC1 adapterDesc = {};
		std::vector<std::pair<UINT, DXGI_OUTPUT_DESC>> outputDescs = {};
	};

	// Device information structure containing Direct3D device, context, hardware, etc
	struct DeviceInfo
	{
		DXGI_MODE_DESC displayMode = { 1920, 1080, { 0, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;

		bool isFullscreen = false;
		bool isVSync = false;
		DWORD antiAliasingLevel = 4;

		std::vector<HardwareInfo> hardwareInfos = {};
	};

	// Shader
	struct TestConstBuffer
	{
		DirectX::XMMATRIX world; // world matrix
		DirectX::XMMATRIX view; // view matrix
		DirectX::XMMATRIX projection; // projection matrix
		DirectX::XMMATRIX WVP; // world-view-projection matrix

		float VSFloatA;
		float VSFloatB;
		float VSFloatC;
		float VSFloatD;
	};

	// Render
	enum class RasterState
	{
		Wireframe_CullNone = 0,
		Wireframe_CullBack = 1,
		Solid_CullNone = 2,
		Solid_CullBack = 3
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	// Variables
	HWND m_hWnd = nullptr;

	// Device
	DeviceInfo m_deviceInfo = {};

	comPtr<ID3D11Device> m_device = nullptr;
	comPtr<ID3D11DeviceContext> m_deviceContext = nullptr;
	comPtr<IDXGISwapChain> m_swapChain = nullptr;
	comPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;
	comPtr<ID3D11Texture2D> m_depthStencilBuffer = nullptr;
	comPtr<ID3D11DepthStencilView> m_depthStencilView = nullptr;

	UINT m_DXVersion = 0;
	UINT m_DXSubVersion = 0;

	// Need to make this global variable later // if possible
	std::unordered_map<std::wstring, std::unique_ptr<DirectX::SpriteBatch>> m_SpriteBatchMap;
	std::unordered_map<std::wstring, std::unique_ptr<DirectX::SpriteFont>> m_SpriteFontMap;

	// Shader
	comPtr<ID3D11VertexShader> m_vertexShader = nullptr;
	comPtr<ID3DBlob> m_VSCode = nullptr;
	comPtr<ID3D11PixelShader> m_pixelShader = nullptr;
	comPtr<ID3D11Buffer> m_constantBuffer = nullptr;

	// Render
	comPtr<ID3D11RasterizerState> g_rasterState[4] = { nullptr, nullptr, nullptr, nullptr }; // 0: Wireframe CullNone, 1: Wireframe CullBack, 2: Solid CullNone, 3: Solid CullBack
	RasterState m_currentRasterState = RasterState::Solid_CullNone;

	comPtr<ID3D11InputLayout> m_inputLayout = nullptr;

	// Static vertex buffer for rendering objects
	static std::unordered_map<Shapes, std::pair<comPtr<ID3D11Buffer>, UINT>> s_shapeVertexBuffers;

	// Functions

	// Device
	void CreateDeviceSwapChain();
	void CreateRenderTarget();
	void CreateDepthStencil();
	void LoadFonts();
	void GetHardwareInfo();

	void ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth = 1.0f, UINT8 stencil = 0);
	void Present() { m_swapChain->Present(m_deviceInfo.isVSync, 0); }

	void CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements, comPtr<ID3DBlob> shaderCode, _Out_ comPtr<ID3D11InputLayout>* inputLayout);

	void CreateVertexBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer, const void* initData, UINT stride);
	void CreateConstBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer);

	void DisplayDeviceInfo();

	// Shader
	void CreateShaders();
	void UpdateShaders();

	void LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, _Out_ comPtr<ID3D11VertexShader>* vertexShader, _Out_ comPtr<ID3DBlob>* VSCode);
	void LoadPixelShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, _Out_ comPtr<ID3D11PixelShader>* pixelShader);

	// Render
	void CreateRasterState();
	void SetInputLayout();
	float EngineUpdate();

	void DrawObjects();

	void UpdateRenderMode();

public:
	Render(HWND hWnd, int width, int height);
	~Render();

	void CreateShapeVertexBuffer();

	void Resize(UINT width, UINT height);
	void SetViewport(float topLeftX = 0.0f, float topLeftY = 0.0f);

	void DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), float scale = 1.0f, const wchar_t* fontName = L"Gugi");

	void SceneRender();

	void ChangeShader(UINT id);
	void ChangeState();
};

#undef comPtr