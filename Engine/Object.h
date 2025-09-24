#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

// Rendering related enums
enum class Shapes
{
	Triangle,
	Square,
	Plane,

	Tetrahedron,
	Cube,

	Tree,
	WindmillWing
};
enum class VertexShaders // Should it alos includes corresponding constant buffer and input layout?
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

	UINT m_id = 0; // For debug purpose

	Shapes m_shape = Shapes::Triangle;
	VertexShaders m_vertexShader = VertexShaders::Default;
	PixelShaders m_pixelShader = PixelShaders::Default;

	// Not sure if these should be private or protected
	DirectX::XMVECTOR m_position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMMATRIX m_positionMatRix = DirectX::XMMatrixIdentity();
	DirectX::XMVECTOR m_rotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX m_rotationMatRix = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMMATRIX m_scaleMatRix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();

	bool m_isDirty = true;
	void MakeChildDirty();

protected:
	bool m_isActive = true;

	Object* m_parent = nullptr;
	std::vector<Object*> m_childrens;

	void AddChild(Object* child);

	enum IgnoreParentAxis // Later change to bit field
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
	Object(Shapes shape = Shapes::Triangle, VertexShaders vertexShader = VertexShaders::Default, PixelShaders pixelShader = PixelShaders::Default);
	virtual ~Object();

	void SetPosition(const DirectX::XMVECTOR& pos);
	void MovePosition(const DirectX::XMVECTOR& delta);
	void MoveDirection(Directions dir, float distance);
	DirectX::XMVECTOR GetPosition() const { return m_position; }

	void SetRotation(const DirectX::XMVECTOR& rot);
	void Rotate(const DirectX::XMVECTOR& delta);
	DirectX::XMVECTOR GetRotation() const { return m_rotation; }

	void SetScale(const DirectX::XMFLOAT3& scl);
	void Scale(const DirectX::XMFLOAT3& factor);
	DirectX::XMFLOAT3 GetScale() const { return m_scale; }

	DirectX::XMMATRIX GetWorldMatrix();

	// Other basic object function
	virtual void Update(float deltaTime) { (void)deltaTime; } // (void) to avoid unused parameter warning // feels odd but makes sense
};

extern std::vector<Object*> g_objects;