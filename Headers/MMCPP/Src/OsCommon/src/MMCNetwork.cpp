/*
 * CMMCNetwork.cpp
 *
 *  Created on: 27/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCNetwork.hpp"

CMMCNetwork::CMMCNetwork() {}

CMMCNetwork::~CMMCNetwork() {}


////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ResetCommStatistics(
///             IN MMC_CONNECT_HNDL hConn,
///             OUT MMC_RESETCOMMSTATISTICS_OUT* pOutParam)
/// \brief This function reset all communication statistics.
/// \param  hConn - [IN] Connection handle
/// \param  pOutParam - [OUT] Pointer to Reset Communication Statistics output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCNetwork::ResetCommStatistics(MMC_RESETCOMMSTATISTICS_OUT& stOutParams) throw (CMMCException) {
    ELMO_INT32 rc;
    if ((rc=MMC_ResetCommStatistics(m_uiConnHndl, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetCommStatistics:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("ResetCommStatistics OK")
}


////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ResetCommDiagnostics(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_RESETCOMMDIAGNOSTICS_IN* pInParam,
///             OUT MMC_RESETCOMMDIAGNOSTICS_OUT* pOutParam)
/// \brief
/// \param  hConn - [IN] Connection handle
/// \param  pInParam - [IN] Pointer to MMC_ResetCommDiagnostics input parameters
/// \param  pOutParam - [OUT] Pointer to MMC_ResetCommDiagnostics output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////

void CMMCNetwork::ResetCommDiagnostics(MMC_RESETCOMMDIAGNOSTICS_OUT& stOutParams) throw (CMMCException) {
    MMC_RESETCOMMDIAGNOSTICS_IN stInParams;

    ELMO_INT32 rc;
    if ((rc=MMC_ResetCommDiagnostics(m_uiConnHndl, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ResetCommDiagnostics:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("ResetCommDiagnostics OK")
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_GetCommDiagnostics(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_GETCOMMDIAGNOSTICS_IN* pInParam,
///             OUT MMC_GETCOMMDIAGNOSTICS_OUT* pOutParam)
/// \brief
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle
/// \param  pInParam - [IN] Pointer to MMC_GetCommDiagnostics input parameters
/// \param  pOutParam - [OUT] Pointer to MMC_GetCommDiagnostics output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCNetwork::GetCommDiagnostics(MMC_GETCOMMDIAGNOSTICS_OUT& stOutParams) throw (CMMCException) {
    MMC_GETCOMMDIAGNOSTICS_IN stInParams;

    ELMO_INT32 rc;
    if ((rc=MMC_GetCommDiagnostics(m_uiConnHndl, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCommDiagnostics:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("GetCommDiagnostics OK");
}

/*! \fn int  GetCommStatistics(MMC_GETCOMMSTATISTICS_OUT& stOutParam)
* \brief This function receive communication statistics for specific axis..
* \param  stOutParam - [OUT] reference to retrieve communication statistics.
* \return   return - 0 on success otherwise throws CMMCException.
*/
ELMO_INT32 CMMCNetwork::GetCommStatistic(MMC_GETCOMMSTATISTICS_OUT& stOutParams) throw (CMMCException) {

    ELMO_INT32 rc;
    if ((rc=MMC_GetCommStatistics(m_uiConnHndl, 0, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetCommStatistics:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("GetNetworkInfo OK")
    return 0;
}

ELMO_INT32 CMMCNetwork::GetNetworkInfo(MMC_NETWORKINFO_OUT& stOutParams) throw (CMMCException) {
    MMC_NETWORKINFO_IN stInParams;

    ELMO_INT32 rc;
    if ((rc=MMC_NetworkInfoCmd(m_uiConnHndl, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetCommStatistics:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("GetNetworkInfo OK")
    return 0;
}
