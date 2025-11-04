#include "pch.h"
#include "Light.h"

#include "Object.h"

using namespace std;
using namespace DirectX;

vector<PointLight*> g_pointLights;

void PointLight::OnDetached()
{
	if (!g_pointLights.empty())
	{
		auto it = find(g_pointLights.begin(), g_pointLights.end(), this);
		if (it != g_pointLights.end()) g_pointLights.erase(it);
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

XMMATRIX PointLight::GetViewMatrix()
{
	m_lightData.position = m_owner->GetWorldPosition();
	const XMVECTOR forward = m_owner->GetWorldDirection(Directions::Forward);
	static const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	return XMMatrixLookAtLH(m_lightData.position, XMVectorAdd(m_lightData.position, forward), up);
}

XMMATRIX PointLight::GetProjectionMatrix()
{
}