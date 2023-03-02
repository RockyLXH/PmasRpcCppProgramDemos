#pragma once

#include "OS_PlatformDependSetting.hpp"
#include "MMCPPlib.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define HOST_IP		"192.168.1.2"
#define MAX_AXES	3

MMC_CONNECT_HNDL	com_handle;
CMMCConnection		connection;
CMMCSingleAxis		cAxis[MAX_AXES];

char DEST_IP[] = "192.168.1.3";
bool is_pmas_connected = false;
char axis_id[3][20];