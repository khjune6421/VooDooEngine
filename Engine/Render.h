#pragma once

// Standard header files
#include "UtilityHeaders.h"

// DirectX header files
#include "DirectXLib.h"

// Other header files
#include "Scene.h"

#include "Shape.h"
#include "Light.h"

// I usually don't use 'using' or #define macro in header files but I'll make this one an exception
#define comPtr Microsoft::WRL::ComPtr

class Object;
extern std::vector<std::pair<Object*, ShapeData>> g_renderShapes;
extern std::vector<std::pair<Object*, LightData>> g_lightDatas;

namespace VDGM
{
	extern float g_deltaTime;

	// this is so cursed // TODO: not this
	extern std::unique_ptr<Scene> g_currentScene;
}

class Render
{
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

	struct MatrixConstBuffer
	{
		DirectX::XMMATRIX world; // world matrix
		DirectX::XMMATRIX view; // view matrix
		DirectX::XMMATRIX projection; // projection matrix
		DirectX::XMMATRIX WVP; // world-view-projection matrix
	};
	struct LightConstBuffer
	{
		DirectX::XMVECTOR localPosition;
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 diffuseColor;

		float range;
		float intensity;
		float attenuation;
		int isBackfaceLighting;
	};
	struct AnimationConstBuffer
	{
		int currentShapeIndex;
		int nextShapeIndex;
		float interpolationFactor;

		float padding;
	};
	enum ConstBufferField
	{
		MatrixBuffer = 0,
		LightBuffer = 1,
		AnimationBuffer = 2
	};

	// 0: MatrixConstBuffer, 1: LightConstBuffer 2: AnimationConstBuffer
	comPtr<ID3D11Buffer> m_constBuffers[3] = {};

	// Input layouts // well this is cursed
	constexpr static UINT DEFAULT_LAYOUT_SIZE = 4;
	static D3D11_INPUT_ELEMENT_DESC s_defaultInputLayoutDesc[DEFAULT_LAYOUT_SIZE];
	constexpr static UINT TRIPLE_LAYOUT_SIZE = 12;
	static D3D11_INPUT_ELEMENT_DESC s_tripleInputLayoutDesc[TRIPLE_LAYOUT_SIZE];
	enum InputLayoutField
	{
		DefaultInputLayout = 0,
		TripleInputLayout = 1
	};

	static std::pair<D3D11_INPUT_ELEMENT_DESC*, UINT> s_layoutDescs[2];

	// Render
	enum class RasterState
	{
		Wireframe_CullNone = 0,
		Wireframe_CullBack = 1,
		Solid_CullNone = 2,
		Solid_CullBack = 3
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

	static UINT s_vertexShaderId;
	//std::unordered_map<UINT, std::pair<comPtr<ID3D11VertexShader>, comPtr<ID3D11InputLayout>>> m_vertexShaderMap;
	enum VertexShaderMapField
	{
		VertexShader = 0,
		ConstBuffers = 1,
		InputLayout = 2
	};
	std::unordered_map<UINT, std::tuple<comPtr<ID3D11VertexShader>, std::vector<UINT>, comPtr<ID3D11InputLayout>>> m_vertexShaderMap;

	static UINT s_pixelShaderId;
	std::unordered_map<UINT, comPtr<ID3D11PixelShader>> m_pixelShaderMap;

	// Render
	// 0: Wireframe CullNone, 1: Wireframe CullBack, 2: Solid CullNone, 3: Solid CullBack
	comPtr<ID3D11RasterizerState> g_rasterState[4] = { nullptr, nullptr, nullptr, nullptr };
	RasterState m_currentRasterState = RasterState::Solid_CullNone;

	// Maps shape ID to its vertex buffer and vertex count // I might change this to vector later // not sure if it's a good idea to make this unintuitive thing more complicated
	static UINT s_nextShapeId;
	std::unordered_map<UINT, std::pair<comPtr<ID3D11Buffer>, UINT>> m_shapeVertexBufferMap;

	// static view and projection matrix for all renders
	static DirectX::XMMATRIX s_viewMatrix;
	static DirectX::XMMATRIX s_projectionMatrix;

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

	void ShowFPS();
	void DisplayDeviceInfo();

	// Shader
	void LoadAllShaders(const std::filesystem::path shaderPath, const char* entryPoint, const char* shaderModel);
	//void LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, int layoutIndex = 0);
	void LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, const std::vector<UINT> constBufferIds = { MatrixBuffer, LightBuffer }, const int layoutIndex = DefaultInputLayout);
	void LoadPixelShader(const wchar_t* file, const char* entryPoint, const char* shaderModel);

	// Render
	void CreateRasterState();

	void CreateSampleShapes();
	void LoadShapeFile(const std::filesystem::path filePath);
	void LoadDefaultShapes(const std::filesystem::path folderPath);

	void EngineUpdate();

	void DrawObjects();

	void UpdateRenderMode();

public:
	Render(HWND hWnd, UINT width, UINT height, const wchar_t* resourcePath = nullptr);
	~Render();

	void Resize(UINT width, UINT height);
	void SetViewport(float topLeftX = 0.0f, float topLeftY = 0.0f);

	void DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), float scale = 1.0f, const wchar_t* fontName = L"Gugi");

	void SceneRender();

	void ChangeState();

	void ScreenPointToWorld(POINT screenPos) const;
};

#undef comPtr