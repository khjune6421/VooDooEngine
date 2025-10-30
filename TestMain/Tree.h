#pragma once

#include "Engine.h"

class Tree : public Object
{
	std::unique_ptr<Object> m_leaves = nullptr;

	Collider* m_collider = nullptr;

	bool m_isDead = false;
	DirectX::XMVECTOR m_fallDirection = DirectX::XMVectorZero();

public:
	Tree(Scene* scnene) : Object(scnene)
	{
		AddComponent<Shape>(L"TreeTrunk", L"VertexShader", L"PixelShader", std::vector<std::wstring>{ L"Bark", L"BarkNormal" });
		AddComponent<Collider>(1.0f, std::vector<UINT>{}, std::vector<UINT>{ 1 });
		m_collider = GetComponent<Collider>();
		m_leaves = std::make_unique<Object>(scnene);
		m_leaves->AddComponent<Shape>(L"Leaves", L"VertexShader", L"PixelShader", std::vector<std::wstring>{ L"Leaves", L"LeavesNormal" });
		AddChild(m_leaves.get());
	}
	void Update(float deltaTime) override;
};