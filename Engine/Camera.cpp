#include "pch.h"
#include "Camera.h"

#include "Object.h"
#include "Scene.h"

using namespace DirectX;

Camera::Camera(UINT screenWidth, UINT screenHeight, float nearPlane, float farPlane, float fov)
	:
	m_screenWidth(screenWidth), m_screenHeight(screenHeight),
	m_nearPlane(nearPlane), m_farPlane(farPlane),
	m_fov(fov),
	m_aspectRatio(static_cast<float>(screenWidth) / static_cast<float>(screenHeight)),
	m_projectionMatrix(XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane)) {}

void Camera::SetScreen(UINT screenWidth, UINT screenHeight, float nearPlane, float farPlane, float fov)
{
	if (screenWidth != 0 && screenHeight != 0)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
		m_aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	}
	if (nearPlane > 0.0f) m_nearPlane = nearPlane;
	if (farPlane > 0.0f) m_farPlane = farPlane;
	if (fov > 0.0f) m_fov = fov;

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

void Camera::OnAttached(Object* owner)
{
	Component::OnAttached(owner);

	owner->m_scene->m_mainCamera = this;
}

void Camera::OnDetached()
{
	m_owner->m_scene->m_mainCamera = nullptr;

	Component::OnDetached();
}