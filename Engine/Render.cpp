#include "Render.h"

#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

#define comPtr Microsoft::WRL::ComPtr

UINT Render::s_nextShapeId = 1;
unordered_map<wstring, UINT> g_shapeIdMap;

D3D11_INPUT_ELEMENT_DESC Render::s_defaultInputLayoutDesc[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
D3D11_INPUT_ELEMENT_DESC Render::s_tripleInputLayoutDesc[12] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	{ "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	{ "POSITION", 2, DXGI_FORMAT_R32G32B32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
pair<D3D11_INPUT_ELEMENT_DESC*, UINT> Render::s_layoutDescs[2] = { {Render::s_defaultInputLayoutDesc, 4}, {Render::s_tripleInputLayoutDesc, 12} };

Camera* g_camera = nullptr;
XMMATRIX Render::s_viewMatrix = XMMatrixIdentity();
XMMATRIX Render::s_projectionMatrix = XMMatrixIdentity();

void Render::CreateDeviceSwapChain()
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
	swapChainDesc.SampleDesc.Quality = 0;

	if (
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

void Render::CreateRenderTarget()
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

void Render::CreateDepthStencil()
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
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = m_deviceInfo.antiAliasingLevel;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	if (FAILED(m_device->CreateTexture2D(&depthStencilDesc, nullptr, m_depthStencilBuffer.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create depth stencil texture", L"Error", MB_OK);
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = (m_deviceInfo.antiAliasingLevel > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create depth stencil view", L"Error", MB_OK);
		m_depthStencilBuffer.Reset();
		return;
	}
}

void Render::LoadFonts()
{
	wstring fontPath = L"../Assets/Fonts/";
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

void Render::GetHardwareInfo()
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

void Render::ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth, UINT8 stencil)
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), reinterpret_cast<const float*>(&color));

	if (m_depthStencilView.Get() != nullptr) m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), flag, depth, stencil);
	else
	{
		MessageBoxW(nullptr, L"Failed to clear depth stencil view", L"Error", MB_OK);
		return;
	}
}

void Render::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements, comPtr<ID3DBlob> shaderCode, _Out_ comPtr<ID3D11InputLayout>* inputLayout)
{
	if (FAILED(m_device->CreateInputLayout(layoutDesc, numElements, shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), inputLayout->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create input layout", L"Error", MB_OK);
		return;
	}
}

void Render::CreateVertexBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer, const void* initData, UINT stride)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = initData;

	if (FAILED(m_device->CreateBuffer(&bufferDesc, &subresourceData, buffer->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create buffer", L"Error", MB_OK);
		return;
	}
}

void Render::CreateConstBuffer(UINT size, _Out_ comPtr<ID3D11Buffer>* buffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.StructureByteStride = 0;
	if (FAILED(m_device->CreateBuffer(&bufferDesc, nullptr, buffer->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create constant buffer", L"Error", MB_OK);
		return;
	}
}

void Render::ShowFPS()
{
	static UINT frameCount = 0;
	static double elapsedTime = 0.0;
	static double fps = 0.0;

	frameCount++;

	elapsedTime += VDGM::g_deltaTimeD;

	if (elapsedTime >= 1.0)
	{
		fps = frameCount * elapsedTime;
		frameCount = 0;
		elapsedTime = 0.0;
	}

	wstring fpsText = L"FPS: " + to_wstring(static_cast<int>(fps));
	DrawText(fpsText.c_str(), XMFLOAT2(static_cast<float>(m_deviceInfo.displayMode.Width / 2), 20.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void Render::DisplayDeviceInfo()
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

void Render::UpdateShaders()
{
	m_deviceContext->VSSetShader(get<0>(m_vertexShaderMap[m_currentVertexShader]).Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShaderMap[m_currentPixelShader].Get(), nullptr, 0);

	if (!get<2>(m_vertexShaderMap[m_currentVertexShader])) CreateConstBuffer(sizeof(TestConstBuffer), &get<2>(m_vertexShaderMap[m_currentVertexShader]));
	ID3D11Buffer* constantBuffer = get<2>(m_vertexShaderMap[m_currentVertexShader]).Get();
	m_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
}

void Render::LoadAllShaders(const wchar_t* shaderPath, const char* entryPoint, const char* shaderModel) // TODO: Refactor this later for better error handling and flexibility
{
	filesystem::path shaderDir(shaderPath);
	filesystem::path vertexShaderPath = shaderDir / L"VertexShader/";
	filesystem::path pixelShaderPath = shaderDir / L"PixelShader/";

	filesystem::path defaultInputLayoutPath = vertexShaderPath / L"DefaultInputLayout/";
	if (filesystem::exists(defaultInputLayoutPath) && filesystem::is_directory(defaultInputLayoutPath))
	{
		for (const auto& entry : filesystem::directory_iterator(defaultInputLayoutPath))
		{
			//if (entry.path().extension() == L".cso") LoadPrecompiledVertexShader(entry.path().c_str()); loading precompiled shader is disabled for now
			if (entry.path().extension() == L".hlsl") LoadVertexShader(entry.path().c_str(), entryPoint, shaderModel);
		}
	}
	filesystem::path tripleInputLayoutPath = vertexShaderPath / L"TripleInputLayout/";
	if (filesystem::exists(tripleInputLayoutPath) && filesystem::is_directory(tripleInputLayoutPath))
	{
		for (const auto& entry : filesystem::directory_iterator(tripleInputLayoutPath))
		{
			if (entry.path().extension() == L".hlsl") LoadVertexShader(entry.path().c_str(), entryPoint, shaderModel, 1);
		}
	}


	if (filesystem::exists(pixelShaderPath) && filesystem::is_directory(pixelShaderPath))
	{
		for (const auto& entry : filesystem::directory_iterator(pixelShaderPath))
		{
			//if (entry.path().extension() == L".cso") LoadPrecompiledPixelShader(entry.path().c_str());
			if (entry.path().extension() == L".hlsl") LoadPixelShader(entry.path().c_str(), entryPoint, shaderModel);
		}
	}
}

void Render::LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, int layoutIndex)
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
	// Use predefined constant buffer
	if (shaderName == L"VertexShader") m_vertexShaderMap[VertexShaders::Default] = make_tuple(vertexShader, VSCode, nullptr, inputLayout);
}

void Render::LoadPixelShader(const wchar_t* file, const char* entryPoint, const char* shaderModel)
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
	if (shaderName == L"PixelShader") m_pixelShaderMap[PixelShaders::Default] = pixelShader;
	else if (shaderName == L"PixelShaderNull") m_pixelShaderMap[PixelShaders::Greyscale] = pixelShader;
	else if (shaderName == L"PixelShaderAll") m_pixelShaderMap[PixelShaders::ColorShift] = pixelShader;
}

void Render::LoadPrecompiledVertexShader(const wchar_t* file)
{
	comPtr<ID3DBlob> VSCode;

	HRESULT hr = D3DReadFileToBlob(file, VSCode.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed to read precompiled vertex shader", L"Error", MB_OK);
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
	CreateInputLayout(s_defaultInputLayoutDesc, _countof(s_defaultInputLayoutDesc), VSCode, &inputLayout);

	wstring shaderName = filesystem::path(file).stem().wstring();
	if (shaderName == L"VertexShader") m_vertexShaderMap[VertexShaders::Default] = make_tuple(vertexShader, VSCode, nullptr, inputLayout);
}

void Render::LoadPrecompiledPixelShader(const wchar_t* file)
{
	comPtr<ID3DBlob> PSCode;
	HRESULT hr = D3DReadFileToBlob(file, PSCode.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed to read precompiled pixel shader", L"Error", MB_OK);
		return;
	}

	comPtr<ID3D11PixelShader> pixelShader;
	hr = m_device->CreatePixelShader(PSCode.Get()->GetBufferPointer(), PSCode.Get()->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
		return;
	}

	wstring shaderName = filesystem::path(file).stem().wstring();
	if (shaderName == L"PixelShader") m_pixelShaderMap[PixelShaders::Default] = pixelShader;
	else if (shaderName == L"PixelShaderNull") m_pixelShaderMap[PixelShaders::Greyscale] = pixelShader;
	else if (shaderName == L"PixelShaderAll") m_pixelShaderMap[PixelShaders::ColorShift] = pixelShader;
}

void Render::CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.MultisampleEnable = TRUE; // Base value is FALSE
	rasterDesc.AntialiasedLineEnable = TRUE; // Base value is FALSE
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, g_rasterState[0].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
		return;
	}

	rasterDesc.CullMode = D3D11_CULL_BACK;
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, g_rasterState[1].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
		return;
	}

	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, g_rasterState[2].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
		return;
	}

	rasterDesc.CullMode = D3D11_CULL_BACK;
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, g_rasterState[3].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
		return;
	}

#ifdef _DEBUG
	m_deviceContext->RSSetState(g_rasterState[0].Get());
	m_currentRasterState = RasterState::Wireframe_CullNone;
#else
	m_deviceContext->RSSetState(g_rasterState[2].Get());
	m_currentRasterState = RasterState::Solid_CullNone;
#endif
}

void Render::LoadDefaultShapes()
{
	static const wchar_t* defaultObjPath = L"../Assets/Shapes/Default.obj";

	ObjFileParser shapes(defaultObjPath);
	for (const auto& [name, vertices] : shapes.m_shapes)
	{
		if (g_shapeIdMap.find(name) == g_shapeIdMap.end()) g_shapeIdMap[name] = s_nextShapeId++;

		CreateVertexBuffer(static_cast<UINT>(sizeof(Vertex) * vertices.size()), &m_shapeVertexBufferMap[g_shapeIdMap[name]].first, vertices.data(), sizeof(Vertex));
		m_shapeVertexBufferMap[g_shapeIdMap[name]].second = static_cast<UINT>(vertices.size());
	}
	
	// Other sample shapes
	if (g_shapeIdMap.find(L"GreenPlane") == g_shapeIdMap.end()) g_shapeIdMap[L"GreenPlane"] = s_nextShapeId++;
	Vertex plainVertices[] =
	{
		{ XMFLOAT3(-50.0f, 0.0f, 50.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 0.0f, 50.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 0.0f, -50.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },

		{ XMFLOAT3(-50.0f, 0.0f, 50.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(50.0f, 0.0f, -50.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(-50.0f, 0.0f, -50.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) }
	};
	CreateVertexBuffer(sizeof(plainVertices), &m_shapeVertexBufferMap[g_shapeIdMap[L"GreenPlane"]].first, plainVertices, sizeof(Vertex));
	m_shapeVertexBufferMap[g_shapeIdMap[L"GreenPlane"]].second = 6;

	if (g_shapeIdMap.find(L"TriHouse") == g_shapeIdMap.end()) g_shapeIdMap[L"TriHouse"] = s_nextShapeId++;
	Vertex triHouseVertices[] =
	{
		// Triangles
		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

		// Square
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
	};
	CreateVertexBuffer(sizeof(triHouseVertices), &m_shapeVertexBufferMap[g_shapeIdMap[L"TriHouse"]].first, triHouseVertices, sizeof(Vertex));
	m_shapeVertexBufferMap[g_shapeIdMap[L"TriHouse"]].second = 18;

	if (g_shapeIdMap.find(L"Tree") == g_shapeIdMap.end()) g_shapeIdMap[L"Tree"] = s_nextShapeId++;
	Vertex treeVertices[] =
	{
		{ XMFLOAT3(-0.5f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f) },

		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, 0.5f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },

		{ XMFLOAT3(0.0f, 0.0f, -0.5f), XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.5f), XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f) },

		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.5f, -0.5f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) }
	};
	CreateVertexBuffer(sizeof(treeVertices), &m_shapeVertexBufferMap[g_shapeIdMap[L"Tree"]].first, treeVertices, sizeof(Vertex));
	m_shapeVertexBufferMap[g_shapeIdMap[L"Tree"]].second = 12;

	if (g_shapeIdMap.find(L"WindmillWing") == g_shapeIdMap.end()) g_shapeIdMap[L"WindmillWing"] = s_nextShapeId++;
	Vertex wingVertices[] =
	{
		{ XMFLOAT3(-1.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -0.5f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -0.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f) }
	};
	CreateVertexBuffer(sizeof(wingVertices), &m_shapeVertexBufferMap[g_shapeIdMap[L"WindmillWing"]].first, wingVertices, sizeof(Vertex));
	m_shapeVertexBufferMap[g_shapeIdMap[L"WindmillWing"]].second = 12;
}

void Render::LoadShapeFolder(const wchar_t* folderPath)
{
	ObjFileParser shapes(folderPath);

	wstring parentName = filesystem::path(folderPath).stem().wstring();
	vector<Vertex> combinedVertices;

	for (const auto& [name, vertices] : shapes.m_shapes)
	{
		wstring childName = parentName + L"_" + name;
		if (g_shapeIdMap.find(parentName) == g_shapeIdMap.end()) g_shapeIdMap[parentName] = s_nextShapeId++;

		CreateVertexBuffer(static_cast<UINT>(sizeof(Vertex) * vertices.size()), &m_shapeVertexBufferMap[g_shapeIdMap[parentName]].first, vertices.data(), sizeof(Vertex));
		m_shapeVertexBufferMap[g_shapeIdMap[parentName]].second = static_cast<UINT>(vertices.size());

		combinedVertices.insert(combinedVertices.end(), vertices.begin(), vertices.end());
	}

	if (g_shapeIdMap.find(parentName) == g_shapeIdMap.end()) g_shapeIdMap[parentName] = s_nextShapeId++;
	CreateVertexBuffer(static_cast<UINT>(sizeof(Vertex) * combinedVertices.size()), &m_shapeVertexBufferMap[g_shapeIdMap[parentName]].first, combinedVertices.data(), sizeof(Vertex));
	m_shapeVertexBufferMap[g_shapeIdMap[parentName]].second = static_cast<UINT>(combinedVertices.size());
}

void Render::EngineUpdate()
{
	UpdateRenderMode();
	UpdateShaders();
}

void Render::DrawObjects()
{
	if (!g_camera)
	{
		DrawText(L"Camera not found", XMFLOAT2(m_deviceInfo.displayMode.Width / 2.0f, m_deviceInfo.displayMode.Height / 2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		return;
	}
	g_camera->SetScreen(-1.0f, m_deviceInfo.displayMode.Width, m_deviceInfo.displayMode.Height);

	static float ATime = 0.0f; // Accumulated time
	ATime += VDGM::g_deltaTimeF;

	for (Object* object : g_objects)
	{
		if (!object || !object->m_isActive) continue;
		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0;

#ifdef _DEBUG
		if (m_shapeVertexBufferMap.find(object->m_shapeId) == m_shapeVertexBufferMap.end()) { MessageBoxW(nullptr, L"Shape not found in vertex buffer map", L"Error", MB_OK); continue; }
#endif
		ID3D11Buffer* vertexBuffer = m_shapeVertexBufferMap[object->m_shapeId].first.Get();

		if (m_currentVertexShader == VertexShaders::Default) m_deviceContext->VSSetShader(get<0>(m_vertexShaderMap[object->m_vertexShader]).Get(), nullptr, 0);
		else m_deviceContext->VSSetShader(get<0>(m_vertexShaderMap[m_currentVertexShader]).Get(), nullptr, 0);
		if (m_currentPixelShader == PixelShaders::Default) m_deviceContext->PSSetShader(m_pixelShaderMap[object->m_pixelShader].Get(), nullptr, 0);
		else m_deviceContext->PSSetShader(m_pixelShaderMap[m_currentPixelShader].Get(), nullptr, 0);

		m_deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		m_deviceContext->IASetInputLayout(get<3>(m_vertexShaderMap[object->m_vertexShader]).Get());
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMMATRIX worldMatrix = object->GetWorldMatrix();
		s_viewMatrix = g_camera->GetViewMatrix();
		s_projectionMatrix = g_camera->GetProjectionMatrix();

		TestConstBuffer constBufferData = {};
		constBufferData.world = XMMatrixTranspose(worldMatrix);
		constBufferData.view = XMMatrixTranspose(s_viewMatrix);
		constBufferData.projection = XMMatrixTranspose(s_projectionMatrix);
		constBufferData.WVP = XMMatrixTranspose(worldMatrix * s_viewMatrix * s_projectionMatrix);

		constBufferData.VSFloatA = sinf(static_cast<float>(ATime));
		constBufferData.VSFloatB = cosf(static_cast<float>(ATime));
		constBufferData.VSFloatC = -constBufferData.VSFloatA;
		constBufferData.VSFloatD = -constBufferData.VSFloatB;

		m_deviceContext->UpdateSubresource(get<2>(m_vertexShaderMap[object->m_vertexShader]).Get(), 0, nullptr, &constBufferData, 0, 0);

		m_deviceContext->Draw(m_shapeVertexBufferMap[object->m_shapeId].second, 0);
	}
}

void Render::UpdateRenderMode()
{
	m_deviceContext->RSSetState(g_rasterState[static_cast<int>(m_currentRasterState)].Get());
}

Render::Render(HWND hWnd, UINT width, UINT height) : m_hWnd(hWnd)
{
	// Initialize device
	GetHardwareInfo();
	//m_deviceInfo.displayMode.Width = m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.right - m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.left;
	//m_deviceInfo.displayMode.Height = m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.bottom - m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.top;

	m_deviceInfo.displayMode.Width = width;
	m_deviceInfo.displayMode.Height = height;

	CreateDeviceSwapChain();
	CreateRenderTarget();
	CreateDepthStencil();
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	SetViewport();
	LoadFonts();

	m_DXVersion = (m_deviceInfo.featureLevels & 0xf000) >> 12;
	m_DXSubVersion = (m_deviceInfo.featureLevels & 0x0f00) >> 8;

	// Initialize render
	CreateRasterState();
	LoadAllShaders(L"../Assets/Shader/", "main", "5_0");
	UpdateShaders();

	LoadDefaultShapes();
	LoadShapeFolder(L"../Assets/Shapes/PlayerAnimationIdle.obj");
}

Render::~Render()
{
	// Clear device
	m_SpriteFontMap.clear();
	m_SpriteBatchMap.clear();
	m_device.Reset();

	// Not sure if this is necessary // microsoft doesn't recommend it
	if (m_deviceContext)
	{
		m_deviceContext->ClearState();
		m_deviceContext->Flush();
	}
	m_deviceContext.Reset();
	m_swapChain.Reset();
	m_renderTargetView.Reset();
	m_depthStencilBuffer.Reset();
	m_depthStencilView.Reset();
	m_deviceInfo.hardwareInfos.clear();

	m_shapeVertexBufferMap.clear();
	m_vertexShaderMap.clear();
	m_pixelShaderMap.clear();

	// Clear render
	for (auto& state : g_rasterState) state.Reset();
}

void Render::Resize(UINT width, UINT height)
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

	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

constexpr UINT VEWPORT_NUM = 1;

void Render::SetViewport(float topLeftX, float topLeftY)
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

void Render::DrawText(const wchar_t* text, XMFLOAT2 position, XMFLOAT4 color, float scale, const wchar_t* fontName)
{
	wchar_t buffer[256] = {};
	wcsncpy_s(buffer, text, _TRUNCATE);

	FXMVECTOR colorVector = XMLoadFloat4(&color);

	if (m_SpriteFontMap.find(fontName) != m_SpriteFontMap.end())
	{
		comPtr<ID3D11DepthStencilState> currentDepthState;
		m_deviceContext->OMGetDepthStencilState(&currentDepthState, nullptr);

		// This fuckes up the depth testing // considering a different way to draw text
		m_SpriteBatchMap[fontName]->Begin();
		m_SpriteFontMap[fontName]->DrawString(m_SpriteBatchMap[fontName].get(), buffer, position, colorVector, 0.0f, XMFLOAT2(0.0f, 0.0f), scale);
		m_SpriteBatchMap[fontName]->End();

		m_deviceContext->OMSetDepthStencilState(currentDepthState.Get(), 0);
	}
}

constexpr XMFLOAT4 CLEAR_COLOR = { 0.5f, 0.5f, 0.5f, 1.0f };

void Render::SceneRender()
{
	EngineUpdate();

	ClearBackBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, CLEAR_COLOR, 1.0f, 0);

	DrawObjects();

	ShowFPS();

#ifdef _DEBUG
	DisplayDeviceInfo();
#endif

	Present();
}

void Render::ChangeShader(PixelShaders pixelShader)
{
	m_deviceContext->PSSetShader(m_pixelShaderMap[pixelShader].Get(), nullptr, 0);
	m_currentPixelShader = pixelShader;
}

void Render::ChangeState()
{
	m_currentRasterState = static_cast<RasterState>((static_cast<int>(m_currentRasterState) + 1) % 4);
}

void Render::ScreenPointToWorld(POINT screenPos) const
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