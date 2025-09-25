#pragma once

#include "UtilityHeaders.h"

struct ObjectPosition
{
	int id = -1;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

class ObjectPositionParser
{
	std::vector<ObjectPosition> positions;
	bool ParseLine(const std::wstring& line, ObjectPosition& position);

public:
	bool LoadPositions(const std::wstring& filename);
	const std::vector<ObjectPosition>& GetPositions() const { return positions; }
	void ClearData() { positions.clear(); }
};