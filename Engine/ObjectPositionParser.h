#pragma once
#include <vector>
#include <string>

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
	bool ParseLine(const std::string& line, ObjectPosition& position);

public:
	bool LoadPositions(const std::string& filename);
	const std::vector<ObjectPosition>& GetPositions() const { return positions; }
	void ClearData() { positions.clear(); }
};