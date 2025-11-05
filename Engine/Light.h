#pragma once
#include "pch.h"
#include "Component.h"

constexpr float DEFAULT_CONSTANT_ATTENUATION = 1.0f;
constexpr float DEFAULT_LINEAR_ATTENUATION = 0.01f;
constexpr float DEFAULT_QUADRATIC_ATTENUATION = 0.005f;
struct PointLightConstBuffer // Is also a SpotLightConstBuffer
{
	DirectX::XMVECTOR position = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f };

	// For SpotLight
	DirectX::XMFLOAT4 directionAndAngle = { 0.0f, 0.0f, 1.0f, 0.0f }; // w is the angle // it's not radian or digree // Im not sure what it is // set to 0 for PointLight

	float range = 100.0f;

	float constant = DEFAULT_CONSTANT_ATTENUATION;
	float linear = DEFAULT_LINEAR_ATTENUATION;
	float quadratic = DEFAULT_QUADRATIC_ATTENUATION;
};
constexpr int MAX_POINT_LIGHTS = 8;
struct PointLightArrayConstBuffer
{
	PointLightConstBuffer pointLights[MAX_POINT_LIGHTS] = {};

	UINT numPointLights = 0;
	UINT padding[3] = {};
};
class PointLight;
extern std::vector<PointLight*> g_pointLights;
class PointLight : public Component
{
	DirectX::XMFLOAT3 m_color = { 0.0f, 0.0f, 0.0f };
	float m_intensity = 1.0f;
	PointLightConstBuffer m_lightData = {};

	void UpdateColor() { m_lightData.color = DirectX::XMFLOAT4{ m_color.x * m_intensity, m_color.y * m_intensity, m_color.z * m_intensity, 0.0f }; }

public:
	PointLight
	(
		const DirectX::XMFLOAT3& color = { 0.0f, 0.0f, 0.0f },
		const float intensity = 1.0f,
		const float spotAngle = 0.0f,
		const float range = 100.0f,
		const float constantAttenuation = DEFAULT_CONSTANT_ATTENUATION,
		const float linearAttenuation = DEFAULT_LINEAR_ATTENUATION,
		const float quadraticAttenuation = DEFAULT_QUADRATIC_ATTENUATION
	)
	{
		m_color = color;
		m_intensity = intensity;
		UpdateColor();
		m_lightData.directionAndAngle = DirectX::XMFLOAT4{ 0.0f, 0.0f, 1.0f, spotAngle };
		m_lightData.range = range;
		m_lightData.constant = constantAttenuation;
		m_lightData.linear = linearAttenuation;
		m_lightData.quadratic = quadraticAttenuation;
	}

	void OnAttached(Object* owner) override { Component::OnAttached(owner); g_pointLights.push_back(this); }
	void OnDetached() override;

	DirectX::XMFLOAT3 GetColor() const { return m_color; }
	void SetColor(const DirectX::XMFLOAT3& color) { m_color = color; UpdateColor(); }
	float GetIntensity() const { return m_intensity; }
	void SetIntensity(float intensity) { m_intensity = intensity; UpdateColor(); }

	float GetRange() const { return m_lightData.range; }
	void SetRange(float range) { m_lightData.range = range; }

	float GetConstantAttenuation() const { return m_lightData.constant; }
	float GetLinearAttenuation() const { return m_lightData.linear; }
	float GetQuadraticAttenuation() const { return m_lightData.quadratic; }
	void SetConstantAttenuation(float constant) { m_lightData.constant = constant; }
	void SetLinearAttenuation(float linear) { m_lightData.linear = linear; }
	void SetQuadraticAttenuation(float quadratic) { m_lightData.quadratic = quadratic; }

	PointLightConstBuffer& GetLightData();
	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectionMatrix();
};