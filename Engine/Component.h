#pragma once

class Object;

class Component
{
protected:
	Object* m_owner = nullptr;

public:
	~Component() { OnDetached(); }

	virtual void Update(float deltaTime) {}
	virtual void OnAttached(class Object* owner) { m_owner = owner; }
	virtual void OnDetached() { m_owner = nullptr; }
};