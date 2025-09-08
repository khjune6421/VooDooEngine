#include "Render.h"

#include "Device.h"
#include "Shader.h"
#include "Camera.h"

using namespace DirectX;

#define DEVICE VDD::g_deviceInfo

constexpr XMFLOAT4 g_clearColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

enum class RasterState
{
	Wireframe_CullNone = 0,
	Wireframe_CullBack = 1,
	Solid_CullNone = 2,
	Solid_CullBack = 3
};

RasterState g_currentRasterState = RasterState::Wireframe_CullNone;

// 0: Wireframe CullNone, 1: Wireframe CullBack, 2: Solid CullNone, 3: Solid CullBack
ComPtr<ID3D11RasterizerState> g_rasterState[4] = { nullptr, nullptr, nullptr, nullptr };
ComPtr<ID3D11Buffer> g_vertexBuffer = nullptr;
ComPtr<ID3D11InputLayout> g_inputLayout = nullptr;

Camera g_testCamera;

void CreateRasterState();
float EngineUpdate();

void CreateTestObject();
void UpdateTestObject(float deltaTime);
void DrawTestObject();

static void ShowInfo()
{
	VDD::ShowFrameRate();
	VDD::DisplayDeviceInfo();
}

void VDR::LoadData()
{
	VDS::CreateShader();
	CreateRasterState();

	// Using vertex in code for now, will later use to show if rendering works in debug mode
	CreateTestObject();
}

void VDR::ReleaseData()
{
	VDS::ReleaseShader();
	for (auto& i : g_rasterState) i.Reset();
	g_inputLayout.Reset();
	g_vertexBuffer.Reset();
}

void VDR::SceneRender()
{
	float deltaTime = EngineUpdate();

	UpdateTestObject(deltaTime);

	VDD::ClearBackBuffer(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, g_clearColor, 1.0f, 0);

	DrawTestObject();

#ifdef _DEBUG
	ShowInfo();
#endif

	VDD::PresentBackBuffer();
}

void VDR::UpdateRenderMode()
{
	if (g_currentRasterState == RasterState::Wireframe_CullNone) DEVICE.context->RSSetState(g_rasterState[0].Get());
	else if (g_currentRasterState == RasterState::Wireframe_CullBack) DEVICE.context->RSSetState(g_rasterState[1].Get());
	else if (g_currentRasterState == RasterState::Solid_CullNone) DEVICE.context->RSSetState(g_rasterState[2].Get());
	else if (g_currentRasterState == RasterState::Solid_CullBack) DEVICE.context->RSSetState(g_rasterState[3].Get());
}

void CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc = {};

	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = TRUE; // Base value is FALSE
	rasterDesc.AntialiasedLineEnable = TRUE; // Base value is FALSE

	if (FAILED(DEVICE.device->CreateRasterizerState(&rasterDesc, g_rasterState[0].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state (Wireframe CullNone)", L"Error", MB_OK);
		return;
	}

	rasterDesc.CullMode = D3D11_CULL_BACK;
	if (FAILED(DEVICE.device->CreateRasterizerState(&rasterDesc, g_rasterState[1].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state (Wireframe CullBack)", L"Error", MB_OK);
		return;
	}

	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	if (FAILED(DEVICE.device->CreateRasterizerState(&rasterDesc, g_rasterState[2].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state (Solid CullNone)", L"Error", MB_OK);
		return;
	}

	rasterDesc.CullMode = D3D11_CULL_BACK;
	if (FAILED(DEVICE.device->CreateRasterizerState(&rasterDesc, g_rasterState[3].GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create rasterizer state (Solid CullBack)", L"Error", MB_OK);
		return;
	}
}

float EngineUpdate()
{
	float deltaTime = VDD::GetdeltaTime();

	if (GetAsyncKeyState(VK_F4) & 0x01) g_currentRasterState = static_cast<RasterState>((static_cast<int>(g_currentRasterState) + 1) % 4);

	//SystemUpdate(deltaTime);
	VDR::UpdateRenderMode();
	VDS::UpdateShader();

	return deltaTime;
}

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

void CreateTestObject()
{
	Vertex vertices[] =
	{
		{ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 0.75f, 0.8f, 1.0f) },

		{ XMFLOAT3(0.25f, -0.25f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.25f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

		{ XMFLOAT3(-0.25f, -0.25f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.25f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	};
	VDD::CreateVertexBuffer(DEVICE.device, sizeof(vertices), &g_vertexBuffer, vertices, sizeof(Vertex));

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	VDD::CreateInputLayout(DEVICE.device, inputElementDesc, _countof(inputElementDesc), VDS::g_VSCode, &g_inputLayout); // Used _countof in modern C++ instead of ARRAYSIZE for safety
}

void UpdateTestObject(float deltaTime)
{
	static float rotationAngle = 0.0f;
	rotationAngle += deltaTime * 1.0f;

	XMMATRIX worldMatrix = XMMatrixRotationY(rotationAngle);

	VDS::ConstBuffer constBufferData = {};
	XMMATRIX viewMatrix = g_testCamera.GetViewMatrix();
	XMMATRIX projMatrix = g_testCamera.GetProjectionMatrix();

	constBufferData.world = XMMatrixTranspose(worldMatrix);
	constBufferData.view = XMMatrixTranspose(viewMatrix);
	constBufferData.projection = XMMatrixTranspose(projMatrix);
	constBufferData.WVP = XMMatrixTranspose(worldMatrix * viewMatrix * projMatrix);

	DEVICE.context->UpdateSubresource(VDS::g_constantBuffer.Get(), 0, nullptr, &constBufferData, 0, 0);
}

void DrawTestObject()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffer = g_vertexBuffer.Get();
	DEVICE.context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	DEVICE.context->IASetInputLayout(g_inputLayout.Get());
	DEVICE.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DEVICE.context->Draw(30, 0);
}