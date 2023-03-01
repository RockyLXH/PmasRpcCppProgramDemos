/*
 * MMCPPGlobalRT.cpp
 *
 *  Created on: 23/04/2017
 *      Author: zivb
 *      Update: 22Sep2019 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */



#include "OS_PlatformDependSetting.hpp"

//#ifndef WIN32
#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)

#include "MMCPPGlobalRT.hpp"



extern "C"
{
    MMC_LIB_API ELMO_PINT8 MMC_MapShmPtr(MMC_CONNECT_HNDL hConn);
    MMC_LIB_API ELMO_INT32 MMC_UnmapShmPtr(ELMO_PINT8 pShmPtr);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
MMCPPGlobalRT::MMCPPGlobalRT()
{
    m_ulCPLDAnanlogCHA1Offset   = 0;
    m_ulCPLDAnanlogCHA2Offset   = 0;
    m_ulCPLDAnanlogCHB1Offset   = 0;
    m_ulCPLDAnanlogCHB2Offset   = 0;
    m_ulCPLDAnanlogOutCH1Offset = 0;
    m_ulCPLDAnanlogOutCH2Offset = 0;
    m_ulCPLDAnanlogOutCH3Offset = 0;
    m_ulCPLDAnanlogOutCH4Offset = 0;
    m_ulCPLDExtendedIOIn1ffset   = 0;
    m_ulCPLDExtendedIOout1ffset  = 0;
    m_ulCPLDExtendedIOout2ffset  = 0;
    m_ulCPLDExtendedIOout3ffset  = 0;

    for (ELMO_INT32 i = 0; i < SIL_DATA_REC_SIZE; i++)
    {
        m_ulSIL_DATA_LONG_offset[i] = 0;
        m_ulSIL_DATA_DOUBLE_offset[i] = 0;
    }

    for (ELMO_INT32 i = 0; i < SIL_TIME_CAP_SIZE; i++)
    {
        m_ulCPLDExtendedITimeCapLatchTimeOffset[i] = 0;
        m_ulCPLDExtendedITimeCapEvtCntOffset[i] = 0;
    }
}
MMCPPGlobalRT::~MMCPPGlobalRT()
{
    MMC_UnmapShmPtr((ELMO_PINT8)m_pShmPtr);
}

void MMCPPGlobalRT::Init( MMC_CONNECT_HNDL uHandle)
{
    MMC_GETGLBLPARAMOFFSET_IN pInParamGlobalParamOffset;
    MMC_GETGLBLPARAMOFFSET_OUT pOutParamGlobalParamOffset;

    m_uiConnHndl = uHandle;
    m_pShmPtr =  (ELMO_PUINT8)MMC_MapShmPtr(m_uiConnHndl);
    if (NULL == m_pShmPtr)
    {
        printf("Failed to map shared memory\n");
        CMMCPPGlobal::Instance()->MMCPPThrow("InitAxisData RT Failed to map shared memory:", m_uiConnHndl, 0, 0, 0,0);
    }

    MMC_GETSYSTEMDATA_OUT pOutParam;
    MMC_GetSystemData(m_uiConnHndl,&pOutParam);

    m_ulGlobalParam_base = (unsigned long)(m_pShmPtr + pOutParam.uiGlobalParamsDataOffset);


    GetGblParamOffset(m_uiConnHndl,&pInParamGlobalParamOffset,&pOutParamGlobalParamOffset);

    m_ulCycleCounterOffset          = pOutParamGlobalParamOffset.ulGblParamOffset[eCOUNTER_OFFSET_IDX];
    m_ulCPLDAnanlogCHA1Offset       = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_A_IN_1_OFFSET_IDX];
    m_ulCPLDAnanlogCHA2Offset       = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_A_IN_2_OFFSET_IDX];
    m_ulCPLDAnanlogCHB1Offset       = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_B_IN_1_OFFSET_IDX];
    m_ulCPLDAnanlogCHB2Offset       = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_B_IN_2_OFFSET_IDX];
    m_ulCPLDAnanlogOutCH1Offset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_OUT_1_OFFSET_IDX];
    m_ulCPLDAnanlogOutCH2Offset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_OUT_2_OFFSET_IDX];
    m_ulCPLDAnanlogOutCH3Offset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_OUT_3_OFFSET_IDX];
    m_ulCPLDAnanlogOutCH4Offset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_ANG_CH_OUT_4_OFFSET_IDX];
    m_ulCPLDExtendedIOIn1ffset      = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_IN_1_OFFSET_IDX];
    m_ulCPLDExtendedIOout1ffset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_OUT_1_OFFSET_IDX];
    m_ulCPLDExtendedIOout2ffset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_OUT_2_OFFSET_IDX];
    m_ulCPLDExtendedIOout3ffset     = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_OUT_3_OFFSET_IDX];

    memcpy(m_ulSIL_DATA_LONG_offset,&(pOutParamGlobalParamOffset.ulGblParamOffset[eSIL_LONG_DATA_0_OFFSET_IDX]),sizeof(m_ulSIL_DATA_LONG_offset));
    memcpy(m_ulSIL_DATA_DOUBLE_offset,&(pOutParamGlobalParamOffset.ulGblParamOffset[eSIL_DOUBLE_DATA_0_OFFSET_IDX]),sizeof(m_ulSIL_DATA_DOUBLE_offset));

    memcpy(m_ulCPLDExtendedITimeCapLatchTimeOffset,&(pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_TIME_CAP_TIME_LATCH0]),sizeof(m_ulCPLDExtendedITimeCapLatchTimeOffset));
    memcpy(m_ulCPLDExtendedITimeCapEvtCntOffset,&(pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_TIME_CAP_EVENT_CNT0]),sizeof(m_ulCPLDExtendedITimeCapEvtCntOffset));

 	m_ulCPLDEncoderQuad1PosUUOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_QUAD_1_POSUU_OFFSET_IDX];
	m_ulCPLDEncoderQuad2PosUUOffset = pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_QUAD_2_POSUU_OFFSET_IDX];
	m_ulCPLDEncoderQuad1VelOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_QUAD_1_VEL_OFFSET_IDX];
	m_ulCPLDEncoderQuad2VelOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_QUAD_2_VEL_OFFSET_IDX];
	m_ulCPLDEncoderAbs1PosUUOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_ABS_1_POSUU_OFFSET_IDX];
	m_ulCPLDEncoderAbs2PosUUOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_ABS_2_POSUU_OFFSET_IDX];
	m_ulCPLDEncoderAbs1VelOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_ABS_1_VEL_OFFSET_IDX];
	m_ulCPLDEncoderAbs2VelOffset	= pOutParamGlobalParamOffset.ulGblParamOffset[eCPLD_EXT_IO_ENC_ABS_2_VEL_OFFSET_IDX];

    m_ulSILBASEver = pOutParamGlobalParamOffset.ulGblParamOffset[eSIL_BASE_VER_IDX];

//	printf("m_ulGlobalParam_base = 0x%lX\n", m_ulGlobalParam_base);
//
//	printf("m_ulCycleCounterOffset = 0x%lX\n", m_ulCycleCounterOffset);
//
//	printf("m_ulCPLDAnanlogCHA1Offset = 0x%lX\n", m_ulCPLDAnanlogCHA1Offset);
//	printf("m_ulCPLDAnanlogCHA2Offset = 0x%lX\n", m_ulCPLDAnanlogCHA2Offset);
//	printf("m_ulCPLDAnanlogCHB1Offset = 0x%lX\n", m_ulCPLDAnanlogCHB1Offset);
//	printf("m_ulCPLDAnanlogCHB2Offset = 0x%lX\n", m_ulCPLDAnanlogCHB2Offset);
//
//	printf("m_ulCPLDAnanlogOutCH1Offset = 0x%lX\n", m_ulCPLDAnanlogOutCH1Offset);
//	printf("m_ulCPLDAnanlogOutCH2Offset = 0x%lX\n", m_ulCPLDAnanlogOutCH2Offset);
//	printf("m_ulCPLDAnanlogOutCH3Offset = 0x%lX\n", m_ulCPLDAnanlogOutCH3Offset);
//	printf("m_ulCPLDAnanlogOutCH4Offset = 0x%lX\n", m_ulCPLDAnanlogOutCH4Offset);
//
//	printf("m_ulCPLDExtendedIOIn1ffset = 0x%lX\n", m_ulCPLDExtendedIOIn1ffset);
//	printf("m_ulCPLDExtendedIOout1ffset = 0x%lX\n", m_ulCPLDExtendedIOout1ffset);
//	printf("m_ulCPLDExtendedIOout2ffset = 0x%lX\n", m_ulCPLDExtendedIOout2ffset);
//	printf("m_ulCPLDExtendedIOout3ffset = 0x%lX\n", m_ulCPLDExtendedIOout3ffset);
//
//	printf("m_ulSIL_DATA_LONG_offset[0] = 0x%lX\n", m_ulSIL_DATA_LONG_offset[0]);
//	printf("m_ulSIL_DATA_LONG_offset[1] = 0x%lX\n", m_ulSIL_DATA_LONG_offset[1]);
//	printf("m_ulSIL_DATA_DOUBLE_offset[0] = 0x%lX\n", m_ulSIL_DATA_DOUBLE_offset[0]);
//	printf("m_ulSIL_DATA_DOUBLE_offset[1] = 0x%lX\n", m_ulSIL_DATA_DOUBLE_offset[1]);
//
//	printf("m_ulCPLDExtendedITimeCapLatchTimeOffset[0] = 0x%lX\n", m_ulCPLDExtendedITimeCapLatchTimeOffset[0]);
//	printf("m_ulCPLDExtendedITimeCapLatchTimeOffset[1] = 0x%lX\n", m_ulCPLDExtendedITimeCapLatchTimeOffset[1]);
//	printf("m_ulCPLDExtendedITimeCapEvtCntOffset[0] = 0x%lX\n", m_ulCPLDExtendedITimeCapEvtCntOffset[0]);
//	printf("m_ulCPLDExtendedITimeCapEvtCntOffset[1] = 0x%lX\n", m_ulCPLDExtendedITimeCapEvtCntOffset[1]);
//
//	printf("m_ulCPLDEncoderQuad1PosUUOffset = 0x%lX\n", m_ulCPLDEncoderQuad1PosUUOffset);
//	printf("m_ulCPLDEncoderQuad2PosUUOffset = 0x%lX\n", m_ulCPLDEncoderQuad2PosUUOffset);
//	printf("m_ulCPLDEncoderQuad1VelOffset = 0x%lX\n", m_ulCPLDEncoderQuad1VelOffset);
//	printf("m_ulCPLDEncoderQuad2VelOffset = 0x%lX\n", m_ulCPLDEncoderQuad2VelOffset);
//
//	printf("m_ulCPLDEncoderAbs1PosUUOffset = 0x%lX\n", m_ulCPLDEncoderAbs1PosUUOffset);
//	printf("m_ulCPLDEncoderAbs2PosUUOffset = 0x%lX\n", m_ulCPLDEncoderAbs2PosUUOffset);
//	printf("m_ulCPLDEncoderAbs1VelOffset = 0x%lX\n", m_ulCPLDEncoderAbs1VelOffset);
//	printf("m_ulCPLDEncoderAbs2VelOffset = 0x%lX\n", m_ulCPLDEncoderAbs2VelOffset);
//
//	printf("m_ulSILBASEver = 0x%lX\n", m_ulSILBASEver);
}

ELMO_ULINT32 MMCPPGlobalRT::GetCycleCounter() throw (CMMCException)
{
    if(m_ulCycleCounterOffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCycleCounter RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    return ULONGPARAMETER(m_ulGlobalParam_base,m_ulCycleCounterOffset);
}


ELMO_DOUBLE MMCPPGlobalRT::GetCpldAdcCh1() throw (CMMCException)
{
    if(m_ulCPLDAnanlogCHA1Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAdcChA1 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    return DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogCHA1Offset);
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldAdcCh2() throw (CMMCException)
{
    if(m_ulCPLDAnanlogCHA2Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAdcChA2 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    return DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogCHA2Offset);
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldAdcCh3() throw (CMMCException)
{
    if(m_ulCPLDAnanlogCHB1Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAdcChB1 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    return DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogCHB1Offset);
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldAdcCh4() throw (CMMCException)
{
    if(m_ulCPLDAnanlogCHB2Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAdcChB2 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    return DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogCHB2Offset);
}


void MMCPPGlobalRT::SetCpldAdcOUTCh1(ELMO_DOUBLE lValue) throw (CMMCException)
{
    if(m_ulCPLDAnanlogOutCH1Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldAdcOUTChA1 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogOutCH1Offset) = lValue;
}

void MMCPPGlobalRT::SetCpldAdcOUTCh2(ELMO_DOUBLE lValue) throw (CMMCException)
{
    if(m_ulCPLDAnanlogOutCH2Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldAdcOUTChA2 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogOutCH2Offset) = lValue;
}

void MMCPPGlobalRT::SetCpldAdcOUTCh3(ELMO_DOUBLE lValue) throw (CMMCException)
{
    if(m_ulCPLDAnanlogOutCH3Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldAdcOUTChB1 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogOutCH3Offset) = lValue;
}

void MMCPPGlobalRT::SetCpldAdcOUTCh4(ELMO_DOUBLE lValue) throw (CMMCException)
{
    if(m_ulCPLDAnanlogOutCH4Offset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldAdcOUTChB2 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulCPLDAnanlogOutCH4Offset) = lValue;
}

ELMO_ULINT32 MMCPPGlobalRT::GetCpldExtendedIOIn1() throw (CMMCException)
{
    if(m_ulCPLDExtendedIOIn1ffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldExtendedIOIn1 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    return ULONGPARAMETER(m_ulGlobalParam_base,m_ulCPLDExtendedIOIn1ffset);
}

void MMCPPGlobalRT::SetCpldExtendedIOOut1(ELMO_ULINT32 ulVal) throw (CMMCException)
{
    if(m_ulCPLDExtendedIOout1ffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldExtendedIOOut1 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    ULONGPARAMETER(m_ulGlobalParam_base,m_ulCPLDExtendedIOout1ffset) = ulVal;
}

void MMCPPGlobalRT::SetCpldExtendedIOOut2(ELMO_ULINT32 ulVal) throw (CMMCException)
{
    if(m_ulCPLDExtendedIOout2ffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldExtendedIOOut2 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    ULONGPARAMETER(m_ulGlobalParam_base,m_ulCPLDExtendedIOout2ffset) = ulVal;
}

void MMCPPGlobalRT::SetCpldExtendedIOOut3(ELMO_ULINT32 ulVal) throw (CMMCException)
{
    if(m_ulCPLDExtendedIOout3ffset == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetCpldExtendedIOOut3 RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    ULONGPARAMETER(m_ulGlobalParam_base,m_ulCPLDExtendedIOout3ffset) = ulVal;
}

void MMCPPGlobalRT::SetSILLongData(ELMO_INT32 iIndex, ELMO_LINT32 lVal) throw (CMMCException)
{
    if(m_ulSIL_DATA_LONG_offset[iIndex] == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetSILLongData RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    LONGPARAMETER(m_ulGlobalParam_base,m_ulSIL_DATA_LONG_offset[iIndex]) = lVal;
}

void MMCPPGlobalRT::SetSILDoubleData(ELMO_INT32 iIndex, ELMO_DOUBLE dVal) throw (CMMCException)
{
    ELMO_DOUBLE dValue;
    if(m_ulSIL_DATA_DOUBLE_offset[iIndex] == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SetSILDoubleData RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulSIL_DATA_DOUBLE_offset[iIndex]) = dVal;
    dValue = DOUBLEPARAMETER(m_ulGlobalParam_base,m_ulSIL_DATA_DOUBLE_offset[iIndex]);
}

void MMCPPGlobalRT::GetInputCapture(ELMO_UINT16 usIndex, ELMO_PUINT32 uiEventCounter, ELMO_PUINT32 uiCaptureTime)  throw (CMMCException)
{
    //index should be between 1 to 5
    if((usIndex  > SIL_TIME_CAP_SIZE) || (usIndex < 1))
        CMMCPPGlobal::Instance()->MMCPPThrow("GetInputCapture RT GLOBAL Index out of range 1-5 ", 0, 0, -1, -1, 0);
    usIndex--;
    if(m_ulCPLDExtendedITimeCapLatchTimeOffset[usIndex] == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetInputCapture RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    if(m_ulCPLDExtendedITimeCapEvtCntOffset[usIndex] == 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("GetInputCapture RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
    }
    (*uiCaptureTime) = UINTPARAMETER(m_ulGlobalParam_base,m_ulCPLDExtendedITimeCapLatchTimeOffset[usIndex]);
    (*uiEventCounter) = UINTPARAMETER(m_ulGlobalParam_base,m_ulCPLDExtendedITimeCapEvtCntOffset[usIndex]);
}

ELMO_ULINT32 MMCPPGlobalRT::GetSILBAEver ()
{
    return m_ulSILBASEver;
}




ELMO_DOUBLE MMCPPGlobalRT::GetCpldQuad1PosUU() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderQuad1PosUUOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldQuad1PosUU RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderQuad1PosUUOffset);
//#else
	return 0;
//#endif

ELMO_DOUBLE MMCPPGlobalRT::GetCpldQuad2PosUU() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderQuad2PosUUOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldQuad2PosUU RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderQuad2PosUUOffset);
//#else
	return 0;
//#endif
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldQuad1Vel() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderQuad1VelOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldQuad1Vel RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderQuad1VelOffset);
//#else
	return 0;
//#endif
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldQuad2Vel() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderQuad2VelOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldQuad2Vel RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderQuad2VelOffset);
//#else
	return 0;
//#endif
}


ELMO_DOUBLE MMCPPGlobalRT::GetCpldAbs1PosUU() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderAbs1PosUUOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAbs1PosUU RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderAbs1PosUUOffset);
//#else
	return 0;
//#endif
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldAbs2PosUU() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderAbs2PosUUOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAbs2PosUU RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderAbs2PosUUOffset);
//#else
	return 0;
//#endif
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldAbs1Vel() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderAbs1VelOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAbs1Vel RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderAbs1VelOffset);
//#else
	return 0;
//#endif
}

ELMO_DOUBLE MMCPPGlobalRT::GetCpldAbs2Vel() throw (CMMCException)
{
//#ifndef WIN32
	if(m_ulCPLDEncoderAbs2VelOffset == 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("GetCpldAbs2Vel RT GLOBAL is not initialized ", 0, 0, -1, -1, 0);
	}
	return DOUBLEPARAMETER(m_ulGlobalParam_base, m_ulCPLDEncoderAbs2VelOffset);
//#else
	return 0;
//#endif
}


#endif /* #if   (OS_PLATFORM == LINUXIPC32_PLATFORM) */

