#pragma once

#include "Engine.h"

class Tree : public Object
{
	std::unique_ptr<Object> m_leaves = nullptr;

public:
	Tree() : Object()
	{
		AddComponent<Shape>(L"TreeTrunk", L"VertexShader", L"PixelShader", std::vector<std::wstring>{ L"Bark", L"BarkNormal" });
		m_leaves = std::make_unique<Object>();
		m_leaves->AddComponent<Shape>(L"Leaves", L"VertexShader", L"PixelShader", std::vector<std::wstring>{ L"Leaves", L"LeavesNormal" });
		AddChild(m_leaves.get());
	}
	void Update(float deltaTime) override;
};