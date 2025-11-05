#include "pch.h"
#include "Light.h"

#include "Object.h"
#include "Scene.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

void PointLight::OnAttached(Object* owner)
{
	Component::OnAttached(owner);

	owner->m_scene->m_pointLights.push_back(this);
}

void PointLight::OnDetached()
{
	if (!m_owner->m_scene->m_pointLights.empty())
	{
		auto it = find(m_owner->m_scene->m_pointLights.begin(), m_owner->m_scene->m_pointLights.end(), this);
		if (it != m_owner->m_scene->m_pointLights.end()) m_owner->m_scene->m_pointLights.erase(it);
	}
	Component::OnDetached();
}

PointLightConstBuffer& PointLight::GetLightData()
{
	m_lightData.position = m_owner->GetWorldPosition();
	XMVECTOR dir = m_owner->GetWorldDirection(Directions::Forward);
	
	m_lightData.directionAndAngle.x = XMVectorGetX(dir);
	m_lightData.directionAndAngle.y = XMVectorGetY(dir);
	m_lightData.directionAndAngle.z = XMVectorGetZ(dir);

	return m_lightData;
}

#define comPtr Microsoft::WRL::ComPtr

void PointLight::CreateShadowMap(Renderer* renderer)
{
	D3D11_TEXTURE2D_DESC shadowCubeDesc = {};
	shadowCubeDesc.Width = SHADOW_MAP_SIZE;
	shadowCubeDesc.Height = SHADOW_MAP_SIZE;
	shadowCubeDesc.MipLevels = 1;
	shadowCubeDesc.ArraySize = 6; // Cube map
	shadowCubeDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowCubeDesc.SampleDesc.Count = 1;
	shadowCubeDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowCubeDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowCubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (FAILED(renderer->m_device->CreateTexture2D(&shadowCubeDesc, nullptr, m_shadowMapTexture.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create shadow cube map texture", L"Error", MB_OK);
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1;
	for (UINT i = 0; i < 6; ++i)
	{
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(renderer->m_device->CreateDepthStencilView(m_shadowMapTexture.Get(), &dsvDesc, m_shadowMapDSVs[i].GetAddressOf())))
		{
			MessageBoxW(nullptr, L"Failed to create shadow cube map DSV", L"Error", MB_OK);
			return;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	if (FAILED(renderer->m_device->CreateShaderResourceView(m_shadowMapTexture.Get(), &srvDesc, m_shadowMapSRV.GetAddressOf())))
	{
		MessageBoxW(nullptr, L"Failed to create shadow cube map SRV", L"Error", MB_OK);
		return;
	}
	XMVECTOR lightPos = m_lightData.position;
	float lightRange = m_lightData.range;

	D3D11_VIEWPORT shadowViewport = {};
	shadowViewport.TopLeftX = 0.0f;
	shadowViewport.TopLeftY = 0.0f;
	shadowViewport.Width = static_cast<FLOAT>(SHADOW_MAP_SIZE);
	shadowViewport.Height = static_cast<FLOAT>(SHADOW_MAP_SIZE);
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;
	renderer->m_deviceContext->RSSetViewports(1, &shadowViewport);

	XMMATRIX lightProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, lightRange);

	XMVECTOR targets[6] =
	{
		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),   // +X
		XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f),  // -X
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),   // +Y
		XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f),  // -Y
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),   // +Z
		XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)   // -Z
	};
	XMVECTOR ups[6] =
	{
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),   // +X
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),   // -X
		XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),  // +Y
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),   // -Y
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),   // +Z
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)    // -Z
	};

	renderer->m_deviceContext->IASetInputLayout(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"DepthOnlyVertexShader"]].second.Get());
	renderer->m_deviceContext->VSSetShader(renderer->m_vertexShaderMap[g_vertexShaderIdMap[L"DepthOnlyVertexShader"]].first.Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetShader(renderer->m_pixelShaderMap[g_pixelShaderIdMap[L"DepthOnlyPixelShader"]].Get(), nullptr, 0);
	renderer->m_deviceContext->PSSetSamplers(0, 1, renderer->m_samplers[Renderer::DefaultSampler].GetAddressOf());


	XMFLOAT4 lightData = XMFLOAT4(XMVectorGetX(lightPos), XMVectorGetY(lightPos), XMVectorGetZ(lightPos), lightRange);
	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::LightPosBuffer].Get(), 0, nullptr, &lightData, 0, 0);
	renderer->m_deviceContext->PSSetConstantBuffers(1, 1, renderer->m_constBuffers[Renderer::LightPosBuffer].GetAddressOf());

	for (UINT face = 0; face < 6; ++face)
	{
		ID3D11RenderTargetView* nullRTV = nullptr;
		renderer->m_deviceContext->OMSetRenderTargets(1, &nullRTV, m_shadowMapDSVs[face].Get());
		renderer->m_deviceContext->ClearDepthStencilView(m_shadowMapDSVs[face].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		XMVECTOR target = XMVectorAdd(lightPos, targets[face]);
		XMMATRIX lightView = XMMatrixLookAtLH(lightPos, target, ups[face]);

		MatrixConstBuffer lightMatrixBuffer = {};
		lightMatrixBuffer.view = XMMatrixTranspose(lightView);
		lightMatrixBuffer.projection = XMMatrixTranspose(lightProjection);

		VDGM::g_currentScene->RenderShadows(renderer, &lightMatrixBuffer);
	}
}

#undef comPtr