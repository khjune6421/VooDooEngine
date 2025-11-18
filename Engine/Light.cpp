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
	const XMVECTOR dir = m_owner->GetWorldDirection(Directions::Forward);
	
	m_lightData.directionAndAngle.x = XMVectorGetX(dir);
	m_lightData.directionAndAngle.y = XMVectorGetY(dir);
	m_lightData.directionAndAngle.z = XMVectorGetZ(dir);

	return m_lightData;
}

void PointLight::CreateShadowMap(Renderer* renderer, UINT index) const
{
	const XMVECTOR lightPos = m_lightData.position;
	const float lightRange = m_lightData.range;

	const XMMATRIX lightProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, lightRange);

	const XMFLOAT4 lightData = XMFLOAT4(XMVectorGetX(lightPos), XMVectorGetY(lightPos), XMVectorGetZ(lightPos), lightRange);
	renderer->m_deviceContext->UpdateSubresource(renderer->m_constBuffers[Renderer::LightPosBuffer].Get(), 0, nullptr, &lightData, 0, 0);
	renderer->m_deviceContext->PSSetConstantBuffers(0, 1, renderer->m_constBuffers[Renderer::LightPosBuffer].GetAddressOf());

	for (UINT face = 0; face < 6; ++face)
	{
		renderer->m_deviceContext->OMSetRenderTargets(0, renderer->m_shadowMapArrayRTV.GetAddressOf(), renderer->m_cubeShadowMapDSVs[verbose_cast(com_ptr<ID3D11DepthStencilView>)(index) + face].Get());
		renderer->m_deviceContext->ClearDepthStencilView(renderer->m_cubeShadowMapDSVs[verbose_cast(com_ptr<ID3D11DepthStencilView>)(index) + face].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		const XMVECTOR target = XMVectorAdd(lightPos, CUBE_TARGET[face]);
		const XMMATRIX lightView = XMMatrixLookAtLH(lightPos, target, CUBE_TARGET_UP[face]);

		MatrixConstBuffer lightMatrixBuffer = {};
		lightMatrixBuffer.view = XMMatrixTranspose(lightView);
		lightMatrixBuffer.projection = XMMatrixTranspose(lightProjection);

		VDGM::g_currentScene->RenderShadows(renderer, &lightMatrixBuffer); // TODO: this is cursed // needs fixing
	}
}