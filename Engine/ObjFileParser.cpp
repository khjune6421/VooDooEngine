#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

// Starting to realize why people just use json
ObjFileParser::ObjFileParser(const wstring& filename)
{
	wifstream file(filename);
	if (!file.is_open()) { MessageBoxW(nullptr, (L"Failed to open .obj file: " + filename).c_str(), L"Error", MB_OK); return; }

	Shape* currentShape = nullptr;
	vector<XMFLOAT3> positions;
	vector<XMFLOAT3> normals;
	vector<XMFLOAT2> uvs;

	wstring line;
	while (getline(file, line))
	{
		if (line.empty() || line[0] == L'#') continue;

		if (line.substr(0, 2) == L"o ")
		{
			currentShape = &m_shapes.emplace_back();
			currentShape->name = line.substr(2);
		}
		if (!currentShape) continue;

		if (line.substr(0, 2) == L"v ")
		{
			wstringstream ss(line.substr(2));
			XMFLOAT3 position = {};
			ss >> position.x >> position.y >> position.z;
			positions.push_back(position);
		}
		else if (line.substr(0, 3) == L"vn ")
		{
			wstringstream ss(line.substr(3));
			XMFLOAT3 normal = {};
			ss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (line.substr(0, 3) == L"vt ")
		{
			wstringstream ss(line.substr(3));
			XMFLOAT2 uv = {};
			ss >> uv.x >> uv.y;
			uvs.push_back(uv);
		}
		else if (line.substr(0, 2) == L"f ")
		{
			wstringstream ss(line.substr(2));
			vector<int> faceIndices;
			wstring vertexData;

			while (ss >> vertexData)
			{
				wstringstream vertexStream(vertexData);
				wstring token;
				vector<wstring> indices;

				while (getline(vertexStream, token, L'/')) indices.push_back(token);

				int positionIndex = indices.size() > 0 && !indices[0].empty() ? stoi(indices[0]) - 1 : -1;
				faceIndices.push_back(positionIndex);

				int uvIndex = indices.size() > 1 && !indices[1].empty() ? stoi(indices[1]) - 1 : -1;
				faceIndices.push_back(uvIndex);

				int normalIndex = indices.size() > 2 && !indices[2].empty() ? stoi(indices[2]) - 1 : -1;
				faceIndices.push_back(normalIndex);
			}

			for (size_t i = 0; i < faceIndices.size(); i += 3)
			{
				Vertex vertex = {};

				int posIdx = faceIndices[i];
				int uvIdx = faceIndices[i + 1];
				int normalIdx = faceIndices[i + 2];

				if (posIdx >= 0 && posIdx < positions.size()) vertex.position = positions[posIdx];
				if (uvIdx >= 0 && uvIdx < uvs.size()) vertex.uv = uvs[uvIdx];
				if (normalIdx >= 0 && normalIdx < normals.size()) vertex.normal = normals[normalIdx];

				currentShape->vertices.push_back(vertex);
			}
		}
	}
}