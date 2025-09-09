#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace VDS
{
	extern ComPtr<ID3D11VertexShader> g_vertexShader;
	extern ComPtr<ID3DBlob> g_VSCode;

	extern ComPtr<ID3D11PixelShader> g_pixelShader;

	extern ComPtr<ID3D11Buffer> g_constantBuffer;

	struct ConstBuffer
	{
		// Not sure if needed
		DirectX::XMMATRIX world; // world matrix
		DirectX::XMMATRIX view; // view matrix
		DirectX::XMMATRIX projection; // projection matrix

		DirectX::XMMATRIX WVP; // world-view-projection matrix

		float time;
	};

	void CreateShader();
	void UpdateShader();
	void ReleaseShader();

	void LoadVertexShader(const wchar_t* filePath, const char* entryPoint, const char* shaderModel, ComPtr<ID3D11VertexShader>* vertexShader, ComPtr<ID3DBlob>* shaderCode = nullptr);
	void LoadPixelShader(const wchar_t* filePath, const char* entryPoint, const char* shaderModel, ComPtr<ID3D11PixelShader>* pixelShader);
}