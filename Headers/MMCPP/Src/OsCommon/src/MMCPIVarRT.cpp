/*
 * MMCPIVarRT.cpp
 *
 *  Created on: 18/04/2017
 *      Author: zivb
 */
 
 
//#ifndef WIN32
#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)

#include "OS_PlatformDependSetting.hpp"
#include "MMCPIVarRT.hpp"

extern "C"
{
    MMC_LIB_API ELMO_PINT8 MMC_MapShmPtr(MMC_CONNECT_HNDL hConn);
    MMC_LIB_API ELMO_INT32 MMC_UnmapShmPtr(ELMO_PINT8 pShmPtr);
}


#define PAGE_SIZE getpagesize()
#define SIZE_PAGE_ALIGN(x) ((x&0xFFF)?((x+PAGE_SIZE)&(~0xFFF)):(x))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMMCPIVarRT::CMMCPIVarRT() : CMMCPIVar()
{
    m_ulPIMemOffset     = 0;
    m_uiNumberOfPIIn    = 0;
    m_uiNumberOfPIOut   = 0;
    m_uiIsMandatory     = 0;

    m_pShmPtr = NULL;
    m_pShmPITable = NULL;
    m_ulbasePtr = 0;

}

CMMCPIVarRT::CMMCPIVarRT(CMMCPIVar& piVar):CMMCPIVar(piVar)
{
    m_ulPIMemOffset     = 0;
    m_uiNumberOfPIIn    = 0;
    m_uiNumberOfPIOut   = 0;
    m_uiIsMandatory     = 0;
}

CMMCPIVarRT::~CMMCPIVarRT()
{
    if (m_uiIsMandatory)
    {
        MMC_UnmapShmPtr((ELMO_PINT8)m_pShmPtr);
    }
    else
    {
        munmap((ELMO_PINT8)m_ulbasePtr, m_uiPIMemSize);
    }
}

void CMMCPIVarRT::Init(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, ELMO_UINT16 usPIVarOffset, PI_DIRECTIONS eDirection ) throw (CMMCException)
{
    ELMO_INT32  iFd = -1;

    m_uiConnHndl    = uHandle;
    m_usAxisRef     = hAxisRef;
    m_usPIVarOffset = usPIVarOffset;
    m_ePIDirection  = eDirection;

    MMC_GETNODEPIMEMOFFSET_IN pInParamPIMemOffset;
    MMC_GETNODEPIMEMOFFSET_OUT pOutParamPIMemOffset;

    pInParamPIMemOffset.uiPIDirection = m_ePIDirection;
    pInParamPIMemOffset.usPIVarOffset = usPIVarOffset;
    pInParamPIMemOffset.usRefAxis = m_usAxisRef;

    MMC_GePIMemOffset(m_uiConnHndl,&pInParamPIMemOffset,&pOutParamPIMemOffset);

    m_ulPIMemOffset = pOutParamPIMemOffset.ulNodePIMemOffset;
    m_uiIsMandatory = pOutParamPIMemOffset.uiIsMandatory;
    m_uiPIMemSize = pOutParamPIMemOffset.uiPIMemSize;

    if(m_uiIsMandatory)
    {
        m_pShmPtr =  (ELMO_PUINT8)MMC_MapShmPtr(m_uiConnHndl);
        if (NULL == m_pShmPtr)
        {
            printf("Failed to map shared memory\n");
            CMMCPPGlobal::Instance()->MMCPPThrow("InitAxisData RT Failed to map shared memory:", m_uiConnHndl, m_usAxisRef, 0, 0,0);
        }
        MMC_GETSYSTEMDATA_OUT pOutParam;
        MMC_GetSystemData(m_uiConnHndl,&pOutParam);

        m_ulbasePtr =(ELMO_ULINT32) (m_pShmPtr + pOutParam.uiNodeDataOffset);
    }
    else
    {
        iFd = open("/dev/nc0", /*O_RDONLY*/O_RDWR | O_SYNC);
        m_ulbasePtr = (ELMO_ULINT32)mmap(0, m_uiPIMemSize, PROT_READ | PROT_WRITE, MAP_SHARED /*| MAP_LOCKED*/  , iFd, 0);
    }

    MMC_GETPINUMBER_IN  pInParamPINum;
    MMC_GETPINUMBER_OUT pOutParamPINum;

    MMC_GetPINumber(m_uiConnHndl,m_usAxisRef,&pInParamPINum,&pOutParamPINum);

    m_uiNumberOfPIIn = pOutParamPINum.uiNumberOfPIInput;
    m_uiNumberOfPIOut = pOutParamPINum.uiNumberOfPIOutPut;
}



/*! \fn void ReadPI(bool &bValue)
*   \brief This function read parameter of type bool
*   \param bValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_BOOL &bValue) throw (CMMCException)
{
    bValue = BOOLPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(char &cValue)
*    \brief This function read parameter of type char
*   \param cValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_SINT8 &cValue) throw (CMMCException)
{
    cValue = CHARPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(unsigned char &ucValue)
*   \brief This function read parameter of type unsigned char
*   \param ucValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_UINT8 &ucValue) throw (CMMCException)
{
    ucValue = UCHARPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(short &sValue)
*   \brief This function read parameter of type short
*   \param sValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(short &sValue) throw (CMMCException)
{
    sValue = SHORTPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(unsigned short &usValue)
*   \brief This function read parameter of type unsigend short
*   \param usValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_UINT16 &usValue) throw (CMMCException)
{
    usValue = USHORTPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(int &iValue)
*   \brief This function read parameter of type int
*   \param iValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_INT32 &iValue) throw (CMMCException)
{
    iValue = INTPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}


/*! \fn void ReadPI(unsigned int &uiValue)
*   \brief This function read parameter of type unsigned int
*   \param uiValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_UINT32 &uiValue) throw (CMMCException)
{
    uiValue = UINTPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(float &fValue)
*   \brief This function read parameter of type float
*   \param fValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_FLOAT &fValue) throw (CMMCException)
{
    fValue = FLOATPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}

/*! \fn void ReadPI(double &dValue)
*   \brief This function read parameter of type double
*   \param dValue - hold the value of the VAR PI that was read.
*   \return void
*/
void CMMCPIVarRT::ReadPI(ELMO_DOUBLE &dValue) throw (CMMCException)
{
    dValue = DOUBLEPARAMETER(m_ulbasePtr,m_ulPIMemOffset);
}


/*! \fn void WritePI(bool bValue)
*   \brief This function write to PI VAR of type char
*   \param bValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_BOOL bValue) throw (CMMCException)
{
    BOOLPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = bValue;
}

/*! \fn void WritePI(char cValue)
*   \brief This function write to PI VAR of type char
*   \param cValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_INT8 cValue) throw (CMMCException)
{
    CHARPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = cValue;
}

/*! \fn void WritePI(char ucValue)
*   \brief This function write to PI VAR of type char
*   \param ucValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_UINT8 ucValue) throw (CMMCException)
{
    UCHARPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = ucValue;
}

/*! \fn void WritePI(unsigned short usValue)
*   \brief This function write to PI VAR of type char
*   \param usValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_UINT16 usValue) throw (CMMCException)
{
    USHORTPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = usValue;
}

/*! \fn void WritePI(short sValue)
*   \brief This function write to PI VAR of type char
*   \param sValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_INT16 sValue) throw (CMMCException)
{
    SHORTPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = sValue;
}

/*! \fn void WritePI(unsigned int uiValue)
*   \brief This function write to PI VAR of type char
*   \param uiValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_UINT32 uiValue) throw (CMMCException)
{
    UINTPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = uiValue;
}

/*! \fn void WritePI(int iValue)
*   \brief This function write to PI VAR of type char
*   \param iValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_INT32 iValue) throw (CMMCException)
{
    INTPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = iValue;
}

/*! \fn void WritePI(float fValue)
*   \brief This function write to PI VAR of type char
*   \param fValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_FLOAT fValue) throw (CMMCException)
{
    FLOATPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = fValue;
}

/*! \fn void WritePI(double dValue)
*   \brief This function write to PI VAR of type char
*   \param dValue - value that will be written
*   \return void
*/
void CMMCPIVarRT::WritePI(ELMO_DOUBLE dValue) throw (CMMCException)
{
    DOUBLEPARAMETER(m_ulbasePtr,m_ulPIMemOffset) = dValue;
}

#endif /* #if   (OS_PLATFORM == LINUXIPC32_PLATFORM) */

