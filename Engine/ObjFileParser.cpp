#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

void ObjFileParser::CalculateTangents(vector<Vertex>& vertices) // How the hell does this even work
{
	for (size_t i = 0; i < vertices.size(); i += 3)
	{
		if (i + 2 >= vertices.size()) break;

		Vertex& v0 = vertices[i];
		Vertex& v1 = vertices[i + 1];
		Vertex& v2 = vertices[i + 2];

		XMFLOAT3 edge1 =
		{
			v1.position.x - v0.position.x,
			v1.position.y - v0.position.y,
			v1.position.z - v0.position.z
		};

		XMFLOAT3 edge2 =
		{
			v2.position.x - v0.position.x,
			v2.position.y - v0.position.y,
			v2.position.z - v0.position.z
		};

		XMFLOAT2 deltaUV1 =
		{
			v1.uv.x - v0.uv.x,
			v1.uv.y - v0.uv.y
		};

		XMFLOAT2 deltaUV2 =
		{
			v2.uv.x - v0.uv.x,
			v2.uv.y - v0.uv.y
		};

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		XMFLOAT3 tangent =
		{
			r * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
			r * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
			r * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
		};

		XMVECTOR tangentVec = XMLoadFloat3(&tangent);
		tangentVec = XMVector3Normalize(tangentVec);

		XMStoreFloat3(&v0.tangent, tangentVec);
		XMStoreFloat3(&v1.tangent, tangentVec);
		XMStoreFloat3(&v2.tangent, tangentVec);
	}
}

// Starting to realize why people just use json
ObjFileParser::ObjFileParser(const wstring& filename)
{
	wifstream file(filename);
	if (!file.is_open()) { MessageBoxW(nullptr, (L"Failed to open .obj file: " + filename).c_str(), L"Error", MB_OK); return; }

	Shape* currentShape = nullptr;

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
			m_positions.push_back(position);
		}
		else if (line.substr(0, 3) == L"vn ")
		{
			wstringstream ss(line.substr(3));
			XMFLOAT3 normal = {};
			ss >> normal.x >> normal.y >> normal.z;
			m_normals.push_back(normal);
		}
		else if (line.substr(0, 3) == L"vt ")
		{
			wstringstream ss(line.substr(3));
			XMFLOAT2 uv = {};
			ss >> uv.x >> uv.y;
			m_uvs.push_back(uv);
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

				if (posIdx >= 0 && posIdx < m_positions.size()) vertex.position = m_positions[posIdx];
				if (normalIdx >= 0 && normalIdx < m_normals.size()) vertex.normal = m_normals[normalIdx];
				if (uvIdx >= 0 && uvIdx < m_uvs.size()) vertex.uv = XMFLOAT2{ m_uvs[uvIdx].x, 1.0f - m_uvs[uvIdx].y }; // Flip y for DirectX

				currentShape->vertices.push_back(vertex);
			}
		}
	}

	for (auto& shape : m_shapes) CalculateTangents(shape.vertices);
}