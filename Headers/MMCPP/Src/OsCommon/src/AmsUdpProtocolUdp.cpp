/* ============================================================================
 Name :         AmsUdpProtocolUdp.cpp
 Author  :      Haim Hillel
 Version :      05Jul2015
				24Feb2020
				23Nov2020
 Description :  ADS / AMS Beckhoof protocol - implementation class.
                GMAS implementation for be Server responce to ADS client.
                The Udp communication functions.


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
	
	
											/* From MMCUDP ???? */
											/* From MMCUDP ???? */
	/*! \fn int SetSocketTimeout(int  iMilliseconds)
	* \brief this function set socket to block only for iMilliseconds on receive operation.
	* \param iMilliseconds - timeout in ms to wait on receive.
	* \return   0 on success, otherwise -1 as an error
	*/
	ELMO_INT32 CMMCAmsUdpProtocol::SetSocketTimeout(ELMO_INT32  iMilliseconds)
	{
	   struct timeval tv;
	
		tv.tv_sec = iMilliseconds / 1000 ;
		tv.tv_usec = ( iMilliseconds % 1000) * 1000  ;
	
	   return setsockopt (_iSock, SOL_SOCKET, SO_RCVTIMEO, (ELMO_PINT8)&tv, sizeof tv);
	}
	
	
	/*! \fn int Close_UDP()
	* \brief this function closes the socket.
	* \return   return - 0 on success, otherwise throws CMMCException or -1 is returned.
	*/
	ELMO_INT32 CMMCAmsUdpProtocol::Close_UDP() {
		return (_iSock!=-1)? ::close(_iSock) : MMC_OK;
	}
	
	/*-------------------------------------------------------------------------
	* new API for UDP class basically to comply with TCP API
	*  iTimeOut milliseconds
	*/
	static ELMO_INT32 IsReadable_UDP(ELMO_INT32 iSock, ELMO_INT32 iTimeOut) throw (CMMCException) {
	  ELMO_INT32 rt_val;
	  fd_set 	 fdRead;

	  if (iSock == -1)
		  CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable_UDP' Is 'CMMCAmsUdpProtocol' Class initialized?", 0, 0, -1, MMC_LIB_UDP_ISREADABLE_INI_ERR, 0);
	
	  FD_ZERO(&fdRead);
	  FD_SET(iSock,&fdRead);
	  struct timeval tv; 
	  struct timeval* ptv = &tv;
	
	  if (iTimeOut > 0)
	  {
		tv.tv_sec  = iTimeOut / 1000;
		tv.tv_usec = 0;
	  }
	  else if (iTimeOut == 0)
	  {
		tv.tv_sec  = 0;
		tv.tv_usec = 0;
	  }
	  else
	  {
		  ptv = NULL;  //wait for ever until data arrives.
	  }
	
	  rt_val = select(FD_SETSIZE, &fdRead, 0, 0, ptv);
	  if (rt_val == MMCPP_SOCKERROR)
	  {
		printf("\n%s: %s: select()=%d, Wait for Data Timeout on iSock=%d, iTimeOut=%d(mSec) ", __FILE__, __func__, rt_val, iSock, iTimeOut);
		fflush(stdout); fflush(stderr);
		CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable_UDP' Error On select ", 0, 0, rt_val, MMC_LIB_UDP_ISREADABLE_SEL_ERR, 0);
		return 0;
	  }
	
	  rt_val = FD_ISSET(iSock,&fdRead);
	  return (rt_val != 0); 
	} /* IsReadable_UDP */
	
	ELMO_BOOL CMMCAmsUdpProtocol::IsReadable_UDP(ELMO_INT32 iTimeOut) { // milliseconds
	
		return (::IsReadable_UDP(_iSock, iTimeOut));
	}
	
	ELMO_INT32 CMMCAmsUdpProtocol::IsReady_UDP()
	{
		return ::IsReadable_UDP(_iSock, 0L);
	}
	
	/**\!fn int CreateSocketfnClbk_UDP (unsigned short usPort, int iMsgMaxSize=AMS_MAX_SIZE_DEF_UDP)
	 * \brief creates UDP none blocking server (listener)
	 * \param usPort port number to listen on (or to bind with)
	 * \param iMsgMaxSize largest possible message (in bytes). relevant only for call-back mode of operation.
	 * \return 0 on success, -1 otherwise. socket number (iSock) is update on success, otherwise -1;
	 */
	ELMO_INT32 CMMCAmsUdpProtocol::CreateSocketfnClbk_UDP (ELMO_UINT16 usPort, ELMO_INT32 iMsgMaxSize) throw (CMMCException)
	{
		_iMsgMaxSize = (iMsgMaxSize < 0) ? AMS_MAX_SIZE_DEF_UDP : iMsgMaxSize;
	
		_usPort = usPort;
		_iSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (_iSock == MMCPP_INVSOCKET)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (UDP) socket ", 0, 0, MMCPP_INVSOCKET, MMC_LIB_UDP_CREATE_SOCKET_ERR, 0);
			return MMCPP_RETERROR;
		}
	
		RunClbkThread();
	
		printf("\n%s: %s: Aft call to RunClbkThread() ", __FILE__, __func__);
		fflush(stdout); fflush(stderr);
	
		return MMC_OK;
	}
	
	/**\!fn int Send_UDP(void * pData, unsigned short usSize, sockaddr_in* pSockaddr)
	 * \brief sends udp message pointed by pData.
	 * \param pData (IN) pointer to data to send
	 * \param usSize message size.
	 * \param pSockaddr pointer to socket address. default is NULL.
	 *        On call-back mode it may be pointed to the socket address with data from last receive.
	 * \return the number sent, or -1 for errors.
	 */
	ELMO_INT32 CMMCAmsUdpProtocol::Send_UDP(ELMO_PVOID pData, ELMO_UINT16 usSize, sockaddr_in* pSockaddr) throw (CMMCException)
	{
		ELMO_INT32 iRet;
		sockaddr_in *psaddr = (pSockaddr)?pSockaddr:&m_sockAddrOut;
	
		if (_iSock == MMCPP_INVSOCKET)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("'Send_UDP' (Is Initialize?)", 0, 0, -1, MMC_LIB_UDP_SEND_SOCKINV_ERR, 0);
			return MMCPP_RETERROR;
		}
	
		if (usSize > _iMsgMaxSize)
			 usSize = _iMsgMaxSize;
	
		iRet = sendto (_iSock, pData, usSize, MSG_NOSIGNAL, (const sockaddr*)psaddr, sizeof (struct sockaddr));
	
		return iRet;
	}
	
	
	/**\!fn int Receive_UDP (void * pData, unsigned short usSize, long lDelay, sockaddr_in* pSockaddr)
	 * \brief receives UDP message pointed by pData.
	 * \param pData (IN) pointer to buffer, which will store the received data.
	 * \param usSize message size to read.
	 * \param pSockaddr pointer to socket address. default is NULL.
	 *        On call-back mode it shell be delivered to Send by call-back function for synchronous matters.
	 * \return the number of bytes read or -1 for errors.
	 */
	ELMO_INT32 CMMCAmsUdpProtocol::Receive_UDP(ELMO_PVOID pData, ELMO_UINT16 usSize, ELMO_LINT32 lDelay, sockaddr_in* pSockaddr) throw (CMMCException)
	{
		ELMO_INT32 	iRet;
		socklen_t 	iAddrSize;
		sockaddr_in *psaddr = (pSockaddr)?pSockaddr:&m_sockAddrOut;
	
		if (_iSock == MMCPP_INVSOCKET)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("'Receive_UDP' (Is init?) ", 0, 0, -1, MMC_LIB_UDP_RECEIVE_SOCKINV_ERR, 0);
			return MMCPP_RETERROR;
		}
	
		if(!::IsReadable_UDP(_iSock, lDelay))
		{
			return 0;
		}
											/* zero mem.    */
		memset(psaddr, 0x0, sizeof(sockaddr_in));
	
		iAddrSize = sizeof (struct sockaddr_in);
	
		if (usSize > _iMsgMaxSize)
			 usSize = _iMsgMaxSize;
	
		iRet = recvfrom (_iSock, (ELMO_PINT8)pData, usSize, 0, (struct sockaddr *)psaddr, &iAddrSize);
		//((char *)pData)[iRet]=0; //what if iRet is -1 or size of pData is exactly usSize
		//printf("\n Receive_UDP: %s, size = %d, iAddrSize = %d ", (char *)pData, iRet, iAddrSize);
		return iRet;
	}
	
	/**! \fn Connect_UDP(char* szAddr, unsigned short usPort, int& iSocket,bool& bWait)
	 * \brief creates a non blocking socket connected to a remote server
	 * \param szAddr (IN) IP address of the server
	 * \param usPort (IN) Ethernet port number of the server
	 * \parm iSocket (OUT) created socket if OK
	 * \param bWait (OUT) set to TRUE if connect is not fully performed
	 * \return: OK or ERROR
	 */
	ELMO_INT32 CMMCAmsUdpProtocol::Connect_UDP(ELMO_PINT8 szAddr, ELMO_UINT16 usPort, ELMO_BOOL& bWait, ELMO_INT32 iMsgMaxSize) throw (CMMCException)
	{
				ELMO_ULINT32   ulRequest;
		struct  sockaddr_in    sadd;
				ELMO_INT32	   rt;
	
		rt = inet_pton(AF_INET, szAddr, &(sadd.sin_addr));
		if (rt != 1)
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_UDP' Illegal IP ", 0, 0, rt, MMC_LIB_UDP_CONNECT_IP_ERR, 0);
	
		_iMsgMaxSize = (iMsgMaxSize < 0) ? AMS_MAX_SIZE_DEF_UDP : iMsgMaxSize;
	
		_iSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (_iSock == MMCPP_INVSOCKET) {
			printf("\n *** %s: socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)=MMCPP_INVSOCKET ", __func__);
			fflush(stdout); fflush(stderr);
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_UDP' socket (is too many?)", 0, 0, MMCPP_INVSOCKET, MMC_LIB_UDP_CONNECT_SOCKET_ERR, 0);
			return MMCPP_RETERROR;
		}
	
		/* set non blocking mode */
		ulRequest = 1L;
		if (ioctl (_iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
		{
			close (_iSock);
			printf("\n *** %s: ioctl()=MMCPP_SOCKERROR  ", __func__);
			fflush(stdout); fflush(stderr);
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_UDP' faile on 'ioctl'", 0, 0, MMCPP_SOCKERROR, MMC_LIB_UDP_CONNECT_CTRL_ERR, 0);
			_iSock = MMCPP_INVSOCKET;
			return MMCPP_RETERROR;
		}
	
		/* set address and port */
		memset(&m_sockAddrOut, 0, sizeof(m_sockAddrOut));
		m_sockAddrOut.sin_family      = AF_INET;
		m_sockAddrOut.sin_port        = htons(usPort);
		m_sockAddrOut.sin_addr.s_addr = inet_addr(szAddr);
					/* Wait is false (no needs wait) while port is writeable */
		bWait = ! IsWritable_UDP();
		return MMC_OK;
	}
	
	/**! \fn bool IsWritable_UDP(int iSock);
	* \brief checks for errors and whether or not connection is ready for write operation.
	* \param iSock client socket connection to check.
	* return true if writable, otherwise false.
	*/
	ELMO_BOOL CMMCAmsUdpProtocol::IsWritable_UDP()
	{
		return (this->IsPending(_iSock)) ? ELMO_FALSE : ELMO_TRUE;
	}
	
	/**! \fn int IsPending(int iSock, bool& bFail)
	* \brief check pending connection on a non blocking socket
	*  actuall checks for errors and whether or not connection is ready for write operation.
	* \param iSock client socket connection to check.
	* \param bFail true if error(socket must be closed then), false otherwise.
	* \return: OK if connection complete / ERROR if fail or still pending
	*/
	ELMO_INT32 CMMCAmsUdpProtocol::IsPending(ELMO_INT32 iSock) throw (CMMCException)
	{
		ELMO_INT32     	iRet;
		struct timeval 	tDelay;
		fd_set 			fdWrite,
						fdCheck;
	
		if (iSock == -1)
			CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' Is 'CMMCAmsUdpProtocol' Class initialized?", 0, 0, -1, MMC_LIB_UDP_ISPENDING_INI_ERR, 0);
	
		/* check if data available */
		FD_ZERO (&fdWrite);
		FD_SET (iSock, &fdWrite);
		FD_ZERO (&fdCheck);
		FD_SET (iSock, &fdCheck);
		tDelay.tv_sec = tDelay.tv_usec = 0L; /* dont wait */
		iRet = select (FD_SETSIZE, NULL, &fdWrite, &fdCheck, &tDelay);
		if (iRet == MMCPP_SOCKERROR)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' failed on 'select' Class initialized?", MMCPP_SOCKERROR, 0, -1, MMC_LIB_UDP_ISPENDING_SEL_ERR, 0);
			return MMCPP_RETERROR;
		}
		/* check for errors */
		if (FD_ISSET (iSock, &fdCheck))
		{
			return MMCPP_RETERROR;
		}
		/* return OK if ready to write */
		if (FD_ISSET (iSock, &fdWrite))
		{
			return MMC_OK;
		}
		/* still pending */
		return MMCPP_RETERROR;
	}
	
	
	ELMO_PVOID fnUDPClbkThread(ELMO_PVOID pObj)
	{
		((CMMCAmsUdpProtocol*)pObj)->ClbkThread();
		return pObj;
	}
	
	ELMO_INT32 CMMCAmsUdpProtocol::RunClbkThread()
	{
		pthread_attr_t attr;
	
		pthread_attr_init(&attr);               /* initialises a thread attributes object with the default value.*/
	
		/* creation of new thread for call-back mode of operation*/
		pthread_create(&_thread, &attr, fnUDPClbkThread, this);
		return MMC_OK;
	}
	
	void CMMCAmsUdpProtocol::ClbkThread()
	{
		ELMO_INT32 	rc, /* Read buff size */
					opt = 1;
		struct sockaddr_in saddr;
	
		_gucBuffer = new ELMO_UINT8[_iMsgMaxSize];
	
		printf("\n%s: %s: _usPort = %d ", __FILE__, __func__, (ELMO_INT32)this->_usPort);
		fflush(stdout); fflush(stderr);
	
		if( setsockopt(_iSock, SOL_SOCKET, SO_REUSEADDR, (ELMO_PINT8)&opt, sizeof(opt)) < 0 )
		{
			perror("setsockopt");
			delete _gucBuffer;
			return;
		}
	
		/* set non blocking mode */
		/*not required on call-back mode since readable data is detected on dedicated thread*/
	
		/* set address and port */
		memset(&saddr, 0, sizeof (saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = INADDR_ANY;
		saddr.sin_port = htons (_usPort);
	
		/* bind */
		if (bind (_iSock, (struct sockaddr *)(&saddr), sizeof (saddr)) == MMCPP_SOCKERROR)
		{
			close (_iSock);
			_iSock = MMCPP_INVSOCKET;
			delete _gucBuffer;
			return;
		}
	
		printf("\n%s: %s: Waiting for data... ", __FILE__, __func__);
		fflush(stdout); fflush(stderr);
		_bClbkThreadAlive = ELMO_TRUE;
	
		while(_bClbkThreadAlive)
		{
			/*if data is ready then receive it from socket and send user notification
			 *  in order to process arrived data and send response. block on socket select.
			 * */
			if ((rc = Receive_UDP(_gucBuffer, _iMsgMaxSize, -1, &m_sockAddrOut)) > 0)
			{
				/*
				 * when serving multiple clients user must save
				 * within the call-back m_sockAddrOut parameters
				 * in his own sockaddr_in storage, then he can use an instance
				 * of his own for sending response.
				 * (e.g. memcpy(&g_sockAddr, pSockaddr, sizeof(sockaddr_in));
				 * */
				SocketfnClbk(MMCPP_SOCK_READY_EVENT, _gucBuffer, rc, &m_sockAddrOut);
			}
			else
			{
				printf("\n%s: %s: Receive_UDP nothing... ", __FILE__, __func__);
				fflush(stdout); fflush(stderr);
				sleep(1);
			}
		}
		printf("\n%s: %s: quit call-back mode thread. ", __FILE__, __func__);
		fflush(stdout); fflush(stderr);
		delete _gucBuffer;
		return;
	}
                                        /* From MMCUDP ???? */
                                        /* From MMCUDP ???? */
#endif /* #if   (OS_PLATFORM == LINUXIPC32_PLATFORM) */

