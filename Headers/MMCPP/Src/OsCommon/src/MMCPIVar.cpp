/*
 * MMCPIVar.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: ZivB
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCPIVar.hpp"

    /*! \fn void CMMCCPIVar
    *   \brief This function is constructor
    *   \param uHandle - Connection handle
    *   \param hAxisRef - Axis reference
    *   \param usPIVarOffset - PI Variable offset
    *   \param eDirection - PI Variable direction (IN/OUT)
    *   \return void
    */
CMMCPIVar::CMMCPIVar(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, ELMO_UINT16 usPIVarOffset, PI_DIRECTIONS eDirection) throw (CMMCException)
{
    Init(uHandle, hAxisRef, usPIVarOffset, eDirection);
}

    /*! \fn void CMMCCPIVar
    *   \brief This function is constructor
    *   \param uHandle - Connection handle
    *   \param hAxisRef - Axis reference
    *   \param alias - alias for the PI Variable
    *   \return void
    */
CMMCPIVar::CMMCPIVar(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, ELMO_INT8 alias[PI_ALIASING_LENGTH]) throw (CMMCException)
{
    Init(uHandle, hAxisRef, alias);
}

    /*! \fn destructor
     */
CMMCPIVar::~CMMCPIVar(){
}

    /*! \fn Init(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, unsigned short usPIVarOffset, PI_DIRECTIONS eDirection)
    *   \brief set inital value for class variable according to offset.
    *   \param uHandle - Connection handle
    *   \param hAxisRef - Axis reference
    *   \param usPIVarOffset - PI Variable offset
    *   \param eDirection - PI Variable direction (IN/OUT)
    */
void CMMCPIVar::Init(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, ELMO_UINT16 usPIVarOffset, PI_DIRECTIONS eDirection ) throw (CMMCException)
{
    MMC_GETPIVARINFO_IN  pInParam;
    MMC_GETPIVARINFO_OUT pOutParam;
    ELMO_INT32            rc;
    //NEW 11.3
    m_pBulkRelated = NULL;
    m_iIsPIVARRelatedToBulkFlag = 0;

    m_uiConnHndl = uHandle;
    m_usAxisRef = hAxisRef;
    m_usPIVarOffset = usPIVarOffset;
    m_ePIDirection = eDirection;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usPIVarIndex = m_usPIVarOffset;

    if ((rc = MMC_GetPIVarInfo(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCCPIVar:", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }

    m_ucVarType = pOutParam.VarInfo.ucVarType;
    m_uiBitOffset = pOutParam.VarInfo.uiBitOffset;
    m_uiBitSize = pOutParam.VarInfo.uiBitSize;

    //calculate the length of the data in bit.
    m_ucBitStart = PI_MOD8(m_uiBitOffset);
    m_ucByteLength = PI_BYTE_NUM_WITH_OFF(m_uiBitSize, m_ucBitStart);
}

    /*! \fn Init(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, char alias[PI_ALIASING_LENGTH])
    *   \brief set inital value for class variable according to alias.
    *   \param uHandle - Connection handle
    *   \param hAxisRef - Axis reference
    *   \param alias - alias for the PI Variable
    *   \return void
    */
void CMMCPIVar::Init(MMC_CONNECT_HNDL uHandle, MMC_AXIS_REF_HNDL hAxisRef, ELMO_INT8 alias[PI_ALIASING_LENGTH]) throw (CMMCException)
{
    MMC_GETPIVARINFOBYALIAS_IN  pInParam;
    MMC_GETPIVARINFOBYALIAS_OUT pOutParam;
    ELMO_INT32                   rc;
    //NEW 11.3
    m_pBulkRelated = NULL;
    m_iIsPIVARRelatedToBulkFlag = 0;
    //

    m_uiConnHndl = uHandle;
    m_usAxisRef = hAxisRef;

    if( strlen(alias) > PI_ALIASING_LENGTH )
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCCPIVar::Init",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_ALIAS_SIZE_GREATER_THEN_PI_ALIASING_LENGTH, 0, 0);
    }

    strcpy(pInParam.pAliasing,alias);

    if ((rc = MMC_GetPIVarInfoByAlias(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCCPIVar::Init", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }

    m_ucVarType = pOutParam.VarInfo.ucVarType;
    m_usPIVarOffset = pOutParam.VarInfo.usPIVarOffset;
    m_ePIDirection = (PI_DIRECTIONS)pOutParam.VarInfo.ucDirection;
    m_uiBitOffset = pOutParam.VarInfo.uiBitOffset;
    m_uiBitSize = pOutParam.VarInfo.uiBitSize;

    //calculate the length of the data in bit.
    m_ucBitStart = PI_MOD8(m_uiBitOffset);
    m_ucByteLength = PI_BYTE_NUM_WITH_OFF(m_uiBitSize, m_ucBitStart);
}

    /*! \fn UnbindFromBulkRead()
    *   \brief remove the option of reading from bulkRead.
    */
void CMMCPIVar::UnbindFromBulkRead()
{
    m_pBulkRelated = NULL;
    m_iIsPIVARRelatedToBulkFlag = 0;
}

    /*! \fn UnbindFromBulkRead()
    *   \brief remove the option of reading from bulkRead.
    */
ELMO_INT32 CMMCPIVar::IsBoundToBulkRead()
{
    return  m_iIsPIVARRelatedToBulkFlag;
}

    /*! \fn void WritePI(bool bValue)
    *   \brief This function write to PI VAR of type char
    *   \param bValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_BOOL bValue) throw (CMMCException)
{
    MMC_WRITEPIVARBOOL_IN   pInParam;
    MMC_WRITEPIVARBOOL_OUT  pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_BOOL bValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_BOOL)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_BOOL bValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.ucBOOL = bValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarBool(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_BOOL bValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(char cValue)
    *   \brief This function write to PI VAR of type char
    *   \param cValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_INT8 cValue) throw (CMMCException)
{
    MMC_WRITEPIVARCHAR_IN   pInParam;
    MMC_WRITEPIVARCHAR_OUT  pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT8 cValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_SIGNED_CHAR)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT8 cValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }

    pInParam.cData = cValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarChar(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT8 cValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(char ucValue)
    *   \brief This function write to PI VAR of type char
    *   \param ucValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_UINT8 ucValue) throw (CMMCException)
{
    MMC_WRITEPIVARUCHAR_IN  pInParam;
    MMC_WRITEPIVARUCHAR_OUT pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT8 ucValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_UNSIGNED_CHAR)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT8 ucValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.ucData = ucValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarUChar(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT8 ucValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(unsigned short usValue)
    *   \brief This function write to PI VAR of type char
    *   \param usValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_UINT16 usValue) throw (CMMCException)
{
    MMC_WRITEPIVARUSHORT_IN  pInParam;
    MMC_WRITEPIVARUSHORT_OUT pOutParam;
    ELMO_INT32                rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT16 usValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_UNSIGNED_SHORT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT16 usValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.usData = usValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarUShort(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT16 usValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(short sValue)
    *   \brief This function write to PI VAR of type char
    *   \param sValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(short sValue) throw (CMMCException)
{
    MMC_WRITEPIVARSHORT_IN  pInParam;
    MMC_WRITEPIVARSHORT_OUT pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT16 sValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_SIGNED_SHORT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT16 sValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.sData = sValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarShort(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT16 sValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(unsigned int uiValue)
    *   \brief This function write to PI VAR of type char
    *   \param uiValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_UINT32 uiValue) throw (CMMCException)
{
    MMC_WRITEPIVARUINT_IN   pInParam;
    MMC_WRITEPIVARUINT_OUT  pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT32 uiValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_UNSIGNED_INT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT32 uiValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }

    pInParam.uiData = uiValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarUInt(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT32 uiValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(int iValue)
    *   \brief This function write to PI VAR of type char
    *   \param iValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_INT32 iValue) throw (CMMCException)
{
    MMC_WRITEPIVARINT_IN    pInParam;
    MMC_WRITEPIVARINT_OUT   pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT32 iValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_SIGNED_INT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT32 iValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.iData = iValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarInt(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT32 iValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(float fValue)
    *   \brief This function write to PI VAR of type char
    *   \param fValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_FLOAT fValue) throw (CMMCException)
{
    MMC_WRITEPIVARFLOAT_IN  pInParam;
    MMC_WRITEPIVARFLOAT_OUT pOutParam;
    ELMO_INT32               rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_FLOAT fValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_FLOAT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_FLOAT fValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.fData = fValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarFloat(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_FLOAT fValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(unsigned char *ucValue,int iSize)
    *   \brief This function write to PI VAR of type RAW PI_REG_VAR_SIZE / PI_LARGE_VAR_SIZE
    *   \param ucValue - value that will be written
    *   \param iSize - value that can receive 4 or 16 which are PI_REG_VAR SIZE
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize) throw (CMMCException)
{
    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if ((m_ucVarType != ePI_BITWISE) &&  (m_ucVarType != ePI_8MULTIPLE))
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if (iSize == PI_REG_VAR_SIZE)
    {
        MMC_WRITEPIVARRAW_IN    pInParam;
        MMC_WRITEPIVARRAW_OUT   pOutParam;
        ELMO_INT32               rc;

        memcpy(pInParam.pRawData,ucValue,m_ucByteLength);
        pInParam.ucByteLength = m_ucByteLength;
        pInParam.usIndex = m_usPIVarOffset;

        if ((rc = MMC_WritePIVarRaw(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT8 ucValue[PI_REG_VAR_SIZE]):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
    }
    //
    else if (iSize == PI_LARGE_VAR_SIZE)
    {
        MMC_WRITELARGEPIVARRAW_IN   pInParam;
        MMC_WRITELARGEPIVARRAW_OUT  pOutParam;
        ELMO_INT32                   rc;

        if ((m_ucVarType != ePI_BITWISE) &&  (m_ucVarType != ePI_8MULTIPLE))
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize) ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
        }

        memcpy(pInParam.pRawData,ucValue,m_ucByteLength);
        pInParam.usByteLength = m_ucByteLength;
        pInParam.usIndex = m_usPIVarOffset;

        if ((rc = MMC_WriteLargePIVarRaw(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
    }
    //
    //WRONG SIZE
    else
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI input size shoud be 4 for PI_REG_VAR_SIZE or 16 for PI_LARGE_VAR_SIZE : ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
}

    /*! \fn void WritePI(unsigned long long ullValue)
    *   \brief This function write to PI VAR of type char
    *   \param ullValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_UINT64 ullValue) throw (CMMCException)
{
    MMC_WRITEPIVARULONGLONG_IN  pInParam;
    MMC_WRITEPIVARULONGLONG_OUT pOutParam;
    ELMO_INT32                   rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT64 ullValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_UNSIGNED_LONG_LONG)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT64 ullValue: ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.ullData = ullValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarULongLong(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_UINT64 ullValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(long long llValue)
    *   \brief This function write to PI VAR of type char
    *   \param llValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_INT64 llValue) throw (CMMCException)
{
    MMC_WRITEPIVARLONGLONG_IN   pInParam;
    MMC_WRITEPIVARLONGLONG_OUT  pOutParam;
    ELMO_INT32                   rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT64 llValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_SIGNED_LONG_LONG)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT64 llValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    pInParam.llData = llValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarLongLong(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_INT64 llValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}

    /*! \fn void WritePI(double dValue)
    *   \brief This function write to PI VAR of type char
    *   \param dValue - value that will be written
    *   \return void
    */
void CMMCPIVar::WritePI(ELMO_DOUBLE dValue) throw (CMMCException)
{
    MMC_WRITEPIVARDOUBLE_IN     pInParam;
    MMC_WRITEPIVARDOUBLE_OUT    pOutParam;
    ELMO_INT32                   rc;

    if (m_ePIDirection != ePI_OUTPUT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_DOUBLE dValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_WRITE_PI_TO_ePI_INPUT_FORBIDDEN, 0, 0);
    }
    //
    if (m_ucVarType != ePI_DOUBLE)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_DOUBLE dValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }

    pInParam.dbVal = dValue;
    pInParam.usIndex = m_usPIVarOffset;

    if ((rc = MMC_WritePIVarDouble(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("WritePI(ELMO_DOUBLE dValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
}


    /*! \fn void ReadPI(bool &bValue)
    *   \brief This function read parameter of type bool
    *   \param bValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_BOOL &bValue) throw (CMMCException)
{
    MMC_READPIVARBOOL_IN    pInParam;
    MMC_READPIVARBOOL_OUT   pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_BOOL)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_BOOL &bValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_BOOL &bValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/
		ELMO_UINT8 eu8temp ;

		m_pBulkRelated->Copy(m_iIndexInDataBuffer,eu8temp);

  		ELMO_UINT8 ucMask = 0x01 << m_ucBitStart;
		bValue = ((eu8temp) & ucMask) >> m_ucBitStart;
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarBOOL(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_BOOL &bValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        bValue=pOutParam.ucBOOL;
    }
}

    /*! \fn void ReadPI(char &cValue)
    *    \brief This function read parameter of type char
    *   \param cValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_INT8 &cValue) throw (CMMCException)
{
    MMC_READPIVARCHAR_IN    pInParam;
    MMC_READPIVARCHAR_OUT   pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_SIGNED_CHAR)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT8 &cValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT8 &cValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,cValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarChar(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT8 &cValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        cValue = pOutParam.cData;
    }
}

    /*! \fn void ReadPI(unsigned char &ucValue)
    *   \brief This function read parameter of type unsigned char
    *   \param ucValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_UINT8 &ucValue) throw (CMMCException)
{
    ELMO_INT32               rc;
    MMC_READPIVARUCHAR_IN   pInParam;
    MMC_READPIVARUCHAR_OUT  pOutParam;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_UNSIGNED_CHAR)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT8 &ucValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT8 &ucValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,ucValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarUChar(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT8 &ucValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        ucValue = pOutParam.ucData;
    }
}

    /*! \fn void ReadPI(short &sValue)
    *   \brief This function read parameter of type short
    *   \param sValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(short &sValue) throw (CMMCException)
{
    MMC_READPIVARSHORT_IN   pInParam;
    MMC_READPIVARSHORT_OUT  pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_SIGNED_SHORT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(short &sValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(short &sValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,sValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarShort(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(short &sValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        sValue = pOutParam.sData;
    }
}

    /*! \fn void ReadPI(unsigned short &usValue)
    *   \brief This function read parameter of type unsigend short
    *   \param usValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_UINT16 &usValue) throw (CMMCException)
{
    MMC_READPIVARUSHORT_IN  pInParam;
    MMC_READPIVARUSHORT_OUT pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_UNSIGNED_SHORT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT16 &usValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT16 &usValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,usValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarUShort(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT16 &usValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        usValue = pOutParam.usData;
    }
}

    /*! \fn void ReadPI(int &iValue)
    *   \brief This function read parameter of type int
    *   \param iValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_INT32 &iValue) throw (CMMCException)
{
    MMC_READPIVARINT_IN     pInParam;
    MMC_READPIVARINT_OUT    pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_SIGNED_INT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT32 &iValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT32 &iValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,iValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarInt(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT32 &iValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
        iValue = pOutParam.iData;
    }
}

    /*! \fn void ReadPI(unsigned int &uiValue)
    *   \brief This function read parameter of type unsigned int
    *   \param uiValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_UINT32 &uiValue) throw (CMMCException)
{
    MMC_READPIVARUINT_IN    pInParam;
    MMC_READPIVARUINT_OUT   pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_UNSIGNED_INT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT32 &uiValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT32 &uiValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,uiValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarUInt(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT32 &uiValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        uiValue = pOutParam.uiData;
    }
}

    /*! \fn void ReadPI(float &fValue)
    *   \brief This function read parameter of type float
    *   \param fValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_FLOAT &fValue) throw (CMMCException)
{
    MMC_READPIVARFLOAT_IN   pInParam;
    MMC_READPIVARFLOAT_OUT  pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_FLOAT)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_FLOAT &fValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_FLOAT &fValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,fValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarFloat(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_FLOAT &fValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }

        fValue = pOutParam.fData;
    }
    //
}

    /*! \fn void ReadPI (unsigned char *ucValue,int iSize)
    *   \brief This function read to PI VAR of type RAW PI_REG_VAR_SIZE / PI_LARGE_VAR_SIZE
    *   \param ucValue - hold the value of the VAR PI that was read
    *   \param iSize - value that can receive 4 or 16 which are PI_REG_VAR SIZE
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize) throw (CMMCException)
{
	ELMO_PINT32 iValue;
    if ((m_ucVarType != ePI_BITWISE) &&  (m_ucVarType != ePI_8MULTIPLE))
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if (iSize == PI_REG_VAR_SIZE)
    {
        MMC_READPIVARRAW_IN     pInParam;
        MMC_READPIVARRAW_OUT    pOutParam;
        ELMO_INT32               rc;

        pInParam.ucDirection = m_ePIDirection;
        pInParam.usIndex = m_usPIVarOffset;
        pInParam.ucByteLength = m_ucByteLength;

        if ((rc = MMC_ReadPIVarRaw(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
        memcpy(ucValue,pOutParam.pRawData,m_ucByteLength);
		iValue = (ELMO_PINT32)ucValue;
		ELMO_UINT32 uiMask = 0x1;
		for (ELMO_UINT32 i = 0; i < m_uiBitSize; i++)
		{
			uiMask |= uiMask << 1;
		}
		uiMask = uiMask << m_ucBitStart;
		(*iValue) = ((*iValue) & uiMask) >> m_ucBitStart;

//		(*iValue) = (*iValue) >> m_ucBitStart;

	}
    //
    else if (iSize == PI_LARGE_VAR_SIZE)
    {
        MMC_READLARGEPIVARRAW_IN    pInParam;
        MMC_READLARGEPIVARRAW_OUT   pOutParam;
        ELMO_INT32                   rc;

        pInParam.ucDirection = m_ePIDirection;
        pInParam.usIndex = m_usPIVarOffset;
        pInParam.usByteLength = m_ucByteLength;

        if ((rc = MMC_ReadLargePIVarRaw(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
        memcpy(ucValue,pOutParam.pRawData,m_ucByteLength);
    }
    //
    else
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_PUINT8 ucValue,ELMO_INT32 iSize) : ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_SIZE_INVALID, 0, 0);
    }
}

    /*! \fn void ReadPI(long long &llValue)
    *   \brief This function read parameter of type long long
    *   \param llValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_INT64 &llValue) throw (CMMCException)
{
    MMC_READPIVARLONGLONG_IN    pInParam;
    MMC_READPIVARLONGLONG_OUT   pOutParam;
    ELMO_INT32                   rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_SIGNED_LONG_LONG)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT64 &llValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(long long &llValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,llValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarLongLong(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_INT64 &llValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
        llValue = pOutParam.llData;
    }
}

    /*! \fn void ReadPI(unsigned long long &ullValue)
    *   \brief This function read parameter of type unsigned long long
    *   \param ullValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_UINT64 &ullValue) throw (CMMCException)
{
    MMC_READPIVARULONGLONG_IN   pInParam;
    MMC_READPIVARULONGLONG_OUT  pOutParam;
    ELMO_INT32                   rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_UNSIGNED_LONG_LONG)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT64 &ullValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(unsigned long long &ullValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/
        m_pBulkRelated->Copy(m_iIndexInDataBuffer,ullValue);
    }
    //
    else
    {
        if ((rc = MMC_ReadPIVarULongLong(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_UINT64 &ullValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
        ullValue = pOutParam.ullData;
    }
    //
}

    /*! \fn void ReadPI(double &dValue)
    *   \brief This function read parameter of type double
    *   \param dValue - hold the value of the VAR PI that was read.
    *   \return void
    */
void CMMCPIVar::ReadPI(ELMO_DOUBLE& dValue) throw (CMMCException)
{
    MMC_READPIVARDOUBLE_IN  pInParam;
    MMC_READPIVARDOUBLE_OUT pOutParam;
    ELMO_INT32               rc;

    pInParam.ucDirection = m_ePIDirection;
    pInParam.usIndex = m_usPIVarOffset;

    if (m_ucVarType != ePI_DOUBLE)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_DOUBLE& dValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_TYPE_NOT_MATCH, 0, 0);
    }
    //
    // IF BULK READ WAS SET the pointer to the data is not null

    if(m_iIsPIVARRelatedToBulkFlag)
    {
        /*if(m_pBulkRelated->m_iIndexInInputBuffer == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_DOUBLE& dValue): ",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_READ_FROM_CLEAR_BULK, 0, 0);
        }*/

        m_pBulkRelated->Copy(m_iIndexInDataBuffer,dValue);
    }
    //
    //The pinter to the bulk read data is null so we will read using read pi
    else
    {
        if ((rc = MMC_ReadPIVarDouble(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ReadPI(ELMO_DOUBLE& dValue):", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
        }
        dValue = pOutParam.dbData;
    }
}

/*! \fn GetPIVarInfoByAlias()
*   \brief gets the PI info according to the PI alaia.
*   \param char alias : the name of the PI
*   \param  MMC_GETPIVARINFOBYALIAS_OUT &pOutParam - retrive the info data of the PI
*/

void CMMCPIVar::GetPIVarInfoByAlias(ELMO_INT8 alias[PI_ALIASING_LENGTH],MMC_GETPIVARINFOBYALIAS_OUT &pOutParam) throw (CMMCException)
{
    MMC_GETPIVARINFOBYALIAS_IN  pInParam;
    ELMO_INT32 rc = 0;

    if( strlen(alias) > PI_ALIASING_LENGTH )
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCCPIVar::GetPIVarInfoByAlias",m_uiConnHndl, m_usAxisRef, MMC_LIB_PI_VAR_ALIAS_SIZE_GREATER_THEN_PI_ALIASING_LENGTH, 0, 0);
    }

    strcpy(pInParam.pAliasing,alias);

    if ((rc = MMC_GetPIVarInfoByAlias(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCCPIVar::GetPIVarInfoByAlias", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }

    return;
}


/*! \fn GetPIVarsRangeInfo()
*   \brief gets the PI info of range of PI.
*   \param MMC_GETPIVARSRANGEINFO_IN  pInParam
*   \param  MMC_GETPIVARSRANGEINFO_OUT &pOutParam - retrive the info data of the PI
*/

void CMMCPIVar::GetPIVarsRangeInfo(MMC_GETPIVARSRANGEINFO_IN  pInParam,MMC_GETPIVARSRANGEINFO_OUT &pOutParam) throw (CMMCException)
{
    ELMO_INT32 rc = 0;

    if ((rc = MMC_GetPIVarsRangeInfo(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCCPIVar::GetPIVarsRangeInfo", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }

    return;
}

