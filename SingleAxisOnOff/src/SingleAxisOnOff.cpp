#include "SingleAxisOnOff.h"
#include <iostream>

int CallbackFunc(char* recv_buf, short buf_size, void* lpsock)
{
	switch (recv_buf[1])
	{
	case ASYNC_REPLY_EVT:
		printf("\n ASYNC event Reply ");
		break;
	case EMCY_EVT:
		printf("\n Emergency Event received ");
		break;
	case MOTIONENDED_EVT:
		printf("\n Motion Ended Event received ");
		break;
	case HBEAT_EVT:
		printf("\n H Beat Fail Event received ");
		break;
	case PDORCV_EVT:
		printf("\n PDO Received Event received - Updating Inputs ");
		break;
	case DRVERROR_EVT:
		printf("\n Drive Error Received Event received ");
		break;
	case HOME_ENDED_EVT:
		printf("\n Home Ended Event received ");
		break;
	case SYSTEMERROR_EVT:
		printf("\n System Error Event received ");
		break;
	case TABLE_UNDERFLOW_EVT:
		printf("\n Underflow event received ");
		break;
	case MODBUS_WRITE_EVT:
		printf("\n ModBus Write event received ");
		break;
	case TOUCH_PROBE_ENDED_EVT:
		printf("\n Touch Probe event received ");
		break;
	default:
		printf("\n Default.... Whatever arrived event received ");
		break;
	}

	return 0;
}

int InitAxis()
{
	com_handle = connection.ConnectRPCEx(HOST_IP, DEST_IP, 0x7fffffff, (MMC_MB_CLBK)CallbackFunc);

	connection.GetVersion();

	connection.RegisterEventCallback(MMCPP_EMCY, (void*)nullptr);

	CMMCPPGlobal::Instance() -> SetThrowFlag(true);
	CMMCPPGlobal::Instance() -> SetThrowWarningFlag(false);

	cAxis[0].InitAxisData("a01", com_handle);

	return 0;
}

int CloseConnection()
{
	MMC_CloseConnection(com_handle);

	return 0;
}

int main(int, char**)
{
	Logger::Init();
	
	LOG_TRACE("TRACE: Logger system is working");
	LOG_DEBUG("DEBUG: Logger system is working");
	LOG_INFO("INFO: Logger system is working");
	LOG_WARN("WARN: Logger system is working");
	LOG_ERROR("ERROR: Logger system is working");
	LOG_CRITICAL("CRITICAL: Logger system is working");

	InitAxis();

	cAxis[0].PowerOn();
	while (!(cAxis[0].ReadStatus() & NC_AXIS_STAND_STILL_MASK));

	Sleep(5000);

	cAxis[0].PowerOff();

	CloseConnection();

	return 0;
}