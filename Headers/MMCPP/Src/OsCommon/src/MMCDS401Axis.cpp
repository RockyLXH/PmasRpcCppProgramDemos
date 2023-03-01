/*
 * CMMCDS401Axis.cpp
 *
 *  Created on: 27/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCDS401Axis.hpp"

CMMCDS401Axis::CMMCDS401Axis(){}

CMMCDS401Axis::~CMMCDS401Axis() {}


////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_EnableDIChangedEvent(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_CONFIGREGULARPARAMEVENTPDO3_IN* pInParam,
///             OUT MMC_CONFIGREGULARPARAMEVENTPDO3_OUT* pOutParam)
////////////////////////////////////////////////////////////////////////////////
void CMMCDS401Axis::EnableDS401DIChangedEvent() throw (CMMCException) {
    MMC_ENABLEDICHANGEDEVENT_IN stInParams;
    MMC_ENABLEDICHANGEDEVENT_OUT stOutParams;
    ELMO_INT32 rc;
    if ((rc = MMC_EnableDS401DIChangedEvent(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_EnableDS401DIChangedEvent:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("EnableDS401DIChangedEvent: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_DisableDIChangedEvent(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_CONFIGREGULARPARAMEVENTPDO3_IN* pInParam,
///             OUT MMC_CONFIGREGULARPARAMEVENTPDO3_OUT* pOutParam)
////////////////////////////////////////////////////////////////////////////////
void CMMCDS401Axis::DisableDS401DIChangedEvent() throw (CMMCException) {
    MMC_DISABLEDICHANGEDEVENT_IN stInParams;
    MMC_DISABLEDICHANGEDEVENT_OUT stOutParams;
    ELMO_INT32 rc;
    if ((rc = MMC_DisableDS401DIChangedEvent(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_DisableDS401DIChangedEvent: ",m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("DisableDS401DIChangedEvent: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_ReadDS401DInput(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_READDI_IN* pInParam,
///             OUT MMC_READDI_OUT* pOutParam)
////////////////////////////////////////////////////////////////////////////////
MMCPPULL_T CMMCDS401Axis::ReadDS401DInput() throw (CMMCException) {
    MMC_READDI_IN stInParams;
    MMC_READDI_OUT stOutParams;
    ELMO_INT32 rc;
    if ((rc = MMC_ReadDS401DInput(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadDS401DInput: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ReadDS401DInput: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return stOutParams.ulliDI;
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_WriteDS401DOutput(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_WRITEDO_IN* pInParam,
///             OUT MMC_WRITEDO_OUT* pOutParam)
////////////////////////////////////////////////////////////////////////////////
void CMMCDS401Axis::WriteDS401DOutput(MMCPPULL_T ulliDO) throw (CMMCException) {
    MMC_WRITEDO_IN stInParams;
    MMC_WRITEDO_OUT stOutParams;
    ELMO_INT32 rc;
    stInParams.ulliDO = ulliDO;
    if ((rc = MMC_WriteDS401DOutput(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteDS401DOutput: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("WriteDS401DOutput: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
}
//
/*! \fn void ConfigGeneralRPDO3()
*   \brief This function configure to receive general PDO3 message.
*   \return - void.
*/
void CMMCDS401Axis::ConfigGeneralRPDO3(ELMO_UINT8 ucEventType,ELMO_UINT8 ucPDOCommParam, ELMO_UINT8 ucPDOLength)throw (CMMCException)
{
    MMC_CONFIGGENERALRPDO3_IN stInParams;
    MMC_CONFIGGENERALRPDO3_OUT stOutParams;
    ELMO_INT32 rc;
    //
    stInParams.ucEventType      =   ucEventType;
    stInParams.ucPDOCommParam   =   ucPDOCommParam;
    stInParams.ucPDOLength      =   ucPDOLength;
    //
    if ((rc = MMC_ConfigGeneralRPDO3(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigGeneralRPDO3: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("ConfigGeneralRPDO3 OK")
    return;
}

//
/*! \fn void ConfigGeneralRPDO4()
*   \brief This function configure to receive general PDO4 message.
*   \return - void.
*/
void CMMCDS401Axis::ConfigGeneralRPDO4(ELMO_UINT8 ucEventType,ELMO_UINT8 ucPDOCommParam, ELMO_UINT8 ucPDOLength)throw (CMMCException)
{
    MMC_CONFIGGENERALRPDO4_IN stInParams;
    MMC_CONFIGGENERALRPDO4_OUT stOutParams;
    ELMO_INT32 rc;
    //
    stInParams.ucEventType      =   ucEventType;
    stInParams.ucPDOCommParam   =   ucPDOCommParam;
    stInParams.ucPDOLength      =   ucPDOLength;
    //
    if ((rc = MMC_ConfigGeneralRPDO4(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigGeneralRPDO4: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("ConfigGeneralRPDO4 OK")
    return;
}

//
/*! \fn void CancelGeneralRPDO3()
*   \brief This function cancel the configuration of receiving general PDO3 message.
*   \return - void.
*/
void CMMCDS401Axis::CancelGeneralRPDO3()throw (CMMCException)
{
    MMC_CANCELGENERALRPDO3_IN stInParams;
    MMC_CANCELGENERALRPDO3_OUT stOutParams;
    ELMO_INT32 rc;
    //
    if ((rc = MMC_CancelGeneralRPDO3(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelGeneralRPDO3: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("CancelGeneralRPDO3 OK")
    return;
}

//
/*! \fn void CancelGeneralRPDO4()
*   \brief This function cancle the configuration of receiving general PDO4 message.
*   \return - void.
*/
void CMMCDS401Axis::CancelGeneralRPDO4()throw (CMMCException)
{
    MMC_CANCELGENERALRPDO4_IN stInParams;
    MMC_CANCELGENERALRPDO4_OUT stOutParams;
    ELMO_INT32 rc;
    //
    if ((rc = MMC_CancelGeneralRPDO4(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelGeneralRPDO4: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("CancelGeneralRPDO4 OK")
    return;
}

//
/*! \fn void ConfigGeneralTPDO3()
*   \brief This function configure to transmit general PDO3 message.
*   \return - void.
*/
void CMMCDS401Axis::ConfigGeneralTPDO3(ELMO_UINT8 ucEventType)throw (CMMCException)
{
    MMC_CONFIGGENERALTPDO3_IN stInParams;
    MMC_CONFIGGENERALTPDO3_OUT stOutParams;
    ELMO_INT32 rc;
    //
    stInParams.ucEventType      =   ucEventType;
    //
    if ((rc = MMC_ConfigGeneralTPDO3(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigGeneralTPDO3: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("ConfigGeneralTPDO3 OK")
    return;
}

//
/*! \fn void ConfigGeneralTPDO4()
*   \brief This function configure to transmit general PDO4 message.
*   \return - void.
*/
void CMMCDS401Axis::ConfigGeneralTPDO4(ELMO_UINT8 ucEventType)throw (CMMCException)
{
    MMC_CONFIGGENERALTPDO4_IN stInParams;
    MMC_CONFIGGENERALTPDO4_OUT stOutParams;
    ELMO_INT32 rc;
    //
    stInParams.ucEventType      =   ucEventType;
    //
    if ((rc = MMC_ConfigGeneralTPDO4(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigGeneralTPDO4: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("ConfigGeneralTPDO4 OK")
    return;
}

//
/*! \fn void CancelGeneralTPDO3()
*   \brief This function cancle the configuration of transmitting general PDO3 message.
*   \return - void.
*/
void CMMCDS401Axis::CancelGeneralTPDO3()throw (CMMCException)
{
    MMC_CANCELGENERALTPDO3_IN stInParams;
    MMC_CANCELGENERALTPDO3_OUT stOutParams;
    ELMO_INT32 rc;
    //
    if ((rc = MMC_CancelGeneralTPDO3(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelGeneralTPDO3: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("CancelGeneralTPDO3 OK")
    return;
}

//
/*! \fn void CancelGeneralTPDO4()
*   \brief This function cancle the configuration of transmitting general PDO3 message.
*   \return - void.
*/
void CMMCDS401Axis::CancelGeneralTPDO4()throw (CMMCException)
{
    MMC_CANCELGENERALTPDO4_IN stInParams;
    MMC_CANCELGENERALTPDO4_OUT stOutParams;
    ELMO_INT32 rc;
    //
    if ((rc = MMC_CancelGeneralTPDO4(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelGeneralTPDO4: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("CancelGeneralTPDO4 OK")
    return;
}
