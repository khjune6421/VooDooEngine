#pragma once

class Scene;

constexpr std::array<DirectX::XMVECTOR, 6> CUBE_TARGET =
{
	DirectX::XMVECTOR{ 1.0f, 0.0f, 0.0f, 0.0f },	// +X
	DirectX::XMVECTOR{ -1.0f, 0.0f, 0.0f, 0.0f },	// -X
	DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f },	// +Y
	DirectX::XMVECTOR{ 0.0f, -1.0f, 0.0f, 0.0f },	// -Y
	DirectX::XMVECTOR{ 0.0f, 0.0f, 1.0f, 0.0f },	// +Z
	DirectX::XMVECTOR{ 0.0f, 0.0f, -1.0f, 0.0f }	// -Z
};

constexpr std::array<DirectX::XMVECTOR, 6> CUBE_TARGET_UP =
{
	DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f },	// +X
	DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f },	// -X
	DirectX::XMVECTOR{ 0.0f, 0.0f, -1.0f, 0.0f },	// +Y
	DirectX::XMVECTOR{ 0.0f, 0.0f, 1.0f, 0.0f },	// -Y
	DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f },	// +Z
	DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f }		// -Z
};

namespace VDGM
{
	extern float g_deltaTime;

	// this is so cursed
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
		DXGI_MODE_DESC displayMode = { 1920, 1080, { 0, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;

		bool isFullscreen = false;
		bool isVSync = false;
		DWORD antiAliasingLevel = 4;

		UINT m_fps = 0;

		std::vector<HardwareInfo> hardwareInfos = {};
	};
	enum ConstBufferType
	{
		MatrixBuffer,
		AmbientLightBuffer,
		CameraBuffer,
		AmbientFogBuffer,
		DirectionalLightBuffer,
		DirectionalLightShadow,
		PointLightBuffer,
		LightPosBuffer,

		ConstBufferCount
	};
	std::array<com_ptr<ID3D11Buffer>, ConstBufferCount> m_constBuffers = {};

	// Input layouts // well this is cursed
	constexpr static UINT DEFAULT_LAYOUT_SIZE = 4;
	const static std::array<D3D11_INPUT_ELEMENT_DESC, DEFAULT_LAYOUT_SIZE> s_defaultInputLayoutDesc;

	static const D3D11_SAMPLER_DESC s_defaultSamplerDesc;
	enum SamplerType
	{
		DefaultSampler,

		SamplerCount
	};
	std::array<com_ptr<ID3D11SamplerState>, SamplerCount> m_samplers = {};
	com_ptr<ID3D11SamplerState> m_shadowSampler = nullptr;

	enum BlendState
	{
		NoBlend,
		AlphaBlend,
		AlphaToCoverage,

		BlendStateCount
	};
	std::array<com_ptr<ID3D11BlendState>, BlendStateCount> m_blendStates = {};

	// Variables
	HWND m_hWnd = nullptr;

	// Device
	DeviceInfo m_deviceInfo = {};

	com_ptr<ID3D11Device> m_device = nullptr;
	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr;
	com_ptr<IDXGISwapChain> m_swapChain = nullptr;
	com_ptr<ID3D11RenderTargetView> m_renderTargetView = nullptr;
	com_ptr<ID3D11Texture2D> m_depthStencilBuffer = nullptr;
	com_ptr<ID3D11DepthStencilView> m_depthStencilView = nullptr;

	std::filesystem::path m_resourcePath = {};

	UINT m_DXVersion = 0;
	UINT m_DXSubVersion = 0;

	// Need to make this global variable later // if possible
	std::unordered_map<std::wstring, std::unique_ptr<DirectX::SpriteBatch>> m_SpriteBatchMap;
	std::unordered_map<std::wstring, std::unique_ptr<DirectX::SpriteFont>> m_SpriteFontMap;

	// Maps shape ID to its vertex buffer and vertex count
	static UINT s_nextMeshId;
	std::unordered_map<UINT, std::pair<com_ptr<ID3D11Buffer>, UINT>> m_meshVertexBufferMap;

	static UINT s_vertexShaderId;
	std::unordered_map<UINT, std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>>> m_vertexShaderMap;

	static UINT s_geometryShaderId;
	std::unordered_map<UINT, com_ptr<ID3D11GeometryShader>> m_geometryShaderMap;

	static UINT s_pixelShaderId;
	std::unordered_map<UINT, com_ptr<ID3D11PixelShader>> m_pixelShaderMap;

	static UINT s_textureId;
	enum TextureType
	{
		Diffuse,
		Normal,

		TextureTypeCount
	};
	std::unordered_map<UINT, std::array<com_ptr<ID3D11ShaderResourceView>, TextureTypeCount>> m_textureMap;

	// Render
	enum RasterState
	{
		Solid,
		Wireframe,

		RasterStateCount
	};
	std::array<com_ptr<ID3D11RasterizerState>, RasterStateCount> m_rasterState;
	RasterState m_currentRasterState = RasterState::Solid;

	static constexpr UINT SHADOW_MAP_SIZE = 4096;
	static constexpr D3D11_VIEWPORT SHADOW_VIWEPORT =
	{
		0.0f, 0.0f,
		static_cast<FLOAT>(SHADOW_MAP_SIZE),
		static_cast<FLOAT>(SHADOW_MAP_SIZE),
		0.0f, 1.0f
	};
	static constexpr D3D11_RECT SHADOW_SCISSOR_REACT =
	{
		0, 0,
		static_cast<LONG>(SHADOW_MAP_SIZE),
		static_cast<LONG>(SHADOW_MAP_SIZE)
	};

	static constexpr UINT CUBE_SHADOW_MAP_SIZE = 1024;
	static constexpr D3D11_VIEWPORT CUBE_SHADOW_VIWEPORT =
	{
		0.0f, 0.0f,
		static_cast<FLOAT>(CUBE_SHADOW_MAP_SIZE),
		static_cast<FLOAT>(CUBE_SHADOW_MAP_SIZE),
		0.0f, 1.0f
	};
	static constexpr D3D11_RECT CUBE_SHADOW_SCISSOR_REACT =
	{
		0, 0,
		static_cast<LONG>(CUBE_SHADOW_MAP_SIZE),
		static_cast<LONG>(CUBE_SHADOW_MAP_SIZE)
	};

	com_ptr<ID3D11RenderTargetView> m_shadowMapArrayRTV = nullptr; // Baseically nullptr for now

	// Directional light shadow map
	com_ptr<ID3D11Texture2D> m_shadowMapTexture = nullptr;
	com_ptr<ID3D11DepthStencilView> m_shadowMapDSV = nullptr;
	com_ptr<ID3D11ShaderResourceView> m_shadowMapSRV = nullptr;

	// Cube shadow map for point lights
	com_ptr<ID3D11Texture2D> m_cubeShadowMapArrayTexture = nullptr;
	std::vector<com_ptr<ID3D11DepthStencilView>> m_cubeShadowMapDSVs{ CUBE_SHADOW_MAP_SIZE * 6 };
	com_ptr<ID3D11ShaderResourceView> m_cubeShadowMapArraySRV = nullptr;
	bool m_shouldUpdateLights = true;

	// Functions

	// Device
	void SetScissorRect(LONG width, LONG height);
	void LoadFonts(const std::filesystem::path fontPath);
	void GetHardwareInfo();

	void ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth = 1.0f, UINT8 stencil = 0);

	void CreateVertexBuffer(UINT size, _Out_ com_ptr<ID3D11Buffer>* buffer, const void* initData, UINT stride);
	void CreateConstBuffer(UINT size, _Out_ com_ptr<ID3D11Buffer>* buffer);

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
	void CreateCubeShadowMap();
	void CreateShadowSampler();

	void LoadObjFile(const std::filesystem::path filePath);
	void LoadDefaultShapes(const std::filesystem::path folderPath);

	void ClearResources();
	void UpdateRenderer();
	void UpdateVertexShader();
	void UpdatePixelShader();

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
	void SaveShadowMapToFile(com_ptr<ID3D11Texture2D> texture, const std::wstring& filename) const;
};