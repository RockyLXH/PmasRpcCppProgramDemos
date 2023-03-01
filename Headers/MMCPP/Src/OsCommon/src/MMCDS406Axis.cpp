/*
 * MMCDS406.cpp
 *
 *  Created on: 27/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCDS406Axis.hpp"

CMMCDS406::CMMCDS406() {}

CMMCDS406::~CMMCDS406() {}



/*! \fn double mGetActualPosition()
 * \brief This function reads actual position of this axis.
 * \param ucEnable - default 1.
 * \return - actual position on success, otherwise throws CMMCException.
 */
ELMO_DOUBLE CMMCDS406::GetActualPosition() throw (CMMCException)
{
    MMC_READACTUALPOSITION_IN stInParam;
    MMC_READACTUALPOSITION_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadActualPositionCmd(m_uiConnHndl, m_usAxisRef, &stInParam,&stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadActualPositionCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetActualPosition OK")
    return stOutParam.dbPosition;
}


//
/*! \fn void ConfigGeneralTPDO3()
*   \brief This function configure to transmit general PDO3 message.
*   \return - void.
*/
void CMMCDS406::ConfigGeneralTPDO3(ELMO_UINT8 ucEventType)throw (CMMCException)
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
void CMMCDS406::ConfigGeneralTPDO4(ELMO_UINT8 ucEventType)throw (CMMCException)
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
void CMMCDS406::CancelGeneralTPDO3()throw (CMMCException)
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
void CMMCDS406::CancelGeneralTPDO4()throw (CMMCException)
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
