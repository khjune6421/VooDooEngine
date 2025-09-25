#include "Render.h"

#include "Object.h"

using namespace std;
using namespace DirectX;

#define comPtr Microsoft::WRL::ComPtr

Camera* g_camera = nullptr;

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

void Render::ChangeShader(PixelShaders pixelShader)
{
	m_deviceContext->PSSetShader(m_pixelShaderMap[pixelShader].Get(), nullptr, 0);
	m_currentPixelShader = pixelShader;
}

void Render::ChangeState()
{
	m_currentRasterState = static_cast<RasterState>((static_cast<int>(m_currentRasterState) + 1) % 4);
}

void Render::UpdateShaders()
{
	m_deviceContext->VSSetShader(get<0>(m_vertexShaderMap[m_currentVertexShader]).Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShaderMap[m_currentPixelShader].Get(), nullptr, 0);

	if (!get<2>(m_vertexShaderMap[m_currentVertexShader])) CreateConstBuffer(sizeof(TestConstBuffer), &get<2>(m_vertexShaderMap[m_currentVertexShader]));
	ID3D11Buffer* constantBuffer = get<2>(m_vertexShaderMap[m_currentVertexShader]).Get();
	m_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
}

void Render::LoadAllShaders(const wchar_t* shaderPath, const char* entryPoint, const char* shaderModel)
{
	filesystem::path path(shaderPath);
	if (filesystem::exists(path) && filesystem::is_directory(path))
	{
		for (const auto& entry : filesystem::recursive_directory_iterator(shaderPath))
		{
			if (entry.path().extension() == L".hlsl")
			{
				if (entry.path().stem().wstring()[0] == L'V') LoadVertexShader(entry.path().c_str(), entryPoint, shaderModel); // Well this is cursed
				else LoadPixelShader(entry.path().c_str(), entryPoint, shaderModel);
			}
		}
	}
	for (const auto& entry : filesystem::directory_iterator(L"."))
	{
		if (entry.path().extension() == L".cso")
		{
			if (entry.path().stem().wstring()[0] == L'V') LoadPrecompiledVertexShader(entry.path().c_str());
			else LoadPrecompiledPixelShader(entry.path().c_str());
		}
	}
}

void Render::LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel)
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

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	comPtr<ID3D11InputLayout> inputLayout;
	CreateInputLayout(layoutDesc, _countof(layoutDesc), VSCode, &inputLayout);

	wstring shaderName = filesystem::path(file).stem().wstring();
	// Use predefined constant buffer and input layout for now
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
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	comPtr<ID3D11InputLayout> inputLayout;
	CreateInputLayout(layoutDesc, _countof(layoutDesc), VSCode, &inputLayout);

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

void Render::CreateShapeVertexBuffer()
{
	if (m_shapeVertexBuffers.find(Shapes::Triangle) == m_shapeVertexBuffers.end())
	{
		Vertex triangleVertices[] =
		{
			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
		};
		CreateVertexBuffer(sizeof(triangleVertices), &m_shapeVertexBuffers[Shapes::Triangle].first, triangleVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Triangle].second = 3;
	}
	if (m_shapeVertexBuffers.find(Shapes::Square) == m_shapeVertexBuffers.end())
	{
		Vertex squareVertices[] =
		{
			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
		};
		CreateVertexBuffer(sizeof(squareVertices), &m_shapeVertexBuffers[Shapes::Square].first, squareVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Square].second = 6;
	}
	if (m_shapeVertexBuffers.find(Shapes::Plane) == m_shapeVertexBuffers.end())
	{
		Vertex planeVertices[] =
		{
			{ XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f) }
		};
		CreateVertexBuffer(sizeof(planeVertices), &m_shapeVertexBuffers[Shapes::Plane].first, planeVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Plane].second = 6;
	}
	if (m_shapeVertexBuffers.find(Shapes::Tetrahedron) == m_shapeVertexBuffers.end())
	{
		Vertex tetrahedronVertices[] =
		{
			// Triangles
			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
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
		CreateVertexBuffer(sizeof(tetrahedronVertices), &m_shapeVertexBuffers[Shapes::Tetrahedron].first, tetrahedronVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Tetrahedron].second = 18;
	}
	if (m_shapeVertexBuffers.find(Shapes::Cube) == m_shapeVertexBuffers.end())
	{
		Vertex cubeVertices[] =
		{
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

			{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
		};
		CreateVertexBuffer(sizeof(cubeVertices), &m_shapeVertexBuffers[Shapes::Cube].first, cubeVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Cube].second = 36;
	}
	// Later need to make a way to load from blender for more complex shapes
	if (m_shapeVertexBuffers.find(Shapes::Icosphere) == m_shapeVertexBuffers.end())
	{
		constexpr float PHI = 1.61803398875f;

		XMFLOAT3 base[12] =
		{
			{ -1.0f,  PHI,  0.0f }, {  1.0f,  PHI,  0.0f }, { -1.0f, -PHI,  0.0f }, {  1.0f, -PHI,  0.0f },
			{  0.0f, -1.0f,  PHI }, {  0.0f,  1.0f,  PHI }, {  0.0f, -1.0f, -PHI }, {  0.0f,  1.0f, -PHI },
			{  PHI,   0.0f, -1.0f }, {  PHI,   0.0f,  1.0f }, { -PHI,  0.0f, -1.0f }, { -PHI,  0.0f,  1.0f }
		};

		XMFLOAT3 pos[12] = {};
		for (int i = 0; i < 12; ++i)
		{
			XMVECTOR v = XMLoadFloat3(&base[i]);
			v = XMVector3Normalize(v);
			XMStoreFloat3(&pos[i], v);
		}

		const uint16_t faces[20][3] =
		{
			{ 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
			{ 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
			{ 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
			{ 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 }
		};
		Vertex icosaVertices[60] = {};
		UINT w = 0;

		for (auto face : faces)
		{
			const uint16_t a = face[0];
			const uint16_t b = face[1];
			const uint16_t c = face[2];

			icosaVertices[w++] = Vertex{ pos[a], XMFLOAT4((pos[a].x + 1.0f) / 2.0f, (pos[a].y + 1.0f) / 2.0f, (pos[a].z + 1.0f) / 2.0f, 1.0f) };
			icosaVertices[w++] = Vertex{ pos[b], XMFLOAT4((pos[b].x + 1.0f) / 2.0f, (pos[b].y + 1.0f) / 2.0f, (pos[b].z + 1.0f) / 2.0f, 1.0f) };
			icosaVertices[w++] = Vertex{ pos[c], XMFLOAT4((pos[c].x + 1.0f) / 2.0f, (pos[c].y + 1.0f) / 2.0f, (pos[c].z + 1.0f) / 2.0f, 1.0f) };
		}

		CreateVertexBuffer(sizeof(icosaVertices), &m_shapeVertexBuffers[Shapes::Icosphere].first, icosaVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Icosphere].second = 60;
	}
	if (m_shapeVertexBuffers.find(Shapes::Tree) == m_shapeVertexBuffers.end())
	{
		Vertex treeVertices[] =
		{
			XMFLOAT3(-0.5f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f),
			XMFLOAT3(0.5f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f),

			XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			XMFLOAT3(-0.5f, 0.5f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f),
			XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f)
		};

		CreateVertexBuffer(sizeof(treeVertices), &m_shapeVertexBuffers[Shapes::Tree].first, treeVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::Tree].second = 6;
	}

	if (m_shapeVertexBuffers.find(Shapes::WindmillWing) == m_shapeVertexBuffers.end())
	{
		Vertex wingVertices[] =
		{
			XMFLOAT3(-1.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			XMFLOAT3(-1.0f, -0.5f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),

			XMFLOAT3(1.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
			XMFLOAT3(1.0f, -0.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),

			XMFLOAT3(-0.5f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
			XMFLOAT3(0.5f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),

			XMFLOAT3(-0.5f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			XMFLOAT3(0.5f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f)
		};

		CreateVertexBuffer(sizeof(wingVertices), &m_shapeVertexBuffers[Shapes::WindmillWing].first, wingVertices, sizeof(Vertex));
		m_shapeVertexBuffers[Shapes::WindmillWing].second = 12;
	}
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
		DrawText(L"Camera not found!", XMFLOAT2(m_deviceInfo.displayMode.Width / 2.0f, m_deviceInfo.displayMode.Height / 2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
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

		ID3D11Buffer* vertexBuffer = nullptr;
		if (m_shapeVertexBuffers.find(object->m_shape) != m_shapeVertexBuffers.end()) vertexBuffer = m_shapeVertexBuffers[object->m_shape].first.Get();
		else
		{
			MessageBoxW(nullptr, L"Shape not found in vertex buffer map", L"Error", MB_OK);
			continue;
		}

		if (m_currentVertexShader == VertexShaders::Default) m_deviceContext->VSSetShader(get<0>(m_vertexShaderMap[object->m_vertexShader]).Get(), nullptr, 0);
		else m_deviceContext->VSSetShader(get<0>(m_vertexShaderMap[m_currentVertexShader]).Get(), nullptr, 0);
		if (m_currentPixelShader == PixelShaders::Default) m_deviceContext->PSSetShader(m_pixelShaderMap[object->m_pixelShader].Get(), nullptr, 0);
		else m_deviceContext->PSSetShader(m_pixelShaderMap[m_currentPixelShader].Get(), nullptr, 0);

		m_deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		m_deviceContext->IASetInputLayout(get<3>(m_vertexShaderMap[object->m_vertexShader]).Get());
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMMATRIX worldMatrix = object->GetWorldMatrix();
		XMMATRIX viewMatrix = g_camera->GetViewMatrix();
		XMMATRIX projMatrix = g_camera->GetProjectionMatrix();

		TestConstBuffer constBufferData = {};
		constBufferData.world = XMMatrixTranspose(worldMatrix);
		constBufferData.view = XMMatrixTranspose(viewMatrix);
		constBufferData.projection = XMMatrixTranspose(projMatrix);
		constBufferData.WVP = XMMatrixTranspose(worldMatrix * viewMatrix * projMatrix);

		constBufferData.VSFloatA = sinf(static_cast<float>(ATime));
		constBufferData.VSFloatB = cosf(static_cast<float>(ATime));
		constBufferData.VSFloatC = -constBufferData.VSFloatA;
		constBufferData.VSFloatD = -constBufferData.VSFloatB;

		m_deviceContext->UpdateSubresource(get<2>(m_vertexShaderMap[object->m_vertexShader]).Get(), 0, nullptr, &constBufferData, 0, 0);

		m_deviceContext->Draw(m_shapeVertexBuffers[object->m_shape].second, 0);
	}
}

void Render::UpdateRenderMode()
{
	m_deviceContext->RSSetState(g_rasterState[static_cast<int>(m_currentRasterState)].Get());
}

Render::Render(HWND hWnd, int width, int height) : m_hWnd(hWnd)
{
	// Initialize device
	GetHardwareInfo();
	m_deviceInfo.displayMode.Width = m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.right - m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.left;
	m_deviceInfo.displayMode.Height = m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.bottom - m_deviceInfo.hardwareInfos[0].outputDescs[0].second.DesktopCoordinates.top;

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
	LoadAllShaders(L"../Engine/", "main", "5_0");
	UpdateShaders();

	CreateShapeVertexBuffer();
}

Render::~Render()
{
	// Clear device
	m_SpriteFontMap.clear();
	m_SpriteBatchMap.clear();
	m_device.Reset();

	// Not sure if this is necessary // microsoft doesnot recommend it
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

void Render::SetViewport(float topLeftX, float topLeftY)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = -topLeftX;
	viewport.TopLeftY = -topLeftY;
	viewport.Width = static_cast<FLOAT>(m_deviceInfo.displayMode.Width);
	viewport.Height = static_cast<FLOAT>(m_deviceInfo.displayMode.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_deviceContext->RSSetViewports(1, &viewport);
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

#undef comPtr