#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

// Rendering related enums
enum class Shapes // This also includes number of vertices for each shape and type of primitiveTopology?
{
	// Defalt // creating the shape with this shape will not add it to g_objects
	None,

	// 2D shapes
	Triangle,
	Square,
	Plane,

	Tetrahedron,
	Cube,
	Icosphere,

	Tree,
	WindmillWing
};
enum class VertexShaders // Should it also includes corresponding constant buffer and input layout?
{
	Default,
};
enum class PixelShaders
{
	Default,
	ColorShift,
	Greyscale
};

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
	friend class Camera;

	UINT m_id = 0; // For debug purpose

	Shapes m_shape = Shapes::None;
	VertexShaders m_vertexShader = VertexShaders::Default;
	PixelShaders m_pixelShader = PixelShaders::Default;

	// Not sure if these should be private or protected
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity();

	DirectX::XMVECTOR QuaternionToEuler(const DirectX::XMVECTOR& quat) const;

	// mutable so that it can be modified in const function GetWorldMatrix
	mutable DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();
	mutable bool m_isDirty = true;

	void SetDirty();

protected:
	DirectX::XMVECTOR m_position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR m_rotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };

	bool m_isActive = true;

	Object* m_parent = nullptr;
	std::vector<Object*> m_childrens;

	enum IgnoreParentAxis // Later change to bit field
	{
		None = 0,
		X = 1 << 1, // 2
		Y = 1 << 2, // 4
		Z = 1 << 3  // 8
	};

	UINT m_ignorePosition = IgnoreParentAxis::None;
	UINT m_ignoreRotation = IgnoreParentAxis::None;
	UINT m_ignoreScale = IgnoreParentAxis::None;

public:
	Object(Shapes shape = Shapes::None, VertexShaders vertexShader = VertexShaders::Default, PixelShaders pixelShader = PixelShaders::Default);
	virtual ~Object();

	void AddChild(Object* child);
	void AddChildViaWorldPosition(Object* child);
	void RemoveChild(Object* child);

	void SetPosition(const DirectX::XMVECTOR& pos);
	void MovePosition(const DirectX::XMVECTOR& delta);
	void MoveDirection(Directions dir, float distance);
	DirectX::XMVECTOR GetPosition() const { return m_position; }
	DirectX::XMVECTOR GetWorldPosition() const;

	void SetRotation(const DirectX::XMVECTOR& rot);
	void Rotate(const DirectX::XMVECTOR& delta);
	void LookAt(const DirectX::XMVECTOR& target);
	DirectX::XMVECTOR GetRotation() const { return m_rotation; }
	DirectX::XMVECTOR GetWorldRotation() const;
	DirectX::XMVECTOR GetWorldDirection(Directions dir) const;

	void SetScale(const DirectX::XMFLOAT3& scl);
	void SetScale(const DirectX::XMVECTOR& scl) { DirectX::XMFLOAT3 f3 = {}; DirectX::XMStoreFloat3(&f3, scl); SetScale(f3); }
	void Scale(const DirectX::XMFLOAT3& factor);
	void Scale(const DirectX::XMVECTOR& factor) { DirectX::XMFLOAT3 f3 = {}; DirectX::XMStoreFloat3(&f3, factor); Scale(f3); }
	DirectX::XMFLOAT3 GetScale() const { return m_scale; }
	DirectX::XMFLOAT3 GetWorldScale() const;

	DirectX::XMMATRIX GetWorldMatrix() const;

	// Other basic object function
	virtual void Update(float deltaTime) { (void)deltaTime; } // (void) to avoid unused parameter warning // feels odd but makes sense
};

extern std::vector<Object*> g_objects;