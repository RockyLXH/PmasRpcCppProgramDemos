/*
 * MMCEoE.cpp
 *
 *  Created on: 06/01/2013
 *      Author: yuvall
 *      Update: 20Jan2014 HaimH add exceptions.
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCEoE.hpp"

/*
 * default constructor
 */
CMMCEoE::CMMCEoE()
{
    //set defaults
    this->SetMaxSize(EoE_BUFFER_SIZE);
    this->InitSemSyncDrvIo();
}

/*! \fn GetAck(char* cBuffer, int iSize)
* \brief read EoE response from device and search for ';' suffix as for acknowledge or  '?' for error
* \param cBuffer address of buffer to store response.
* \param iSize expected response message length.
* \return   return number of read characters, -1 if error.
*/
ELMO_INT32 CMMCEoE::GetAck(ELMO_PINT8 szBuffer, ELMO_INT32 iSize) throw (CMMCException)
{
    if (iSize <= 1)
        CMMCPPGlobal::Instance()->MMCPPThrow("'GetAck' Illegal iSize ", 0, 0, -1, -1, 0);
    ELMO_INT32 iLength = (iSize > EoE_BUFFER_SIZE)? EoE_BUFFER_SIZE : iSize;
    ELMO_UINT8 ucRetries = 0;
//  ELMO_INT32 iRet = this->ReceiveFrom(szBuffer, iLength, 0);
    ELMO_INT32 iRet = this->Receive(szBuffer, iLength, 50); //wait 1 ms
    while (iRet  > 1)
    { //only ';' means nothing
        if (szBuffer[iRet-1]== ';')
        {
            szBuffer[iRet-1] = '\0';
            if ( szBuffer[iRet-2] == '?')
                iRet = MMC_LIB_EoE_DRV_ERR;
            break;
        }
        if (++ucRetries > MMCPP_SOCK_EOE_RETRIES)
        {
            break;
        }
        iRet += this->Receive(&szBuffer[iRet], iLength, 50);
    }
    return iRet;
}

/*! \fn ElmoReadData(char* cBuffer, int iSize)
* \brief read EoE response from driver and search for ';' suffix
*        as for asynchronous response.
* \param cBuffer address of buffer to store response.
* \param iSize expected response message length.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoReadData(ELMO_PINT8 szBuffer, ELMO_INT32 iSize) throw (CMMCException)
{
    if (iSize <= 1)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoReadData' Illegal iSize ", 0, 0, -1, -1, 0);
    ELMO_INT32 iLength = (iSize > EoE_BUFFER_SIZE)? EoE_BUFFER_SIZE : iSize;
    ELMO_UINT8 ucRetries = 0;
    this->TakeSemSyncDrvIo();
//  ELMO_INT32 iRet = this->ReceiveFrom(_szBuffer, EoE_BUFFER_SIZE, 0);//no wait
    ELMO_INT32 iRet = this->Receive(_szBuffer, iLength, 0);
    while (iRet > 1) {
        if (_szBuffer[iRet-1]== ';') {
            if ( _szBuffer[iRet-2] == '?')
                iRet = MMC_LIB_EoE_DRV_ERR;
            else {
                _szBuffer[iRet -1] = '\0';
                iRet = ParseResult(_szBuffer, szBuffer, iLength);
            }
            break;
        }
        if (++ucRetries > MMCPP_SOCK_EOE_RETRIES) {
            iRet = MMC_LIB_EoE_RETRIES_ERR;
            break;
        }
        iRet += this->Receive(&_szBuffer[iRet], iLength, 0);//no wait
    }
    this->ReleaseSemSyncDrvIo();
    return(iRet>0)?0:iRet;
}

/*! \fn ElmoAck(char* szBuffer, int iSize)
* \brief checks wether or not a previous EoE command is acknowlaged.
* \param szBuffer string buffer to store the acknowledgement.
* \param iSize buffer size - number of bytes to store on success.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoAck(ELMO_PINT8 szBuffer, ELMO_INT32 iSize) throw (CMMCException)
{
    if (iSize <= 1)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoAck' Illegal iSize ", 0, 0, -1, -1, 0);
    ELMO_INT32 iLength = (iSize > EoE_BUFFER_SIZE)? EoE_BUFFER_SIZE : iSize;
    ELMO_UINT8 ucRetries = 0;
    this->TakeSemSyncDrvIo();
//  ELMO_INT32 iRet = this->ReceiveFrom(_szBuffer, EoE_BUFFER_SIZE, 0);//no wait
    ELMO_INT32 iRet = this->Receive(szBuffer, iLength, 0);
    while (iRet > 1) {
        if (szBuffer[iRet-1]== ';') {
            if ( szBuffer[iRet-2] == '?')
                iRet = MMC_LIB_EoE_DRV_ERR;
            break;
        }
        if (++ucRetries > MMCPP_SOCK_EOE_RETRIES)
        {
            iRet = MMC_LIB_EoE_RETRIES_ERR;
            break;
        }
        iRet += this->Receive(&szBuffer[iRet], iLength, 0);//no wait
    }
    this->ReleaseSemSyncDrvIo();
    return(iRet>0)?0:iRet;
}


/**! \fn ElmoReadData(int& iValue)
 * \brief read EoE data and set iValue accordingly.
 * \param iValue set with returned value if completed successfully.
 * \return 0 on success otherwise error (<0).
 *  */
ELMO_INT32 CMMCEoE::ElmoReadData(ELMO_INT32& iValue) {
    ELMO_UINT8 ucRetries = 0;
    this->TakeSemSyncDrvIo();
//  ELMO_INT32 iRet = this->ReceiveFrom(_szBuffer, EoE_BUFFER_SIZE, 0);//no wait
    ELMO_INT32 iRet = this->Receive(_szBuffer, EoE_BUFFER_SIZE, 0); //no wait
    while (iRet > 1) {
        if (_szBuffer[iRet-1]== ';') {
            if ( _szBuffer[iRet-2] == '?')
                        iRet = MMC_LIB_EoE_DRV_ERR;
            else {
                _szBuffer[iRet -1] = '\0';
                iRet = ParseResult(_szBuffer, iValue);
            }
            break;
        }
        if (++ucRetries > MMCPP_SOCK_EOE_RETRIES) {
            iRet = MMC_LIB_EoE_RETRIES_ERR;
            break;
        }
        iRet += this->Receive(&_szBuffer[iRet], EoE_BUFFER_SIZE-iRet, 0); //no wait
    }
    this->ReleaseSemSyncDrvIo();
    return(iRet>0)?0:iRet;
}

/**! \fn ElmoReadData(float& fValue)
 * \brief read EoE data and set fValue accordingly.
 * \param fValue set with returned value if completed successfully.
 * \return 0 on success otherwise error (<0).
 */
ELMO_INT32 CMMCEoE::ElmoReadData(ELMO_FLOAT& fValue) {
    ELMO_UINT8 ucRetries = 0;
    this->TakeSemSyncDrvIo();
//  ELMO_INT32 iRet = this->ReceiveFrom(_szBuffer, EoE_BUFFER_SIZE, 0);//no wait
    ELMO_INT32 iRet = this->Receive(_szBuffer, EoE_BUFFER_SIZE, 0);//no wait
    while (iRet > 1) {
        if (_szBuffer[iRet-1]== ';') {
            if ( _szBuffer[iRet-2] == '?')
                        iRet = MMC_LIB_EoE_DRV_ERR;
            else {
                _szBuffer[iRet -1] = '\0';
                iRet = ParseResult(_szBuffer, fValue);
            }
            break;
        }
        if (++ucRetries > MMCPP_SOCK_EOE_RETRIES) {
            iRet = MMC_LIB_EoE_RETRIES_ERR;
            break;
        }
        iRet += this->Receive(&_szBuffer[iRet], EoE_BUFFER_SIZE-iRet, 0); //no wait
    }
    this->ReleaseSemSyncDrvIo();
    return(iRet>0)?0:iRet;
}


/**! \fn ElmoSetAsyncArray(char szCmd[3], short iIndex, const int iVal)
 * \brief send EoE array command asynchronously.
 * \param szCmd the command
 * \param iIndex EoE array index
 * \param iValue integer value to set.
 * \return 0 on success otherwise error (<0).
 */
ELMO_INT32 CMMCEoE::ElmoSetAsyncArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex, const ELMO_INT32 iVal) throw (CMMCException)
{
   	if (iIndex < 0)
	{
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncArray' (ELMO_INT32) Illegal iIndex ", 0, 0, -1, -1, 0);
	}

    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]=%d\r", szCmd, iIndex, iVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]=%d\r", szCmd, iIndex, iVal);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if (rc < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncArray' failed in SendTo", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETASYNC_ERR ;
}
/**! \fn ElmoSetAsyncArray(char szCmd[3], short iIndex, const float fVal)
 * \brief send EoE array command asynchronously.
 * \param szCmd the command
 * \param iIndex EoE array index
 * \param fVal real value to set.
 * \return 0 on success otherwise error (<0).
  */
ELMO_INT32 CMMCEoE::ElmoSetAsyncArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex, const ELMO_FLOAT fVal) throw (CMMCException)
{
    if (iIndex < 0 || iIndex > 2)
	{
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncArray' (ELMO_FLOAT) Illegal iIndex ", 0, 0, -1, -1, 0);
	}
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]=%f\r", szCmd, iIndex, fVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]=%f\r", szCmd, iIndex, fVal);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if ( rc < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncArray' (ELMO_FLOAT) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETASYNC_ERR ;
}

/**! \fn ElmoSetAsyncParameter(char szCmd[3], const int iVal)
 * \brief send EoE command asynchronously.
 * \param szCmd the command
 * \param iVal value to set.
 * \return 0 on success otherwise error (<0).
 */
ELMO_INT32 CMMCEoE::ElmoSetAsyncParameter(ELMO_INT8 szCmd[3], const ELMO_INT32 iVal) throw (CMMCException)
{
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s=%d\r", szCmd, iVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s=%d\r", szCmd, iVal);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if (rc  < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncParameter' (ELMO_INT32) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETASYNC_ERR ;
}

/**! \fn ElmoSetAsyncParameter(char szCmd[3], const int fVal)
 * \brief sends EoE command asynchronously.
 * \param szCmd the command
 * \param fValue value to set.
 * \return 0 on success otherwise error (<0).
 */
ELMO_INT32 CMMCEoE::ElmoSetAsyncParameter(ELMO_INT8 szCmd[3], const ELMO_FLOAT fVal) throw (CMMCException)
{
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s=%f\r", szCmd, fVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s=%f\r", szCmd, fVal);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if (rc  < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetAsyncParameter' (ELMO_FLOAT) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETASYNC_ERR ;
}

/**! \fn ElmoSetArray(char szCmd[3], short iIndex, const int iVal)
 * \brief send EoE array command.
 * \param szCmd the command
 * \param iIndex EoE array index
 *  \param iVal integer value to set.
 * \return 0 on success otherwise error (<0).
 */
ELMO_INT32 CMMCEoE::ElmoSetArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex, const ELMO_INT32 iVal) throw (CMMCException)
{
   	if (iIndex < 0 || iIndex > 1)
	{
        fprintf(stderr, "\x1b[31m" "%s(%d): " "\x1b[0m" "\n", __FUNCTION__, __LINE__);
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetArray' (ELMO_INT32) Illegal iIndex ", 0, 0, -1, -1, 0);
	}

    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]=%d\r", szCmd, iIndex, iVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]=%d\r", szCmd, iIndex, iVal);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetArray' (ELMO_INT32) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETSYNC_ERR;
}

/**! \fn ElmoSetArray(char szCmd[3], short iIndex, const int fVal)
 * \brief send EoE array command.
 * \param szCmd the command
 * \param iIndex EoE array index
 *  \param fVal real value to set.
 * \return 0 on success otherwise error (<0).
 */
ELMO_INT32 CMMCEoE::ElmoSetArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex, const ELMO_FLOAT fVal) throw (CMMCException)
{
    if (iIndex < 0 || iIndex > 1)
	{
        fprintf(stderr, "\x1b[31m" "%s(%d): " "\x1b[0m" "\n", __FUNCTION__, __LINE__);
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetArray' (ELMO_FLOAT) Illegal iIndex ", 0, 0, -1, -1, 0);
	}
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]=%f\r", szCmd, iIndex, fVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]=%f\r", szCmd, iIndex, fVal);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetArray' (ELMO_FLOAT) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETSYNC_ERR;
}


/*! \fn ElmoSetParameter(char szCmd[3], const int iVal)
* \brief sets parameter of type int.
* \param szCmd string command.
* \param iVal integer value to set.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoSetParameter(ELMO_INT8 szCmd[3], const ELMO_INT32 iVal) throw (CMMCException)
{
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s=%d\r", szCmd, iVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s=%d\r", szCmd, iVal);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetParameter' (ELMO_INT32) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETSYNC_ERR;
}

/*! \fn ElmoSetParameter(char szCmd[3], const int fVal)
* \brief sets parameter of type float.
* \param szCmd string command.
* \param fVal float value to set.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoSetParameter(ELMO_INT8 szCmd[3], const ELMO_FLOAT fVal) throw (CMMCException)
{
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s=%f\r", szCmd, fVal);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s=%f\r", szCmd, fVal);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoSetParameter' (ELMO_FLOAT) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETSYNC_ERR;
}

/*! \fn ElmoGetAsyncArray(char szCmd[3], short iIndex, int& fVal)
* \brief sends asynchronous array command to get EoE parameter.
*       one may get the result asynchronously as follows:
*       call IsReady, then ElmoReadData API.
* \param szCmd string command.
* \param iIndex index to array cell.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetAsyncArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex) throw (CMMCException)
{
    if (iIndex < 0 || iIndex > 1)
	{
        fprintf(stderr, "\x1b[31m" "%s(%d): " "\x1b[0m" "\n", __FUNCTION__, __LINE__);
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetAsyncArray' Illegal iIndex ", 0, 0, -1, -1, 0);
	}

    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]\r", szCmd, iIndex);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]\r", szCmd, iIndex);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if (rc < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetAsyncArray' failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETASYNC_ERR ;
}

/*! \fn ElmoGetAsyncParameter(char szCmd[3])
* \brief sends asynchronous command to get EoE parameter.
*       one may get the result asynchronously as follows:
*       call IsReady, then ElmoReadData API.
* \param szCmd string command.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetAsyncParameter(ELMO_INT8 szCmd[3]) throw (CMMCException)
{
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s\r", szCmd);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s\r", szCmd);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if ( rc  < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetAsyncParameter' failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETASYNC_ERR ;
}

/*! \fn ElmoGetArray(char szCmd[3], short iIndex, int& iVal)
* \brief sends command to get EoE parameter of type integer.
* \param szCmd string command.
* \param iIndex index to EoE array.
* \param iVal reference for returned value.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex, ELMO_INT32& iVal) throw (CMMCException)
{
    if (iIndex < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetArray' (ELMO_INT32) Illegal iIndex ", 0, 0, -1, -1, 0);
    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]\r", szCmd, iIndex);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]\r", szCmd, iIndex);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetArray' (ELMO_INT32) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    if (rc>0)
        rc = ParseResult(_szBuffer, iVal);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETSYNC_ERR ;
}

/*! \fn ElmoGetArray(char szCmd[3], short iIndex, int& iVal)
* \brief sends command to get EoE parameter of type float.
* \param szCmd string command.
* \param iIndex index to EoE array.
* \param fVal reference for returned value.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetArray(ELMO_INT8 szCmd[3], ELMO_INT16 iIndex, ELMO_FLOAT & fVal) throw (CMMCException)
{
    if (iIndex < 0)
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetArray' (ELMO_FLOAT) Illegal iIndex ", 0, 0, -1, -1, 0);

    ELMO_INT32 rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iSize = sprintf_s(_szBuffer,"%s[%d]\r", szCmd, iIndex);
#else
    ELMO_INT32 iSize = sprintf  (_szBuffer,"%s[%d]\r", szCmd, iIndex);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetArray' (ELMO_FLOAT) failed in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    if (rc>0)
        rc = ParseResult(_szBuffer, fVal);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETSYNC_ERR ;
}

/**! \fn ParseResult(char *szResult, float& fVal)
 * \brief parses read string and set fVal with accordance.
 * \param szResult read data from UDP socket (EoE)
 * \param fVal set with real value if completed successfully.
 * \return 1 if completed successfully, 0 otherwise.
 */
ELMO_INT32 CMMCEoE::ParseResult(ELMO_PINT8 szResult, ELMO_FLOAT& fVal)
{
    ELMO_INT32 rc;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    rc = sscanf_s(szResult,"%*s\r%f", &fVal);
#else
    rc = sscanf  (szResult,"%*s\r%f", &fVal);
#endif

    return (rc);
}
/**! \fn ParseResult(char *szResult, int& iVal)
 * \brief parses read string and set fVal with accordance.
 * \param szResult read data from UDP socket (EoE)
 * \param iVal set with real value if completed successfully.
 * \return 1 if completed successfully, 0 otherwise.
 */
ELMO_INT32 CMMCEoE::ParseResult(ELMO_PINT8 szResult, ELMO_INT32& iVal)
{
    ELMO_INT32 rc;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    rc = sscanf_s(szResult,"%*s\r%d", &iVal);
#else
    rc = sscanf  (szResult,"%*s\r%d", &iVal);
#endif

    return (rc);
}

/**! \fn ParseResult(char *szResult, char szRetVal[], int iSize)
 * \brief parses read string and set szRetVal with accordance.
 * \param szResult read data from UDP socket (EoE)
 * \param szRetVal set with string value if completed successfully.
 * \param iSize maximum length allowed for szRetVal.
 * \return 1 if completed successfully, 0 otherwise.
 */
ELMO_INT32 CMMCEoE::ParseResult(ELMO_PINT8 szResult, ELMO_INT8 szRetVal[], ELMO_INT32 iSize)
{
    ELMO_INT32 rc;
    ELMO_INT8 format[32];
    if (iSize > EoE_BUFFER_SIZE)
        iSize = EoE_BUFFER_SIZE;

    //prepare format with required length according to iSize parameter.
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    _snprintf_s(format, sizeof(format), sizeof(format), "%%*s\r%%%d[^\n]", iSize);
    rc = sscanf_s(szResult, format, szRetVal);
#else
    snprintf(format, sizeof(format), "%%*s\r%%%d[^\n]", iSize);
    rc = sscanf(szResult, format, szRetVal);
#endif

    rc = strlen(szRetVal);
    return (rc);
}

/*! \fn ElmoGetParameter(char szCmd[3], char szVal[], int iSize)
* \brief gets EoE parameter of type string.
* \param szCmd string command.
* \param szVal  storage for returned value.
* \param iSize size of szVal.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetParameter(ELMO_INT8 szCmd[3], ELMO_INT8 szVal[], ELMO_INT32 iSize) throw (CMMCException) 
{
    ELMO_INT32 iLen, rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iLen = sprintf_s(_szBuffer,"%s\r", szCmd);
#else
    iLen = sprintf  (_szBuffer,"%s\r", szCmd);
#endif

    if ( (rc = this->SendTo(_szBuffer, iLen)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetParameter' fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    if (rc > 0)
    {
        rc = ParseResult(_szBuffer,  szVal, iSize);
    }
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETSYNC_ERR ;
}

/*! \fn ElmoGetParameter(char szCmd[3], int& iVal)
* \brief gets EoE parameter of type integer.
* \param szCmd string command.
* \param iVal reference for returned value.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetParameter(ELMO_INT8 szCmd[3], ELMO_INT32& iVal) throw (CMMCException)
{
    ELMO_INT32 iSize, rc = MMC_OK;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iSize = sprintf_s(_szBuffer,"%s\r", szCmd);
#else
    iSize = sprintf  (_szBuffer,"%s\r", szCmd);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetParameter' (ELMO_INT32) fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }

    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    if (rc > 0) {
        rc = ParseResult(_szBuffer, iVal);
    }
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETSYNC_ERR ;
}

/*! \fn ElmoGetParameter(char szCmd[3], float& fVal)
* \brief gets EoE parameter of type float.
* \param szCmd string command.
* \param fVal reference for returned value.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoGetParameter(ELMO_INT8 szCmd[3], ELMO_FLOAT& fVal) throw (CMMCException)
{
    ELMO_INT32 iSize, rc;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iSize = sprintf_s(_szBuffer,"%s\r", szCmd);
#else
    iSize = sprintf  (_szBuffer,"%s\r", szCmd);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoGetParameter' (ELMO_FLOAT) fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    if (rc) {
        rc = ParseResult(_szBuffer, fVal);
    }
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_GETSYNC_ERR ;
}


/*! \fn ElmoCall(const char szCmd[3])
* \brief sends EoE command for execution.
* \param szCmd string command.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoCall(const ELMO_INT8 szCmd[3]) throw (CMMCException)
{
    ELMO_INT32 iSize, rc;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iSize = sprintf_s(_szBuffer,"%s\r", szCmd);
#else
    iSize = sprintf  (_szBuffer,"%s\r", szCmd);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoCall' fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETSYNC_ERR;
}

/*! \fn ElmoCallAsync(const char szCmd[3])
* \brief sends EoE command for execution.
* \param szCmd string command.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoCallAsync(const ELMO_INT8 szCmd[3]) throw (CMMCException)
{
    ELMO_INT32 iSize, rc;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iSize = sprintf_s(_szBuffer,"%s\r", szCmd);
#else
    iSize = sprintf  (_szBuffer,"%s\r", szCmd);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if ( rc < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoCallAsync' fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETASYNC_ERR ;
}

/*! \fn ElmoExecuteLabel(const char *szCmd)
* \brief sends EoE command for user program execution.
* \param szCmd string command.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoExecuteLabel(const ELMO_INT8* szCmd) throw (CMMCException)
{
    ELMO_INT32 iSize, rc;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iSize = sprintf_s(_szBuffer,"XQ##%s\r", szCmd);
#else
    iSize = sprintf  (_szBuffer,"XQ##%s\r", szCmd);
#endif

    if ( (rc = this->SendTo(_szBuffer, iSize)) < 0)
    {
        this->ReleaseSemSyncDrvIo();
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoExecuteLabel' fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    rc  = GetAck(_szBuffer, EoE_BUFFER_SIZE);
    this->ReleaseSemSyncDrvIo();
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETSYNC_ERR;
}

/*! \fn ElmoExecuteLabelAsync(const char *szCmd)
* \brief sends EoE command for user program execution.
* \param szCmd string command.
* \return 0 on success otherwise error (<0).
*/
ELMO_INT32 CMMCEoE::ElmoExecuteLabelAsync(const ELMO_INT8* szCmd) throw (CMMCException)
{
    ELMO_INT32 iSize, rc;
    this->TakeSemSyncDrvIo();

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iSize = sprintf_s(_szBuffer,"XQ##%s\r", szCmd);
#else
    iSize = sprintf  (_szBuffer,"XQ##%s\r", szCmd);
#endif

    rc = this->SendTo(_szBuffer, iSize);
    this->ReleaseSemSyncDrvIo();
    if (rc < 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'ElmoExecuteLabelAsync' fail in SendTo ", 0, 0, rc, -1, 0);
        return MMC_LIB_EoE_SENDCMD_ERR;
    }
    return (rc > 0) ? MMC_OK : MMC_LIB_EoE_SETASYNC_ERR ;
}


#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
                /* Windows implementation: Private functions Sync I/O from/to driver */
    ELMO_INT32 CMMCEoE::InitSemSyncDrvIo(void) throw (CMMCException)
    {
        return (0);
    }
    
    ELMO_INT32 CMMCEoE::DestroySemSyncDrvIo(void) throw (CMMCException)
    {
        return (0);
    }
    
    ELMO_INT32 CMMCEoE::TakeSemSyncDrvIo(void) throw (CMMCException)
    {
        return (0);
    }
    
    ELMO_INT32 CMMCEoE::ReleaseSemSyncDrvIo(void) throw (CMMCException)
    {
        return (0);
    }
    
#elif ((OS_PLATFORM == LINUXIPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC32_PLATFORM) || (OS_PLATFORM == LINUXRPC64_PLATFORM))



                /* Linux   implementation: Private functions Sync I/O from/to driver */
    ELMO_INT32 CMMCEoE::InitSemSyncDrvIo(void) throw (CMMCException)
    {
        ELMO_INT32           rt          = 0;
  const ELMO_INT8*           cFunctionName = "pthread_mutex_init failed";
        MMC_CONNECT_HNDL    uiConnHndl  = NULL;
        ELMO_UINT16          usAxisRef   = 0;
        ELMO_INT32           iRetCode;
        ELMO_INT16           sErrorID;
        ELMO_UINT16          usStatus;
    
    
        rt = pthread_mutex_init(&this->gSemSyncDrvIo, NULL);
        if (rt != 0)
        {
            iRetCode    = rt;
            sErrorID    = 0;
            usStatus    = -1;
            CMMCPPGlobal::Instance()->MMCPPThrow(cFunctionName, uiConnHndl, usAxisRef, iRetCode, sErrorID, usStatus);
            rt = -1;
        }
    
    return (rt);
    }
    
    ELMO_INT32 CMMCEoE::DestroySemSyncDrvIo(void) throw (CMMCException)
    {
        ELMO_INT32       rt  = 0;
  const ELMO_INT8*       cFunctionName = "pthread_mutex_destroy failed";
        MMC_CONNECT_HNDL uiConnHndl  = NULL;
        ELMO_UINT16      usAxisRef   = 0;
        ELMO_INT32       iRetCode;
        ELMO_INT16       sErrorID;
        ELMO_UINT16      usStatus;
    
        rt = pthread_mutex_destroy(&this->gSemSyncDrvIo);
        if (rt != 0)
        {
            iRetCode    = rt;
            sErrorID    = 0;
            usStatus    = -1;
            CMMCPPGlobal::Instance()->MMCPPThrow(cFunctionName, uiConnHndl, usAxisRef, iRetCode, sErrorID, usStatus);
            rt = -1;
        }
        return (rt);
    }
    
    ELMO_INT32 CMMCEoE::TakeSemSyncDrvIo(void) throw (CMMCException)
    {
        ELMO_INT32       rt          = 0;
  const ELMO_INT8*       cFunctionName = "pthread_mutex_lock failed";
        MMC_CONNECT_HNDL uiConnHndl  = NULL;
        ELMO_UINT16      usAxisRef   = 0;
        ELMO_INT32       iRetCode;
        ELMO_INT16       sErrorID;
        ELMO_UINT16      usStatus;
    
        rt = pthread_mutex_lock(&this->gSemSyncDrvIo);
        if (rt)
        {
            iRetCode    = rt;
            sErrorID    = 0;
            usStatus    = -1;
            CMMCPPGlobal::Instance()->MMCPPThrow(cFunctionName, uiConnHndl, usAxisRef, iRetCode, sErrorID, usStatus);
            rt = -1;
        }
        return (rt);
    }
    
    ELMO_INT32 CMMCEoE::ReleaseSemSyncDrvIo(void) throw (CMMCException)
    {
        ELMO_INT32       rt          = 0;
  const ELMO_INT8*       cFunctionName = "pthread_mutex_unlock failed";
        MMC_CONNECT_HNDL uiConnHndl  = NULL;
        ELMO_UINT16      usAxisRef   = 0;
        ELMO_INT32       iRetCode;
        ELMO_INT16       sErrorID;
        ELMO_UINT16      usStatus;
    
        rt = pthread_mutex_unlock(&this->gSemSyncDrvIo);
        if (rt != 0)
        {
            iRetCode    = rt;
            sErrorID    = 0;
            usStatus    = -1;
            CMMCPPGlobal::Instance()->MMCPPThrow(cFunctionName, uiConnHndl, usAxisRef, iRetCode, sErrorID, usStatus);
            rt = -1;
        }
        return (rt);
    }

#elif    (OS_PLATFORM == VXWORKS32_PLATFORM)
    #error "***MMCEoE.cpp Symbol 'OS_PLATFORM' (1) is out of range or not implemented yet for vxWorks... (See OS_PlatformSelect.h) "
#else
    #error "***MMCEoE.cpp Symbol 'OS_PLATFORM' (1) is out of range... (See OS_PlatformSelect.h) "
#endif
