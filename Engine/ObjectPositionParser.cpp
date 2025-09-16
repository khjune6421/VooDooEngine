#include "ObjectPositionParser.h"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

bool ObjectPositionParser::ParseLine(const string& line, ObjectPosition& position)
{
	stringstream ss(line);
	string token;
	int fieldIndex = 0;

	while (getline(ss, token, ','))
	{
		size_t start = token.find_first_not_of(" \t");
		size_t end = token.find_last_not_of(" \t");

		if (start == string::npos) continue;

		token = token.substr(start, end - start + 1);

		try
		{
			switch (fieldIndex)
			{
			case 0:
				position.id = stoi(token);
				break;

			case 1:
				position.x = stof(token);
				break;

			case 2:
				position.y = stof(token);
				break;

			case 3:
				position.z = stof(token);
				return true;

			default:
				return false;
			}
			fieldIndex++;
		}
		catch (const exception&)
		{
			return false;
		}
	}

	return false;
}

bool ObjectPositionParser::LoadPositions(const string& filename)
{
	ClearData();

	ifstream file(filename);
	if (!file.is_open())
	{
		cerr << "Error: Could not open file " << filename << endl;
		return false;
	}

	string line;
	while (getline(file, line))
	{
		if (line.empty() || line == "eof") continue;

		ObjectPosition position;
		if (ParseLine(line, position))
		{
			positions.push_back(position);
		}
		else
		{
			cerr << "Warning: Failed to parse line: " << line << endl;
		}
	}

	file.close();
	cout << "Loaded " << positions.size() << " tree positions from " << filename << endl;
	return true;
}