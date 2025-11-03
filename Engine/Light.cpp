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
		auto it = remove(g_pointLights.begin(), g_pointLights.end(), this);
		g_pointLights.erase(it, g_pointLights.end());
	}
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