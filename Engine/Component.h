#pragma once

class Object;

class Component
{
public:
	Component() = default;
	~Component() { OnDetached(); }
	Component(const Component& other) = default;
	Component& operator=(const Component& other) = default;
	Component(Component&& other) noexcept = default;
	Component& operator=(Component&& other) noexcept = default;

	Object* m_owner = nullptr;

	virtual void Update(float deltaTime) {}
	virtual void OnAttached(class Object* owner) { m_owner = owner; }
	virtual void OnDetached() { m_owner = nullptr; }
};