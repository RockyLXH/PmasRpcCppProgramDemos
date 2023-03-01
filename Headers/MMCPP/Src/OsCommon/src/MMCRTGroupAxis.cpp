/*
 * MMCRTGroupAxis.cpp
 *
 *  Created on: 07/07/2016
 *      Author: zivb
 */



#include "OS_PlatformDependSetting.hpp"
//#include "StdAfx.h"
#include "MMCRTGroupAxis.hpp"

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
extern "C"
{
    MMC_LIB_API ELMO_PINT8 MMC_MapShmPtr(MMC_CONNECT_HNDL hConn);
    MMC_LIB_API ELMO_INT32 MMC_UnmapShmPtr(ELMO_PINT8 pShmPtr);
}
#endif // !WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMMCRTGroupAxis::CMMCRTGroupAxis() : CMMCGroupAxis()
{
        m_ulaPosOffset                   = 0;
        m_ulaVelOffset                   = 0;
}

CMMCRTGroupAxis::~CMMCRTGroupAxis()
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        MMC_UnmapShmPtr((ELMO_PINT8)m_pShmPtr);
#endif // !WIN32
}

CMMCRTGroupAxis::CMMCRTGroupAxis(CMMCGroupAxis& axis):CMMCGroupAxis(axis)
{
        m_ulaPosOffset                   = 0;
        m_ulaVelOffset                   = 0;
        m_ulVecVelocityOffset    = 0;
        m_ulACDCOffset                   = 0;
}


void CMMCRTGroupAxis::InitAxisData(const ELMO_PINT8 cName, MMC_CONNECT_HNDL uHandle) throw (CMMCException)
{
        CMMCGroupAxis::InitAxisData(cName,uHandle);

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
        stGetDirectDataIn.eNodeParam[0] = NC_REC_DESIRED_PCS_X_POS_LOW_PARAM; //aPos[]
        stGetDirectDataIn.eNodeParam[1] = NC_REC_DESIRED_PCS_X_VEL_LOW_PARAM; //aVel[]
        stGetDirectDataIn.eNodeParam[2] = NC_REC_AXIS_STATUS_PARAM;
        stGetDirectDataIn.eNodeParam[3] = NC_REC_VECT_VEL_LOW_PARAM;
        stGetDirectDataIn.eNodeParam[4] = NC_REC_DESIRED_PCS_X_AC_DC_LOW_PARAM;
//      stGetDirectDataIn.eNodeParam[2] = NC_USR_60FF_PARAM; //aJerk[]
        stGetDirectDataIn.eNodeParam[5] = (NC_REC_PARAM_NAMES_ENUM)-1;//NC_REC_MAX_NUM_PARAM;
        GetNodeDirectData(m_uiConnHndl,&stGetDirectDataIn,&stGetDirectDataOut);

        m_ulaPosOffset                   = stGetDirectDataOut.ulNodeDataOffset[0];
        m_ulaVelOffset                   = stGetDirectDataOut.ulNodeDataOffset[1];
        m_ulStatusOffset                 = stGetDirectDataOut.ulNodeDataOffset[2];
        m_ulVecVelocityOffset    =  stGetDirectDataOut.ulNodeDataOffset[3];
        m_ulACDCOffset                   =  stGetDirectDataOut.ulNodeDataOffset[4];
#endif // !WIN32

}

void CMMCRTGroupAxis::GetPos(ELMO_PDOUBLE dArr,NC_AXIS_IN_GROUP_TYPE_ENUM_EX startIndex ,NC_AXIS_IN_GROUP_TYPE_ENUM_EX endIndex)throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        if(m_ulaPosOffset == 0)
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP is not initialized ", 0, 0, -1, -1, 0);
#endif

        if( (startIndex > endIndex) || (dArr == NULL) )
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP WRONG INPUT PARAMETERS ", 0, 0, -1, -1, 0);
				
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        for(ELMO_INT32 i = (ELMO_INT32)startIndex; i<=endIndex ;i++)
        {
                dArr[i - startIndex ] = DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulaPosOffset+ i*sizeof(ELMO_DOUBLE));
        }
#else
        //this->GetPos(dArr, startIndex, endIndex);
#endif
}

void CMMCRTGroupAxis::GetVel(ELMO_PDOUBLE dArr, NC_AXIS_IN_GROUP_TYPE_ENUM_EX startIndex, NC_AXIS_IN_GROUP_TYPE_ENUM_EX endIndex) throw (CMMCException)
{

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        if(m_ulaVelOffset == 0)
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP is not initialized ", 0, 0, -1, -1, 0);
#endif

        if( (startIndex > endIndex) || (dArr == NULL) )
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP WRONG INPUT PARAMETERS ", 0, 0, -1, -1, 0);

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        for(ELMO_INT32 i = (int)startIndex; i<=endIndex ;i++)
        {
                dArr[i - startIndex ] = DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulaVelOffset+i*sizeof(ELMO_DOUBLE));
        }
#else
        //this->GetVel(dArr, startIndex, endIndex);
#endif

}

void CMMCRTGroupAxis::GetACDC(ELMO_PDOUBLE dArr, NC_AXIS_IN_GROUP_TYPE_ENUM_EX startIndex, NC_AXIS_IN_GROUP_TYPE_ENUM_EX endIndex)throw (CMMCException)
{

//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        if(m_ulaPosOffset == 0)
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP is not initialized ", 0, 0, -1, -1, 0);
#endif

        if( (startIndex > endIndex) || (dArr == NULL) )
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP WRONG INPUT PARAMETERS ", 0, 0, -1, -1, 0);
                
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        for(ELMO_INT32 i = (ELMO_INT32)startIndex; i<=endIndex ;i++)
        {
                dArr[i - startIndex ] = DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulACDCOffset+ i*sizeof(double));
        }
#else
        //this->GetPos(dArr, startIndex, endIndex);
#endif

}

/*
void CMMCRTGroupAxis::GetJerk(ELMO_PDOUBLE dArr, NC_AXIS_IN_GROUP_TYPE_ENUM_EX startIndex, NC_AXIS_IN_GROUP_TYPE_ENUM_EX endIndex) throw (CMMCException)
{
        if(m_sData_ptr == NULL)
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP is not initialized ", 0, 0, -1, -1, 0);

        if( (startIndex > endIndex) || (dArr == NULL) )
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP WRONG INPUT PARAMETERS ", 0, 0, -1, -1, 0);

        for(int i = (int)startIndex; i<=endIndex ;i++)
        {
                dArr[i - startIndex ] = m_sData_ptr->aJerk[i];
        }
}
*/


ELMO_UINT32 CMMCRTGroupAxis::GetPLCOpenStatus() throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)
        if (m_ulStatusOffset == 0)
        {
                CMMCPPGlobal::Instance()->MMCPPThrow("RT AXIS is not initialized ", 0, 0, -1, -1, 0);
        }
        return UINTPARAMETER(m_ulnode_list_param_base, m_ulStatusOffset);
#else
        return (ELMO_UINT32)(this->GetBoolParameter(MMC_AXIS_STATE_PARAM, 0));
#endif
}

void CMMCRTGroupAxis::GetVectorVel(ELMO_PDOUBLE dValue) throw (CMMCException)
{
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)

        if(m_ulVecVelocityOffset == 0)
                CMMCPPGlobal::Instance()->MMCPPThrow("RT GROUP is not initialized ", 0, 0, -1, -1, 0);

                (*dValue) = DOUBLEPARAMETER(m_ulnode_list_param_base,m_ulVecVelocityOffset);
#else
        //this->GetVel(dArr, startIndex, endIndex);
#endif
}
