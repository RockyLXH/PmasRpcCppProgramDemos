/*
 * CMMCNode.cpp
 *
 *  Created on: 27/03/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCNode.hpp"


CMMCNode::CMMCNode() {}
CMMCNode::~CMMCNode() {}
//
/*! \fn int  MMC_SendSdoCmd(
* \brief This function send SDO message command.
* \param  lData -
* \param  ucService -
* \param  ucSubIndex -
* \param  ulDataLength -
* \param  usIndex -
* \param  usSlaveID -
* \return   return - none on success, otherwise throws CMMCException
*/
void CMMCNode::SendSdoCmd(ELMO_LINT32 lData,
        ELMO_UINT8   ucService,
        ELMO_UINT8   ucSubIndex,
        ELMO_ULINT32 ulDataLength,
        ELMO_UINT16  usIndex,
        ELMO_UINT16  usSlaveID) throw (CMMCException)
{
    // This function is not support Upload sdo because we not have a proper inteface to the user
    // We add two new functions that perform Upload\Download sdo
    // If you still using the old function you can send only Download function.
    if(ucService == 2)
    {
        _MMCPP_TRACE_INFO("Cannot Upload data using this function, please use  ""SendSdoUpload"" function.")
        CMMCPPGlobal::Instance()->MMCPPThrow("SendSdoCmd",m_uiConnHndl,m_usAxisRef,(ELMO_INT32)(-1),(ELMO_INT16)(NC_PARAM_OUT_OF_RANGE),(ELMO_UINT16)(0));
        return;
    }
    else if(ucService == 1)
    {
        SendSdoDownload(lData,ucSubIndex,ulDataLength,usIndex,usSlaveID);
    }
    else
    {
        // This service ID is not exist
        CMMCPPGlobal::Instance()->MMCPPThrow("SendSdoCmd",m_uiConnHndl,m_usAxisRef,(ELMO_INT32)(-1),(ELMO_INT16)(NC_PARAM_OUT_OF_RANGE),(ELMO_UINT16)(0));
    }
    //
    _MMCPP_TRACE_INFO("SendSdoCmd OK")
}

//
/*! \fn int  SendSdoDownloadExCmd(
* \brief This function send SDO message command.
* \param  uData -
* \param  usIndex -
* \param  ucSubIndex -
* \param  ucDataLength -
* \return	return - none on success, otherwise throws CMMCException
//@IR - 13.09.17 - Added EC 8+ bytes SDO support.
*/
void CMMCNode::SendSdoDownloadExCmd(
	SEND_SDO_DATA_EX *uData,	
	ELMO_UINT16 usIndex,
	ELMO_UINT8  ucSubIndex,	
	ELMO_UINT8  ucDataLength)throw (CMMCException)
{
	ELMO_INT32 rc;
	MMC_SENDSDOEX_IN stInParams;
	MMC_SENDSDOEX_OUT stOutParam;

	memcpy((void*)&stInParams.uData.pData, (void*)uData, ucDataLength);	
	stInParams.ucService = 1;//download
	stInParams.usIndex = usIndex;
	stInParams.ucSubIndex = ucSubIndex;
	stInParams.ucDataLength = ucDataLength;
	
	if ((rc = MMC_SendSdoExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam)) != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("SendSdoDownloadExCmd (Download):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
	}
		
	_MMCPP_TRACE_INFO("SendSdoDownloadExCmd OK")
}

/*! \fn int  SendSdoUploadExCmd(
* \brief This function send SDO message command.
* \param  uData -
* \param  usIndex -
* \param  ucSubIndex -
* \param  ucDataLength -
* \return	return - none on success, otherwise throws CMMCException
//@IR - 13.09.17 - Added EC 8+ bytes SDO support.
*/
void CMMCNode::SendSdoUploadExCmd(
	SEND_SDO_DATA_EX *uData,
	ELMO_UINT16 usIndex,
	ELMO_UINT8  ucSubIndex,
	ELMO_UINT8  ucDataLength)throw (CMMCException)
{
	ELMO_INT32 rc;
	MMC_SENDSDOEX_IN stInParams;
	MMC_SENDSDOEX_OUT stOutParam;
	
	stInParams.ucService = 2;//upload
	stInParams.usIndex = usIndex;
	stInParams.ucSubIndex = ucSubIndex;
	stInParams.ucDataLength = ucDataLength;

	if ((rc = MMC_SendSdoExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam)) != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("SendSdoUploadExCmd (upload):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
	}

	memcpy((void*)uData,(void*)&stOutParam.uData.pData, ucDataLength);

	_MMCPP_TRACE_INFO("SendSdoUploadExCmd OK")
}

/*! \fn int  SendSdoUploadAsyncExCmd(
* \brief This function send SDO message command.
* \param  uData -
* \param  usIndex -
* \param  ucSubIndex -
* \param  ucDataLength -
* \return	return - none on success, otherwise throws CMMCException
//@IR - 13.09.17 - Added EC 8+ bytes SDO support.
*/
void CMMCNode::SendSdoUploadAsyncExCmd(
	SEND_SDO_DATA_EX *uData,
	ELMO_UINT16 usIndex,
	ELMO_UINT8  ucSubIndex,
	ELMO_UINT8  ucDataLength)throw (CMMCException)
{
	ELMO_INT32 rc;
	MMC_SENDSDOEX_IN stInParams;
	MMC_SENDSDOEX_OUT stOutParam;

	stInParams.ucService = 2;//upload
	stInParams.usIndex = usIndex;
	stInParams.ucSubIndex = ucSubIndex;
	stInParams.ucDataLength = ucDataLength;
	
	if ((rc = MMC_SendSdoAsyncExCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam)) != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("SendSdoUploadAsyncExCmd (upload):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
	}
	
	_MMCPP_TRACE_INFO("SendSdoUploadAsyncExCmd OK")
}

/*! \fn int  RetrieveSdoUploadAsyncExCmd(
* \brief This function send SDO message command.
* \param  uData -
* \param  usIndex -
* \param  ucSubIndex -
* \param  ucDataLength -
* \return	return - none on success, otherwise throws CMMCException
//@IR - 13.09.17 - Added EC 8+ bytes SDO support.
*/
void CMMCNode::RetrieveSdoUploadAsyncExCmd(
	SEND_SDO_DATA_EX *uData,
	ELMO_UINT16 usIndex,
	ELMO_UINT8  ucSubIndex,
	ELMO_UINT8  ucDataLength)throw (CMMCException)
{
	ELMO_INT32 rc;	
	MMC_SENDSDOEX_IN stInParam;
	MMC_SENDSDOEX_OUT stOutParam;	

	stInParam.ucDataLength = ucDataLength;
	if ((rc = MMC_RetrieveSdoAsyncExCmd(m_uiConnHndl, m_usAxisRef,&stInParam, &stOutParam)) != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("RetreiveSdoUploadAsyncExCmd (upload):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
	}

	memcpy((void*)uData, (void*)&stOutParam.uData.pData, ucDataLength);

	_MMCPP_TRACE_INFO("RetreiveSdoUploadAsyncExCmd OK")
}


/*! \fn int  MMC_SendSdoCmd(
* \brief This function send SDO message command.*/
void CMMCNode::SendSdoDownload(ELMO_LINT32 lData,
        ELMO_UINT8   ucSubIndex,
        ELMO_ULINT32 ulDataLength,
        ELMO_UINT16  usIndex,
        ELMO_UINT16  usSlaveID) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_SENDSDO_IN stInParams;
    MMC_SENDSDO_OUT stOutParam;
    stInParams.lData = lData;
    stInParams.ucService = 1;
    stInParams.ucSubIndex = ucSubIndex;
    stInParams.ulDataLength = ulDataLength;
    stInParams.usIndex = usIndex;
    stInParams.usSlaveID = usSlaveID;
    //
    if ((rc = MMC_SendSdoCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SendSdoCmd (Download):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("SendSdoCmd OK")
}

/*! \fn int  MMC_SendSdoCmd(
* \brief This function send SDO message command.*/
ELMO_LINT32 CMMCNode::SendSdoUpload(ELMO_UINT8 ucSubIndex,
        ELMO_ULINT32 ulDataLength,
        ELMO_UINT16 usIndex,
        ELMO_UINT16 usSlaveID) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_SENDSDO_IN stInParams;
    MMC_SENDSDO_OUT stOutParam;
    stInParams.lData = 0;
    stInParams.ucService = 2;
    stInParams.ucSubIndex = ucSubIndex;
    stInParams.ulDataLength = ulDataLength;
    stInParams.usIndex = usIndex;
    stInParams.usSlaveID = usSlaveID;
    //
    if ((rc = MMC_SendSdoCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_SendSdoCmd (Upload):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    //
    _MMCPP_TRACE_INFO("SendSdoCmd OK")
    //
    return  stOutParam.lData;
}



/*! \fn int  SendSdoUpload(
* \brief This function send SDO message command.*/
void CMMCNode::SendSdoUploadAsync(ELMO_UINT8 ucSubIndex,
        ELMO_ULINT32 ulDataLength,
        ELMO_UINT16 usIndex,
        ELMO_UINT16 usSlaveID) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_SENDSDO_IN stInParams;
    MMC_SENDSDO_OUT stOutParam;
    stInParams.lData = 0;
    stInParams.ucService = 2;
    stInParams.ucSubIndex = ucSubIndex;
    stInParams.ulDataLength = ulDataLength;
    stInParams.usIndex = usIndex;
    stInParams.usSlaveID = usSlaveID;
    //
    if ((rc = MMC_SendSdoAsyncCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("SendSdoAsyncCmd (Upload):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    //
    _MMCPP_TRACE_INFO("SendSdoAsync OK")
    //
    return;
}

//
/*! \fn int  SendSdoUpload(
* \brief This function send SDO message command.*/
void CMMCNode::RetreiveSdoUploadAsync(ELMO_LINT32 & lData) throw (CMMCException)
{
    ELMO_INT32 rc;
    MMC_SENDSDO_OUT stOutParam;
    //
    //
    if ((rc = MMC_RetrieveSdoAsyncCmd(m_uiConnHndl, m_usAxisRef, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("RetrieveSdoAsyncCmd (Retreive):", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    //
    lData = stOutParam.lData;
    _MMCPP_TRACE_INFO("RetrieveSdoAsync OK")
    //
    return;
}

/*! \fn int ReadStatus()
*   \brief This function gets axis status for specific axis.
*   \return return - status on success, otherwise throws CMMCException.
*/
ELMO_ULINT32 CMMCNode::ReadStatus() throw (CMMCException)
{
      MMC_READSTATUS_IN       read_status_in ;
      MMC_READSTATUS_OUT      read_status_out ;
      ELMO_INT32 rc;
      if ((rc = MMC_ReadStatusCmd(m_uiConnHndl, m_usAxisRef, &read_status_in, &read_status_out)) != 0)
      {
          CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadStatusCmd:", m_uiConnHndl, m_usAxisRef, rc, read_status_out.usErrorID, read_status_out.usStatus);
      }

      _MMCPP_TRACE_INFO("ReadStatus OK")
      return read_status_out.ulState;
}
//
ELMO_ULINT32 CMMCNode::ReadStatus(ELMO_UINT16& usAxisErrorID, ELMO_UINT16& usStatusWord) throw (CMMCException)
{
      MMC_READSTATUS_IN       read_status_in ;
      MMC_READSTATUS_OUT      read_status_out ;
      ELMO_INT32 rc;
      if ((rc = MMC_ReadStatusCmd(m_uiConnHndl, m_usAxisRef, &read_status_in, &read_status_out)) != 0)
      {
          CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadStatusCmd:", m_uiConnHndl, m_usAxisRef, rc, read_status_out.usErrorID, read_status_out.usStatus);
      }
      usAxisErrorID = read_status_out.usAxisErrorID;
      usStatusWord = read_status_out.usStatusWord;
      _MMCPP_TRACE_INFO("ReadStatus OK")
      return read_status_out.ulState;
}
//
/*! \fn void mReset()
 * \brief This function sends reset command to MMC server for this axis.
 * \return - 0 on success, otherwise throws CMMCException
 */
void CMMCNode::Reset() throw (CMMCException)
{
    IN MMC_RESET_IN stInParam;
    OUT MMC_RESET_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.ucExecute = 1;
    if ((rc = MMC_Reset(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_Reset:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("Reset OK")
    return;
}
//
/*! \fn void ResetAsync()
 * \brief This function sends reset Async. command to MMC server for this axis.
 * \return - 0 on success, otherwise throws CMMCException
 */
void CMMCNode::ResetAsync() throw (CMMCException)
{
    IN MMC_RESET_IN stInParam;
    OUT MMC_RESET_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.ucExecute = 1;
    if ((rc = MMC_ResetAsync(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ResetAsync:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("ResetAsync OK")
    return;
}
/*! \fn void PDOGeneralRead()
*   \brief This function gets General PDO from the axis (this PDO defined by the user).
*   \return - Return the actual value of the PDO.
*/
MMCPPULL_T CMMCNode::PDOGeneralRead(ELMO_UINT8 ucParam)throw (CMMCException)
{
    ELMO_INT32 rc;
    IN MMC_GENERALPARAMPDOREAD_IN   pInParam;
    OUT MMC_GENERALPARAMPDOREAD_OUT pOutParam;
    //
    pInParam.ucParam = ucParam;
    //
    if ((rc = MMC_PDOGeneralReadCmd(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PDOGeneralRead:", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("PDOGeneralRead OK")
    return pOutParam.ulliVal;
}
//
/*! \fn void PDOGeneralWrite()
*   \brief This function sets General PDO from the axis (this PDO defined by the user).
*   \return - void.
*/
//
void CMMCNode::PDOGeneralWrite(ELMO_UINT8 ucParam,MMCPPULL_T ulliVal)throw (CMMCException)
{
    PDOGeneralWrite(ucParam,ulliVal);
    return;
}

//
/*! \fn void PDOGeneralWrite()
*   \brief This function sets General PDO from the axis (this PDO defined by the user).
*   \return - void.
*/
//
void CMMCNode::PDOGeneralWrite(ELMO_UINT8 ucParam,unGeneralPDOWriteData DataUnion)throw (CMMCException)
{
    ELMO_INT32 rc;
    IN MMC_GENERALPARAMPDOWRITE_IN      pInParam;
    OUT MMC_GENERALPARAMPDOWRITE_OUT    pOutParam;
    //
    pInParam.ucParam = ucParam;
    pInParam.DataUnion = DataUnion;
    //
    if ((rc = MMC_PDOGeneralWriteCmd(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_PDOGeneralWrite:", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("PDOGeneralWrite OK")
    return;
}

//
/*! \fn void GetPDOInfo()
*   \brief This function get information about PDO 3\4.
*   IN uiPDONumber          - PDO number (3\4)
*   OUT iPDOEventMode       - PDO event mode (NO\CYCLE\IMMEDIATE)
*   OUT ucPDOCommType       - Communication parameter (SYNC\ASYNC\NO_EVENT)
*   OUT ucPDOCommEventGroup - PDO Current group (1 .... 17)
*   \return - void.
*/
void CMMCNode::GetPDOInfo(ELMO_UINT8 uiPDONumber,ELMO_INT32 &iPDOEventMode, ELMO_UINT32 &uiCommParamEventPDO,ELMO_UINT16 &usEventTimerPDO, ELMO_UINT8 &ucRPDOCommType,ELMO_UINT8 &ucTPDOCommType, ELMO_UINT8 &ucTPDOCommEventGroup, ELMO_UINT8 &ucRPDOCommEventGroup, ELMO_UINT8 &ucSubIndexTPDO, ELMO_UINT8 &ucSubindexRPDO) throw(CMMCException)
{
    ELMO_INT32 rc;
    IN MMC_GETPDOINFO_IN    pInParam;
    OUT MMC_GETPDOINFO_OUT  pOutParam;
    //
    pInParam.ucPDONumber = uiPDONumber;
    //
    if ((rc = MMC_GetPDOInfoCmd(m_uiConnHndl, m_usAxisRef, &pInParam, &pOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetPDOInfoCmd:", m_uiConnHndl, m_usAxisRef, rc, pOutParam.usErrorID, pOutParam.usStatus);
    }
    iPDOEventMode           =   pOutParam.iPDOEventMode;
    uiCommParamEventPDO     =   pOutParam.uiCommParamEventPDO;
    usEventTimerPDO         =   pOutParam.usEventTimerPDO;
    ucRPDOCommType          =   pOutParam.ucRPDOCommType;
    ucTPDOCommType          =   pOutParam.ucTPDOCommType;
    ucTPDOCommEventGroup    =   pOutParam.ucTPDOCommEventGroup;
    ucRPDOCommEventGroup    =   pOutParam.ucRPDOCommEventGroup;
    ucSubIndexTPDO          =   pOutParam.ucSubIndexTPDO;
    ucSubindexRPDO          =   pOutParam.ucSubIndexRPDO;
    //
    _MMCPP_TRACE_INFO("GetPDOInfo OK")
    return;
}


/*! \fn double pGetParameter(MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief This function reads specific axis parameter.
 * \param eNumber - parameter number.
 * \param iIndex - index into parameters array if axis group.
 * \return - parameter value on success, otherwise throws CMMCException.
 */
ELMO_DOUBLE CMMCNode::GetParameter(MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_READPARAMETER_IN stInParam;
    MMC_READPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetParameter OK");
    return stOutParam.dbValue;
}

/*!fn unsigned long pGetBoolParameter(MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief This function reads boolean specific axis parameter.
 * \param eNumber - parameter enumeration.
 * \param  iIndex - index into array in case of a group.
 * \return  return - value of specified parameter on success, otherwise throws CMMCException.
 */
ELMO_LINT32 CMMCNode::GetBoolParameter(MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_READBOOLPARAMETER_IN stInParam;
    MMC_READBOOLPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadBoolParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadBoolParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("GetBoolParameter OK")
    return stOutParam.lValue;
}

/*! \fn void pSetParameter(double dbValue, MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief this method writes specific axis parameter.
 * \param dbValue - value to set.
 * \param eNumber - parameter enumeration.
 * \param  iIndex - index into array in case of a group.
 * \return  return - none on success, otherwise throws CMMCException.
 */
void CMMCNode::SetParameter(ELMO_DOUBLE dbValue, MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_WRITEPARAMETER_IN stInParam;
    MMC_WRITEPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.dbValue = dbValue;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_WriteParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetParameter OK")
}

/*! \fn void pSetBoolParameter(unsigned long ulValue, MMC_PARAMETER_LIST_ENUM eNumber, int iIndex)
 * \brief this method write boolean specific axis parameter.
 * \param ulValue - value to set.
 * \param eNumber - parameter enumeration.
 * \param  iIndex - index into array in case of a group.
 * \return  return - none on success, otherwise throws CMMCException.
 */
void CMMCNode::SetBoolParameter(ELMO_LINT32 lValue, MMC_PARAMETER_LIST_ENUM eNumber, ELMO_INT32 iIndex) throw (CMMCException)
{
    MMC_WRITEBOOLPARAMETER_IN stInParam;
    MMC_WRITEBOOLPARAMETER_OUT stOutParam;
    ELMO_INT32 rc;
    stInParam.lValue = lValue;
    stInParam.eParameterNumber = eNumber;
    stInParam.iParameterArrIndex = iIndex;
    stInParam.ucEnable = 1;

    if ((rc=MMC_WriteBoolParameter(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteBoolParameter:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    _MMCPP_TRACE_INFO("SetBoolParameter OK")
}

/*! \fn unsigned short mGetAxisError(unsigned short* usLastEmergencyErrCode)
 * \brief This function read axis error.
 * \param usLastEmergencyErrCode - reference to retrieve last emergency error.
 * \return - axis error and last emergency error on success, otherwise throws MMCEception.
 */
ELMO_UINT16 CMMCNode::GetAxisError(ELMO_PUINT16 usLastEmergencyErrCode) throw (CMMCException)
{
    MMC_READAXISERROR_IN    stInParam;
    MMC_READAXISERROR_OUT   stOutParam;
    ELMO_INT32 rc;
    stInParam.ucEnable = 1;

    if ((rc=MMC_ReadAxisError(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadAxisError:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    
    *usLastEmergencyErrCode = stOutParam.usLastEmergencyErrCode;
    _MMCPP_TRACE_INFO("GetAxisError OK")
    return stOutParam.usAxisErrorID;
}


/*! \fn void ConfigPDOEventMode(unsigned char ucPDOEventMode)
 * \brief this method sets PDO event mode for immediate response.
 * \param ePDOEventMode - no-notification(0), on-cycle(1), immediate(2)
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCNode::ConfigPDOEventMode(MC_PDO_EVENT_NOTIF_MODE_ENUM ePDOEventMode,  PDO_NUMBER_ENUM ePDONum) throw (CMMCException) {
	ELMO_INT32 rc = 0;

	switch(ePDONum)
	{
	case PDO_NUM_3:
		MMC_CONFIGEVENTMODEPDO3_IN stInParams3;
		MMC_CONFIGEVENTMODEPDO3_OUT stOutParams3;
		stInParams3.ucPDOEventMode = (ELMO_UINT8)ePDOEventMode;
		rc=MMC_CfgEventModePDO3Cmd(m_uiConnHndl, m_usAxisRef, &stInParams3, &stOutParams3);
		if ((rc)!=0)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("ConfigPDOEventMode:", m_uiConnHndl, m_usAxisRef, rc, stOutParams3.usErrorID, stOutParams3.usStatus);
		}
	break;
	case PDO_NUM_4:
		MMC_CONFIGEVENTMODEPDO4_IN stInParams4;
		MMC_CONFIGEVENTMODEPDO4_OUT stOutParams4;
		stInParams4.ucPDOEventMode = (ELMO_UINT8)ePDOEventMode;
		rc=MMC_CfgEventModePDO4Cmd(m_uiConnHndl, m_usAxisRef, &stInParams4, &stOutParams4);
		if ((rc)!=0)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("ConfigPDOEventMode:", m_uiConnHndl, m_usAxisRef, rc, stOutParams4.usErrorID, stOutParams4.usStatus);
		}
	break;
	default:
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CfgEventModePDO3Cmd:", m_uiConnHndl, m_usAxisRef, (-1), NC_WRONG_FUNC_ARGUMENT_TYPE, 0);
	break;
	}
    _MMCPP_TRACE_INFO("ConfigPDOEventMode OK")
}


/*! \fn void ConfigPDOEventMode(unsigned char ucPDOEventMode)
 * \brief this method sets PDO event mode for immediate response.
 * \param ucPDOEventMode - no-notification(0), on-cycle(1), immediate(2)
 * \return - none on success, otherwise throws CMMCException.
 */
void CMMCNode::ConfigPDOEventMode(ELMO_UINT8 ucPDOEventMode,  PDO_NUMBER_ENUM ePDONum) throw (CMMCException) {
    ELMO_INT32 rc = 0;

    switch(ePDONum)
    {
    case PDO_NUM_3:
        MMC_CONFIGEVENTMODEPDO3_IN stInParams3;
        MMC_CONFIGEVENTMODEPDO3_OUT stOutParams3;
        stInParams3.ucPDOEventMode = ucPDOEventMode;
        rc=MMC_CfgEventModePDO3Cmd(m_uiConnHndl, m_usAxisRef, &stInParams3, &stOutParams3);
        if ((rc)!=0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ConfigPDOEventMode:", m_uiConnHndl, m_usAxisRef, rc, stOutParams3.usErrorID, stOutParams3.usStatus);
        }
    break;
    case PDO_NUM_4:
        MMC_CONFIGEVENTMODEPDO4_IN stInParams4;
        MMC_CONFIGEVENTMODEPDO4_OUT stOutParams4;
        stInParams4.ucPDOEventMode = ucPDOEventMode;
        rc=MMC_CfgEventModePDO4Cmd(m_uiConnHndl, m_usAxisRef, &stInParams4, &stOutParams4);
        if ((rc)!=0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("ConfigPDOEventMode:", m_uiConnHndl, m_usAxisRef, rc, stOutParams4.usErrorID, stOutParams4.usStatus);
        }
    break;
    default:
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CfgEventModePDO3Cmd:", m_uiConnHndl, m_usAxisRef, (-1), NC_WRONG_FUNC_ARGUMENT_TYPE, 0);
    break;
    }
    _MMCPP_TRACE_INFO("ConfigPDOEventMode OK")
}



void CMMCNode::EthercatWriteMemoryRange(ELMO_UINT16 usRegAddr, ELMO_UINT8 ucLength, ELMO_UINT8 pData[ETHERCAT_MEMORY_WRITE_MAX_SIZE]) throw (CMMCException)
{
    MMC_WRITEMEMORYRANGE_IN stInParam;
    MMC_WRITEMEMORYRANGE_OUT stOutParam;
    ELMO_INT32 rc;
    //
    memcpy(stInParam.pData,pData,ETHERCAT_MEMORY_WRITE_MAX_SIZE);
    stInParam.ucLength = ucLength;
    stInParam.usRegAddr = usRegAddr;
    //
    if ((rc=MMC_WriteMemoryRange(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteMemoryRange:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WriteMemoryRange OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucByteLength,   ELMO_UINT8 pRawData[PI_REG_VAR_SIZE]) throw (CMMCException)
{
    MMC_WRITEPIVARRAW_IN    stInParam;
    MMC_WRITEPIVARRAW_OUT stOutParam;
    ELMO_INT32 rc;
    //
    memcpy(stInParam.pRawData,pRawData,PI_REG_VAR_SIZE);
    stInParam.ucByteLength = ucByteLength;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarRaw(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarRaw:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarRaw OK")
    return;
}


void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT16 usByteLength, ELMO_UINT8 pRawData[PI_LARGE_VAR_SIZE]) throw (CMMCException)
{
    MMC_WRITELARGEPIVARRAW_IN   stInParam;
    MMC_WRITELARGEPIVARRAW_OUT stOutParam;
    ELMO_INT32 rc;
    //
    memcpy(stInParam.pRawData,pRawData,PI_LARGE_VAR_SIZE);
    stInParam.usByteLength = usByteLength;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WriteLargePIVarRaw(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WriteLargePIVarRaw:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WriteLargePIVarRaw OK")
    return;
}

// HH: see remarks in 'EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucData)'
void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_BOOL bData) throw (CMMCException)
{
    MMC_WRITEPIVARBOOL_IN   stInParam;
    MMC_WRITEPIVARBOOL_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.ucBOOL = bData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarBool(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarBool:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarBool OK")
    return;
}


void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_INT8 cData) throw (CMMCException)
{
    MMC_WRITEPIVARCHAR_IN   stInParam;
    MMC_WRITEPIVARCHAR_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.cData = cData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarChar(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarChar:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarChar OK")
    return;
}


// HH: On IntegrationOS3264 (when data type for C, CPP, GNU & Visual are share) 'ELMO_BOOL' and
//      one of 'ELMO_UINT8' or 'ELMO_INT8' are same. This cause cpp compiler
//      cannot overload functions (specific func is acording to 'ELMO_BOOL'
//      define).
//      Now (when 'ELMO_BOOL' define 'unsigned char') the functions with
//      same signature should remove:
//          Same signature:
//          EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_BOOL bData)      -->> Stay
//          EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucData)    -->> Removed
//
//          Same signature:
//          EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_BOOL &bData)   -->> Stay
//          EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT8 &ucData) -->> Removed
//      There also conflic (or undefined) data type when the 'ELMO_BOOL' defined
//      as '_Bool' (in one of set compile by C, CPP, GNU & Visual).
void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucData) throw (CMMCException)
{
    MMC_WRITEPIVARUCHAR_IN  stInParam;
    MMC_WRITEPIVARUCHAR_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.ucData = ucData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarUChar(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarUChar:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarUChar OK")
    return;
}


void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT16 usData) throw (CMMCException)
{
    MMC_WRITEPIVARUSHORT_IN stInParam;
    MMC_WRITEPIVARUSHORT_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.usData = usData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarUShort(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarUShort:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarUShort OK")
    return;
}


void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_INT16 sData) throw (CMMCException)
{
    MMC_WRITEPIVARSHORT_IN  stInParam;
    MMC_WRITEPIVARSHORT_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.sData = sData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarShort(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarShort:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarShort OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT32 uiData) throw (CMMCException)
{
    MMC_WRITEPIVARUINT_IN   stInParam;
    MMC_WRITEPIVARUINT_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.uiData = uiData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarUInt(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarUInt:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarUInt OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_INT32 iData) throw (CMMCException)
{
    MMC_WRITEPIVARINT_IN    stInParam;
    MMC_WRITEPIVARINT_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.iData = iData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarInt(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarInt:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarInt OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, float fData) throw (CMMCException)
{
    MMC_WRITEPIVARFLOAT_IN  stInParam;
    MMC_WRITEPIVARFLOAT_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.fData = fData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarFloat(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarFloat:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarFloat OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_DOUBLE dbData) throw (CMMCException)
{
    MMC_WRITEPIVARDOUBLE_IN stInParam;
    MMC_WRITEPIVARDOUBLE_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.dbVal = dbData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarDouble(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarDouble:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarDouble OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT64 ullData) throw (CMMCException)
{
    MMC_WRITEPIVARULONGLONG_IN  stInParam;
    MMC_WRITEPIVARULONGLONG_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.ullData = ullData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarULongLong(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarULongLong:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarULongLong OK")
    return;
}

void CMMCNode::EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_INT64 llData) throw (CMMCException)
{
    MMC_WRITEPIVARLONGLONG_IN   stInParam;
    MMC_WRITEPIVARLONGLONG_OUT stOutParam;
    ELMO_INT32 rc;
    //
    stInParam.llData = llData;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_WritePIVarLongLong(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WritePIVarLongLong:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    _MMCPP_TRACE_INFO("WritePIVarLongLong OK")
    return;
}


void CMMCNode::EthercatReadMemoryRange(ELMO_UINT16 usRegAddr, ELMO_UINT8 ucLength, ELMO_UINT8 pData[ETHERCAT_MEMORY_READ_MAX_SIZE]) throw (CMMCException)
{
    MMC_READMEMORYRANGE_IN  stInParam;
    MMC_READMEMORYRANGE_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucLength = ucLength;
    stInParam.usRegAddr = usRegAddr;
    //
    if ((rc=MMC_ReadMemoryRange(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadMemoryRange:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //
    memcpy(pData,stOutParam.pData,ETHERCAT_MEMORY_WRITE_MAX_SIZE);

    _MMCPP_TRACE_INFO("ReadMemoryRange OK")
    return;
}

void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT8 ucByteLength, ELMO_UINT8 pRawData[PI_REG_VAR_SIZE]) throw (CMMCException)
{
    MMC_READPIVARRAW_IN stInParam;
    MMC_READPIVARRAW_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucByteLength = ucByteLength;
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarRaw(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarRaw:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    memcpy(pRawData, stOutParam.pRawData, PI_REG_VAR_SIZE);
    _MMCPP_TRACE_INFO("ReadPIVarRaw OK")
    return;
}

void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT16 usByteLength, ELMO_UINT8 pRawData[PI_LARGE_VAR_SIZE]) throw (CMMCException)
{
    MMC_READLARGEPIVARRAW_IN    stInParam;
    MMC_READLARGEPIVARRAW_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.usByteLength = usByteLength;
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadLargePIVarRaw(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadLargePIVarRaw:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    memcpy(pRawData, stOutParam.pRawData, PI_LARGE_VAR_SIZE);
    _MMCPP_TRACE_INFO("ReadLargePIVarRaw OK")
    return;
}

// HH: see remarks in 'EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucData)'
void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_BOOL &bData) throw (CMMCException)
{
    MMC_READPIVARBOOL_IN    stInParam;
    MMC_READPIVARBOOL_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarBOOL(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarBOOL:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    bData = (ELMO_BOOL)stOutParam.ucBOOL;
    _MMCPP_TRACE_INFO("ReadPIVarBOOL OK")
    return;
}

void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_INT8 &cData) throw (CMMCException)
{
    MMC_READPIVARCHAR_IN    stInParam;
    MMC_READPIVARCHAR_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarChar(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarChar:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    cData = stOutParam.cData;
    _MMCPP_TRACE_INFO("ReadPIVarChar OK")
    return;
}

//
// HH: see remarks in 'EthercatWritePIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucData)'
void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT8 &ucData) throw (CMMCException)
{
    MMC_READPIVARUCHAR_IN   stInParam;
    MMC_READPIVARUCHAR_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarUChar(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarUChar:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    ucData = stOutParam.ucData;
    _MMCPP_TRACE_INFO("ReadPIVarUChar OK")
    return;
}


void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT16 &usData) throw (CMMCException)
{
    MMC_READPIVARUSHORT_IN  stInParam;
    MMC_READPIVARUSHORT_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarUShort(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarUShort:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    usData = stOutParam.usData;
    _MMCPP_TRACE_INFO("ReadPIVarUShort OK")
    return;
}


void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_INT16 &sData) throw (CMMCException)
{
    MMC_READPIVARSHORT_IN   stInParam;
    MMC_READPIVARSHORT_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarShort(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarShort:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    sData = stOutParam.sData;
    _MMCPP_TRACE_INFO("ReadPIVarShort OK")
    return;
}


void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT32 &uiData) throw (CMMCException)
{
    MMC_READPIVARUINT_IN    stInParam;
    MMC_READPIVARUINT_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarUInt(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarUInt:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    uiData = stOutParam.uiData;
    _MMCPP_TRACE_INFO("ReadPIVarUInt OK")
    return;
}


void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_INT32 &iData) throw (CMMCException)
{
    MMC_READPIVARINT_IN stInParam;
    MMC_READPIVARINT_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarInt(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarInt:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    iData = stOutParam.iData;
    _MMCPP_TRACE_INFO("ReadPIVarInt OK")
    return;
}


void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_FLOAT &fData) throw (CMMCException)
{
    MMC_READPIVARFLOAT_IN   stInParam;
    MMC_READPIVARFLOAT_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarFloat(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarFloat:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    fData = stOutParam.fData;
    _MMCPP_TRACE_INFO("ReadPIVarFloat OK")
    return;
}



void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_DOUBLE &dbData) throw (CMMCException)
{
    MMC_READPIVARDOUBLE_IN  stInParam;
    MMC_READPIVARDOUBLE_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarDouble(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarDouble:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    dbData = stOutParam.dbData;
    _MMCPP_TRACE_INFO("ReadPIVarDouble OK")
    return;
}

    
void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_UINT64 &ullData) throw (CMMCException)
{
    MMC_READPIVARULONGLONG_IN   stInParam;
    MMC_READPIVARULONGLONG_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarULongLong(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarULongLong:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    ullData = stOutParam.ullData;
    _MMCPP_TRACE_INFO("ReadPIVarULongLong OK")
    return;
}


void CMMCNode::EthercatReadPIVar(ELMO_UINT16 usIndex, ELMO_UINT8 ucDirection, ELMO_INT64 &llData) throw (CMMCException)
{
    MMC_READPIVARLONGLONG_IN    stInParam;
    MMC_READPIVARLONGLONG_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usIndex = usIndex;
    //
    if ((rc=MMC_ReadPIVarLongLong(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ReadPIVarLongLong:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    llData = stOutParam.llData;
    _MMCPP_TRACE_INFO("ReadPIVarLongLong OK")
    return;
}


void CMMCNode::EthercatPIVarInfo(ELMO_UINT16 usPIVarIndex, ELMO_UINT8 ucDirection, NC_PI_ENTRY &VarInfo) throw (CMMCException)
{
    MMC_GETPIVARINFO_IN stInParam;
    MMC_GETPIVARINFO_OUT stOutParam;
    ELMO_INT32 rc;
    //  
    stInParam.ucDirection = ucDirection;
    stInParam.usPIVarIndex = usPIVarIndex;
    //
    if ((rc=MMC_GetPIVarInfo(m_uiConnHndl, m_usAxisRef, &stInParam, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetPIVarInfo:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }
    //  
    VarInfo = stOutParam.VarInfo;
    _MMCPP_TRACE_INFO("GetPIVarInfo OK")
    return;
}
