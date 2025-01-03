#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <dwmapi.h>

#include <dxgi.h>
#include <d3d11.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_freetype.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include <functional>
#include <thread>

using std::function;
using std::thread;

#include "Overlay.h"
