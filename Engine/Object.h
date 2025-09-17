#pragma once

#include "Render.h"

class Object
{
	// is this a good idea?
	friend class Render;

	// Not sure if these should be private or protected
	DirectX::XMMATRIX m_position = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_rotation = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_scale = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();

	bool m_isDirty = true;

	Shapes m_shape = Shapes::Triangle;

protected:
	bool m_isActive = true;

	Object* m_parent = nullptr;
	std::vector<std::unique_ptr<Object>> m_childrens;

public:
	Object(Shapes shape = Shapes::Triangle);
	virtual ~Object();

	// Using simple math for easier vector operations // for now
	void SetPosition(const DirectX::SimpleMath::Vector3& pos);
	void MovePosition(const DirectX::SimpleMath::Vector3& delta);
	DirectX::SimpleMath::Vector3 GetPosition() const;

	void SetRotation(const DirectX::SimpleMath::Vector3& rot); // in radians
	void Rotate(const DirectX::SimpleMath::Vector3& delta); // in radians
	DirectX::SimpleMath::Vector3 GetRotation() const; // in radians

	void SetScale(const DirectX::SimpleMath::Vector3& scl);
	void Scale(const DirectX::SimpleMath::Vector3& factor);
	DirectX::SimpleMath::Vector3 GetScale() const;

	DirectX::XMMATRIX GetWorldMatrix();

	void SetParent(Object* parent) { m_parent = parent; }

	virtual void Update(float deltaTime) { (void)deltaTime; } // (void) to avoid unused parameter warning // feels odd but makes sense
};