#pragma once

#include "Object.h"
#include "Camera.h"

struct DirectionalLightConstBuffer
{
	DirectX::XMVECTOR direction = { 0.0f, -1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
};

class Collider;

class Scene
{
	friend class Shape;
	std::vector<Shape*> m_renderShapes;

	friend class Collider;
	std::unordered_map<UINT, std::pair<std::vector<Collider*>, std::vector<Collider*>>> m_collidersMap;
	void CheckCollisions();

	void UpdateCamera();

protected:
	std::vector<std::unique_ptr<Object>> m_objects;

public:
	Scene() = default;

	Camera* m_mainCamera = nullptr;
	DirectX::XMVECTOR m_mainCameraPosition = { 0.0f, 0.0f, 0.0f, 1.0f };

	MatrixConstBuffer m_matrixConstBuffer = {};

	DirectX::XMFLOAT4 m_backgroundColor = { 0.5f, 0.5f, 0.5f, 1.0f };
	DirectX::XMFLOAT4 m_ambientLight = { 0.25f, 0.25f, 0.25f, 1.0f }; // The w value(1) is important // it's the only value that does not get multiplied in shader
	DirectionalLightConstBuffer m_directionalLight = {};

	DirectX::XMFLOAT4 m_ambientFog = { 0.5f, 0.5f, 0.5f, 100.0f }; // w value is range

	virtual void Update(float deltaTime);
	virtual void Render(Renderer* renderer);

	virtual void Raycast(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayEnd) {}
};