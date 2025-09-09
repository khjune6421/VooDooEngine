#include "Shader.h"

#include <d3dcompiler.h>

#include "Device.h"

#pragma comment(lib, "d3dcompiler.lib")

#define DEVICE VDD::g_deviceInfo

ComPtr<ID3D11VertexShader> VDS::g_vertexShader = nullptr;
ComPtr<ID3DBlob> VDS::g_VSCode = nullptr;

ComPtr<ID3D11PixelShader> VDS::g_pixelShader = nullptr;

ComPtr<ID3D11Buffer> VDS::g_constantBuffer = nullptr;

void VDS::CreateShader()
{
	VDS::CompileVertexShader(L"../Renderer/VertexShader.hlsl", "main", "vs_5_0", &VDS::g_vertexShader, &VDS::g_VSCode);
	VDS::CompilePixelShader(L"../Renderer/PixelShader.hlsl", "main", "ps_5_0", &VDS::g_pixelShader);
	// Need to make something that can use cso flies if there is one already compiled and hlsl has not changed

	DEVICE.context->VSSetShader(VDS::g_vertexShader.Get(), nullptr, 0);
	DEVICE.context->PSSetShader(VDS::g_pixelShader.Get(), nullptr, 0);

	VDD::CreateConstBuffer(DEVICE.device, sizeof(VDS::ConstBuffer), &VDS::g_constantBuffer);
}

void VDS::UpdateShader()
{
    DEVICE.context->VSSetShader(VDS::g_vertexShader.Get(), nullptr, 0);
	DEVICE.context->PSSetShader(VDS::g_pixelShader.Get(), nullptr, 0);

    ID3D11Buffer* constantBuffer = VDS::g_constantBuffer.Get();
    DEVICE.context->VSSetConstantBuffers(0, 1, &constantBuffer);
}

void VDS::ReleaseShader()
{
	VDS::g_constantBuffer.Reset();
	VDS::g_VSCode.Reset();
	VDS::g_pixelShader.Reset();
	VDS::g_vertexShader.Reset();
}

void VDS::CompileVertexShader(const wchar_t* filePath, const char* entryPoint, const char* shaderModel, ComPtr<ID3D11VertexShader>* vertexShader, ComPtr<ID3DBlob>* VSCode)
{
	ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	if (FAILED(D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, compileFlags, 0, VSCode->GetAddressOf(), errorBlob.GetAddressOf())))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		MessageBoxW(nullptr, L"Failed to compile vertex shader", L"Error", MB_OK);
		return;
	}

	if (FAILED(DEVICE.device->CreateVertexShader((*VSCode)->GetBufferPointer(), (*VSCode)->GetBufferSize(), nullptr, vertexShader->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create vertex shader", L"Error", MB_OK);
		return;
	}
}

void VDS::CompilePixelShader(const wchar_t* filePath, const char* entryPoint, const char* shaderModel, ComPtr<ID3D11PixelShader>* pixelShader)
{
	ComPtr<ID3DBlob> PSCode;
	ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	if (FAILED(D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, compileFlags, 0, PSCode.GetAddressOf(), errorBlob.GetAddressOf())))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		MessageBoxW(nullptr, L"Failed to compile pixel shader", L"Error", MB_OK);
		return;
	}

	if (FAILED(DEVICE.device->CreatePixelShader(PSCode->GetBufferPointer(), PSCode->GetBufferSize(), nullptr, pixelShader->GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
		return;
	}
}