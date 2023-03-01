/*
 * CMMCConnection.cpp
 *
 *  Created on: 22/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMC_definitions.h"
#include "MMCConnection.hpp"
#include "datastructs.h"

#include "MMCAxis.hpp"

#define GMAS_DEVICE						0x80


using namespace MMCPPUtils;


CMMCConnection::CMMCConnection() {
    m_fpClbk = NULL;
    m_cHost[0]=0;
    m_cDest[0]=0;
    m_uiConnHndl = 0;

    m_pfTouchProbeEndCallback = NULL;
    m_pfPdoRcvEventClbk = NULL;
    m_pfHBeateEventClbk = NULL;
    m_pfMotionEndEventClbk = NULL;
    m_pfEmergencyEventClbk = NULL;
    m_pfHomeEndEventClbk = NULL;
    m_pfModbusWriteEventClbk = NULL;
    m_pfAsyncReplyEventClbk = NULL;
    m_pfNodeErrorEventClbk = NULL;
    m_pfTableUnderflowEventClbk = NULL;
    m_pfNodeConnectedEventClbk = NULL;
    m_pfGlobalAsyncReplyEventClbk = NULL;
    m_pfNodeInitEventClbk = NULL;
    m_pfFBNotifyEventClbk = NULL;
	m_pfPolicyEndedEventClbk = NULL ;
	m_pfStopOnLimitEventClbk = NULL ;
}

CMMCConnection::~CMMCConnection() {
}

void CMMCConnection::CloseConnection() {
    ELMO_INT32 rc;
    if ((rc=MMC_CloseConnection(m_uiConnHndl)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CloseConnection:", m_uiConnHndl, 0, rc, (ELMO_INT16)0, (ELMO_UINT16)(-1));
    }
    CMMCPPGlobal::Instance()->ClearConnectionReg(m_uiConnHndl);
    _MMCPP_TRACE_INFO("CloseConnection OK")
}


ELMO_INT32 CMMCCallbackFunc(ELMO_PUINT8 pBuff, ELMO_INT8 sSize, ELMO_PVOID pIPSock, const MMC_IF_STRUCT* hndl) {

    MMC_CONNECT_HNDL uiConnHndle = (MMC_CONNECT_HNDL)hndl;

    CMMCConnection* pConn = (CMMCConnection*)CMMCPPGlobal::Instance()->GetConnectionReg(uiConnHndle);
    pConn->CallbackFunc(pBuff, sSize, pIPSock);
    return 0;
}

#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    MMC_CONNECT_HNDL CMMCConnection::ConnectIPC(ELMO_INT32 iEventMask, MMC_CB_FUNC fpClbk) {
        MMC_OPENUDPCHANNEL_IN  openudp_param_in;
        MMC_OPENUDPCHANNEL_OUT openudp_param_out;
        MMC_IPC_CONNECTION_PARAM_STRUCT stInParams;
        ELMO_INT32 rc;
        //
        CMMCPPGlobal::Instance()->SetConnectionType(CT_IPC);
        //
        stInParams.uiParams = 1;
        //m_fpClbk = fpClbk; //backward compatibility @YL 28-06-2011
        if ((rc=MMC_IPCInitConnection(stInParams, fpClbk, &m_uiConnHndl)) != 0)
        {
              CMMCPPGlobal::Instance()->MMCPPThrow("MMC_IPCInitConnection:", m_uiConnHndl, 0, rc, (ELMO_INT16)0, (ELMO_UINT16)(-1));
        }
        //CMMCPPGlobal::Instance()->RegisterConnection(m_uiConnHndl, this); //backward compatibility @YL 28-06-2011
    
        openudp_param_in.cFirst = 127;
        openudp_param_in.cSecond = 0;
        openudp_param_in.cThird = 0;
        openudp_param_in.cFourth = 1;
        openudp_param_in.iEventsMask = iEventMask;//0x7fffffff;
    
        if((rc = MMC_OpenUdpChannelCmd(m_uiConnHndl, &openudp_param_in, &openudp_param_out)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_OpenUdpChannelCmd:", m_uiConnHndl, 0, rc, openudp_param_out.usErrorID, openudp_param_out.usStatus);
        }
        _MMCPP_TRACE_INFO("ConnectIPC OK")
        GetVersion_Ex();
        GetLibVersion();
        return m_uiConnHndl;
    }

    MMC_CONNECT_HNDL CMMCConnection::ConnectIPCEx(ELMO_INT32 iEventMask, MMC_MB_CLBK fpClbk) {
        MMC_OPENUDPCHANNEL_IN  openudp_param_in;
        MMC_OPENUDPCHANNEL_OUT openudp_param_out;
        MMC_IPC_CONNECTION_PARAM_STRUCT stInParams;
        ELMO_INT32 rc;
        //
        CMMCPPGlobal::Instance()->SetConnectionType(CT_IPC);
        //
        stInParams.uiParams = 1;
        m_fpClbk = fpClbk;
        if ((rc=MMC_IPCInitConnection(stInParams, (MMC_CB_FUNC)CMMCCallbackFunc, &m_uiConnHndl)) != 0)
        {
              CMMCPPGlobal::Instance()->MMCPPThrow("MMC_IPCInitConnection:", m_uiConnHndl, 0, rc, (ELMO_INT16)0, (ELMO_UINT16)(-1));
        }
        CMMCPPGlobal::Instance()->RegisterConnection(m_uiConnHndl, this);
    
    
        openudp_param_in.cFirst = 127;
        openudp_param_in.cSecond = 0;
        openudp_param_in.cThird = 0;
        openudp_param_in.cFourth = 1;
        openudp_param_in.iEventsMask = iEventMask;//0x7fffffff;
    
        if((rc=MMC_OpenUdpChannelCmd(m_uiConnHndl, &openudp_param_in, &openudp_param_out)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_OpenUdpChannelCmd:", m_uiConnHndl, 0, rc, openudp_param_out.usErrorID, openudp_param_out.usStatus);
        }
        _MMCPP_TRACE_INFO("ConnectIPC OK")
        GetVersion_Ex();
        GetLibVersion();
        return m_uiConnHndl;
    }
#endif

MMC_CONNECT_HNDL CMMCConnection::ConnectRPC(ELMO_PINT8 cHostIP, ELMO_PINT8 cDestIP, ELMO_INT32 iEventMask, MMC_CB_FUNC fpClbk){
    MMC_CONNECTION_PARAM_STRUCT conn_param;
    MMC_OPENUDPCHANNEL_IN  openudp_param_in;
    MMC_OPENUDPCHANNEL_OUT openudp_param_out;
    ELMO_INT32 rc=-1;
    //
    CMMCPPGlobal::Instance()->SetConnectionType(CT_RPC);
    //
    //m_fpClbk = fpClbk; //backward compatibility @YL 28-06-2011
    if (strlen(cHostIP)<4 || strlen(cDestIP)<4) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_InitConnection: invalid IP address", m_uiConnHndl, 0, (-1), (ELMO_INT16)(NC_WRONG_FUNC_ARGUMENT_TYPE), (ELMO_UINT16)(-1));
    }
    strncpy(m_cDest , cDestIP, 16);
    strncpy(m_cHost , cHostIP, 16);
    m_cDest[15]=0;
    m_cHost[15]=0;

    conn_param.uiTcpPort=4000 ;
    strcpy((ELMO_PINT8)conn_param.ucIp , m_cDest);   // if RPC then receives an IP. Otherwise as set here.

    if ((rc=MMC_RpcInitConnection(MMC_RPC_CONN_TYPE, conn_param, fpClbk, m_cHost, &m_uiConnHndl)) != 0)
    {
          CMMCPPGlobal::Instance()->MMCPPThrow("MMC_InitConnection:", m_uiConnHndl, 0, rc, (ELMO_INT16)(0), (ELMO_UINT16)(-1));
    }

//#ifdef WIN32
#if (OS_PLATFORM != LINUXIPC32_PLATFORM)
	m_ucDeviceType = ((MMC_IF_STRUCT*)m_uiConnHndl)->ucDeviceType;
#endif

    //CMMCPPGlobal::Instance()->RegisterConnection(m_uiConnHndl, this); //backward compatibility @YL 28-06-2011

    ELMO_INT32 IN_A,IN_B,IN_C,IN_D;
    sscanf(cHostIP,"%d %*c %d %*c %d %*c %d",&IN_A,&IN_B,&IN_C,&IN_D);
    openudp_param_in.cFirst = (ELMO_UINT8)IN_A;
    openudp_param_in.cSecond = (ELMO_UINT8)IN_B;
    openudp_param_in.cThird = (ELMO_UINT8)IN_C;
    openudp_param_in.cFourth = (ELMO_UINT8)IN_D;
    openudp_param_in.iEventsMask = iEventMask;//0x7fffffff;


    if((rc=MMC_OpenUdpChannelCmd(m_uiConnHndl, &openudp_param_in, &openudp_param_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_OpenUdpChannelCmd:", m_uiConnHndl, 0, rc, openudp_param_out.usErrorID, openudp_param_out.usStatus);
    }
    _MMCPP_TRACE_INFO("ConnectRPC OK")
    return m_uiConnHndl;
}

MMC_CONNECT_HNDL CMMCConnection::ConnectRPCEx(ELMO_PINT8 cHostIP, ELMO_PINT8 cDestIP, ELMO_INT32 iEventMask, MMC_MB_CLBK fpClbk){
    MMC_CONNECTION_PARAM_STRUCT conn_param;
    MMC_OPENUDPCHANNEL_IN  openudp_param_in;
    MMC_OPENUDPCHANNEL_OUT openudp_param_out;
    ELMO_INT32 rc=-1;
    //
    CMMCPPGlobal::Instance()->SetConnectionType(CT_RPC);
    //
    m_fpClbk = fpClbk;
    if (strlen(cHostIP)<4 || strlen(cDestIP)<4) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_InitConnection: invalid IP address", m_uiConnHndl, 0, (-1), (ELMO_INT16)NC_WRONG_FUNC_ARGUMENT_TYPE, (ELMO_UINT16)(-1));
    }
    strncpy(m_cDest , cDestIP, 16);
    strncpy(m_cHost , cHostIP, 16);
    m_cDest[15]=0;
    m_cHost[15]=0;

    conn_param.uiTcpPort=4000 ;
    strcpy((ELMO_PINT8)conn_param.ucIp , m_cDest);   // if RPC then receives an IP. Otherwise as set here.

    if ((rc=MMC_RpcInitConnection(MMC_RPC_CONN_TYPE, conn_param, (MMC_CB_FUNC)CMMCCallbackFunc, m_cHost, &m_uiConnHndl)) != 0)
    {
          CMMCPPGlobal::Instance()->MMCPPThrow("MMC_InitConnection:", m_uiConnHndl, 0, rc, (ELMO_INT16)0, (ELMO_UINT16)(-1));
    }

//#ifdef WIN32
#if (OS_PLATFORM != LINUXIPC32_PLATFORM)
	m_ucDeviceType = ((MMC_IF_STRUCT*)m_uiConnHndl)->ucDeviceType;
#endif

    CMMCPPGlobal::Instance()->RegisterConnection(m_uiConnHndl, this);

    ELMO_INT32 IN_A,IN_B,IN_C,IN_D;
    sscanf(cHostIP,"%d %*c %d %*c %d %*c %d",&IN_A,&IN_B,&IN_C,&IN_D);
    openudp_param_in.cFirst = (ELMO_UINT8)IN_A;
    openudp_param_in.cSecond = (ELMO_UINT8)IN_B;
    openudp_param_in.cThird = (ELMO_UINT8)IN_C;
    openudp_param_in.cFourth = (ELMO_UINT8)IN_D;
    openudp_param_in.iEventsMask = iEventMask;//0x7fffffff;

    if((rc=MMC_OpenUdpChannelCmdEx(m_uiConnHndl, &openudp_param_in, &openudp_param_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_OpenUdpChannelCmdEx:", m_uiConnHndl, 0, rc, openudp_param_out.usErrorID, openudp_param_out.usStatus);
    }
    _MMCPP_TRACE_INFO("ConnectRPCEx OK")
    return m_uiConnHndl;
}

void CMMCConnection::GetVersion() throw (CMMCException)
{
    MMC_GET_VER_OUT ver_out;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetVersionCmd(m_uiConnHndl, &ver_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetVersionCmd:", m_uiConnHndl, 0, rc, ver_out.usErrorID, ver_out.usStatus);
    }
    ELMO_INT8 cMsg[512];
    sprintf(cMsg, "GMAS VERSION : <%c.%c.%c.%c> U-BOOT VERSION: <%d> \n",
                ver_out.cFirst, ver_out.cSecond, ver_out.cThird, ver_out.cFourth,
                ver_out.uiUbootVer);
    cout << cMsg << endl;
}

void CMMCConnection::GetVersionEx(std::string& strVersion) throw (CMMCException)
{
    MMC_GET_VEREX_OUT ver_out;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetVersionExCmd(m_uiConnHndl, &ver_out)) == 0)
    {
        cout << ver_out.pcData << endl;
        strVersion = ver_out.pcData;
    }
    else
    {
        MMC_GET_VER_OUT ver_out;
        if ((rc=MMC_GetVersionCmd(m_uiConnHndl, &ver_out)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetVersionCmd:", m_uiConnHndl, 0, rc, ver_out.usErrorID, ver_out.usStatus);
        }
        ELMO_INT8 cMsg[512];
        sprintf(cMsg, "GMAS VERSION : <%c.%c.%c.%c> U-BOOT VERSION: <%d> \n",
                    ver_out.cFirst, ver_out.cSecond, ver_out.cThird, ver_out.cFourth,
                    ver_out.uiUbootVer);
        cout << cMsg << endl;
        strVersion = cMsg;
    }
}   
void CMMCConnection::GetVersion(ELMO_INT32& iVer) throw (CMMCException)
{
    MMC_GET_VER_OUT ver_out;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetVersionCmd(m_uiConnHndl, &ver_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetVersionCmd:", m_uiConnHndl, 0, rc, ver_out.usErrorID, ver_out.usStatus);
    }
    ELMO_INT8 cMsg[512];
    sprintf(cMsg, "GMAS VERSION : <%c.%c.%c.%c> U-BOOT VERSION: <%d> \n",
                ver_out.cFirst, ver_out.cSecond, ver_out.cThird, ver_out.cFourth,
                ver_out.uiUbootVer);
    cout << cMsg << endl;
    iVer = rc;
}

void CMMCConnection::GetVersion_Ex() throw (CMMCException)
{
    MMC_GET_VEREX_OUT ver_out;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetVersionExCmd(m_uiConnHndl, &ver_out)) == 0)
    {
        cout << ver_out.pcData << endl;
    }
    else
    {
        MMC_GET_VER_OUT ver_out;
        if ((rc=MMC_GetVersionCmd(m_uiConnHndl, &ver_out)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetVersionCmd:", m_uiConnHndl, 0, rc, ver_out.usErrorID, ver_out.usStatus);
        }
        ELMO_INT8 cMsg[512];
        sprintf(cMsg, "GMAS VERSION : <%c.%c.%c.%c> U-BOOT VERSION: <%d> \n",
                    ver_out.cFirst, ver_out.cSecond, ver_out.cThird, ver_out.cFourth,
                    ver_out.uiUbootVer);
        cout << cMsg << endl;
    }
}   

void CMMCConnection::GetLibVersion()
{
    ELMO_INT32 rc;
    if ((rc = MMC_GetLibVersion()) < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetLibVersion:", m_uiConnHndl, 0, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    cout << "GMAS LIB VERSION:  " <<  rc << endl;
}
/*! \fn void pSetGlobalBoolParameter(unsigned long ulValue, MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
* \brief This function write boolean global parameter.
* \param ulValue - value to set.
* \param eNumber - parameter enumeration.
* \param iIndex - index into array in case of axis group.
* \return   return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::SetGlobalBoolParameter(ELMO_LINT32 lValue, MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_WRITEBOOLPARAMETER_IN stInParam;
    MMC_WRITEBOOLPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.lValue = lValue;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GlobalWriteBoolParameter(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GlobalWriteBoolParameter:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GlobalWriteBoolParameter OK");
}

/*! \fn void pSetGlobalBoolParameter(double dblValue, MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
* \brief This function write real global parameter.
* \param dbValue - value to set.
* \param eNumber - parameter enumeration.
* \param iIndex - index into array in case of axis group.
* \return   return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::SetGlobalParameter(ELMO_DOUBLE dbValue, MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_WRITEPARAMETER_IN stInParam;
    MMC_WRITEPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.dbValue = dbValue;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GlobalWriteParameter(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GlobalWriteParameter:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GlobalWriteParameter OK");
}

/*! \fn unsigned long GetGlobalBoolParameter(MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
* \brief This function read boolean global parameter.
* \param eNumber - parameter enumeration.
* \param iIndex - index into array in case of axis group.
* \return   return - actual parameter value on success, otherwise throws CMMCException.
*/
ELMO_LINT32 CMMCConnection::GetGlobalBoolParameter(MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_READBOOLPARAMETER_IN stInParam;
    MMC_READBOOLPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GlobalReadBoolParameter(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GlobalReadBoolParameter:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GlobalReadBoolParameter OK");
    return stOutParam.lValue;
}

/*! \fn double GetGlobalParameter(MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
* \brief This function read real global parameter.
* \param eNumber - parameter enumeration.
* \param iIndex - index into array in case of axis group.
* \return   return - actual parameter value on success, otherwise throws CMMCException.
*/
ELMO_DOUBLE CMMCConnection::GetGlobalParameter(MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_READPARAMETER_IN stInParam;
    MMC_READPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GlobalReadParameter(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GlobalReadParameter:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GlobalReadParameter OK");
    return stOutParam.dbValue;
}


/*! \fn SaveParameters() throw (CMMCException);
* \brief This function Save the global parameters to file.
* \return   return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::SaveParameters() throw (CMMCException)
{
    MMC_SAVEPARAM_IN stInParam;
    MMC_SAVEPARAM_OUT stOutParam;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_SaveParamCmd(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SaveParamCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SaveParam OK");
    return;
}


/*! \fn void LoadParameters() throw (CMMCException);
* \brief This function Load the global parameters to file.
* \return   return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::LoadParameters() throw (CMMCException)
{
    MMC_LOADPARAM_IN stInParam;
    MMC_LOADPARAM_OUT stOutParam;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_LoadParamCmd(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_LoadParamCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("LoadParam OK");
    return;
}


/*! \fn void SetDefaultManufacturerParameters() throw (CMMCException);
* \brief This function set the default manufacturer parameters to global parameters.
* \return   return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::SetDefaultManufacturerParameters() throw (CMMCException)
{
    MMC_SETDEFAULTPARAMETERSGLOBAL_IN stInParam;
    MMC_SETDEFAULTPARAMETERSGLOBAL_OUT stOutParam;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_SetDefaultParametersGlobalCmd(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetDefaultParametersGlobalCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetDefaultParametersGlobal OK");
    return;
}


/*! \fn void SetIsToLoadGlobalParams(int iVal) throw (CMMCException);
* \brief This function set the default manufacturer parameters to global parameters.
* \return   return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::SetIsToLoadGlobalParams(ELMO_UINT8 ucVal) throw (CMMCException)
{
    MMC_SETISTOLOADGLOBALPARAMS_IN stInParam;
    MMC_SETISTOLOADGLOBALPARAMS_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.ucValue = ucVal;
    //
    if ((rc=MMC_SetIsToLoadGlobalParamsCmd(m_uiConnHndl, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetDefaultParametersGlobalCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetDefaultParametersGlobal OK");
    return;
}


/*! \fn void eSetHeartBeatConsumer(unsigned int uiHeartbeatTimeFactor)
 *  \brief The function sets the consumer heartbeat time for all axes in the system
 *  \param  uiHeartbeatTimeFactor - time factor to set.
 *  \return return - none on success, otherwise throws CMMCException.
*/
void CMMCConnection::SetHeartBeatConsumer(ELMO_UINT32 uiHeartbeatTimeFactor) throw (CMMCException) {
    MMC_SETHEARTBEATCONSUMER_IN stInParams;
    MMC_SETHEARTBEATCONSUMER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParams.uiHeartbeatTimeFactor = uiHeartbeatTimeFactor;
    if ((rc=MMC_SetHeartBeatConsumerCmd(m_uiConnHndl, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetHeartBeatConsumerCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetHeartBeatConsumer OK");
}

ELMO_INT32 CMMCConnection::CallbackFunc(ELMO_PUINT8 pBuff, ELMO_INT16 sSize, ELMO_PVOID pIPSock) {

    ELMO_UINT16 	axis_ref;
    ELMO_UINT16 	status;
    ELMO_UINT16 	error_id;
    ELMO_UINT16 	event_ID;
    ELMO_UINT16 	cob_id;
    ELMO_UINT8		data_len;
    ELMO_UINT16 	EventGrp = 0;
    ELMO_UINT16 	EmergencyCode;
    ELMO_UINT8 		ucAsyncEventType;
    MMC_CAN_REPLY_DATA_OUT* can_reply_ptr;
	ELMO_PUINT16 	pui16Aux;
	ELMO_PUINT32 	pui32Aux;

	if(m_ucDeviceType == GMAS_DEVICE)
	{
		NetToLocal((ELMO_PVOID)&pBuff[EVENT_ID_INDX], (ELMO_PUINT16)&event_ID);
		NetToLocal((ELMO_PVOID)&pBuff[AXIS_REF_INDX], (ELMO_PUINT16)&axis_ref);
	}
	else
	{
		memcpy((ELMO_PVOID)&event_ID, (ELMO_PVOID)&pBuff[EVENT_ID_INDX], 2);
		memcpy((ELMO_PVOID)&axis_ref, (ELMO_PVOID)&pBuff[AXIS_REF_INDX], 2);
	}


    switch(event_ID)
    {
    case ASYNC_REPLY_EVT:
    {
        can_reply_ptr = (MMC_CAN_REPLY_DATA_OUT*)pBuff;
        data_len = can_reply_ptr->can_data_length;
        ucAsyncEventType = can_reply_ptr->ucAsyncEventType;
        //

		if(m_ucDeviceType == GMAS_DEVICE)
		{
			NetToLocal((ELMO_PVOID)&axis_ref, (ELMO_PUINT16)&axis_ref);
		}
		//else
		//{
		//	memcpy((ELMO_PUINT8)&axis_ref,(ELMO_PUINT8)&axis_ref, 2);
		//}
		//

        error_id = can_reply_ptr->usErrorid;
        cob_id = can_reply_ptr->usCOB_ID;
        status = can_reply_ptr->usStatus;
        //
        if (m_pfAsyncReplyEventClbk != NULL)
        {
            ELMO_UINT32	uiAbortCode = 0;
            ELMO_UINT16 usIndex 	= 0;
            ELMO_UINT8 	ucHeader 	= 0;
            ELMO_UINT8 	ucSubIndex 	= 0;
            //
            if((ELMO_INT16)error_id == NC_SDO_ABORTED)
            {
                ucHeader 	= can_reply_ptr->data[0];
                ucSubIndex 	= can_reply_ptr->data[3];

				pui16Aux	= (ELMO_PUINT16)(&can_reply_ptr->data[1]);
                usIndex 	= *pui16Aux;
				pui32Aux 	= (ELMO_PUINT32)(&can_reply_ptr->data[4]);
                uiAbortCode = *pui32Aux;


				//#ifndef WIN32
				#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
					//	#ifndef WIN32
					//	// m_ucDeviceType = ((MMC_IF_STRUCT*)m_uiConnHndl)->ucDeviceType;
					//	if(m_ucDeviceType == GMAS_DEVICE)
					//	{
					//		endian_swap32(&(uiAbortCode));
					//		endian_swap16(&(usIndex));
					//	}
					//	#endif
	
					//#if   (ENDIAN_SELECT == LITTLEENDIAN_CPU)
					//    endian_swap32(&(uiAbortCode));
					//    endian_swap16(&(usIndex));
					//#endif
	
					if(m_ucDeviceType == GMAS_DEVICE)
					{
						endian_swap32(&(uiAbortCode));
						endian_swap16(&(usIndex));
					}
			   #endif
            }
            m_pfAsyncReplyEventClbk(axis_ref,status, error_id, cob_id, data_len, pBuff,ucAsyncEventType,uiAbortCode,usIndex,ucHeader,ucSubIndex);
        }
    }
    break ;

    case EMCY_EVT:
    {
        if (m_pfEmergencyEventClbk != NULL)
        {   
            ELMO_UINT16 usDataLen;
            ELMO_UINT16 usEmergencyCode;
            ELMO_INT8 cErrReg = 0;
            ELMO_INT8 cManSpec[EMERGENCY_MANUFACTURER_SPECIFIC_BLOCK_SIZE] = {0};
            //

			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[EMGCY_LEN_INDX],  (ELMO_PUINT16)&usDataLen);
				NetToLocal((ELMO_PVOID)&pBuff[EMGCY_DATA_INDX], (ELMO_PUINT16)&usEmergencyCode);
			}
			else
			{
				memcpy((ELMO_PVOID)&usDataLen,      (ELMO_PVOID)&pBuff[EMGCY_LEN_INDX],  2);
				memcpy((ELMO_PVOID)&usEmergencyCode,(ELMO_PVOID)&pBuff[EMGCY_DATA_INDX], 2);
			}
			//

            //
            if(usDataLen == 14)
            {// New implementation - two additional variables (the old length is 8)
                cErrReg = pBuff[EMGCY_DATA_INDX+2];
                memcpy((ELMO_PVOID)cManSpec, (ELMO_PVOID)&pBuff[EMGCY_DATA_INDX+3], EMERGENCY_MANUFACTURER_SPECIFIC_BLOCK_SIZE);
            }
            m_pfEmergencyEventClbk(axis_ref, usEmergencyCode, cErrReg, cManSpec); //send axis ref end emergency code
        }
    }
    break ;

    case MOTIONENDED_EVT:
    {
        if (m_pfMotionEndEventClbk != NULL) 
        {
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], (ELMO_PUINT16)&error_id);
			}
			else
			{
				memcpy((ELMO_PVOID)&error_id, (ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], 2);
			}
            m_pfMotionEndEventClbk(axis_ref, error_id == 0); //send axis ref end OK or not
        }
    }
    break;

    case HBEAT_EVT:
    {   
        if (m_pfHBeateEventClbk != NULL) 
        {
            m_pfHBeateEventClbk(axis_ref); //send axis ref
        }
    }
    break ;

    case PDORCV_EVT:
    {
        UNPDODAT unData1;
        UNPDODAT unData2;
        //
        unData1.lVal = 0;
        unData2.lVal = 0;
        //
        EventGrp = pBuff[PDO_GROUP_INDX];
        switch (EventGrp)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 7:
            case 8:
            case 9:
            case 10:
            case 12:
            case 13:
            case 14:
            case 15:
				if(m_ucDeviceType == GMAS_DEVICE)
				{
					NetToLocal((ELMO_PVOID)&pBuff[PDO_DATA_INDX],   (ELMO_PVOID)&unData1.lVal);
					NetToLocal((ELMO_PVOID)&pBuff[PDO_DATA_INDX+4], (ELMO_PVOID)&unData2.lVal);
				}
				else
				{
					memcpy((ELMO_PVOID)&unData1.lVal, (ELMO_PVOID)&pBuff[PDO_DATA_INDX],  4);
					memcpy((ELMO_PVOID)&unData2.lVal, (ELMO_PVOID)&pBuff[PDO_DATA_INDX+4],4);
				}
                break;
            case 5:
            case 6:
				if(m_ucDeviceType == GMAS_DEVICE)
				{
					NetToLocal((ELMO_PVOID)&pBuff[PDO_DATA_INDX],   (ELMO_PUINT16)&unData1.usVal);
					NetToLocal((ELMO_PVOID)&pBuff[PDO_DATA_INDX+2], (ELMO_PVOID)&unData2.fVal);
				}
				else
				{
					memcpy((ELMO_PVOID)&unData1.usVal, (ELMO_PVOID)&pBuff[PDO_DATA_INDX],  2);
					memcpy((ELMO_PVOID)&unData2.fVal,  (ELMO_PVOID)&pBuff[PDO_DATA_INDX+2],4);
				}
                break;
            case 11:
				if(m_ucDeviceType == GMAS_DEVICE)
				{
					NetToLocal((ELMO_PVOID)&pBuff[PDO_DATA_INDX], (ELMO_PVOID)&unData1.lVal);
				}
				else
				{
					memcpy((ELMO_PVOID)&unData1.lVal, (ELMO_PVOID)&pBuff[PDO_DATA_INDX], 4);
				}
                unData2.lVal = 0; //irrelevance
                break;
            default:
               break;
       }
        if (m_pfPdoRcvEventClbk != NULL) 
        {
            m_pfPdoRcvEventClbk(axis_ref, EventGrp, unData1, unData2);
        }
    }
    break ;
    case HOME_ENDED_EVT:
    {
        if (m_pfHomeEndEventClbk != NULL) 
        {
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], (ELMO_PVOID)&error_id);
			}
			else
			{
				memcpy((ELMO_PVOID)&error_id, (ELMO_PVOID)&pBuff[MSG_DATA_INDX+2],2);
			}
            m_pfHomeEndEventClbk(axis_ref, error_id); //send axis ref and error
        }
    }
    break;
    case MODBUS_WRITE_EVT:
    {
        if (m_pfModbusWriteEventClbk != NULL) 
        {
            ELMO_UINT16 usDataLen;
            ELMO_UINT16 usSubType = 0;
			//
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[EMGCY_LEN_INDX], (ELMO_PVOID)&usDataLen);			
			}
			else
			{
				memcpy((ELMO_PVOID)&usDataLen, (ELMO_PVOID)&pBuff[EMGCY_LEN_INDX],2);
			}
            if(usDataLen == 6) // new message that contain modbus event subtype
            {
				if(m_ucDeviceType == GMAS_DEVICE)
				{
					NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+4], (ELMO_PVOID)&usSubType);
				}
				else
				{
					memcpy((ELMO_PVOID)&usSubType, (ELMO_PVOID)&pBuff[MSG_DATA_INDX+4],2);
				}
            }
            m_pfModbusWriteEventClbk(usSubType);
        }
        break ;
    }
    case TOUCH_PROBE_ENDED_EVT: 
    {
        if (m_pfTouchProbeEndCallback != NULL) 
        {
            ELMO_LINT32 lTouchProbePosition;
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[TOUCHP_POS_INDX], &lTouchProbePosition);
			}
			else
			{
				memcpy((ELMO_PVOID)&lTouchProbePosition, (ELMO_PVOID)&pBuff[TOUCHP_POS_INDX],4);
			}
            m_pfTouchProbeEndCallback( axis_ref,lTouchProbePosition);
        }
    }
    break ;

    case NODE_ERROR_EVT:
    {
        if (m_pfNodeErrorEventClbk != NULL) 
        {
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], (ELMO_PUINT16)&error_id);
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+6], (ELMO_PUINT16)&EmergencyCode);
			}
			else
			{
				memcpy((ELMO_PVOID)&error_id,     (ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], 2);
				memcpy((ELMO_PVOID)&EmergencyCode,(ELMO_PVOID)&pBuff[MSG_DATA_INDX+6], 2);
			}
            m_pfNodeErrorEventClbk(axis_ref,error_id,EmergencyCode); //send axis ref end OK or not
        }
    }
    break;

    case STOP_ON_LIMIT_EVT:
    {
        if (m_pfStopOnLimitEventClbk != NULL) 
        {
            ELMO_UINT32 uiStatusRegister;
            ELMO_UINT32 uiMcsLimitRegister;
            //
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], &error_id);
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+6], &uiStatusRegister);
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+10],&uiMcsLimitRegister);
			}
			else
			{
				memcpy((ELMO_PVOID)&error_id,			(ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], 2);
				memcpy((ELMO_PVOID)&uiStatusRegister,	(ELMO_PVOID)&pBuff[MSG_DATA_INDX+6], 4);
				memcpy((ELMO_PVOID)&uiMcsLimitRegister, (ELMO_PVOID)&pBuff[MSG_DATA_INDX+10],4);
			}
            m_pfStopOnLimitEventClbk(axis_ref,error_id,uiStatusRegister,uiMcsLimitRegister); //send axis ref end OK or not
        }

    }
    break;

    case TABLE_UNDERFLOW_EVT:
    {
        if (NULL != m_pfTableUnderflowEventClbk)
        {
            m_pfTableUnderflowEventClbk(axis_ref);
        }
    }
    break;
    case NODE_CONNECTED_EVT:
    {
        if (NULL != m_pfNodeConnectedEventClbk)
        {
            m_pfNodeConnectedEventClbk(axis_ref);
        }
    }
    break;
    case GLOBAL_ASYNC_REPLY_EVT:
    {
        if (NULL != m_pfGlobalAsyncReplyEventClbk)
        {
            ELMO_UINT8 ucMessageID;
            //
			if(m_ucDeviceType == GMAS_DEVICE)
			{
            NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX],   &status);
            NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], &error_id);
			}
			else
			{
				memcpy((ELMO_PVOID)&status,  (ELMO_PVOID)&pBuff[MSG_DATA_INDX],  2);
				memcpy((ELMO_PVOID)&error_id,(ELMO_PVOID)&pBuff[MSG_DATA_INDX+2],2);
			}
            ucMessageID = pBuff[MSG_DATA_INDX+4];
            m_pfGlobalAsyncReplyEventClbk(status,error_id,ucMessageID);
        }
    }
    break;
    case NODE_INIT_FINISHED_EVT:
    {
        if (NULL != m_pfNodeInitEventClbk)
        {
			if(m_ucDeviceType == GMAS_DEVICE)
			{
				NetToLocal((ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], &error_id);
			}
			else
			{
				memcpy((ELMO_PVOID)&error_id, (ELMO_PVOID)&pBuff[MSG_DATA_INDX+2],2);
			}
            m_pfNodeInitEventClbk(error_id,axis_ref);
        }
    }
    break;
    case FB_NOTIFICATION_EVT:
    {
        if (m_pfFBNotifyEventClbk != NULL)
        {
            ELMO_INT32 iEventCode;
			if(m_ucDeviceType == GMAS_DEVICE)
			{
            NetToLocal((ELMO_PVOID)&pBuff[EVENT_CODE_INDX], &iEventCode);
			}
			else
			{
				memcpy((ELMO_PVOID)&iEventCode, (ELMO_PVOID)&pBuff[EVENT_CODE_INDX], 4);
			}
            m_pfFBNotifyEventClbk(axis_ref, iEventCode);
        }
	}
	break ;
	case POLICY_ENDED_EVT:
		{
			if (m_pfPolicyEndedEventClbk != NULL)
			{
				if(m_ucDeviceType == GMAS_DEVICE)
				{
					NetToLocal((void *)&pBuff[MSG_DATA_INDX+2], &error_id);
				}
				else
				{
					memcpy((ELMO_PVOID)&error_id, (ELMO_PVOID)&pBuff[MSG_DATA_INDX+2], 2);
				}

				ELMO_UINT8 ucPolType = *((ELMO_PUINT8)&pBuff[14]);
				ELMO_UINT8 ucState   = *((ELMO_PUINT8)&pBuff[15]);
				ELMO_UINT8 ucErrType = *((ELMO_PUINT8)&pBuff[16]);

				m_pfPolicyEndedEventClbk(axis_ref, error_id,ucPolType,ucState,ucErrType);
			}
		}
    break ;
    default:
    break ;
    }
    //assigned only by ConnectIPCEx & ConnectRPCEx otherwise this method is not being called at all.
    //this method is called by CMMCCallbackFunc (above) which is set as callback to MMC api only by ConnectIPCEx & ConnectRPCEx
    if (m_fpClbk!=NULL)
    {
        return m_fpClbk(pBuff, sSize, pIPSock);
    }
    return 0;
}



//register event callbacks
void CMMCConnection::RegisterEventCallback(MMC_EVENT_ENUM eClbType, ELMO_PVOID pfClbk) {
    switch (eClbType) {
        case MMCPP_TOUCH_PROBE_ENDED:
        {
            m_pfTouchProbeEndCallback = (TouchProbeEndCallback)pfClbk;
        }
        break;
        case MMCPP_PDORCV:
        {
            m_pfPdoRcvEventClbk = (PdoRcvEventCallback)pfClbk;
        }
        break;
        case MMCPP_HBEAT:
        {
            m_pfHBeateEventClbk = (HBeateEventCallback)pfClbk;
        }
        break;
        case MMCPP_MOTIONENDED:
        {
            m_pfMotionEndEventClbk = (MotionEndEventCallback)pfClbk;
        }
        break;
        case MMCPP_EMCY:
        {
            m_pfEmergencyEventClbk = (EmergencyEventCallback)pfClbk;
        }
        break;
        case MMCPP_HOME_ENDED:
        {
            m_pfHomeEndEventClbk = (HomeEndedEventCallback)pfClbk;
        }
        break;
        case MMCPP_MODBUS_WRITE:
        {
            m_pfModbusWriteEventClbk = (ModbusWriteEventCallback)pfClbk;
        }
        break;
        case MMCPP_ASYNC_REPLY:
        {
            m_pfAsyncReplyEventClbk = (AsyncReplyEventCallback)pfClbk;
        }
        break;
        case MMCPP_NODE_ERROR:
        {
            m_pfNodeErrorEventClbk = (NodeErrorEventCallback)pfClbk;
        }
        break;
        case MMCPP_STOP_ON_LIMIT:
        {
            m_pfStopOnLimitEventClbk = (StopOnLimitEventCallback)pfClbk;
        }
        break;
        case MMCPP_TABLE_UNDERFLOW:
        {
            m_pfTableUnderflowEventClbk = (TableUnderflowEventCallback )pfClbk;
        }
        break;
        case MMCPP_NODE_CONNECTED:
        {
            m_pfNodeConnectedEventClbk = (NodeConnectedEventCallback )pfClbk;
        }
        break;
        case MMCPP_GLOBAL_ASYNC_REPLY:
        {
            m_pfGlobalAsyncReplyEventClbk = (GlobalAsyncReplyEventCallback )pfClbk;
        }
        break ;
        case MMCPP_NODE_INIT:
        {
            m_pfNodeInitEventClbk = (NodeInitEventCallback )pfClbk;
        }   
        break;
        case MMCPP_FB_NOTIFICATION:
        {
            m_pfFBNotifyEventClbk = (FBNotifyEventCallback )pfClbk;
        }
        break;
		case MMCPP_POLICY_ENDED:
		{
			m_pfPolicyEndedEventClbk = (PolicyEndedEventCallback )pfClbk;
		}
		break;
        default:
            break;
    }
}


void CMMCConnection::RegisterSyncTimerFunction(MMC_SYNC_TIMER_CB_FUNC func,ELMO_UINT16 usSYNCTimerTime)
{
    ELMO_INT32 rc;
    //
    if ((rc=MMC_CreateSYNCTimer(m_uiConnHndl,func,usSYNCTimerTime))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CreateSYNCTimer:", m_uiConnHndl, 0, rc, rc, 0);
    }
     _MMCPP_TRACE_INFO("MMC_CreateSYNCTimer OK");
}


void CMMCConnection::SetEventsMask(ELMO_UINT32 uiEventMask) throw (CMMCException)
{
    MMC_SETEVENTSMASK_IN stInParams;
    MMC_SETEVENTSMASK_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParams.iEventsMask = uiEventMask;
    if ((rc=MMC_SetEventsMaskCmd(m_uiConnHndl, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetEventsMaskCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetEventsMask OK");
    return;
}


void CMMCConnection::ClearEventsMask(ELMO_UINT32 uiEventMask) throw (CMMCException)
{
    MMC_CLEAREVENTSMASK_IN stInParams;
    MMC_CLEAREVENTSMASK_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParams.iEventsMask = uiEventMask;
    if ((rc=MMC_ClearEventsMaskCmd(m_uiConnHndl, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ClearEventsMaskCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("ClearEventsMask OK");
    return;
}


ELMO_UINT32 CMMCConnection::GetEventsMask() throw (CMMCException)
{
    MMC_GETEVENTSMASK_IN stInParams;
    MMC_GETEVENTSMASK_OUT stOutParam;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetEventsMaskCmd(m_uiConnHndl, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetEventsMaskCmd:", m_uiConnHndl, 0, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetEventsMask OK");
    return stOutParam.iEventsMask;
}

void CMMCConnection::GetMaestroTime(ELMO_UINT16 &year,   ELMO_UINT16 &month,  ELMO_UINT16 &day, ELMO_UINT16 &hour,
									ELMO_UINT16 &minute, ELMO_UINT16 &second, ELMO_UINT16 &milliSecond) throw (CMMCException)
{
	MMC_GET_MAESTRO_TIME_IN stInParams;
	MMC_GET_MAESTRO_TIME_OUT stOutParam;
	int rc;
	//
	if ((rc=MMC_GetMaestroTime(m_uiConnHndl, &stInParams, &stOutParam))!=0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetMaestroTime:", m_uiConnHndl, 0, rc, stOutParam.sErrorID, stOutParam.usStatus);
	}
    _MMCPP_TRACE_INFO("MMC_GetMaestroTime OK");

    month = stOutParam.usMonth;
	day = stOutParam.usDay;
	year= stOutParam.usYear;
	hour = stOutParam.usHour;
	minute = stOutParam.usMinute;
	second = stOutParam.usSecond;
	milliSecond = stOutParam.usMillisecond;
}

void CMMCConnection::SetMaestroTime(ELMO_UINT16   year, ELMO_UINT16 month,  ELMO_UINT16 day, ELMO_UINT16 hour,
									ELMO_UINT16 minute, ELMO_UINT16 second, ELMO_UINT16 milliSecond) throw (CMMCException)
{
	MMC_SET_MAESTRO_TIME_IN stInParams;
	MMC_SET_MAESTRO_TIME_OUT stOutParam;
	int rc;

	stInParams.usMonth = month;
	stInParams.usDay = day;
	stInParams.usYear = year;
	stInParams.usHour = hour;
	stInParams.usMinute = minute;
	stInParams.usSecond = second;
	stInParams.usMillisecond = milliSecond;

	if ((rc=MMC_SetMaestroTime(m_uiConnHndl, &stInParams, &stOutParam))!=0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetMaestroTime:", m_uiConnHndl, 0, rc, stOutParam.sErrorID, stOutParam.usStatus);
	}
    _MMCPP_TRACE_INFO("MMC_SetMaestroTime OK");
}
