#include "Light.h"

Light::Light(DirectX::XMFLOAT4 ambient, DirectX::XMFLOAT4 diffuse, float range, float intensity, float attenuation)
	: Object(),
	  m_ambientColor(ambient),
	  m_diffuseColor(diffuse),
	  m_range(range),
	  m_intensity(intensity),
	  m_attenuation(attenuation)
{ g_lights.push_back(this); }

Light::~Light()
{
	auto it = find(g_lights.begin(), g_lights.end(), this);
	if (it != g_lights.end()) g_lights.erase(it);
}