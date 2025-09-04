#include "Device.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>

using namespace std;
using namespace DirectX;

#define DEVICE VDR::g_DeviceInfo

VDR::DeviceInfo DEVICE = {};
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
	swapChainDesc.SampleDesc.Count = 1;
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
		MessageBoxW(0, L"Failed to create device and swap chain", L"Error", MB_OK);
		exit(-1);
	}
}

static void CreateRenderTarget()
{
	ID3D11Texture2D* backBuffer = nullptr;

	if (FAILED(DEVICE.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
	{
		MessageBoxW(0, L"Failed to get back buffer", L"Error", MB_OK);
		exit(-1);
	}

	if (FAILED(DEVICE.device->CreateRenderTargetView(backBuffer, nullptr, &DEVICE.renderTargetView)))
	{
		if (backBuffer) { backBuffer->Release(); backBuffer = nullptr; }
		MessageBox(0, L"Failed to create render target view", L"Error", MB_OK);
		exit(-1);
	}
	if (backBuffer) { backBuffer->Release(); backBuffer = nullptr; }
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
	IDXGIAdapter1* padapter = nullptr;
	IDXGIFactory1* pfactory = nullptr;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pfactory))))
	{
		MessageBoxW(0, L"Failed to create DXGI factory", L"Error", MB_OK);
		return;
	}

	for (UINT adapterIndex = 0; pfactory->EnumAdapters1(adapterIndex, &padapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		VDR::HardwareInfo hardwareInfo = {};
		hardwareInfo.adapterIndex = adapterIndex;
		if (FAILED(padapter->GetDesc1(&hardwareInfo.adapterDesc)))
		{
			if (padapter) { padapter->Release(); padapter = nullptr; }
			MessageBoxW(0, L"Failed to get adapter description", L"Error", MB_OK);
			return;
		}

		IDXGIOutput* poutput = nullptr;
		for (UINT outputIndex = 0; padapter->EnumOutputs(outputIndex, &poutput) != DXGI_ERROR_NOT_FOUND; ++outputIndex)
		{
			DXGI_OUTPUT_DESC outputDesc = {};
			if (FAILED(poutput->GetDesc(&outputDesc)))
			{
				if (poutput) { poutput->Release(); poutput = nullptr; }
				if (padapter) { padapter->Release(); padapter = nullptr; }
				MessageBoxW(0, L"Failed to get output description", L"Error", MB_OK);
				return;
			}
			hardwareInfo.outputDescs.push_back(make_pair(outputIndex, outputDesc));
			if (poutput) { poutput->Release(); poutput = nullptr; }
		}

		DEVICE.hardwareInfos.push_back(hardwareInfo);
		if (padapter) { padapter->Release(); padapter = nullptr; }
	}
	if (pfactory) { pfactory->Release(); pfactory = nullptr; }
}

void VDR::Initialize()
{
	CreateDeviceSwapChain();
	CreateRenderTarget();
	DEVICE.context->OMSetRenderTargets(1, &DEVICE.renderTargetView, nullptr);
	SetViewport();
	VDR::LoadFont();

	GetHardwareInfo();
	DxVersion = (DeviceInfo().featureLevels & 0xf000) >> 12;
	DxSubVersion = (DeviceInfo().featureLevels & 0x0f00) >> 8;
}

void VDR::Release()
{
	g_SpriteFontMap.clear();
	g_SpriteBatchMap.clear();

	if (DEVICE.renderTargetView) { DEVICE.renderTargetView->Release(); DEVICE.renderTargetView = nullptr; }
	if (DEVICE.swapChain) { DEVICE.swapChain->Release(); DEVICE.swapChain = nullptr; }
	if (DEVICE.context) { DEVICE.context->ClearState(); DEVICE.context->Flush(); DEVICE.context->Release(); DEVICE.context = nullptr; }
	if (DEVICE.device) { DEVICE.device->Release(); DEVICE.device = nullptr; }
}

double VDR::GetdeltaTime()
{
	static ULONGLONG previousTime = GetTickCount64();
	ULONGLONG currentTime = GetTickCount64();
	double deltaTime = currentTime - previousTime;
	previousTime = currentTime;

	return deltaTime;
}

void VDR::ShowFrameRate()
{
	static UINT frameCount = 0;
	static double elapsedTime = 0.0;
	static double fps = 0.0;

	frameCount++;

	elapsedTime += GetdeltaTime();

	if (elapsedTime >= 1000.0)
	{
		fps = frameCount * 1000.0 / elapsedTime;
		frameCount = 0;
		elapsedTime = 0.0;
	}

	wstring fpsText = L"FPS: " + to_wstring(static_cast<int>(fps));
	VDR::DrawText(fpsText.c_str(), XMFLOAT2(20.0f, 20.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void VDR::DisplayDeviceInfo()
{
	constexpr float offset = 20.0f;
	UINT posIndex = 2;

	// System Information
	posIndex++;
	VDR::DrawText(L"SYSTEM", XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	posIndex++;
	wstring dxVersion = L"DX Version: " + to_wstring(DxVersion) + L"." + to_wstring(DxSubVersion);
	VDR::DrawText(dxVersion.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

	posIndex++;
	wstring resolution = L"Resolution: " + to_wstring(DEVICE.displayMode.Width) + L"x" + to_wstring(DEVICE.displayMode.Height);
	VDR::DrawText(resolution.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
	posIndex++;

	// Hardware Information
	posIndex++;
	VDR::DrawText(L"HARDWARE", XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	for (const auto& hardwareInfo : DEVICE.hardwareInfos)
	{
		posIndex++;
		wstring adapterIndex = L"GPU " + to_wstring(hardwareInfo.adapterIndex) + L": " + wstring(hardwareInfo.adapterDesc.Description);
		VDR::DrawText(adapterIndex.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

		posIndex++;
		wstring vendorId = L"Vendor ID: " + to_wstring(hardwareInfo.adapterDesc.VendorId);
		VDR::DrawText(vendorId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring deviceId = L"Device ID: " + to_wstring(hardwareInfo.adapterDesc.DeviceId);
		VDR::DrawText(deviceId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring subSysId = L"SubSystem ID: " + to_wstring(hardwareInfo.adapterDesc.SubSysId);
		VDR::DrawText(subSysId.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring revision = L"Revision: " + to_wstring(hardwareInfo.adapterDesc.Revision);
		VDR::DrawText(revision.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring vram = L"VRAM: " + to_wstring(hardwareInfo.adapterDesc.DedicatedVideoMemory / (1024 * 1024)) + L" MB";
		VDR::DrawText(vram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring sysram = L"System RAM: " + to_wstring(hardwareInfo.adapterDesc.DedicatedSystemMemory / (1024 * 1024)) + L" MB";
		VDR::DrawText(sysram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring sharedram = L"Shared RAM: " + to_wstring(hardwareInfo.adapterDesc.SharedSystemMemory / (1024 * 1024)) + L" MB";
		VDR::DrawText(sharedram.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		posIndex++;
		wstring adapterLuid = L"Adapter LUID: " + to_wstring(hardwareInfo.adapterDesc.AdapterLuid.LowPart) + L"," + to_wstring(hardwareInfo.adapterDesc.AdapterLuid.HighPart);
		VDR::DrawText(adapterLuid.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

		for (const auto& outputDesc : hardwareInfo.outputDescs)
		{
			posIndex++;
			wstring outputInfo = L"Monitor: " + wstring(outputDesc.second.DeviceName);
			VDR::DrawText(outputInfo.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));

			posIndex++;
			wstring resolution = L"Resolution: " + to_wstring(outputDesc.second.DesktopCoordinates.right - outputDesc.second.DesktopCoordinates.left) + L"x" + to_wstring(outputDesc.second.DesktopCoordinates.bottom - outputDesc.second.DesktopCoordinates.top);
			VDR::DrawText(resolution.c_str(), XMFLOAT2(offset, offset * posIndex), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		}
		posIndex++;
	}
}

void VDR::LoadFont()
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
			unique_ptr<SpriteFont> spriteFont = make_unique<SpriteFont>(DEVICE.device, entry.path().c_str());
			unique_ptr<SpriteBatch> spriteBatch = make_unique<SpriteBatch>(DEVICE.context);
			g_SpriteFontMap[fontName] = move(spriteFont);
			g_SpriteBatchMap[fontName] = move(spriteBatch);
		}
	}
}

void VDR::DrawText(const wchar_t* text, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color, float scale, const wchar_t* font)
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