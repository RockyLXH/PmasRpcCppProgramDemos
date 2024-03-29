#pragma once

#include "OS_PlatformDependSetting.hpp"
#include "MMCPPlib.hpp"

#define HOST_IP		"192.168.1.2"
#define DEST_IP		"192.168.1.3"
#define MAX_AXES	2

MMC_CONNECT_HNDL	com_handle;
CMMCConnection		connection;
CMMCSingleAxis		cAxis[MAX_AXES];
