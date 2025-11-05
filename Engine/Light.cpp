#include "pch.h"
#include "Light.h"

#include "Object.h"
#include "Scene.h"

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

XMVECTOR PointLight::GetWorldPosition()
{
	m_lightData.position = m_owner->GetWorldPosition();
	return m_lightData.position;
}