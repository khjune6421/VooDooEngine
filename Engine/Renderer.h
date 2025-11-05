#pragma once
#include "pch.h"

#include "Scene.h"

// I usually don't use 'using namespace' or #define macro in header files but I'll make this one an exception
#define comPtr Microsoft::WRL::ComPtr

namespace VDGM
{
	extern float g_deltaTime;

	// this is so cursed // TODO: not this
	extern std::unique_ptr<class Scene> g_currentScene;
}

class Renderer
{
	friend class Scene;
	friend class Shape;
	friend class PointLight;

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
	enum ConstBufferType
	{
		MatrixBuffer,
		AmbientLightBuffer,
		CameraBuffer,
		AmbientFogBuffer,
		DirectionalLightBuffer,
		PointLightBuffer,
		LightPosBuffer,

		ConstBufferCount
	};
	comPtr<ID3D11Buffer> m_constBuffers[ConstBufferCount] = {};

	// Input layouts // well this is cursed
	constexpr static UINT DEFAULT_LAYOUT_SIZE = 4;
	static D3D11_INPUT_ELEMENT_DESC s_defaultInputLayoutDesc[DEFAULT_LAYOUT_SIZE];

	static D3D11_SAMPLER_DESC s_defaultSamplerDesc;
	enum SamplerType
	{
		DefaultSampler,

		SamplerCount
	};
	comPtr<ID3D11SamplerState> m_samplers[SamplerCount] = {};

	enum BlendState
	{
		NoBlend,
		AlphaBlend,
		AlphaToCoverage,

		BlendStateCount
	};
	comPtr<ID3D11BlendState> m_blendStates[BlendStateCount] = {};

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

	// Maps shape ID to its vertex buffer and vertex count
	static UINT s_nextMeshId;
	std::unordered_map<UINT, std::pair<comPtr<ID3D11Buffer>, UINT>> m_meshVertexBufferMap;

	static UINT s_vertexShaderId;
	std::unordered_map<UINT, std::pair<comPtr<ID3D11VertexShader>, comPtr<ID3D11InputLayout>>> m_vertexShaderMap;

	static UINT s_geometryShaderId;
	std::unordered_map<UINT, comPtr<ID3D11GeometryShader>> m_geometryShaderMap;

	static UINT s_pixelShaderId;
	std::unordered_map<UINT, comPtr<ID3D11PixelShader>> m_pixelShaderMap;

	static UINT s_textureId;
	std::unordered_map<UINT, comPtr<ID3D11ShaderResourceView>> m_textureMap;

	// Render
	enum RasterState
	{
		Solid,
		Wireframe,

		RasterStateCount
	};
	comPtr<ID3D11RasterizerState> g_rasterState[RasterStateCount] = {};
	RasterState m_currentRasterState = RasterState::Solid;

	static constexpr UINT SHADOW_MAP_SIZE = 1024;
	comPtr<ID3D11Texture2D> m_shadowMapTexture = nullptr;
	comPtr<ID3D11DepthStencilView> m_shadowMapDSVs[6] = {};
	comPtr<ID3D11ShaderResourceView> m_shadowMapSRV = nullptr;
	comPtr<ID3D11SamplerState> m_shadowSampler = nullptr;

	// Functions

	// Device
	void SetScissorRect(LONG width, LONG height);
	void LoadFonts(const std::filesystem::path fontPath);
	void GetHardwareInfo();

	void ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth = 1.0f, UINT8 stencil = 0);

	void CreateVertexBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer, const void* initData, UINT stride);
	void CreateConstBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer);

	void ShowFPS();
	void DisplayDeviceInfo();

	// Shader
	void InitializeConstBuffers();
	void LoadAllShaders(const std::filesystem::path shaderPath, const char* entryPoint, const char* shaderModel);
	void LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel);
	void LoadGeometryShader(const wchar_t* file, const char* entryPoint, const char* shaderModel);
	void LoadPixelShader(const wchar_t* file, const char* entryPoint, const char* shaderModel);

	void LoadAllTextures(const std::filesystem::path texturePath);

	// Render
	void CreateDeviceSwapChain();
	void CreateRenderTarget();
	void CreateDepthStencil();
	void CreateRasterState();
	void CreateSamplerState();
	void CreateBlendState();
	void CreateShadowMap();
	void CreateShadowSampler();

	void LoadObjFile(const std::filesystem::path filePath);
	void LoadDefaultShapes(const std::filesystem::path folderPath);

	void UpdateRenderer();
	void UpdateVSConstBuffers();
	void UpdatePSConstBuffers();

	void UpdateRenderMode();

public:
	Renderer(HWND hWnd, LONG width, LONG height, const wchar_t* resourcePath = nullptr);
	~Renderer();
	Renderer(const Renderer& other) = default;
	Renderer& operator=(const Renderer& other) = default;
	Renderer(Renderer&& other) noexcept = default;
	Renderer& operator=(Renderer&& other) noexcept = default;

	void Resize(UINT width, UINT height);
	void SetViewport(float topLeftX = 0.0f, float topLeftY = 0.0f);

	void DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), float scale = 1.0f, const wchar_t* fontName = L"Gugi");

	void Render();

	void ChangeState();

	void ScreenPointToWorld(POINT screenPos) const;
};

#undef comPtr