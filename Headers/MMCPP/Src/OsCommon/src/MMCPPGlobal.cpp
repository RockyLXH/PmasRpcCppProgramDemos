/*
 * CMMCPPGlobal.cpp
 *
 *  Created on: 15/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *              0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCPPGlobal.hpp"
#include "MMC_definitions.h"


ELMO_INT32 CountSetBits(ELMO_ULINT32 ulValue);
ELMO_INT32 ConfigBulkReadCmn(MMC_CONNECT_HNDL        hConnHndl,
                            NC_BULKREAD_CONFIG_ENUM eConfig,
                            NC_BULKREAD_PRESET_ENUM ePreset,
                            ELMO_ULINT32             ulParametersArray[],
                            ELMO_UINT8               ucIsPreset,
                            ELMO_UINT16              usAxisRefArray[],
                            ELMO_UINT16              usAxisNumber,
                            ELMO_PFLOAT              fFactorsArray,
                            ELMO_UINT16&             usStatus);

ELMO_INT32 PerformBulkReadCmn(MMC_CONNECT_HNDL       hConnHndl,
                            NC_BULKREAD_CONFIG_ENUM eConfiguration,
                            ELMO_UINT16              usNumberOfAxes,
                            ELMO_INT32               iDataSize,
                            NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                            ELMO_PULINT32            ulOutputData,
                            ELMO_UINT16&             usAxisStatus);

CMMCException::CMMCException(const ELMO_INT8* msg, const ELMO_INT8* cAxisName, ELMO_UINT16 usAxisRef, ELMO_INT32 errid, unsigned short status) : m_usErrorID(errid), m_usStatus(status), m_usAxisRef(usAxisRef)
{
	strcpy(m_cAxisName, cAxisName);
	strcpy(m_cMessage, msg);
}

void  CMMCPPGlobal::MMCPPThrow(const ELMO_INT8*  cFunctionName, MMC_CONNECT_HNDL uiConnHndl, ELMO_UINT16 usAxisRef, ELMO_INT32 iRetCode, ELMO_INT16 sErrorID, ELMO_UINT16 usStatus)
{
    //
    MMCPP_TRACE("%s return code=%d, axis name=%s, axis ref=%d, error id=%d, status=%d\n", cFunctionName, iRetCode, "NA", usAxisRef, sErrorID, usStatus);
    //
    if(iRetCode > 0)
    {
        // WARNING
        if(IsToThrowWarning())
        {
            // Try-Catch
            throw (CMMCException(cFunctionName, "NA", usAxisRef, sErrorID, usStatus));
        }
        else if(IsToCallWarningClbk())
        {
            // Special callback that implemented by user
            m_pWarningClbk(cFunctionName, uiConnHndl, usAxisRef, iRetCode, sErrorID, usStatus);
        }

        if(m_bPrintWarning == true)
        {
            //If this is a warning, just print it and exit back to the program
            ThrowMessage(iRetCode, sErrorID, cFunctionName);
        }
        // Return to the program without exit
        return;
    }
    //
    // If the throw feature is enabled , print the message and go to throw
    if (IsToThrowError())
    {
        throw (CMMCException(cFunctionName, "NA", usAxisRef, sErrorID, usStatus));
        //
        // When we use "Try-Catch" mechanism we should define
        // whether we want to close the programm in case of error or not
        // If not, return from this point to main program without
        // reaching the exit(0) point (below).
        if(m_bIsToCloseOnThrow == false)
        {
            if(m_bPrintError == true)
            {
                //If this is a error, just print it and exit back to the program
                ThrowMessage(iRetCode,sErrorID,cFunctionName);
            }
            //
            return;
        }
    }
    // If the throw feature is disabled, go to callback function(if defined) and then print message
    else if (IsToCallRTEClbk())
    {
        m_pRTEClbk(cFunctionName, uiConnHndl, usAxisRef, sErrorID, usStatus);
        //
        // When we register the RTE function callback we should define
        // whether we want to close the programm in case of error or not
        // If not, return from this point to main program without
        // reaching the exit(0) point (below).
        if(m_bIsToCloseOnRTE == false)
        {
            if(m_bPrintError == true)
            {
                //If this is a error, just print it and exit back to the program
                ThrowMessage(iRetCode,sErrorID,cFunctionName);
            }
            //
            return;
        }
    }
    //
    else if(m_bPrintError == true)
    {
        //If this is a error, just print it and exit back to the program
        ThrowMessage(iRetCode,sErrorID,cFunctionName);
    }
    //
    MMC_CloseConnection(uiConnHndl);
    exit(0);
    //
}
//
//
/*! \fn unsigned short GetSyncTime()
* \brief The function returns the SYNC time, in case of CAN communication.
* \return   return - sync time on success, otherwise throws CMMCException.
*/
ELMO_UINT16 CMMCPPGlobal::GetSyncTime(MMC_CONNECT_HNDL uiConnHndl) throw (CMMCException)
{
    MMC_GETSYNCTIME_IN  stInParams;
    MMC_GETSYNCTIME_OUT stOutParam;
    ELMO_INT32           rc;

    if ((rc=MMC_GetSyncTimeCmd(uiConnHndl, &stInParams, &stOutParam))!=0)
    {
        MMCPPThrow("MMC_GetSyncTimeCmd:", uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetSyncTime OK");
    return stOutParam.usSYNCTime;
}
//
/*! \fn int SetSyncTime(unsigned short usSync)
* \brief The function sets the SYNC time in the communication module.
* \param  usSync - sync time.
* \return   return - 0 if success, otherwise throws CMMCException.
*/
ELMO_INT32 CMMCPPGlobal::SetSyncTime(MMC_CONNECT_HNDL uiConnHndl, ELMO_UINT16 usSync) throw (CMMCException)
{
    MMC_SETSYNCTIME_IN  set_sync_time_in;
    MMC_SETSYNCTIME_OUT set_sync_time_out;
    ELMO_INT32           rc;


    set_sync_time_in.usSYNCTime=usSync;

    if ((rc=MMC_SetSyncTimeCmd(uiConnHndl, &set_sync_time_in,&set_sync_time_out))!=0)
    {
        MMCPPThrow("MMC_SetSyncTimeCmd:", uiConnHndl, 0, rc, set_sync_time_out.usErrorID, set_sync_time_out.usStatus);
    }
    _MMCPP_TRACE_INFO("SetSyncTime OK");
    return(0);
}
//
/*! \fn int DestroySYNCTimer(unsigned int uiConnHndl
* \brief
* \param  uiConnHndl connection handler
* \return   return - 0 if success, otherwise throws CMMCException.
*/
void CMMCPPGlobal::DestroySYNCTimer(MMC_CONNECT_HNDL uiConnHndl) throw (CMMCException)
{
    ELMO_INT32   rc;

    if ((rc=MMC_DestroySYNCTimer(uiConnHndl))!=0)
    {
        MMCPPThrow("MMC_DestroySYNCTimer:", uiConnHndl, 0, rc, (ELMO_INT16)(0), (ELMO_UINT16)(-1));
    }
    _MMCPP_TRACE_INFO("DestroySYNCTimer OK");
}

void CMMCPPGlobal::CreateSYNCTimer(MMC_CONNECT_HNDL uiConnHndl, MMC_SYNC_TIMER_CB_FUNC  pfClbk, ELMO_UINT16 usSYNCTimerTime)  throw (CMMCException)
{
    ELMO_INT32 rc;

    if ((rc=MMC_CreateSYNCTimer(uiConnHndl, pfClbk, usSYNCTimerTime))!=0)
    {
        MMCPPThrow("MMC_CreateSYNCTimer:", uiConnHndl, 0, rc, (ELMO_INT16)(0), (ELMO_UINT16)(-1));
    }
    _MMCPP_TRACE_INFO("CreateSYNCTimer OK");
}
//
//      MMCPP_CONN_REG m_ConnRegTable[MAX_CONNECTIONS_NUM];
ELMO_INT32 CMMCPPGlobal::RegisterConnection(MMC_CONNECT_HNDL uiConnHndl, ELMO_PVOID pConn)
{
    for(ELMO_INT32 i=0; i<MMCPP_MAX_CONNECTIONS_NUM; i++) {
        if (m_ConnRegTable[i].m_uiConnHndl == 0) {
            m_ConnRegTable[i].m_pConn=pConn;
            m_ConnRegTable[i].m_uiConnHndl = uiConnHndl;
            break;
        }
    }
    return 0;
}
//
ELMO_PVOID CMMCPPGlobal::GetConnectionReg(MMC_CONNECT_HNDL uiConnHndl)
{
    for(ELMO_INT32 i=0; i<MMCPP_MAX_CONNECTIONS_NUM; i++)
    {
        if (uiConnHndl == m_ConnRegTable[i].m_uiConnHndl)
            return m_ConnRegTable[i].m_pConn;
    }
    return NULL;
}
//
void CMMCPPGlobal::ClearConnectionReg(MMC_CONNECT_HNDL uiConnHndl)
{
    for(ELMO_INT32 i=0; i<MMCPP_MAX_CONNECTIONS_NUM; i++) {
        if (uiConnHndl == m_ConnRegTable[i].m_uiConnHndl) {
            m_ConnRegTable[i].m_pConn=NULL;
            m_ConnRegTable[i].m_uiConnHndl = 0;
            break;
        }
    }
}
//
void CMMCPPGlobal::ThrowMessage(ELMO_INT32 iRetval,ELMO_INT32 iErrorID,const ELMO_INT8* cFunctionName)
{
    cout << endl;
    cout << "-----------------------------------------" << endl;
    cout << "----     Get Error\\Warning           ----" << endl;
    cout << "----     From API function           ----" << endl;
    cout << "-----------------------------------------" << endl;
    cout << "Function name:\t" << cFunctionName << endl;
    cout << "Return value :\t" << iRetval << endl;
    cout << "ErrorID      :\t" << iErrorID << endl;
    //
    #if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        m_cMessage.MessageHandler(iRetval,iErrorID);
    #endif

    return;
}
//
//
void CMMCPPGlobal::ConfigBulkRead(MMC_CONNECT_HNDL          hConnHndl,
                                  NC_BULKREAD_CONFIG_ENUM   eConfig,
                                  NC_BULKREAD_PRESET_ENUM   ePreset,
                                  ELMO_UINT16                usAxisRefArray[],
                                  ELMO_UINT16                usNumberOfAxes,
                                  ELMO_PFLOAT                fFactorsArray) throw (CMMCException)
{
    ELMO_INT32 rc = NC_OK;
    ELMO_UINT16 usAxisStatus = 0;

    rc = ConfigBulkReadCmn(hConnHndl,
                           eConfig,
                           ePreset,
                           NULL,
                           true,
                           usAxisRefArray,
                           usNumberOfAxes,
                           fFactorsArray,
                           usAxisStatus);
        if (NC_OK != rc)
        {
            MMCPPThrow("ConfigBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
        }
    _MMCPP_TRACE_INFO("ConfigBulkRead() with presets OK");
}
//
//
void CMMCPPGlobal::ConfigBulkRead(MMC_CONNECT_HNDL          hConnHndl,
                                  NC_BULKREAD_CONFIG_ENUM   eConfig,
                                  ELMO_ULINT32               ulParameters[],
                                  ELMO_UINT16                usAxisRefArray[],
                                  ELMO_UINT16                usNumberOfAxes,
                                  ELMO_PFLOAT                fFactorsArray) throw (CMMCException)

{
    ELMO_INT32 rc = NC_OK;
    ELMO_UINT16 usAxisStatus = 0;

    rc = ConfigBulkReadCmn(hConnHndl,
                           eConfig,
                           eNC_BULKREAD_PRESET_NONE,
                           ulParameters,
                           false,
                           usAxisRefArray,
                           usNumberOfAxes,
                           fFactorsArray,
                           usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("ConfigBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }
    _MMCPP_TRACE_INFO("ConfigBulkRead() with custom parameters OK");
}

void CMMCPPGlobal::PerformBulkRead(MMC_CONNECT_HNDL         hConnHndl,
                                   ELMO_UINT16               usNumberOfAxes,
                                   NC_BULKREAD_CONFIG_ENUM  eConfiguration,
                                   NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                                   NC_BULKREAD_PRESET_1     stOutputData[])
{
    ELMO_INT32 rc = NC_OK;
    ELMO_ULINT32 ulOutputData[NC_MAX_BULK_READ_READABLE_PACKET_SIZE];
    ELMO_UINT16 usAxisStatus = 0;
    ELMO_INT32 iBufferSize = 0;

    rc = PerformBulkReadCmn(hConnHndl,
                            eConfiguration,
                            usNumberOfAxes,
                            sizeof(NC_BULKREAD_PRESET_1),
                            eChosenPreset,
                            ulOutputData,
                            usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("PerformBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }

    iBufferSize = usNumberOfAxes * (NC_BULK_READ_DEFAULT_VARIABLES_PER_PRESET + 1) * sizeof(ELMO_LINT32);
    memcpy(stOutputData, ulOutputData, iBufferSize);
}


void CMMCPPGlobal::PerformBulkRead(MMC_CONNECT_HNDL         hConnHndl,
                                   ELMO_UINT16               usNumberOfAxes,
                                   NC_BULKREAD_CONFIG_ENUM  eConfiguration,
                                   NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                                   NC_BULKREAD_PRESET_2     stOutputData[])
{
    ELMO_INT32   rc = NC_OK;
    ELMO_ULINT32 ulOutputData[NC_MAX_BULK_READ_READABLE_PACKET_SIZE];
    ELMO_UINT16  usAxisStatus = 0;
    ELMO_INT32   iBufferSize = 0;

    rc = PerformBulkReadCmn(hConnHndl,
                            eConfiguration,
                            usNumberOfAxes,
                            sizeof(NC_BULKREAD_PRESET_2),
                            eChosenPreset,
                            ulOutputData,
                            usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("PerformBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }

    iBufferSize = usNumberOfAxes * (NC_BULK_READ_DEFAULT_VARIABLES_PER_PRESET) * sizeof(ELMO_LINT32);
    memcpy(stOutputData, ulOutputData, iBufferSize);
}

void CMMCPPGlobal::PerformBulkRead(MMC_CONNECT_HNDL         hConnHndl,
                                   ELMO_UINT16               usNumberOfAxes,
                                   NC_BULKREAD_CONFIG_ENUM  eConfiguration,
                                   NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                                   NC_BULKREAD_PRESET_3     stOutputData[])
{
    ELMO_INT32 rc = NC_OK;
    ELMO_ULINT32 ulOutputData[NC_MAX_BULK_READ_READABLE_PACKET_SIZE];
    ELMO_UINT16 usAxisStatus = 0;
    ELMO_INT32 iBufferSize = 0;

    rc = PerformBulkReadCmn(hConnHndl,
                            eConfiguration,
                            usNumberOfAxes,
                            sizeof(NC_BULKREAD_PRESET_3),
                            eChosenPreset,
                            ulOutputData,
                            usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("PerformBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }

    iBufferSize = usNumberOfAxes * (NC_BULK_READ_DEFAULT_VARIABLES_PER_PRESET + 5) * sizeof(ELMO_LINT32);
    memcpy(stOutputData, ulOutputData, iBufferSize);
}

void CMMCPPGlobal::PerformBulkRead(MMC_CONNECT_HNDL         hConnHndl,
                                   ELMO_UINT16               usNumberOfAxes,
                                   NC_BULKREAD_CONFIG_ENUM  eConfiguration,
                                   NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                                   NC_BULKREAD_PRESET_4     stOutputData[])
{
    ELMO_INT32 rc = NC_OK;
    ELMO_ULINT32 ulOutputData[NC_MAX_BULK_READ_READABLE_PACKET_SIZE];
    ELMO_UINT16 usAxisStatus = 0;
    ELMO_INT32 iBufferSize = 0;

    rc = PerformBulkReadCmn(hConnHndl,
                            eConfiguration,
                            usNumberOfAxes,
                            sizeof(NC_BULKREAD_PRESET_4),
                            eChosenPreset,
                            ulOutputData,
                            usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("PerformBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }

    iBufferSize = usNumberOfAxes * (NC_BULK_READ_DEFAULT_VARIABLES_PER_PRESET + 5) * sizeof(ELMO_LINT32);
    memcpy(stOutputData, ulOutputData, iBufferSize);
}

void CMMCPPGlobal::PerformBulkRead(MMC_CONNECT_HNDL         hConnHndl,
                                   ELMO_UINT16               usNumberOfAxes,
                                   NC_BULKREAD_CONFIG_ENUM  eConfiguration,
                                   NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                                   NC_BULKREAD_PRESET_5     stOutputData[])
{
    ELMO_INT32 rc = NC_OK;
    ELMO_ULINT32 ulOutputData[NC_MAX_BULK_READ_READABLE_PACKET_SIZE];
    ELMO_UINT16 usAxisStatus = 0;
    ELMO_INT32 iBufferSize = 0;

    rc = PerformBulkReadCmn(hConnHndl,
                            eConfiguration,
                            usNumberOfAxes,
                            sizeof(NC_BULKREAD_PRESET_5),
                            eChosenPreset,
                            ulOutputData,
                            usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("PerformBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }

    iBufferSize = usNumberOfAxes * (NC_BULK_READ_DEFAULT_VARIABLES_PER_PRESET + 9) * sizeof(ELMO_LINT32);
    memcpy(stOutputData, ulOutputData, iBufferSize);
}


void CMMCPPGlobal::PerformBulkRead(MMC_CONNECT_HNDL         hConnHndl,
                                   ELMO_UINT16               usNumberOfAxes,
                                   NC_BULKREAD_CONFIG_ENUM  eConfiguration,
                                   NC_BULKREAD_PRESET_ENUM& eChosenPreset,
                                   ELMO_PULINT32             ulOutputData)
{
    ELMO_INT32 rc = NC_OK;
    ELMO_UINT16 usAxisStatus = 0;

    rc = PerformBulkReadCmn(hConnHndl,
                            eConfiguration,
                            usNumberOfAxes,
                            NC_MAX_BULK_READ_READABLE_PACKET_SIZE * sizeof(ELMO_LINT32)/usNumberOfAxes,
                            eChosenPreset,
                            ulOutputData,
                            usAxisStatus);
    if (NC_OK != rc)
    {
        MMCPPThrow("PerformBulkRead:", hConnHndl, 0, rc, rc, usAxisStatus);
    }
}

ELMO_INT32 ConfigBulkReadCmn(MMC_CONNECT_HNDL        hConnHndl,
                            NC_BULKREAD_CONFIG_ENUM eConfig,
                            NC_BULKREAD_PRESET_ENUM ePreset,
                            ELMO_ULINT32             ulParametersArray[],
                            ELMO_UINT8               ucIsPreset,
                            ELMO_UINT16              usAxisRefArray[],
                            ELMO_UINT16              usNumberOfAxes,
                            ELMO_PFLOAT              fFactorsArray,
                            ELMO_UINT16&             usStatus)
{
    ELMO_INT32 rc = NC_OK;
    MMC_CONFIGBULKREAD_IN stConfigBRIn;
    MMC_CONFIGBULKREAD_OUT stConfigBROut;
    ELMO_INT32 iNumberOfSignals = 0;

    ELMO_INT32 i = 0;


    // iNumberOfAxes = CountSetBits(ulAxisRefMask);
    stConfigBRIn.eConfiguration = eConfig;
    if (false == ucIsPreset)
    {
        while (0 != ulParametersArray[i])
        {
            i++;
        }
        iNumberOfSignals = i;
        memcpy(stConfigBRIn.uBulkReadParams.ulBulkReadParameters, ulParametersArray, iNumberOfSignals * usNumberOfAxes * sizeof(ELMO_LINT32));
    }
    else
    {
        iNumberOfSignals = NC_BULK_READ_DEFAULT_VARIABLES_PER_PRESET;
        if (eNC_BULKREAD_PRESET_1 == ePreset)
        {
            iNumberOfSignals++;
        }

        stConfigBRIn.uBulkReadParams.eBulkReadPreset = ePreset;
    }
    stConfigBRIn.usNumberOfAxes = usNumberOfAxes;
    stConfigBRIn.ucIsPreset = ucIsPreset;
    memcpy(stConfigBRIn.usAxisRefArray, usAxisRefArray, usNumberOfAxes * sizeof(ELMO_INT16));

    rc = MMC_ConfigBulkReadCmd(hConnHndl, &stConfigBRIn, &stConfigBROut);

    if (NC_OK != rc)
    {
        usStatus = stConfigBROut.usStatus;
        return stConfigBROut.usErrorID;
    }

    memcpy(fFactorsArray, stConfigBROut.fFactorsArray, usNumberOfAxes * iNumberOfSignals * sizeof(ELMO_LINT32));

    _MMCPP_TRACE_INFO("ConfigBulkRead() custom OK");

    rc = NC_OK;
    usStatus = stConfigBROut.usStatus;

    return rc;

}


ELMO_INT32 PerformBulkReadCmn(MMC_CONNECT_HNDL           hConnHndl,
                             NC_BULKREAD_CONFIG_ENUM    eConfiguration,
                             ELMO_UINT16                 usNumberOfAxes,
                             ELMO_INT32                  iDataSize,
                             NC_BULKREAD_PRESET_ENUM&   eChosenPreset,
                             ELMO_PULINT32               ulOutputData,
                             ELMO_UINT16&                usAxisData)
{
    MMC_PERFORMBULKREAD_IN stPerformBRIn;
    MMC_PERFORMBULKREAD_OUT stPerformBROut;

    ELMO_INT32 rc = NC_OK;
    ELMO_INT32 iReadDataSize = usNumberOfAxes * iDataSize;

    stPerformBRIn.eConfiguration = eConfiguration;

    rc = MMC_PerformBulkReadCmd(hConnHndl, &stPerformBRIn, &stPerformBROut);
    if (NC_OK != rc)
    {
        usAxisData = stPerformBROut.usStatus;
        return rc;
    }

    memcpy(ulOutputData, stPerformBROut.ulOutBuf, iReadDataSize);
    eChosenPreset = stPerformBROut.eChosenPreset;
    return NC_OK;
}

ELMO_INT32 CountSetBits(ELMO_ULINT32 ulValue)
{
    ELMO_INT16 sCnt = 0;
    for (ELMO_INT32 i = 0; i < ((ELMO_INT32 )sizeof(ELMO_ULINT32) * 8); i++)
    {
        if (ulValue & (1 << i))
        {
            sCnt++;
        }
    }

    return sCnt;
}

//execute user command function
void CMMCPPGlobal::UserCommandControlExe(MMC_CONNECT_HNDL hConnHndl, ELMO_INT8 cUserCommandIn[128],MC_COMMAND_OPERATION eUsrCommandOpIn,ELMO_INT32 &iStatus, ELMO_INT8 cExecutableFileName[64])
{
    MMC_USRCOMMAND_IN   pInUserCommand;
    MMC_USRCOMMAND_OUT  pOutUserCommand;
    ELMO_INT32          rc;

    strcpy(pInUserCommand.cUserCommand,cUserCommandIn);
    pInUserCommand.eUsrCommandOp = eUsrCommandOpIn;

    if ((rc = MMC_UserCommandControl(hConnHndl,&pInUserCommand, &pOutUserCommand)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("UserCommandControlExe:", hConnHndl, 0, rc, pOutUserCommand.usErrorID, pOutUserCommand.usStatus);
    }
    if(eUsrCommandOpIn == eMMC_COMMAND_OPERATION_ISRUNNING)
    {
        iStatus = pOutUserCommand.ucIsRunning;
    }
    if(eUsrCommandOpIn == eMMC_COMMAND_OPERATION_ISEXIST)
    {
        iStatus = pOutUserCommand.ucIsExist;
    }
    if(eUsrCommandOpIn == eMMC_COMMAND_OPERATION_ISEXECUTABLERUNNING)
    {
        strcpy(cExecutableFileName,pOutUserCommand.cExecutableFileName);
        iStatus = pOutUserCommand.ucIsRunning;
    }
}

void CMMCPPGlobal::RegErrPolicy(MMC_CONNECT_HNDL hConnHndl,MMC_REGERRPOLICY_IN  stInParams) throw (CMMCException)
{
    MMC_REGERRPOLICY_OUT    stOutParams;
    ELMO_INT32 rc = 0;

    rc = MMC_RegErrPolicy(hConnHndl,&stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RegErrPolicy: ", hConnHndl, 0, rc, stOutParams.sErrorID, stOutParams.usStatus);
    }
}

void CMMCPPGlobal::GetErrPolicy(MMC_CONNECT_HNDL hConnHndl,MMC_GETERRPOLICY_IN  stInParams,MMC_GETERRPOLICY_OUT &stOutParams) throw (CMMCException)
{
    ELMO_INT32 rc = 0;

    rc = MMC_GetErrPolicy(hConnHndl,&stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetErrPolicy: ", hConnHndl, 0, rc, stOutParams.sErrorID, stOutParams.usStatus);
    }
}

void CMMCPPGlobal::ResetSystem(MMC_CONNECT_HNDL hConnHndl) throw (CMMCException)
{
    MMC_RESETSYSTEM_IN  stInParams;
    MMC_RESETSYSTEM_OUT stOutParams;
    ELMO_INT32 rc = 0;

    rc = MMC_ResetSystem(hConnHndl,&stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ResetSystem: ", hConnHndl, 0, rc, stOutParams.sErrorID, stOutParams.usStatus);
    }
}

void CMMCPPGlobal::RunSineGenrator(MMC_CONNECT_HNDL hConnHndl,MMC_SINEGENERATOR_IN &stInParams)throw (CMMCException)
{
	MMC_SINEGENERATOR_OUT stOutParams;
	ELMO_INT32 rc = 0;

	rc = MMC_SineGen(hConnHndl, &stInParams, &stOutParams);
	if (rc != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("RunSineGenrator: ", hConnHndl, 0, rc, stOutParams.sErrorID, stOutParams.usStatus);
	}
}
/**
* this method simply retrieves this library version release
* printout as follows:
* cerr << "mmcpp_lib version: " << CMMCPPGlobal::Instance().GetLibVersion() << endl;
*/
std::string CMMCPPGlobal::GetLibVersion() 
{
    string str = _MMCPP_VERSION_RELEASE;
    return str;
}

/*!
 * \fn	int GetProfileConditioning(MMC_PROFCONDINF_OUT& o_params)
 * \brief	this method gets 'profile conditioning' information.
 *
 * it gets 'profile conditioning' data for axes on which this mode of operation is enabled.
 *
 * \param hConnHndl	connection handler.
 * \param o_params	stores data information of active axes by profile conditioning.
 * \return 0 if completed successfully, otherwise error or throws exception.
 */
ELMO_INT32 CMMCPPGlobal::GetProfileConditioning(MMC_CONNECT_HNDL hConnHndl, MMC_PROFCONDINF_OUT& o_params) throw (CMMCException)
{
	MMC_PROFCONDINF_IN i_params;
	ELMO_INT32 rc = 0;
	/*
	 * this function is indifferent to axis reference
	 */
	if ( (rc = MMC_GetProfileConditioning(hConnHndl, 0, &i_params, &o_params)) != 0)
	{
		rc = o_params.usErrorID;
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetProfileConditioning: ", hConnHndl, -1, rc, o_params.usErrorID, o_params.usStatus);
	}
	_MMCPP_TRACE_INFO("MMC_GetProfileConditioning OK")

	return rc;
}


/*!
 * \fn void RegisterRTE(RTE_CLBKP clbk)
 * \brief this method registers  exceptions from Elmo's C++ library.
 * \param pRTEClbk the call-back function (the exception handler).
 * \param bIsToCloseOnRTE a flag. do not close on runtime exception if false, otherwise do close (default true).
 * \return none.
 */
void CMMCPPGlobal::RegisterRTE(RTE_CLBKP pRTEClbk, ELMO_BOOL bIsToCloseOnRTE)
{
	m_pRTEClbk = pRTEClbk;
	m_bIsToCloseOnRTE = bIsToCloseOnRTE;
}

/*!
 * \fn void register_rte_clbk(RTE_CLBKP clbk)
 * \brief this function is C interface, which allows C code to handle runtime exception from Elmo's C++ library.
 * \param clbk the call-back function (the exception handler).
 * \return none.
 */
void register_rte_clbk(RTE_CLBKP clbk)
{
	fprintf(stderr, "\x1b[33m" "%s: " "\x1b[0m" "\n", __FUNCTION__);
	CMMCPPGlobal::Instance()->RegisterRTE(clbk, false);
}
/*!
 * \fn void unregister_rte_clbk(void)
 * \brief this function is C interface, which allows C code to decouple from the  runtime exception handler via Elmo's C++ library.
 * \return none.
 */
void unregister_rte_clbk()
{
	CMMCPPGlobal::Instance()->RegisterRTE(NULL, ELMO_FALSE);
	fprintf(stderr, "\x1b[33m" "%s: " "\x1b[0m" "\n", __FUNCTION__);
}



#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))

    void nanosleep (const struct timespec *requested_delay)
    {
      if (requested_delay->tv_sec > 0)
        /* at least one second. Millisecond resolution is sufficient. */
        Sleep ((DWORD)requested_delay->tv_sec * 1000 + requested_delay->tv_nsec / 1000000);
      else {
          /* Use Sleep for the largest part, and busy-loop for the rest. */
          static ELMO_DOUBLE frequency;
          if (frequency == 0)
            {
              LARGE_INTEGER freq;
              if (!QueryPerformanceFrequency (&freq))
                {
                  /* Cannot use QueryPerformanceCounter. */
                  Sleep (requested_delay->tv_nsec / 1000000);
                  return;
                }
              frequency = (ELMO_DOUBLE) freq.QuadPart / 1000000000.0;
            }
          ELMO_DOUBLE expected_counter_difference = requested_delay->tv_nsec * frequency;
          ELMO_INT32 sleep_part = (ELMO_INT32) requested_delay->tv_nsec / 1000000 - 10;
          LARGE_INTEGER before;
          QueryPerformanceCounter (&before);
          ELMO_DOUBLE expected_counter = before.QuadPart + expected_counter_difference;
          if (sleep_part > 0)
            Sleep (sleep_part);
          for (;;) {
              LARGE_INTEGER after;
              QueryPerformanceCounter (&after);
              if (after.QuadPart >= expected_counter)
                break;
          }
        }
    }
#endif

