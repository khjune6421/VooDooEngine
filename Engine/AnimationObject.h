#pragma once

#include "Object.h"

class AnimationObject : public Object
{
	friend class Render;
	std::vector<UINT> m_shapeIds = {};
	void GetAnimationState(int& currentShapeIndex, int& nextShapeIndex, float& interpolationFactor) const { currentShapeIndex = m_currentShapeIndex; nextShapeIndex = m_nextShapeIndex; interpolationFactor = m_interpolationFactor; }

protected:
	int m_currentShapeIndex = 0;
	int m_nextShapeIndex = 0;
	float m_interpolationFactor = 0.0f;

public:
	AnimationObject(const std::vector<std::wstring>& shapeNames, const std::wstring& vertexShader = L"DefaultVertexShader", const std::wstring& pixelShader = L"DefaultPixelShader");
	virtual ~AnimationObject() override = default;
};