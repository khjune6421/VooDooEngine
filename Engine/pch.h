#pragma once

// Standard library header
#include <Windows.h>
#include <iostream>
#include <vector>
#include <deque>
#include <wrl/client.h>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <typeindex>
#include <chrono>
#include <random>

// DirectX header
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

// DirectXTK header
#include <SimpleMath.h>
#include <SpriteFont.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

// Engine header
#include "Singleton.h"
#include "RandomNumberGenerator.h"

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