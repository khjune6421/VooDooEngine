#include "ObjFileParser.h"

using namespace std;
using namespace DirectX;

ObjFileParser::ObjFileParser(const std::wstring& filename)
{
	wifstream file(filename);
	if (!file.is_open()) { MessageBoxW(nullptr, (L"Failed to open OBJ file: " + filename).c_str(), L"Error", MB_OK); return; }
}