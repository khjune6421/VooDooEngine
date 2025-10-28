#pragma once

// DirectX header files
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

// DirectXTK header files
#include <SimpleMath.h>
#include <SpriteFont.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

// Assimp header files
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// DirectX libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// DirectXTK libraries
#pragma comment(lib, "DirectXTK.lib")

// Assimp library
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif