#include "ObjectPositionParser.h"

using namespace std;

bool ObjectPositionParser::ParseLine(const wstring& line, ObjectPosition& position)
{
	wstringstream ss(line);
	wstring token;
	int fieldIndex = 0;

	while (getline(ss, token, L','))
	{
		size_t start = token.find_first_not_of(L" \t");
		size_t end = token.find_last_not_of(L" \t");

		if (start == wstring::npos) continue;

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

bool ObjectPositionParser::LoadPositions(const wstring& filename)
{
	ClearData();

	wifstream file(filename);
	if (!file.is_open())
	{
		wcerr << L"Error: Could not open file " << filename << endl;
		return false;
	}

	wstring line;
	while (getline(file, line))
	{
		if (line.empty() || line == L"eof") continue;

		ObjectPosition position;
		if (ParseLine(line, position))
		{
			positions.push_back(position);
		}
		else
		{
			wcerr << L"Warning: Failed to parse line: " << line << endl;
		}
	}

	file.close();
	wcout << L"Loaded " << positions.size() << L" tree positions from " << filename << endl;
	return true;
}