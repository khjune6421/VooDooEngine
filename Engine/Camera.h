#pragma once

#include "DirectXLib.h"

#include "Component.h"
#include "Object.h"

extern class Camera* g_camera;

class Camera : public Component
{
	float m_fov;

	UINT m_screenWidth;
	UINT m_screenHeight;
	float m_aspectRatio;

	float m_nearPlane;
	float m_farPlane;

	DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_projectionMatrix;

public:
	Camera
	(
		float fov = DirectX::XM_PIDIV4,
		UINT screenWidth = 1920, UINT screenHeight = 1080,
		float nearPlane = 0.1f, float farPlane = 1000.0f
	);
	~Camera() { if (g_camera == this) g_camera = nullptr; }

	void SetScreen(float fov = -1.0f, UINT screenWidth = 0, UINT screenHeight = 0, float nearPlane = -1.0f, float farPlane = -1.0f);

	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectionMatrix() const { return m_projectionMatrix; }
};