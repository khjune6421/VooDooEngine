#pragma once

#include "UtilityHeaders.h"
#include "DirectXLib.h"

#include "Component.h"

#include "Object.h"

class Light : public Component
{
protected:
	DirectX::XMFLOAT3 m_color = { 0.0f, 0.0f, 0.0f };
	float m_intensity = 1.0f;

public:
	Light(const DirectX::XMFLOAT3& color = { 0.0f, 0.0f, 0.0f }, const float intensity = 1.0f)
		: m_color(color), m_intensity(intensity) {}

	virtual DirectX::XMFLOAT3 GetColor() const { return m_color; }
	virtual void SetColor(const DirectX::XMFLOAT3& color) { m_color = color; }

	virtual float GetIntensity() const { return m_intensity; }
	virtual void SetIntensity(float intensity) { m_intensity = intensity; }
};

class AmbientLight : public Light
{
	void AddColor() const;
	void RemoveColor() const;

	friend class Render;
	static DirectX::XMFLOAT4 s_ambientColor;

public:
	AmbientLight
	(
		const DirectX::XMFLOAT3& color = { 0.0f, 0.0f, 0.0f },
		const float intensity = 1.0f
	) : Light(color, intensity) {}

	void OnAttached(class Object* owner) override { Component::OnAttached(owner); AddColor(); }
	void OnDetached() override { Component::OnDetached(); RemoveColor(); }

	void SetColor(const DirectX::XMFLOAT3& color) override { RemoveColor(); m_color = color; AddColor(); }
	void SetIntensity(float intensity) override { RemoveColor(); m_intensity = intensity; AddColor(); };

	static const DirectX::XMFLOAT4& GetAmbientColor() { return s_ambientColor; }
};

struct DirectionalLightConstBuffer
{
	DirectX::XMVECTOR direction = { 0.0f, -1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
};
class DirectionalLight : public Light
{
	DirectX::XMVECTOR m_direction = { 0.0f, -1.0f, 0.0f, 0.0f };

	friend class Render;
	static DirectionalLightConstBuffer s_lightData;

	void UpdateColor() { s_lightData.color = DirectX::XMFLOAT4{ m_color.x * m_intensity, m_color.y * m_intensity, m_color.z * m_intensity, 0.0f }; }

public:
	DirectionalLight
	(
		const DirectX::XMFLOAT3& color = { 0.0f, 0.0f, 0.0f },
		const float intensity = 1.0f,
		const DirectX::XMVECTOR& direction = { 1.0f, -1.0f, 0.0f, 0.0f }
	) : Light(color, intensity) { m_direction = DirectX::XMVector3Normalize(direction); }

	void OnAttached(class Object* owner) override { Component::OnAttached(owner); UpdateColor(); s_lightData.direction = m_direction; }
	void OnDetached() override { s_lightData = {}; }

	void SetColor(const DirectX::XMFLOAT3& color) override { m_color = color; UpdateColor(); }
	void SetIntensity(float intensity) override { m_intensity = intensity; UpdateColor(); }

	DirectX::XMVECTOR GetDirection() const { return m_direction; }
	void SetDirection(const DirectX::XMVECTOR& direction) { m_direction = DirectX::XMVector3Normalize(direction); s_lightData.direction = m_direction; }
};

constexpr int MAX_POINT_LIGHTS = 2;

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
struct PointLightArrayConstBuffer
{
	PointLightConstBuffer pointLights[MAX_POINT_LIGHTS] = {};
};
class PointLight;
extern std::vector<PointLight*> g_pointLights;
class PointLight : public Light
{
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
	) : Light(color, intensity)
	{
		UpdateColor();
		m_lightData.directionAndAngle = DirectX::XMFLOAT4{ 0.0f, 0.0f, 1.0f, spotAngle };
		m_lightData.range = range;
		m_lightData.constant = constantAttenuation;
		m_lightData.linear = linearAttenuation;
		m_lightData.quadratic = quadraticAttenuation;
	}

	void OnAttached(class Object* owner) override { Component::OnAttached(owner); g_pointLights.push_back(this); }
	void OnDetached() override;

	void SetColor(const DirectX::XMFLOAT3& color) override { m_color = color; UpdateColor(); }
	void SetIntensity(float intensity) override { m_intensity = intensity; UpdateColor(); }

	float GetRange() const { return m_lightData.range; }
	void SetRange(float range) { m_lightData.range = range; }

	float GetConstantAttenuation() const { return m_lightData.constant; }
	float GetLinearAttenuation() const { return m_lightData.linear; }
	float GetQuadraticAttenuation() const { return m_lightData.quadratic; }
	void SetConstantAttenuation(float constant) { m_lightData.constant = constant; }
	void SetLinearAttenuation(float linear) { m_lightData.linear = linear; }
	void SetQuadraticAttenuation(float quadratic) { m_lightData.quadratic = quadratic; }

	PointLightConstBuffer& GetLightData();
};