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

void PointLight::CreateShadowMap(Renderer* renderer, UINT index) const
{
	XMVECTOR lightPos = m_lightData.position;
	float lightRange = m_lightData.range;

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


	XMFLOAT4 lightData = XMFLOAT4(XMVectorGetX(lightPos), XMVectorGetY(lightPos), XMVectorGetZ(lightPos), lightRange);
	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::LightPosBuffer].Get(), 0, nullptr, &lightData, 0, 0);
	renderer->m_deviceContext->PSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::LightPosBuffer].GetAddressOf());

	for (UINT face = 0; face < 6; ++face)
	{
 		renderer->m_deviceContext->OMSetRenderTargets(1, renderer->m_shadowMapArrayRTV.GetAddressOf(), renderer->m_shadowMapDSVs[static_cast<vector<com_ptr<ID3D11DepthStencilView>, allocator<com_ptr<ID3D11DepthStencilView>>>::size_type>(index) + face].Get());
		renderer->m_deviceContext->ClearDepthStencilView(renderer->m_shadowMapDSVs[static_cast<vector<com_ptr<ID3D11DepthStencilView>, allocator<com_ptr<ID3D11DepthStencilView>>>::size_type>(index) + face].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		XMVECTOR target = XMVectorAdd(lightPos, targets[face]);
		XMMATRIX lightView = XMMatrixLookAtLH(lightPos, target, ups[face]);

		MatrixConstBuffer lightMatrixBuffer = {};
		lightMatrixBuffer.view = XMMatrixTranspose(lightView);
		lightMatrixBuffer.projection = XMMatrixTranspose(lightProjection);

		VDGM::g_currentScene->RenderShadows(renderer, &lightMatrixBuffer);
	}
}