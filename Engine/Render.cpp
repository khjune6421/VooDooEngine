#include "Render.h"
#include "Object.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace DirectX;

#define comPtr Microsoft::WRL::ComPtr

unordered_map<Shapes, pair<comPtr<ID3D11Buffer>, UINT>> Render::s_shapeVertexBuffers;

// Test object and camera
Camera g_camera = Camera();

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

// Default shader paths and names
constexpr const wchar_t* DEFAULT_SHADER_PATH = L"../Engine/";
constexpr const wchar_t* DEFAULT_VS = L"VertexShader";
constexpr const wchar_t* DEFAULT_PS = L"PixelShader";
constexpr const wchar_t* NullPS = L"NullPixelShader";
constexpr const wchar_t* AllPS = L"AllPixelShader";
constexpr const char* DEFAULT_ENTRY_POINT = "main";
constexpr const char* DEFAULT_SHADERMODEL = "5_0";

void Render::CreateShaders()
{
	LoadVertexShader(DEFAULT_VS, DEFAULT_ENTRY_POINT, ("vs_" + string(DEFAULT_SHADERMODEL)).c_str(), &m_vertexShader, &m_VSCode);
	LoadPixelShader(DEFAULT_PS, DEFAULT_ENTRY_POINT, ("ps_" + string(DEFAULT_SHADERMODEL)).c_str(), &m_pixelShader);

	m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	CreateConstBuffer(sizeof(TestConstBuffer), &m_constantBuffer);
}

constexpr const wchar_t* SHADERS[3] = { DEFAULT_PS, NullPS, AllPS };

void Render::ChangeShader(UINT id)
{
	LoadPixelShader(SHADERS[id], DEFAULT_ENTRY_POINT, ("ps_" + string(DEFAULT_SHADERMODEL)).c_str(), &m_pixelShader);

	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	CreateConstBuffer(sizeof(TestConstBuffer), &m_constantBuffer);
}

void Render::ChangeState()
{
	m_currentRasterState = static_cast<RasterState>((static_cast<int>(m_currentRasterState) + 1) % 4);
}

void Render::UpdateShaders()
{
	m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	ID3D11Buffer* constantBuffer = m_constantBuffer.Get();
	m_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
}

void Render::LoadVertexShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, _Out_ comPtr<ID3D11VertexShader>* vertexShader, _Out_ comPtr<ID3DBlob>* VSCode)
{
	comPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr;

	hr = D3DReadFileToBlob((wstring(file) + L".cso").c_str(), VSCode->GetAddressOf()); // Try to load precompiled shader first
	if (FAILED(hr)) hr = D3DCompileFromFile(((wstring(DEFAULT_SHADER_PATH) + file) + L".hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, compileFlags, 0, VSCode->GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr)) MessageBoxW(nullptr, L"Failed to compile vertex shader", L"Error", MB_OK);

	hr = m_device->CreateVertexShader(VSCode->Get()->GetBufferPointer(), VSCode->Get()->GetBufferSize(), nullptr, vertexShader->GetAddressOf());
	if (FAILED(hr)) MessageBoxW(nullptr, L"Failed to create vertex shader", L"Error", MB_OK);
}

void Render::LoadPixelShader(const wchar_t* file, const char* entryPoint, const char* shaderModel, _Out_ comPtr<ID3D11PixelShader>* pixelShader)
{
	comPtr<ID3DBlob> psCode;
	comPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr;

	hr = D3DReadFileToBlob((wstring(file) + L".cso").c_str(), psCode.GetAddressOf());
	if (FAILED(hr)) hr = D3DCompileFromFile(((wstring(DEFAULT_SHADER_PATH) + file) + L".hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, compileFlags, 0, psCode.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr)) MessageBoxW(nullptr, L"Failed to compile pixel shader", L"Error", MB_OK);

	hr = m_device->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), nullptr, pixelShader->GetAddressOf());
	if (FAILED(hr)) MessageBoxW(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
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

void Render::SetInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CreateInputLayout(layoutDesc, _countof(layoutDesc), m_VSCode, &m_inputLayout);
}

float Render::EngineUpdate()
{
	float deltaTime = VDGM::g_deltaTimeF;

	UpdateRenderMode();
	UpdateShaders();

	//UpdateTestObject(deltaTime);

	return deltaTime;
}

void Render::DrawObjects()
{
	static float ATime = 0.0f; // Accumulated time
	ATime += VDGM::g_deltaTimeF;

	for (Object* object : VDGM::g_objects)
	{
		if (!object || !object->m_isActive) continue;
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		ID3D11Buffer* vertexBuffer = nullptr;
		if (s_shapeVertexBuffers.find(object->m_shape) != s_shapeVertexBuffers.end()) vertexBuffer = s_shapeVertexBuffers[object->m_shape].first.Get();
		else
		{
			MessageBoxW(nullptr, L"Shape not found in vertex buffer map", L"Error", MB_OK);
			continue;
		}

		m_deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		m_deviceContext->IASetInputLayout(m_inputLayout.Get());
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		TestConstBuffer constBufferData = {};
		XMMATRIX worldMatrix = object->GetWorldMatrix();
		XMMATRIX viewMatrix = g_camera.GetViewMatrix();
		XMMATRIX projMatrix = g_camera.GetProjectionMatrix();

		constBufferData.world = XMMatrixTranspose(worldMatrix);
		constBufferData.view = XMMatrixTranspose(viewMatrix);
		constBufferData.projection = XMMatrixTranspose(projMatrix);

		constBufferData.VSFloatA = sinf(static_cast<float>(ATime));
		constBufferData.VSFloatB = cosf(static_cast<float>(ATime));
		constBufferData.VSFloatC = -constBufferData.VSFloatA;
		constBufferData.VSFloatD = -constBufferData.VSFloatB;

		constBufferData.WVP = XMMatrixTranspose(worldMatrix * viewMatrix * projMatrix);

		m_deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &constBufferData, 0, 0);

		m_deviceContext->Draw(s_shapeVertexBuffers[object->m_shape].second, 0);
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

	// Initialize shaders
	CreateShaders();

	// Initialize render
	CreateRasterState();
	SetInputLayout();

	// Initialize camera // this should be moved to somewhere else later
	g_camera.SetScreenSize(static_cast<int>(m_deviceInfo.displayMode.Width), static_cast<int>(m_deviceInfo.displayMode.Height));
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

	// Clear shaders
	m_vertexShader.Reset();
	m_VSCode.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();

	// Clear render
	for (auto& state : g_rasterState) state.Reset();
	m_inputLayout.Reset();
}

void Render::CreateShapeVertexBuffer()
{
	if (s_shapeVertexBuffers.find(Shapes::Triangle) == s_shapeVertexBuffers.end())
	{
		Vertex triangleVertices[] =
		{
			{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
		};
		CreateVertexBuffer(sizeof(triangleVertices), &s_shapeVertexBuffers[Shapes::Triangle].first, triangleVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::Triangle].second = 3;
	}
	if (s_shapeVertexBuffers.find(Shapes::Square) == s_shapeVertexBuffers.end())
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
		CreateVertexBuffer(sizeof(squareVertices), &s_shapeVertexBuffers[Shapes::Square].first, squareVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::Square].second = 6;
	}
	if (s_shapeVertexBuffers.find(Shapes::Plane) == s_shapeVertexBuffers.end())
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
		CreateVertexBuffer(sizeof(planeVertices), &s_shapeVertexBuffers[Shapes::Plane].first, planeVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::Plane].second = 6;
	}
	if (s_shapeVertexBuffers.find(Shapes::Tetrahedron) == s_shapeVertexBuffers.end())
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
		CreateVertexBuffer(sizeof(tetrahedronVertices), &s_shapeVertexBuffers[Shapes::Tetrahedron].first, tetrahedronVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::Tetrahedron].second = 18;
	}
	if (s_shapeVertexBuffers.find(Shapes::Cube) == s_shapeVertexBuffers.end())
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
		CreateVertexBuffer(sizeof(cubeVertices), &s_shapeVertexBuffers[Shapes::Cube].first, cubeVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::Cube].second = 36;
	}
	if (s_shapeVertexBuffers.find(Shapes::Tree) == s_shapeVertexBuffers.end())
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

		CreateVertexBuffer(sizeof(treeVertices), &s_shapeVertexBuffers[Shapes::Tree].first, treeVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::Tree].second = 6;
	}

	if (s_shapeVertexBuffers.find(Shapes::WindmillWing) == s_shapeVertexBuffers.end())
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

		CreateVertexBuffer(sizeof(wingVertices), &s_shapeVertexBuffers[Shapes::WindmillWing].first, wingVertices, sizeof(Vertex));
		s_shapeVertexBuffers[Shapes::WindmillWing].second = 12;
	}
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
		m_SpriteBatchMap[fontName]->Begin();
		m_SpriteFontMap[fontName]->DrawString(m_SpriteBatchMap[fontName].get(), buffer, position, colorVector, 0.0f, XMFLOAT2(0.0f, 0.0f), scale);
		m_SpriteBatchMap[fontName]->End();
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