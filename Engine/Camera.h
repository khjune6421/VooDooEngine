#pragma once
#include "pch.h"
#include "Component.h"

class Camera : public Component
{
	float m_fov;

	UINT m_screenWidth;
	UINT m_screenHeight;
	float m_aspectRatio;

	float m_nearPlane;
	float m_farPlane;

	DirectX::XMVECTOR m_cameraPosition = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_projectionMatrix;

public:
	Camera
	(
		UINT screenWidth = 1920, UINT screenHeight = 1080,
		float nearPlane = 0.1f, float farPlane = 100.0f,
		float fov = DirectX::XM_PIDIV4
	);

	void SetScreen(UINT screenWidth = 0, UINT screenHeight = 0, float nearPlane = -1.0f, float farPlane = -1.0f, float fov = -1.0f);

	float GetFarPlane() const { return m_farPlane; }

	DirectX::XMVECTOR GetPosition() const { return m_cameraPosition; }
	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }

	void OnAttached(Object* owner) override;
	void OnDetached() override;
};