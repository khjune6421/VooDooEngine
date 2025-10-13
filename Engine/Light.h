#pragma once

#include "Object.h"

class Light : public Object
{
private:
    DirectX::XMFLOAT4 m_ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	DirectX::XMFLOAT4 m_diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Lambertian light only for now

    float m_range = 100.0f;
    float m_intensity = 1.0f;
    float m_attenuation = 0.0f;

public:
    Light
    (
        DirectX::XMFLOAT4 ambient = { 0.2f, 0.2f, 0.2f, 1.0f },
        DirectX::XMFLOAT4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f },
        float range = 100.0f,
        float intensity = 1.0f,
        float attenuation = 0.0f
    );
    ~Light();

    DirectX::XMFLOAT4 GetAmbientColor() const { return m_ambientColor; }
    void SetAmbientColor(const DirectX::XMFLOAT4& color) { m_ambientColor = color; }

    DirectX::XMFLOAT4 GetDiffuseColor() const { return m_diffuseColor; }
    void SetDiffuseColor(const DirectX::XMFLOAT4& color) { m_diffuseColor = color; }

    float GetRange() const { return m_range; }
    void SetRange(float range) { m_range = range; }

    float GetIntensity() const { return m_intensity; }
    void SetIntensity(float intensity) { m_intensity = intensity; }

    float GetAttenuation() const { return m_attenuation; }
    void SetAttenuation(float attenuation) { m_attenuation = attenuation; }
};

extern std::vector<Light*> g_lights;