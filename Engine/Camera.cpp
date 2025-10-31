#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera* g_camera = nullptr;

Camera::Camera(float fov, UINT screenWidth, UINT screenHeight, float nearPlane, float farPlane)
	:
	m_fov(fov),
	m_screenWidth(screenWidth), m_screenHeight(screenHeight),
	m_nearPlane(nearPlane), m_farPlane(farPlane),
	m_aspectRatio(static_cast<float>(screenWidth) / static_cast<float>(screenHeight)),
	m_projectionMatrix(XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane))
{
	// How should I handle multiple cameras? // send an error message?
	g_camera = this;
}

void Camera::SetScreen(float fov, UINT screenWidth, UINT screenHeight, float nearPlane, float farPlane)
{
	if (fov > 0.0f) m_fov = fov;
	if (screenWidth != 0 && screenHeight != 0)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
		m_aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	}
	if (nearPlane > 0.0f) m_nearPlane = nearPlane;
	if (farPlane > 0.0f) m_farPlane = farPlane;

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

XMMATRIX Camera::GetViewMatrix()
{
	m_cameraPosition = m_owner->GetWorldPosition();
	const XMVECTOR forward = m_owner->GetWorldDirection(Directions::Forward);
	static const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_viewMatrix = XMMatrixLookAtLH(m_cameraPosition, XMVectorAdd(m_cameraPosition, forward), up);

	return m_viewMatrix;
}