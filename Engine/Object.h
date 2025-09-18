#pragma once

#include "Render.h"

class Object
{
	// is this a good idea?
	friend class Render;

	Shapes m_shape = Shapes::Triangle;

	// Not sure if these should be private or protected
	DirectX::XMMATRIX m_position = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_rotation = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_scale = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();

	bool m_isDirty = true;
	void MakeChildDirty();

protected:
	bool m_isActive = true;

	Object* m_parent = nullptr;
	std::vector<Object*> m_childrens;

	void AddChild(Object* child);

public:
	Object(Shapes shape = Shapes::Triangle);
	virtual ~Object();

	// Transformation related functions
	void SetPosition(const DirectX::XMFLOAT3& pos);
	void MovePosition(const DirectX::XMFLOAT3& delta);
	DirectX::XMFLOAT3 GetPosition() const;

	void SetRotation(const DirectX::XMFLOAT3& rot);
	void Rotate(const DirectX::XMFLOAT3& delta);
	DirectX::XMFLOAT3 GetRotation() const;

	void SetScale(const DirectX::XMFLOAT3& scl);
	void Scale(const DirectX::XMFLOAT3& factor);
	DirectX::XMFLOAT3 GetScale() const;

	DirectX::XMMATRIX GetWorldMatrix();

	// Other basic object function
	virtual void Update(float deltaTime) { (void)deltaTime; } // (void) to avoid unused parameter warning // feels odd but makes sense
};