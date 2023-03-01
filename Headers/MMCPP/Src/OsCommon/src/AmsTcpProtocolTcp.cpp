/* ============================================================================
 Name :         AmsTcpProtocolTcp.cpp
 Author  :      Haim Hillel
 Version :      14Jul2015
				24Feb2020
				23Nov2020
 Description :  ADS / AMS Beckhoof protocol - implementation class.
                GMAS implementation for be Server responce to ADS client.
                The TCP communication functions.


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Meanwhile support only size of AMS_MAX_SIZE_DEF_TCP
	meaning: whenever appear 'size' as parameter set AMS_MAX_SIZE_DEF_TCP
	this because of fix static assinge of (see size of array - look on code):
        // unsigned char   _gucTcpSndBuf[_iMsgMaxSize];
        unsigned char   _gucTcpSndBuf[AMS_MAX_SIZE_DEF_TCP];
	The code (almost) ready for dynmic (on initialize parameter) frames
	size...
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
===================================================================== */

#include "OS_PlatformDependSetting.hpp"

#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)

	#include "AmsTcpProtocol.h"


											/* From MMCTCP ???? */
											/* From MMCTCP ???? */
	
	#ifndef 	MSG_NOSIGNAL
		#define MSG_NOSIGNAL 0
	#endif
	
	
	#undef  max
	#define max(x,y) ((x) > (y) ? (x) : (y))
	
	static ELMO_INT32 IsReadable_TCP(ELMO_INT32 iSock, ELMO_INT32 iTimeOut) throw (CMMCException) { // milliseconds
	  ELMO_INT32 rt_val;
	  ELMO_INT32 rt;
	  ELMO_INT32 nfds;  /* should be one more than the maximum of any file descriptor in any of the sets */
	  fd_set fdRead;

	  if (iSock == -1)
		  CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable_TCP' Is 'CMMCAmsTcpProtocol' Class initialized?", 0, 0, -1, MMC_LIB_TCP_ISREADABLE_INI_ERR, 0);
	
	  FD_ZERO(&fdRead);
	  FD_SET(iSock,&fdRead);
	  struct timeval tv;
	  struct timeval* ptv = &tv;
	  if (iTimeOut > 0)
	  {
		tv.tv_sec  = iTimeOut / 1000;
		tv.tv_usec = (iTimeOut % 1000) * 1000;
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
	
	  nfds = 0;
	  nfds = max(nfds, iSock) + 1;
	  // rt_val = select(FD_SETSIZE, &fdRead, 0, 0, ptv);
	  rt_val = select(nfds, &fdRead, 0, 0, ptv);
	  if (rt_val == MMCPP_SOCKERROR)
	  {
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		  printf("\n%s: %s: (TCP) select()=MMCPP_SOCKERROR, Wait for Data iTimeOut=%d(mSec) WSAGetLastError()=%d ", __FILE__, __func__, iTimeOut, WSAGetLastError());
	#else
	//#ifndef WIN32
		  printf("\n%s: %s: (TCP) select()=MMCPP_SOCKERROR, Wait for Data iTimeOut=%d(mSec) ", __FILE__, __func__, iTimeOut);
	#endif
	
		fflush(stdout); fflush(stderr);
	
		CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable_TCP' Error On select ", 0, 0, rt_val, MMC_LIB_TCP_ISREADABLE_SEL_ERR, 0);
		return 0;
	  }
	
	  rt = FD_ISSET(iSock,&fdRead);
	  return (rt != 0);
	} /* IsReadable_TCP */
	
	
	/*---------------------------------------------------
	 * public methods
	 */
	
	ELMO_INT32 CMMCAmsTcpProtocol::CreateSocketfnClbk_TCP(ELMO_UINT32 uiPort, ELMO_INT32 iMsgMaxSize) throw (CMMCException)
	{
		ELMO_INT32 rt_val;
		struct sockaddr_in address;
		ELMO_INT32 iOption = 1;
	
	
		if (_iServerSock != -1) //that is to say already created
		{
			perror("*** something wrong with _iServerSock (!= -1)");
			return MMCPP_RETERROR;
		}
	
		/*initialize object members*/
		_uiPort = uiPort;
		_iMsgMaxSize = (iMsgMaxSize < 0) ? AMS_MAX_SIZE_DEF_TCP : iMsgMaxSize;
	
	
		//create a master socket
		if( (_iServerSock = socket(AF_INET , SOCK_STREAM , 0)) == 0)
		{
			perror("socket failed");
			CMMCPPGlobal::Instance()->MMCPPThrow("'CreateSocketfnClbk_TCP' (TCP) socket ", 0, 0, -1, MMC_LIB_TCP_CRTCLBK_SOCKET_ERR, 0);
			return (MMCPP_RETERROR);
		}
		//set master socket to allow multiple connections , this is just a good habit, it will work without this
		if((rt_val = setsockopt(_iServerSock, SOL_SOCKET, SO_REUSEADDR, (ELMO_PINT8)&iOption, sizeof(iOption))) < 0 )
		{
			perror("setsockopt");
			CMMCPPGlobal::Instance()->MMCPPThrow("'CreateSocketfnClbk_TCP' failed in setsockopt", 0, 0, rt_val, MMC_LIB_TCP_CRTCLBK_OPT_ERR, 0);
			return (MMCPP_RETERROR);
		}
	
		//type of socket created
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons( _uiPort );
	
		// Assigns the 'address' to _iServerSock descriptor -> _iServerSock to localhost port
		if ((rt_val = bind(_iServerSock, (struct sockaddr *)&address, sizeof(address))) < 0)
		{
			perror("bind failed");
			this->CloseSock_TCP(_iServerSock);
			CMMCPPGlobal::Instance()->MMCPPThrow("'CreateSocketfnClbk_TCP' failed in 'bind'", 0, 0, rt_val, MMC_LIB_TCP_CRTCLBK_BIND_ERR, 0);
			return (MMCPP_RETERROR);
		}
	
		// Marks the _iServerSock as a passive socket, a socket that will be used to accept incoming connection requests using accept
		if ((rt_val = listen(_iServerSock, AMS_MAX_CLIENTS_TCP)) < 0)
		{
			perror("listen");
			this->CloseSock_TCP(_iServerSock);
			CMMCPPGlobal::Instance()->MMCPPThrow("'CreateSocketfnClbk_TCP' failed in 'listen'", 0, 0, rt_val, MMC_LIB_TCP_CRTCLBK_LSN_ERR, 0);
			return (MMCPP_RETERROR);
		}
	
		sleep(1);
	
		//accept the incoming connection
		_iAddrLen = sizeof(address);
		_bClbkThreadAlive = ELMO_TRUE;
		RunClbkThread ();
	
		printf("\n%s: %s: Aft call to RunClbkThread() ", __FILE__, __func__);
		fflush(stdout); fflush(stderr);
	
		return MMC_OK;
	}
	
	
	//#define MAX_iReportsLoopCounter	1
	#define MAX_iReportsLoopCounter	4
	
	void CMMCAmsTcpProtocol::ClbkThread()
	{
	//    int iActivity;
		ELMO_INT32	iReadNum;
		//int indx;
		ELMO_INT32	iReportsLoopCounter = 0;
	
		struct sockaddr_in address;
	//    fd_set fdsRead;
		struct timeval tv;
	//    struct timeval* ptv = &tv;
	
	//  unsigned char ucBuffer[_iMsgMaxSize]; //_iMsgMaxSize is previously set by Create;
	
		_gucBuffer = new ELMO_UINT8[_iMsgMaxSize];
	
	
		iReportsLoopCounter = 0;
		tv.tv_sec  = 5;
		tv.tv_usec = 0;
		//ptv = NULL;
	
	
		while (_bClbkThreadAlive)
		{
									//clear the socket set
	//        FD_ZERO(&fdsRead);
	
	//        //add master socket to set
	//        FD_SET(_iServerSock, &fdsRead);
	//
	//        FD_SET(_iSock, &fdsRead);
	
			if (iReportsLoopCounter < MAX_iReportsLoopCounter)
			{
				printf("\n%s: %s: _uiPort=%d Waiting for connections... (iReportsLoopCounter=%d Ver: %s %s) ", __FILE__, __func__, (int)this->_uiPort, iReportsLoopCounter, __DATE__, __TIME__);
				iReportsLoopCounter ++;
				fflush(stdout); fflush(stderr);
			}
	
	
	
	//												/* If timeout is specified as NULL, select() blocks indefinitely ptv=NULL   */
	//												/* waiting for a file descriptor to become ready.							*/
	//        iActivity = select(AMS_MAX_CLIENTS_TCP + 3, &fdsRead, NULL, NULL, ptv);
	//        if ((iActivity < 0) && (errno != EINTR))
	//        {
	//            printf("\n *** select error");
	//			fflush(stdout); fflush(stderr);
	//        }
	
	
	
	
			//If something happened on the master socket , then its an incoming connection
	//        if (FD_ISSET(_iServerSock, &fdsRead))
	//        {
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
			if ((_iSock = accept(_iServerSock, (struct sockaddr *)&address, &_iAddrLen)) < 0)
	#else
	//#ifndef WIN32
	   		if ((_iSock = accept(_iServerSock, (struct sockaddr *)&address, (socklen_t*)&_iAddrLen)) < 0)
	#endif
				{
					printf("\n%s: %s: *** accept error... errno=%d ", __FILE__, __func__, errno);
					_bClbkThreadAlive = ELMO_FALSE;
	
					break;
				}
	
				printf("\n Adding new Client Connection _iSock=%d ", _iSock);
				fflush(stdout); fflush(stderr);
	//        }
	
	
	
	
	
	//		if (FD_ISSET(_iSock, &fdsRead))
												/* Read & Send in loop, exit when EOF on read (by break) */
			for(;;)
			{
				errno = 0;
					// 	Some IO operation on some other socket :)
					//  check if it was for closing , and also read the incoming message
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
				iReadNum = recv(_iSock, (ELMO_PINT8)_gucBuffer, _iMsgMaxSize, 0);
				if (iReadNum <= 0)
				{
					getpeername(_iSock , (struct sockaddr*)&address , &_iAddrLen);
	#else
	//#ifndef WIN32
				iReadNum = read(_iSock,         _gucBuffer, _iMsgMaxSize   );
				if (iReadNum <= 0) //-1 means an error
				{
					getpeername(_iSock , (struct sockaddr*)&address , (socklen_t*)&_iAddrLen);
	#endif			
					printf("\n --- Recive()=%d, ip=%s , port=%d _iSock=%d errno=%d " , iReadNum, inet_ntoa(address.sin_addr) , ntohs(address.sin_port), _iSock, errno);
					fflush(stdout); fflush(stderr);
					   
					if (errno != 0)
					{
							// close the socket and mark as 0 in list for reuse
						this->CloseSock_TCP(_iSock);
							//???? _iSock = 0;
					   
						printf("\n Host disconnected iReadByn=%d - close the Socket... " , iReadNum);
						fflush(stdout); fflush(stderr);
					}
	
					break;
				}
				else
				{
					printf("\n Read Data From Socket: iReadNum=%d, _iSock=%d ", iReadNum, _iSock);
	
									/* send response for message that just has came in	*/
					SocketfnClbk(MMCPP_SOCK_READY_EVENT, _gucBuffer, iReadNum);
				}
	
			}   /* for(;;) */
	
		}	/* while (_bClbkThreadAlive) */
	
		delete _gucBuffer;
		printf("\n%s: %s: quit call-back mode thread. ", __FILE__, __func__);
		fflush(stdout); fflush(stderr);
		return;
	}
	
	
	/**
	 * defined as friend function of CMMCAmsTcpProtocol class, under protected
	 */
	//#ifndef WIN32
	#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)
		ELMO_PVOID fnTCPClbkThread(ELMO_PVOID pThis)
		{
			((CMMCAmsTcpProtocol*)pThis)->ClbkThread();
			return pThis;
		}
	#else

		ELMO_INT32 WINAPI fnTCPClbkThread(ELMO_PVOID lpParam)
		{
			((CMMCAmsTcpProtocol*)lpParam)->ClbkThread();
			return MMC_OK;
		}
	#endif
	
	
	ELMO_INT32 CMMCAmsTcpProtocol::RunClbkThread()
	{
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		DWORD ulThreadID;
		_thread = CreateThread(
				NULL,               // default security attributes
				0,                  // use default stack size
				fnTCPClbkThread,    // thread function name
				this,               // argument to thread function
				0,                  // use default creation flags
				&ulThreadID);       // returns the thread identifier
	#else
	//#ifndef WIN32
		pthread_attr_t attr;
	
		pthread_attr_init(&attr);               /* initializes a thread attributes object with the default value.*/
	
		/* creation of new thread for call-back mode of operation*/
		pthread_create(&_thread, &attr, fnTCPClbkThread, this);
	#endif
		return MMC_OK;
	}
	
	ELMO_INT32 CMMCAmsTcpProtocol::Close_TCP()
	{
		return this->CloseSock_TCP(_iServerSock);
	}
	
	/*! \fn int Close(int iSock);
	*   \brief close socket connection.
	*   \param iSock socket connection to close.
	*   \return 0 on success, error id otherwise.
	*/
	ELMO_INT32 CMMCAmsTcpProtocol::CloseSock_TCP(ELMO_INT32 iSock)
	{
		struct linger ling;
		if (iSock == -1)
			return MMC_OK;
	
		ling.l_onoff = 1;
		ling.l_linger = 0;
		setsockopt (iSock, SOL_SOCKET, SO_LINGER, (ELMO_PINT8)(&ling), sizeof (ling));

	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		closesocket(iSock);
	#else
	//#ifndef WIN32
		close(iSock);
	#endif
		if (iSock == _iServerSock)
			_iServerSock = -1;
		else
			_iSock = -1; //this client connection is closed;
	
		memset(&_saddConnect, 0, sizeof (_saddConnect));
		printf("\n%s: %s: Close iSock=%d ", __FILE__, __func__, iSock);
		fflush(stdout); fflush(stderr);
		return MMC_OK;
	}
	
	/*! \fn int Accept_TCP();
	*   \brief fetch new connection if exists.
	*   \return new new connection from socket listener if exists, otherwise -1.
	*/
	ELMO_INT32 CMMCAmsTcpProtocol::Accept_TCP()
	{
		struct sockaddr sadd;
		ELMO_INT32 rtErr;

	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		ELMO_INT32 iLen;
	#else
	//#ifndef WIN32
		socklen_t iLen;
	#endif
		ELMO_INT32 iSock;
		ELMO_ULINT32 ulRequest;
		ELMO_ULINT32 ulOption;
	
		iLen = sizeof (sadd);
		iSock = accept(_iServerSock, &sadd, &iLen);
		if (iSock < 0)
		{
	//#ifdef WIN32
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
			rtErr = iSock = -(WSAGetLastError());
	#else
			rtErr = errno;
	#endif
		}
		else
		{
			/* set non blocking mode */
			ulRequest = 1L;
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
			if (ioctlsocket (iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
	#else
	//#ifndef WIN32
			if (ioctl       (iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
	#endif
			{
				this->CloseSock_TCP(iSock);
				iSock = MMCPP_INVSOCKET;
			}
			else
			{
				ulOption = 1L;
				setsockopt(iSock, SOL_SOCKET,  SO_KEEPALIVE, (const char *)(&ulOption), sizeof (ulOption));
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
				setsockopt(iSock, IPPROTO_TCP, TCP_NODELAY,  (const char *)(&ulOption), sizeof (ulOption));
	#else
	//#ifndef WIN32
				setsockopt(iSock, SOL_TCP,     TCP_NODELAY,  (const char *)(&ulOption), sizeof (ulOption));
	#endif
			}
		}
	
		return iSock;
	}
	
	/*! \fn int Receive_TCP(int iSock, unsigned short usSize, void *pData);
	*   \brief receives data from given socket connection.
	*   \param iSock socket connection by which message is being read.
	*   \param usSize expected size (bytes) to read
	*   \param pData message buffer to read message into.
	*   \param bFail true if error, false otherwise.
	*   \return number of bytes actually received or -1 for error.
	*/
	ELMO_INT32 CMMCAmsTcpProtocol::Receive_TCP(ELMO_INT32 iSock, ELMO_UINT16 usSize, ELMO_PVOID pData, ELMO_LINT32 lDelayMilSec) throw (CMMCException)
	{
		ELMO_INT32  iRet;
		struct 		timeval tv;
		struct 		timeval* ptv = &tv;
		fd_set 		fdRead;
		ELMO_INT32  nfds;   /* should be one more than the maximum of any file descriptor in any of the sets */
	
		/* check if data available */
		FD_ZERO (&fdRead);
		FD_SET (iSock, &fdRead);
	
		/*set delay is milliseconds*/
		if (lDelayMilSec > 0)
		{
		  tv.tv_sec  = lDelayMilSec / 1000;
		  tv.tv_usec = (lDelayMilSec % 1000) * 1000;
		}
		else if (lDelayMilSec == 0)
		{
		  tv.tv_sec  = 0;
		  tv.tv_usec = 0;
		}
		else
		{ //-1 (<0)
		  ptv = NULL;  //wait for ever until data arrives.
		}
	
	
		nfds = 0;
		nfds = max(nfds, iSock) + 1;
		// iRet = select (FD_SETSIZE, &fdRead, NULL, NULL, ptv);
		iRet = select (nfds, &fdRead, NULL, NULL, ptv);
		if (iRet == MMCPP_SOCKERROR)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("'Receive_TCP' Error On select ", 0, 0, MMCPP_SOCKERROR, MMC_LIB_TCP_RECEIVE_SEL_ERR, 0);
			return iRet;
		}
		/* return 0 if nothing to get */
		if (iRet == 0 || !FD_ISSET (iSock, &fdRead))
		{
			return 0;
		}
		/* receive data */
		if (usSize > _iMsgMaxSize)
			 usSize = _iMsgMaxSize;
	
		//some data is ready for read otherwise we would not get to this stage.
		iRet = recv (iSock, (ELMO_PINT8)pData, usSize, 0); //if (iRet == 0)  socket closed
		return iRet;
	}
	
	/**! \fn Send_TCP(int iSock, unsigned short usSize, void * pData)
	 * \brief sends data on a non blocking client socket
	 * \param iSock (IN) client socket
	 * \param usSize IN) number of bytes to send
	 * \param pData pData (IN) pointer to data to send
	 * \param (OUT) set to TRUE on error: socket must be closed
	 * \return number of bytes actually sent or -1 for error
	 */
	ELMO_INT32 CMMCAmsTcpProtocol::Send_TCP(ELMO_INT32 iSock, ELMO_UINT16 usSize, ELMO_PVOID pData) throw (CMMCException)
	{
	  ELMO_INT32 iRet;
	
	  //  iRet = IsPending(iSock);
	  //  if (iRet != MMCPP_RETOK)
	  //  {
	  //    CMMCPPGlobal::Instance()->MMCPPThrow("'Send_TCP' failed in 'IsPending'", 0, 0, iRet, -1, 0);
	  //      return iRet;
	  //  }
	
		if (usSize > _iMsgMaxSize)
			 usSize = _iMsgMaxSize;
	
		errno = 0;
		iRet = send (iSock, (const ELMO_INT8 *)pData, usSize, MSG_NOSIGNAL);
		printf("\n Send Data On Socket: usSize=%d, iSock=%d iRet=%d errno=%d ", usSize, iSock, iRet, errno);
	
		return(iRet);
	}
	
	/**! \fn Connect_TCP(char* szAddr, unsigned short usPort, int& iSock,bool& bWait)
	 * \brief creates a non blocking socket connected to a remote server
	 * \param szAddr (IN) IP address of the server
	 * \param usPort (IN) Ethernet port number of the server
	 * \parm iSock (OUT) created socket if OK
	 * \param bWait (OUT) set to TRUE if connect is not fully completed
	 * \return: OK or ERROR
	 * \
	 * \While using for AMS generaly the Maestro is server so this function is not in use
	 * \
	 */
	ELMO_INT32 CMMCAmsTcpProtocol::Connect_TCP(ELMO_PINT8 szAddr, ELMO_UINT16 usPort, ELMO_INT32& iSock, ELMO_BOOL& bWait, int iMsgMaxSize) throw (CMMCException){
		ELMO_INT32 			iOpt;
		ELMO_ULINT32 		ulRequest;
		struct sockaddr_in	sadd;
		ELMO_INT32         	rt;
		ELMO_INT32     		errnoRt;
	
		_iSock = -1;
	
	//#ifdef WIN32
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
	// HH: When in Visual "Solution Explorer"-><Project Name Ex: MMCPP_Lib>->Properties->Configuration Properties->
	//      General->Character Set->"Use Unicode Character Set"... it cause define as char* if selected "NOT SET"
	//HH rt = InetPton (AF_INET, (PCWSTR)szAddr, &(sadd.sin_addr));
		 rt =InetPton (AF_INET, szAddr, &(sadd.sin_addr));
	#else
		rt = inet_pton(AF_INET, szAddr, &(sadd.sin_addr));
	#endif
		if (rt != MMCPP_RETERROR)
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_TCP' Illegal IP ", 0, 0, rt, MMC_LIB_TCP_CONNECT_IP_ERR, 0);
	
		_iMsgMaxSize = (iMsgMaxSize < 0) ? AMS_MAX_SIZE_DEF_TCP : iMsgMaxSize;
	
		bWait = ELMO_FALSE;
	
		/* create socket */
		iSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (iSock == MMCPP_INVSOCKET)
		{
			printf("\n *** %s: socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)=MMCPP_INVSOCKET ", __func__);
			fflush(stdout); fflush(stderr);
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_TCP' socket", 0, 0, MMCPP_INVSOCKET, MMC_LIB_TCP_CONNECT_SOCKET_ERR, 0);
			return MMCPP_RETERROR;
		}
	
		/* set non blocking mode */
		ulRequest = 1L;
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		if (ioctlsocket (iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
	#else
	//#ifndef WIN32
		if (ioctl       (iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
	#endif
		{
			printf("\n%s: %s: ioctl()=MMCPP_SOCKERROR ", __FILE__, __func__);
			fflush(stdout); fflush(stderr);
			this->CloseSock_TCP(iSock);
			iSock = MMCPP_INVSOCKET;
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_TCP' failed in 'ioctl'", 0, 0, MMCPP_SOCKERROR, MMC_LIB_TCP_CONNECT_CTRL_ERR, 0);
			return MMCPP_RETERROR;
		}
	
		/* Added 20101110 */
		iOpt = 1L;
		setsockopt (iSock, SOL_SOCKET, SO_KEEPALIVE, (const ELMO_INT8 *)(&iOpt), sizeof (iOpt));
		setsockopt (iSock, IPPROTO_TCP, TCP_NODELAY, (const ELMO_INT8 *)(&iOpt), sizeof (iOpt));
	
		/* set address and port */
		memset(&sadd, 0, sizeof (sadd));
		sadd.sin_family = AF_INET;
		sadd.sin_port = htons (usPort);
	//#ifdef WIN32
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		if( (sadd.sin_addr.s_addr = inet_addr(szAddr)) == INADDR_NONE) {
	#else
	
		if (inet_aton(szAddr, &sadd.sin_addr)==0) {
	#endif
			_iSock = iSock;
			CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_TCP' Is Ip valid ?", 0, 0, -1, MMC_LIB_TCP_CONNECT_IP_ERR, 0);
			return MMCPP_RETERROR;
	
		}
	
		/* try to connect */
		rt = connect (iSock, (const struct sockaddr *)(&sadd),sizeof (sadd));
		errnoRt = errno;
		if (rt ==  0)
		{
			_iSock = iSock;
			_saddConnect = sadd;
			return MMC_OK;
		}
	
		/* accept blocking error code */
	//#ifdef WIN32
	#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		if (WSAGetLastError() == WSAEWOULDBLOCK)
	#else      
		if (errno == EINPROGRESS)
	#endif
		{
			bWait = ELMO_TRUE;
			_saddConnect = sadd;
			_iSock = iSock;
			return MMC_OK;
		}
	
		bWait = ELMO_TRUE;
		this->CloseSock_TCP(iSock);
		iSock = MMCPP_INVSOCKET;
		CMMCPPGlobal::Instance()->MMCPPThrow("'Connect_TCP' ", 0, 0, -1, MMC_LIB_TCP_CONNECT_CONN_ERR, 0);
		return MMCPP_RETERROR;
	}
	
	/**! \fn bool IsWritable_TCP(int iSock);
	 * \brief checks for errors and whether or not connection is ready for write operation.
	 * \param iSock client socket connection to check.
	 * return true if writable, otherwise false.
	 */
	ELMO_BOOL CMMCAmsTcpProtocol::IsWritable_TCP(ELMO_INT32 iSock) throw (CMMCException)
	{
		if ((_saddConnect.sin_family == 0) && (_saddConnect.sin_port == 0))
			CMMCPPGlobal::Instance()->MMCPPThrow("'IsWritable_TCP' Is MMCTCP connection already established? ", 0, 0, -1, MMC_LIB_TCP_ISWRITABLE_INI_ERR, 0);
	
		return (this->IsPending(iSock)) ? ELMO_FALSE : ELMO_TRUE;
	}
	
										 // milliseconds
	ELMO_BOOL CMMCAmsTcpProtocol::IsReadable_TCP(ELMO_INT32 iSock, ELMO_INT32 iTimeOut) throw (CMMCException)
	{
	
		if ((_saddConnect.sin_family == 0) && (_saddConnect.sin_port == 0))
			CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable_TCP' Is MMCTCP connection already established? ", 0, 0, -1, MMC_LIB_TCP_ISREADABLE_INI_ERR, 0);
	
	//  int connectRt;
	//  int errnoRt;
	//
	//  connectRt = connect (iSock, (const struct sockaddr *)(&_saddConnect), sizeof (_saddConnect));
	//  errnoRt = errno;
	//
	//      // EALREADY    => A connection request is already in progress for the specified socket. 
	//      // EINPROGRESS => O_NONBLOCK is set for the file descriptor for the socket and the connection
	//      //                cannot be immediately established; the connection shall be established asynchronously. 
	//      // EISCONN     => Transport endpoint is already connected.
	//      // In this state connection shuld be establish...
	//  if ((connectRt != EALREADY) && (connectRt != EINPROGRESS) && (errnoRt != EISCONN))
	//  {
	//      CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable_TCP' Is MMCTCP connection already establish? ", 0, 0, -1, -1, 0);
	//      return MMCPP_RETERROR;
	//  }
	
		return ::IsReadable_TCP(iSock, iTimeOut);
	}
	
	
	/*---------------------------------------------------
	 * protected methods
	 */
	
	/**! \fn int IsPending(int iSock)
	 * \brief check pending connection on a non blocking socket
	 *  actuall checks for errors and whether or not connection is ready for write operation.
	 * \param iSock socket connection to check.
	 * \return: OK if iSock ready for write operation or ERROR otherwise (still pending for instance)
	 */
	ELMO_INT32 CMMCAmsTcpProtocol::IsPending(ELMO_INT32 iSock) throw (CMMCException)
	{
		ELMO_INT32     	iRet;
		struct  timeval tDelay;
		fd_set  		fdWrite, fdCheck;
		ELMO_INT32      nfds;   /* should be one more than the maximum of any file descriptor in any of the sets */
	
		if (iSock == -1)
			CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' Is 'CMMCAmsTcpProtocol' Class initialized?", 0, 0, -1, MMC_LIB_TCP_ISPENDING_INI_ERR, 0);
	
		/* check if sockets is ready writing (fdWrite),
		 * or have an exceptional condition pending ( fdCheck ).
		 * */
		FD_ZERO (&fdWrite);
		FD_SET (iSock, &fdWrite);
		FD_ZERO (&fdCheck);
		FD_SET (iSock, &fdCheck);
		tDelay.tv_sec = tDelay.tv_usec = 0L; /* dont wait */
		nfds = 0;
		nfds = max(nfds, iSock) + 1;
		iRet = select (nfds, NULL, &fdWrite, &fdCheck, &tDelay);
		if (iRet == MMCPP_SOCKERROR)
		{
			CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' (TCP) failed in select ", 0, 0, MMCPP_SOCKERROR, MMC_LIB_TCP_ISPENDING_SEL_ERR, 0);
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
	
											/* From MMCTCP ???? */
											/* From MMCTCP ???? */

#endif /* #if   (OS_PLATFORM == LINUXIPC32_PLATFORM) */

