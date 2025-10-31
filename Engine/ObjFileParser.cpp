#include "pch.h"
#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

void ObjFileParser::CalculateTangents(vector<Vertex>& vertices, const vector<UINT>& indices)
{
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		if (i + 2 >= indices.size()) break;

		UINT i0 = indices[i];
		UINT i1 = indices[i + 1];
		UINT i2 = indices[i + 2];

		if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) continue;

		Vertex& v0 = vertices[i0];
		Vertex& v1 = vertices[i1];
		Vertex& v2 = vertices[i2];

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
			m_uvs.emplace_back(uv.x, 1.0f - uv.y); // Flip y for DirectX
		}
		else if (line.substr(0, 2) == L"f ")
		{
			wstringstream ss(line.substr(2));
			vector<VertexKey> faceVertexKeys;
			wstring vertexData;

			while (ss >> vertexData)
			{
				wstringstream vertexStream(vertexData);
				wstring token;
				vector<wstring> indices;

				while (getline(vertexStream, token, L'/')) indices.push_back(token);

				VertexKey key = {};
				key.positionIndex = indices.size() > 0 && !indices[0].empty() ? stoi(indices[0]) - 1 : -1;
				key.uvIndex = indices.size() > 1 && !indices[1].empty() ? stoi(indices[1]) - 1 : -1;
				key.normalIndex = indices.size() > 2 && !indices[2].empty() ? stoi(indices[2]) - 1 : -1;

				faceVertexKeys.push_back(key);
			}

			for (size_t i = 1; i < faceVertexKeys.size() - 1; ++i)
			{
				for (size_t j = 0; j < 3; ++j)
				{
					size_t keyIndex = (j == 0) ? 0 : ((j == 1) ? i : i + 1);
					const VertexKey& key = faceVertexKeys[keyIndex];

					static unordered_map<VertexKey, UINT, VertexKeyHash> vertexMap;
					auto it = vertexMap.find(key);

					if (it != vertexMap.end()) currentShape->indices.push_back(it->second);
					else
					{
						Vertex vertex = {};

						if (key.positionIndex >= 0 && key.positionIndex < m_positions.size()) vertex.position = m_positions[key.positionIndex];
						if (key.normalIndex >= 0 && key.normalIndex < m_normals.size()) vertex.normal = m_normals[key.normalIndex];
						if (key.uvIndex >= 0 && key.uvIndex < m_uvs.size()) vertex.uv = m_uvs[key.uvIndex];

						UINT newIndex = static_cast<UINT>(currentShape->vertices.size());
						currentShape->vertices.push_back(vertex);
						currentShape->indices.push_back(newIndex);
						vertexMap[key] = newIndex;
					}
				}
			}
		}
	}

	for (auto& shape : m_shapes) CalculateTangents(shape.vertices, shape.indices);
}