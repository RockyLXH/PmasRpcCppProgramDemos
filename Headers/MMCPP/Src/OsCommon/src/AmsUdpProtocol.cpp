/* ============================================================================
 Name :         AmsUdpProtocol.cpp
 Author  :      Haim Hillel
 Version :      05Jul2015
				24Feb2020
				18May2020
				23Nov2020
 Description :  ADS / AMS Beckhoof protocol - implementation class.
                GMAS implementation for be Server responce to ADS client.


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Meanwhile support only size of AMS_MAX_SIZE_DEF_UDP
	meaning: whenever appear 'size' as parameter set AMS_MAX_SIZE_DEF_UDP
	this because of fix static assinge of (see size of array - look on code):
        // unsigned char   _gucUdpSndBuf[_iMsgMaxSize];
           unsigned char   _gucUdpSndBuf[AMS_MAX_SIZE_DEF_UDP];
	The code (almost) ready for dynmic (on initialize parameter) frames
	size...
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
===================================================================== */



#include "OS_PlatformDependSetting.hpp"

#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)

	#include "AmsUdpProtocol.h"



								/* Call from thread while soket receive data                            */
								/* !!! pucReadBuf[] created and within recive communication thread...   */
	ELMO_INT32     CMMCAmsUdpProtocol::SocketfnClbk             (ELMO_INT16 sSockEvent, ELMO_UINT8 pucReadBuf[], ELMO_INT32 iReadBufSize, ...)
	// =======================================================================================================================================
	{
			ELMO_INT32		iRtVal = 0;
	
	
		if (_giDbgCommuFlag != 0)
		{
			printf("\n ");
			printf("\n ------------------------------------------------------------ ");
			printf("\n >>>AmsDbg %s: %s: Rcv buf size=%d ", __FILE__, __func__, iReadBufSize);
			DumpStreamBuffer(pucReadBuf, iReadBufSize);
			fflush(stdout); fflush(stderr);
		}
	
		DecodeAmsHeader(pucReadBuf);
								/* Assume only recive client requst of type Read or Write.  */
								/* decode ADS/AMS GroupIndex, OffsetIndex and data lenthe.  */
		DecodeGrpOffsetLen(pucReadBuf);
	
	
								/* Write value into Ads Send structure:                                     */
								/*      Fill Header and Response data, copy (some) fields from receive data */
		iRtVal = ResponseClientReq(pucReadBuf, _gucUdpSndBuf);
	
												/* Flag, when==true mean set class methode behave for	*/
												/* IEC. Meaning the user should initiate send remote	*/
												/* client respose. Other it enter after return from the	*/
												/* callback functions.									*/
		if (_gbIecCourse == ELMO_FALSE)
		{
			iRtVal = CMMCAmsUdpProtocol::AmsSendClientRes_UDP();
		}
	
		return (iRtVal);
	}
	
	
								/* Send response to client 											*/
								/* While use for IEC the call initialize from user function.		*/
								/* Function with no parameters, support the case the function calls	*/
								/* (or marks for call) by API set in IEC user program.				*/
	ELMO_INT32     CMMCAmsUdpProtocol::AmsSendClientRes_UDP()
	// ======================================================
	{
		ELMO_INT32  iRtVal = 0;
	
	
	
		iRtVal = BuildUdpSendBuf();
	
	
		iRtVal = Send_UDP((ELMO_PVOID)(_gucUdpSndBuf), _gusSndRespSize);
		if (iRtVal == -1)
		{
			printf("\n *** %s: _gusSndRespSize=%d, iRtVal=%d \n", __func__, (int)_gusSndRespSize, iRtVal);
			fflush(stdout); fflush(stderr);
		}
		else
		{
			if (_giDbgCommuFlag != 0)
			{
				printf("\n >>>AmsDbg %s: _gusSndRespSize=%d ", __func__, _gusSndRespSize);
				DumpStreamBuffer(_gucUdpSndBuf, _gusSndRespSize);
				fflush(stdout); fflush(stderr);
			}
		}
	
		return (iRtVal);
	}
	
								/* Use this (instead of AmsProtocolInit_TCP(...)) while in	*/
								/* IEC course.												*/
	ELMO_INT32     CMMCAmsUdpProtocol::AmsProtocolInitIEC_UDP(ELMO_UINT16  	  usPort,
													   AMS_CLBK_REQ_READ_UDP  fnClbkRespClientReqRead_UDP,
													   AMS_CLBK_REQ_WRITE_UDP fnClbkClientReqWrite_UDP,
													   ELMO_INT32		      iMsgMaxSize)
	// ===================================================================================================
	{
		ELMO_INT32  iRtVal = 0;
												/* Flag, when==true mean set class methode behave for	*/
												/* IEC. Meaning the user should initiate send remote	*/
												/* client respose. Other it enter after return from the	*/
												/* callback functions.									*/
		_gbIecCourse = ELMO_TRUE;
	
		iRtVal = CMMCAmsUdpProtocol::AmsProtocolInit_UDP(usPort,
													   fnClbkRespClientReqRead_UDP,
													   fnClbkClientReqWrite_UDP,
													   iMsgMaxSize);
		return (iRtVal);
	}
	
								/* Use this instead of AmsProtocolInitIEC_UDP(...) while    */
								/* not working against IEC.									*/
	ELMO_INT32     CMMCAmsUdpProtocol::AmsProtocolInit_UDP(ELMO_UINT16     	  usPort,
													   AMS_CLBK_REQ_READ_UDP  fnClbkRespClientReqRead_UDP,
													   AMS_CLBK_REQ_WRITE_UDP fnClbkClientReqWrite_UDP,
													   ELMO_INT32		      iMsgMaxSize)
	// ===================================================================================================
	{
		ELMO_INT32  iRtVal = 0;
	
	
	// ?????????????????? for del    ????????????????
	printf("\n%s: %s _giDbgLclFlag=%d    _giDbgCommuFlag=%d usPort=%d ", __FILE__, __func__, _giDbgLclFlag, _giDbgCommuFlag, usPort);
	_giDbgLclFlag=1;    _giDbgCommuFlag=1;
	printf("\n%s: %s _giDbgLclFlag=%d    _giDbgCommuFlag=%d usPort=%d ", __FILE__, __func__, _giDbgLclFlag, _giDbgCommuFlag, usPort);
	// ?????????????????? for del   ??????????????
	
		if (_giDbgCommuFlag != 0)
		{
			printf("\n >>>AmsDbg %s: %s: _gbIecCourse=%d usPort=%d iMsgMaxSize=%d fnClbkRespClientReqRead_UDP=%p fnClbkClientReqWrite_UDP=%p ",
				   __FILE__, __func__, _gbIecCourse, usPort, iMsgMaxSize, fnClbkRespClientReqRead_UDP, fnClbkClientReqWrite_UDP);
			fflush(stdout); fflush(stderr);
		}
								/* Pointers to user functions calling in responce to Client */
								/* requst Read/Write (return data from server to client).	*/
		_fnClbkRespClientReqRead_UDP = fnClbkRespClientReqRead_UDP;
		_fnClbkClientReqWrite_UDP = fnClbkClientReqWrite_UDP;
	
								/* Create thread listener to UDP Client requst          	*/
		iRtVal = CreateSocketfnClbk_UDP(usPort, iMsgMaxSize);
		if (iRtVal != 0)
		{
			printf("\n *** %s: CreateSocketfnClbk_UDP(usPort=%d, iMsgMaxSize=%d)=%d \n", __func__, (ELMO_INT32)usPort, iMsgMaxSize, iRtVal);
			fflush(stdout); fflush(stderr);
		}
	
	
		return (iRtVal);
	}
	
	
								/* Header includs CommandId which address the req (continue state...)   */
	ELMO_INT32     CMMCAmsUdpProtocol::DecodeAmsHeader       (ELMO_UINT8 ucStrStream[])
	// ================================================================================
	{
		ELMO_INT32     iRtVal = 0;
	
		_gusRcvPortTarget = GET_USHORT(ucStrStream, AMSHDR_PORT_TRG_UDP);
		_gusRcvPortSource = GET_USHORT(ucStrStream, AMSHDR_PORT_SRC_UDP);
								/* Commad Id, type appearing both (exchange between) 'unsigned short'   */
								/* and 'eAMS_CMDId' be carfull because enumerated type is 'int'.        */
								/* in ADS/AMS header the command appear in size of 'short'              */
		_gusRcvCommandId  = (eAMS_CMDId)GET_USHORT(ucStrStream, AMSHDR_CMD_UDP); 
		_gusRcvStateFlags = GET_USHORT(ucStrStream, AMSHDR_STATE_UDP);  
		_guiRcvHdrLen     = GET_USINT (ucStrStream, AMSHDR_LEN_UDP);
		_guiRcvErrorCode  = GET_USINT (ucStrStream, AMSHDR_ERR_CODE_UDP); 
		_guiRcvInvokeId   = GET_USINT (ucStrStream, AMSHDR_INVOKE_ID_UDP);
	
		return (iRtVal);
	}
	
								/* Decode values from Ads Data Area:    */
								/* IndexGroup, IndexOffset, RequstLength*/
	ELMO_INT32     CMMCAmsUdpProtocol::DecodeGrpOffsetLen   (ELMO_UINT8 ucStrStream[])
	// ===============================================================================
	{
		ELMO_INT32     iRtVal = 0;
	
		_guiIndexGroup = GET_USINT(ucStrStream, AMSDATA_IDX_GRP_UDP);
		_guiIndexOffset= GET_USINT(ucStrStream, AMSDATA_IDX_OFST_UDP);
		_guiPureDataLen= GET_USINT(ucStrStream, AMSDATA_REQ_LEN_UDP);
	
		return (iRtVal);
	}
	
								/* Write value into Ads Send structure:                                     		*/
								/*      Fill the Header and the Response data, copy (some) fields from receive data */
								/*      pusRespSize whole size of data put into Send buffer (response size)         */
	ELMO_INT32     CMMCAmsUdpProtocol::ResponseClientReq(ELMO_UINT8 ucRcvBuf[], ELMO_UINT8 ucSndBuf[])
	// ===============================================================================================
	{
	 ELMO_INT32     iRtVal = 0;
	 ELMO_INT32     iInd;
	 eAMS_CMDId    	AMS_CMDId;
	
	
	
		AMS_CMDId = (eAMS_CMDId)_gusRcvCommandId;
	
								/* Preapare Header Data-Structure for user, source: received from client        */
		for (iInd = 0; iInd < 6; iInd++)
		{
			_gtAmsHdrFromClient.cpAMSNetIdTarget[iInd] = ucRcvBuf[AMSHDR_NETID_TRG_UDP+iInd]; 
			_gtAmsHdrFromClient.cpAMSNetIdSource[iInd] = ucRcvBuf[AMSHDR_NETID_SRC_UDP+iInd]; 
		}
	
		_gtAmsHdrFromClient.usAMSPortTarget = _gusRcvPortTarget;
		_gtAmsHdrFromClient.usAMSPortSource = _gusRcvPortSource;
										 
		_gtAmsHdrFromClient.usCommandId     = _gusRcvCommandId;
		_gtAmsHdrFromClient.usStateFlags    = _gusRcvStateFlags;
		_gtAmsHdrFromClient.uiHdrLen		= _guiRcvHdrLen;
		_gtAmsHdrFromClient.uiErrorCode     = _guiRcvErrorCode;
		_gtAmsHdrFromClient.uiInvokeId      = _guiRcvInvokeId;
	
								/* Point on local header. User callback func should change      */
								/* it for point to Null... or update the points data...         */
		_gptAmsHdrToClient = &_gtAmsHdrToClient;
	
		if (_giDbgCommuFlag != 0)
		{
			printf("\n >>>AmsDbg %s: %s: AMS_CMDId=%d(eAMS_CMDIdWrite=%d,  eAMS_CMDIdRead=%d), _guiPureDataLen=%d, _guiIndexGroup=%d, _guiIndexOffset=%d ",
															   __FILE__, __func__, AMS_CMDId, eAMS_CMDIdWrite,  eAMS_CMDIdRead, _guiPureDataLen, _guiIndexGroup, _guiIndexOffset);
			fflush(stdout); fflush(stderr);
		}
	
		if (AMS_CMDId == eAMS_CMDIdWrite)
		{
								/* **************************************************** */
								/* Pointer to user function call while:                 */
								/* CLIENT REQUEST WRITE (client send the data to server)*/
								/* **************************************************** */
			(*_fnClbkClientReqWrite_UDP)(&ucRcvBuf[AMSDATA_WRITE_STR_UDP],
										_guiPureDataLen, _guiIndexGroup, _guiIndexOffset, &_guiResultToClient, &_gtAmsHdrFromClient, &_gptAmsHdrToClient);
		}
		else if (AMS_CMDId == eAMS_CMDIdRead)
		{
								/* **************************************************** */
								/* Pointer to user function call while:                 */
								/* CLIENT REQUEST READ (Server send the data to Client).*/
								/* **************************************************** */
			(*_fnClbkRespClientReqRead_UDP)(&ucSndBuf[AMSDATA_RESP_READ_PURE_DATA_UDP],
										_guiPureDataLen, _guiIndexGroup, _guiIndexOffset, &_guiResultToClient, &_gtAmsHdrFromClient, &_gptAmsHdrToClient);
		}
		else
		{
								/* Unsupport command    */
			printf("\n *** %s: Unsupport AMS_CMDId=%d (Supporting: Write=%d or Read=%d) ", __func__, (ELMO_INT32)AMS_CMDId, (int)eAMS_CMDIdWrite, (int)eAMS_CMDIdRead);
			fflush(stdout); fflush(stderr);
	
			iRtVal = (ELMO_INT32)AMS_CMDId;
		}
	
		return (iRtVal);
	}
	
								/* Build the buffer should send to Client as response */
	ELMO_INT32	CMMCAmsUdpProtocol::BuildUdpSendBuf(void)
	// ==================================================
	{
		ELMO_INT32 	iRtVal = 0;
		ELMO_INT32  iInd;
		ELMO_UINT32	uiCalcHdrDataLen;
		eAMS_CMDId  AMS_CMDId;
		ELMO_UINT16	usStateFlags;
		ELMO_UINT32	uiErrCode;
											/* pointer to buffer filling by recive from client	*/
		ELMO_PUINT8 ucSndBuf;
		ELMO_PUINT8 ucRcvBuf;
	
	
											/* link Secive and Send data buffers to local pointer	*/
											/* using this pointers as arrays.						*/
		ucSndBuf = &_gucUdpSndBuf[0];
		ucRcvBuf = _gucBuffer;
	
	
		AMS_CMDId = (eAMS_CMDId)_gusRcvCommandId;
	
		if (AMS_CMDId == eAMS_CMDIdWrite)
		{
								/* Whole size of compose stream */
								/* size for send.               */
			_gusSndRespSize = AMSDATA_STR_UDP + sizeof(ELMO_INT32);
								/* Whole data area length (=4 => size of resultsField)  */
			uiCalcHdrDataLen = 4;
		}
		else if (AMS_CMDId == eAMS_CMDIdRead)
		{
								/* Whole size of composed stream:       		*/
								/* Size of Header (= start Data index)  		*/
								/* + sizeof(_guiResultToClient + Data_len) (= 8)*/
								/* + sizeof_PureData                    		*/
			_gusSndRespSize = AMSDATA_STR_UDP + 8 + _guiPureDataLen;
								/* Length in Header => Size of whole data area:             */
								/*  sizeof(Result[=4])+sizeof(LenthField[=4])+sizeOfPureData*/
			uiCalcHdrDataLen = 8 + _guiPureDataLen;
								/* Data len => pure data area length => according the Req read... */
			PUT_USINT(ucSndBuf, AMSDATA_RESP_READ_LEN_UDP, _guiPureDataLen);
		}
		else
		{
								/* Unsupport command    */
			printf("\n *** %s: Unsupport AMS_CMDId=%d (Supporting: Write=%d or Read=%d) ", __func__, (ELMO_INT32)AMS_CMDId, (ELMO_INT32)eAMS_CMDIdWrite, (ELMO_INT32)eAMS_CMDIdRead);
			fflush(stdout); fflush(stderr);
	
			iRtVal = (ELMO_INT32)AMS_CMDId;
			goto ExitBuildUdp;
		}
	
	
	// ????????????????????????? For DEL / UPDATE / Correct....  ??????????????!!!!!!!!!!!!!!!
	_gptAmsHdrToClient = NULL;
	// ????????????????????????? For DEL / UPDATE / Correct....  ??????????????!!!!!!!!!!!!!!!
	
		if (_gptAmsHdrToClient == NULL)
		{
						/* Header NOT updated in CallbackFunc.      */
						/* Copy AMS Header from RcvBuf to SndBuf    */
						/* **************************************** */
								/* Cross copy AMS-NetId Target and Source   */
			for (iInd = 0; iInd < 6; iInd++)
			{
				ucSndBuf[AMSHDR_NETID_TRG_UDP+iInd] = ucRcvBuf[AMSHDR_NETID_SRC_UDP+iInd];
				ucSndBuf[AMSHDR_NETID_SRC_UDP+iInd] = ucRcvBuf[AMSHDR_NETID_TRG_UDP+iInd];
			}
								/* Cross copy AMS-port Target and Source,   */
								/* keep Endian as revived                   */
			for (iInd = 0; iInd < 2; iInd++)
			{
				ucSndBuf[AMSHDR_PORT_TRG_UDP+iInd] = ucRcvBuf[AMSHDR_PORT_SRC_UDP+iInd];
				ucSndBuf[AMSHDR_PORT_SRC_UDP+iInd] = ucRcvBuf[AMSHDR_PORT_TRG_UDP+iInd];
			}
	
								/* Command Id: same invoke Id as in RCV */
			for (iInd = 0; iInd < 2; iInd++)
			{
				ucSndBuf[AMSHDR_CMD_UDP+iInd] = ucRcvBuf[AMSHDR_CMD_UDP+iInd];
			}
	
						/* Header for Response  */
						/* ******************** */
								/* Ads State Flags:                                         */
								/* The first bit marks, whether it´s a request or response. */
								/* Mask 0x0001 0: Request / 1: Response                     */
								/* Third bit set to 1, to exchange data with ADS commands   */
								/* Mask 0x0004 1: ADS command                               */
								/* Bit 7 marks, if it should be transfered with TCP or UDP. */
								/* 0x000x   TCP Protocol                                    */
								/* 0x004x   UDP Protocol                                    */
			usStateFlags  = 0x0001;     /* bit#1 = 1 => Response        */
			usStateFlags += 0x0004;     /* bit#3 = 0 => its ADS command */
			usStateFlags += 0x0040;     /* bit#7 = 1 => UDP             */
			PUT_USHORT(ucSndBuf, AMSHDR_STATE_UDP, usStateFlags);
								/* Set Length in Hdr */
			PUT_USINT(ucSndBuf, AMSHDR_LEN_UDP, uiCalcHdrDataLen);
			
								/* Put Error Code */
			uiErrCode = 0; /* !!!! Should Be Parameter... !!!*/
			PUT_USINT(ucSndBuf, AMSHDR_ERR_CODE_UDP,  uiErrCode);
			
								/* Invoke Id, same invoke Id as in RCV */
			for (iInd = 0; iInd < 4; iInd++)
			{
				ucSndBuf[AMSHDR_INVOKE_ID_UDP+iInd] = ucRcvBuf[AMSHDR_INVOKE_ID_UDP+iInd];
			}
		}
		else
		{
						/* Header Updated in CallbackFunc.                      */
						/* User update the header, supplay data (for Header)    */
						/* should write into fields of '_gtAmsHdrToClient'      */
						/* **************************************************** */
								/* Cross copy AMS-NetId Target and Source*/
			for (iInd = 0; iInd < 6; iInd++)
			{
				ucSndBuf[AMSHDR_NETID_TRG_UDP+iInd] = _gptAmsHdrToClient->cpAMSNetIdSource[iInd];
				ucSndBuf[AMSHDR_NETID_SRC_UDP+iInd] = _gptAmsHdrToClient->cpAMSNetIdTarget[iInd];
			}
	
								/* Cross copy AMS-port Target and Source,*/
								/* keep Endian as revived                */
			PUT_USHORT(ucSndBuf, AMSHDR_PORT_TRG_UDP, _gptAmsHdrToClient->usAMSPortSource);
			PUT_USHORT(ucSndBuf, AMSHDR_PORT_SRC_UDP, _gptAmsHdrToClient->usAMSPortTarget);
								/* Command Id   */
			PUT_USHORT(ucSndBuf, AMSHDR_CMD_UDP, _gptAmsHdrToClient->usCommandId);
								/* State Flags  */
			PUT_USHORT(ucSndBuf, AMSHDR_STATE_UDP, _gptAmsHdrToClient->usStateFlags);
								/* Set Length in Hdr */
			PUT_USINT(ucSndBuf, AMSHDR_LEN_UDP, _gptAmsHdrToClient->uiHdrLen);
								/* Error Code   */
			PUT_USINT(ucSndBuf, AMSHDR_ERR_CODE_UDP,  _gptAmsHdrToClient->uiErrorCode);
								/* Invoke Id    */
			PUT_USINT(ucSndBuf, AMSHDR_INVOKE_ID_UDP, _gptAmsHdrToClient->uiInvokeId);
		}
								/* Put results    */
		PUT_USINT(ucSndBuf, AMSDATA_STR_UDP, _guiResultToClient);
	
		if (_giDbgCommuFlag != 0)
		{
			printf("\n >>>AmsDbg %s: %s: AMS_CMDId=%d  _guiResultToClient=%d ", __FILE__, __func__, AMS_CMDId, _guiResultToClient);
			fflush(stdout); fflush(stderr);
		}
	
	ExitBuildUdp:
		return (iRtVal);
	}
	
	
	void    CMMCAmsUdpProtocol::AmsSetDbg(ELMO_INT32 iDbgLclFlag, ELMO_INT32 iDbgCommuFlag)
	// ====================================================================================
	{
		_giDbgLclFlag = iDbgLclFlag;
		_giDbgCommuFlag = iDbgCommuFlag;
	}
	
	
								/* Service for debug    */
	void CMMCAmsUdpProtocol::DumpStreamBuffer(ELMO_UINT8 pucStrStream[], ELMO_UINT32 uiNumCharForPrint)
	// ================================================================================================
	{
											/* According needs can change for be parameter */
	ELMO_UINT32    	uiNumInOneLine = ADS_AMS_BYTES_IN_DUMP_LINE;
	ELMO_UINT32
					uiNumCompLine,
					uiLineInd,
					uiInLineInd,
					uiResInLineNum,
					uiResForComplLine;
	
	ELMO_UINT8	    uchData;
	
	
	
								/* Print the recive data in Hex and Ascii   */
								/* ======================================   */
	 uiNumCompLine = uiNumCharForPrint / uiNumInOneLine;
								/* Loop on complet lines including ADS_AMS_BYTES_IN_DUMP_LINE char data */
	 for (uiLineInd = 0; uiLineInd < uiNumCompLine; uiLineInd++)
	 {
								/* Possition of the new line data   */
		printf("\n\t");
								/* Hex: print complet in line data  */
		for (uiInLineInd = 0; uiInLineInd < uiNumInOneLine; uiInLineInd++)
		{
			printf("%3x", (ELMO_INT32)pucStrStream[uiLineInd*uiNumInOneLine+uiInLineInd]);
		}
		
								/* shift for position of writting ascii */
		printf("\t\t");
		
								/* Ascii: print complet in line data    */
		for (uiInLineInd = 0; uiInLineInd < uiNumInOneLine; uiInLineInd++)
		{
			uchData = pucStrStream[uiLineInd*uiNumInOneLine+uiInLineInd];
			if ( isprint(uchData) )
			{
				printf("%c ", (int)pucStrStream[uiLineInd*uiNumInOneLine+uiInLineInd]);
			}
			else
			{
				printf(". ");
			}
		}
	 }
		
								/* Resido - less then one complet line  */
								/* ===================================  */
	 uiResInLineNum = uiNumCompLine*uiNumInOneLine;
	 if (uiResInLineNum < uiNumCharForPrint)
	 {
								/* Possition of the new line data */
		printf("\n\t");
								/* Hex: print resido in line data */
		for (uiInLineInd = uiResInLineNum; uiInLineInd < uiNumCharForPrint; uiInLineInd++)
		{
		   printf("%3x", (int)pucStrStream[uiInLineInd]);
		}
		   
								/* Add space for till (like it) complet line*/
								/* start next print on expecting poss.       */
		uiResForComplLine = uiNumInOneLine - (uiNumCharForPrint % uiNumInOneLine);
		for (uiInLineInd = 0; uiInLineInd < uiResForComplLine; uiInLineInd++)
		{
		   printf("   ");
		}
		   
								/* shift for position of writting ascii  */
		printf("\t\t");
		   
								/* Ascii: print resido in line data  */
		for (uiInLineInd = uiResInLineNum; uiInLineInd < uiNumCharForPrint; uiInLineInd++)
		{
		   uchData = pucStrStream[uiInLineInd];
		   if ( isprint(uchData) )
		   {
			   printf("%c ", (int)pucStrStream[uiInLineInd]);
		   }
		   else
		   {
			   printf(". ");
		   }
		}
	 }
	}

#endif /* #if   (OS_PLATFORM == LINUXIPC32_PLATFORM) */

