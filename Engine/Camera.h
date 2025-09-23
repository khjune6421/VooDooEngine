#pragma once

#include <DirectXMath.h>

class Camera
{
	DirectX::XMFLOAT3 m_position = { 0.0f, 20.f, 40.0f };
	DirectX::XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };

	DirectX::XMVECTOR m_eyePosition;
	DirectX::XMVECTOR m_focusPosition;
	DirectX::XMVECTOR m_upDirection;

	int m_screenWidth;
	int m_screenHeight;
	float m_aspectRatio;

	float m_nearPlane;
	float m_farPlane;
	float m_fov;

	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMMATRIX m_viewMatrix;

public:
	Camera(DirectX::XMFLOAT3 position = { 0.0f, 20.f, 40.0f }, DirectX::XMFLOAT3 rotation = { 0.5f, 0.0f, 0.0f }, int screenWidth = 1920, int screenHeight = 1080, float nearPlane = 0.1f, float farPlane = 1000.0f, float fov = DirectX::XM_PIDIV4);
	~Camera() = default;

	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetRotation(const DirectX::XMFLOAT3& rotation);
	const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
	const DirectX::XMFLOAT3& GetRotation() const { return m_rotation; }

	void LookAt(const DirectX::XMVECTOR& target);

	void SetScreenSize(int screenWidth, int screenHeight);
	int GetScreenWidth() const { return m_screenWidth; }
	int GetScreenHeight() const { return m_screenHeight; }

	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	float GetNearPlane() const { return m_nearPlane; }
	float GetFarPlane() const { return m_farPlane; }

	void SetFov(float fov);
	float GetFov() const { return m_fov; }

	float GetAspectRatio() const { return m_aspectRatio; }

	const DirectX::XMMATRIX& GetProjectionMatrix() const { return m_projectionMatrix; }
	const DirectX::XMMATRIX& GetViewMatrix() const { return m_viewMatrix; }
};