/*
 * CMMCGroupAxis.cpp
 *
 *  Created on: 10/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *      Update: 06Jul2017 HH
 *                  Methode: int CMMCGroupAxis::SetCartesianTransform(..)
 *                  add the line:
 *                      stInParams.eRotAngleUnits = eRotAngleUnits;
 *              0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"

#include "MMCGroupAxis.hpp"

/*
 * warnings caused by methods, which are  themselves defined as deprecation.
 * (see SetCartesianKinematics & SetDeltaRobotKinematics)
 */

#ifdef PROAUT_CHANGES
    //save compiler switches
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif



MMC_MOTIONPARAMS_GROUP::MMC_MOTIONPARAMS_GROUP()
{
    memset(dAuxPoint, 0, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memset(dEndPoint, 0, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memset(fTransitionParameter, 0, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    fVelocity = 0;
    fAcceleration = 0;
    fDeceleration = 0;
    fJerk = 0;
    eCoordSystem = MC_NONE_COORD;
    eTransitionMode = MC_TM_NONE_MODE;
    ucSuperimposed = 0;
    m_uiExecDelayMs = 0;
    ucExecute = 1;
    eArcShortLong = MC_NONE_ARC_CHOICE;
    ePathChoice = MC_NONE_PATH_CHOICE;
    eCircleMode = MC_NONE_CIRC_MODE;
}



CMMCGroupAxis::CMMCGroupAxis() 
{
    memset(m_dAuxPoint, 0, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memset(m_dEndPoint, 0, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memset(m_fTransitionParameter, 0, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    m_fVelocity = 100000;
    m_fAcceleration = 1000000;
    m_fDeceleration = 1000000;
    m_fJerk = 20000000;
    m_eCoordSystem = MC_NONE_COORD;
    m_eTransitionMode = MC_TM_NONE_MODE;
    m_eArcShortLong = MC_NONE_ARC_CHOICE;
    m_ePathChoice = MC_NONE_PATH_CHOICE;
    m_eCircleMode = MC_NONE_CIRC_MODE;
    m_ucSuperimposed = 0;
    m_uiExecDelayMs = 0;
    m_ucExecute = 1;
}

/**
 * copy constructor
 */
CMMCGroupAxis::CMMCGroupAxis(CMMCGroupAxis& axis):CMMCMotionAxis(axis)
{
    memcpy(m_dAuxPoint, axis.m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, axis.m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_fTransitionParameter, axis.m_fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    m_fVelocity = axis.m_fVelocity;
    m_fAcceleration = axis.m_fAcceleration;
    m_fDeceleration = axis.m_fDeceleration;
    m_fJerk = axis.m_fJerk;
    m_eCoordSystem = axis.m_eCoordSystem;
    m_eTransitionMode = axis.m_eTransitionMode;
    m_eArcShortLong = axis.m_eArcShortLong;
    m_ePathChoice = axis.m_ePathChoice;
    m_eCircleMode = axis.m_eCircleMode;
    m_ucSuperimposed = axis.m_ucSuperimposed;
    m_uiExecDelayMs = axis.m_uiExecDelayMs;
    m_ucExecute = axis.m_ucExecute;
}

CMMCGroupAxis::~CMMCGroupAxis() {}

void CMMCGroupAxis::SetDefaultParams(const MMC_MOTIONPARAMS_GROUP& stGroupAxisParams) throw (CMMCException) {
    memcpy(m_dAuxPoint, stGroupAxisParams.dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, stGroupAxisParams.dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_fTransitionParameter, stGroupAxisParams.fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    m_eCoordSystem = stGroupAxisParams.eCoordSystem;
    m_eTransitionMode = stGroupAxisParams.eTransitionMode;
    m_fAcceleration = stGroupAxisParams.fAcceleration;
    m_fDeceleration = stGroupAxisParams.fDeceleration;
    m_fJerk = stGroupAxisParams.fJerk;
    m_fVelocity = stGroupAxisParams.fVelocity;
    m_ucExecute = stGroupAxisParams.ucExecute;
    m_ucSuperimposed = stGroupAxisParams.ucSuperimposed;
    m_uiExecDelayMs = stGroupAxisParams.m_uiExecDelayMs;
    m_eArcShortLong = stGroupAxisParams.eArcShortLong;
    m_ePathChoice = stGroupAxisParams.ePathChoice;
    m_eCircleMode = stGroupAxisParams.eCircleMode;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_SetKinTransform(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_SETKINTRANSFORM_IN* pInParam,
///             OUT MMC_SETKINTRANSFORM_OUT* pOutParam)
/// \brief This function set group's kinematic transformation parameter.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Set Kinematic Transform input parameters
/// \param  pOutParam - [OUT] Pointer to Set Kinematic Transform output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::SetKinTransform(MMC_SETKINTRANSFORM_IN& stInParam) throw (CMMCException) {
    //MMC_SETKINTRANSFORM_IN stInParam;
    MMC_SETKINTRANSFORM_OUT stOutParam;
    ELMO_INT32 rc;
    if ((rc = MMC_SetKinTransform(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetKinTransform failed", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("pSetKinTransform: %d, %s, %d, %d, %d\n", rc, GetName(), GetRef(), stOutParam.usErrorID, stOutParam.usStatus);
}
/*! \fn RemoveAxisFromGroup(NC_IDENT_IN_GROUP_ENUM eIdentInGroup)
 * \brief This function remove axis from axis group.
 * \param  eIdentInGroup - group identifier.
 * \return - none on success, otherwise throws CMMCException
 */
void CMMCGroupAxis::RemoveAxisFromGroup(NC_IDENT_IN_GROUP_ENUM eIdentInGroup) throw (CMMCException) {
    MMC_REMOVEAXISFROMGROUP_IN stInParam;
    MMC_REMOVEAXISFROMGROUP_OUT stOutParam;
    ELMO_INT32 rc;

    stInParam.eIdentInGroup = eIdentInGroup;
    if ((rc = MMC_RemoveAxisFromGroup(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_RemoveAxisFromGroup: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("pRemoveAxisFromGroup: %d, %s, %d, %d, %d\n", rc, GetName(), GetRef(), stOutParam.usErrorID, stOutParam.usStatus);
}

void CMMCGroupAxis::CopyMoveCircularAbsBorderParams(MMC_MOVECIRCULARABSOLUTEBORDER_IN& m_stInParams) {
    memcpy(m_stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT)*NC_MAX_NUM_AXES_IN_NODE);
    m_stInParams.eCoordSystem = m_eCoordSystem;
    m_stInParams.eTransitionMode = m_eTransitionMode;
    m_stInParams.fAcceleration = m_fAcceleration;
    m_stInParams.fDeceleration = m_fDeceleration;
    m_stInParams.fJerk = m_fJerk;
    m_stInParams.fVelocity = m_fVelocity;
    m_stInParams.ucExecute = m_ucExecute;
    m_stInParams.ucSuperimposed = m_ucSuperimposed;
}

void CMMCGroupAxis::CopyMoveCircularAbsParams(MMC_MOVECIRCULARABSOLUTE_IN& m_stInParams) {
    memcpy(m_stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT)*NC_MAX_NUM_AXES_IN_NODE);
    m_stInParams.eCoordSystem = m_eCoordSystem;
    m_stInParams.eTransitionMode = m_eTransitionMode;
    m_stInParams.fAcceleration = m_fAcceleration;
    m_stInParams.fDeceleration = m_fDeceleration;
    m_stInParams.fJerk = m_fJerk;
    m_stInParams.fVelocity = m_fVelocity;
    m_stInParams.ucExecute = m_ucExecute;
    m_stInParams.ucSuperimposed = m_ucSuperimposed;
    m_stInParams.eArcShortLong = m_eArcShortLong;
    m_stInParams.ePathChoice = m_ePathChoice;
    m_stInParams.eCircleMode = m_eCircleMode;
}


void CMMCGroupAxis::CopyMoveCircularAbsCenterParams(MMC_MOVECIRCULARABSOLUTECENTER_IN& stInParams) {
    memcpy(stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT)*NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
    stInParams.eArcShortLong = m_eArcShortLong;
}

void CMMCGroupAxis::CopyMoveCircularAbsAngleParams(MMC_MOVECIRCULARABSOLUTEANGLE_IN& stInParams)
{
    memcpy(stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
}

void CMMCGroupAxis::CopyMoveCircularAbsRadiusParams(MMC_MOVECIRCULARABSOLUTERADIUS_IN& stInParams)
{
    memcpy(stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
    stInParams.eArcShortLong = m_eArcShortLong;
    stInParams.ePathChoice = m_ePathChoice;
}


void CMMCGroupAxis::CopyMoveLinearAbsParams(MMC_MOVELINEARABSOLUTE_IN& stInParams)
{
    memcpy(stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
}

void CMMCGroupAxis::CopyMoveLinearRltParams(MMC_MOVELINEARRELATIVE_IN& stInParams)
{
    memcpy(stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
}

void CMMCGroupAxis::CopyMoveLinearAddParams(MMC_MOVELINEARADDITIVE_IN& stInParams)
{
    memcpy(stInParams.fTransitionParameter, m_fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
}

void CMMCGroupAxis::CopyMoveLinearAbsRepParams(MMC_MOVELINEARABSOLUTEREPETITIVE_IN& stInParams)
{
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.uiExecDelayMs = m_uiExecDelayMs;
    stInParams.ucSuperImposed = m_ucSuperimposed;
}
void CMMCGroupAxis::CopyMoveLinearRltRepParams(MMC_MOVELINEARRELATIVEREPETITIVE_IN& stInParams)
{
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.fVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.uiExecDelayMs = m_uiExecDelayMs;
    stInParams.ucSuperImposed = m_ucSuperimposed;
}

void CMMCGroupAxis::CopyMovePolynomAbsParams(MMC_MOVEPOLYNOMABSOLUTE_IN& stInParams)
{
    stInParams.eCoordSystem = m_eCoordSystem;
    stInParams.dAcceleration = m_fAcceleration;
    stInParams.dDeceleration = m_fDeceleration;
    stInParams.dJerk = m_fJerk;
    stInParams.dVelocity = m_fVelocity;
    stInParams.ucExecute = m_ucExecute;
    stInParams.ucSuperimposed = m_ucSuperimposed;
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsolute(
    NC_ARC_SHORT_LONG_ENUM eArcShortLong,
    NC_PATH_CHOICE_ENUM ePathChoice,
    NC_CIRC_MODE_ENUM eCircleMode, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {

    MMC_MOVECIRCULARABSOLUTE_OUT stOutParam;
    MMC_MOVECIRCULARABSOLUTE_IN stInParams;

    ELMO_INT32 rc;
    m_eArcShortLong = eArcShortLong;
    m_ePathChoice = ePathChoice;
    m_eCircleMode = eCircleMode;
    stInParams.eBufferMode = eBufferMode;
    memcpy(stInParams.dAuxPoint, m_dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dEndPoint, m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    CopyMoveCircularAbsParams(stInParams);
    if ((rc = MMC_MoveCircularAbsoluteCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveCircularAbsoluteCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("MMC_MoveCircularAbsoluteCmd: %d, %s, %d, %d, %d\n", rc, GetName(), GetRef(), stOutParam.usErrorID, stOutParam.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsolute(
    NC_ARC_SHORT_LONG_ENUM eArcShortLong,
    NC_PATH_CHOICE_ENUM ePathChoice,
    NC_CIRC_MODE_ENUM eCircleMode, ELMO_DOUBLE dAuxPoint[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    //
    memcpy(m_dAuxPoint, dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsolute(eArcShortLong,ePathChoice,eCircleMode,eBufferMode);
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsolute(
    NC_ARC_SHORT_LONG_ENUM eArcShortLong,
    NC_PATH_CHOICE_ENUM ePathChoice,
    NC_CIRC_MODE_ENUM eCircleMode,
    ELMO_DOUBLE dAuxPoint[NC_MAX_NUM_AXES_IN_NODE], ELMO_DOUBLE dEndPoint[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    //
    memcpy(m_dAuxPoint, dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsolute(eArcShortLong,ePathChoice,eCircleMode,eBufferMode);
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteCenter(NC_ARC_SHORT_LONG_ENUM eArcShortLong, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVECIRCULARABSOLUTE_OUT stOutParams;
    MMC_MOVECIRCULARABSOLUTECENTER_IN stInParams;

    memcpy(stInParams.dCenterPoint, m_dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dEndPoint, m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    m_eArcShortLong = eArcShortLong;
    stInParams.eBufferMode= eBufferMode;
    CopyMoveCircularAbsCenterParams(stInParams);
    ELMO_INT32 rc;
    if ((rc = MMC_MoveCircularAbsoluteCenterCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveCircularAbsoluteCenterCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveCircularAbsoluteCenter: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteCenter(NC_ARC_SHORT_LONG_ENUM eArcShortLong, ELMO_DOUBLE dCenterPoint[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    //
    memcpy(m_dAuxPoint, dCenterPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsoluteCenter(eArcShortLong, eBufferMode);
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteCenter(NC_ARC_SHORT_LONG_ENUM eArcShortLong, ELMO_DOUBLE dCenterPoint[NC_MAX_NUM_AXES_IN_NODE], ELMO_DOUBLE dEndPoint[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    //
    memcpy(m_dAuxPoint, dCenterPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsoluteCenter(eArcShortLong, eBufferMode);
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_MoveCircularAbsoluteBorderCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_MOVECIRCULARABSOLUTEBORDER_IN* pInParam,
///             OUT MMC_MOVECIRCULARABSOLUTE_OUT* pOutParam)
/// \brief This function This function  send Move Circular Absolute Border command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Move Circular Absolute Border input parameters
/// \param  pOutParam - [OUT] Pointer to Move Circular Absolute output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteBorder(ELMO_DOUBLE dBorderPoint[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVECIRCULARABSOLUTEBORDER_IN stInParams;
    MMC_MOVECIRCULARABSOLUTE_OUT stOutParams;
    memcpy(m_dAuxPoint, dBorderPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dBorderPoint, dBorderPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dEndPoint, m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eBufferMode = eBufferMode;
    CopyMoveCircularAbsBorderParams(stInParams);
    ELMO_INT32 rc;
    if ((rc = MMC_MoveCircularAbsoluteBorderCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveCircularAbsoluteBorderCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveCircularAbsoluteBorder: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteBorder(ELMO_DOUBLE dBorderPoint[NC_MAX_NUM_AXES_IN_NODE], ELMO_DOUBLE dEndPoint[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    //
    memcpy(m_dEndPoint, dEndPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsoluteBorder(dBorderPoint, eBufferMode);
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_MoveCircularAbsoluteRadiusCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_MOVECIRCULARABSOLUTERADIUS_IN* pInParam,
///             OUT MMC_MOVECIRCULARABSOLUTE_OUT* pOutParam)
/// \brief This function This function  send Move Circular Absolute Radius command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Move Circular Absolute Radius input parameters
/// \param  pOutParam - [OUT] Pointer to Move Circular Absolute output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteRadius(
        NC_ARC_SHORT_LONG_ENUM eArcShortLong,
        NC_PATH_CHOICE_ENUM ePathChoice,
        MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {

    MMC_MOVECIRCULARABSOLUTERADIUS_IN stInParams;
    MMC_MOVECIRCULARABSOLUTE_OUT stOutParams;

    memcpy(stInParams.dSpearHeadPoint, m_dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dEndPoint, m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    m_eArcShortLong = eArcShortLong;
    m_ePathChoice = ePathChoice;
    stInParams.eBufferMode = eBufferMode;
    ELMO_INT32 rc;
	
	// 15.10.2015 - Bug fix - Added a call to CopyMoveCircularAbsRadiusParams function.
	CopyMoveCircularAbsRadiusParams(stInParams);
	if ((rc = MMC_MoveCircularAbsoluteRadiusCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveCircularAbsoluteRadiusCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
	}
	_MMCPP_TRACE("MoveCircularAbsoluteRadius: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
	return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteRadius(
        NC_ARC_SHORT_LONG_ENUM eArcShortLong,
        NC_PATH_CHOICE_ENUM ePathChoice,
        ELMO_DOUBLE dSpearHeadPoint[NC_MAX_NUM_AXES_IN_NODE],
        MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    //
    memcpy(m_dAuxPoint, dSpearHeadPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsoluteRadius(eArcShortLong,ePathChoice,eBufferMode);
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteRadius(
        NC_ARC_SHORT_LONG_ENUM eArcShortLong,
        NC_PATH_CHOICE_ENUM ePathChoice,
        ELMO_DOUBLE dSpearHeadPoint[NC_MAX_NUM_AXES_IN_NODE],
        ELMO_DOUBLE dEndPoint[NC_MAX_NUM_AXES_IN_NODE],
        MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    memcpy(m_dAuxPoint, dSpearHeadPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsoluteRadius(eArcShortLong,ePathChoice,eBufferMode);
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_MoveCircularAbsoluteAngleCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_MOVECIRCULARABSOLUTEANGLE_IN* pInParam,
///             OUT MMC_MOVECIRCULARABSOLUTE_OUT* pOutParam)
/// \brief This function This function  send Move Circular Absolute Angle command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Move Circular Absolute Angle input parameters
/// \param  pOutParam - [OUT] Pointer to Move Circular Absolute output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteAngle(
        ELMO_DOUBLE dAngle,
        ELMO_DOUBLE dCenterPoint[NC_MAX_NUM_AXES_IN_NODE],
        MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    memcpy(m_dAuxPoint, dCenterPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    return MoveCircularAbsoluteAngle(dAngle,eBufferMode);
}

ELMO_INT32 CMMCGroupAxis::MoveCircularAbsoluteAngle(
        ELMO_DOUBLE dAngle,
        MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVECIRCULARABSOLUTE_OUT stOutParams;
    MMC_MOVECIRCULARABSOLUTE_IN stInParams;

    memcpy(stInParams.dAuxPoint, m_dAuxPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dEndPoint, m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.dEndPoint[0] = dAngle;
    stInParams.eBufferMode = eBufferMode;
    CopyMoveCircularAbsParams(stInParams);
    stInParams.eCircleMode = MC_ANGLE_CIRC_MODE;

    ELMO_INT32 rc;
    if ((rc = MMC_MoveCircularAbsoluteCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveCircularAbsoluteAngleCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveCircularAbsoluteAngle: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsolute(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVELINEARABSOLUTE_IN stInParams;
    MMC_MOVELINEARABSOLUTE_OUT stOutParams;

    stInParams.eBufferMode= eBufferMode;
    memcpy(stInParams.dbPosition, m_dEndPoint, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    CopyMoveLinearAbsParams(stInParams);
    ELMO_INT32 rc;
    if ((rc = MMC_MoveLinearAbsoluteCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveLinearAbsoluteCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveLinearAbsolute: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsolute(ELMO_FLOAT fVelocity, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    rc = MoveLinearAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsolute(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsolute(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode ) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsolute(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode ) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelative(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVELINEARRELATIVE_IN stInParams;
    MMC_MOVELINEARRELATIVE_OUT stOutParams;
    stInParams.eBufferMode= eBufferMode;
    memcpy(stInParams.dbDistance, m_dEndPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    CopyMoveLinearRltParams(stInParams);

    ELMO_INT32 rc;
    if ((rc = MMC_MoveLinearRelativeCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveLinearRelativeCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveLinearRelative: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelative(ELMO_FLOAT fVelocity,MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    rc = MoveLinearRelative(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelative(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE],MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE) * NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearRelative(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelative(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration,MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearRelative(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelative(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearRelative(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAdditive(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVELINEARADDITIVE_IN stInParams;
    MMC_MOVELINEARADDITIVE_OUT stOutParams;

    stInParams.eBufferMode= eBufferMode;
    memcpy(stInParams.dbDistance, m_dEndPoint, sizeof(ELMO_DOUBLE)* NC_MAX_NUM_AXES_IN_NODE);
    CopyMoveLinearAddParams(stInParams);

    ELMO_INT32 rc;
    if ((rc = MMC_MoveLinearAdditiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveLinearAdditiveCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveLinearAdditive: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAdditive(ELMO_FLOAT fVelocity, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    rc = MoveLinearAdditive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAdditive(ELMO_FLOAT fVelocity,  ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE],  MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAdditive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAdditive(ELMO_FLOAT fVelocity,  ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration,  MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAdditive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAdditive(ELMO_FLOAT fVelocity,  ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE], ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAdditive(eBufferMode);
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupSetOverrideCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_SETOVERRIDE_IN* pInParam,
///             OUT MMC_SETOVERRIDE_OUT* pOutParam)
/// \brief This function  send Group Set Override command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Group Reference handle
/// \param  pInParam - [IN] Pointer to Set Override input parameters
/// \param  pOutParam - [OUT] Pointer to Set Override output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_INT32 CMMCGroupAxis::GroupSetOverride(
        ELMO_FLOAT fVelFactor,
        ELMO_FLOAT fAccFactor,
        ELMO_FLOAT fJerkFactor,
        ELMO_UINT16 usUpdateVelFactorIdx) throw (CMMCException)
{
    MMC_SETOVERRIDE_OUT stOutParam;
    MMC_SETOVERRIDE_IN stInParams;
    ELMO_INT32 rc;
    stInParams.fVelFactor = fVelFactor;
    stInParams.fAccFactor = fAccFactor;
    stInParams.fJerkFactor = fJerkFactor;
    stInParams.usUpdateVelFactorIdx = usUpdateVelFactorIdx;

    if ((rc = MMC_GroupSetOverrideCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupSetOverrideCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupSetOverride: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupSetPositionCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPSETPOSITION_IN* pInParam,
///             OUT MMC_GROUPSETPOSITION_OUT* pOutParam)
/// \brief This function  send Set Position command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Set Position input parameters
/// \param  pOutParam - [OUT] Pointer to Set POsition output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_INT32 CMMCGroupAxis::GroupSetPosition(
        ELMO_DOUBLE dbPosition[],
        MC_COORD_SYSTEM_ENUM eCordSystem,
        ELMO_UINT8 ucMode,
        MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_GROUPSETPOSITION_OUT stOutParam;
    MMC_GROUPSETPOSITION_IN stInParams;
    ELMO_INT32 rc;

    memcpy(stInParams.dbPosition, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eBufferMode = eBufferMode;
    stInParams.eCordSystem = eCordSystem;
    stInParams.eExecutionMode = eMMC_EXECUTION_MODE_IMMEDIATE;
    stInParams.ucExecute = 1;
    stInParams.ucMode = ucMode;

    if ((rc = MMC_GroupSetPositionCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupSetPositionCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupSetPosition: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupReadStatusCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPREADSTATUS_IN* pInParam,
///             OUT MMC_GROUPREADSTATUS_OUT* pOutParam)
/// \brief This function  send Read Group Status command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Axis Status input parameters structure
/// \param  pOutParam - [OUT] Axis Status output parameters structure
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_ULINT32 CMMCGroupAxis::GroupReadStatus() throw (CMMCException)
{
    MMC_GROUPREADSTATUS_OUT stOutParam;
    MMC_GROUPREADSTATUS_IN stInParams;
    ELMO_INT32 rc;

    stInParams.ucEnable = 1;

    if ((rc = MMC_GroupReadStatusCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadStatusCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReadStatus: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    return stOutParam.ulState;
}

ELMO_ULINT32 CMMCGroupAxis::GroupReadStatus(ELMO_UINT16& usGroupErrorID) throw (CMMCException)
{
    MMC_GROUPREADSTATUS_OUT stOutParam;
    MMC_GROUPREADSTATUS_IN stInParams;
    ELMO_INT32 rc;

    stInParams.ucEnable = 1;

    if ((rc = MMC_GroupReadStatusCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadStatusCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReadStatus: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    usGroupErrorID = stOutParam.usGroupErrorID;
    return stOutParam.ulState;
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupEnableCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPENABLE_IN* pInParam,
///             OUT MMC_GROUPENABLE_OUT* pOutParam)
/// \brief This function  send Group Enable command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to....
/// \param  pOutParam - [OUT] Axis ..
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::GroupEnable() throw (CMMCException)
{
    MMC_GROUPENABLE_OUT stOutParam;
    MMC_GROUPENABLE_IN stInParams;
    ELMO_INT32 rc;

    stInParams.ucExecute = 1;

    if ((rc = MMC_GroupEnableCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupEnableCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupEnable: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupDisableCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPDISABLE_IN* pInParam,
///             OUT MMC_GROUPDISABLE_OUT* pOutParam)
/// \brief This function This function  send Group Disable command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to....
/// \param  pOutParam - [OUT] Axis ..
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::GroupDisable() throw (CMMCException)
{
    MMC_GROUPDISABLE_OUT stOutParam;
    MMC_GROUPDISABLE_IN stInParams;
    ELMO_INT32 rc;

    stInParams.ucExecute = 1;

    if ((rc = MMC_GroupDisableCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupDisableCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupDisable: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupResetCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPRESET_IN* pInParam,
///             OUT MMC_GROUPRESET_OUT* pOutParam)
/// \brief This function  send Group Reset command to MMC server for specific Group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to....
/// \param  pOutParam - [OUT] Pointer to....
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::GroupReset() throw (CMMCException)
{
    MMC_GROUPRESET_OUT stOutParam;
    MMC_GROUPRESET_IN stInParams;
    ELMO_INT32 rc;

    stInParams.ucExecute = 1;

    if ((rc = MMC_GroupResetCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupResetCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReset: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
}
////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupReadActualVelocity(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPREADACTUALVELOCITY_IN* pInParam,
///             OUT MMC_GROUPREADACTUALVELOCITY_OUT* pOutParam)
/// \brief This function read group actual velocity.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Group Read Actual Velocity input parameters
/// \param  pOutParam - [OUT] Pointer to Group Read Actual Velocity output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_DOUBLE CMMCGroupAxis::GroupReadActualVelocity(MC_COORD_SYSTEM_ENUM eCoordSystem) throw (CMMCException)
{
    MMC_GROUPREADACTUALVELOCITY_OUT stOutParam;
    MMC_GROUPREADACTUALVELOCITY_IN stInParams;
    ELMO_INT32 rc;

    stInParams.eCoordSystem = eCoordSystem;
    stInParams.ucEnable = 1;

    if ((rc = MMC_GroupReadActualVelocity(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadActualVelocity: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReadActualVelocity: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    return stOutParam.dPathVelocity;
}

ELMO_DOUBLE CMMCGroupAxis::GroupReadActualVelocity(MC_COORD_SYSTEM_ENUM eCoordSystem, ELMO_DOUBLE dVelocity[NC_MAX_NUM_AXES_IN_NODE]) throw (CMMCException)
{
    MMC_GROUPREADACTUALVELOCITY_OUT stOutParam;
    MMC_GROUPREADACTUALVELOCITY_IN stInParams;
    ELMO_INT32 rc;

    stInParams.eCoordSystem = eCoordSystem;
    stInParams.ucEnable = 1;
    if ((rc = MMC_GroupReadActualVelocity(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadActualVelocity: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReadActualVelocity: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    memcpy(dVelocity, stOutParam.dVelocity, sizeof (ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    return stOutParam.dPathVelocity;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupReadError(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPREADERROR_IN* pInParam,
///             OUT MMC_GROUPREADERROR_OUT* pOutParam)
/// \brief This function read group error.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Group Read Error input parameters
/// \param  pOutParam - [OUT] Pointer to Group Read Error output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_UINT16 CMMCGroupAxis::GroupReadError() throw (CMMCException)
{
    MMC_GROUPREADERROR_OUT stOutParam;
    MMC_GROUPREADERROR_IN stInParams;
    ELMO_INT32 rc;

    stInParams.ucEnable = 1;

    if ((rc = MMC_GroupReadError(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadError: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReadError: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    return stOutParam.usGroupErrorID;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_AddAxisToGroup(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_ADDAXISTOGROUP_IN* pInParam,
///             OUT MMC_ADDAXISTOGROUP_OUT* pOutParam)
/// \brief This function add axis to axis group.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Add Axis To Group input parameters
/// \param  pOutParam - [OUT] Pointer to Add Axis To Group output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::AddAxisToGroup(NC_NODE_HNDL_T hNode, NC_IDENT_IN_GROUP_ENUM eIdentInGroup) throw (CMMCException)
{
    MMC_ADDAXISTOGROUP_OUT stOutParam;
    MMC_ADDAXISTOGROUP_IN stInParams;
    ELMO_INT32 rc;

    stInParams.eIdentInGroup = eIdentInGroup;
    stInParams.hNode = hNode;

    if ((rc = MMC_AddAxisToGroup(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_AddAxisToGroup: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mAddAxisToGroup: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupReadActualPosition(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPREADACTUALPOSITION_IN* pInParam,
///             OUT MMC_GROUPREADACTUALPOSITION_OUT* pOutParam)
/// \brief This function read group actual position.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis Reference handle
/// \param  pInParam - [IN] Pointer to Group Read Actual Position input parameters
/// \param  pOutParam - [OUT] Pointer to Group Read Actual Position output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
ELMO_INT32 CMMCGroupAxis::GroupReadActualPosition(MC_COORD_SYSTEM_ENUM eCoordSystem, ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE]) throw (CMMCException)
{
    MMC_GROUPREADACTUALPOSITION_OUT stOutParam;
    MMC_GROUPREADACTUALPOSITION_IN stInParams;
    ELMO_INT32 rc;

    stInParams.eCoordSystem = eCoordSystem;
    stInParams.ucEnable = 1;

    if ((rc = MMC_GroupReadActualPosition(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadActualPosition: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupReadActualPosition: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
    memcpy(dbPosition, stOutParam.dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupStopCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPSTOP_IN* pInParam,
///             OUT MMC_GROUPSTOP_OUT* pOutParam)
/// \brief This function ....
/// \param hConn - [IN] Connection handle
/// \param hAxisRef - [IN] Axis Reference handle
/// \param pInParam - [IN] Pointer to Group Stop input parameters
/// \param pOutParam - [OUT] Pointer to Group Stop output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::GroupStop(ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_GROUPSTOP_OUT stOutParam;
    MMC_GROUPSTOP_IN stInParams;
    ELMO_INT32 rc;

    stInParams.eBufferMode = eBufferMode;
    stInParams.fDeceleration = fDeceleration;
    stInParams.fJerk = fJerk;
    stInParams.ucExecute = 1;

    if ((rc = MMC_GroupStopCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupStopCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupStop: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int MMC_GroupHaltCmd(
///             IN MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_GROUPHALT_IN* pInParam,
///             OUT MMC_GROUPHALT_OUT* pOutParam)
/// \brief This function ....
/// \param hConn - [IN] Connection handle
/// \param hAxisRef - [IN] Axis Reference handle
/// \param pInParam - [IN] Pointer to Group Halt input parameters
/// \param pOutParam - [OUT] Pointer to Group Halt output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::GroupHalt(ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_GROUPHALT_OUT stOutParam;
    MMC_GROUPHALT_IN stInParams;
    ELMO_INT32 rc;

    stInParams.eBufferMode = eBufferMode;
    stInParams.fDeceleration = fDeceleration;
    stInParams.fJerk = fJerk;
    stInParams.ucExecute = 1;

    if ((rc = MMC_GroupHaltCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupHaltCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE("mGroupHalt: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParam.usErrorID, stOutParam.usStatus);
}

ELMO_INT32 CMMCGroupAxis::GetGroupAxisByName(const ELMO_INT8* cName) throw (CMMCException)
{
    ELMO_INT32 rc = -1;
    MMC_AXISBYNAME_IN   axis_by_name_in;
    MMC_AXISBYNAME_OUT  axis_by_name_out;

    strncpy(axis_by_name_in.cAxisName, cName, NODE_NAME_MAX_LENGTH);
    axis_by_name_in.cAxisName[NODE_NAME_MAX_LENGTH-1] = 0;

    if ((rc = MMC_GetGroupByNameCmd(m_uiConnHndl, &axis_by_name_in, &axis_by_name_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetGroupByNameCmd:", m_uiConnHndl, m_usAxisRef, rc, axis_by_name_out.usErrorID, axis_by_name_out.usStatus);
    }
    else
    {
        rc = (ELMO_INT32)axis_by_name_out.usAxisIdx;
    }
    SetRef(axis_by_name_out.usAxisIdx);
    SetName(cName);

    _MMCPP_TRACE("GetGroupAxisByName: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, axis_by_name_out.usErrorID, axis_by_name_out.usStatus);
    return (rc);
}


void CMMCGroupAxis::InitAxisData(const ELMO_INT8* cName, MMC_CONNECT_HNDL uHandle) throw (CMMCException)
{
    SetName(cName);
    SetConnHndl(uHandle);
    GetGroupAxisByName(cName);
}


ELMO_INT32 CMMCGroupAxis::MoveLinearAbsoluteRepetitive(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVELINEARABSOLUTEREPETITIVE_IN stInParams;
    MMC_MOVELINEARABSOLUTEREPETITIVE_OUT stOutParams;

    stInParams.eBufferMode= eBufferMode;
    memcpy(stInParams.dbPosition, m_dEndPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);

    CopyMoveLinearAbsRepParams(stInParams);

    ELMO_INT32 rc;
    if ((rc = MMC_MoveLinearAbsoluteRepetitiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveLinearAbsoluteRepetitiveCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveLinearAbsoluteRepetitive: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsoluteRepetitive(ELMO_FLOAT fVelocity,MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    rc = MoveLinearAbsoluteRepetitive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsoluteRepetitive(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsoluteRepetitive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsoluteRepetitive(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsoluteRepetitive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearAbsoluteRepetitive(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk,MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsoluteRepetitive(eBufferMode);
    return rc;
}


ELMO_INT32 CMMCGroupAxis::MoveLinearRelativeRepetitive(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVELINEARRELATIVEREPETITIVE_IN stInParams;
    MMC_MOVELINEARRELATIVEREPETITIVE_OUT stOutParams;

    memcpy(stInParams.dbPosition, m_dEndPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eBufferMode= eBufferMode;

    CopyMoveLinearRltRepParams(stInParams);

    ELMO_INT32 rc;
    if ((rc = MMC_MoveLinearRelativeRepetitiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveLinearRelativeRepetitiveCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MoveLinearRelativeRepetitive: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}


ELMO_INT32 CMMCGroupAxis::MoveLinearRelativeRepetitive(ELMO_FLOAT fVelocity,MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    rc = MoveLinearRelativeRepetitive(eBufferMode);
    return rc;
}



ELMO_INT32 CMMCGroupAxis::MoveLinearRelativeRepetitive(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE],MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearRelativeRepetitive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelativeRepetitive(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE], ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsoluteRepetitive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MoveLinearRelativeRepetitive(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbDistance[NC_MAX_NUM_AXES_IN_NODE], ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    memcpy(m_dEndPoint, dbDistance, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MoveLinearAbsoluteRepetitive(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MovePolynomAbsolute(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEPOLYNOMABSOLUTE_IN stInParams;
    MMC_MOVEPOLYNOMABSOLUTE_OUT stOutParams;
    //
    memcpy(stInParams.dbAuxPoint, m_dAuxPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    memcpy(stInParams.dbEndPoint, m_dEndPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eBufferMode= eBufferMode;

    CopyMovePolynomAbsParams(stInParams);

    ELMO_INT32 rc;
    if ((rc = MMC_MovePolynomAbsoluteCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MovePolynomAbsoluteCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MovePolynomAbsolute: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MovePolynomAbsolute(ELMO_FLOAT fVelocity,MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    rc = MovePolynomAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MovePolynomAbsolute(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbAuxPoint[NC_MAX_NUM_AXES_IN_NODE],ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    memcpy(m_dAuxPoint, dbAuxPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MovePolynomAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MovePolynomAbsolute(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbAuxPoint[NC_MAX_NUM_AXES_IN_NODE],ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode ) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    memcpy(m_dAuxPoint, dbAuxPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MovePolynomAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MovePolynomAbsolute(ELMO_FLOAT fVelocity,ELMO_DOUBLE dbAuxPoint[NC_MAX_NUM_AXES_IN_NODE],ELMO_DOUBLE dbPosition[NC_MAX_NUM_AXES_IN_NODE],ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode ) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    memcpy(m_dAuxPoint, dbAuxPoint, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_dEndPoint, dbPosition, sizeof(ELMO_DOUBLE)*NC_MAX_NUM_AXES_IN_NODE);
    rc = MovePolynomAbsolute(eBufferMode);
    return rc;
}

ELMO_INT32 CMMCGroupAxis::MovePolynomAbsolute(ELMO_FLOAT fVelocity,ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode ) throw (CMMCException)
{
    ELMO_INT32 rc;
    m_fVelocity = fVelocity;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MovePolynomAbsolute(eBufferMode);
    return rc;
}

/**
* \fn   unsigned int PathGetLengths(MC_PATH_REF hMemHandle,....) 
* \brief    retrieves length values of specifies segments.
            buffer must comply to number of values, which programer expects to get 
            and cannot be greater than 170 elements.
* \param    hMemHandle - table handler of offline spline table.
* \param    uiStartIndex - specifies the table segment from witch to start the length values collection.
* \param    uiNumOfSegments specifies the number of segments for the length values collection.
* \param    dbValues - the buffer in which this API stores the collected values.
* \return   number of returned values if completed successfuly, otherwise throws an error or error (< 0)
*/
ELMO_UINT32 CMMCGroupAxis::PathGetLengths(MC_PATH_REF hMemHandle,
                ELMO_UINT32 uiStartIndex,
                ELMO_UINT32 uiNumOfSegments,
                ELMO_PDOUBLE dbValues) throw (CMMCException)
{
    MMC_PATHGETLENGTHS_IN stInParams;
    MMC_PATHGETLENGTHS_OUT stOutParams;

    stInParams.hMemHandle = hMemHandle;
    stInParams.uiStartSeg = uiStartIndex;
    stInParams.uiSegmentsNum = uiNumOfSegments;
    stInParams.ucExecute = 1;

    ELMO_INT32 rc;
    if ((rc = MMC_PathGetLengthsCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PathGetLengthsCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    else
    {
        memcpy(dbValues, stOutParams.dbLengths, stOutParams.iRetValues*sizeof(ELMO_DOUBLE));
        rc = stOutParams.iRetValues;
    }
    _MMCPP_TRACE("PathSelect: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);

    return rc;
}

ELMO_UINT32 CMMCGroupAxis::PathSelect(MC_PATH_DATA_REF pPathToSplineFile) throw (CMMCException)
{
    MC_PATH_REF     hMemHandle;
    hMemHandle = PathSelect(pPathToSplineFile, m_eCoordSystem);
    return hMemHandle;
}

ELMO_UINT32 CMMCGroupAxis::PathSelect(MC_PATH_DATA_REF pPathToSplineFile, MC_COORD_SYSTEM_ENUM eCoordSystem, ELMO_UINT8 ucExecute) throw (CMMCException)
{
    MMC_PATHSELECT_IN stInParams;
    MMC_PATHSELECT_OUT stOutParams;

    memcpy(stInParams.pPathToSplineFile, pPathToSplineFile, sizeof(char)*NC_MAX_SPLINES_FILE_PATH_LENGTH);
    m_eCoordSystem = eCoordSystem;
    m_ucExecute = ucExecute;
    stInParams.eCoordSystem = eCoordSystem;
    stInParams.ucExecute = ucExecute;

    ELMO_INT32 rc;
    if ((rc = MMC_PathSelectCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PathSelectCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("PathSelect: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return stOutParams.hMemHandle;
}

void CMMCGroupAxis::PathDeselect(MC_PATH_REF hMemHandle) throw (CMMCException)
{
    MMC_PATHUNSELECT_IN stInParams;
    MMC_PATHUNSELECT_OUT stOutParams;

    stInParams.hMemHandle = hMemHandle;
    stInParams.ucExecute = m_ucExecute;

    ELMO_INT32 rc;
    if ((rc = MMC_PathUnselectCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("PathDeselect: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("PathDeselect: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return;
}

void CMMCGroupAxis::MovePath(MC_PATH_REF hMemHandle, MC_COORD_SYSTEM_ENUM eCoordSystem) throw (CMMCException)
{
    MovePath(hMemHandle,m_fTransitionParameter,MC_BUFFERED_MODE,eCoordSystem);
    return;
}

void CMMCGroupAxis::MovePath(MC_PATH_REF hMemHandle, ELMO_FLOAT fTransitionParameter[NC_MAX_NUM_AXES_IN_NODE], MC_BUFFERED_MODE_ENUM eBufferMode, MC_COORD_SYSTEM_ENUM eCoordSystem) throw (CMMCException)
{
    MMC_MOVEPATH_IN stInParams;
    MMC_MOVEPATH_OUT stOutParams;

    m_eCoordSystem = eCoordSystem;
    stInParams.hMemHandle = hMemHandle;
    memcpy(stInParams.fTransitionParameter, fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    memcpy(m_fTransitionParameter, fTransitionParameter, sizeof(ELMO_FLOAT) * NC_MAX_NUM_AXES_IN_NODE);
    stInParams.eBufferMode = eBufferMode;
    stInParams.ucExecute = m_ucExecute;
    stInParams.eCoordSystem = eCoordSystem;
    stInParams.eTransitionMode = m_eTransitionMode;
    stInParams.ucSuperImposed = m_ucSuperimposed;

    ELMO_INT32 rc;
    if ((rc = MMC_MovePathCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MovePathCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("MovePath: %d, %s, %d, %d, %d\n", rc, m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return;
}


/*! \fn double pGetParameter(MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief This function reads specific group parameter.
 * \param eNumber - parameter number.
 * \param iIndex - index into parameters array if axis group.
 * \return - parameter value on success, otherwise throws CMMCException.
 */
ELMO_DOUBLE CMMCGroupAxis::GetParameter(MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_READPARAMETER_IN stInParam;
    MMC_READPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GroupReadParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GroupReadParameter OK");
    return stOutParam.dbValue;
}

/*!fn unsigned long pGetBoolParameter(MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief This function reads boolean specific group parameter.
 * \param eNumber - parameter enumeration.
 * \param  iIndex - index into array in case of a group.
 * \return  return - value of specified parameter on success, otherwise throws CMMCException.
 */
ELMO_LINT32 CMMCGroupAxis::GetBoolParameter(MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_READBOOLPARAMETER_IN stInParam;
    MMC_READBOOLPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GroupReadBoolParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupReadBoolParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GroupReadBoolParameter OK")
    return stOutParam.lValue;
}

/*! \fn void pSetParameter(double dbValue, MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief this method writes specific group parameter.
 * \param dbValue - value to set.
 * \param eNumber - parameter enumeration.
 * \param  iIndex - index into array in case of a group.
 * \return  return - none on success, otherwise throws CMMCException.
 */
void CMMCGroupAxis::SetParameter(ELMO_DOUBLE dbValue, MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_WRITEPARAMETER_IN stInParam;
    MMC_WRITEPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.dbValue = dbValue;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GroupWriteParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupWriteParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GroupWriteParameter OK")
}

/*! \fn void pSetBoolParameter(unsigned long ulValue, MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief this method write boolean specific group parameter.
 * \param ulValue - value to set.
 * \param eNumber - parameter enumeration.
 * \param  iIndex - index into array in case of a group.
 * \return  return - none on success, otherwise throws CMMCException.
 */
void CMMCGroupAxis::SetBoolParameter(ELMO_LINT32 lValue, MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_WRITEBOOLPARAMETER_IN stInParam;
    MMC_WRITEBOOLPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.lValue = lValue;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_GroupWriteBoolParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GroupWriteBoolParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GroupWriteBoolParameter OK")
}


/*! \fn void pGetMembersInfo(MMC_GETGROUPMEMBERSINFO_OUT* sOutput)
 * \brief this method retreive the data regarding group members
 * \return  return - on success
 * sOutput - the output returns two strings, status and error id.
 *   1. Number of axes in group (members)
 *   2. Array of axes names.
 *   3. Array of axes references.
 *   4. Array of axes ID's.
 * otherwise throws CMMCException.
 */

void CMMCGroupAxis::GetMembersInfo(MMC_GETGROUPMEMBERSINFO_OUT* stOutput) throw (CMMCException)
{
    MMC_GETGROUPMEMBERSINFO_IN stInParam;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetGroupMembersInfo(m_uiConnHndl, m_usAxisRef, &stInParam, stOutput))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetGroupMembersInfo:", m_uiConnHndl, m_usAxisRef, rc, stOutput->usErrorID, stOutput->usStatus);
    }
    _MMCPP_TRACE_INFO("GetGroupMembersInfo OK")
}

/*! \fn unsigned int GetStatusRegister()
* \brief This function returns the motion status word
* \return   return - 0 on success, otherwise throws CMMCException.
*/
ELMO_UINT32 CMMCGroupAxis::GetStatusRegister(MMC_GETSTATUSREGISTER_OUT& sOutput) throw (CMMCException)
{
    MMC_GETSTATUSREGISTER_IN get_param_in;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetStatusRegisterCmd(m_uiConnHndl, m_usAxisRef, &get_param_in, &sOutput))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetStatusRegister:", m_uiConnHndl, m_usAxisRef, rc, sOutput.usErrorID, sOutput.usStatus);

    _MMCPP_TRACE_INFO("MMC_GetStatusRegister OK")
    return sOutput.uiStatusRegister;
}


/*! \fn unsigned int GetStatusRegister()
* \brief This function returns the motion status word
* \return   return - 0 on success, otherwise throws CMMCException.
*/
ELMO_UINT32 CMMCGroupAxis::GetStatusRegister() throw (CMMCException)
{
    MMC_GETSTATUSREGISTER_OUT get_param_out;
    return GetStatusRegister(get_param_out);
}


/*! \fn unsigned int GetMcsLimitRegister()
* \brief This function returns the MCS limit register
* \return   return - 0 on success, otherwise throws CMMCException.
*/
ELMO_UINT32 CMMCGroupAxis::GetMcsLimitRegister() throw (CMMCException)
{
        MMC_GETSTATUSREGISTER_OUT get_param_out;
        GetStatusRegister(get_param_out);
        return get_param_out.uiMcsLimitRegister;
}



void CMMCGroupAxis::SetCartesianKinematics(MC_KIN_REF_CARTESIAN& stCart) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_SETKINTRANSFORMEX_IN param_in;
    MMC_SETKINTRANSFORMEX_OUT param_out;
    param_in.eKinType = NC_CARTESIAN_TYPE;
    param_in.eBufferMode = MC_BUFFERED_MODE;
    param_in.ucExecute = 1;
    memcpy((ELMO_PVOID)&param_in.stInput.stCart, (const void *)&stCart, sizeof(MC_KIN_REF_CARTESIAN));

    if ((rc=MMC_SetKinTransformEx(m_uiConnHndl, m_usAxisRef, &param_in, &param_out))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetKinTransformex:", m_uiConnHndl, m_usAxisRef, rc, param_out.usErrorID, param_out.usStatus);
    //
    _MMCPP_TRACE_INFO("MMC_SetKinTransformex OK")

}

void CMMCGroupAxis::SetDeltaRobotKinematics(MC_KIN_REF_DELTA& stDelta) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_SETKINTRANSFORMEX_IN param_in;
    MMC_SETKINTRANSFORMEX_OUT param_out;
    memcpy((ELMO_PVOID)&param_in.stInput.stDelta,(const void *)&stDelta, sizeof(MC_KIN_REF_DELTA));
    param_in.eKinType = NC_DELTA_ROBOT_TYPE;
    param_in.eBufferMode = MC_BUFFERED_MODE;
    param_in.ucExecute = 1;

    if ((rc=MMC_SetKinTransformEx(m_uiConnHndl, m_usAxisRef, &param_in, &param_out))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetKinTransformex:", m_uiConnHndl, m_usAxisRef, rc, param_out.usErrorID, param_out.usStatus);
    //
    _MMCPP_TRACE_INFO("MMC_SetKinTransformex OK")
}

void CMMCGroupAxis::SetKinematic(MC_KIN_REF& stInput,NC_KIN_TYPE eKinType) throw (CMMCException)
{
    MMC_SETKINTRANSFORMEX_IN param_in;
    MMC_SETKINTRANSFORMEX_OUT param_out;
    ELMO_INT32 rc;
    //
    param_in.eBufferMode = MC_BUFFERED_MODE;    
    param_in.ucExecute = 1;
    //
    if(eKinType == NC_CARTESIAN_TYPE)
    {
        param_in.eKinType = NC_CARTESIAN_TYPE;
        memcpy((ELMO_PVOID)&param_in.stInput.stCart,(ELMO_PVOID)&stInput.stCart,sizeof(MC_KIN_REF_CARTESIAN));
    }
    else if(eKinType == NC_DELTA_ROBOT_TYPE)
    {
        param_in.eKinType = NC_DELTA_ROBOT_TYPE;
        memcpy((ELMO_PVOID)&param_in.stInput.stDelta,(ELMO_PVOID)&stInput.stDelta,sizeof(MC_KIN_REF_DELTA));
    }
    else if(eKinType == NC_DUAL_HEAD_TYPE)
    {
        param_in.eKinType = NC_DUAL_HEAD_TYPE;
        memcpy((void*)&param_in.stInput.stDualHead,(ELMO_PVOID)&stInput.stDualHead,sizeof(MC_KIN_REF_DUAL_HEAD));
    }
    else
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetKinTransformex:", m_uiConnHndl, m_usAxisRef, MMC_GMAS_ERROR, NC_KINEMATIC_TYPE_OUT_OF_RANGE, MC_FB_ERROR_BIT_MASK);
    }   
    //
    if ((rc=MMC_SetKinTransformEx(m_uiConnHndl, m_usAxisRef, &param_in, &param_out))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetKinTransformex:", m_uiConnHndl, m_usAxisRef, rc, param_out.usErrorID, param_out.usStatus);
    //
    _MMCPP_TRACE_INFO("MMC_SetKinTransformex OK")
    return;
}


/**! \fn void SetCartesianTransform(MMC_SETCARTESIANTRANSFORM_IN& stInParam)
*   \brief sets MCS to PCS parameters for group's kinematic transformation.
*   \param stInParam reference of structure with kinematic parameters.
*   \return none. or throws CMMCException on failure.
*/
ELMO_INT32 CMMCGroupAxis::SetCartesianTransform(MMC_SETCARTESIANTRANSFORM_IN* stInParam) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    //MMC_SETCARTESIANTRANSFORM_IN param_in;
    MMC_SETCARTESIANTRANSFORM_OUT param_out;

    if ((rc=MMC_SetCartesianTransform(m_uiConnHndl, m_usAxisRef, stInParam, &param_out))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCartesianTransform: ", m_uiConnHndl, m_usAxisRef, rc, param_out.sErrorID, param_out.usStatus);
    _MMCPP_TRACE_INFO("MMC_SetCartesianTransform OK")
    return rc;
}

/**! \fn void SetCartesianTransform(double (&dbOffset)[3], double (&dbRotAngle)[3], 
*       PCS_ROTATION_ANGLE_UNITS_ENUM eRotAngleUnits=PCS_DEGREE,
*       MC_BUFFERED_MODE_ENUM eBufferMode=MC_BUFFERED_MODE, 
*       MC_EXECUTION_MODE eExecutionMode=eMMC_EXECUTION_MODE_IMMEDIATE)
*   \brief sets MCS to PCS parameters for group's kinematic transformation.
*   \param dbOffset     group of three positions in cartesian system.
*   \param dbRotAngle   group of three angles around the vector axes (see dbOffsets).
*   \param eRotAngleUnits degrees (0) or radians (1).
*   \param eBufferMode one of the buffered modes in the list.
*   \param eExecutionMode   immediate (0) or queued (1) (currently immediate only).
*   \return none. or throws CMMCException on failure.
*/
ELMO_INT32 CMMCGroupAxis::SetCartesianTransform(ELMO_DOUBLE (&dbOffset)[3], ELMO_DOUBLE (&dbRotAngle)[3], 
    PCS_ROTATION_ANGLE_UNITS_ENUM eRotAngleUnits,
    MC_BUFFERED_MODE_ENUM eBufferMod, 
    MC_EXECUTION_MODE eExecutionMode) throw (CMMCException)
{
    int rc = 0;
    MMC_SETCARTESIANTRANSFORM_IN stInParams;
    MMC_SETCARTESIANTRANSFORM_OUT stOutParams;

    memcpy(stInParams.dOffset,      dbOffset,   3*sizeof(ELMO_DOUBLE));
    memcpy(stInParams.dRotAngle,    dbRotAngle, 3*sizeof(ELMO_DOUBLE));
    stInParams.eBufferMode = eBufferMod;
    stInParams.eExecutionMode = eMMC_EXECUTION_MODE_IMMEDIATE; //administrative function block
    stInParams.ucExecute = 1;

    if ( (rc=MMC_SetCartesianTransform(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams)) != 0 )
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCartesianTransform: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
    _MMCPP_TRACE_INFO("MMC_SetCartesianTransform OK")
    return rc;
}

/*! \fn void ReadKinTransform((MMC_READKINTRANSFORMEX_OUT& stOutParam)
*   \brief This function get group's kinematic transformation parameter.
*   \param stOutParam reference of structure with kinematic parameters.
*   \return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::ReadKinTransform(MMC_READKINTRANSFORMEX_OUT& stOutParam) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_READKINTRANSFORMEX_IN stParamIn;
    
    stParamIn.ucEnable = 1;

	if ((rc = MMC_ReadKinTransformEx(m_uiConnHndl, m_usAxisRef, &stParamIn, &stOutParam)) != 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetKinTransformex:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    //
    _MMCPP_TRACE_INFO("MMC_ReadKinTransformEx OK")
    return rc;
}


/*! \fn void ClearKinTransform()
*   \brief This function clear group's kinematic parameters.
*   \return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::ClearKinTransform() throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_CLEARKINTRANSFORM_IN stParamIn;
    MMC_CLEARKINTRANSFORM_OUT stParamOut;

    stParamIn.ucEnable = 1;

    if ((rc = MMC_ClearKinTransform(m_uiConnHndl, m_usAxisRef, &stParamIn, &stParamOut)) != 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("ClearKinTransform:", m_uiConnHndl, m_usAxisRef, rc, stParamOut.usErrorID, stParamOut.usStatus);
    //
    _MMCPP_TRACE_INFO("MMC_ReadKinTransformEx OK")
        return rc;
}

/*
 * wrappers for kinematics's transformation API
 */
/*! \fn void SetKinTransformDelta(MMC_KINTRANSFORM_DELTA_IN& i_params, unsigned char ucLinearUU, unsigned char ucRotaryUU)
*	\brief sets kinematic transformation parameters (MSC to ACS) for delta robot using selected user units.
*	\param i_params reference of data structure with kinematic parameters.
*	\param ucRotaryUU rotary selected user unit (radians, degrees, miliradians. default is no conversion).
*	\param ucLinearUU linear selected user unit (millimeters, micrometers, nanometers. default is no conversion)
*	\return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::SetKinTransformDelta(MMC_KINTRANSFORM_DELTA_IN& i_params, ELMO_UINT8 ucLinearUU, ELMO_UINT8 ucRotaryUU) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_SETKINTRANSFORMUU_IN i_nparams;	//new API input UU
	MMC_SETKINTRANSFORMEX_OUT o_params;	//both APIs output

	i_nparams.eBufferMode = MC_BUFFERED_MODE;
	i_nparams.eKinType = NC_DELTA_ROBOT_TYPE;
	i_nparams.ucExecute = 1;
	i_nparams.ucLinearUU = ucLinearUU;
	i_nparams.ucRotaryUU = ucRotaryUU;
	memcpy(&i_nparams.stInput.stDelta, &i_params.stParams, sizeof(MC_KIN_REF_DELTA));
	if ((rc = MMC_SetKinTransformData(m_uiConnHndl, m_usAxisRef, &i_nparams, &o_params)) != 0)
		CMMCPPGlobal::Instance()->MMCPPThrow("SetKinTransformDelta: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	_MMCPP_TRACE_INFO("SetKinTransformDelta OK")
		return rc;
}


/*! \fn void SetKinTransformCartesian(MMC_KINTRANSFORM_CARTESIAN_IN& i_params, unsigned char ucLinearUU, unsigned char ucRotaryUU)
*	\brief sets kinematic transformation parameters (MSC to ACS) for cartesian using selected user units.
*	\param i_params reference of data structure with kinematic parameters.
*	\param ucRotaryUU rotary selected user unit (radians, degrees, miliradians. default is no conversion).
*	\param ucLinearUU linear selected user unit (millimeters, micrometers, nanometers. default is no conversion)
*	\return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::SetKinTransformCartesian(MMC_KINTRANSFORM_CARTESIAN_IN& i_params, ELMO_UINT8 ucLinearUU, ELMO_UINT8 ucRotaryUU) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_SETKINTRANSFORMUU_IN i_nparams;	//new API input UU
	MMC_SETKINTRANSFORMEX_OUT o_params;	//both APIs output

	i_nparams.eBufferMode = MC_BUFFERED_MODE;
	i_nparams.eKinType = NC_CARTESIAN_TYPE;
	i_nparams.ucExecute = 1;
	i_nparams.ucLinearUU = ucLinearUU;
	i_nparams.ucRotaryUU = ucRotaryUU;
	memcpy(&i_nparams.stInput.stCart, &i_params.stParams, sizeof(MC_KIN_REF_CARTESIAN));
	if ((rc = MMC_SetKinTransformData(m_uiConnHndl, m_usAxisRef, &i_nparams, &o_params)) != 0)
		CMMCPPGlobal::Instance()->MMCPPThrow("SetKinTransformCartesian: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	_MMCPP_TRACE_INFO("SetKinTransformCartesian OK")
		return rc;
}

/*! \fn void SetKinTransformScara(MMC_KINTRANSFORM_SCARA_IN& i_params, unsigned char ucLinearUU, unsigned char ucRotaryUU)
*	\brief sets kinematic transformation parameters (MSC to ACS) for scara robot using selected user units.
*	\param i_params reference of data structure with kinematic parameters.
*	\param ucRotaryUU rotary selected user unit (radians, degrees, miliradians. default is no conversion).
*	\param ucLinearUU linear selected user unit (millimeters, micrometers, nanometers. default is no conversion)
*	\return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::SetKinTransformScara(MMC_KINTRANSFORM_SCARA_IN& i_params, ELMO_UINT8 ucLinearUU, ELMO_UINT8 ucRotaryUU) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_SETKINTRANSFORMUU_IN i_nparams;	//new API input UU
	MMC_SETKINTRANSFORMEX_OUT o_params;	//both APIs output

	i_nparams.eBufferMode = MC_BUFFERED_MODE;
	i_nparams.eKinType = NC_SCARA_ROBOT_TYPE;
	i_nparams.ucExecute = 1;
	i_nparams.ucLinearUU = ucLinearUU;
	i_nparams.ucRotaryUU = ucRotaryUU;
	memcpy(&i_nparams.stInput.stScara, &i_params.stParams, sizeof(MC_KIN_REF_SCARA));
	if ((rc = MMC_SetKinTransformData(m_uiConnHndl, m_usAxisRef, &i_nparams, &o_params)) != 0)
		CMMCPPGlobal::Instance()->MMCPPThrow("SetKinTransformScara: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	_MMCPP_TRACE_INFO("SetKinTransformScara OK")
		return rc;
}


/*! \fn void SetKinTransformThreeLink(MMC_KINTRANSFORM_THREELINK_IN& i_params, unsigned char ucLinearUU, unsigned char ucRotaryUU)
*	\brief sets kinematic transformation parameters (MSC to ACS) for threelink robot using selected user units.
*	\param i_params reference of data structure with kinematic parameters.
*	\param ucRotaryUU rotary selected user unit (radians, degrees, miliradians. default is no conversion).
*	\param ucLinearUU linear selected user unit (millimeters, micrometers, nanometers. default is no conversion)
*	\return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::SetKinTransformThreeLink(MMC_KINTRANSFORM_THREELINK_IN& i_params, ELMO_UINT8 ucLinearUU, ELMO_UINT8 ucRotaryUU) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_SETKINTRANSFORMUU_IN i_nparams;	//new API input UU
	MMC_SETKINTRANSFORMEX_OUT o_params;	//both APIs output

	i_nparams.eBufferMode = MC_BUFFERED_MODE;
	i_nparams.eKinType = NC_THREE_LINK_ROBOT_TYPE;
	i_nparams.ucExecute = 1;
	i_nparams.ucLinearUU = ucLinearUU;
	i_nparams.ucRotaryUU = ucRotaryUU;
	memcpy(&i_nparams.stInput.stThreeLink, &i_params.stParams, sizeof(MC_KIN_REF_THREE_LINK));
	if ((rc = MMC_SetKinTransformData(m_uiConnHndl, m_usAxisRef, &i_nparams, &o_params)) != 0)
		CMMCPPGlobal::Instance()->MMCPPThrow("SetKinTransformThreeLink: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	_MMCPP_TRACE_INFO("SetKinTransformThreeLink OK")
		return rc;
}

/*! \fn void SetKinTransformHxpd(MMC_KINTRANSFORM_HXPD_IN& i_params, unsigned char ucLinearUU, unsigned char ucRotaryUU)
*	\brief sets kinematic transformation parameters (MSC to ACS) for Hexapod robot using selected user units.
*	\param i_params reference of data structure with kinematic parameters.
*	\param ucRotaryUU rotary selected user unit (radians, degrees, miliradians. default is no conversion).
*	\param ucLinearUU linear selected user unit (millimeters, micrometers, nanometers. default is no conversion)
*	\return 0 if completed successfully, otherwise error or throws CMMCException.
*/
ELMO_INT32 CMMCGroupAxis::SetKinTransformHxpd(MMC_KINTRANSFORM_HXPD_IN& i_params, ELMO_UINT8 ucLinearUU, ELMO_UINT8 ucRotaryUU) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_SETKINTRANSFORMUU_IN i_nparams;	//new API input UU
	MMC_SETKINTRANSFORMEX_OUT o_params;	//both APIs output

	i_nparams.eBufferMode = MC_BUFFERED_MODE;
	i_nparams.eKinType = NC_HXPD_ROBOT_TYPE;
	i_nparams.ucExecute = 1;
	i_nparams.ucLinearUU = ucLinearUU;
	i_nparams.ucRotaryUU = ucRotaryUU;
	memcpy(&i_nparams.stInput.stHxpd, &i_params.stParams, sizeof(MC_KIN_REF_HXPD));
	if ((rc = MMC_SetKinTransformData(m_uiConnHndl, m_usAxisRef, &i_nparams, &o_params)) != 0)
		CMMCPPGlobal::Instance()->MMCPPThrow("SetKinTransformHxpd: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	_MMCPP_TRACE_INFO("SetKinTransformHxpd OK")
	return rc;
}


/**
* \fn   int TrackRotaryTable(unsigned short usMaster,   ...)
* \brief    this function block offers an abstraction layer for a rotary table,
*           assisting the user with tracking objects moving on a cyrcle space.
*           in short, a dynamic MCS to PCS transition depends on rotary table axis position.
*           this command operates a real motion profiler, opposed to 'TrackRotaryTable', which is a command of type 'admin'.
* \param usMaster               rotary table axis reference.
* \param dbMasterOrigin positions of conveyor belt relative to MCS.
* \param dbPCSOrigin    positions of the part (PCS) relative to dbMasterOrigin.
* \param dbInitialObjectPosition positions of an object lying on the part, relative to PCS.
* \param dbRampTrajectoryParams trajectory's parameters (Z-safe, T1-T4, see document).
* \param dbMasterInitialPosition    initial position set by user (e.g. at activation time, camera event etc...)
* \param dbMasterSyncPosition relative distance from initial position (see ucAutoSyncPosition)
* \param dbMasterScaling    scaling for position/speed (mainly counts to radians).
* \param ucAutoSyncPosition if 0 then sync position relative to dbMasterInitialPosition
*                           otherwise it is relative to master position at activation time.
* \param eRotAngleUnits         angle units, degrees (0) or radians (1)
* \param eSourceType            position source type from rotary table axis.
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::TrackRotaryTable(
    ELMO_UINT16 usMaster,
    ELMO_DOUBLE(&dbMasterOrigin)[6],
    ELMO_DOUBLE(&dbPCSOrigin)[6],
    ELMO_DOUBLE(&dbInitialObjectPosition)[6],
    ELMO_DOUBLE(&dbRampTrajectoryParams)[12],
    ELMO_DOUBLE dbMasterInitialPosition,
    ELMO_DOUBLE dbMasterSyncPosition,
    ELMO_DOUBLE dbMasterScaling,
    ELMO_UINT8  ucAutoSyncPosition,
    PCS_ROTATION_ANGLE_UNITS_ENUM eRotAngleUnits,
    PCS_REF_AXIS_SRC_ENUM eSourceType) throw(CMMCException)
{
    ELMO_INT32 rc;
    MMC_TRACKROTARYTABLE_IN i_params;
    MMC_TRACKROTARYTABLE_OUT o_params;

    memcpy(i_params.dbRotaryTableOrigin,    dbMasterOrigin,         6  * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbPCSOrigin,            dbPCSOrigin,            6  * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbInitialObjectPosition,dbInitialObjectPosition,6  * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbRampTrajectoryParams, dbRampTrajectoryParams, 12 * sizeof(ELMO_DOUBLE));

    i_params.usRotaryTable = usMaster;
    i_params.dbMasterInitialPosition = dbMasterInitialPosition;
    i_params.dbMasterScaling = dbMasterScaling;
    i_params.dbMasterSyncPosition = dbMasterSyncPosition;
    i_params.eBufferMode = MC_BUFFERED_MODE;
    i_params.eRotAngleUnits = eRotAngleUnits;
    i_params.eTrajectoryMode = eTRJ_MODE_BASE;
    i_params.ucAutoSyncPosition = ucAutoSyncPosition;
    i_params.ucExecute = 1; //only buffered is permitted

    if ((rc = MMC_TrackRotaryTable(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) != 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TrackRotaryTable: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
    _MMCPP_TRACE_INFO("MMC_TrackRotaryTable OK")
    return rc;
}

/**
* \fn   int TrackRotaryTable(MMC_TRACKSYNCIN_IN& params)
* \brief    this function block offers an abstraction layer for a rotary table,
*           assisting the user with tracking objects moving on a cyrcle space.
*           in short, a dynamic MCS to PCS transition depends on rotary table axis position.
*           this command operates a real motion profiler, opposed to 'TrackRotaryTable', which is a command of type 'admin'.
* \param param  reference of parameters data structure.
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::TrackRotaryTable(MMC_TRACKSYNCIN_IN& params) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    rc = TrackRotaryTable(params.usMaster,
        params.dbMasterOrigin,
        params.dbPCSOrigin,
        params.dbInitialObjectPosition,
        params.dbRampTrajectoryParams,
        params.dbMasterInitialPosition,
        params.dbMasterSyncPosition,
        params.dbMasterScaling,
        params.ucAutoSyncPosition,
        params.eRotAngleUnits);
    return rc;
}

/**
* \fn   int TrackConveyorBelt(unsigned short usMaster,  ...)
* \brief    this function block offers an abstraction layer for a conveyor belt,
*           assisting the user with tracking objects moving on a conveyor.
*           in short, a dynamic MCS to PCS transition depends on conveyor belt axis position.
*           this command operates a real motion profiler, opposed to 'TrackConveyorBelt', which is a command of type 'admin'.
* \param usMaster               rotary table axis reference.
* \param dbMasterOrigin positions of conveyor belt relative to MCS.
* \param dbPCSOrigin    positions of the part (PCS) relative to dbMasterOrigin.
* \param dbInitialObjectPosition positions of an object lying on the part, relative to PCS.
* \param dbRampTrajectoryParams trajectory's parameters (Z-safe, T1-T4, see document).
* \param dbMasterInitialPosition    initial position set by user (e.g. at activation time, camera event etc...)
* \param dbMasterSyncPosition relative distance from initial position (see ucAutoSyncPosition)
* \param dbMasterScaling    scaling for position/speed (mainly counts to radians).
* \param ucAutoSyncPosition if 0 then sync position relative to dbMasterInitialPosition
*                           otherwise it is relative to master position at activation time.
* \param eRotAngleUnits         angle units, degrees (0) or radians (1)
* \param eSourceType            position source type from rotary table axis.
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::TrackConveyorBelt(
    ELMO_UINT16 usMaster,
    ELMO_DOUBLE(&dbMasterOrigin)[6],
    ELMO_DOUBLE(&dbPCSOrigin)[6],
    ELMO_DOUBLE(&dbInitialObjectPosition)[6],
    ELMO_DOUBLE(&dbRampTrajectoryParams)[12],
    ELMO_DOUBLE dbMasterInitialPosition,
    ELMO_DOUBLE dbMasterSyncPosition,
    ELMO_DOUBLE dbMasterScaling,
    ELMO_UINT8 ucAutoSyncPosition,
    PCS_ROTATION_ANGLE_UNITS_ENUM eRotAngleUnits,
    PCS_REF_AXIS_SRC_ENUM eSourceType) throw(CMMCException)
{
    ELMO_INT32 rc;
    MMC_TRACKCONVEYORBELT_IN i_params;
    MMC_TRACKCONVEYORBELT_OUT o_params;

    memcpy(i_params.dbConveyorBeltOrigin,   dbMasterOrigin,          6 * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbPCSOrigin,            dbPCSOrigin,             6 * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbInitialObjectPosition,dbInitialObjectPosition, 6 * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbRampTrajectoryParams, dbRampTrajectoryParams, 12 * sizeof(ELMO_DOUBLE));

    i_params.usConveyorBelt = usMaster;
    i_params.dbMasterInitialPosition = dbMasterInitialPosition;
    i_params.dbMasterScaling = dbMasterScaling;
    i_params.dbMasterSyncPosition = dbMasterSyncPosition;
    i_params.eBufferMode = MC_BUFFERED_MODE;
    i_params.eRotAngleUnits = eRotAngleUnits;
    i_params.eTrajectoryMode = eTRJ_MODE_BASE;
    i_params.ucAutoSyncPosition = ucAutoSyncPosition;
    i_params.ucExecute = 1; //only buffered is permitted

    if ((rc = MMC_TrackConveyorBelt(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) != 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TrackConveyorBeltEx: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
    _MMCPP_TRACE_INFO("MMC_TrackConveyorBeltEx OK")
    return rc;
}

/**
* \fn   int TrackConveyorBelt(MMC_TRACKSYNCIN_IN& params)
* \brief    this function block offers an abstraction layer for a conveyor belt,
*           assisting the user with tracking objects moving on a cyrcle space.
*           in short, a dynamic MCS to PCS transition depends on conveyor axis position.
*           this command operates a real motion profiler, opposed to 'TrackConveyorBelt', which is a command of type 'admin'.
* \param param  reference of parameters data structure.
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::TrackConveyorBelt(MMC_TRACKSYNCIN_IN& params) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    rc = TrackConveyorBelt(params.usMaster,
        params.dbMasterOrigin,
        params.dbPCSOrigin,
        params.dbInitialObjectPosition,
        params.dbRampTrajectoryParams,
        params.dbMasterInitialPosition,
        params.dbMasterSyncPosition,
        params.dbMasterScaling,
        params.ucAutoSyncPosition,
        params.eRotAngleUnits);
    return rc;
}

/**
* \fn   int TrackSyncOut(unsigned short usMaster,   ...)
* \brief    this function block offers an abstraction layer for syncing out a tracking process.
*           in short, a dynamic PCS to MCS transition depends on master (RT/CB) axis position.
*           this command operates a real motion profiler.
* \param usMaster       rotary table or conveyor belt axis reference.
* \param dbMasterOrigin positions of master (RT/CB) relative to MCS.
* \param dbTargetPosition   target positions (MCS). relvant only on none immediate mode of operation.
* \param dbRampTrajectoryParams trajectory's parameters (Z-safe, T1-T4, see document).
* \param dbMasterScaling    scaling for position/speed (mainly counts to radians).
* \param dbTime     time to sync out (seconds).
* \param dbStopDeceleration override node's definition for stop deceleration.
* \param ucInstantly    Zsafe only {immediate (1)} or x,y as well {none immediate (0)}.
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::TrackSyncOut(
    ELMO_UINT16 usMaster,
    ELMO_DOUBLE(&dbMasterOrigin)[6],
    ELMO_DOUBLE(&dbTargetPosition)[6],          //relevant only for none immediate mode of operation
    ELMO_DOUBLE(&dbRampTrajectoryParams)[12],   //ARRAY[12] of LREAL[<Z Safe Height (%)>, t1-t4, spare 7 doubles]
    ELMO_DOUBLE dbMasterScaling,
    ELMO_DOUBLE dbTime,                         //Time to sync out (seconds)
    ELMO_DOUBLE dbStopDeceleration,             //override node definition
    ELMO_UINT8 ucInstantly              //Zsafe only {immediate (1)} or x,y as well {none immediate (0)}
) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    MMC_TRACKSYNCOUT_IN i_params;

    memcpy(i_params.dbMasterOrigin,         dbMasterOrigin,          6 * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbTargetPosition,       dbTargetPosition,        6 * sizeof(ELMO_DOUBLE));
    memcpy(i_params.dbRampTrajectoryParams, dbRampTrajectoryParams, 12 * sizeof(ELMO_DOUBLE));

    i_params.dbMasterScaling = dbMasterScaling;
    i_params.dbTime = dbTime;
    i_params.dbStopDeceleration = dbStopDeceleration;
    i_params.ucInstantly = ucInstantly;
    i_params.usMaster = usMaster;

    rc = TrackSyncOut(i_params);
    return rc;
}

/**
* \fn   int TrackSyncOut(MMC_SYNCIN_IN& params)
* \brief    this function block offers an abstraction layer for syncing out a tracking process.
*           in short, a dynamic PCS to MCS transition depends on master (RT/CB) axis position.
*           this command operates a real motion profiler.
* \param params reference for parameters data structure.
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::TrackSyncOut(MMC_TRACKSYNCOUT_IN& i_params) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_TRACKSYNCOUT_OUT o_params;

    if ((rc = MMC_TrackSyncOut(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) != 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TrackSyncOut: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
    _MMCPP_TRACE_INFO("MMC_TrackSyncOut OK")
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
/// \fn void GetMotionInfo
/// \brief This function return fb motion information.
/// \param  reference for parameters data structure.

////////////////////////////////////////////////////////////////////////////////
void CMMCGroupAxis::GetMotionInfo(MMC_MOTIONINFO_OUT& stMotionInfoOut) throw (CMMCException) {
    ELMO_INT32 rc;
    MMC_MOTIONINFO_IN stMotionInfoIn;
    stMotionInfoIn.eMotionInfo = eMOTION_INFO_DATA;
    if ((rc = MMC_GetMotionInfo(m_uiConnHndl, m_usAxisRef, &stMotionInfoIn, &stMotionInfoOut)) != 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("GetMotionInfo: ", m_uiConnHndl, m_usAxisRef, rc, stMotionInfoOut.usErrorID, stMotionInfoOut.usStatus);
}


void CMMCGroupAxis::SetDualHeadCartesianKinematics(MC_DUAL_HEAD_SET& stInputDualRef) throw (CMMCException)
{

    MC_KIN_REF stInput;
    stInput.stDualHead.cAutoOffset = stInputDualRef.cAutoOffset;
    stInput.stDualHead.iNumAxes = 6;
    stInput.stDualHead.dOffsetX2 = stInputDualRef.dOffsetX2;
    stInput.stDualHead.dOffsetY2 = stInputDualRef.dOffsetY2;
    stInput.stDualHead.dOffsetZ2 = stInputDualRef.dOffsetZ2;

    for(int i = 0; i < 6; i++)
    {
        stInput.stDualHead.sNode[i].eType = (NC_AXIS_IN_GROUP_TYPE_ENUM_EX)(NC_ACS_A1_AXIS_TYPE + i);
        stInput.stDualHead.sNode[i].hNode = stInputDualRef.sNode[i].hNode;
        stInput.stDualHead.sNode[i].iMcsToAcsFuncID = stInputDualRef.sNode[i].iMcsToAcsFuncID;
        stInput.stDualHead.sNode[i].ulTrCoef[0] = stInputDualRef.sNode[i].dTrCoef;
        stInput.stDualHead.sNode[i].ulTrCoef[1] = (1 / stInputDualRef.sNode[i].dTrCoef);
        stInput.stDualHead.sNode[i].ulTrCoef[2] = stInputDualRef.sNode[i].dTrOffset;
    }
    SetKinematic(stInput,NC_DUAL_HEAD_TYPE);

}

/*!
* \fn   int CMMCGroupAxis::SetNormalcyMode(MMC_NORMALCY_TYPE_ENUM eType, MMC_NORMALCY_PLANE_ENUM ePlane)
* \brief    this function sets parameters for normalcy mode of operation.
*
*           please note this setting is tightly coupled with selected kinematic.
* \param eType  normalcy mode of operation(disabled if 0).
* \param ePlane selected plane on which the normalcy mode is activated (xy, xz or yz).
* \return - 0 if completed successfully, otherwise error or throws CMMCException
*/
ELMO_INT32 CMMCGroupAxis::SetNormalcyMode(MMC_NORMALCY_TYPE_ENUM eType, MMC_NORMALCY_PLANE_ENUM ePlane)  throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_NORMALCY_PARAMS_IN i_params;
    MMC_NORMALCY_PARAMS_OUT o_params;
    i_params.eType = eType;
    i_params.ePlane = ePlane;
//  fprintf(stderr, "%s: type=%d, plane=%d\n", __PRETTY_FUNCTION__, eType, ePlane);
    if ((rc = MMC_SetNormalcyMode(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("SetNormalcyMode: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
    _MMCPP_TRACE_INFO("SetNormalcyMode OK")
    return rc;
}

/*!
 * \fn int SetNormalcyOff();
 * \brief disable normalcy mode.
 * \return - 0 if completed successfully, otherwise error or throws CMMCException
 */
ELMO_INT32 CMMCGroupAxis::SetNormalcyOff()  throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_NORMALCY_PARAMS_OUT o_params;
//  fprintf(stderr, "%s:\n", __PRETTY_FUNCTION__);
    if ((rc = MMC_SetNormalcyOff(m_uiConnHndl, m_usAxisRef, &o_params)) < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("SetNormalcyOff: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
    _MMCPP_TRACE_INFO("SetNormalcyOff OK")
    return rc;
}

/*!
 * \fn int GetNormalcyMode(MMC_NORMALCY_TYPE_ENUM& eType, MMC_NORMALCY_PLANE_ENUM& ePlane)
 * \brief get normalcy mode of operation and selected plane (xy/xz/yz).
 * \param eType reference of normalcy type (mode).
 * \param ePlane reference of normalcy plane(coordinates).
 * \return - 0 if completed successfully, otherwise error or throws CMMCException
 */
ELMO_INT32 CMMCGroupAxis::GetNormalcyMode(MMC_NORMALCY_TYPE_ENUM& eType, MMC_NORMALCY_PLANE_ENUM& ePlane)  throw (CMMCException)
{
    MMC_NORMALCY_STAUS_OUT o_params;
    ELMO_INT32 rc=0;
//  fprintf(stderr, "%s:\n", __PRETTY_FUNCTION__);
    if ((rc = MMC_GetNormalcyMode(m_uiConnHndl, m_usAxisRef, &o_params)) < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("SetNormalcyOff: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
    _MMCPP_TRACE_INFO("SetNormalcyOff OK")
    eType = o_params.eType;
    ePlane = o_params.ePlane;
    return rc;
}


/*! \fn int MoveAngle(double (&dCenter)[MAX_CENTER_POINTS], double dAngle, MC_COORD_AXES ePlain=NC_XY_AXES, MC_BUFFERED_MODE_ENUM eBufferMode = MC_BUFFERED_MODE, double dHelixPos=0)
 * \brief sends Move Angle command to Maestro for specific Group.
 *
 * \param  dCenter	center points.
 * \param  dAngle	the angle.
 * \param  ePlain	selected plain (default XY).
 * \param  eBufferMode	buffered mode (default buffered).
 * \param  dHelixPos	future use.
 * \return - 0 on success, otherwise throws CMMCException
 */
ELMO_INT32 CMMCGroupAxis::MoveAngle(ELMO_DOUBLE (&dCenter)[MAX_CENTER_POINTS], ELMO_DOUBLE dAngle, MC_COORD_AXES ePlain, MC_BUFFERED_MODE_ENUM eBufferMode, ELMO_DOUBLE dHelixPos) throw (CMMCException)
{
	ELMO_DOUBLE dTransitionParameter[NC_MAX_NUM_AXES_IN_NODE];
	for (ELMO_INT32 k = 0; k < NC_MAX_NUM_AXES_IN_NODE; ++k)
		dTransitionParameter[k]= (ELMO_DOUBLE)m_fTransitionParameter[k];

	return MoveAngle(dCenter, dAngle, m_fVelocity, m_fAcceleration, m_fDeceleration, m_fJerk, dTransitionParameter, m_eTransitionMode, ePlain, eBufferMode, dHelixPos);
}
/*! \fn int MoveAngle(double (&dCenter)[MAX_CENTER_POINTS], double dAngle, ...)
 * \brief sends Move Angle command to Maestro for specific Group.
 *
 * overloaded method
 *
 * \param  dCenter	center points.
 * \param  dAngle	the angle.
 * \param  ePlain	selected plain (default XY).
 * \param  eBufferMode	buffered mode (default buffered).
 * \param  dHelixPos	future use.
 * \return - 0 on success, otherwise throws CMMCException
 */
ELMO_INT32 CMMCGroupAxis::MoveAngle(ELMO_DOUBLE (&dCenter)[MAX_CENTER_POINTS], ELMO_DOUBLE dAngle,
		ELMO_DOUBLE dVelocity,
		ELMO_DOUBLE dAcceleration,
		ELMO_DOUBLE dDeceleration,
		ELMO_DOUBLE dJerk,
		ELMO_DOUBLE (&dTransitionParameter)[NC_MAX_NUM_AXES_IN_NODE],
		NC_TRANSITION_MODE_ENUM eTransitionMode,
		MC_COORD_AXES ePlain, MC_BUFFERED_MODE_ENUM eBufferMode, ELMO_DOUBLE dHelixPos) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_MOVEANGLE_IN i_params;
	MMC_MOVEANGLE_OUT o_params;

	i_params.dAngle = dAngle;
	i_params.dVelocity = dVelocity;
	i_params.dAcceleration = dAcceleration;
	i_params.dDeceleration = dDeceleration;
	i_params.dJerk = dJerk;
	memcpy(i_params.dCenterPoint, dCenter, MAX_CENTER_POINTS * sizeof(ELMO_DOUBLE));
	i_params.eBufferMode = eBufferMode;
	i_params.ePlain = ePlain;
	i_params.eCoordSystem = MC_MCS_COORD;
	i_params.eTransitionMode = eTransitionMode;
	i_params.ucExecute = 1;

	if ((rc = MMC_MoveAngle(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) != 0)
	{
		rc = o_params.usErrorID;
		fprintf(stderr, "%s(%d): error %d\n", __func__, __LINE__, o_params.usErrorID);
		CMMCPPGlobal::Instance()->MMCPPThrow("MoveAngle: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	}

	_MMCPP_TRACE_INFO("MoveAngle OK")

	return rc;
}


#ifdef PROAUT_CHANGES
    //restore compiler switches
    #pragma GCC diagnostic pop
#endif

