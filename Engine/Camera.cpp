#include "Camera.h"

using namespace DirectX;

Camera::Camera(XMFLOAT3 position, XMFLOAT3 rotation, int screenWidth, int screenHeight, float nearPlane, float farPlane, float fov)
	: m_position(position), m_rotation(rotation), m_screenWidth(screenWidth), m_screenHeight(screenHeight), m_nearPlane(nearPlane), m_farPlane(farPlane), m_fov(fov)
{
	m_aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

	m_eyePosition = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);

	XMVECTOR lookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	m_focusPosition = XMVectorAdd(m_eyePosition, lookAt);

	m_upDirection = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationMatrix);

	m_viewMatrix = XMMatrixLookAtLH(m_eyePosition, m_focusPosition, m_upDirection);
}

void Camera::SetPosition(const XMFLOAT3& position)
{
	m_position = position;

	m_eyePosition = XMVectorSet(m_position.x, m_position.y, m_position.z, 1.0f);

	m_viewMatrix = XMMatrixLookAtLH(m_eyePosition, m_focusPosition, m_upDirection);
}

void Camera::SetRotation(const XMFLOAT3& rotation)
{
	m_rotation = rotation;

	if (m_rotation.x > XM_2PI) m_rotation.x -= XM_2PI;
	else if (m_rotation.x < 0.0f) m_rotation.x += XM_2PI;
	if (m_rotation.y > XM_2PI) m_rotation.y -= XM_2PI;
	else if (m_rotation.y < 0.0f) m_rotation.y += XM_2PI;
	if (m_rotation.z > XM_2PI) m_rotation.z -= XM_2PI;
	else if (m_rotation.z < 0.0f) m_rotation.z += XM_2PI;

	XMVECTOR lookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	m_focusPosition = XMVectorAdd(m_eyePosition, lookAt);

	m_upDirection = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationMatrix);

	m_viewMatrix = XMMatrixLookAtLH(m_eyePosition, m_focusPosition, m_upDirection);
}

void Camera::LookAt(const XMVECTOR& target) // Need to properly learn how this works one day // ...one day
{
	m_focusPosition = target;
	XMVECTOR direction = XMVectorSubtract(m_focusPosition, m_eyePosition);
	direction = XMVector3Normalize(direction);

	m_rotation.x = asinf(-XMVectorGetY(direction)); // pitch
	m_rotation.y = atan2f(XMVectorGetX(direction), XMVectorGetZ(direction)); // yaw
	m_rotation.z = 0.0f; // roll
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	m_upDirection = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationMatrix);
	m_viewMatrix = XMMatrixLookAtLH(m_eyePosition, m_focusPosition, m_upDirection);
}

void Camera::SetScreenSize(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::SetNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::SetFarPlane(float farPlane)
{
	m_farPlane = farPlane;

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::SetFov(float fov)
{
	m_fov = fov;

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

//Camera::Camera(UINT screenWidth, UINT screenHeight, float nearPlane, float farPlane, float fov, DirectX::XMVECTOR focusPosition)
//	: m_screenWidth(screenWidth), m_screenHeight(screenHeight),
//	m_nearPlane(nearPlane), m_farPlane(farPlane),
//	m_fov(fov),
//	m_focusPosition(focusPosition),
//	m_aspectRatio(static_cast<float>(screenWidth) / static_cast<float>(screenHeight)),
//	m_projectionMatrix(XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane)),
//	m_viewMatrix(XMMatrixIdentity())
//{
//}