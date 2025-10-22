#pragma once

#include "DirectXLib.h"
#include "UtilityHeaders.h"

#include "Component.h"


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
	friend class Render;
	UINT m_id = 0; // For debug purpose

	// Not sure if these should be private or protected
	DirectX::XMMATRIX m_positionMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_rotationMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_scaleMatrix = DirectX::XMMatrixIdentity();

	mutable DirectX::XMMATRIX m_worldMatrix = DirectX::XMMatrixIdentity();
	mutable DirectX::XMMATRIX m_inverseScaleMatrix = DirectX::XMMatrixIdentity(); // For normal

	DirectX::XMVECTOR QuaternionToEuler(const DirectX::XMVECTOR& quat) const;

	mutable bool m_isDirty = true;
	void SetDirty(); // Recursive

	// Component system
	std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;

protected:
	DirectX::XMVECTOR m_position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR m_rotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };

	bool m_isActive = true;

	Object* m_parent = nullptr;
	std::vector<Object*> m_childrens;

public:
	Object() = default;
	~Object();
	Object(const Object& other) = default;
	Object& operator=(const Object& other) = default;
	Object(Object&& other) noexcept = default;
	Object& operator=(Object&& other) noexcept = default;

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

	void AddChild(Object* child);
	void AddChildViaWorldPosition(Object* child);
	void RemoveChild(Object* child);

	virtual void Update(float deltaTime) {}

	// Component system
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T* componentPtr = component.get();

		m_components[std::type_index(typeid(T))] = std::move(component);
		componentPtr->OnAttached(this);

		return componentPtr;
	}
	template<typename T>
	T* GetComponent() const
	{
		static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

		auto it = m_components.find(std::type_index(typeid(T)));
		if (it != m_components.end()) return static_cast<T*>(it->second.get());

		return nullptr;
	}
	template<typename T>
	bool RemoveComponent()
	{
		static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

		auto it = m_components.find(std::type_index(typeid(T)));
		if (it != m_components.end())
		{
			it->second->OnDetached();
			m_components.erase(it);

			return true;
		}

#ifdef _DEBUG
		MessageBoxA(nullptr, "Component to remove not found", "Error", MB_OK | MB_ICONERROR);
#endif
		return false;
	}
};