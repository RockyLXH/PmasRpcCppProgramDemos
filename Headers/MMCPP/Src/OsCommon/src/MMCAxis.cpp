/*
 * CMMCAxis.cpp
 *
 *  Created on: 10/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMC_definitions.h"
#include "MMCAxis.hpp"


/**
 * copy constructor
 */
CMMCAxis::CMMCAxis(CMMCAxis& axis)
{
    m_uiConnHndl = axis.m_uiConnHndl;
    SetName(axis.m_strName);
    m_usAxisRef = axis.m_usAxisRef;
}


ELMO_INT32 CMMCAxis::GetAxisByName(const ELMO_INT8*  cName) throw (CMMCException)
{
    ELMO_INT32 rc = -1;
    MMC_AXISBYNAME_IN   axis_by_name_in;
    MMC_AXISBYNAME_OUT  axis_by_name_out;

    strncpy(axis_by_name_in.cAxisName, cName, NODE_NAME_MAX_LENGTH);
    axis_by_name_in.cAxisName[NODE_NAME_MAX_LENGTH-1] = 0;

    if ((rc = MMC_GetAxisByNameCmd(m_uiConnHndl, &axis_by_name_in, &axis_by_name_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetAxisByName:", m_uiConnHndl, m_usAxisRef, rc, axis_by_name_out.usErrorID, axis_by_name_out.usStatus);
    }
    else
    {
        rc = (int)axis_by_name_out.usAxisIdx;
    }
    SetRef(axis_by_name_out.usAxisIdx);
    SetName(cName);

    _MMCPP_TRACE("MMC_GetAxisByName: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, axis_by_name_out.usErrorID, axis_by_name_out.usStatus);
    return (rc);
}

void CMMCAxis::SetName(const ELMO_INT8* cName) { strcpy(m_strName, cName); }


/*! \fn unsigned int SetDefaultManufacturerParameters()
* \brief This function restore the axis\group parameters
* \param
* \return
*/
void CMMCAxis::SetDefaultManufacturerParameters() throw (CMMCException)
{
    MMC_SETDEFAULTPARAMETERS_IN stInParams;
    MMC_SETDEFAULTPARAMETERS_OUT stOutParams;
    //
    int rc = 0;
    rc = MMC_SetDefaultParametersCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetDefaultParametersCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("SetDefaultParametersCmd OK")
    //
    return;
}


ELMO_UINT32 CMMCAxis::GetSystemCounter() throw (CMMCException)
{
    MMC_GETCYCLESCOUNTER_IN stInParams;
    MMC_GETCYCLESCOUNTER_OUT stOutParams;
    //
    ELMO_INT32 rc = 0;
    rc = MMC_GetCyclesCounterCmd(m_uiConnHndl,&stInParams,&stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetCyclesCounterCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("GetSystemCounter OK")
    //
    return stOutParams.ulCyclesCounter;
}
