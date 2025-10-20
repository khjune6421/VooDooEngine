#include "Light.h"

using namespace std;
using namespace DirectX;

XMFLOAT4 AmbientLight::s_ambientColor = { 0.0f, 0.0f, 0.0f, 1.0f };
DirectionalLightConstBuffer DirectionalLight::s_lightData = {};

void AmbientLight::AddColor() const
{
	s_ambientColor.x += m_color.x * m_intensity;
	s_ambientColor.y += m_color.y * m_intensity;
	s_ambientColor.z += m_color.z * m_intensity;
}

void AmbientLight::RemoveColor() const
{
	s_ambientColor.x -= m_color.x * m_intensity;
	s_ambientColor.y -= m_color.y * m_intensity;
	s_ambientColor.z -= m_color.z * m_intensity;
}

vector<PointLight*> g_pointLights;