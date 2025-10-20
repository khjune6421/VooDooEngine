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
	) : Light(color, intensity) { AddColor(); }

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
	friend class Render;
	static DirectionalLightConstBuffer s_lightData;

	void UpdateColor() { s_lightData.color = DirectX::XMFLOAT4{ m_color.x * m_intensity, m_color.y * m_intensity, m_color.z * m_intensity, 0.0f }; }

public:
	DirectionalLight
	(
		const DirectX::XMFLOAT3& color = { 0.0f, 0.0f, 0.0f },
		const float intensity = 1.0f,
		const DirectX::XMVECTOR& direction = { 1.0f, -1.0f, 0.0f, 0.0f }
	) : Light(color, intensity) { UpdateColor(); s_lightData.direction = DirectX::XMVector3Normalize(direction); }

	void SetColor(const DirectX::XMFLOAT3& color) override { m_color = color; UpdateColor(); }
	void SetIntensity(float intensity) override { m_intensity = intensity; UpdateColor(); }

	DirectX::XMVECTOR GetDirection() const { return s_lightData.direction; }
	void SetDirection(const DirectX::XMVECTOR& direction) { s_lightData.direction = DirectX::XMVector3Normalize(direction); }
};

constexpr float DEFAULT_CONSTANT_ATTENUATION = 1.0f;
constexpr float DEFAULT_LINEAR_ATTENUATION = 0.1f;
constexpr float DEFAULT_QUADRATIC_ATTENUATION = 0.05f;
struct Attenuation
{
	float constant = DEFAULT_CONSTANT_ATTENUATION;
	float linear = DEFAULT_LINEAR_ATTENUATION;
	float quadratic = DEFAULT_QUADRATIC_ATTENUATION;
};
constexpr int MAX_POINT_LIGHTS = 4;
struct PointLightConstBuffer
{
	DirectX::XMVECTOR position = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
	float range = 100.0f;
	Attenuation attenuation = {};
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
		const float range = 100.0f,
		const Attenuation & attenuation = { DEFAULT_CONSTANT_ATTENUATION, DEFAULT_LINEAR_ATTENUATION, DEFAULT_QUADRATIC_ATTENUATION }
	) : Light(color, intensity) { UpdateColor(); m_lightData.range = range; m_lightData.attenuation = attenuation; }

	void OnAttached(class Object* owner) override { Component::OnAttached(owner); g_pointLights.push_back(this); }

	void SetColor(const DirectX::XMFLOAT3& color) override { m_color = color; UpdateColor(); }
	void SetIntensity(float intensity) override { m_intensity = intensity; UpdateColor(); }

	float GetRange() const { return m_lightData.range; }
	void SetRange(float range) { m_lightData.range = range; }

	Attenuation GetAttenuation() const { return m_lightData.attenuation; }
	void SetAttenuation(const Attenuation& attenuation) { m_lightData.attenuation = attenuation; }

	PointLightConstBuffer& GetLightData() { m_lightData.position = m_owner->GetWorldPosition(); return m_lightData; }
};