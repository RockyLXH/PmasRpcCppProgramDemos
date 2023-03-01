/*
 * MMCBulkRead.cpp
 *
 *  Created on: 19/01/2012
 *      Author: alexanderk
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCBulkRead.hpp"

CMMCBulkRead::CMMCBulkRead()
{
    m_hConnectHandle = (MMC_CONNECT_HNDL)(-1);
    m_eConfiguration = eBULKREAD_CONFIG_NONE;
    m_ePreset = eNC_BULKREAD_PRESET_NONE;
    m_ucIsPreset = 0;

    m_usNumberOfAxes = 0;
    memset(m_usAxisRefArray, 0, sizeof(NC_MAX_AXES_PER_BULK_READ) * sizeof(ELMO_INT16));
    memset(m_ulRvArray, 0, NC_MAX_REC_SIGNALS_NUM * sizeof(ELMO_LINT32));
    memset(m_fFactorsArray, 0, NC_MAX_BULK_READ_READABLE_PACKET_SIZE * sizeof(ELMO_FLOAT));

    if ((0 < m_usNumberOfAxes) && (NC_MAX_AXES_PER_BULK_READ >= m_usNumberOfAxes))
    {
        for (ELMO_UINT32 i = 0; i < m_usNumberOfAxes; i++)
        {
            m_usAxisRefArray[i] = i;
        }
    }

    m_pPreset2Data.stAxisParams = NULL;
}


CMMCBulkRead::CMMCBulkRead(MMC_CONNECT_HNDL hConnectHandle)
{
    m_hConnectHandle = hConnectHandle;
    m_eConfiguration = eBULKREAD_CONFIG_NONE;
    m_ePreset = eNC_BULKREAD_PRESET_NONE;
    m_ucIsPreset = 0;

    m_usNumberOfAxes = GetAxesNum();
    memset(m_usAxisRefArray, 0, sizeof(NC_MAX_AXES_PER_BULK_READ) * sizeof(ELMO_INT16));
    memset(m_ulRvArray, 0, NC_MAX_REC_SIGNALS_NUM * sizeof(ELMO_LINT32));
    memset(m_fFactorsArray, 0, NC_MAX_BULK_READ_READABLE_PACKET_SIZE * sizeof(ELMO_FLOAT));

    if ((0 < m_usNumberOfAxes) && (NC_MAX_AXES_PER_BULK_READ >= m_usNumberOfAxes))
    {
        for (ELMO_UINT32 i = 0; i < m_usNumberOfAxes; i++)
        {
            m_usAxisRefArray[i] = i;
        }
    }

    m_pPreset2Data.stAxisParams = NULL;
}

CMMCBulkRead::CMMCBulkRead(MMC_CONNECT_HNDL hConnectHandle,
                           NC_BULKREAD_CONFIG_ENUM eConfig,
                           NC_BULKREAD_PRESET_ENUM ePreset)
{
    m_hConnectHandle = hConnectHandle;

    m_eConfiguration = eConfig;
    m_ePreset = ePreset;
    m_usNumberOfAxes = GetAxesNum();
    memset(m_usAxisRefArray, 0, sizeof(NC_MAX_AXES_PER_BULK_READ) * sizeof(ELMO_INT16));
    memset(m_ulRvArray, 0, NC_MAX_REC_SIGNALS_NUM * sizeof(ELMO_LINT32));
    memset(m_fFactorsArray, 0, NC_MAX_BULK_READ_READABLE_PACKET_SIZE * sizeof(ELMO_FLOAT));

    if ((0 < m_usNumberOfAxes) && (NC_MAX_AXES_PER_BULK_READ >= m_usNumberOfAxes))
    {
        for (ELMO_UINT32 i = 0; i < m_usNumberOfAxes; i++)
        {
            m_usAxisRefArray[i] = i;
        }
    }

    if ((eNC_BULKREAD_PRESET_2 == m_ePreset) && (NULL == m_pPreset2Data.stAxisParams))
    {
        m_pPreset2Data.stAxisParams = new NC_BULKREAD_PRESET_1[m_usNumberOfAxes];
    }
}

CMMCBulkRead::~CMMCBulkRead()
{
    if (NULL != m_pPreset2Data.stAxisParams)
    {
        delete[] m_pPreset2Data.stAxisParams;
        m_pPreset2Data.stAxisParams = NULL;
    }
}

int CMMCBulkRead::GetAxesNum() throw (CMMCException)
{
    MMC_GETACTIVEAXESNUM_IN stGetActiveAxesNumIn;
    MMC_GETACTIVEAXESNUM_OUT stGetActiveAxesNumOut;

    ELMO_INT32 rc = NC_OK;

    rc = MMC_GetActiveAxesNum(m_hConnectHandle, &stGetActiveAxesNumIn, &stGetActiveAxesNumOut);
    if (NC_OK != rc)
    {
        return stGetActiveAxesNumOut.usErrorID;
    }

    return stGetActiveAxesNumOut.iActiveAxesNum;
}

void CMMCBulkRead::SetDefaultParameters()
{
    m_eConfiguration = (NC_BULKREAD_CONFIG_ENUM)BULKREAD_DEFAULT_CONFIG;
    m_ePreset = (NC_BULKREAD_PRESET_ENUM)BULKREAD_DEFAULT_PRESET;
    m_ucIsPreset = BULKREAD_DEFAULT_IS_PRESET;
}

void CMMCBulkRead::SetDefaultParameters(MMC_CONFIGBULKREAD_IN stConfigBulkReadParams)
{
    m_eConfiguration = stConfigBulkReadParams.eConfiguration;
    m_ucIsPreset = stConfigBulkReadParams.ucIsPreset;

    if (0 == m_ucIsPreset)
    {
        memcpy(m_ulRvArray,
               stConfigBulkReadParams.uBulkReadParams.ulBulkReadParameters,
               sizeof(ELMO_ULINT32) * NC_MAX_REC_SIGNALS_NUM);
    }

    else
    {
        m_ePreset = stConfigBulkReadParams.uBulkReadParams.eBulkReadPreset;
        if ((eNC_BULKREAD_PRESET_2 == m_ePreset) && (NULL == m_pPreset2Data.stAxisParams))
        {
            m_pPreset2Data.stAxisParams = new NC_BULKREAD_PRESET_1 [m_usNumberOfAxes];
        }
    }

    if (m_usNumberOfAxes != stConfigBulkReadParams.usNumberOfAxes)
    {
        memset(m_usAxisRefArray, 0, NC_MAX_AXES_PER_BULK_READ * sizeof(ELMO_LINT32));
    }
}

void CMMCBulkRead::Config() throw (CMMCException)
{
    ELMO_INT32 rc = NC_OK;
    MMC_CONFIGBULKREAD_IN stCfgBulkReadIn;
    MMC_CONFIGBULKREAD_OUT stCfgBulkReadOut;

    stCfgBulkReadIn.usNumberOfAxes = m_usNumberOfAxes;
    stCfgBulkReadIn.eConfiguration = m_eConfiguration;
    stCfgBulkReadIn.ucIsPreset = m_ucIsPreset;

    if (0 != m_ucIsPreset)
    {

        stCfgBulkReadIn.uBulkReadParams.eBulkReadPreset = m_ePreset;
    }

    else
    {
        memcpy(stCfgBulkReadIn.uBulkReadParams.ulBulkReadParameters,
               m_ulRvArray,
               NC_MAX_REC_SIGNALS_NUM * sizeof(ELMO_LINT32));
    }

    memcpy(stCfgBulkReadIn.usAxisRefArray, m_usAxisRefArray, m_usNumberOfAxes * sizeof(ELMO_INT16));

    rc = MMC_ConfigBulkReadCmd(m_hConnectHandle, &stCfgBulkReadIn, &stCfgBulkReadOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigBulkReadCmd:", m_hConnectHandle, 0, rc, stCfgBulkReadOut.usErrorID, stCfgBulkReadOut.usStatus);
    }

    memcpy(m_fFactorsArray,
           stCfgBulkReadOut.fFactorsArray,
           NC_MAX_BULK_READ_READABLE_PACKET_SIZE * sizeof(ELMO_FLOAT));
}


void CMMCBulkRead::Config(MMC_CONFIGBULKREAD_IN stCfgBulkReadIn) throw (CMMCException)
{
    int rc = NC_OK;
    MMC_CONFIGBULKREAD_OUT stCfgBulkReadOut;
    m_eConfiguration = stCfgBulkReadIn.eConfiguration;
    m_ucIsPreset = stCfgBulkReadIn.ucIsPreset;
    m_ePreset = stCfgBulkReadIn.uBulkReadParams.eBulkReadPreset;
    memcpy((ELMO_PVOID)m_ulRvArray,(ELMO_PVOID)stCfgBulkReadIn.uBulkReadParams.ulBulkReadParameters,sizeof(NC_MAX_REC_SIGNALS_NUM));
    memcpy((ELMO_PVOID)m_usAxisRefArray,(ELMO_PVOID)stCfgBulkReadIn.usAxisRefArray,sizeof(NC_MAX_AXES_PER_BULK_READ));
    m_usNumberOfAxes = stCfgBulkReadIn.usNumberOfAxes;  
    m_ucIsPreset = stCfgBulkReadIn.ucIsPreset;


    rc = MMC_ConfigBulkReadCmd(m_hConnectHandle, &stCfgBulkReadIn, &stCfgBulkReadOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ConfigBulkReadCmd:", m_hConnectHandle, 0, rc, stCfgBulkReadOut.usErrorID, stCfgBulkReadOut.usStatus);
    }

    memcpy(m_fFactorsArray,
           stCfgBulkReadOut.fFactorsArray,
           NC_MAX_BULK_READ_READABLE_PACKET_SIZE * sizeof(ELMO_FLOAT));
}

void CMMCBulkRead::BulkRead() throw (CMMCException)
{
    MMC_PERFORMBULKREAD_IN stPerformBulkReadIn;
    MMC_PERFORMBULKREAD_OUT stPerformBulkReadOut;
    int iGlobalParamsStartOffset = 0;
    int rc = NC_OK;

    stPerformBulkReadIn.eConfiguration = m_eConfiguration;

    rc = MMC_PerformBulkReadCmd(m_hConnectHandle, &stPerformBulkReadIn, &stPerformBulkReadOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PerformBulkReadCmd:", m_hConnectHandle, 0, rc, stPerformBulkReadOut.usErrorID, stPerformBulkReadOut.usStatus);
    }

    switch (m_ePreset)
    {
        case eNC_BULKREAD_PRESET_1:
            memcpy(m_pPreset1Data, stPerformBulkReadOut.ulOutBuf, m_usNumberOfAxes * sizeof(NC_BULKREAD_PRESET_1));
            break;

        case eNC_BULKREAD_PRESET_2:
            iGlobalParamsStartOffset = m_usNumberOfAxes * (sizeof(NC_BULKREAD_PRESET_1) / sizeof(ELMO_LINT32));
            if (NULL != m_pPreset2Data.stAxisParams)
            {
                memcpy(m_pPreset2Data.stAxisParams, stPerformBulkReadOut.ulOutBuf, m_usNumberOfAxes * sizeof(NC_BULKREAD_PRESET_1));
            }
            else
            {
                CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PerformBulkReadCmd:", m_hConnectHandle, 0, rc, stPerformBulkReadOut.usErrorID, stPerformBulkReadOut.usStatus);
            }

            m_pPreset2Data.iFreeLargeFbsNumber = stPerformBulkReadOut.ulOutBuf[iGlobalParamsStartOffset];
            m_pPreset2Data.iFreeMediumFbsNumber = stPerformBulkReadOut.ulOutBuf[iGlobalParamsStartOffset + 1];
            m_pPreset2Data.iFreeSmallFbsNumber = stPerformBulkReadOut.ulOutBuf[iGlobalParamsStartOffset + 2];
            break;

        case eNC_BULKREAD_PRESET_3:
            memcpy(m_pPreset3Data, stPerformBulkReadOut.ulOutBuf, m_usNumberOfAxes * sizeof(NC_BULKREAD_PRESET_3));
            break;

        case eNC_BULKREAD_PRESET_4:
            memcpy(m_pPreset4Data, stPerformBulkReadOut.ulOutBuf, m_usNumberOfAxes * sizeof(NC_BULKREAD_PRESET_4));
            break;

        case eNC_BULKREAD_PRESET_5:
            memcpy(m_pPreset5Data, stPerformBulkReadOut.ulOutBuf, m_usNumberOfAxes * sizeof(NC_BULKREAD_PRESET_5));
            break;

        default:
            break;

    }
}
