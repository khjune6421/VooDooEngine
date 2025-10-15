#pragma once

class Object;

class IComponent
{
protected:
	Object* m_owner = nullptr;

public:
	virtual ~IComponent() = default;

	virtual void Update(float deltaTime) {}
	virtual void OnAttached(class Object* owner) { m_owner = owner; }
	virtual void OnDetached() { m_owner = nullptr; }
};