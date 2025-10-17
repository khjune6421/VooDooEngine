#pragma once

#include "UtilityHeaders.h"
#include "DirectXLib.h"

#include "Component.h"

#include "Object.h"

class AmbientLight : public Component
{
	static DirectX::XMFLOAT4 s_ambientColor;
	DirectX::XMFLOAT4 m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

public:
	AmbientLight(const DirectX::XMFLOAT4& color = { 0.25f, 0.25f, 0.25f, 1.0f }) : m_color(color) {}

	void OnAttached(class Object* owner) override
	{
		Component::OnAttached(owner);
		s_ambientColor.x += m_color.x * m_color.w;
		s_ambientColor.y += m_color.y * m_color.w;
		s_ambientColor.z += m_color.z * m_color.w;
	}
	void OnDetached() override
	{
		Component::OnDetached();
		s_ambientColor.x -= m_color.x * m_color.w;
		s_ambientColor.y -= m_color.y * m_color.w;
		s_ambientColor.z -= m_color.z * m_color.w;
	}


	DirectX::XMFLOAT4 GetColor() const { return m_color; }
	void SetColor(const DirectX::XMFLOAT4& color)
	{ 
		s_ambientColor.x -= m_color.x * m_color.w;
		s_ambientColor.y -= m_color.y * m_color.w;
		s_ambientColor.z -= m_color.z * m_color.w;

		m_color = color; 
		s_ambientColor.x += m_color.x * m_color.w;
		s_ambientColor.y += m_color.y * m_color.w;
		s_ambientColor.z += m_color.z * m_color.w;
	}

	static const DirectX::XMFLOAT4& GetAmbientColor() { return s_ambientColor; }
};

constexpr float DEFAULT_CONSTANT_ATTENUATION = 1.0f;
constexpr float DEFAULT_LINEAR_ATTENUATION = 0.09f;
constexpr float DEFAULT_QUADRATIC_ATTENUATION = 0.032f;
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
	DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	float range = 100.0f;
	Attenuation attenuation = {};
};
class PointLight;
extern std::vector<PointLight*> g_pointLights;
class PointLight : public Component
{
	PointLightConstBuffer m_lightData = {};

public:
	PointLight
	(
		const DirectX::XMVECTOR& position = { 0.0f, 0.0f, 0.0f, 1.0f },
		const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 5.0f },
		float range = 100.0f,
		const Attenuation& attenuation = { DEFAULT_CONSTANT_ATTENUATION, DEFAULT_LINEAR_ATTENUATION, DEFAULT_QUADRATIC_ATTENUATION }
	) : m_lightData{ position, color, range, attenuation } {}

	void OnAttached(class Object* owner) override { Component::OnAttached(owner); g_pointLights.push_back(this); }

	DirectX::XMFLOAT4 GetColor() const { return m_lightData.color; }
	void SetColor(const DirectX::XMFLOAT4& color) { m_lightData.color = color; }

	float GetRange() const { return m_lightData.range; }
	void SetRange(float range) { m_lightData.range = range; }

	Attenuation GetAttenuation() const { return m_lightData.attenuation; }
	void SetAttenuation(const Attenuation& attenuation) { m_lightData.attenuation = attenuation; }

	PointLightConstBuffer& GetLightData()
	{
		m_lightData.position = m_owner->GetWorldPosition();
		return m_lightData;
	}
};