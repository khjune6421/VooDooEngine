#pragma once
// Standard library header
#include <typeindex>
#include <filesystem>
#include <fstream>
#include <array>

// Windows header
#include <wrl/client.h>
#include <wincodec.h>

// DirectX header
#include <d3dcompiler.h>

// DirectXTK header
#include <SimpleMath.h>
#include <SpriteFont.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <ScreenGrab.h>

// Windows libraries
#pragma comment(lib, "winmm.lib")

// DirectX libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// DirectXTK libraries
#pragma comment(lib, "DirectXTK.lib")

// Macro definitions
#define com_ptr Microsoft::WRL::ComPtr
#define verbose_cast(x) static_cast<vector<x, allocator<x>>::size_type>