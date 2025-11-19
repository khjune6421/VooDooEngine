#pragma once
#include "pch.h"

class Node : public Object
{
public:
	bool m_isWalkable = true;
	bool m_isVisited = false;
	int m_pathCost = 0;

	Node(Scene* scene) : Object(scene) { AddComponent<Shape>(L"Shape_Cube", L"VertexShader", L"PixelShader", L"Bark"); }

	void Update(float deltaTime) override;
};