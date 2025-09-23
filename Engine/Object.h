#pragma once

#include "Render.h"

enum class Directions
{
	Forward,
	Backward,
	Right,
	Left,
	Up,
	Down
};

class Object
{
	// is this a good idea?
	friend class Render;

	UINT m_id = 0; // For debug purpose

	Shapes m_shape = Shapes::Triangle;

	// Not sure if these should be private or protected
	DirectX::XMMATRIX m_position = DirectX::XMMatrixIdentity();
	DirectX::XMVECTOR m_pitchYawRoll = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
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

	enum IgnoreParentAxis
	{
		None = 0,
		X = 2,
		Y = 3,
		Z = 5
	};

	UINT m_ignorePosition = IgnoreParentAxis::None;
	UINT m_ignoreRotation = IgnoreParentAxis::None;
	UINT m_ignoreScale = IgnoreParentAxis::None;

public:
	Object(Shapes shape = Shapes::Triangle);
	virtual ~Object();

	void SetPosition(const DirectX::XMVECTOR& pos);
	void MovePosition(const DirectX::XMVECTOR& delta);
	void MoveDirection(Directions dir, float distance);

	DirectX::XMVECTOR GetPosition() const;

	void SetRotation(const DirectX::XMVECTOR& rot);
	void Rotate(const DirectX::XMVECTOR& delta);
	DirectX::XMVECTOR GetRotation() const;

	void SetScale(const DirectX::XMVECTOR& scl);
	void Scale(const DirectX::XMVECTOR& factor);
	DirectX::XMVECTOR GetScale() const;

	DirectX::XMMATRIX GetWorldMatrix();

	// Other basic object function
	virtual void Update(float deltaTime) { (void)deltaTime; } // (void) to avoid unused parameter warning // feels odd but makes sense
};