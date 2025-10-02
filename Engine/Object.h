#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

// Converts string to uint for mapped shader, shape, etc
extern std::unordered_map<std::wstring, UINT> g_vertexShaderIdMap;
extern std::unordered_map<std::wstring, UINT> g_shapeIdMap;

// Rendering related enums
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

	std::vector<UINT> m_shapeIds;
	UINT m_vertexShaderId = 0;
	//VertexShaders m_vertexShader = VertexShaders::Default;
	PixelShaders m_pixelShader = PixelShaders::Default;

	// TODO: Remove this later
	virtual void SetConstBufferVar(_Out_ float* var1, _Out_ float* var2, _Out_ float* var3, _Out_ float* var4) const; // For setting constant buffer variables

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

	enum IgnoreParentAxis // TODO: later change this to bit field
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
	Object
	(
		const std::vector<std::wstring>& shapeNames = std::vector<std::wstring>{ L"None" },
		const std::wstring& vertexShader = L"VertexShader",
		PixelShaders pixelShader = PixelShaders::Default
	);
	Object
	(
		const std::wstring& shapeName,
		const std::wstring& vertexShader = L"VertexShader",
		PixelShaders pixelShader = PixelShaders::Default
	) : Object(std::vector<std::wstring>{ shapeName }, vertexShader, pixelShader) {}
	virtual ~Object();

	void AddChild(Object* child);
	void AddChildViaWorldPosition(Object* child);
	void RemoveChild(Object* child);

	void SetPosition(const DirectX::XMVECTOR& pos);
	void MovePosition(const DirectX::XMVECTOR& delta);
	void MoveDirection(Directions dir, float distance);
	void LerpPosition(const DirectX::XMVECTOR& start, const DirectX::XMVECTOR& target, float t); // t: 0.0f ~ 1.0f
	DirectX::XMVECTOR GetPosition() const { return m_position; }
	DirectX::XMVECTOR GetWorldPosition() const;

	void SetRotation(const DirectX::XMVECTOR& rot);
	void Rotate(const DirectX::XMVECTOR& delta);
	void LookAt(const DirectX::XMVECTOR& target);
	void LerpRotation(const DirectX::XMVECTOR& start, const DirectX::XMVECTOR& target, float t);
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

	virtual void Update(float deltaTime) { (void)deltaTime; } // (void) to avoid unused parameter warning // feels odd but makes sense
};