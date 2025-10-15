#pragma once

#include "UtilityHeaders.h"
#include "DirectXLib.h"

#include "IComponent.h"

struct LightData
{
	DirectX::XMFLOAT4 ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	DirectX::XMFLOAT4 diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float range = 100.0f;
	float intensity = 1.0f;
	float attenuation = 0.0f;
};

extern std::vector<std::pair<Object*, LightData*>> g_lightDatas;

class Light : public IComponent
{
	LightData m_lightData = {};

public:
	Light
	(
		DirectX::XMFLOAT4 ambient = { 0.2f, 0.2f, 0.2f, 1.0f },
		DirectX::XMFLOAT4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f },
		float range = 100.0f,
		float intensity = 1.0f,
		float attenuation = 0.0f
	) : m_lightData{ ambient, diffuse, range, intensity, attenuation } {}
	~Light() { OnDetached(); }

	void OnAttached(class Object* owner) override { IComponent::OnAttached(owner); g_lightDatas.emplace_back(owner, &m_lightData); }
	void OnDetached() override;

	DirectX::XMFLOAT4 GetAmbientColor() const { return m_lightData.ambientColor; }
	void SetAmbientColor(const DirectX::XMFLOAT4& color) { m_lightData.ambientColor = color; }

	DirectX::XMFLOAT4 GetDiffuseColor() const { return m_lightData.diffuseColor; }
	void SetDiffuseColor(const DirectX::XMFLOAT4& color) { m_lightData.diffuseColor = color; }

	float GetRange() const { return m_lightData.range; }
	void SetRange(float range) { m_lightData.range = range; }

	float GetIntensity() const { return m_lightData.intensity; }
	void SetIntensity(float intensity) { m_lightData.intensity = intensity; }

	float GetAttenuation() const { return m_lightData.attenuation; }
	void SetAttenuation(float attenuation) { m_lightData.attenuation = attenuation; }
};