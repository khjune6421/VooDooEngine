#include "Renderer.h"

#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

#define comPtr Microsoft::WRL::ComPtr

UINT Renderer::s_nextShapeId = 0;
unordered_map<wstring, UINT> g_meshIdMap;

UINT Renderer::s_vertexShaderId = 0;
unordered_map<wstring, UINT> g_vertexShaderIdMap;

UINT Renderer::s_geometryShaderId = 0;
unordered_map<wstring, UINT> g_geometryShaderIdMap;

UINT Renderer::s_pixelShaderId = 0;
unordered_map<wstring, UINT> g_pixelShaderIdMap;

UINT Renderer::s_textureId = 0;
unordered_map<wstring, UINT> g_textureIdMap;

D3D11_INPUT_ELEMENT_DESC Renderer::s_defaultInputLayoutDesc[DEFAULT_LAYOUT_SIZE] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
pair<D3D11_INPUT_ELEMENT_DESC*, UINT> Renderer::s_layoutDescs[1] = { { Renderer::s_defaultInputLayoutDesc, DEFAULT_LAYOUT_SIZE } };

D3D11_SAMPLER_DESC Renderer::s_defaultSamplerDesc =
{
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	0.0f,
	8,
	D3D11_COMPARISON_NEVER,
	{ 1, 1, 1, 1 },
	0.0f,
	D3D11_FLOAT32_MAX
};

XMMATRIX Renderer::s_viewMatrix = XMMatrixIdentity();
XMMATRIX Renderer::s_projectionMatrix = XMMatrixIdentity();

void Renderer::CreateDeviceSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.Windowed = !m_deviceInfo.isFullscreen;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc = m_deviceInfo.displayMode;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = m_deviceInfo.isVSync * m_deviceInfo.displayMode.RefreshRate.Numerator;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.SampleDesc.Count = m_deviceInfo.antiAliasingLevel;

	if
		(
		FAILED
		(
			D3D11CreateDeviceAndSwapChain
			(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				0,
				&m_deviceInfo.featureLevels,
				1,
				D3D11_SDK_VERSION,
				&swapChainDesc,
				&m_swapChain,
				&m_device,
				nullptr,
				&m_deviceContext
			)
		)
		)
	{
		MessageBoxW(nullptr, L"Failed to create device and swap chain", L"Error", MB_OK);
		exit(-1);
	}
}

void Renderer::CreateRenderTarget()
{
	comPtr<ID3D11Texture2D> backBuffer;

	if (FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
	{
		MessageBoxW(nullptr, L"Failed to get back buffer", L"Error", MB_OK);
		exit(-1);
	}

	if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf())))
	{
		MessageBox(nullptr, L"Failed to create render target view", L"Error", MB_OK);
		exit(-1);
	}
}

void Renderer::CreateDepthStencil()
{
	m_depthStencilView.Reset();
	m_depthStencilBuffer.Reset();

	if (!m_device.Get())
	{
		MessageBoxW(nullptr, L"Device is null", L"Error", MB_OK);
		return;
	}
	if (m_deviceInfo.displayMode.Width == 0 || m_deviceInfo.displayMode.Height == 0)
	{
		MessageBoxW(nullptr, L"Invalid display size", L"Error", MB_OK);
		return;
	}

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = m_deviceInfo.displayMode.Width;
	depthStencilDesc.Height = m_deviceInfo.displayMode.Height;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = m_deviceInfo.antiAliasingLevel;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (FAILED(m_device->CreateTexture2D(&depthStencilDesc, nullptr, m_depthStencilBuffer.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create depth stencil texture", L"Error", MB_OK);
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = (m_deviceInfo.antiAliasingLevel > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create depth stencil view", L"Error", MB_OK);
		m_depthStencilBuffer.Reset();
		return;
	}
}

void Renderer::SetScissorRect(LONG width, LONG height)
{
	D3D11_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;
	m_deviceContext->RSSetScissorRects(1, &scissorRect);
}

void Renderer::LoadFonts()
{
	wstring fontPath = L"../Assets/Default/Fonts/";
	if (!filesystem::exists(fontPath))
	{
		MessageBoxW(nullptr, L"Font directory does not exist", L"Error", MB_OK);
		return;
	}
	for (const auto& entry : filesystem::directory_iterator(fontPath))
	{
		if (entry.path().extension() == L".spritefont")
		{
			wstring fontName = entry.path().stem().wstring();
			m_SpriteFontMap[fontName] = make_unique<SpriteFont>(m_device.Get(), entry.path().c_str());
			m_SpriteBatchMap[fontName] = make_unique<SpriteBatch>(m_deviceContext.Get());
		}
	}
}

void Renderer::GetHardwareInfo()
{
	comPtr<IDXGIAdapter1> padapter;
	comPtr<IDXGIFactory1> pfactory;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), &pfactory)))
	{
		MessageBoxW(nullptr, L"Failed to create DXGI factory", L"Error", MB_OK);
		return;
	}

	for (UINT adapterIndex = 0; pfactory->EnumAdapters1(adapterIndex, &padapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		HardwareInfo hardwareInfo = {};
		hardwareInfo.adapterIndex = adapterIndex;
		if (FAILED(padapter->GetDesc1(&hardwareInfo.adapterDesc)))
		{
			MessageBoxW(nullptr, L"Failed to get adapter description", L"Error", MB_OK);
			return;
		}

		comPtr<IDXGIOutput> poutput;
		for (UINT outputIndex = 0; padapter->EnumOutputs(outputIndex, &poutput) != DXGI_ERROR_NOT_FOUND; ++outputIndex)
		{
			DXGI_OUTPUT_DESC outputDesc = {};
			if (FAILED(poutput->GetDesc(&outputDesc)))
			{
				MessageBoxW(nullptr, L"Failed to get output description", L"Error", MB_OK);
				return;
			}
			hardwareInfo.outputDescs.emplace_back(outputIndex, outputDesc);
			poutput.Reset();
		}

		m_deviceInfo.hardwareInfos.push_back(hardwareInfo);
		padapter.Reset();
	}
}

void Renderer::ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth, UINT8 stencil)
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), reinterpret_cast<const float*>(&color));

	if (m_depthStencilView.Get() != nullptr) m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), flag, depth, stencil);
	else
	{
		MessageBoxW(nullptr, L"Failed to clear depth stencil view", L"Error", MB_OK);
		return;
	}
}

void Renderer::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements, comPtr<ID3DBlob> shaderCode, _Out_ comPtr<ID3D11InputLayout>* inputLayout)
{
	if (FAILED(m_device->CreateInputLayout(layoutDesc, numElements, shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), inputLayout->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create input layout", L"Error", MB_OK);
		return;
	}
}

void Renderer::CreateVertexBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer, const void* initData, UINT stride)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = initData;

	if (FAILED(m_device->CreateBuffer(&bufferDesc, &subresourceData, buffer->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create buffer", L"Error", MB_OK);
		return;
	}
}

void Renderer::CreateConstBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (FAILED(m_device->CreateBuffer(&bufferDesc, nullptr, buffer->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create constant buffer", L"Error", MB_OK);
		return;
	}
}

void Renderer::ShowFPS()
{
	static UINT frameCount = 0;
	static float elapsedTime = 0.0f;
	static float fps = 0.0f;

	frameCount++;

	elapsedTime += VDGM::g_deltaTime;

	if (elapsedTime >= 1.0)
	{
		fps = frameCount * elapsedTime;
		frameCount = 0;
		elapsedTime = 0.0;
	}

	wstring fpsText = L"FPS: " + to_wstring(static_cast<int>(fps));
	DrawText(fpsText.c_str(), XMFLOAT2(static_cast<float>(m_deviceInfo.displayMode.Width / 2), 20.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void Renderer::DisplayDeviceInfo()
{
	constexpr float offset = 20.0f;
	UINT posIndex = 1;

	// System Information
	DrawText(L"SYSTEM", XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	posIndex++;
	wstring dxVersion = L"DX Version: " + to_wstring(m_DXVersion) + L"." + to_wstring(m_DXSubVersion);
	DrawText(dxVersion.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

	posIndex++;
	wstring resolution = L"Resolution: " + to_wstring(m_deviceInfo.displayMode.Width) + L"x" + to_wstring(m_deviceInfo.displayMode.Height);
	DrawText(resolution.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
	posIndex++;

	// Hardware Information
	posIndex++;
	DrawText(L"HARDWARE", XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	for (const auto& hardwareInfo : m_deviceInfo.hardwareInfos)
	{
		posIndex++;
		wstring adapterIndex = L"GPU " + to_wstring(hardwareInfo.adapterIndex) + L": " + wstring(hardwareInfo.adapterDesc.Description);
		DrawText(adapterIndex.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

		posIndex++;
		wstring vendorId = L"Vendor ID: " + to_wstring(hardwareInfo.adapterDesc.VendorId);
		DrawText(vendorId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring deviceId = L"Device ID: " + to_wstring(hardwareInfo.adapterDesc.DeviceId);
		DrawText(deviceId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring subSysId = L"SubSystem ID: " + to_wstring(hardwareInfo.adapterDesc.SubSysId);
		DrawText(subSysId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring revision = L"Revision: " + to_wstring(hardwareInfo.adapterDesc.Revision);
		DrawText(revision.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring vram = L"VRAM: " + to_wstring(hardwareInfo.adapterDesc.DedicatedVideoMemory / (static_cast<unsigned long long>(1024) * 1024)) + L" MB";
		DrawText(vram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring sysram = L"System RAM: " + to_wstring(hardwareInfo.adapterDesc.DedicatedSystemMemory / (static_cast<unsigned long long>(1024) * 1024)) + L" MB";
		DrawText(sysram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring sharedram = L"Shared RAM: " + to_wstring(hardwareInfo.adapterDesc.SharedSystemMemory / (static_cast<unsigned long long>(1024) * 1024)) + L" MB";
		DrawText(sharedram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring adapterLuid = L"Adapter LUID: " + to_wstring(hardwareInfo.adapterDesc.AdapterLuid.LowPart) + L"," + to_wstring(hardwareInfo.adapterDesc.AdapterLuid.HighPart);
		DrawText(adapterLuid.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		for (const auto& outputDesc : hardwareInfo.outputDescs)
		{
			posIndex++;
			wstring outputInfo = L"Monitor: " + wstring(outputDesc.second.DeviceName);
			DrawText(outputInfo.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

			posIndex++;
			wstring resolution = L"Resolution: " + to_wstring(outputDesc.second.DesktopCoordinates.right - outputDesc.second.DesktopCoordinates.left) + L"x" + to_wstring(outputDesc.second.DesktopCoordinates.bottom - outputDesc.second.DesktopCoordinates.top);
			DrawText(resolution.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		}
		posIndex++;
	}
}

void Renderer::LoadAllShaders(const filesystem::path shaderPath, const char* entryPoint, const char* shaderModel)
{
	const filesystem::path vertexShaderPath = shaderPath / L"VertexShader/";
	const filesystem::path geometryShaderPath = shaderPath / L"GeometryShader/";
	const filesystem::path pixelShaderPath = shaderPath / L"PixelShader/";

	if (filesystem::exists(vertexShaderPath) && filesystem::is_directory(vertexShaderPath))
	{
		for (const auto& entry : filesystem::directory_iterator(vertexShaderPath))
		{
			if (entry.path().extension() == L".hlsl") LoadVertexShader(entry.path().c_str(), entryPoint, shaderModel);
		}
	}

	if (filesystem::exists(geometryShaderPath) && filesystem::is_directory(geometryShaderPath))
	{
		for (const auto& entry : filesystem::directory_iterator(geometryShaderPath))
		{
			if (entry.path().extension() == L".hlsl") LoadGeometryShader(entry.path().c_str(), entryPoint, shaderModel);
		}
	}

	if (filesystem::exists(pixelShaderPath) && filesystem::is_directory(pixelShaderPath))
	{
		for (const auto& entry : filesystem::directory_iterator(pixelShaderPath))
		{
			if (entry.path().extension() == L".hlsl") LoadPixelShader(entry.path().c_str(), entryPoint, shaderModel);
		}
	}
}

void Renderer::LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, const int layoutIndex)
{
	comPtr<ID3DBlob> VSCode;
	comPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, ("vs_" + string(shaderModel)).c_str(), compileFlags, 0, VSCode.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Vertex Shader Compilation Error", MB_OK);
		else MessageBoxW(nullptr, L"Failed to compile vertex shader", L"Error", MB_OK);
		return;
	}

	comPtr<ID3D11VertexShader> vertexShader;
	hr = m_device->CreateVertexShader(VSCode.Get()->GetBufferPointer(), VSCode.Get()->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed to create vertex shader", L"Error", MB_OK);
		return;
	}

	comPtr<ID3D11InputLayout> inputLayout;
	CreateInputLayout(s_layoutDescs[layoutIndex].first, s_layoutDescs[layoutIndex].second, VSCode, &inputLayout);

	wstring shaderName = filesystem::path(file).stem().wstring();
	if (g_vertexShaderIdMap.find(shaderName) == g_vertexShaderIdMap.end()) g_vertexShaderIdMap[shaderName] = s_vertexShaderId++;
	m_vertexShaderMap[g_vertexShaderIdMap[shaderName]] = make_pair(vertexShader, inputLayout);
}

void Renderer::LoadGeometryShader(const wchar_t* file, const char* entryPoint, const char* shaderModel)
{
	comPtr<ID3DBlob> GSCode;
	comPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, ("gs_" + string(shaderModel)).c_str(), compileFlags, 0, GSCode.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Geometry Shader Compilation Error", MB_OK);
		else MessageBoxW(nullptr, L"Failed to compile geometry shader", L"Error", MB_OK);
		return;
	}

	comPtr<ID3D11GeometryShader> geometryShader;
	hr = m_device->CreateGeometryShader(GSCode->GetBufferPointer(), GSCode->GetBufferSize(), nullptr, geometryShader.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed to create geometry shader", L"Error", MB_OK);
		return;
	}

	wstring shaderName = filesystem::path(file).stem().wstring();
	if (g_geometryShaderIdMap.find(shaderName) == g_geometryShaderIdMap.end()) g_geometryShaderIdMap[shaderName] = s_geometryShaderId++;
	m_geometryShaderMap[g_geometryShaderIdMap[shaderName]] = geometryShader;
}

void Renderer::LoadPixelShader(const wchar_t* file, const char* entryPoint, const char* shaderModel)
{
	comPtr<ID3DBlob> PSCode;
	comPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, ("ps_" + string(shaderModel)).c_str(), compileFlags, 0, PSCode.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorBlob) MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "Pixel Shader Compilation Error", MB_OK);
		else MessageBoxW(nullptr, L"Failed to compile pixel shader", L"Error", MB_OK);
		return;
	}

	comPtr<ID3D11PixelShader> pixelShader;
	hr = m_device->CreatePixelShader(PSCode->GetBufferPointer(), PSCode->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
		return;
	}

	wstring shaderName = filesystem::path(file).stem().wstring();

	if (g_pixelShaderIdMap.find(shaderName) == g_pixelShaderIdMap.end()) g_pixelShaderIdMap[shaderName] = s_pixelShaderId++;
	m_pixelShaderMap[g_pixelShaderIdMap[shaderName]] = pixelShader;
}

void Renderer::LoadAllTextures(const std::filesystem::path texturePath)
{
	if (filesystem::exists(texturePath) && filesystem::is_directory(texturePath))
	{
		for (const auto& entry : filesystem::directory_iterator(texturePath))
		{
			wstring textureName = entry.path().stem().wstring();
			if (g_textureIdMap.find(textureName) == g_textureIdMap.end()) g_textureIdMap[textureName] = s_textureId++;

			comPtr<ID3D11ShaderResourceView> texture;
			HRESULT hr = DirectX::CreateWICTextureFromFile(m_device.Get(), entry.path().c_str(), nullptr, texture.GetAddressOf());
			if (FAILED(hr))
			{
				hr = DirectX::CreateDDSTextureFromFile(m_device.Get(), entry.path().c_str(), nullptr, texture.GetAddressOf());
				if (FAILED(hr))
				{
					MessageBoxW(nullptr, (L"Failed to load texture: " + entry.path().wstring()).c_str(), L"Error", MB_OK);
					continue;
				}
			}

			m_textureMap[g_textureIdMap[textureName]] = texture;
		}
	}
}

void Renderer::CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.ScissorEnable = TRUE;
	rasterDesc.MultisampleEnable = TRUE; // Base value is FALSE
	rasterDesc.AntialiasedLineEnable = TRUE; // Base value is FALSE
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, g_rasterState[0].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
		return;
	}

	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, g_rasterState[1].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
		return;
	}

#ifdef _DEBUG
	m_deviceContext->RSSetState(g_rasterState[1].Get());
	m_currentRasterState = RasterState::Wireframe;
#else
	m_deviceContext->RSSetState(g_rasterState[0].Get());
	m_currentRasterState = RasterState::Solid;
#endif
}

void Renderer::CreateSamplerState()
{
	if (FAILED(m_device->CreateSamplerState(&s_defaultSamplerDesc, m_samplers[0].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create sampler state", L"Error", MB_OK);
		return;
	}
}

void Renderer::LoadShapeFile(const filesystem::path filePath)
{
	ObjFileParser shapes(filePath.c_str());

	wstring parentName = filePath.stem().wstring();
	vector<Vertex> combinedVertices;

	for (const auto& [name, vertices] : shapes.m_shapes)
	{
		wstring childName = parentName + L"_" + name; // Save child shapes as parentName_childName
		if (g_meshIdMap.find(parentName) == g_meshIdMap.end()) g_meshIdMap[parentName] = s_nextShapeId++;

		CreateVertexBuffer(static_cast<UINT>(sizeof(Vertex) * vertices.size()), &m_shapeVertexBufferMap[g_meshIdMap[parentName]].first, vertices.data(), sizeof(Vertex));
		m_shapeVertexBufferMap[g_meshIdMap[parentName]].second = static_cast<UINT>(vertices.size());

		combinedVertices.insert(combinedVertices.end(), vertices.begin(), vertices.end());
	}

	if (g_meshIdMap.find(parentName) == g_meshIdMap.end()) g_meshIdMap[parentName] = s_nextShapeId++;
	CreateVertexBuffer(static_cast<UINT>(sizeof(Vertex) * combinedVertices.size()), &m_shapeVertexBufferMap[g_meshIdMap[parentName]].first, combinedVertices.data(), sizeof(Vertex));
	m_shapeVertexBufferMap[g_meshIdMap[parentName]].second = static_cast<UINT>(combinedVertices.size());
}

void Renderer::LoadDefaultShapes(const filesystem::path folderPath)
{
	if (filesystem::exists(folderPath) && filesystem::is_directory(folderPath))
	{
		for (const auto& entry : filesystem::directory_iterator(folderPath))
		{
			if (entry.path().extension() == L".obj") LoadShapeFile(entry.path().c_str());
		}
	}
}

void Renderer::UpdateRenderer()
{
	ClearBackBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, VDGM::g_currentScene->m_backgroundColor, 1.0f, 0);

	AmbientFogConstBuffer fogData = {};
	if (!g_camera) DrawText(L"Camera not found", XMFLOAT2(m_deviceInfo.displayMode.Width / 2.0f, m_deviceInfo.displayMode.Height / 2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	else
	{
		g_camera->SetScreen(-1.0f, m_deviceInfo.displayMode.Width, m_deviceInfo.displayMode.Height); // Feels wasteful to do this every frame // TODO: Fix this

		s_viewMatrix = g_camera->GetViewMatrix();
		fogData.cameraPosition = g_camera->m_cameraPosition;
		s_projectionMatrix = g_camera->GetProjectionMatrix();
	}

	// Ambient Light
	m_deviceContext->UpdateSubresource(m_constBuffers[AmbientLightBuffer].Get(), 0, nullptr, &VDGM::g_currentScene->m_ambientLight, 0, 0);
	m_deviceContext->VSSetConstantBuffers(1, 1, m_constBuffers[AmbientLightBuffer].GetAddressOf());

	// Directional Light
	m_deviceContext->UpdateSubresource(m_constBuffers[DirectionalLightBuffer].Get(), 0, nullptr, &DirectionalLight::s_lightData, 0, 0);
	m_deviceContext->VSSetConstantBuffers(2, 1, m_constBuffers[DirectionalLightBuffer].GetAddressOf());

	// Ambient Fog
	fogData.colorAndRange = VDGM::g_currentScene->m_ambientFog;
	m_deviceContext->UpdateSubresource(m_constBuffers[AmbientFogBuffer].Get(), 0, nullptr, &fogData, 0, 0);
	m_deviceContext->PSSetConstantBuffers(0, 1, m_constBuffers[AmbientFogBuffer].GetAddressOf());

	// Point Lights // later add for loop
	PointLightArrayConstBuffer pointLightBufferData = {};
	pointLightBufferData.pointLights[0] = g_pointLights[0]->GetLightData();
	pointLightBufferData.pointLights[1] = g_pointLights[1]->GetLightData();
	m_deviceContext->UpdateSubresource(m_constBuffers[PointLightBuffer].Get(), 0, nullptr, &pointLightBufferData, 0, 0);
	m_deviceContext->PSSetConstantBuffers(1, 1, m_constBuffers[PointLightBuffer].GetAddressOf());

	m_deviceContext->PSSetSamplers(0, 1, m_samplers[0].GetAddressOf());

	UpdateRenderMode();
}

constexpr UINT stride = sizeof(Vertex);
constexpr UINT offset = 0;

// TODO: CreateDepthStencilState
void Renderer::DrawObjects()
{
	DrawShapes();

	if (m_drawNormalLines) DrawNormalLines();
}

void Renderer::DrawShapes() // Only triangle topology
{
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (const auto& [object, shapeData] : g_renderShapes)
	{
		m_deviceContext->IASetVertexBuffers(0, 1, m_shapeVertexBufferMap[shapeData->meshId].first.GetAddressOf(), &stride, &offset);

		m_deviceContext->VSSetShader((m_vertexShaderMap[shapeData->vertexShaderId]).first.Get(), nullptr, 0);
		m_deviceContext->PSSetShader(m_pixelShaderMap[shapeData->pixelShaderId].Get(), nullptr, 0);

		XMMATRIX worldMatrix = object->GetWorldMatrix();

		MatrixConstBuffer constBufferData = {};
		constBufferData.world = XMMatrixTranspose(worldMatrix);
		constBufferData.view = XMMatrixTranspose(s_viewMatrix);
		constBufferData.projection = XMMatrixTranspose(s_projectionMatrix);
		constBufferData.WVP = XMMatrixTranspose(worldMatrix * s_viewMatrix * s_projectionMatrix);
		constBufferData.normalMatrix = XMMatrixTranspose(object->m_inverseScaleMatrix * worldMatrix);

		m_deviceContext->UpdateSubresource(m_constBuffers[MatrixBuffer].Get(), 0, nullptr, &constBufferData, 0, 0);
		m_deviceContext->VSSetConstantBuffers(0, 1, m_constBuffers[MatrixBuffer].GetAddressOf());

		m_deviceContext->IASetInputLayout(m_vertexShaderMap[shapeData->vertexShaderId].second.Get());

		for (size_t i = 0; i < shapeData->textureIds.size(); ++i)
		{
			m_deviceContext->PSSetShaderResources(static_cast<UINT>(i), 1, m_textureMap[shapeData->textureIds[i]].GetAddressOf()); // This can be optimized further
		}

		m_deviceContext->Draw(m_shapeVertexBufferMap[shapeData->meshId].second, 0);

		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		for (size_t i = 0; i < shapeData->textureIds.size(); ++i)
		{
			m_deviceContext->PSSetShaderResources(static_cast<UINT>(i), 1, nullSRV);
		}
	}
}

void Renderer::DrawNormalLines()
{
	for (const auto& [object, shapeData] : g_renderShapes)
	{
		m_deviceContext->IASetVertexBuffers(0, 1, m_shapeVertexBufferMap[shapeData->meshId].first.GetAddressOf(), &stride, &offset);
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		m_deviceContext->VSSetShader(m_vertexShaderMap[g_vertexShaderIdMap[L"VSShowNormal"]].first.Get(), nullptr, 0);
		m_deviceContext->GSSetShader(m_geometryShaderMap[g_geometryShaderIdMap[L"GSShowNormal"]].Get(), nullptr, 0);
		m_deviceContext->PSSetShader(m_pixelShaderMap[g_pixelShaderIdMap[L"PSShowNormal"]].Get(), nullptr, 0);

		XMMATRIX worldMatrix = object->GetWorldMatrix();

		MatrixConstBuffer constBufferData = {};
		constBufferData.world = XMMatrixTranspose(worldMatrix);
		constBufferData.view = XMMatrixTranspose(s_viewMatrix);
		constBufferData.projection = XMMatrixTranspose(s_projectionMatrix);
		constBufferData.WVP = XMMatrixTranspose(worldMatrix * s_viewMatrix * s_projectionMatrix);
		constBufferData.normalMatrix = XMMatrixTranspose(object->m_inverseScaleMatrix * worldMatrix);

		m_deviceContext->UpdateSubresource(m_constBuffers[MatrixBuffer].Get(), 0, nullptr, &constBufferData, 0, 0);
		m_deviceContext->VSSetConstantBuffers(0, 1, m_constBuffers[MatrixBuffer].GetAddressOf());
		m_deviceContext->GSSetConstantBuffers(0, 1, m_constBuffers[MatrixBuffer].GetAddressOf());

		m_deviceContext->IASetInputLayout(m_vertexShaderMap[g_vertexShaderIdMap[L"VSShowNormal"]].second.Get());

		m_deviceContext->Draw(m_shapeVertexBufferMap[shapeData->meshId].second, 0);

		m_deviceContext->GSSetShader(nullptr, nullptr, 0);
	}
}

void Renderer::UpdateRenderMode()
{
	m_deviceContext->RSSetState(g_rasterState[static_cast<int>(m_currentRasterState)].Get());
}

Renderer::Renderer(HWND hWnd, LONG width, LONG height, const wchar_t* resourcePath) : m_hWnd(hWnd)
{
	// Initialize device
	GetHardwareInfo();
	m_deviceInfo.displayMode.Width = m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.right - m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.left;
	m_deviceInfo.displayMode.Height = m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.bottom - m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.top;

	//m_deviceInfo.displayMode.Width = width;
	//m_deviceInfo.displayMode.Height = height;

	CreateDeviceSwapChain();
	CreateRenderTarget();
	CreateDepthStencil();
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	SetViewport();
	SetScissorRect(width, height);

	LoadFonts();

	m_DXVersion = (m_deviceInfo.featureLevels & 0xf000) >> 12;
	m_DXSubVersion = (m_deviceInfo.featureLevels & 0x0f00) >> 8;

	// Initialize render
	CreateRasterState();
	CreateSamplerState();

	// Initialize constant buffers
	CreateConstBuffer(sizeof(MatrixConstBuffer), &m_constBuffers[MatrixBuffer]);
	CreateConstBuffer(sizeof(XMFLOAT4), &m_constBuffers[AmbientLightBuffer]); // Ambient light buffer
	CreateConstBuffer(sizeof(AmbientFogConstBuffer), &m_constBuffers[AmbientFogBuffer]); // Ambient fog buffer
	CreateConstBuffer(sizeof(DirectionalLightConstBuffer), &m_constBuffers[DirectionalLightBuffer]); // Directional light buffer
	CreateConstBuffer(sizeof(PointLightArrayConstBuffer), &m_constBuffers[PointLightBuffer]); // Point light buffer

	static const filesystem::path defaultPath(L"../Assets/Default/");
	LoadAllShaders(defaultPath / L"Shader/", "main", "5_0");
	LoadDefaultShapes(defaultPath / L"Shapes/");
	LoadAllTextures(defaultPath / L"Texture/");

	if (resourcePath) // This will override the default assets if corrisponding files are found
	{
		filesystem::path resPath(resourcePath);
		LoadAllShaders(resPath / L"Shader/", "main", "5_0");
		LoadDefaultShapes(resPath / L"Shapes/");
		LoadAllTextures(resPath / L"Texture/");
	}
}

Renderer::~Renderer()
{
	if (m_deviceContext) // Microsoft does recommend this!
	{
		m_deviceContext->ClearState();
		m_deviceContext->Flush();
	}
}

void Renderer::Resize(UINT width, UINT height)
{
	if (width == 0 || height == 0) return;

	m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_depthStencilBuffer.Reset();

	if (FAILED(m_swapChain->ResizeBuffers(1, width, height, m_deviceInfo.displayMode.Format, 0)))
	{
		MessageBoxW(nullptr, L"Failed to resize swap chain buffers", L"Error", MB_OK);
		return;
	}

	CreateRenderTarget();
	CreateDepthStencil();

	SetScissorRect(width, height);
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

constexpr UINT VEWPORT_NUM = 1;

void Renderer::SetViewport(float topLeftX, float topLeftY)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = -topLeftX;
	viewport.TopLeftY = -topLeftY;
	viewport.Width = static_cast<FLOAT>(m_deviceInfo.displayMode.Width);
	viewport.Height = static_cast<FLOAT>(m_deviceInfo.displayMode.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_deviceContext->RSSetViewports(VEWPORT_NUM, &viewport);
}

void Renderer::DrawText(const wchar_t* text, XMFLOAT2 position, XMFLOAT4 color, float scale, const wchar_t* fontName)
{
	wchar_t buffer[256] = {};
	wcsncpy_s(buffer, text, _TRUNCATE);

	FXMVECTOR colorVector = XMLoadFloat4(&color);

	if (m_SpriteFontMap.find(fontName) != m_SpriteFontMap.end())
	{
		comPtr<ID3D11DepthStencilState> currentDepthState;
		m_deviceContext->OMGetDepthStencilState(&currentDepthState, nullptr);

		// This fuckes up depth testing // considering a different way to draw text
		m_SpriteBatchMap[fontName]->Begin();
		m_SpriteFontMap[fontName]->DrawString(m_SpriteBatchMap[fontName].get(), buffer, position, colorVector, 0.0f, XMFLOAT2(0.0f, 0.0f), scale);
		m_SpriteBatchMap[fontName]->End();

		m_deviceContext->OMSetDepthStencilState(currentDepthState.Get(), 0);
	}
}

void Renderer::Render()
{
	UpdateRenderer();

	DrawObjects();

	ShowFPS();

#ifdef _DEBUG
	DisplayDeviceInfo();
#endif

	m_swapChain->Present(m_deviceInfo.isVSync, 0);
}

void Renderer::ChangeState()
{
	m_currentRasterState = static_cast<RasterState>((static_cast<int>(m_currentRasterState) + 1) % RasterStateCount);
}

void Renderer::ScreenPointToWorld(POINT screenPos) const
{
	D3D11_VIEWPORT vp;
	UINT numViewports = VEWPORT_NUM;
	m_deviceContext->RSGetViewports(&numViewports, &vp);

	XMVECTOR rayOrigin = XMVectorSet(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y), 0.0f, 1.0f);
	XMVECTOR rayEnd = XMVectorSet(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y), 1.0f, 1.0f);

	rayOrigin = XMVector3Unproject
	(
		rayOrigin,
		vp.TopLeftX, vp.TopLeftY,
		vp.Width, vp.Height,
		vp.MinDepth, vp.MaxDepth,
		s_projectionMatrix, s_viewMatrix, XMMatrixIdentity()
	);
	rayEnd = XMVector3Unproject
	(
		rayEnd,
		vp.TopLeftX, vp.TopLeftY,
		vp.Width, vp.Height,
		vp.MinDepth, vp.MaxDepth,
		s_projectionMatrix, s_viewMatrix, XMMatrixIdentity()
	);

	if (VDGM::g_currentScene) VDGM::g_currentScene->Raycast(rayOrigin, rayEnd);
}

#undef comPtr