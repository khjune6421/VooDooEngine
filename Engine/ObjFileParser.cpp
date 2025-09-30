#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

D3D11_INPUT_ELEMENT_DESC g_layoutDesc[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

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
				size_t slashPos = vertexData.find(L'/');
				wstring positionIndex = (slashPos != wstring::npos) ? vertexData.substr(0, slashPos) : vertexData;
				faceIndices.push_back(stoi(positionIndex) - 1);
			}

			for (size_t i = 1; i < faceIndices.size() - 1; ++i)
			{
				for (int idx : {faceIndices[0], faceIndices[i], faceIndices[i + 1]})
				{
					Vertex vertex = {};

					if (idx >= 0 && idx < positions.size()) vertex.position = positions[idx];
					if (idx >= 0 && idx < uvs.size()) vertex.uv = uvs[idx];
					if (idx >= 0 && idx < normals.size()) vertex.normal = normals[idx];

					currentShape->vertices.push_back(vertex);
				}
			}
		}
	}
}