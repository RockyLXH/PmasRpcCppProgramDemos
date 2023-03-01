/*
 * CMMCHostComm.cpp
 *
 *  Created on: 24/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */


#include "OS_PlatformDependSetting.hpp"
#include "MMCHostComm.hpp"

void CMMCHostComm::MbusStartServer(MMC_CONNECT_HNDL uiConnHndl, ELMO_UINT16 usID) throw (CMMCException) {
    MMC_MODBUSSTARTSERVER_IN mbus_startserver_in;
    MMC_MODBUSSTARTSERVER_OUT mbus_startserver_out;
    MMC_MODBUSISRUNNING_IN mbus_IsRunningIn;
    MMC_MODBUSISRUNNING_OUT mbus_IsRunningOut;
    ELMO_INT32 rc;
    m_uiConnHndl = uiConnHndl;
    MMC_MbusIsRunning(uiConnHndl, &mbus_IsRunningIn, &mbus_IsRunningOut);
    if (mbus_IsRunningOut.isrunning == 0) {
        mbus_startserver_in.id = usID;
        if((rc=MMC_MbusStartServer(uiConnHndl, &mbus_startserver_in, &mbus_startserver_out))!=0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusStartServer:", m_uiConnHndl, 0, rc, mbus_startserver_out.usErrorID, mbus_startserver_out.usStatus);
        }
    }
    _MMCPP_TRACE_INFO("MbusStartServer OK");
}


void CMMCHostComm::MbusStopServer() throw (CMMCException) {
    MMC_MODBUSSTOPSERVER_IN mbus_stopserver_in;
    MMC_MODBUSSTOPSERVER_OUT mbus_stopserver_out;
    MMC_MODBUSISRUNNING_IN mbus_IsRunningIn;
    MMC_MODBUSISRUNNING_OUT mbus_IsRunningOut;
    ELMO_INT32 rc;
    MMC_MbusIsRunning(m_uiConnHndl, &mbus_IsRunningIn, &mbus_IsRunningOut);
    if (mbus_IsRunningOut.isrunning != 0) {
        if((rc=MMC_MbusStopServer(m_uiConnHndl, &mbus_stopserver_in, &mbus_stopserver_out))!=0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusStopServer:", m_uiConnHndl, 0, rc, mbus_stopserver_out.usErrorID, mbus_stopserver_out.usStatus);
        }
    }
    _MMCPP_TRACE_INFO("MbusStoptServer OK");
}

void CMMCHostComm::MbusWriteHoldingRegisterTable(MMC_MODBUSWRITEHOLDINGREGISTERSTABLE_IN& stInParams) throw (CMMCException)
{
    MMC_MODBUSWRITEHOLDINGREGISTERSTABLE_OUT mbus_write_out;
    ELMO_INT32 rc;
      if ((rc=MMC_MbusWriteHoldingRegisterTable(m_uiConnHndl, &stInParams, &mbus_write_out)) != 0) {
          CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusWriteHoldingRegisterTable:", m_uiConnHndl, 0, rc, mbus_write_out.usErrorID, mbus_write_out.usStatus);
      }
      _MMCPP_TRACE_INFO("MbusWriteHoldingRegisterTable OK");
}

void CMMCHostComm::MbusReadHoldingRegisterTable(ELMO_INT32 startRef, ELMO_INT32 refCnt, MMC_MODBUSREADHOLDINGREGISTERSTABLE_OUT& stOutParams) throw (CMMCException)
{
    MMC_MODBUSREADHOLDINGREGISTERSTABLE_IN mbus_read_in;

    mbus_read_in.startRef = startRef;
    mbus_read_in.refCnt = refCnt;

    ELMO_INT32 rc;
    if ((rc=MMC_MbusReadHoldingRegisterTable(m_uiConnHndl, &mbus_read_in, &stOutParams)) != 0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusReadHoldingRegisterTable:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MbusReadHoldingRegisterTable OK");
}

////////////////////////////////////////////////////////////////////////////////
/// \fn ELMO_INT32 MMC_MbusIsRunning (
///
///
/// \brief      This function stops the modbus server thread
/// \param  hConn - [IN] Connection handle.
/// \param  pInParam - [IN] Pointer to input parameters
/// \param  pOutParam - [OUT] Pointer output parameters including isrunning which equals 1, if modbus is running

/// \return return - 0 if failed
///                  error_id in case of error
///
////////////////////////////////////////////////////////////////////////////////
bool CMMCHostComm::MbusIsRunning(MMC_CONNECT_HNDL uiConnHndl) throw (CMMCException)
{
    MMC_MODBUSISRUNNING_IN stInParams;
    MMC_MODBUSISRUNNING_OUT stOutParams;

    ELMO_INT32 rc;
    MMC_CONNECT_HNDL uiTempConnHndl ;

    //
    // Due to bug Oren found. User cannot just ask whether the modbus is running
    // he must call start. He has the ability to enquire on parameter.
    uiTempConnHndl = (uiConnHndl != NULL) ? uiConnHndl : m_uiConnHndl ;
    if ((rc=MMC_MbusIsRunning(uiTempConnHndl, &stInParams, &stOutParams)) != 0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusIsRunning:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    } else {
        /*
        All API of a class instance (an object) must use the same connection handle.
        connection handle is initialized by MbusStartServer.
        If it is already started, connection handler may not be initialized @YL 20/10/2013
         */
        m_uiConnHndl = uiConnHndl;
    }
    _MMCPP_TRACE_INFO("MbusIsRunning OK");
    return (stOutParams.isrunning != 0);
}

//
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_MbusReadCoilsTable (
///
///
/// \brief      This function reads part of modbus register table
/// \param  hConn - [IN] Connection handle.
/// \param  pInParam - [IN] Pointer to modbus holding reference count and number of parameters
/// \param  pOutParam - [OUT] Pointer to modbus holding coils table array and output parameters

/// \return return - 0 if failed
///                  error_id in case of error
///
////////////////////////////////////////////////////////////////////////////////
void CMMCHostComm::MbusReadCoilsTable(ELMO_INT32 startRef, ELMO_INT32 refCnt, MMC_MODBUSREADCOILS_OUT& stOutParams) throw (CMMCException)
{
    MMC_MODBUSREADCOILS_IN stInParams;
    ELMO_INT32 rc;
    stInParams.refCnt = refCnt;
    stInParams.startRef = startRef;

    if ((rc=MMC_MbusReadCoilsTable(m_uiConnHndl, &stInParams, &stOutParams)) != 0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusReadCoilsTable:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MbusReadCoilsTable OK");
}
//
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_MbusWriteCoilsTable (
///
///
/// \brief      This function writes part of modbus register table
/// \param  hConn - [IN] Connection handle.
/// \param  pInParam - [IN] Pointer to modbus holding coils table input parameters
/// \param  pOutParam - [OUT] Pointer to output parameters

/// \return return - 0 if failed
///                  error_id in case of error
///
////////////////////////////////////////////////////////////////////////////////
void CMMCHostComm::MbusWriteCoilsTable(MMC_MODBUSWRITECOILS_IN& stInParams) throw (CMMCException)
{
    MMC_MODBUSWRITECOILS_OUT stOutParams;
    ELMO_INT32 rc;

    if ((rc=MMC_MbusWriteCoilsTable(m_uiConnHndl, &stInParams, &stOutParams)) != 0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusWriteCoilsTable:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MbusWriteCoilsTable OK");
}

//
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_MbusReadInputsTable (
///
///
/// \brief      This function writes part of modbus register table
/// \param  hConn - [IN] Connection handle.
/// \param  pInParam - [IN] Pointer to modbus input parameters, start reference, and reference count
/// \param  pOutParam - [OUT] Pointer to output parameters, including inputs table parameters

/// \return return - 0 if failed
///                  error_id in case of error
///
////////////////////////////////////////////////////////////////////////////////
void CMMCHostComm::MbusReadInputsTable(ELMO_INT32 startRef, ELMO_INT32 refCnt, MMC_MODBUSREADINPUTS_OUT& stOutParams) throw (CMMCException)
{
    MMC_MODBUSREADINPUTS_IN stInParams;

    ELMO_INT32 rc;

    stInParams.refCnt = refCnt;
    stInParams.startRef = startRef;
    if ((rc=MMC_MbusReadInputsTable(m_uiConnHndl, &stInParams, &stOutParams)) != 0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MbusReadInputsTable:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MbusReadInputsTable OK");
}
//
void CMMCHostComm::SetModbus(ELMO_INT32 iOffset, ELMO_LINT32 lValue) throw (CMMCException) {
    MMC_MODBUSWRITEHOLDINGREGISTERSTABLE_IN stInParams;
    stInParams.refCnt = 2;
    stInParams.startRef = iOffset;
    LongToModbusShortArr(&stInParams.regArr[iOffset*2],  lValue) ;
    MbusWriteHoldingRegisterTable(stInParams);
    _MMCPP_TRACE_INFO("SetModbus OK");
}
void CMMCHostComm::SetModbus(ELMO_INT32 iOffset, ELMO_FLOAT fValue) throw (CMMCException) {
    MMC_MODBUSWRITEHOLDINGREGISTERSTABLE_IN stInParams;
    stInParams.refCnt = 2;
    stInParams.startRef = iOffset;
    FloatToModbusShortArr(&stInParams.regArr[iOffset*2],  fValue) ;
    MbusWriteHoldingRegisterTable(stInParams);
    _MMCPP_TRACE_INFO("SetModbus OK");
}

void CMMCHostComm::SetModbus(ELMO_INT32 iOffset, ELMO_INT16 sValue) throw (CMMCException) {
    MMC_MODBUSWRITEHOLDINGREGISTERSTABLE_IN stInParams;
    stInParams.refCnt = 1;
    stInParams.startRef = iOffset;
    stInParams.regArr[iOffset] = sValue;
    MbusWriteHoldingRegisterTable(stInParams);
    _MMCPP_TRACE_INFO("SetModbus OK");
}


void CMMCHostComm::SetModbus(ELMO_INT32 iOffset, ELMO_INT32 iRefCount, CMMCModbusSwapBuffer& stInParams) throw (CMMCException) {
    stInParams.SetOffset(iOffset);
    stInParams.SetRefCount(iRefCount);
    MbusWriteHoldingRegisterTable(stInParams.GetBuffer());
    _MMCPP_TRACE_INFO("SetModbus OK");
}

void CMMCHostComm::SetModbus(ELMO_INT32 iOffset, ELMO_INT32 iRefCount, CMMCModbusBuffer& stInParams) throw (CMMCException) {
    stInParams.SetOffset(iOffset);
    stInParams.SetRefCount(iRefCount);
    MbusWriteHoldingRegisterTable(stInParams.GetBuffer());
    _MMCPP_TRACE_INFO("SetModbus OK");
}


void CMMCHostComm::GetModbus(ELMO_INT32 iOffset, ELMO_INT32 iRefCount, CMMCModbusBuffer& stOutParams) throw (CMMCException)
{
    MMC_MODBUSREADHOLDINGREGISTERSTABLE_OUT _stOutParams;
    MbusReadHoldingRegisterTable(iOffset, iRefCount, _stOutParams);
    for (ELMO_INT32 i = 0;i<iRefCount;i++) {
          stOutParams.TypeToModbusShortArr(i, _stOutParams.regArr[i]);
    }
    _MMCPP_TRACE_INFO("GetModbus OK");
}

void CMMCHostComm::GetModbus(ELMO_INT32 iOffset, ELMO_INT32 iRefCount, CMMCModbusSwapBuffer& stOutParams) throw (CMMCException) {
    MMC_MODBUSREADHOLDINGREGISTERSTABLE_OUT _stOutParams;
    MbusReadHoldingRegisterTable(iOffset, iRefCount, _stOutParams);

    for (ELMO_INT32 i = 0;i<iRefCount;i++) {
          stOutParams.TypeToModbusShortArr(i, _stOutParams.regArr[i]);
    }

    _MMCPP_TRACE_INFO("GetModbus OK");
}
