// MMCSingleAxis.cpp: implementation of the CMMCSingleAxis class.
// Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
//				  0.4.0 Updated 11Sep2017 Haim H.
//
//////////////////////////////////////////////////////////////////////

//stdafx.h must be at he top of every source file  (.cpp) for windows precompile headers option.

#include "OS_PlatformDependSetting.hpp"
#include "MMCSingleAxis.hpp"

                        /* For SDO Comand functions */
    #define DATALENGTHFIXVAL        4
    #define SLAVEIDFIXVAL           0
    #define SdoBaseOfst             12288
    
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    #define __func__   __FUNCTION__
#endif    

    typedef union FLOAT_LONG_t
    {
            ELMO_LINT32  lData;
            ELMO_FLOAT   fData;
    } FLOAT_OR_LONG;

ELMO_ULINT32	MMCSingledAxisDbgFlag = 0;

MMC_MOTIONPARAMS_SINGLE::MMC_MOTIONPARAMS_SINGLE()
{
    dbPosition      = 0;
    dbDistance      = 0;
    fEndVelocity    = 0;
    fVelocity       = 0;
    fAcceleration   = 0;
    fDeceleration   = 0;
    fJerk           = 0;
    eDirection      = MC_POSITIVE_DIRECTION;
    eBufferMode     = MC_ABORTING_MODE;
    ucExecute       = 1;
    uiExecDelayMs   = 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMMCSingleAxis::CMMCSingleAxis() : CMMCMotionAxis() {
    m_dbTorqueVelocity = 1000;
    m_dbTorqueAcceleration = 10000;
    m_fVelocity = 100000;
    m_fAcceleration = 1000000;
    m_fDeceleration = 1000000;
    m_fJerk = 20000000;
    m_eDirection = MC_POSITIVE_DIRECTION;
    m_ucExecute = 1;
    m_uiExecDelayMs = 0;
    //homing
    m_dbHomePosition = 0;
    m_fHomeAcceleration = 1000000;
    m_fHomeVelocity = 100000;
    m_fHomeDistanceLimit = 100000;
    m_fHomeTorqueLimit = 100000;
    m_eHomingMode = MC_DIRECT;
    m_eHomeBufferMode = MC_ABORTING_MODE;
    m_eHomeDirection = MC_POSITIVE;
    m_eHomeSwitchMode = MC_ON;
    m_uiHomeTimeLimit = 0;
    m_uiHomingMethod = 0;
    m_ucHomeExecute = 1;
}
CMMCSingleAxis::~CMMCSingleAxis(){}

/**
 * copy constructor
 */
CMMCSingleAxis::CMMCSingleAxis(CMMCSingleAxis& axis):CMMCMotionAxis(axis)
{
    m_ucExecute = axis.m_ucExecute;
    m_eDirection = axis.m_eDirection;
    m_fAcceleration = axis.m_fAcceleration;
    m_fDeceleration = axis.m_fDeceleration;
    m_fJerk = axis.m_fJerk;
    m_fVelocity = axis.m_fVelocity;
    m_uiExecDelayMs = axis.m_uiExecDelayMs;
    //homing
    m_dbHomePosition = axis.m_dbHomePosition;
    m_fHomeAcceleration = axis.m_fHomeAcceleration;
    m_fHomeVelocity = axis.m_fHomeVelocity;
    m_fHomeDistanceLimit = axis.m_fHomeDistanceLimit;
    m_fHomeTorqueLimit = axis.m_fHomeTorqueLimit;
    m_eHomingMode = axis.m_eHomingMode;
    m_eHomeBufferMode = axis.m_eHomeBufferMode;
    m_eHomeDirection = axis.m_eHomeDirection;
    m_eHomeSwitchMode = axis.m_eHomeSwitchMode;
    m_uiHomeTimeLimit = axis.m_uiHomeTimeLimit;
    m_uiHomingMethod = axis.m_uiHomingMethod;
    m_ucHomeExecute = axis.m_ucHomeExecute;
    m_dbTorqueVelocity = axis.m_dbTorqueVelocity;
    m_dbTorqueAcceleration = axis.m_dbTorqueAcceleration;
}
//
//  Gets Axis name, and sets axis reference,
//
ELMO_INT32 CMMCSingleAxis::MoveAbsolute(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEABSOLUTE_IN     stInParams;
    MMC_MOVEABSOLUTE_OUT    stOutParams;

    stInParams.dbPosition       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.fVelocity        = m_fVelocity;
    stInParams.fAcceleration    = m_fAcceleration ;
    stInParams.fDeceleration    = m_fDeceleration ;
    stInParams.fJerk            = m_fJerk ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveAbsoluteCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsolute OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveAbsolute(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    rc = MoveAbsolute(dPos,eBufferMode);
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAbsolute(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration,  MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    rc = MoveAbsolute(dPos,eBufferMode);
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAbsolute(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MoveAbsolute(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveAdditive(ELMO_DOUBLE dbDistance, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEADDITIVE_IN stInParams;
    MMC_MOVEADDITIVE_OUT    stOutParams;

    stInParams.dbDistance = dbDistance ;
    stInParams.eBufferMode = eBufferMode;
    stInParams.fVelocity    = m_fVelocity ;
    stInParams.eDirection = m_eDirection;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.ucExecute = m_ucExecute;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveAdditiveCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditive OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAdditive(ELMO_DOUBLE dbDistance, ELMO_FLOAT fVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    rc = MoveAdditive(dbDistance,eBufferMode);
    return rc;
}

ELMO_INT32 CMMCSingleAxis::MoveAdditive(ELMO_DOUBLE dbDistance, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;    
    rc = MoveAdditive(dbDistance,eBufferMode);
    return rc;
}

ELMO_INT32 CMMCSingleAxis::MoveAdditive(ELMO_DOUBLE dbDistance, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MoveAdditive(dbDistance,eBufferMode);
    return rc;
}

/*! \fn int MoveRelative(double dbDistance)
*   \brief This function send Move Reletive command to MMC server for specific Axis.
*   \param dPos target position to move.
*   \return return - 0 if success. error_id in case of error
*/
ELMO_INT32 CMMCSingleAxis::MoveRelative(ELMO_DOUBLE dbDistance, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVERELATIVE_IN stInParams;
    MMC_MOVERELATIVE_OUT    stOutParams;
    ELMO_INT32 rc = 0;
    stInParams.dbDistance = dbDistance ;
    stInParams.fVelocity    = m_fVelocity ;
    stInParams.eBufferMode = eBufferMode;
    stInParams.eDirection = m_eDirection;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.ucExecute = m_ucExecute;
    rc = MMC_MoveRelativeCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveRelativeCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelative OK")
    return rc ;
}

/*! \fn int MoveRelative(double dbDistance, float fVel)
*   \brief This function send Move Relitivecommand to MMC server for specific Axis.
*   \param dPos target position to move.
*   \param fVal desired valocity for movment.
*   \return return - 0 if success. error_id in case of error
*/
ELMO_INT32 CMMCSingleAxis::MoveRelative(ELMO_DOUBLE dbDistance, ELMO_FLOAT fVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel ;
    rc = MoveRelative(dbDistance,eBufferMode);
    return rc;
}

ELMO_INT32 CMMCSingleAxis::MoveRelative(ELMO_DOUBLE dbDistance, ELMO_FLOAT fVel,  ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel ;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    rc = MoveRelative(dbDistance,eBufferMode);
    return rc;
}

ELMO_INT32 CMMCSingleAxis::MoveRelative(ELMO_DOUBLE dbDistance, ELMO_FLOAT fVel,  ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel ;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MoveRelative(dbDistance,eBufferMode);
    return rc;
}

ELMO_INT32 CMMCSingleAxis::MoveVelocity(ELMO_FLOAT fVelocity, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEVELOCITY_IN     stInParams;
    MMC_MOVEVELOCITY_OUT    stOutParams;

    stInParams.fVelocity = fVelocity ;
    m_fVelocity = fVelocity;
    stInParams.eBufferMode = eBufferMode;
    stInParams.eDirection = m_eDirection;
    stInParams.fAcceleration = m_fAcceleration;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.ucExecute = m_ucExecute;

    ELMO_INT32 rc = 0;
    rc = MMC_MoveVelocityCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveVelocityCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveVelocity OK")
    return rc ;
}

void CMMCSingleAxis::SetDefaultHomeParams(const MMC_HOME_IN& stSingleParams)
{
    //
    // Copy default parameters
    //
    m_dbHomePosition = stSingleParams.dbPosition;
    m_eHomeBufferMode = stSingleParams.eBufferMode;
    m_fHomeAcceleration = stSingleParams.fAcceleration;
    m_fHomeDistanceLimit = stSingleParams.fDistanceLimit;
    m_fHomeTorqueLimit = stSingleParams.fTorqueLimit;
    m_fHomeVelocity = stSingleParams.fVelocity;
    m_ucHomeExecute= stSingleParams.ucExecute;
    m_uiHomeTimeLimit = stSingleParams.uiTimeLimit;
    m_eHomingMode = stSingleParams.eHomingMode;
    m_eHomeDirection = stSingleParams.eDirection;
    m_eHomeSwitchMode = stSingleParams.eSwitchMode;
}

void CMMCSingleAxis::SetDefaultHomeDS402Params(const MMC_HOMEDS402_IN& stSingleParams)
{
    //
    // Copy default parameters
    //
    m_dbHomePosition = stSingleParams.dbPosition;
    m_eHomeBufferMode = stSingleParams.eBufferMode;
    m_fHomeAcceleration = stSingleParams.fAcceleration;
    m_fHomeDistanceLimit = stSingleParams.fDistanceLimit;
    m_fHomeTorqueLimit = stSingleParams.fTorqueLimit;
    m_fHomeVelocity = stSingleParams.fVelocity;
    m_ucHomeExecute= stSingleParams.ucExecute;
    m_uiHomingMethod = stSingleParams.uiHomingMethod;
    m_uiHomeTimeLimit = stSingleParams.uiTimeLimit;
}

void CMMCSingleAxis::SetDefaultHomeDS402ExParams(const MMC_HOMEDS402EX_IN& stParams)
{
    //
    // Copy default parameters
    //
    m_dbHomePosition = stParams.dbPosition;
    m_eHomeBufferMode = stParams.eBufferMode;
    m_fHomeAcceleration = stParams.fAcceleration;
    m_fHomeDistanceLimit = stParams.fDistanceLimit;
    m_fHomeTorqueLimit = stParams.fTorqueLimit;
    m_fHomeVelocity = stParams.fVelocityHi; //Speed during search for switch
    m_ucHomeExecute= stParams.ucExecute;
    m_uiHomingMethod = stParams.uiHomingMethod;
    m_uiHomeTimeLimit = stParams.uiTimeLimit;
    m_fHomeVelocityLo = stParams.fVelocityLo;
    m_uiDetectionTimeLimit = stParams.uiDetectionTimeLimit;
    m_dbDetectionVelocityLimit  = stParams.dbDetectionVelocityLimit;
}


void CMMCSingleAxis::SetDefaultParams(const MMC_MOTIONPARAMS_SINGLE& stSingleParams)
{
    //
    // Copy default parameters
    //
    m_fVelocity         = stSingleParams.fVelocity;
    m_fAcceleration     = stSingleParams.fAcceleration;
    m_fDeceleration     = stSingleParams.fDeceleration;
    m_fJerk             = stSingleParams.fJerk;
    m_eDirection        = stSingleParams.eDirection;
    m_ucExecute         = stSingleParams.ucExecute;
    m_uiExecDelayMs     = stSingleParams.uiExecDelayMs;
}

/*! \fn int SetOpMode(OPM402 eMode)
*   \brief This function changes motion mode between NC/Non NC.
*   \param eMode operational mode.
*   \return return - 0 on success, otherwise throws CMMCException.
*/
ELMO_INT32 CMMCSingleAxis::SetOpMode(OPM402 eMode) throw (CMMCException)
{
    MMC_CHANGEMOTIONMODE_IN chng_opmode_in;
    MMC_CHANGEMOTIONMODE_OUT chng_opmode_out;
    ELMO_INT32 rc;
    chng_opmode_in.ucMotionMode= (ELMO_UINT8)eMode;
    if ((rc = MMC_ChngOpMode(m_uiConnHndl,m_usAxisRef, &chng_opmode_in,&chng_opmode_out))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ChngOpMode:", m_uiConnHndl, m_usAxisRef, rc, chng_opmode_out.usErrorID, chng_opmode_out.usStatus);
    _MMCPP_TRACE_INFO("SetOpMode OK")
    return(0);
}

/*! \fn int SetOpMode(OPM402 eMode)
*   \brief This function changes motion mode between NC/Non NC.
*   \param eMode operational mode.
*   \return return - 0 on success, otherwise throws CMMCException.
*/
void CMMCSingleAxis::SetOpMode(OPM402 eMode, MC_EXECUTION_MODE eExecutionMode,  ELMO_DOUBLE dbInitModeValue) throw (CMMCException)
{
    MMC_CHANGEOPMODE_EX_IN chng_opmode_in;
    MMC_CHANGEOPMODE_EX_OUT chng_opmode_out;
    ELMO_INT32 rc;
    //
    chng_opmode_in.ucMotionMode = (ELMO_UINT8)eMode;
    chng_opmode_in.eExecutionMode = eExecutionMode;
    chng_opmode_in.dbInitModeValue = dbInitModeValue;
    //
    if ((rc = MMC_ChangeOpModeEx(m_uiConnHndl,m_usAxisRef, &chng_opmode_in,&chng_opmode_out))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ChangeOpModeFB:", m_uiConnHndl, m_usAxisRef, rc, chng_opmode_out.usErrorID, chng_opmode_out.usStatus);
    }
    _MMCPP_TRACE_INFO("ChangeOpModeEX OK")
    return;
}

/*! \fn OPM402 GetOpMode()
* \brief This function gets motion mode between NC/Non NC.
* \return   return - 0 on success, otherwise throws CMMCException.
*/
OPM402 CMMCSingleAxis::GetOpMode() throw (CMMCException)
{
    MMC_READBOOLPARAMETER_IN read_bparam_in;
    MMC_READBOOLPARAMETER_OUT read_bparam_out;
    ELMO_INT32 rc;
    read_bparam_in.eParameterNumber     = MMC_AXIS_OP_MOTION_MODE_PARAM;  // Motion mode by DS402
    read_bparam_in.iParameterArrIndex   = 0;
    read_bparam_in.ucEnable             = 1;

    if ((rc=MMC_ReadBoolParameter(m_uiConnHndl, m_usAxisRef, &read_bparam_in, &read_bparam_out))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("GetOpMode:", m_uiConnHndl, m_usAxisRef, rc, read_bparam_out.usErrorID, read_bparam_out.usStatus);

    _MMCPP_TRACE_INFO("GetOpMode OK")
    return (OPM402)read_bparam_out.lValue ;
}



/*! \fn unsigned int GetStatusRegister()
* \brief This function returns the status register
* \return   return - 0 on success, otherwise throws CMMCException.
*/
ELMO_UINT32 CMMCSingleAxis::GetStatusRegister() throw (CMMCException)
{
    MMC_GETSTATUSREGISTER_OUT get_param_out;
    return GetStatusRegister(get_param_out);
}



/*! \fn unsigned int GetStatusRegister(MMC_GETSTATUSREGISTER_OUT& sOutput)
* \brief This function returns the status register output structure
* \return   return - 0 on success, otherwise throws CMMCException.
*/
ELMO_UINT32 CMMCSingleAxis::GetStatusRegister(MMC_GETSTATUSREGISTER_OUT& sOutput) throw (CMMCException)
{
    MMC_GETSTATUSREGISTER_IN get_param_in;
    ELMO_INT32 rc;
    //
    if ((rc=MMC_GetStatusRegisterCmd(m_uiConnHndl, m_usAxisRef, &get_param_in, &sOutput))!=0)
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetStatusRegister:", m_uiConnHndl, m_usAxisRef, rc, sOutput.usErrorID, sOutput.usStatus);

    _MMCPP_TRACE_INFO("MMC_GetStatusRegister OK")
    return sOutput.uiStatusRegister;
}


/*! \fn void PowerOn()
* \brief This function sends Power On command to MMC server.
* \return   return - none. throws CMMCException on error.
*/
void CMMCSingleAxis::PowerOn(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_POWER_IN power_in;
    MMC_POWER_OUT power_out;
    ELMO_INT32 rc;
    power_in.ucEnable             = 1;
    power_in.ucEnablePositive     = 1;
    power_in.ucEnableNegative     = 0;
    power_in.eBufferMode          = eBufferMode;
    power_in.ucExecute            = 1;
    //
    if ((rc = MMC_PowerCmd(m_uiConnHndl, m_usAxisRef, &power_in, &power_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PowerCmd:", m_uiConnHndl, m_usAxisRef, rc, power_out.usErrorID, power_out.usStatus);
    }
    _MMCPP_TRACE_INFO("PowerOn OK")
}

/*! \fn void PowerOff()
* \brief This function sends Power Off command to MMC server.
* \return   return - none. throws CMMCException on error.
*/
void CMMCSingleAxis::PowerOff(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_POWER_IN power_in;
    MMC_POWER_OUT power_out;
    ELMO_INT32 rc;
    power_in.ucEnable             = 0;
    power_in.ucEnablePositive     = 1;
    power_in.ucEnableNegative     = 0;
    power_in.eBufferMode          = eBufferMode;
    power_in.ucExecute            = 1;
    //
    if ((rc = MMC_PowerCmd(m_uiConnHndl, m_usAxisRef, &power_in, &power_out)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PowerCmd:", m_uiConnHndl, m_usAxisRef, rc, power_out.usErrorID, power_out.usStatus);
    }
    _MMCPP_TRACE_INFO("PowerOff OK")
}

/*! \fn double mGetActualPosition()
 * \brief This function reads actual position of this axis.
 * \param ucEnable - default 1.
 * \return - actual position on success, otherwise throws CMMCException.
 */
ELMO_DOUBLE CMMCSingleAxis::GetActualPosition() throw (CMMCException)
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

/*! \fn double mGetActualVelocity()
 * \brief This function reads actual velocity of this axis.
 * \param ucEnable - default 1.
 * \return - actual velocity on success, otherwise throws CMMCException.
 */
ELMO_DOUBLE CMMCSingleAxis::GetActualVelocity() throw (CMMCException)
{
    MMC_READACTUALVELOCITY_IN stInParam;
    MMC_READACTUALVELOCITY_OUT stOutParam;
    ELMO_INT32 rc;

    stInParam.ucEnable = 1;
    if ((rc=MMC_ReadActualVelocityCmd(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadActualVelocityCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetActualVelocity OK")
    return stOutParam.dVelocity;
}

/*! \fn double mGetActualTorque()
 * \brief This function reads actual torque of this axis.
 * \return - actual torque on success, otherwise throws CMMCException.
 */
ELMO_DOUBLE CMMCSingleAxis::GetActualTorque() throw (CMMCException)
{
    MMC_READACTUALTORQUE_IN stInParam;
    MMC_READACTUALTORQUE_OUT stOutParam;
    ELMO_INT32 rc;

    stInParam.ucEnable = 1;
    if ((rc=MMC_ReadActualTorqueCmd(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadActualTorqueCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("GetActualTorque OK")
    return stOutParam.dActualTorque;
}

/*! \fn void mHalt()
 * \brief This function  send Halt command to MMC server for this axis.
 * \return - none on success, otherwise throws CMMCException
 */
void CMMCSingleAxis::Halt(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_HALT_IN stInParams;
    MMC_HALT_OUT stOutParams;
    ELMO_INT32 rc;

    stInParams.eBufferMode = eBufferMode;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk = m_fJerk;
    stInParams.ucExecute = m_ucExecute;

    if ((rc=MMC_HaltCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HaltCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("Halt OK")
    return;
}
void CMMCSingleAxis::Halt(ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    m_fDeceleration = fDeceleration;
    Halt(eBufferMode);
}
void CMMCSingleAxis::Halt(ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    Halt(eBufferMode);
}

/*! \fn void mStop()
 * \brief This function sends Stop command to MMC server for this axis.
 * \return - none on success, otherwise throws CMMCException
 */

void CMMCSingleAxis::Stop(MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_STOP_IN stInParams;
    MMC_STOP_OUT stOutParams;
    ELMO_INT32 rc;

    stInParams.eBufferMode  = eBufferMode;
    stInParams.fDeceleration = m_fDeceleration;
    stInParams.fJerk            = m_fJerk;
    stInParams.ucExecute        = 1;

    if ((rc=MMC_StopCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_StopCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("Stop OK")
}
void CMMCSingleAxis::Stop(ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    m_fDeceleration = fDeceleration;
    Stop(eBufferMode);
}
void CMMCSingleAxis::Stop(ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    Stop(eBufferMode);
}

/*! \fn int iGetDigInput(int iInputNumber)
 * \brief This function  return value of specific bit digital input.
 * \param iInputNumber -  bit number.
 * \return - value of specified bit digital input success, otherwise throws CMMCException
 */
ELMO_UINT8  CMMCSingleAxis::GetDigInput(ELMO_INT32 iInputNumber) throw (CMMCException)
{
    MMC_READDIGITALINPUT_IN stInParam;
    MMC_READDIGITALINPUT_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.iInputNumber = iInputNumber;
    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadDigitalInputCmd(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadDigitalInputCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetDigInput OK")
    return stOutParam.ucValue;
}

/*! \fn unsigned long iGetDigInputs()
 * \brief This function  return value of specific bit digital input.
 * \return - long value with all digital input bits, otherwise throws CMMCException.
 */
ELMO_ULINT32 CMMCSingleAxis::GetDigInputs() throw (CMMCException)
{
    MMC_READDIGITALINPUTS_IN  stInParam;
    MMC_READDIGITALINPUTS_OUT stOutParam;
    ELMO_INT32 rc;

    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadDigitalInputsCmd(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadDigitalInputsCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetDigInputs OK")
    return stOutParam.ulValue;
}

/*! \fn unsigned long iGetDigOutputs32bit(int iOutputNumber)
 * \brief The function returns the actual Digital Output for specific Node.
 * \param iOutputNumber - digital output number.
 * \return - digital output for the specified node on success, otherwise throws MMCEception.
 */
ELMO_ULINT32 CMMCSingleAxis::GetDigOutputs32bit(ELMO_INT32 iOutputNumber, ELMO_UINT8 ucEnable) throw (CMMCException) {
    MMC_READDIGITALOUTPUT32Bit_IN   stInParam;
    MMC_READDIGITALOUTPUT32Bit_OUT  stOutParam;
    stInParam.iOutputNumber = iOutputNumber;
    stInParam.ucEnable = ucEnable;
    ELMO_INT32 rc;
    if ((rc=MMC_ReadDigitalOutputs32Bit(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadDigitalOutputs32Bit:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetDigOutputs32bit OK")
    return stOutParam.ulValue;
}

ELMO_ULINT32 CMMCSingleAxis::GetDigOutputs32bit(ELMO_INT32 iOutputNumber) throw (CMMCException)
{
    return GetDigOutputs32bit(iOutputNumber,1);
}

/*! \fn
 * \brief This function  sends Home DS402 command to MMC server for this axis.
 *  it uses default parameters for HomeDS402.
 * \param usErrorID - reference to store command error.
 * \param uiHndl -  reference to store handler.
 * \return - status and output parameters on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::HomeDS402() throw (CMMCException)
{
    MMC_HOMEDS402_IN    stInParams;
    MMC_HOME_OUT    stOutParams;

    stInParams.dbPosition = m_dbHomePosition;
    stInParams.eBufferMode = m_eHomeBufferMode;
    stInParams.fAcceleration = m_fHomeAcceleration;
    stInParams.fDistanceLimit = m_fHomeDistanceLimit;
    stInParams.fTorqueLimit = m_fHomeTorqueLimit;
    stInParams.fVelocity = m_fHomeVelocity;
    stInParams.ucExecute = m_ucHomeExecute;
    stInParams.uiHomingMethod = m_uiHomingMethod;
    stInParams.uiTimeLimit = m_uiHomeTimeLimit;

    ELMO_INT32 rc;
    if ((rc=MMC_HomeDS402Cmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HomeDS402Cmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }

    _MMCPP_TRACE_INFO("HomeDS402 OK")
}


/*! \fn
 * \brief This function  sends Home DS402Ex command to MMC server for this axis.
 *  it uses default parameters for HomeDS402Ex.
 * \param usErrorID - reference to store command error.
 * \param uiHndl -  reference to store handler.
 * \return - status and output parameters on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::HomeDS402Ex() throw (CMMCException)
{
    MMC_HOMEDS402EX_IN  stInParams;
    MMC_HOME_OUT    stOutParams;

    stInParams.dbPosition = m_dbHomePosition;
    stInParams.eBufferMode = m_eHomeBufferMode;
    stInParams.fAcceleration = m_fHomeAcceleration;
    stInParams.fDistanceLimit = m_fHomeDistanceLimit;
    stInParams.fTorqueLimit = m_fHomeTorqueLimit;
    stInParams.fVelocityHi = m_fHomeVelocity;
    stInParams.ucExecute = m_ucHomeExecute;
    stInParams.uiHomingMethod = m_uiHomingMethod;
    stInParams.uiTimeLimit = m_uiHomeTimeLimit;

    stInParams.fVelocityLo = m_fHomeVelocityLo;
    stInParams.uiDetectionTimeLimit = m_uiDetectionTimeLimit;
    stInParams.dbDetectionVelocityLimit = m_dbDetectionVelocityLimit;


    ELMO_INT32 rc;
    if ((rc=MMC_HomeDS402ExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HomeDS402ExCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }

    _MMCPP_TRACE_INFO("HomeDS402 OK")
}


/*! \fn unsigned short mHomeDS402(MMC_HOMEDS402_IN stHomeDS402Params, short* usErrorID, unsigned int* uiHndl)
 * \brief This function  sends Home DS402 command to MMC server for this axis.
 * \param stHomeDS402Params - overloads default parameters.
 * \param usErrorID - [out] reference to store command error.
 * \param uiHndl -  [out] reference to store handler.
 * \return - status and output parameters on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::HomeDS402(MMC_HOMEDS402_IN stHomeDS402Params) throw (CMMCException)
{
    MMC_HOME_OUT    stOutParam;
    ELMO_INT32 rc;
    SetDefaultHomeDS402Params(stHomeDS402Params);

    if ((rc=MMC_HomeDS402Cmd(m_uiConnHndl, m_usAxisRef, &stHomeDS402Params, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HomeDS402Cmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("HomeDS402 OK")
}


/*! \fn unsigned short mHomeDS402(MMC_HOMEDS402_IN stHomeDS402Params, short* usErrorID, unsigned int* uiHndl)
 * \brief This function  sends Home DS402 command to MMC server for this axis.
 * \param stHomeDS402Params - overloads default parameters.
 * \param usErrorID - [out] reference to store command error.
 * \param uiHndl -  [out] reference to store handler.
 * \return - status and output parameters on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::HomeDS402Ex(MMC_HOMEDS402EX_IN stHomeDS402Params) throw (CMMCException)
{
    MMC_HOME_OUT    stOutParam;
    ELMO_INT32 rc;
    SetDefaultHomeDS402ExParams(stHomeDS402Params);

    if ((rc=MMC_HomeDS402ExCmd(m_uiConnHndl, m_usAxisRef, &stHomeDS402Params, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HomeDS402ExCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("HomeDS402 OK")
}



/*! \fn int Home(short* usErrorID, unsigned int* uiHndl)
 * \brief this function sends Home command to MMC server for this axis.
 * \param usErrorID - [out] reference to store command error.
 * \param uiHndl -  [out] reference to store handler.
 * \return - status output parameters on success otherwise throws CMMCException.
*/
ELMO_UINT16 CMMCSingleAxis::Home(ELMO_PINT16 usErrorID, ELMO_PUINT32 uiHndl) throw (CMMCException)
{
    MMC_HOME_IN stInParams;
    MMC_HOME_OUT    stOutParams;
    ELMO_INT32 rc;
    stInParams.dbPosition = m_dbHomePosition;
    stInParams.eBufferMode = m_eHomeBufferMode;
    stInParams.fAcceleration = m_fHomeAcceleration;
    stInParams.fDistanceLimit = m_fHomeDistanceLimit;
    stInParams.fTorqueLimit = m_fHomeTorqueLimit;
    stInParams.fVelocity = m_fHomeVelocity;
    stInParams.ucExecute = m_ucHomeExecute;
    stInParams.uiTimeLimit = m_uiHomeTimeLimit;
    stInParams.eHomingMode = m_eHomingMode;
    stInParams.eDirection = m_eHomeDirection;
    stInParams.eSwitchMode = m_eHomeSwitchMode;

    if ((rc=MMC_HomeCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HomeCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    *usErrorID = stOutParams.usErrorID;
    *uiHndl = stOutParams.uiHndl;
    _MMCPP_TRACE_INFO("Home OK")
    return stOutParams.usStatus;
}

/*! \fn int Home(short* usErrorID, unsigned int* uiHndl)
* \brief This function  send Home command to MMC server for specific Axis.
* \return   return - status if success then error and handler in out parameters usErrorID and uiHndl.
*       otherwise throws CMMCException.
*/
ELMO_UINT16 CMMCSingleAxis::Home(MMC_HOME_IN stHomeParams, ELMO_PINT16 usErrorID, ELMO_PUINT32 uiHndl) throw (CMMCException)
{
    MMC_HOME_OUT    stOutParam;
    ELMO_INT32 rc;
    SetDefaultHomeParams(stHomeParams);

    if ((rc=MMC_HomeCmd(m_uiConnHndl, m_usAxisRef, &stHomeParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_HomeCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    *usErrorID = stOutParam.usErrorID;
    *uiHndl = stOutParam.uiHndl;

    _MMCPP_TRACE_INFO("Home OK")
    return stOutParam.usStatus;
}

/*! \fn void iSetDigOutputs32Bit(const int iOutputNumber, const unsigned long ulValue)
 * \brief - The function sets the actual digital output for specific Node.
 * \parma iOutputNumber - digital output number.
 * \param ulValue - value to set.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::SetDigOutputs32Bit(const ELMO_INT32 iOutputNumber, const ELMO_ULINT32 ulValue) throw (CMMCException)
{
    MMC_WRITEDIGITALOUTPUT32Bit_IN stInParams;
    MMC_WRITEDIGITALOUTPUT32Bit_OUT stOutParam;
    ELMO_INT32 rc;
    stInParams.iOutputNumber = iOutputNumber;
    stInParams.ulValue = ulValue;
    stInParams.ucEnable = 1;
    if ((rc=MMC_WriteDigitalOutputs32Bit(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteDigitalOutputs32Bit:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("SetDigOutputs32Bit OK")
    
    return;
}


/*! \fn void iSetDigOutputs32Bit(const int iOutputNumber, const unsigned long ulValue)
 * \brief - The function sets the actual digital output for specific Node.
 * \parma iOutputNumber - digital output number.
 * \param ulValue - value to set.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::SetDigOutputs32Bit(const ELMO_ULINT32 ulValue) throw (CMMCException)
{
    SetDigOutputs32Bit(0,ulValue);
    return;
}


/*! \fn void SetOverride(float fAccFactor, float fJerkFactor, float fVelFactor)
* \brief this method send Set Override command to MMC server for specific Axis.
* \param fAccFactor - acceleration factor.
* \param fJerkFactor - jerk factor.
* \param fVelFactor - velocity factor.
* \return   return - none. throws CMMCException on error.
*/
void CMMCSingleAxis::SetOverride(ELMO_FLOAT fAccFactor, ELMO_FLOAT fJerkFactor, ELMO_FLOAT fVelFactor, ELMO_UINT16 usUpdateVelFactorIdx) throw (CMMCException)
{
    MMC_SETOVERRIDE_IN stInParam;
    MMC_SETOVERRIDE_OUT stOutParam;
    ELMO_INT32 rc;

    stInParam.fAccFactor =  fAccFactor;
    stInParam.fJerkFactor = fJerkFactor;
    stInParam.fVelFactor = fVelFactor;
    stInParam.ucEnable = 1;
    stInParam.usUpdateVelFactorIdx = usUpdateVelFactorIdx;


    if ((rc=MMC_SetOverrideCmd(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetOverrideCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetOverride OK");
}

/*! \fn unsigned char CMMCSingleAxis::GetDigOutputs(int iOutputNumber)
 * \brief the method returns the actual digital output for specific node.
 * \param iOutputNumber - number that specifies the output digital parameter.
 * \return - value of specified parameter on success, otherwise throws CMMCException.
 */
ELMO_UINT8 CMMCSingleAxis::GetDigOutputs(ELMO_INT32 iOutputNumber) throw (CMMCException)
{
    MMC_READDIGITALOUTPUT_IN    stInParam;
    MMC_READDIGITALOUTPUT_OUT   stOutParam;
    ELMO_INT32 rc;
    stInParam.iOutputNumber = iOutputNumber;
    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadDigitalOutputs(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadDigitalOutputs:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("GetDigOutputs OK")
    return stOutParam.ucValue;
}

/*! \fn void iSetDigOutputs(const int iOutputNumber, const unsigned char ucValue,  unsigned char ucEnable)
 * \brief this method sets the actual digital output for specific node.
 * \param iOutputNumber - number that specifies the output digital parameter.
 * \param ucValue - digital output value to set.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::SetDigOutputs(const ELMO_INT32 iOutputNumber, const ELMO_UINT8 ucValue, ELMO_UINT8 ucEnable) throw (CMMCException) {
    MMC_WRITEDIGITALOUTPUT_IN stInParams;
    ELMO_INT32 rc;
    stInParams.iOutputNumber = iOutputNumber;
    stInParams.ucValue = ucValue;
    stInParams.ucEnable = ucEnable;

    MMC_WRITEDIGITALOUTPUT_OUT stOutParam;

    if ((rc=MMC_WriteDigitalOutputs(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteDigitalOutputs:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetDigOutputs OK")
    return;
}

/*! \fn void iSetDigOutputs(const int iOutputNumber, const unsigned char ucValue)
 * \brief this method sets the actual digital output for specific node.
 * \param iOutputNumber - number that specifies the output digital parameter.
 * \param ucValue - digital output value to set.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::SetDigOutputs(const ELMO_INT32 iOutputNumber, const ELMO_UINT8 ucValue) throw (CMMCException)
{
    SetDigOutputs(iOutputNumber,ucValue,1);
    return;
}


/*! \fn void iSetDigOutputs(const int iOutputNumber, const unsigned char ucValue)
 * \brief this method sets the actual digital output for specific node.
 * \param iOutputNumber - number that specifies the output digital parameter.
 * \param ucValue - digital output value to set.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::SetDigOutputs(const ELMO_UINT8 ucValue) throw (CMMCException)
{
    SetDigOutputs(0,ucValue,1);
    return;
}


/*! \fn void eConfigPDO(PDO_NUMBER_ENUM ePDONum, PDO_PARAM_TYPE_ENUM eParamType,
        unsigned int uiPDOCommParamEvent,
        unsigned short usEventTimer,
        unsigned char ucEventGroup,
        unsigned char ucPDOCommParam,
        unsigned char ucSubIndex = 1,
        unsigned char ucPDOType = 1)
 * \brief this method configures regular parameter event PDO3/4 according group and parameter type.
 * \param ePDONum - defines PDO3 or PDO4
 * \param eParamType - defines regular or user
 * \param uiPDOCommParamEvent -
 * \param usEventTimer -
 * \param ucEventGroup -
 * \param ucPDOCommParam -
 * \param ucSubIndex -
 * \param ucPDOType -
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::ConfigPDO(PDO_NUMBER_ENUM ePDONum, PDO_PARAM_TYPE_ENUM eParamType,
        ELMO_UINT32 uiPDOCommParamEvent,
        ELMO_UINT16 usEventTimer,
        ELMO_UINT8 ucEventGroup,
        ELMO_UINT8 ucPDOCommParam,
        ELMO_UINT8 ucSubIndex = 1,
        ELMO_UINT8 ucPDOType = 1)  throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_CONFIGREGULARPARAMEVENTPDO3_IN stInRegPDO3;
    MMC_CONFIGREGULARPARAMEVENTPDO3_OUT stOutRegPDO3;
    MMC_CONFIGREGULARPARAMEVENTPDO4_IN stInRegPDO4;
    MMC_CONFIGREGULARPARAMEVENTPDO4_OUT stOutRegPDO4;
    MMC_CONFIGUSERPARAMEVENTPDO3_IN pInUserPDO3;
    MMC_CONFIGUSERPARAMEVENTPDO3_OUT pOutUserPDO3;
    MMC_CONFIGUSERPARAMEVENTPDO4_IN pInUserPDO4;
    MMC_CONFIGUSERPARAMEVENTPDO4_OUT pOutUserPDO4;

    if (ePDONum == PDO_NUM_3) {
        if (eParamType == PDO_PARAM_REG) {
            stInRegPDO3.ucEventGroup = ucEventGroup;
            stInRegPDO3.ucPDOCommParam = ucPDOCommParam;
            stInRegPDO3.uiPDOCommParamEvent = uiPDOCommParamEvent;
            stInRegPDO3.usEventTimer = usEventTimer;
            if ((rc=MMC_CfgRegParamEvPDO3Cmd(m_uiConnHndl, m_usAxisRef, &stInRegPDO3, &stOutRegPDO3))!=0)
            {
                CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CfgRegParamEvPDO3Cmd:", m_uiConnHndl, m_usAxisRef, rc, stOutRegPDO3.usErrorID, stOutRegPDO3.usStatus);
            }
            _MMCPP_TRACE_INFO("ConfigPDO OK")
        } else { //PDO_PARAM_USER
            pInUserPDO3.ucEventGroup = ucEventGroup;
            pInUserPDO3.ucPDOCommParam = ucPDOCommParam;
            pInUserPDO3.uiPDOCommParamEvent = uiPDOCommParamEvent;
            pInUserPDO3.usEventTimer = usEventTimer;
            pInUserPDO3.ucSubIndex = ucSubIndex;
            pInUserPDO3.ucPDOType = ucPDOType;
            if ((rc=MMC_CfgUserParamEvPDO3Cmd(m_uiConnHndl, m_usAxisRef, &pInUserPDO3, &pOutUserPDO3))!=0)
            {
                CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CfgUserParamEvPDO3Cmd:", m_uiConnHndl, m_usAxisRef, rc, pOutUserPDO3.usErrorID, pOutUserPDO3.usStatus);
            }
            _MMCPP_TRACE_INFO("ConfigPDO OK")
        }
    } else {//(ePDONum == PDO_NUM_4) {
        if (eParamType == PDO_PARAM_REG) {
            stInRegPDO4.ucEventGroup = ucEventGroup;
            stInRegPDO4.ucPDOCommParam = ucPDOCommParam;
            stInRegPDO4.uiPDOCommParamEvent = uiPDOCommParamEvent;
            stInRegPDO4.usEventTimer = usEventTimer;
            if ((rc=MMC_CfgRegParamEvPDO4Cmd(m_uiConnHndl, m_usAxisRef, &stInRegPDO4, &stOutRegPDO4))!=0)
            {
                CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CfgRegParamEvPDO4Cmd:", m_uiConnHndl, m_usAxisRef, rc, stOutRegPDO4.usErrorID, stOutRegPDO4.usStatus);
            }
            _MMCPP_TRACE_INFO("ConfigPDO OK")

        } else { //PDO_PARAM_USER
            pInUserPDO4.ucEventGroup = ucEventGroup;
            pInUserPDO4.ucPDOCommParam = ucPDOCommParam;
            pInUserPDO4.uiPDOCommParamEvent = uiPDOCommParamEvent;
            pInUserPDO4.usEventTimer = usEventTimer;
            pInUserPDO4.ucSubIndex = ucSubIndex;
            pInUserPDO4.ucPDOType = ucPDOType;
            if ((rc=MMC_CfgUserParamEvPDO4Cmd(m_uiConnHndl, m_usAxisRef, &pInUserPDO4, &pOutUserPDO4))!=0)
            {
                CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CfgUserParamEvPDO4Cmd:", m_uiConnHndl, m_usAxisRef, rc, pOutUserPDO4.usErrorID, pOutUserPDO4.usStatus);
            }
            _MMCPP_TRACE_INFO("ConfigPDO OK")
        }
    }
}

void CMMCSingleAxis::CancelPDO(PDO_NUMBER_ENUM ePDONum) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_CANCELPARAMEVENTPDO3_IN  stInRegPDO3;
    MMC_CANCELPARAMEVENTPDO3_OUT stOutRegPDO3;
    MMC_CANCELPARAMEVENTPDO4_IN  stInRegPDO4;
    MMC_CANCELPARAMEVENTPDO4_OUT stOutRegPDO4;

    if (ePDONum == PDO_NUM_3) {
        if ((rc=MMC_CancelParamEvPDO3Cmd(m_uiConnHndl, m_usAxisRef, &stInRegPDO3, &stOutRegPDO3))!=0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelParamEvPDO3Cmd:", m_uiConnHndl, m_usAxisRef, rc, stOutRegPDO3.usErrorID, stOutRegPDO3.usStatus);
        }
        _MMCPP_TRACE_INFO("CancelPDO OK")
    } else {//(ePDONum == PDO_NUM_4) {
        if ((rc=MMC_CancelParamEvPDO4Cmd(m_uiConnHndl, m_usAxisRef, &stInRegPDO4, &stOutRegPDO4))!=0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelParamEvPDO4Cmd:", m_uiConnHndl, m_usAxisRef, rc, stOutRegPDO4.usErrorID, stOutRegPDO4.usStatus);
        }
        _MMCPP_TRACE_INFO("CancelPDO OK")
    }
}

/** \fn int  ChangeDefaultPDOConfig(
 *  unsigned char ucPDONum, unsigned char ucPDODir, unsigned char ucPDOCommParam)
 *  \brief
 *  \param  ucPDONum PDO number(1-4)
 *  \param  ucPDODir RXPDO(0), TXPDO(1)
 *  \param  ucPDOCommParam
 *  \return none.
 */
void CMMCSingleAxis::ChangeDefaultPDOConfig(
        ELMO_UINT8 ucPDONum,         // 1 -4.
        ELMO_UINT8 ucPDODir,         // 0 RXPDO. 1 TXPDO
        ELMO_UINT8 ucPDOCommParam    // 1. OnSync. 255. On Event (ASYNCHRONOUS).
        ) throw (CMMCException)

{
    ELMO_INT32 rc;
    MMC_CONFIGPDOCOMMPARAM_IN stInParams;
    MMC_CONFIGPDOCOMMPARAM_OUT stOutParams;
    stInParams.ucPDONum = ucPDONum;
    stInParams.ucPDODir = ucPDODir;
    stInParams.ucPDOCommParam = ucPDOCommParam;

    if ((rc=MMC_ChangeDefaultPDOConfiguration(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ChangeDefaultPDOConfiguration:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("ChangeDefaultPDOConfig OK")
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoSetParameter(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN char cCmd[3],
///             IN unsigned char ucValType,
///             IN void* pVal)
/// \brief This function set parameters.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \param  ucValType - [IN]  Data value type (integer/float)
/// \param  pVal - [IN] Pointer to data value. (max 4 bytes)
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoSetAsyncParam(ELMO_INT8 cCmd[3], ELMO_INT32& iVal) throw (CMMCException)
{
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ((rc = MMC_ElmoSetParameter(m_uiConnHndl, m_usAxisRef, cCmd, 0, (ELMO_PVOID)&iVal)) == 0)
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoSetParameter:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoSetAsyncParam OK")
}
void CMMCSingleAxis::ElmoSetAsyncParam(ELMO_INT8 cCmd[3], ELMO_FLOAT& fVal) throw (CMMCException)
{
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ((rc = MMC_ElmoSetParameter(m_uiConnHndl, m_usAxisRef, cCmd, 1, (ELMO_PVOID)&fVal)) == 0)
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoSetParameter:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoSetAsyncParam OK")
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoGetParameter(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN char cCmd[3])
/// \brief This function send get parameters request.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoGetAsyncIntParam(ELMO_INT8 cCmd[3]) throw (CMMCException)
{
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoGetParameter(m_uiConnHndl, m_usAxisRef, cCmd, 0)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetParameter:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetAsyncParam OK")
}
void CMMCSingleAxis::ElmoGetAsyncFloatParam(ELMO_INT8 cCmd[3]) throw (CMMCException)
{
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoGetParameter(m_uiConnHndl, m_usAxisRef, cCmd, 1)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetParameter:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetAsyncParam OK")
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoSetArray(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN char cCmd[3],
///             IN short iArrayIdx
///             IN unsigned char ucValType,
///             IN unsigned char ucVal[8])
/// \brief This function set parameters.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \param  iArrayIdx - [IN] Index of array element
/// \param  ucValType - [IN]  Data value type (integer/float)
/// \param  pVal - [IN] Pointer to data value. (max 4 bytes)
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoSetAsyncArray(ELMO_INT8 cCmd[3], ELMO_INT16 iArrayIdx, ELMO_FLOAT& fVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    if (iArrayIdx < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncArray' (ELMO_FLOAT) Illegal iIndex ", 0, 0, -1, -1, 0);
    while(1)
    {
        if ( (rc = MMC_ElmoSetArray(m_uiConnHndl, m_usAxisRef, cCmd, iArrayIdx, 1,  (ELMO_PVOID)&fVal)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoSetArray:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoSetAsyncArray OK")
}
void CMMCSingleAxis::ElmoSetAsyncArray(ELMO_INT8 cCmd[3], ELMO_INT16 iArrayIdx, ELMO_INT32& iVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    if (iArrayIdx < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncArray' (ELMO_INT32&) Illegal iIndex ", 0, 0, -1, -1, 0);
    while(1)
    {
        if ( (rc = MMC_ElmoSetArray(m_uiConnHndl, m_usAxisRef, cCmd, iArrayIdx, 0,  (ELMO_PVOID)&iVal)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoSetArray:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoSetAsyncArray OK")
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoGetArray(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN char cCmd[3],
///             IN short iArrayIdx)
/// \brief This function send get array element parameters request.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \param  iArrayIdx - [IN] Index of array element
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoGetAsyncIntArray(ELMO_INT8 cCmd[3], ELMO_INT16 iArrayIdx) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    if (iArrayIdx < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetAsyncIntArray' Illegal iIndex ", 0, 0, -1, -1, 0);
    while(1)
    {
        if ( (rc = MMC_ElmoGetArray( m_uiConnHndl, m_usAxisRef, cCmd, iArrayIdx, 0)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetArray:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetAsyncArray OK")
}
void CMMCSingleAxis::ElmoGetAsyncFloatArray(ELMO_INT8 cCmd[3], ELMO_INT16 iArrayIdx) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    if (iArrayIdx < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetAsyncFloatArray' Illegal iIndex ", 0, 0, -1, -1, 0);
    while(1)
    {
        if ( (rc = MMC_ElmoGetArray( m_uiConnHndl, m_usAxisRef, cCmd, iArrayIdx, 1)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetArray:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetAsyncArray OK")
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoGetParameterAndRetrieveData(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef
///             IN char cCmd[3],
///             IN unsigned char ucValType,
///             OUT void *pVal,
///             OUT unsigned int* uiErrorID)
/// \brief This function do synchronous  get parameter
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \param  ucValType - [IN]  Data value type (integer/float)
/// \param  pVal - [OUT] Pointer to retrived data.
/// \param  uiErrorID - [OUT] returned Error Code
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoGetSyncParam(ELMO_INT8 cCmd[3], ELMO_FLOAT& fVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_UINT32 uiErrorID;

    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoGetParameterAndRetrieveData(m_uiConnHndl, m_usAxisRef, cCmd, 1, (ELMO_PVOID)&fVal, &uiErrorID)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetParameterAndRetrieveData:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)uiErrorID, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetSyncParam OK")
}
void CMMCSingleAxis::ElmoGetSyncParam(ELMO_INT8 cCmd[3], ELMO_INT32& iVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_UINT32 uiErrorID;

    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoGetParameterAndRetrieveData(m_uiConnHndl, m_usAxisRef, cCmd, 0, (ELMO_PVOID)&iVal, &uiErrorID)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetParameterAndRetrieveData:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)uiErrorID, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetSyncParam OK")
}


////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoGetArrayAndRetrieveData(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef
///             IN char cCmd[3],
///             IN short iArrayIdx,
///             IN unsigned char ucValType,
///             OUT void *pVal,
///             OUT unsigned int* uiErrorID)
/// \brief This function do synchronous  get array parameter
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \param  iArrayIdx - [IN] Index of array element
/// \param  ucValType - [IN]  Data value type (integer/float)
/// \param  pVal - [OUT] Pointer to retrived data.
/// \param  uiErrorID - [OUT] returned Error Code
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoGetSyncArray(ELMO_INT8 cCmd[3], ELMO_INT16 iArrayIdx, ELMO_FLOAT& fVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_UINT32 uiErrorID;


    if (iArrayIdx < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetSyncArray' Illegal iIndex ", 0, 0, -1, -1, 0);

    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoGetArrayAndRetrieveData(m_uiConnHndl, m_usAxisRef, cCmd, iArrayIdx, 1, (ELMO_PVOID)&fVal, &uiErrorID)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetArrayAndRetrieveData:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)uiErrorID, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetSyncArray OK")
}


void CMMCSingleAxis::ElmoGetSyncArray(ELMO_INT8 cCmd[3], ELMO_INT16 iArrayIdx, ELMO_INT32& iVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_UINT32 uiErrorID;

    ELMO_INT32 iFailCnt = 0;


    if (iArrayIdx < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetSyncArray' Illegal iIndex ", 0, 0, -1, -1, 0);

    while(1)
    {
        if ( (rc = MMC_ElmoGetArrayAndRetrieveData(m_uiConnHndl, m_usAxisRef, cCmd, iArrayIdx, 0, (ELMO_PVOID)&iVal, &uiErrorID)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoGetArrayAndRetrieveData:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)uiErrorID, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetSyncArray OK")
}

////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoExecuteLabel(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN MMC_INTERPEXECUTECMD_IN* pInParam,
///             OUT MMC_INTERPEXECUTECMD_OUT* pOutParam)
/// \brief This function send command to execute label
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  pInParam - [IN] Pointer to Interpreter Execute Command input parameters
/// \param  pOutParam - [OUT] Pointer to Interpreter Execute Command output parameters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoExecute(ELMO_PUINT8 pData, ELMO_UINT8 ucLength) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_INTERPEXECUTECMD_IN stInParams;
    MMC_INTERPEXECUTECMD_OUT stOutParam;

    memcpy((ELMO_PVOID)stInParams.pData, (const void*)pData, (size_t)80);
    stInParams.ucLength = ucLength;

    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoExecuteLabel(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoExecuteLabel:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("ElmoExecute OK")
}


////////////////////////////////////////////////////////////////////////////////
/// \fn int  MMC_ElmoCall(
///             N MMC_CONNECT_HNDL hConn,
///             IN MMC_AXIS_REF_HNDL hAxisRef,
///             IN char cCmd[3])
/// \brief This function call execute command.
/// \param  hConn - [IN] Connection handle
/// \param  hAxisRef - [IN] Axis/Group Reference handle.
/// \param  cCmd - [IN] Command characters
/// \return return - 0 if success
///                  error_id in case of error
////////////////////////////////////////////////////////////////////////////////
void CMMCSingleAxis::ElmoCallAsync(ELMO_INT8 cCmd[3]) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_INT32 iFailCnt = 0;
    while(1)
    {
        if ( (rc = MMC_ElmoCall(m_uiConnHndl, m_usAxisRef, cCmd)) ==  0 )
            break;
        this->USleep(100);
        iFailCnt++;
        if(iFailCnt > MAX_RETRIES || rc != NC_NON_NC_REACTOR_OVERFLOW_FAIL)
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoCall:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoCallAsync OK")
}

ELMO_INT32 CMMCSingleAxis::ElmoIsReplyAwaiting() throw (CMMCException) {
    ELMO_INT32 iReceivedMsgIdx = -1;
    ELMO_INT32 rc;

    if ( (rc = MMC_ElmoQueryOperationFIFOIndex(m_uiConnHndl, m_usAxisRef, &iReceivedMsgIdx)) !=  0 ) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoQueryOperationFIFOIndex:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)0, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoIsReplyAwaiting OK")
    return iReceivedMsgIdx;
}

void CMMCSingleAxis::ElmoGetReply(ELMO_INT32& iVal) throw (CMMCException) {
    ELMO_INT32 rc;
    ELMO_UINT32 uiErrorID;
    ELMO_INT32 iResult;

    if ( (rc = MMC_ElmoQueryOperationFIFORetrieveData(m_uiConnHndl, m_usAxisRef, (ELMO_PVOID)&iResult, &uiErrorID)) !=  0 ) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoQueryOperationFIFORetrieveData:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)uiErrorID, (ELMO_UINT16)-1);
    }

    _MMCPP_TRACE_INFO("ElmoGetReply OK")
    iVal = iResult;
}

void CMMCSingleAxis::ElmoGetReply(ELMO_FLOAT& fVal) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    ELMO_UINT32 uiErrorID;
    ELMO_FLOAT fResult;
    if ( (rc = MMC_ElmoQueryOperationFIFORetrieveData(m_uiConnHndl, m_usAxisRef, (ELMO_PVOID)&fResult, &uiErrorID)) !=  0 ) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ElmoQueryOperationFIFORetrieveData:", m_uiConnHndl, m_usAxisRef, rc, (ELMO_INT16)uiErrorID, (ELMO_UINT16)-1);
    }
    _MMCPP_TRACE_INFO("ElmoGetReply OK")
    fVal = fResult;
}


/*! \fn void ConfigVirtualEncoder(unsigned char ucPDOEventMode)
 * \brief this method configure virtual encoder for axis.
 * \param dbLowPos - low position
 * \param dbHighPos - high position
 * \param fFactor -
 * \param ucMode -
 * \param ucGroupID -
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::ConfigVirtualEncoder(
                ELMO_DOUBLE dbLowPos,
                ELMO_DOUBLE dbHighPos,
                ELMO_FLOAT       fFactor,
                ELMO_UINT8 ucMode,
                ELMO_UINT8 ucGroupID) throw (CMMCException) {

    ELMO_INT32 rc = 0;
    MMC_CONFIGVIRTUALENCODER_IN stInParams;
    MMC_CONFIGVIRTUALENCODER_OUT stOutParams;

    stInParams.dbHighPos = dbHighPos;
    stInParams.dbLowPos = dbLowPos;
    stInParams.fFactor = fFactor;
    stInParams.ucGroupID = ucGroupID;
    stInParams.ucMode = ucMode;

    if ((rc=MMC_ConfigVirtualEncoder(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigVirtualEncoder:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("ConfigVirtualEncoder OK")
}


/*! \fn void CancelVirtualEncoder(unsigned char ucPDOEventMode)
 * \brief this method cancel virtual encoder for axis.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::CancelVirtualEncoder() throw (CMMCException) {

    ELMO_INT32 rc = 0;
    MMC_CONFIGVIRTUALENCODER_OUT stOutParams;


    if ((rc=MMC_CancelVirtualEncoder(m_uiConnHndl, m_usAxisRef, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CancelVirtualEncoder:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("CancelVirtualEncoder OK")
}



/*! \fn void CancelVirtualEncoder(unsigned char ucPDOEventMode)
 * \brief 
 * \ param dbPosition
 * \ param dbModulus
 * \ param ucPosMode - abs mode = 0. relative mode = 1.
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCSingleAxis::SetPosition(ELMO_DOUBLE dbPosition, ELMO_UINT8 ucPosMode) throw (CMMCException) {

    ELMO_INT32 rc = 0;
    MMC_SETPOSITION_IN stInParams;
    MMC_SETPOSITION_OUT stOutParams;

    
    stInParams.dbPosition = dbPosition;
    stInParams.eExecutionMode = eMMC_EXECUTION_MODE_IMMEDIATE;
    stInParams.ucPosMode = ucPosMode;// abs mode = 0. relative mode = 1.
    stInParams.ucExecute = 1;

    if ((rc=MMC_SetPositionCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetPositionCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("SetPosition OK")
}

ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitive(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEABSOLUTEREPETITIVE_IN stInParams;
    MMC_MOVEABSOLUTEREPETITIVE_OUT stOutParams;
    stInParams.dbPosition       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.fVelocity        = m_fVelocity;
    stInParams.fAcceleration    = m_fAcceleration ;
    stInParams.fDeceleration    = m_fDeceleration ;
    stInParams.fJerk            = m_fJerk ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteRepetitiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveAbsoluteRepetitiveCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsoluteRepetitive OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    rc = MoveAbsoluteRepetitive(dPos,eBufferMode);
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    rc = MoveAbsoluteRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MoveAbsoluteRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_UINT32 uiExecDelayMs, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_uiExecDelayMs = uiExecDelayMs;
    rc = MoveAbsoluteRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitive(ELMO_DOUBLE dPos,  MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVERELATIVEREPETITIVE_IN stInParams;
    MMC_MOVERELATIVEREPETITIVE_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.fVelocity        = m_fVelocity;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    stInParams.fAcceleration    = m_fAcceleration ;
    stInParams.fDeceleration    = m_fDeceleration ;
    stInParams.fJerk            = m_fJerk ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveRelativeRepetitiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveRelativeRepetitiveCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelativeRepetitive OK")
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    rc = MoveRelativeRepetitive(dPos,eBufferMode);
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    rc = MoveRelativeRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MoveRelativeRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_UINT32 uiExecDelayMs, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_uiExecDelayMs = uiExecDelayMs;
    rc = MoveRelativeRepetitive(dPos,eBufferMode);
    return rc ;
}



ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitive(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEADDITIVEREPETITIVE_IN stInParams;
    MMC_MOVEADDITIVEREPETITIVE_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.fVelocity        = m_fVelocity;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    stInParams.fAcceleration    = m_fAcceleration ;
    stInParams.fDeceleration    = m_fDeceleration ;
    stInParams.fJerk            = m_fJerk ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveRepetitiveCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveAdditiveRepetitiveCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditiveRepetitive OK")
    return rc ;
}



ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    rc = MoveAdditiveRepetitive(dPos,eBufferMode);
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    rc = MoveAdditiveRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_FLOAT fAcceleration, ELMO_FLOAT fDeceleration, ELMO_FLOAT fJerk, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_fAcceleration = fAcceleration;
    m_fDeceleration = fDeceleration;
    m_fJerk = fJerk;
    rc = MoveAdditiveRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitive(ELMO_DOUBLE dPos, ELMO_FLOAT fVel, ELMO_UINT32 uiExecDelayMs, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    ELMO_INT32 rc = 0;
    m_fVelocity = fVel;
    m_uiExecDelayMs = uiExecDelayMs;
    rc = MoveAdditiveRepetitive(dPos,eBufferMode);
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::PositionProfile(MC_PATH_REF hMemHandle, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_POSITIONPROFILE_IN stInParams;
    MMC_POSITIONPROFILE_OUT stOutParams;
    ELMO_INT32 rc = 0;
    stInParams.hMemHandle = hMemHandle;
    stInParams.eBufferMode = eBufferMode;
    stInParams.ucExecute = m_ucExecute;

    rc = MMC_PositionProfileCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PositionProfileCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("PositionProfile OK")
    return rc ;
}

/*
ELMO_INT32 CMMCSingleAxis::MoveAbsoluteEx(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEABSOLUTEEX_IN   stInParams;
    MMC_MOVEABSOLUTEEX_OUT  stOutParams;

    stInParams.dbPosition       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = m_fVelocity;
    stInParams.dAcceleration    = m_fAcceleration ;
    stInParams.dDeceleration    = m_fDeceleration ;
    stInParams.dJerk            = m_fJerk ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAbsoluteExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsoluteEx OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveAbsoluteEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEABSOLUTEEX_IN stInParams;
    MMC_MOVEABSOLUTEEX_OUT  stOutParams;

    stInParams.dbPosition = dPos ;
    stInParams.dVelocity    = dVel ;
    m_fVelocity = dVel;
    stInParams.eBufferMode = eBufferMode;
    stInParams.dAcceleration    = m_fAcceleration ;
    stInParams.dDeceleration    = m_fDeceleration ;
    stInParams.dJerk            = m_fJerk ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAbsoluteExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsoluteEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveRelativeEx(ELMO_DOUBLE dDistance, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVERELATIVEEX_IN stInParams;
    MMC_MOVERELATIVEEX_OUT  stOutParams;
    ELMO_INT32 rc = 0;
    stInParams.dbDistance = dDistance ;
    stInParams.dVelocity    = m_fVelocity ;
    stInParams.eBufferMode = eBufferMode;
    stInParams.eDirection = m_eDirection;
    stInParams.dAcceleration = m_fAcceleration;
    stInParams.dDeceleration = m_fDeceleration;
    stInParams.dJerk = m_fJerk;
    stInParams.ucExecute = m_ucExecute;
    rc = MMC_MoveRelativeExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveRelativeExCmd:", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelativeEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveRelativeEx(ELMO_DOUBLE dDistance, ELMO_DOUBLE dVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVERELATIVEEX_IN stInParams;
    MMC_MOVERELATIVEEX_OUT  stOutParams;
    ELMO_INT32 rc = 0;
    stInParams.dbDistance = dDistance ;
    stInParams.dVelocity    = dVel ;
    m_fVelocity = static_cast<float>(dVel) ;
    stInParams.eBufferMode = eBufferMode;
    stInParams.eDirection = m_eDirection;
    stInParams.dAcceleration = m_fAcceleration;
    stInParams.dDeceleration = m_fDeceleration;
    stInParams.dJerk = static_cast<double>(m_fJerk);
    stInParams.ucExecute = m_ucExecute;
    rc = MMC_MoveRelativeExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveRelativeExCmd:", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelativeEx OK")
    return rc ;
}



ELMO_INT32 CMMCSingleAxis::MoveAdditiveEx(ELMO_DOUBLE dDistance, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEADDITIVEEX_IN stInParams;
    MMC_MOVEADDITIVEEX_OUT  stOutParams;

    stInParams.dbDistance = dDistance;
    stInParams.eBufferMode = eBufferMode;
    stInParams.dVelocity    = static_cast<double>(m_fVelocity) ;
    stInParams.eDirection = m_eDirection;
    stInParams.dAcceleration = static_cast<ELMO_DOUBLE>(m_fAcceleration);
    stInParams.dDeceleration = static_cast<ELMO_DOUBLE>(m_fDeceleration);
    stInParams.dJerk = static_cast<ELMO_DOUBLE>(m_fJerk);
    stInParams.ucExecute = m_ucExecute;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAdditiveExCmd:", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditiveEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAdditiveEx(ELMO_DOUBLE dDistance, ELMO_DOUBLE dVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEADDITIVEEX_IN stInParams;
    MMC_MOVEADDITIVEEX_OUT  stOutParams;
    stInParams.dbDistance = dDistance ;
    stInParams.dVelocity    = dVel ;
    m_fVelocity = static_cast<float>(dVel);
    stInParams.eBufferMode = eBufferMode;
    stInParams.eDirection = m_eDirection;
    stInParams.dAcceleration = static_cast<ELMO_DOUBLE>(m_fAcceleration);
    stInParams.dDeceleration = static_cast<ELMO_DOUBLE>(m_fDeceleration);
    stInParams.dJerk = static_cast<ELMO_DOUBLE>(m_fJerk);
    stInParams.ucExecute = m_ucExecute;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAdditiveExCmd:", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditiveEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveVelocityEx(ELMO_DOUBLE dVelocity, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVEVELOCITYEX_IN       stInParams;
    MMC_MOVEVELOCITYEX_OUT    stOutParams;

    stInParams.dVelocity = dVelocity ;
    m_fVelocity = static_cast<float>(dVelocity);
    stInParams.eBufferMode = eBufferMode;
    stInParams.eDirection = m_eDirection;
    stInParams.dAcceleration = static_cast<ELMO_DOUBLE>(m_fAcceleration);
    stInParams.dDeceleration = static_cast<ELMO_DOUBLE>(m_fDeceleration);
    stInParams.dJerk = static_cast<ELMO_DOUBLE>(m_fJerk);
    stInParams.ucExecute = m_ucExecute;

    ELMO_INT32 rc = 0;
    rc = MMC_MoveVelocityExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveVelocityExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveVelocityEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitiveEx(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEABSOLUTEREPETITIVEEX_IN stInParams;
    MMC_MOVEABSOLUTEREPETITIVEEX_OUT stOutParams;
    stInParams.dbPosition       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = static_cast<ELMO_DOUBLE>(m_fVelocity);
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration) ;
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAbsoluteRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsoluteRepetitiveEx OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitiveEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEABSOLUTEREPETITIVEEX_IN stInParams;
    MMC_MOVEABSOLUTEREPETITIVEEX_OUT stOutParams;
    stInParams.dbPosition       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = static_cast<ELMO_DOUBLE>(dVel);
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration) ;
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAbsoluteRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsoluteRepetitiveEx OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveAbsoluteRepetitiveEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel, ELMO_UINT32 uiExecDelayMs, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEABSOLUTEREPETITIVEEX_IN stInParams;
    MMC_MOVEABSOLUTEREPETITIVEEX_OUT stOutParams;
    stInParams.dbPosition       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = static_cast<ELMO_DOUBLE>(dVel);
    stInParams.uiExecDelayMs    = uiExecDelayMs;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration) ;
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_MoveAbsoluteRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAbsoluteRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAbsoluteRepetitiveEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitiveEx(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVERELATIVEREPETITIVEEX_IN stInParams;
    MMC_MOVERELATIVEREPETITIVEEX_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    stInParams.dVelocity        = m_fVelocity;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration) ;
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;

    ELMO_INT32 rc = 0;
    rc = MMC_MoveRelativeRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveRelativeRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelativeRepetitiveEx OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitiveEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel,  MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVERELATIVEREPETITIVEEX_IN stInParams;
    MMC_MOVERELATIVEREPETITIVEEX_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    stInParams.dVelocity        = dVel;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration) ;
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;

    m_fVelocity = static_cast<float>(dVel);

    ELMO_INT32 rc = 0;
    rc = MMC_MoveRelativeRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveRelativeRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelativeRepetitiveEx OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveRelativeRepetitiveEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel, ELMO_UINT32 uiExecDelayMs, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVERELATIVEREPETITIVEEX_IN stInParams;
    MMC_MOVERELATIVEREPETITIVEEX_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.uiExecDelayMs    = uiExecDelayMs;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;
    stInParams.dVelocity        = dVel;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration) ;
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;

    m_fVelocity = static_cast<float>(dVel);
    m_uiExecDelayMs = uiExecDelayMs;

    ELMO_INT32 rc = 0;
    rc = MMC_MoveRelativeRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveRelativeRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveRelativeRepetitiveEx OK")
    return rc ;
}



ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitiveEx(ELMO_DOUBLE dPos, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEADDITIVEREPETITIVEEX_IN stInParams;
    MMC_MOVEADDITIVEREPETITIVEEX_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = static_cast<ELMO_DOUBLE>(m_fVelocity);
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration );
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;

    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAdditiveRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditiveRepetitiveEx OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitiveEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEADDITIVEREPETITIVEEX_IN stInParams;
    MMC_MOVEADDITIVEREPETITIVEEX_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = dVel;
    stInParams.uiExecDelayMs    = m_uiExecDelayMs;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration );
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;

    m_fVelocity = static_cast<float>(dVel);

    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAdditiveRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditiveRepetitiveEx OK")
    return rc ;
}
ELMO_INT32 CMMCSingleAxis::MoveAdditiveRepetitiveEx(ELMO_DOUBLE dPos, ELMO_DOUBLE dVel, ELMO_UINT32 uiExecDelayMs, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException) {
    MMC_MOVEADDITIVEREPETITIVEEX_IN stInParams;
    MMC_MOVEADDITIVEREPETITIVEEX_OUT stOutParams;
    stInParams.dbDistance       = dPos ;
    stInParams.eBufferMode      = eBufferMode;
    stInParams.dVelocity        = dVel;
    stInParams.uiExecDelayMs    = uiExecDelayMs;
    stInParams.dAcceleration    = static_cast<ELMO_DOUBLE>(m_fAcceleration) ;
    stInParams.dDeceleration    = static_cast<ELMO_DOUBLE>(m_fDeceleration );
    stInParams.dJerk            = static_cast<ELMO_DOUBLE>(m_fJerk) ;
    stInParams.eDirection       = m_eDirection ;
    stInParams.ucExecute        = m_ucExecute ;

    m_fVelocity = static_cast<float>(dVel);
    m_uiExecDelayMs = uiExecDelayMs;

    ELMO_INT32 rc = 0;
    rc = MMC_MoveAdditiveRepetitiveExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        MMCPPThrow("MMC_MoveAdditiveRepetitiveExCmd: ", rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveAdditiveRepetitiveEx OK")
    return rc ;
}
*/

ELMO_INT32 CMMCSingleAxis::TouchProbeEnable(ELMO_UINT8 ucTriggerType) throw (CMMCException)
{
    MMC_TOUCHPROBEENABLE_IN stInParams;
    MMC_TOUCHPROBEENABLE_OUT stOutParams;
    stInParams.ucTriggerType = ucTriggerType ;
    stInParams.ucExecute    = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_TouchProbeEnable (m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TouchProbeEnable: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("TouchProbeEnable OK")
    return rc ;
}

ELMO_INT32 CMMCSingleAxis::TouchProbeDisable() throw (CMMCException)
{
    MMC_TOUCHPROBEDISABLE_IN stInParams;
    MMC_TOUCHPROBEDISABLE_OUT stOutParams;
    stInParams.ucExecute    = m_ucExecute ;
    ELMO_INT32 rc = 0;
    rc = MMC_TouchProbeDisable(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TouchProbeDisable: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("TouchProbeDisable OK")
    return rc ;
}


ELMO_INT32 CMMCSingleAxis::TouchProbeEnable_Ex(ELMO_ULINT32 ulShmArraySizeAllocIN, ELMO_UINT16 usConfigIN, ELMO_UINT16 &usTPIndexOUT) throw (CMMCException)
{
	MMC_TOUCHPROBEENABLE_EX_IN stInParams;
	MMC_TOUCHPROBEENABLE_EX_OUT stOutParams;
	stInParams.ulShmArraySizeAlloc = ulShmArraySizeAllocIN;
	stInParams.usConfig = usConfigIN;
	ELMO_INT32 rc = 0;
	rc = MMC_TouchProbeEnable_Ex(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
	if (rc != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TouchProbeEnable_Ex: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
	}
	usTPIndexOUT = stOutParams.usTPIndex;
//	printf("\33[1;32m  a1.TouchProbeEnable_Ex(): ErrorID = %d, usTPIndexOUT = %d\n\33[0m", stOutParams.sErrorID, usTPIndexOUT);
	_MMCPP_TRACE_INFO("TouchProbeEnable_Ex OK")
	return rc ;
}

ELMO_INT32 CMMCSingleAxis::TouchProbeDisable_Ex(ELMO_INT16 sIndexIN) throw (CMMCException)
{
	MMC_TOUCHPROBEDISABLE_EX_IN stInParams;
	MMC_TOUCHPROBEDISABLE_EX_OUT stOutParams;
	stInParams.sTPIndex = sIndexIN;
	ELMO_INT32 rc = 0;
	rc = MMC_TouchProbeDisable_Ex(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
	if (rc != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_TouchProbeDisable_Ex: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
	}
//	printf("\33[1;32m  a2.TouchProbeDisable_Ex(): ErrorID = %d, sTPIndexIN = %d\n\33[0m", stOutParams.sErrorID, sIndexIN);
	_MMCPP_TRACE_INFO("TouchProbeDisable_Ex OK")
	return rc ;
}

ELMO_INT32 CMMCSingleAxis::GetTouchProbeData(ELMO_PLINT32 plBuffer, ELMO_UINT16 usIndexIN, ELMO_UINT16 usNumOfPointsIN) throw (CMMCException)
{
	MMC_GETTOUCHPROBEDATA_IN stInParams;
	MMC_GETTOUCHPROBEDATA_OUT stOutParams;
	stInParams.usTPIndex = usIndexIN;
	stInParams.usNumOfPoints = usNumOfPointsIN;

	plBuffer = stOutParams.lData;

	ELMO_INT32 rc = 0;
	rc = MMC_GetTouchProbeData(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
	if (rc != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetTouchProbeData: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
	}
	_MMCPP_TRACE_INFO("GetTouchProbeData OK")
//	printf("\33[1;32m  GetTouchProbeData(): ErrorID = %d\n\n\33[0m", stOutParams.sErrorID);

	if((stOutParams.sErrorID < 0))
	{
		return stOutParams.sErrorID;
	}

//	printf("\33[0;34m TP usCopiedPoints = %hu, ulExistingPoints = %lu, ulTPEvtCnt = %lu\n\33[0m", stOutParams.usCopiedPoints, stOutParams.ulExistingPoints, stOutParams.ulTPEvtCnt);
//	printf("\33[0;34m TP dUURatio = %.2lf, usMissedTPFlag = %hu, usOverflowTPFlag = %hu\n\n\33[0m", stOutParams.dUURatio, stOutParams.usMissedTPFlag, stOutParams.usOverflowTPFlag);
//
//	sleep(1);
//
//	unsigned int i;
//
//	for(i=0; i < 41; i++)
//	{
////	printf("\33[0;34m lData[%d] = %ld\n\33[0m", i, stOutParams.lData[i]);
//		printf("\33[0;34m lBuffer[%d] = %ld\n\33[0m", i, *(plBuffer+i));
//	}
//	printf("\n");

	return stOutParams.sErrorID;
}

ELMO_INT32 CMMCSingleAxis::GetTouchProbeData(ELMO_PDOUBLE pdBuffer, ELMO_UINT16 usIndexIN, ELMO_UINT16 usNumOfPointsIN) throw (CMMCException)
{
	MMC_GETTOUCHPROBEDATA_IN stInParams;
	MMC_GETTOUCHPROBEDATA_OUT stOutParams;
	stInParams.usTPIndex = usIndexIN;
	stInParams.usNumOfPoints = usNumOfPointsIN;

	ELMO_INT32 rc = 0;
	rc = MMC_GetTouchProbeData(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
	if (rc != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetTouchProbeData: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
	}
	_MMCPP_TRACE_INFO("GetTouchProbeData OK")
//	printf("\33[1;32m  GetTouchProbeData(): ErrorID = %d\n\n\33[0m", stOutParams.sErrorID);

	if((stOutParams.sErrorID < 0))
	{
		return stOutParams.sErrorID;
	}

//	printf("\33[0;34m TP usCopiedPoints = %hu, ulExistingPoints = %lu, ulTPEvtCnt = %lu\n\33[0m", stOutParams.usCopiedPoints, stOutParams.ulExistingPoints, stOutParams.ulTPEvtCnt);
//	printf("\33[0;34m TP dUURatio = %.2lf, usMissedTPFlag = %hu, usOverflowTPFlag = %hu\n\n\33[0m", stOutParams.dUURatio, stOutParams.usMissedTPFlag, stOutParams.usOverflowTPFlag);
//
//	sleep(1);

	ELMO_UINT32 i;

	for(i=0; i < stOutParams.usCopiedPoints; i++)
	{
		*(pdBuffer+i) = stOutParams.lData[i] * stOutParams.dUURatio;
////	printf("\33[0;34m lData[%d] = %ld\n\33[0m", i, stOutParams.lData[i]);
//		printf("\33[0;34m lBuffer[%d] = %lf\n\33[0m", i, *(pdBuffer+i));
	}
//	printf("\n");

	return stOutParams.sErrorID;
}


/*! \fn void AxisLink(unsigned short usAxisRef, unsigned char ucMode = 0)
* \brief This function link between two axes
* \return   return - when erro throws CMMCException.
*/
void CMMCSingleAxis::AxisLink(ELMO_UINT16 usAxisRef, ELMO_UINT8 ucMode) throw (CMMCException)
{
    MMC_AXISLINK_IN stInParams;
    MMC_AXISLINK_OUT stOutParams;
    //
    stInParams.ucMode = ucMode;
    stInParams.usSlaveAxisReference = usAxisRef;
    //
    ELMO_INT32 rc = 0;
    rc = MMC_AxisLink(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_AxisLink: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("AxisLink OK")
    return;
}

/*! \fn void AxisUnLink()
* \brief This function unlink the axis
* \return   return - when erro throws CMMCException.
*/
void CMMCSingleAxis::AxisUnLink() throw (CMMCException)
{
    MMC_AXISUNLINK_IN stInParams;
    MMC_AXISUNLINK_OUT stOutParams;

    ELMO_INT32 rc = 0;
    rc = MMC_AxisUnLink(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_AxisUnLink: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("AxisUnLink OK")
    return;
}


void CMMCSingleAxis::MoveTorque(ELMO_DOUBLE dbTargetTorque, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    MMC_MOVETORQUE_IN   stInParams;
    MMC_MOVETORQUE_OUT  stOutParams;
    ELMO_INT32 rc = 0;
    //
    stInParams.dbTargetTorque = dbTargetTorque;
    stInParams.dbTorquetVelocity    = m_dbTorqueVelocity ;
    stInParams.dbTorqueAcceleration = m_dbTorqueAcceleration;
    stInParams.eBufferMode = eBufferMode;
    stInParams.ucExecute = m_ucExecute;
    //
    rc = MMC_MoveTorqueCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_MoveTorqueCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MoveTorque OK")
    return;
}


void CMMCSingleAxis::MoveTorque(ELMO_DOUBLE dbTargetTorque, ELMO_DOUBLE dbTorqeVelocity, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    m_dbTorqueVelocity  = dbTorqeVelocity;
    MoveTorque(dbTargetTorque,eBufferMode);
    return;
}


void CMMCSingleAxis::MoveTorque(ELMO_DOUBLE dbTargetTorque, ELMO_DOUBLE dbTorqeVelocity, ELMO_DOUBLE dbTorqueAcceleration, MC_BUFFERED_MODE_ENUM eBufferMode) throw (CMMCException)
{
    m_dbTorqueVelocity  = dbTorqeVelocity;
    m_dbTorqueAcceleration = dbTorqueAcceleration;
    MoveTorque(dbTargetTorque,eBufferMode);
    return;
}
void CMMCSingleAxis::SendCmdViaSdoDownload(ELMO_LINT32   lData, const ELMO_INT8*  pcCmdIdx, ELMO_UINT8 ucSubIndex) throw (CMMCException)
{
    ELMO_UINT16  usIndex;

    if (pcCmdIdx == NULL) { return; }

    usIndex = CalcSdoIdx(pcCmdIdx);
    if (MMCSingledAxisDbgFlag) { printf("\n %s dbg: %2s (usIndex=0x%x=%d; SubIdx=%d)=%d ", __func__, pcCmdIdx, (ELMO_INT32)usIndex, (ELMO_INT32)usIndex, (ELMO_INT32)ucSubIndex, (ELMO_INT32)lData); }
    CMMCNode::SendSdoDownload(lData, ucSubIndex, DATALENGTHFIXVAL, usIndex, SLAVEIDFIXVAL);
}
//
void CMMCSingleAxis::SendCmdViaSdoDownload(ELMO_FLOAT  fData, const ELMO_INT8*  pcCmdIdx, ELMO_UINT8 ucSubIndex) throw (CMMCException)
{
    ELMO_UINT16      usIndex;
    ELMO_LINT32      lData;
    FLOAT_OR_LONG   f_or_l;

    if (pcCmdIdx == NULL) { return; }

    f_or_l.fData = fData;
    lData = f_or_l.lData;

    usIndex = CalcSdoIdx(pcCmdIdx);
    if (MMCSingledAxisDbgFlag) { printf("\n %s dbg: %2s (usIndex=0x%x=%d; SubIdx=%d)=%f (= int: %d)", __func__, pcCmdIdx, (ELMO_INT32)usIndex, (ELMO_INT32)usIndex, (ELMO_INT32)ucSubIndex, fData, (int)f_or_l.lData); }
    CMMCNode::SendSdoDownload(lData, ucSubIndex, DATALENGTHFIXVAL, usIndex, SLAVEIDFIXVAL);
}
//
void CMMCSingleAxis::SendCmdViaSdoUpload  (ELMO_LINT32&  rlData, const ELMO_INT8*  pcCmdIdx, ELMO_UINT8 ucSubIndex) throw (CMMCException)
{
    ELMO_UINT16  usIndex;
    ELMO_LINT32  lData;

    if (pcCmdIdx == NULL) { return; }

    usIndex = CalcSdoIdx(pcCmdIdx);
    lData = CMMCNode::SendSdoUpload(ucSubIndex, DATALENGTHFIXVAL, usIndex, SLAVEIDFIXVAL);
    rlData = lData;
    if (MMCSingledAxisDbgFlag) { printf("\n %s dbg: %2s (usIndex=0x%x=%d; SubIdx=%d)=%d ", __func__, pcCmdIdx, (ELMO_INT32)usIndex, (ELMO_INT32)usIndex, (ELMO_INT32)ucSubIndex, (ELMO_INT32)rlData); }
}
//
void CMMCSingleAxis::SendCmdViaSdoUpload  (ELMO_FLOAT& rfData, const ELMO_INT8*  pcCmdIdx, ELMO_UINT8 ucSubIndex) throw (CMMCException)
{
    ELMO_UINT16  usIndex;
    FLOAT_OR_LONG   f_or_l;

    if (pcCmdIdx == NULL) { return; }

    usIndex = CalcSdoIdx(pcCmdIdx);
    f_or_l.lData = CMMCNode::SendSdoUpload(ucSubIndex, DATALENGTHFIXVAL, usIndex, SLAVEIDFIXVAL);
    rfData = f_or_l.fData;
    if (MMCSingledAxisDbgFlag) { printf("\n %s dbg: %2s (usIndex=0x%x=%d; SubIdx=%d)=%f (= int: %d)", __func__, pcCmdIdx, (int)usIndex, (int)usIndex, (int)ucSubIndex, rfData, (int)f_or_l.lData); }
}
//
//
ELMO_UINT16  CMMCSingleAxis::CalcSdoIdx(const ELMO_INT8*  pcCmdIdx)
{
    ELMO_UINT16 SdoIdx = 0;
    ELMO_UINT8  CmdCh0;
    ELMO_UINT8  CmdCh1;

    CmdCh0 = toupper(pcCmdIdx[0]);
    CmdCh1 = toupper(pcCmdIdx[1]);
    
    SdoIdx = SdoBaseOfst+26*(CmdCh0-'A')+(CmdCh1-'A');

    return (SdoIdx);
}


    inline void CMMCSingleAxis::USleep(ELMO_ULINT32 usecs)
	{
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))

          LARGE_INTEGER 	pcfreq;
          LARGE_INTEGER 	before, after;
          struct timespec	tspec;

          tspec.tv_sec = usecs / 1000000;
          tspec.tv_nsec = (usecs % 1000000) * 1000;

          if (!QueryPerformanceFrequency (&pcfreq))
            printf ("QueryPerformanceFrequency failed\n");

          if (!QueryPerformanceCounter (&before))
            printf ("QueryPerformanceCounter failed\n");

                // ??????????????????? HH (Needs change...) ????????????
          nanosleep (&tspec);

          if (!QueryPerformanceCounter (&after))
            printf ("QueryPerformanceCounter failed\n");

          printf ("time slept: %g s\n", (ELMO_DOUBLE) (after.QuadPart - before.QuadPart) / (ELMO_DOUBLE) pcfreq.QuadPart);
#elif ((OS_PLATFORM == LINUXIPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC64_PLATFORM))
          usleep(usecs);
// #elif    (OS_PLATFORM == VXWORKS32_PLATFORM)
//  // Not implemented yet...
#else
    #error "***MMCSingleAxis.cpp Symbol 'OS_PLATFORM' (1) is out of range... (See OS_PlatformSelect.h) "
#endif
    }


/**
 * \fn	GearInPos(unsigned short usMaster,...)
 * \brief	perform gear-in on position of a single axis
 * \param usMaster	axis reference of master
 * \param dbMasterStartDistance	master distance for gear-in procedure (backward distance from dbMasterSyncPosition for ramp-in)
 * \param dbMasterSyncPosition	position of the master in which the slave is in-sync with the master.
 * \param dbSlaveSyncPosition	slave position at which the slave is in-sync with the master.
 * \param eBufferMode			buffered mode of this function block
 * \param iRatioNumerator		gear ratio numerator
 * \param iRatioDenominator		gear ratio denominator
 * \param eMasterValueSource	defines the source for synchronization (mcSetValue, mcActualValue, auxiliary)
 * 								mcSetValue - Synchronization on master set value
 * 								mcActualValue - Synchronization on master actual value
 * 								mcAux	-	master value retrieved from an auxiliary device
 * \param eSyncMode				eCATCH_UP(0), eSLOW_DOWN(1, performed only when conditions allow it)
 * \param dbVelocity			best effort at rump-in time
 * \param dbAcceleration		best effort at rump-in time
 * \param dbDeceleration		best effort at rump-in time
 * \param dbJerk				best effort at rump-in time
 * \return 0 if completed successfully, otherwise error or throws exception .
 */
ELMO_INT32 CMMCSingleAxis::GearInPos(
	ELMO_UINT16 usMaster,
	ELMO_DOUBLE dbMasterStartDistance,
	ELMO_DOUBLE dbMasterSyncPosition,
	ELMO_DOUBLE dbSlaveSyncPosition,
	MC_BUFFERED_MODE_ENUM eBufferMode,
	ELMO_INT32 	iRatioNumerator,
	ELMO_INT32 	iRatioDenominator,
	ECAM_VALUE_SRC_ENUM eMasterValueSource,
	GEAR_IN_SYNC_MODE_ENUM eSyncMode,
	ELMO_DOUBLE dbVelocity,
	ELMO_DOUBLE dbAcceleration,
	ELMO_DOUBLE dbDeceleration,
	ELMO_DOUBLE dbJerk) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_GEARINPOS_IN i_params;
	MMC_GEARINPOS_OUT o_params;
	i_params.usMaster = usMaster;
	i_params.dbAcceleration = dbAcceleration;
	i_params.dbDeceleration = dbDeceleration;
	i_params.dbJerk = dbJerk;
	i_params.dbMasterStartDistance = dbMasterStartDistance;
	i_params.dbMasterSyncPosition = dbMasterSyncPosition;
	i_params.dbSlaveSyncPosition = dbSlaveSyncPosition;
	i_params.dbVelocity = dbVelocity;
	i_params.eBufferMode = eBufferMode;
	i_params.eMasterValueSource = eMasterValueSource;
	i_params.eSyncMode = eSyncMode;
	i_params.iRatioNumerator = iRatioNumerator;
	i_params.iRatioDenominator = iRatioDenominator;
	i_params.ucExecute = 1;
	if ( (rc=MMC_GearInPosCmd(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) < 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GearInPosCmd: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
	}
	_MMCPP_TRACE_INFO("MMC_GearInPosCmd OK")
	return rc;
}

/**
* \fn	GearInPos(MMC_GEARINPOS_IN& i_params)
* \brief	perform gear-in on position of a single axis
* \param i_params	data structure with all input parameters for gear-in position.
* \return 0 if completed successfully, otherwise error or throws exception .
*/
ELMO_INT32 CMMCSingleAxis::GearInPos(MMC_GEARINPOS_IN& i_params) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_GEARINPOS_OUT o_params;
	if ( (rc=MMC_GearInPosCmd(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) < 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GearInPosCmd: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
	}
	_MMCPP_TRACE_INFO("MMC_GearInPosCmd OK")
	return rc;
}

/*!
 * \fn	int SetProfileConditioning(MMC_PROFILECOND_IN& i_params)
 * \brief	this method sets 'profile conditioning' mode of operation.
 *
 * it turns on(1)/off(0) 'profile conditioning' and sets other input parameters.
 *
 * \param i_params	reference of input parameters.
 * \return 0 if completed successfully, otherwise error or throws exception.
 */
ELMO_INT32 CMMCSingleAxis::SetProfileConditioning(MMC_PROFILECOND_IN& i_params) throw (CMMCException)
{
	MMC_PROFILECOND_OUT o_params;
	ELMO_INT32 rc = 0;
	/*
	 * please note:
	 * in order to set profile conditioning parameters, single axis must be disabled  or stand still.
	 */
	if ( (rc = MMC_SetProfileConditioning(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) != 0 )
	{
		rc = o_params.usErrorID;
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SetProfileConditioning: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	}
	_MMCPP_TRACE_INFO("SetProfileConditioning OK")

	return rc;
}

/*!
 * \fn	int EnableProfileConditioning(bool enable)
 * \brief	this interface enables "profile conditioning" mode on a single axis.
 *
 * it uses parameters, which were stored in one of three buffers on previous settings.
 * that is why other input parameters are redundant here.
 *
 * \param enable	its a flag which enable(1), disable(0) or clear(0xFF) the profile conditioning mode with the existing parameters.
 * \return 0 if completed successfully, otherwise error or throws exception.
 */
ELMO_INT32 CMMCSingleAxis::EnableProfileConditioning(ELMO_UINT8 enable) throw (CMMCException) {

	MMC_PRFCND_ENABLE_IN i_params;
	MMC_PRFCND_ENABLE_OUT o_params;
	ELMO_INT32 rc = 0;
	i_params.enable = enable;
	/*
	 * please note:
	 * in order to enable profile conditioning parameters, single axis must be disabled or stand still.
	 */
	if ( (rc = MMC_EnableProfileConditioning(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) != 0 )
	{
		rc = o_params.usErrorID;
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_EnableProfileConditioning: ", m_uiConnHndl, m_usAxisRef, rc, o_params.usErrorID, o_params.usStatus);
	}
	_MMCPP_TRACE_INFO("MMC_EnableProfileConditioning OK")

	return rc;
}




