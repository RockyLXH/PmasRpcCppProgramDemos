/*
 * MMCPIBulk.cpp
 *
 *  Created on: Mar 9, 2014
 *      Author: ZivB
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCPIBulk.hpp"

#define CELL_BYTE_SIZE      4
#define END_OF_PI_VAR       1
#define NEXT_CELL_OFFSET    1
#define SHIFT_32_BIT        32
#define INCREMENT_BY_2      2 //Increment by 2 for type which are 64bit
#define FLAG_ON_VAL         1


    /*! \fn default constructor
    */
CMMCPIBulk::CMMCPIBulk()
{
    m_uiConnHndl = 0;
    m_eConfiguration = eBULKREAD_CONFIG_PI_MAX;
    m_iIndexInDataBuffer = 0;
    m_iIndexInInputBuffer = 0;
    m_iIsBulkReadInitializedFlag = 0;
}

    /*! \fn constructor
    */
CMMCPIBulk::CMMCPIBulk(MMC_CONNECT_HNDL ConnHndl, int iSize, NC_BULKREAD_CONFIG_PI_ENUM  eConfiguration, CMMCPIVar *PIVARArr) throw (CMMCException)
{
    m_iIsBulkReadInitializedFlag = 0;
    Init(ConnHndl, iSize, eConfiguration, PIVARArr);
}

    /*! \fn destructor
     */
CMMCPIBulk::~CMMCPIBulk()
{
    Clear();
}

/*! \fn Init()
*   \brief set inital value for class
*/
void CMMCPIBulk::Init(MMC_CONNECT_HNDL ConnHndl, int iSize, NC_BULKREAD_CONFIG_PI_ENUM  eConfiguration, CMMCPIVar *PIVARArr) throw (CMMCException)
{
    //check if the CMMCPIBulk is allready inititalized.
    if(m_iIsBulkReadInitializedFlag)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Init",m_uiConnHndl, 0, MMC_LIB_PI_BULK_ALLREADY_INITIALIZED, 0, 0);
    }

    m_iIndexInDataBuffer = 0;
    m_iIndexInInputBuffer = 0;

    m_uiConnHndl = ConnHndl;
    m_eConfiguration = eConfiguration;
    m_iIsBulkReadInitializedFlag = 1;

    if( (iSize > NC_MAX_PI_BULK_READ_VARIABLES) || (iSize < 1) )
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Init",m_uiConnHndl, 0, MMC_LIB_PI_BULK_TRY_TO_EXCEED_BULK_READ_PI_SIGNALS_NUMBER, 0, 0);
    }
    //
    //If we received array of PIVAR we need to copy the relevant data
    if(PIVARArr)
    {
        //
        for(ELMO_INT32 i=0; i<iSize; i++)
        {
            //check that the pivar is not allready appended to bulkRead
            if( PIVARArr[i].m_iIsPIVARRelatedToBulkFlag )
            {
                CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Init",m_uiConnHndl, 0, MMC_LIB_PI_BULK_THE_PIVAR_ALLREADY_APPEND, 0, 0);
            }

            //copy to the bulk read var array data
            m_ConfigBulkReadPIInParam.pVarsArray[i].usAxisRef = PIVARArr[i].m_usAxisRef;
            m_ConfigBulkReadPIInParam.pVarsArray[i].ucPiDirection = PIVARArr[i].m_ePIDirection;
            m_ConfigBulkReadPIInParam.pVarsArray[i].usIndex = PIVARArr[i].m_usPIVarOffset;

            //copy to the PIVAR the index and the referenc to the relevant bulk
            PIVARArr[i].m_pBulkRelated = this;
            PIVARArr[i].m_iIndexInDataBuffer = m_iIndexInDataBuffer;
            PIVARArr[i].m_iIsPIVARRelatedToBulkFlag = FLAG_ON_VAL;

            //increamnet the index in the input and output databuffer
            if ((PIVARArr[i].m_ucVarType == ePI_DOUBLE) || (PIVARArr[i].m_ucVarType == ePI_SIGNED_LONG_LONG) || (PIVARArr[i].m_ucVarType == ePI_UNSIGNED_LONG_LONG))
            {
                m_iIndexInDataBuffer = + INCREMENT_BY_2;
            }
            else
            {
                m_iIndexInDataBuffer++;
            }
            //
            m_iIndexInInputBuffer++;
        }
        //
        if(iSize != NC_MAX_PI_BULK_READ_VARIABLES)
        {
            //In order that config bulk read know until where are the signals need to put ePI_NONE in the index after the signals
            m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].ucPiDirection = ePI_NONE;

            //m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].usAxisRef = 0;
        }
    }
    //
    //else the PIVAR Array is empty
    else
    {
        //DO NOTING
    }
    //
}

    /*! \fn Append(CMMCCPIVar &PIVar)
    *   \brief append PI var to use for config bulk read.
    *   \param PIVar - Intance of the pivar that will be added to the config bulk read
    *   \
    *   \
    *   \return void
    */

void CMMCPIBulk::Append(CMMCPIVar &PIVar) throw (CMMCException)
{
    if(! (m_iIndexInInputBuffer <= NC_MAX_PI_BULK_READ_VARIABLES) )
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Append",m_uiConnHndl, 0, MMC_LIB_PI_BULK_CANNOT_APPEND_ARRAY_FULL, 0, 0);
    }
    //check that the pivar is not allready appended to bulkRead
    if( PIVar.m_iIsPIVARRelatedToBulkFlag )
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Append",m_uiConnHndl, 0, MMC_LIB_PI_BULK_THE_PIVAR_ALLREADY_APPEND, 0, 0);
    }

    PIVar.m_iIsPIVARRelatedToBulkFlag = FLAG_ON_VAL;
    PIVar.m_iIndexInDataBuffer = m_iIndexInDataBuffer;
    PIVar.m_pBulkRelated = this;

    m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].usAxisRef = PIVar.m_usAxisRef;
    m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].ucPiDirection = PIVar.m_ePIDirection;
    m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].usIndex = PIVar.m_usPIVarOffset;
    m_iIndexInInputBuffer++;

    if ((PIVar.m_ucVarType == ePI_DOUBLE) || (PIVar.m_ucVarType == ePI_SIGNED_LONG_LONG) || (PIVar.m_ucVarType == ePI_UNSIGNED_LONG_LONG))
    {
        m_iIndexInDataBuffer = + INCREMENT_BY_2;
    }
    else
    {
        m_iIndexInDataBuffer++;
    }
    //
    //in case we are doing append to array that is in the size of the NC_MAX_BULK_READ_PI_SIGNALS
    if( m_iIndexInInputBuffer < NC_MAX_PI_BULK_READ_VARIABLES)
    {
        //In order that config bulk read know until where are the signals need to put ePI_NONE in the index after the signals
        m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].ucPiDirection = ePI_NONE;

        //m_ConfigBulkReadPIInParam.pVarsArray[m_iIndexInInputBuffer].usAxisRef = 0;
    }
    //
}

    /*! \fn Reset()
    *   \Reset the instance of the PIBULKREAD
    */
void CMMCPIBulk::Clear()throw (CMMCException)
{
    m_uiConnHndl = 0;
    m_eConfiguration = eBULKREAD_CONFIG_PI_MAX;
    m_iIndexInDataBuffer = 0;
    m_iIndexInInputBuffer = 0;
    m_iIsBulkReadInitializedFlag = 0;
}

    /*! \fn Config()
    *   \brief execute config bulk read
    *   \return void
    */
void CMMCPIBulk::Config()throw (CMMCException)
{
    MMC_CONFIGBULKREADPI_OUT pOutParam;
    ELMO_INT32                rc;

    m_ConfigBulkReadPIInParam.eConfiguration = m_eConfiguration;

    if ((rc = MMC_ConfigBulkReadCmdPI(m_uiConnHndl,&m_ConfigBulkReadPIInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Config():", m_uiConnHndl, 0, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn Upload()
    *   \brief preform read bulk
    *   \return void
    */
void CMMCPIBulk::Upload()throw (CMMCException)
{
    MMC_PERFORMBULKREADPI_IN pInParam;
    ELMO_INT32                rc;

    pInParam.eConfiguration = m_eConfiguration;

    if ((rc = MMC_PerformBulkReadCmdPI(m_uiConnHndl,&pInParam, &m_pOut_performBulkread)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPIBulk::Upload():", m_uiConnHndl, 0, rc, m_pOut_performBulkread.usErrorID, m_pOut_performBulkread.usStatus);
    }
}

    /*! \fn Copy()
    *   \Perform copy of the requested data from m_pOut_performBulkread according to the index
    */
void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_BOOL &bValue)
{
    bValue = (ELMO_BOOL) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_INT8& cValue)
{
    cValue = (ELMO_INT8) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_UINT8& ucValue)
{
    ucValue = (ELMO_UINT8) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_INT16 &sValue)
{
    sValue = (ELMO_INT16) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_UINT16& usValue)
{
    usValue = (ELMO_UINT16) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_INT32& iValue)
{
    iValue = (ELMO_INT32) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_UINT32& uiValue)
{
    uiValue = (ELMO_UINT32) m_pOut_performBulkread.ulOutBuf[index];
}

void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_FLOAT& fValue)
{
    fValue = (ELMO_FLOAT) m_pOut_performBulkread.ulOutBuf[index];
}

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_INT64& llValue)
    {
        llValue = ( ( (ELMO_INT64) m_pOut_performBulkread.ulOutBuf[index] << SHIFT_32_BIT ) | ( m_pOut_performBulkread.ulOutBuf[index + NEXT_CELL_OFFSET] ) );
    }
    
#elif ((OS_PLATFORM == LINUXIPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC64_PLATFORM))

    void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_INT64& llValue)
    {
        llValue = *(ELMO_PINT64)(m_pOut_performBulkread.ulOutBuf + index);
    }
    

// #elif    (OS_PLATFORM == VXWORKS32_PLATFORM)
//  // Not implemented yet...
#else
    #error "***MMCPIBulk.cpp Symbol 'OS_PLATFORM' (1) is out of range... (See OS_PlatformSelect.h) "
#endif


void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_UINT64& ullValue)
{
    ullValue = ( ( (ELMO_UINT64) m_pOut_performBulkread.ulOutBuf[index] << SHIFT_32_BIT ) | ( m_pOut_performBulkread.ulOutBuf[index + NEXT_CELL_OFFSET] ) );
}


void CMMCPIBulk::Copy(ELMO_INT32 index, ELMO_DOUBLE& dValue)
{
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
	ELMO_PDOUBLE	pdAux;
    //dValue = (double) ( ( (unsigned long long) m_pOut_performBulkread.ulOutBuf[index] << SHIFT_32_BIT ) | ( m_pOut_performBulkread.ulOutBuf[index + NEXT_CELL_OFFSET] ) );
    ELMO_INT64 llVal1,llVal2,llResult;
    llVal1 = (0xFFFFFFFF00000000ULL  & (ELMO_UINT64)m_pOut_performBulkread.ulOutBuf[index] << SHIFT_32_BIT);
    llVal2 = (0x00000000FFFFFFFFULL & ((ELMO_UINT64)m_pOut_performBulkread.ulOutBuf[index + NEXT_CELL_OFFSET]));
    llResult = llVal1 | llVal2;

	pdAux = (ELMO_PDOUBLE)&llResult;
    dValue = *pdAux;

#elif ((OS_PLATFORM == LINUXIPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC64_PLATFORM))

    dValue = (*(ELMO_PDOUBLE)(m_pOut_performBulkread.ulOutBuf + index));

// #elif    (OS_PLATFORM == VXWORKS32_PLATFORM)
//  // Not implemented yet...
#else
    #error "***MMCPIBulk.cpp Symbol 'OS_PLATFORM' (2) is out of range... (See OS_PlatformSelect.h) "
#endif
}

