#include "ElectronicGearing.h"
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
	char buf[10];

	com_handle = connection.ConnectRPCEx(HOST_IP, DEST_IP, 0x7fffffff, (MMC_MB_CLBK)CallbackFunc);

	connection.GetVersion();
	connection.GetLibVersion();

	connection.RegisterEventCallback(MMCPP_EMCY, (void*)nullptr);

	CMMCPPGlobal::Instance() -> SetThrowFlag(true);
	CMMCPPGlobal::Instance() -> SetThrowWarningFlag(false);

	for (int i = 0; i < MAX_AXES; ++i)
	{
		sprintf(buf, "a%02d", i+1);
		cAxis[i].InitAxisData(buf, com_handle);
	}

	return 0;
}

int CloseConnection()
{
	MMC_CloseConnection(com_handle);

	return 0;
}

int MainLoop()
{
	for (int i = 0; i < MAX_AXES; ++i)
	{
		cAxis[i].PowerOn();
		while (!(cAxis[i].ReadStatus() & NC_AXIS_STAND_STILL_MASK));

		cAxis[i].MoveAbsolute(0, 10000);
		while (!(cAxis[i].ReadStatus() & NC_AXIS_STAND_STILL_MASK));
	}
/*
	there are two ways to perform the GearIn function (overloaded 2 methods)
	but testing with lib version 350 and found that the 1 methods cannot work.
	after perform it, the slave axis cannot change the status to synchoniztion and cannot operation anymore.
	even cannot recovery by reset the Pmas. power off and on the Pmas is the only way.
*/
	//MMC_GEARINPOS_IN inParam;
	//inParam.usMaster = 0;
	//inParam.dbMasterStartDistance = 500;
	//inParam.dbMasterSyncPosition = 1000;
	//inParam.dbSlaveSyncPosition = 2000;
	//inParam.iRatioDenominator = 1;
	//inParam.iRatioNumerator = 1;
	//inParam.eMasterValueSource = eECAM_SET_VALUE;
	//inParam.eSyncMode = eSLOW_DOWN;
	//inParam.dbVelocity = 100000;
	//inParam.dbAcceleration = 1000000;
	//inParam.dbDeceleration = 1000000;
	//inParam.eBufferMode = MC_BUFFERED_MODE;
	//inParam.dbJerk = 2000000;

/* this one has bug and cannot work*/
	//cAxis[1].GearInPos(inParam);
/* this one works well*/
	cAxis[1].GearInPos(0, 500, 1000, 2000);

	while (!(cAxis[1].ReadStatus() & NC_AXIS_SYNCHRONIZED_MOTION_MASK));

	cAxis[0].MoveAbsolute(20000, 2000);

	return 0;
}

int main(int, char**)
{
	//Logger::Init();
	//
	//LOG_TRACE("TRACE: Logger system is working");
	//LOG_DEBUG("DEBUG: Logger system is working");
	//LOG_INFO("INFO: Logger system is working");
	//LOG_WARN("WARN: Logger system is working");
	//LOG_ERROR("ERROR: Logger system is working");
	//LOG_CRITICAL("CRITICAL: Logger system is working");

	InitAxis();

	MainLoop();

	CloseConnection();

	return 0;
}