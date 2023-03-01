/*
 * MMCRTSingleAxis.cpp
 *
 *  Created on: Mar 31, 2016
 *      Author: ZivB
 */
 
#include "OS_PlatformDependSetting.hpp"
#include "MMCRTSingleAxis.hpp"

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    extern "C"
    {
        MMC_LIB_API ELMO_PINT8 MMC_MapShmPtr(MMC_CONNECT_HNDL hConn);
        MMC_LIB_API ELMO_INT32 MMC_UnmapShmPtr(ELMO_PINT8 pShmPtr);
    }
#endif

#define NC_NODE_IDX(hNode) (hNode & 0xFF)
#define NC_NODE_TYPE(hNode) (NC_NODE_TYPE_ENUM)((hNode >> 8) & 0xFF)



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMMCRTSingleAxis::CMMCRTSingleAxis() : CMMCSingleAxis()
{
    m_uldPosOffset          = 0;
    m_ulUser607AOffset      = 0;
    m_ulUser60FFOffset      = 0;
    m_ulUser60B1Offset      = 0;
    m_ulUser6071Offset      = 0;
    m_ulUser60B2Offset      = 0;
    m_ulMB1Offset           = 0;
    m_ulMB2Offset           = 0;
    m_ulTargetVelOffset     = 0;
    m_ulActualPosOffset     = 0;
    m_ulActualVelUUOffset   = 0;
    m_ulAnsalogInputOffset  = 0;
    m_uiDigitalInputsOffset = 0;
    m_ulActualCurrentOffset = 0;
    m_ulStatusOffset        = 0;
    m_CycleCounterOffset    = 0;
    m_dbTorquemAToRCRatio   = 0;
    m_fMotorRatedCurrent    = 0;
    m_ulControlWordOffset   = 0;
    m_ulStatusWordOffset    = 0;
    m_ulUserCWOffset        = 0;
    m_ulActualPosINTOffset  = 0;
    m_ulActualVelINTOffset  = 0;
    m_ulStatusRegisterOffset= 0;

    m_stAPICom.uiInternalState= 0;
    m_stAPICom.uiIsInSequence= 0;
    m_stAPICom.uiApiReq     = 0;
}
CMMCRTSingleAxis::~CMMCRTSingleAxis()
{

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_pShmPtr)
    {
        MMC_UnmapShmPtr((ELMO_PINT8)m_pShmPtr);
    }
#endif

}
CMMCRTSingleAxis::CMMCRTSingleAxis(CMMCSingleAxis& axis):CMMCSingleAxis(axis)
{
    m_uldPosOffset          = 0;
    m_ulUser607AOffset      = 0;
    m_ulUser60FFOffset      = 0;
    m_ulUser60B1Offset      = 0;
    m_ulUser6071Offset      = 0;
    m_ulUser60B2Offset      = 0;
    m_ulMB1Offset           = 0;
    m_ulMB2Offset           = 0;
    m_ulTargetVelOffset     = 0;
    m_ulActualPosOffset     = 0;
    m_ulActualVelUUOffset   = 0;
    m_ulAnsalogInputOffset  = 0;
    m_uiDigitalInputsOffset = 0;
    m_ulActualCurrentOffset = 0;
    m_ulStatusOffset        = 0;
    m_CycleCounterOffset    = 0;
    m_dbTorquemAToRCRatio   = 0;
    m_fMotorRatedCurrent    = 0;
    m_ulControlWordOffset   = 0;
    m_ulStatusWordOffset    = 0;
    m_ulUserCWOffset        = 0;
    m_ulActualPosINTOffset  = 0;
    m_ulActualVelINTOffset  = 0;
    m_ulStatusRegisterOffset= 0;

    m_stAPICom.uiInternalState= 0;
    m_stAPICom.uiIsInSequence= 0;
    m_stAPICom.uiApiReq     = 0;
}

void CMMCRTSingleAxis::InitAxisData(const ELMO_INT8* cName, MMC_CONNECT_HNDL uHandle) throw (CMMCException)
{
    CMMCAxis::InitAxisData(cName,uHandle);

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    m_pShmPtr =  (ELMO_PUINT8)MMC_MapShmPtr(m_uiConnHndl);
    if (NULL == m_pShmPtr)
    {
        printf("Failed to map shared memory\n");
        CMMCPPGlobal::Instance()->MMCPPThrow("InitAxisData RT Failed to map shared memory:", m_uiConnHndl, m_usAxisRef, 0, 0,0);
    }

    MMC_GETSYSTEMDATA_OUT pOutParam;
    MMC_GetSystemData(m_uiConnHndl,&pOutParam);

    m_ulGlobalParam_base = (ELMO_ULINT32)(m_pShmPtr + pOutParam.uiGlobalParamsDataOffset);

    m_ulnode_list_param_base =(ELMO_ULINT32) (m_pShmPtr + pOutParam.uiNodeDataOffset);

    MMC_GETNODEDIRECTDATA_IN stGetDirectDataIn;
    MMC_GETNODEDIRECTDATA_OUT stGetDirectDataOut;
    stGetDirectDataIn.usRefAxis =  m_usAxisRef;
    stGetDirectDataIn.eNodeParam[0] = NC_REC_DESIRED_POS_LOW_PARAM;
    stGetDirectDataIn.eNodeParam[1] = NC_USR_607A_PARAM;
    stGetDirectDataIn.eNodeParam[2] = NC_USR_60FF_PARAM;
    stGetDirectDataIn.eNodeParam[3] = NC_USR_60B1_PARAM;
    stGetDirectDataIn.eNodeParam[4] = NC_USR_6071_PARAM;
    stGetDirectDataIn.eNodeParam[5] = NC_USR_60B2_PARAM;
    stGetDirectDataIn.eNodeParam[6] = NC_MB1_PARAM;
    stGetDirectDataIn.eNodeParam[7] = NC_MB2_PARAM;
    stGetDirectDataIn.eNodeParam[8] = NC_REC_DESIRED_VEL_LOW_PARAM;//NC_REC_D_TARGET_VEL_CNT_LOW_PARAM;//NC_REC_DESIRED_VEL_LOW_PARAM;
    stGetDirectDataIn.eNodeParam[9] = NC_REC_ACTUAL_POS_UU_LOW_PARAM;
    stGetDirectDataIn.eNodeParam[10] = NC_REC_ACTUAL_VEL_UU_LOW_PARAM; //dActualVelUU
    stGetDirectDataIn.eNodeParam[11] = NC_REC_ANALOG_INPUT_PARAM; //sAnalogInput
    stGetDirectDataIn.eNodeParam[12] = NC_REC_DIGITAL_INPUTS_PARAM; //uiDigitalInputs
    stGetDirectDataIn.eNodeParam[13] = NC_REC_ACTUAL_TORQUE_PARAM; //sActualCurrent
    stGetDirectDataIn.eNodeParam[14] = NC_REC_AXIS_STATUS_PARAM; //uistatus
    stGetDirectDataIn.eNodeParam[15] = NC_REC_CONTROL_WORD_PARAM;//usControlWord
    stGetDirectDataIn.eNodeParam[16] = NC_REC_STATUS_WORD_PARAM; //usStatusWord
    stGetDirectDataIn.eNodeParam[17] = NC_REC_USR_CW_PARAM; //user control word
    stGetDirectDataIn.eNodeParam[18] = NC_REC_I_ACTUAL_POS_PARAM;//NC_REC_ACTUAL_POS_PARAM; //actual position INT
    stGetDirectDataIn.eNodeParam[19] = NC_REC_ACTUAL_VEL_PARAM; //actual velocity INT
    stGetDirectDataIn.eNodeParam[20] = NC_STATUS_REGISTER;
    stGetDirectDataIn.eNodeParam[21] = NC_REC_TARGET_TORQUE_UU_LOW_PARAM;

    stGetDirectDataIn.eNodeParam[22] = NC_REC_I_USER_1_PARAM;
    stGetDirectDataIn.eNodeParam[23] = NC_REC_I_USER_AUX_1_PARAM;
    stGetDirectDataIn.eNodeParam[24] = NC_REC_I_USER_2_PARAM;
    stGetDirectDataIn.eNodeParam[25] = NC_REC_I_USER_AUX_2_PARAM;
    stGetDirectDataIn.eNodeParam[26] = NC_REC_I_USER_3_PARAM;
    stGetDirectDataIn.eNodeParam[27] = NC_REC_I_USER_AUX_3_PARAM;
    stGetDirectDataIn.eNodeParam[28] = NC_REC_I_USER_4_PARAM;
    stGetDirectDataIn.eNodeParam[29] = NC_REC_I_USER_AUX_4_PARAM;


    stGetDirectDataIn.eNodeParam[30] = NC_REC_F_USER_1_PARAM;
    stGetDirectDataIn.eNodeParam[31] = NC_REC_F_USER_AUX_1_PARAM;
    stGetDirectDataIn.eNodeParam[32] = NC_REC_F_USER_2_PARAM;
    stGetDirectDataIn.eNodeParam[33] = NC_REC_F_USER_AUX_2_PARAM;
    stGetDirectDataIn.eNodeParam[34] = NC_REC_F_USER_3_PARAM;
    stGetDirectDataIn.eNodeParam[35] = NC_REC_F_USER_AUX_3_PARAM;
    stGetDirectDataIn.eNodeParam[36] = NC_REC_F_USER_4_PARAM;
    stGetDirectDataIn.eNodeParam[37] = NC_REC_F_USER_AUX_4_PARAM;



    stGetDirectDataIn.eNodeParam[38] = (NC_REC_PARAM_NAMES_ENUM)-1;//NC_REC_MAX_NUM_PARAM;
    GetNodeDirectData(m_uiConnHndl,&stGetDirectDataIn,&stGetDirectDataOut);

    m_uldPosOffset           = stGetDirectDataOut.ulNodeDataOffset[0];
    m_ulUser607AOffset       = stGetDirectDataOut.ulNodeDataOffset[1];
    m_ulUser60FFOffset       = stGetDirectDataOut.ulNodeDataOffset[2];
    m_ulUser60B1Offset       = stGetDirectDataOut.ulNodeDataOffset[3];
    m_ulUser6071Offset       = stGetDirectDataOut.ulNodeDataOffset[4];
    m_ulUser60B2Offset       = stGetDirectDataOut.ulNodeDataOffset[5];
    m_ulMB1Offset            = stGetDirectDataOut.ulNodeDataOffset[6];
    m_ulMB2Offset            = stGetDirectDataOut.ulNodeDataOffset[7];
    m_ulTargetVelOffset      = stGetDirectDataOut.ulNodeDataOffset[8];
    m_ulActualPosOffset      = stGetDirectDataOut.ulNodeDataOffset[9];
    m_ulActualVelUUOffset    = stGetDirectDataOut.ulNodeDataOffset[10];
    m_ulAnsalogInputOffset   = stGetDirectDataOut.ulNodeDataOffset[11];
    m_uiDigitalInputsOffset  = stGetDirectDataOut.ulNodeDataOffset[12];
    m_ulActualCurrentOffset  = stGetDirectDataOut.ulNodeDataOffset[13];
    m_ulStatusOffset         = stGetDirectDataOut.ulNodeDataOffset[14];
    m_ulControlWordOffset    = stGetDirectDataOut.ulNodeDataOffset[15];
    m_ulStatusWordOffset     = stGetDirectDataOut.ulNodeDataOffset[16];
    m_ulUserCWOffset         = stGetDirectDataOut.ulNodeDataOffset[17];
    m_ulActualPosINTOffset   = stGetDirectDataOut.ulNodeDataOffset[18];
    m_ulActualVelINTOffset   = stGetDirectDataOut.ulNodeDataOffset[19];
    m_ulStatusRegisterOffset = stGetDirectDataOut.ulNodeDataOffset[20];
    m_ulTargetTorqueUUOffset = stGetDirectDataOut.ulNodeDataOffset[21];
    m_ulUserParamIntOffset[0] = stGetDirectDataOut.ulNodeDataOffset[22];
    m_ulUserParamIntOffset[1] = stGetDirectDataOut.ulNodeDataOffset[23];
    m_ulUserParamIntOffset[2] = stGetDirectDataOut.ulNodeDataOffset[24];
    m_ulUserParamIntOffset[3] = stGetDirectDataOut.ulNodeDataOffset[25];
    m_ulUserParamIntOffset[4] = stGetDirectDataOut.ulNodeDataOffset[26];
    m_ulUserParamIntOffset[5] = stGetDirectDataOut.ulNodeDataOffset[27];
    m_ulUserParamIntOffset[6] = stGetDirectDataOut.ulNodeDataOffset[28];
    m_ulUserParamIntOffset[7] = stGetDirectDataOut.ulNodeDataOffset[29];

    m_ulUserParamFloatOffset[0] = stGetDirectDataOut.ulNodeDataOffset[30];
    m_ulUserParamFloatOffset[1] = stGetDirectDataOut.ulNodeDataOffset[31];
    m_ulUserParamFloatOffset[2] = stGetDirectDataOut.ulNodeDataOffset[32];
    m_ulUserParamFloatOffset[3] = stGetDirectDataOut.ulNodeDataOffset[33];
    m_ulUserParamFloatOffset[4] = stGetDirectDataOut.ulNodeDataOffset[34];
    m_ulUserParamFloatOffset[5] = stGetDirectDataOut.ulNodeDataOffset[35];
    m_ulUserParamFloatOffset[6] = stGetDirectDataOut.ulNodeDataOffset[36];
    m_ulUserParamFloatOffset[7] = stGetDirectDataOut.ulNodeDataOffset[37];

    MMC_GETGLBLPARAMOFFSET_IN stGblParamOffsetIn;
    MMC_GETGLBLPARAMOFFSET_OUT stGblParamOffsetOut;

    GetGblParamOffset(m_uiConnHndl,&stGblParamOffsetIn,&stGblParamOffsetOut);

    m_CycleCounterOffset = stGblParamOffsetOut.ulGblParamOffset[0];

    MMC_READGROUPOFPARAMETERS_IN stReadGroupOfParamIn;
    MMC_READGROUPOFPARAMETERS_OUT stReadGroupOfParamOut;

    stReadGroupOfParamIn.ucNumberOfParameters = 3;
    stReadGroupOfParamIn.sParameters[0].eParameterNumber = MMC_TORQEMATORCRATIO;
    stReadGroupOfParamIn.sParameters[0].iParameterIndex = 0;
    stReadGroupOfParamIn.sParameters[0].usAxisRef = m_usAxisRef;
    stReadGroupOfParamIn.sParameters[1].eParameterNumber = MMC_MOTORRATEDCURRENT;
    stReadGroupOfParamIn.sParameters[1].iParameterIndex = 0;
    stReadGroupOfParamIn.sParameters[1].usAxisRef = m_usAxisRef;
    stReadGroupOfParamIn.sParameters[2].eParameterNumber = MMC_CYCLE_TIME_PARAM;
    stReadGroupOfParamIn.sParameters[2].iParameterIndex = 0;
    stReadGroupOfParamIn.sParameters[2].usAxisRef = m_usAxisRef;
    MMC_ReadGroupOfParameters(m_uiConnHndl,&stReadGroupOfParamIn,&stReadGroupOfParamOut);
    m_dbTorquemAToRCRatio = stReadGroupOfParamOut.dbValue[0];
    m_fMotorRatedCurrent = stReadGroupOfParamOut.dbValue[1];
    m_ulCycleTime = stReadGroupOfParamOut.dbValue[2];

#else
    m_dbTorquemAToRCRatio = this->GetParameter(MMC_TORQEMATORCRATIO,0);
    m_fMotorRatedCurrent = (ELMO_FLOAT)this->GetParameter(MMC_MOTORRATEDCURRENT,0);
#endif
}

void CMMCRTSingleAxis::SetUserCW(ELMO_UINT16 usUserCWval) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUserCWOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    USHORTPARAMETER(m_ulnode_list_param_base,m_ulUserCWOffset) = usUserCWval;
#else
    this->SetBoolParameter(usUserCWval,MMC_USERCW,0);
#endif
}

void CMMCRTSingleAxis::SetUser607A(int iUser607Aval) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser607AOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulUser607AOffset) = iUser607Aval;
#else
    this->SetBoolParameter(iUser607Aval,MMC_IUSER607A,0);
#endif
}

void CMMCRTSingleAxis::SetUser60FF(int iUser60FFval) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser60FFOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulUser60FFOffset) =  iUser60FFval;
#else
    this->SetBoolParameter(iUser60FFval,MMC_IUSER60FF,0);
#endif
}

void CMMCRTSingleAxis::SetUser60B1(int iUser60B1val) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser60B1Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulUser60B1Offset) =  iUser60B1val;
#else
    this->SetBoolParameter(iUser60B1val,MMC_IUSER60B1,0);
#endif
}

void CMMCRTSingleAxis::SetUser60B2(double dUser60B2val) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser60B2Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulUser60B2Offset) = (int)(dUser60B2val * m_dbTorquemAToRCRatio * 1000.0);
#else
    this->SetParameter(dUser60B2val,MMC_IUSER60B2,0);
#endif
}

void CMMCRTSingleAxis::SetUser6071(ELMO_DOUBLE dUser6071val) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser6071Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    SHORTPARAMETER(m_ulnode_list_param_base,m_ulUser6071Offset) = (ELMO_INT16)((ELMO_DOUBLE)(dUser6071val *(ELMO_DOUBLE) m_dbTorquemAToRCRatio  * 1000.0)) ;
#else
    this->SetBoolParameter((ELMO_INT16)((ELMO_DOUBLE)(dUser6071val *(ELMO_DOUBLE) m_dbTorquemAToRCRatio  * 1000.0)) ,MMC_IUSER6071,0);
#endif
}

void CMMCRTSingleAxis::SetUserMB1(ELMO_INT32 iUserMB1) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulMB1Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulMB1Offset) =  iUserMB1;
#else
    this->SetBoolParameter(iUserMB1,MMC_IUSERMB1,0);
#endif
}

void CMMCRTSingleAxis::SetUserMB2(ELMO_INT32 iUserMB2) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulMB2Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulMB2Offset) =  iUserMB2;
#else
    this->SetBoolParameter(iUserMB2,MMC_IUSERMB2,0);
#endif
}

//get user RT parameters

int CMMCRTSingleAxis::GetUserCW() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUserCWOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return USHORTPARAMETER(m_ulnode_list_param_base,m_ulUserCWOffset);
#else
    return this->GetBoolParameter(MMC_USERCW,0);
#endif
}

ELMO_INT32 CMMCRTSingleAxis::GetUser607A() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser607AOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return INTPARAMETER(m_ulnode_list_param_base,m_ulUser607AOffset);
#else
    return this->GetBoolParameter(MMC_IUSER607A,0);
#endif
}

ELMO_INT32 CMMCRTSingleAxis::GetUser60FF() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser60FFOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return INTPARAMETER(m_ulnode_list_param_base,m_ulUser60FFOffset);
#else
    return this->GetBoolParameter(MMC_IUSER60FF,0);
#endif
}

ELMO_INT32 CMMCRTSingleAxis::GetUser60B1() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser60B1Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return INTPARAMETER(m_ulnode_list_param_base,m_ulUser60B1Offset);
#else
    return this->GetBoolParameter(MMC_IUSER60B1,0);
#endif
}

ELMO_DOUBLE CMMCRTSingleAxis::GetUser60B2() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser60B2Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return ELMO_DOUBLE( (INTPARAMETER(m_ulnode_list_param_base,m_ulUser60B2Offset)) /  (m_dbTorquemAToRCRatio * 1000.0) );
#else
    return ELMO_DOUBLE((this->GetBoolParameter(MMC_IUSER60B2,0)) / (m_dbTorquemAToRCRatio * 1000.0));
#endif
}

ELMO_DOUBLE  CMMCRTSingleAxis::GetUser6071() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulUser6071Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return ELMO_DOUBLE( (SHORTPARAMETER(m_ulnode_list_param_base,m_ulUser6071Offset)) /  (m_dbTorquemAToRCRatio * 1000.0) );
#else
    return ELMO_DOUBLE((this->GetBoolParameter(MMC_IUSER6071,0)) / (m_dbTorquemAToRCRatio * 1000.0));
#endif

}

ELMO_INT32 CMMCRTSingleAxis::GetUserMB1() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulMB1Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return INTPARAMETER(m_ulnode_list_param_base,m_ulMB1Offset);
#else
    return this->GetBoolParameter(MMC_IUSERMB1,0);
#endif
}

ELMO_INT32 CMMCRTSingleAxis::GetUserMB2() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulMB2Offset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    return INTPARAMETER(m_ulnode_list_param_base,m_ulMB2Offset);
#else
    return this->GetBoolParameter(MMC_IUSERMB2,0);
#endif
}


ELMO_DOUBLE CMMCRTSingleAxis::GetdPos() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_uldPosOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return DOUBLEPARAMETER(m_ulnode_list_param_base,m_uldPosOffset);
#else
    return (ELMO_DOUBLE)(this->GetBoolParameter(MMC_AIM_POSITION_PARAM,0));
#endif
}

ELMO_DOUBLE CMMCRTSingleAxis::GetdVel() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulTargetVelOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    return (DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulTargetVelOffset) * (1/(m_ulCycleTime * 0.000001)));
#else
    return this->GetParameter(MMC_SET_VELOCITY_PARAM,0);
#endif
}

ELMO_DOUBLE CMMCRTSingleAxis::GetActualPosition()throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulActualPosOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulActualPosOffset);
#else
    return this->GetBoolParameter(MMC_ACT_POSITION_PARAM,0);
#endif
}

ELMO_DOUBLE CMMCRTSingleAxis::GetActualVelocity() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulActualVelUUOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulActualVelUUOffset);
#else
    return (ELMO_DOUBLE)(this->GetBoolParameter(MMC_ACT_VELOCITY_PARAM,0));
#endif
}

ELMO_INT16 CMMCRTSingleAxis::GetAnalogInput() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulAnsalogInputOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    return SHORTPARAMETER(m_ulnode_list_param_base,m_ulAnsalogInputOffset);
#else
    return (ELMO_INT16)(this->GetBoolParameter(MMC_ANALOG_INPUT,0));
#endif
}

ELMO_UINT32 CMMCRTSingleAxis::GetDigitalInputs() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_uiDigitalInputsOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    return UINTPARAMETER(m_ulnode_list_param_base,m_uiDigitalInputsOffset);
#else
    return (ELMO_UINT32)(this->GetBoolParameter(MMC_DIGITAL_INPUT_LOGIC,0));
#endif
}

void CMMCRTSingleAxis::SetDigitalOutPuts(ELMO_UINT32 uiDigitalOutputs) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_uiDigitalInputsOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    UINTPARAMETER(m_ulnode_list_param_base,m_uiDigitalInputsOffset)  = uiDigitalOutputs;
#else
    this->SetBoolParameter(uiDigitalOutputs,MMC_DIGITAL_INPUT_LOGIC,0);
#endif
}

ELMO_DOUBLE CMMCRTSingleAxis::GetActualCurrent() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulActualCurrentOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    return (SHORTPARAMETER(m_ulnode_list_param_base,m_ulActualCurrentOffset) * m_fMotorRatedCurrent);
#else
    return (ELMO_DOUBLE)(this->GetParameter(MMC_MOTORRATEDCURRENT, 0));
#endif
}

ELMO_UINT32 CMMCRTSingleAxis::GetPLCOpenStatus() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulStatusOffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return UINTPARAMETER(m_ulnode_list_param_base,m_ulStatusOffset);
#else
    return (ELMO_UINT32)(this->GetBoolParameter(MMC_AXIS_STATE_PARAM, 0));
#endif
}

ELMO_UINT16 CMMCRTSingleAxis::GetControlWord() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulControlWordOffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return USHORTPARAMETER(m_ulnode_list_param_base,m_ulControlWordOffset);
#else
    return 0;
#endif
}

ELMO_UINT16 CMMCRTSingleAxis::GetStatusWord() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_ulStatusWordOffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return USHORTPARAMETER(m_ulnode_list_param_base,m_ulStatusWordOffset);
#else
    return 0;
#endif
}

ELMO_ULINT32 CMMCRTSingleAxis::GetCycleCounter() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if(m_CycleCounterOffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return ULONGPARAMETER(m_ulGlobalParam_base,m_CycleCounterOffset);
#else
    return 0;
#endif
}


ELMO_INT32 CMMCRTSingleAxis::GetActualPositionINT() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (m_ulActualPosINTOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return INTPARAMETER(m_ulnode_list_param_base,m_ulActualPosINTOffset);
#else
    return this->GetBoolParameter(MMC_ACT_POSITION_PARAM, 0);
#endif
}

ELMO_INT32 CMMCRTSingleAxis::GetActualVelocityINT() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (m_ulActualVelINTOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return INTPARAMETER(m_ulnode_list_param_base,m_ulActualVelINTOffset);
#else
    return (ELMO_INT32)(this->GetBoolParameter(MMC_ACT_VELOCITY_PARAM,0));
#endif
}

ELMO_UINT32 CMMCRTSingleAxis::GetStatusRegister() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (m_ulStatusRegisterOffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return UINTPARAMETER(m_ulnode_list_param_base,m_ulStatusRegisterOffset);
#else
    return 0;
#endif
}

ELMO_DOUBLE CMMCRTSingleAxis::GetdTorqueUU() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (m_ulTargetTorqueUUOffset == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    return DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulTargetTorqueUUOffset);
#else
    return (ELMO_DOUBLE)(this->GetBoolParameter(MMC_AIM_POSITION_PARAM,0));
#endif
}

void CMMCRTSingleAxis::SetUI(ELMO_INT32 iValue, ELMO_UINT32 iIndex) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (iIndex > UI_UF_MAX_IDX )
        CMMCPPGlobal::Instance()->MMCPPThrow("Index out of range ", 0, 0, -1, -1, 0);
    if (m_ulUserParamIntOffset[iIndex - 1] == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    INTPARAMETER(m_ulnode_list_param_base,m_ulUserParamIntOffset[iIndex - 1]) =  iValue;
#else
    return;
#endif
}

ELMO_INT32 CMMCRTSingleAxis::GetUI(ELMO_UINT32 iIndex) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (iIndex > UI_UF_MAX_IDX )
        CMMCPPGlobal::Instance()->MMCPPThrow("Index out of range ", 0, 0, -1, -1, 0);

    if (m_ulUserParamIntOffset[iIndex-1] == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return INTPARAMETER(m_ulnode_list_param_base,m_ulUserParamIntOffset[iIndex - 1]);
#else
    return 0;
#endif
}

void CMMCRTSingleAxis::SetUF(ELMO_FLOAT fValue, ELMO_UINT32 iIndex) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (iIndex > UI_UF_MAX_IDX )
        CMMCPPGlobal::Instance()->MMCPPThrow("Index out of range ", 0, 0, -1, -1, 0);

    if (m_ulUserParamFloatOffset[iIndex - 1] == 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);

    FLOATPARAMETER(m_ulnode_list_param_base,m_ulUserParamFloatOffset[iIndex - 1]) =  fValue;
#else
    return ;
#endif
}

ELMO_FLOAT CMMCRTSingleAxis::GetUF(ELMO_UINT32 iIndex) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
    if (iIndex > UI_UF_MAX_IDX )
        CMMCPPGlobal::Instance()->MMCPPThrow("Index out of range ", 0, 0, -1, -1, 0);

    if (m_ulUserParamFloatOffset[iIndex - 1] == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
    }
    return FLOATPARAMETER(m_ulnode_list_param_base,m_ulUserParamFloatOffset[iIndex - 1]);
#else
    return 0;
#endif
}

void CMMCRTSingleAxis::sm_activation(int iAxisIndex)
{
    switch(m_stAPICom.uiInternalState)
    {
        case INTERNAL_STATE_WAIT_FOR_RISING:
        {
            if((m_stAPICom.uiInternalState == INTERNAL_STATE_WAIT_FOR_RISING) && (m_stAPICom.uiEnable))
            {
                m_stAPICom.uiIsInSequence = 1;
                m_stAPICom.uiInternalState = INTERNAL_STATE_ACITVATIE;
            }

            if(m_stAPICom.uiInternalState  == INTERNAL_STATE_ACITVATIE)
            {
                m_stAPICom.pFunc(iAxisIndex,m_usAxisRef);

                m_stAPICom.uiInternalState = INTERNAL_STATE_STATUS_POOL;
            }
        }
        break;
        //
        case INTERNAL_STATE_STATUS_POOL:
        {
//          while(!(ReadStatus() & NC_AXIS_ERROR_MASK ||
//                  ReadStatus() & NC_AXIS_STAND_STILL_MASK) )
//          {
//              continue;
//          }
            if((ReadStatus() & NC_AXIS_ERROR_MASK ||ReadStatus() & NC_AXIS_STAND_STILL_MASK) )
            {
                    m_stAPICom.uiInternalState = INTERNAL_STATE_WAIT_FOR_FALLING;
                    m_stAPICom.data[30] = 1;
            }
        }
        break;
        //
        case INTERNAL_STATE_WAIT_FOR_FALLING:
        {
            m_stAPICom.uiIsInSequence = 0;
            if((m_stAPICom.uiInternalState == INTERNAL_STATE_WAIT_FOR_FALLING) && (!m_stAPICom.uiEnable))
            {
                m_stAPICom.data[30] = 0;
                m_stAPICom.uiInternalState = INTERNAL_STATE_WAIT_FOR_RISING;
            }
        }
        break;
        //
    }
}

//#endif //#ifndef _WIN32
