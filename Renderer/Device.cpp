#include "Device.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>

using namespace std;
using namespace DirectX;

#define DEVICE VDD::g_deviceInfo

VDD::DeviceInfo DEVICE = {};
UINT DxVersion;
UINT DxSubVersion;

unordered_map<wstring, unique_ptr<SpriteBatch>> g_SpriteBatchMap;
unordered_map<wstring, unique_ptr<SpriteFont>> g_SpriteFontMap;

static void CreateDeviceSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.Windowed = !DEVICE.isFullscreen;
	swapChainDesc.OutputWindow = g_hWnd;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc = DEVICE.displayMode;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = DEVICE.isVSync * DEVICE.displayMode.RefreshRate.Numerator;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.SampleDesc.Count = DEVICE.antiAliasingLevel;
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
					&DEVICE.featureLevels,
					1,
					D3D11_SDK_VERSION,
					&swapChainDesc,
					&DEVICE.swapChain,
					&DEVICE.device,
					nullptr,
					&DEVICE.context
				)
			)
		)
	{
		MessageBoxW(nullptr, L"Failed to create device and swap chain", L"Error", MB_OK);
		exit(-1);
	}
}

static void CreateRenderTarget()
{
	ComPtr<ID3D11Texture2D> backBuffer;

	if (FAILED(DEVICE.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
	{
		MessageBoxW(nullptr, L"Failed to get back buffer", L"Error", MB_OK);
		exit(-1);
	}

	if (FAILED(DEVICE.device->CreateRenderTargetView(backBuffer.Get(), nullptr, DEVICE.renderTargetView.GetAddressOf())))
	{
		MessageBox(nullptr, L"Failed to create render target view", L"Error", MB_OK);
		exit(-1);
	}
}

static void CreateDepthStencil()
{
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};

	depthStencilDesc.Width = DEVICE.displayMode.Width;
	depthStencilDesc.Height = DEVICE.displayMode.Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = DEVICE.antiAliasingLevel;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	if (FAILED(DEVICE.device->CreateTexture2D(&depthStencilDesc, nullptr, DEVICE.depthStencilBuffer.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create depth stencil texture", L"Error", MB_OK);
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = (DEVICE.antiAliasingLevel > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	if (FAILED(DEVICE.device->CreateDepthStencilView(DEVICE.depthStencilBuffer.Get(), &depthStencilViewDesc, DEVICE.depthStencilView.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create depth stencil view", L"Error", MB_OK);
		return;
	}
}

static void SetViewport()
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<FLOAT>(DEVICE.displayMode.Width);
	viewport.Height = static_cast<FLOAT>(DEVICE.displayMode.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	DEVICE.context->RSSetViewports(1, &viewport);
}

static void GetHardwareInfo()
{
	ComPtr<IDXGIAdapter1> padapter;
	ComPtr<IDXGIFactory1> pfactory;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), &pfactory)))
	{
		MessageBoxW(nullptr, L"Failed to create DXGI factory", L"Error", MB_OK);
		return;
	}

	for (UINT adapterIndex = 0; pfactory->EnumAdapters1(adapterIndex, &padapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		VDD::HardwareInfo hardwareInfo = {};
		hardwareInfo.adapterIndex = adapterIndex;
		if (FAILED(padapter->GetDesc1(&hardwareInfo.adapterDesc)))
		{
			MessageBoxW(nullptr, L"Failed to get adapter description", L"Error", MB_OK);
			return;
		}

		ComPtr<IDXGIOutput> poutput;
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

		DEVICE.hardwareInfos.push_back(hardwareInfo);
		padapter.Reset();
	}
}

void VDD::Initialize()
{
	CreateDeviceSwapChain();
	CreateRenderTarget();
	CreateDepthStencil();
	DEVICE.context->OMSetRenderTargets(1, DEVICE.renderTargetView.GetAddressOf(), DEVICE.depthStencilView.Get());
	SetViewport();
	VDD::LoadFont();

	GetHardwareInfo();
	DxVersion = (DeviceInfo().featureLevels & 0xf000) >> 12;
	DxSubVersion = (DeviceInfo().featureLevels & 0x0f00) >> 8;
}

void VDD::Release()
{
	g_SpriteFontMap.clear();
	g_SpriteBatchMap.clear();

	DEVICE.device.Reset();
	if (DEVICE.context)
	{
		DEVICE.context->ClearState();
		DEVICE.context->Flush();
	}
	DEVICE.context.Reset();
	DEVICE.swapChain.Reset();
	DEVICE.renderTargetView.Reset();
	DEVICE.depthStencilBuffer.Reset();
	DEVICE.depthStencilView.Reset();
	DEVICE.hardwareInfos.clear();
}

void VDD::ShowFrameRate()
{
	static UINT frameCount = 0;
	static double elapsedTime = 0.0;
	static double fps = 0.0;

	frameCount++;

	elapsedTime += GetdeltaTime<double>();

	if (elapsedTime >= 1.0)
	{
		fps = frameCount * elapsedTime;
		frameCount = 0;
		elapsedTime = 0.0;
	}

	wstring fpsText = L"FPS: " + to_wstring(static_cast<int>(fps));
	VDD::DrawText(fpsText.c_str(), XMFLOAT2(20.0f, 20.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void VDD::ClearBackBuffer(UINT flag, DirectX::XMFLOAT4 color, float depth, UINT8 stencil)
{
	DEVICE.context->ClearRenderTargetView(DEVICE.renderTargetView.Get(), reinterpret_cast<const float*>(&color));
	DEVICE.context->ClearDepthStencilView(DEVICE.depthStencilView.Get(), flag, depth, stencil);
}

void VDD::CreateInputLayout(ComPtr<ID3D11Device> device, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements, ComPtr<ID3DBlob> shaderCode, _Out_ ComPtr<ID3D11InputLayout>* inputLayout)
{
	if (FAILED(device->CreateInputLayout(layoutDesc, numElements, shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(), inputLayout->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create input layout", L"Error", MB_OK);
		return;
	}
}

void VDD::CreateVertexBuffer(ComPtr<ID3D11Device> device, UINT size, _Out_ ComPtr<ID3D11Buffer>* buffer, const void* initData, UINT stride)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = initData;

	if (FAILED(device->CreateBuffer(&bufferDesc, &subresourceData, buffer->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create buffer", L"Error", MB_OK);
		return;
	}
}

void VDD::CreateConstBuffer(ComPtr<ID3D11Device> device, UINT size, _Out_ ComPtr<ID3D11Buffer>* buffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&bufferDesc, nullptr, buffer->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create constant buffer", L"Error", MB_OK);
		return;
	}
}

void VDD::DisplayDeviceInfo()
{
	constexpr float offset = 20.0f;
	UINT posIndex = 2;

	// System Information
	posIndex++;
	VDD::DrawText(L"SYSTEM", XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	posIndex++;
	wstring dxVersion = L"DX Version: " + to_wstring(DxVersion) + L"." + to_wstring(DxSubVersion);
	VDD::DrawText(dxVersion.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

	posIndex++;
	wstring resolution = L"Resolution: " + to_wstring(DEVICE.displayMode.Width) + L"x" + to_wstring(DEVICE.displayMode.Height);
	VDD::DrawText(resolution.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
	posIndex++;

	// Hardware Information
	posIndex++;
	VDD::DrawText(L"HARDWARE", XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	for (const auto& hardwareInfo : DEVICE.hardwareInfos)
	{
		posIndex++;
		wstring adapterIndex = L"GPU " + to_wstring(hardwareInfo.adapterIndex) + L": " + wstring(hardwareInfo.adapterDesc.Description);
		VDD::DrawText(adapterIndex.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

		posIndex++;
		wstring vendorId = L"Vendor ID: " + to_wstring(hardwareInfo.adapterDesc.VendorId);
		VDD::DrawText(vendorId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring deviceId = L"Device ID: " + to_wstring(hardwareInfo.adapterDesc.DeviceId);
		VDD::DrawText(deviceId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring subSysId = L"SubSystem ID: " + to_wstring(hardwareInfo.adapterDesc.SubSysId);
		VDD::DrawText(subSysId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring revision = L"Revision: " + to_wstring(hardwareInfo.adapterDesc.Revision);
		VDD::DrawText(revision.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring vram = L"VRAM: " + to_wstring(hardwareInfo.adapterDesc.DedicatedVideoMemory / (1024 * 1024)) + L" MB";
		VDD::DrawText(vram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring sysram = L"System RAM: " + to_wstring(hardwareInfo.adapterDesc.DedicatedSystemMemory / (1024 * 1024)) + L" MB";
		VDD::DrawText(sysram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring sharedram = L"Shared RAM: " + to_wstring(hardwareInfo.adapterDesc.SharedSystemMemory / (1024 * 1024)) + L" MB";
		VDD::DrawText(sharedram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring adapterLuid = L"Adapter LUID: " + to_wstring(hardwareInfo.adapterDesc.AdapterLuid.LowPart) + L"," + to_wstring(hardwareInfo.adapterDesc.AdapterLuid.HighPart);
		VDD::DrawText(adapterLuid.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		for (const auto& outputDesc : hardwareInfo.outputDescs)
		{
			posIndex++;
			wstring outputInfo = L"Monitor: " + wstring(outputDesc.second.DeviceName);
			VDD::DrawText(outputInfo.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

			posIndex++;
			wstring resolution = L"Resolution: " + to_wstring(outputDesc.second.DesktopCoordinates.right - outputDesc.second.DesktopCoordinates.left) + L"x" + to_wstring(outputDesc.second.DesktopCoordinates.bottom - outputDesc.second.DesktopCoordinates.top);
			VDD::DrawText(resolution.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		}
		posIndex++;
	}
}

void VDD::LoadFont()
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
			unique_ptr<SpriteFont> spriteFont = make_unique<SpriteFont>(DEVICE.device.Get(), entry.path().c_str());
			unique_ptr<SpriteBatch> spriteBatch = make_unique<SpriteBatch>(DEVICE.context.Get());
			g_SpriteFontMap[fontName] = move(spriteFont);
			g_SpriteBatchMap[fontName] = move(spriteBatch);
		}
	}
}

void VDD::DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color, float scale, const wchar_t* font)
{
	wchar_t buffer[256] = {};
	wcsncpy_s(buffer, text, _TRUNCATE);

	FXMVECTOR colorVector = DirectX::XMLoadFloat4(&color);

	if (g_SpriteFontMap.find(font) != g_SpriteFontMap.end())
	{
		g_SpriteBatchMap[font]->Begin();
		g_SpriteFontMap[font]->DrawString(g_SpriteBatchMap[font].get(), buffer, position, colorVector, 0.0f, XMFLOAT2(0.0f, 0.0f), scale);
		g_SpriteBatchMap[font]->End();
	}
}