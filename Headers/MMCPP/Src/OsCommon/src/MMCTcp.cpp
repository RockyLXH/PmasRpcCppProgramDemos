/*
 * CMMCTCP.cpp
 *
 *  Created on: 16/12/2012
 *      Author: yuvall
 *      Update: HaimH 20Jan2014 Add exception.
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */



#include "OS_PlatformDependSetting.hpp"
#include "MMCTcp.hpp"

#ifdef PROAUT_CHANGES
  #if defined(_WIN64)
    // Microsoft Windows (64-bit)
    #undef _WIN64_WINNT
    #define _WIN64_WINNT 0x0600
	#include <winsock2.h>
    #include <Ws2tcpip.h>
	
    extern "C" {
      WINSOCK_API_LINKAGE  INT WSAAPI inet_pton( INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
    }
	
  #elif defined(_WIN32)
    // Microsoft Windows (32-bit)
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
	#include <winsock2.h>
    #include <Ws2tcpip.h>

    extern "C" {
      WINSOCK_API_LINKAGE  INT WSAAPI inet_pton( INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
    }
	
  #elif defined(__linux__)
    // -- Linux --
  #endif
#endif

#ifndef  MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif


#undef  max
#define max(x,y) ((x) > (y) ? (x) : (y))
static ELMO_INT32 IsReadable(ELMO_INT32 iSock, ELMO_INT32 iTimeOut) throw (CMMCException) { // milliseconds
  ELMO_INT32 rt_val;
  ELMO_INT32   rt;
  ELMO_INT32    nfds;  /* should be one more than the maximum of any file descriptor in any of the sets */
  fd_set fdRead;
  if (iSock == -1)
      CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable' Is 'CMMCTCP' Class initialized?", 0, 0, -1, MMC_LIB_TCP_ISREADABLE_INI_ERR, 0);

  FD_ZERO(&fdRead);
  FD_SET(iSock,&fdRead);
  struct timeval tv;
  struct timeval* ptv = &tv;
  if (iTimeOut > 0) {
    tv.tv_sec  = iTimeOut / 1000;
    tv.tv_usec = (iTimeOut % 1000) * 1000;
  } else if (iTimeOut == 0) {
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
  } else {
      ptv = NULL;  //wait for ever until data arrives.
  }

  nfds = 0;
  nfds = max(nfds, iSock) + 1;
  // rt_val = select(FD_SETSIZE, &fdRead, 0, 0, ptv);
  rt_val = select(nfds, &fdRead, 0, 0, ptv);
  if (rt_val == MMCPP_SOCKERROR) {
    CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable' (TCP) Error On select ", 0, 0, rt_val, MMC_LIB_TCP_ISREADABLE_SEL_ERR, 0);

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    fprintf(stderr, /*"%s*/"IsReadable: (TCP) Wait for Data Timeout %d, \n", /*__func__,*/ WSAGetLastError());
#else
    fprintf(stderr, /*"%s*/"IsReadable: (TCP) Wait for Data Timeout, tv_usec = %d\n", /*__func__,*/ iTimeOut);
#endif
    return 0;
  }

  rt = FD_ISSET(iSock,&fdRead);
  return (rt != 0);
} /* IsReadable */


/*---------------------------------------------------
 * public methods
 */

CMMCTCP::CMMCTCP(): _iServerSock(-1), _bClbkThreadAlive(false) {
    // TODO Auto-generated constructor stub

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    static ELMO_INT32 err = WSAStartup(MAKEWORD(2,0),&m_wsaData); //one-time initialization for a proccess.
    if(err != MMC_OK) {
        cout<<"Socket Initialization Error. Program aborted\n";
        return;
    }
#endif

    memset(&_saddConnect, 0, sizeof (_saddConnect));
    _iSock = -1;
}

CMMCTCP::~CMMCTCP() {
    // TODO Auto-generated destructor stub
    _bClbkThreadAlive = false; //quit call-back mode thread.
    if (_iServerSock!=-1) //if function as server
    {
        this->Close(_iServerSock); //close listener file descriptor.
    }
    if (_iSock!=-1)
    {
        this->Close(_iSock); //close client file descriptor.
    }
}

/*! \fn int Create(unsigned int uiPort, bool bClbk);
*   \brief creates singleton socket listener
*   creates singleton socket listener, which is invoked within dedicated thread
*   if bClbk is true, otherwise within the main thread.
*   \param uiPort socket listener port
*   \param usMaxConn max nuber of allowed connections
*   \param bClbk sets call-back mode of operation.
*   \return 0 on success, error id otherwise.
*/
ELMO_INT32 CMMCTCP::Create(ELMO_UINT32 uiPort, SOCK_CLBK fnClbk, ELMO_INT32 iMsgMaxSize) throw (CMMCException)
{

    ELMO_ULINT32 ulRequest;
    struct sockaddr_in sadd;

    if (_iServerSock != -1) //that is to say already created
        return MMCPP_RETERROR;

    /*initialize object members*/
    _uiPort = uiPort;
    _fnClbk = fnClbk;
    fprintf(stderr, "CMMCTCP::Create\n");

    _iMsgMaxSize = (iMsgMaxSize>MMCPP_TCP_MAX_SIZE || iMsgMaxSize < 0) ? MMCPP_TCP_MAX_SIZE : iMsgMaxSize;

    if (_fnClbk != NULL) {
        CreateClbk();
    }
    else
    {
        /* create socket */
        _iServerSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP/*6*/);
        if (_iServerSock == MMCPP_INVSOCKET)
        {
            fprintf(stderr, "Create: MMCPP_INVSOCKET, Line %d\n", __LINE__);
            CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (TCP) socket", 0, 0, MMCPP_INVSOCKET, MMC_LIB_TCP_CREATE_SOCKET_ERR, 0);
            return MMCPP_RETERROR;
        }

        /* set non blocking mode */
        ulRequest = 1L;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        if (ioctlsocket (_iServerSock, FIONBIO, (u_long *)&ulRequest) == MMCPP_SOCKERROR)
#else
        if (ioctl (_iServerSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
#endif
        {
            fprintf(stderr, "Create: ioctl - MMCPP_SOCKERROR \n");
            this->Close(_iServerSock);
            CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (TCP) fail in 'ioctl' ", 0, 0, MMCPP_SOCKERROR, MMC_LIB_TCP_CREATE_OPT_ERR, 0);
            return MMCPP_RETERROR;
        }

        /* set address and port */
        memset(&sadd, 0, sizeof (sadd));
        sadd.sin_family = AF_INET;
        sadd.sin_addr.s_addr = INADDR_ANY;
        sadd.sin_port = htons (uiPort);

        /* bind */
        if (bind (_iServerSock, (struct sockaddr *)(&sadd), sizeof (sadd)) == -1)
        {
            fprintf(stderr, "Create: bind error \n");
            this->Close(_iServerSock);
            CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (TCP) fail in 'bind' ", 0, 0, -1, MMC_LIB_TCP_CREATE_BIND_ERR, 0);
            return MMCPP_RETERROR;
        }

        /* listen */
        if (listen (_iServerSock, MMCPP_TCP_MAX_CLIENTS) == MMCPP_SOCKERROR)
        {
            fprintf(stderr, "Create: listen MMCPP_SOCKERROR \n");
            this->Close(_iServerSock);
            return MMCPP_RETERROR;
        }
    }
    return MMC_OK;
}

ELMO_INT32 CMMCTCP::CreateClbk() throw (CMMCException)
{
    ELMO_INT32 rt_val;
    struct sockaddr_in address;
    ELMO_INT32 iOption = 1;

    //set of socket descriptors

    //initialise all client_socket[] to 0 so not checked
    for (ELMO_INT32 i = 0; i < MMCPP_TCP_MAX_CLIENTS; i++)
    {
        _iClientConnectionsArr[i] = 0;
    }
    //create a master socket
    if( (_iServerSock = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        CMMCPPGlobal::Instance()->MMCPPThrow("'CreateClbk' (TCP) socket ", 0, 0, -1, MMC_LIB_TCP_CRTCLBK_SOCKET_ERR, 0);
        return (MMCPP_RETERROR);
    }
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if((rt_val = setsockopt(_iServerSock, SOL_SOCKET, SO_REUSEADDR, (ELMO_PINT8)&iOption, sizeof(iOption))) < 0 )
    {
        perror("setsockopt");
        CMMCPPGlobal::Instance()->MMCPPThrow("'CreateClbk' failed in setsockopt", 0, 0, rt_val, MMC_LIB_TCP_CRTCLBK_OPT_ERR, 0);
        return (MMCPP_RETERROR);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( _uiPort );

    //bind the socket to localhost port _uiPort
    if ((rt_val = bind(_iServerSock, (struct sockaddr *)&address, sizeof(address)))<0)
    {
        perror("bind failed");
        this->Close(_iServerSock);
        CMMCPPGlobal::Instance()->MMCPPThrow("'CreateClbk' failed in 'bind'", 0, 0, rt_val, MMC_LIB_TCP_CRTCLBK_BIND_ERR, 0);
        return (MMCPP_RETERROR);
    }
    //try to specify maximum of 3 pending connections for the master socket
    if ((rt_val = listen(_iServerSock, MMCPP_TCP_MAX_CLIENTS)) < 0)
    {
        perror("listen");
        this->Close(_iServerSock);
        CMMCPPGlobal::Instance()->MMCPPThrow("'CreateClbk' failed in 'listen'", 0, 0, rt_val, MMC_LIB_TCP_CRTCLBK_LSN_ERR, 0);
        return (MMCPP_RETERROR);
    }

    //accept the incoming connection
    _iAddrLen = sizeof(address);
    _bClbkThreadAlive = true;
    RunClbkThread ();

    return MMC_OK;
}

void CMMCTCP::ClbkThread()
{
    ELMO_INT32 iNewSock = -1;
    ELMO_INT32 iActivity, i , iReadNum , iSock;
    struct sockaddr_in address;
    fd_set fdsRead;

//    unsigned char ucBuffer[_iMsgMaxSize]; //_iMsgMaxSize is previously set by Create;
    ELMO_PUINT8 ucBuffer = new ELMO_UINT8[_iMsgMaxSize];

    fprintf(stderr,"ClbkThread: _uiPort = %d\n", (ELMO_INT32)this->_uiPort);

    puts("Waiting for connections...");

    while(_bClbkThreadAlive)
    {
        //clear the socket set
        FD_ZERO(&fdsRead);

        //add master socket to set
        FD_SET(_iServerSock, &fdsRead);
        //add child sockets to set
        for ( i = 0 ; i < MMCPP_TCP_MAX_CLIENTS ; i++)
        {
            iSock = _iClientConnectionsArr[i];
            if(iSock > 0)
            {
                FD_SET( iSock , &fdsRead);
            }
        }
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        iActivity = select( MMCPP_TCP_MAX_CLIENTS + 3 , &fdsRead , NULL , NULL , NULL);

        if ((iActivity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(_iServerSock, &fdsRead))
        {

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
            if ((iNewSock = accept(_iServerSock, (struct sockaddr *)&address, &_iAddrLen))<0)
#else
            if ((iNewSock = accept(_iServerSock, (struct sockaddr *)&address, (socklen_t*)&_iAddrLen))<0)
#endif

            {
                perror("accept");
                _bClbkThreadAlive = false;
                _fnClbk(iNewSock, MMCPP_SOCK_ABORT_EVENT, ucBuffer, 0);
                break;
            }
            //add new socket to array of sockets
            for (i = 0; i < MMCPP_TCP_MAX_CLIENTS; i++)

            {
                iSock = _iClientConnectionsArr[i];
                if (iSock == 0)
                {
                    _iClientConnectionsArr[i] = iNewSock;
                    printf("Adding to list of sockets as %d\n" , i);
                    i = MMCPP_TCP_MAX_CLIENTS;
                    _fnClbk(iNewSock, MMCPP_SOCK_NEW_EVENT, ucBuffer, 0); //
                }
            }

        }
        //else its some IO operation on some other socket :)
        for (i = 0; i < MMCPP_TCP_MAX_CLIENTS; i++)

        {
            iSock = _iClientConnectionsArr[i];
            if (FD_ISSET( iSock , &fdsRead))
            {
                //check if it was for closing , and also read the incoming message
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
                if ((iReadNum = recv( iSock, (ELMO_PINT8)ucBuffer, _iMsgMaxSize, 0)) <= 0)
#else
                if ((iReadNum = read( iSock, ucBuffer, _iMsgMaxSize)) <= 0) //-1 means an error
#endif
                {
                    //TODO: call to user callback with socket connection and event type close.
                    _fnClbk(iSock, MMCPP_SOCK_CLOSED_EVENT, ucBuffer, iReadNum);

                    //Somebody disconnected , get his details and print
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
                    getpeername(iSock , (struct sockaddr*)&address , &_iAddrLen);
#else
                    getpeername(iSock , (struct sockaddr*)&address , (socklen_t*)&_iAddrLen);
#endif

                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    //close the socket and mark as 0 in list for reuse
                    this->Close( iSock );
                    _iClientConnectionsArr[i] = 0;
                }
                //send back the response for message that just has came in
                else
                {
                    _fnClbk(iSock, MMCPP_SOCK_READY_EVENT, ucBuffer, iReadNum); //
                }
            }

        }
    }
    delete ucBuffer;
    fprintf(stderr, "ClbkThread: quit call-back mode thread.\n");
    return;
}

/**
 * defined as friend function of CMMCTCP class, under protected
 */
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    DWORD WINAPI fnTCPClbkThread( LPVOID lpParam )
    {
        ((CMMCTCP*)lpParam)->ClbkThread();
        return MMC_OK;
    }
#else
    ELMO_PVOID fnTCPClbkThread(ELMO_PVOID pThis)
    {
        ((CMMCTCP*)pThis)->ClbkThread();
        return pThis;
    }
#endif

ELMO_INT32 CMMCTCP::RunClbkThread()
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

    pthread_attr_t attr;
//    struct sched_param sp;

    pthread_attr_init(&attr);               /* initializes a thread attributes object with the default value.*/
//    pthread_attr_getschedparam(&attr,&sp);    /* get the scheduling parameter default attributes */
//    if( getuid()==0)                      /* is this process run by root */
//    {
//      /* priority for time slice */
//        sp.sched_priority = 60;
//        pthread_attr_setschedparam(&attr,&sp);
//        pthread_attr_setschedpolicy(&attr, SCHED_RR);
//    }

    /* creation of new thread for call-back mode of operation*/
    pthread_create(&_thread, &attr, fnTCPClbkThread, this);
#endif

    return MMC_OK;
}

ELMO_INT32 CMMCTCP::Close()
{
    return this->Close(_iServerSock);
}

/*! \fn int Close(int iSock);
*   \brief close socket connection.
*   \param iSock socket connection to close.
*   \return 0 on success, error id otherwise.
*/
ELMO_INT32 CMMCTCP::Close(ELMO_INT32 iSock)
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
    close(iSock);
#endif

    if (iSock == _iServerSock)
        _iServerSock = -1;
    else
        _iSock = -1; //this client connection is closed;

    memset(&_saddConnect, 0, sizeof (_saddConnect));
    fprintf(stderr, "Close: iSock %d is closed\n", iSock);
    return MMC_OK;
}

/*! \fn int Accept();
*   \brief fetch new connection if exists.
*   \return new new connection from socket listener if exists, otherwise -1.
*/
ELMO_INT32 CMMCTCP::Accept()
{
    struct sockaddr sadd;
//    ELMO_INT32 rtErr;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iLen;
#else
    socklen_t iLen;
#endif

    ELMO_INT32 iSock;
    ELMO_ULINT32 ulRequest;
    ELMO_ULINT32 ulOption;

    iLen = sizeof (sadd);
    iSock = accept (_iServerSock, &sadd, &iLen);
    if (iSock < 0)
    {

//#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
//        rtErr = iSock = -(WSAGetLastError());
//#else
//        rtErr = errno;
//#endif

    }
    else
    {
        /* set non blocking mode */
        ulRequest = 1L;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        if (ioctlsocket (iSock, FIONBIO, (u_long *)&ulRequest) == MMCPP_SOCKERROR)
#else
        if (ioctl (iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
#endif

        {
            this->Close(iSock);
            iSock = MMCPP_INVSOCKET;
        }
        else
        {
            ulOption = 1L;
            setsockopt (iSock, SOL_SOCKET, SO_KEEPALIVE,
                    (const ELMO_INT8*)(&ulOption), sizeof (ulOption));

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
            setsockopt (iSock, IPPROTO_TCP, TCP_NODELAY,
                    (const ELMO_INT8*)(&ulOption), sizeof (ulOption));
#else
            setsockopt (iSock, SOL_TCP, TCP_NODELAY,
                    (const ELMO_INT8*)(&ulOption), sizeof (ulOption));
#endif
        }
    }

    return iSock;
}

/*! \fn int Receive(int iSock, unsigned short usSize, void *pData);
*   \brief receives data from given socket connection.
*   \param iSock socket connection by which message is being read.
*   \param usSize expected size (bytes) to read
*   \param pData message buffer to read message into.
*   \param bFail true if error, false otherwise.
*   \return number of bytes actually received or -1 for error.
*/
ELMO_INT32 CMMCTCP::Receive(ELMO_INT32 iSock, ELMO_UINT16 usSize, ELMO_PVOID pData, ELMO_LINT32 lDelayMilSec) throw (CMMCException)
{
    ELMO_INT32     iRet;
    struct timeval tv;
    struct timeval* ptv = &tv;
    fd_set fdRead;
    ELMO_INT32     nfds;   /* should be one more than the maximum of any file descriptor in any of the sets */

    /* check if data available */
    FD_ZERO (&fdRead);
    FD_SET (iSock, &fdRead);

    /*set delay is milliseconds*/
    if (lDelayMilSec > 0) {
      tv.tv_sec  = lDelayMilSec / 1000;
      tv.tv_usec = (lDelayMilSec % 1000) * 1000;
    } else if (lDelayMilSec == 0) {
      tv.tv_sec  = 0;
      tv.tv_usec = 0;
    } else { //-1 (<0)
      ptv = NULL;  //wait for ever until data arrives.
    }


    nfds = 0;
    nfds = max(nfds, iSock) + 1;
    // iRet = select (FD_SETSIZE, &fdRead, NULL, NULL, ptv);
    iRet = select (nfds, &fdRead, NULL, NULL, ptv);
    if (iRet == MMCPP_SOCKERROR)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'Receive' (TCP) Error On select ", 0, 0, MMCPP_SOCKERROR, MMC_LIB_TCP_RECEIVE_SEL_ERR, 0);
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

/**! \fn Send (int iSock, unsigned short usSize, void * pData)
 * \brief sends data on a non blocking client socket
 * \param iSock (IN) client socket
 * \param usSize IN) number of bytes to send
 * \param pData pData (IN) pointer to data to send
 * \param (OUT) set to TRUE on error: socket must be closed
 * \return number of bytes actually sent or -1 for error
 */
ELMO_INT32 CMMCTCP::Send (ELMO_INT32 iSock, ELMO_UINT16 usSize, ELMO_PVOID pData) throw (CMMCException)
{
  //  ELMO_INT32 iRet;

  //  iRet = IsPending(iSock);
  //  if (iRet != MMCPP_RETOK)
  //  {
  //    CMMCPPGlobal::Instance()->MMCPPThrow("'Send' failed in 'IsPending'", 0, 0, iRet, -1, 0);
  //      return iRet;
  //  }

    if (usSize > _iMsgMaxSize)
        usSize = _iMsgMaxSize;

    return send (iSock, (const ELMO_INT8*)pData, usSize, MSG_NOSIGNAL);
}

/**! \fn Connect(char* szAddr, unsigned short usPort, int& iSock,bool& bWait)
 * \brief creates a non blocking socket connected to a remote server
 * \param szAddr (IN) IP address of the server
 * \param usPort (IN) Ethernet port number of the server
 * \parm iSock (OUT) created socket if OK
 * \param bWait (OUT) set to TRUE if connect is not fully completed
 * \return: OK or ERROR
 */
ELMO_INT32 CMMCTCP::Connect(ELMO_PINT8 szAddr, ELMO_UINT16 usPort, ELMO_INT32& iSock, ELMO_BOOL& bWait, ELMO_INT32 iMsgMaxSize) throw (CMMCException){
    ELMO_INT32 iOpt;
    ELMO_ULINT32 ulRequest;
    struct sockaddr_in sadd;
    ELMO_INT32         rt;
//    ELMO_INT32     errnoRt;

    _iSock = -1;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))

  #ifdef PROAUT_CHANGES
    #if defined(_WIN64) || defined(_WIN32)
    // Mingw for Microsoft Windows (64-bit) and Microsoft Windows (32-bit)    
	  rt = inet_pton (AF_INET, szAddr, &(sadd.sin_addr));
    #elif defined(__linux__)
    // -- Linux --
    #endif
  #else  
    //original code from ELMO:
    rt = InetPton (AF_INET, (PCTSTR)szAddr, &(sadd.sin_addr));  
  #endif

#else
    rt = inet_pton(AF_INET, szAddr, &(sadd.sin_addr));
#endif

    if (rt != MMCPP_RETERROR)
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' TCP Illegal IP ", 0, 0, rt, MMC_LIB_TCP_CONNECT_IP_ERR, 0);

    _iMsgMaxSize = (iMsgMaxSize>MMCPP_TCP_MAX_SIZE || iMsgMaxSize < 0) ? MMCPP_TCP_MAX_SIZE : iMsgMaxSize;

    bWait = false;

    /* create socket */
    iSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iSock == MMCPP_INVSOCKET) {
        fprintf(stderr, "Connect: MMCPP_INVSOCKET, Line %d\n", __LINE__);
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' (TCP) socket", 0, 0, MMCPP_INVSOCKET, MMC_LIB_TCP_CONNECT_SOCKET_ERR, 0);
        return MMCPP_RETERROR;
    }

    /* set non blocking mode */
    ulRequest = 1L;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    if (ioctlsocket (iSock, FIONBIO, (u_long *)&ulRequest) == MMCPP_SOCKERROR)
#else
    if (ioctl (iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
#endif
    {
        fprintf(stderr, "Connect: ioctl - MMCPP_SOCKERROR, Line %d\n", __LINE__);
        this->Close(iSock);
        iSock = MMCPP_INVSOCKET;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' (TCP) failed in 'ioctl'", 0, 0, MMCPP_SOCKERROR, MMC_LIB_TCP_CONNECT_CTRL_ERR, 0);
        return MMCPP_RETERROR;
    }

    /* Added 20101110 */
    iOpt = 1L;
    setsockopt (iSock, SOL_SOCKET, SO_KEEPALIVE, (const ELMO_INT8*)(&iOpt), sizeof (iOpt));
    setsockopt (iSock, IPPROTO_TCP, TCP_NODELAY, (const ELMO_INT8*)(&iOpt), sizeof (iOpt));

    /* set address and port */
    memset(&sadd, 0, sizeof (sadd));
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons (usPort);

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    if( (sadd.sin_addr.s_addr = inet_addr(szAddr)) == INADDR_NONE) {
#else
    if (inet_aton(szAddr, &sadd.sin_addr)==0) {
#endif

        _iSock = iSock;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' (TCP) Is Ip valid ?", 0, 0, -1, MMC_LIB_TCP_CONNECT_IP_ERR, 0);
        return MMCPP_RETERROR;

    }

    /* try to connect */
    rt = connect (iSock, (const struct sockaddr *)(&sadd),sizeof (sadd));
//    errnoRt = errno;
    if (rt ==  0)
    {
        _iSock = iSock;
        _saddConnect = sadd;
        return MMC_OK;
    }

    /* accept blocking error code */
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    if (WSAGetLastError() == WSAEWOULDBLOCK)
#else      
    if (errno == EINPROGRESS)
#endif
    {
//      fprintf(stderr, "Connect: EINPROGRESS, Line %d\n", __LINE__);
        bWait = true;
        _saddConnect = sadd;
        _iSock = iSock;
        return MMC_OK;
    }

    bWait = true;
    this->Close(iSock);
    iSock = MMCPP_INVSOCKET;
    CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' (TCP) ", 0, 0, -1, MMC_LIB_TCP_CONNECT_CONN_ERR, 0);
    return MMCPP_RETERROR;
}

/**! \fn bool IsWritable(int iSock);
 * \brief checks for errors and whether or not connection is ready for write operation.
 * \param iSock client socket connection to check.
 * return true if writable, otherwise false.
 */
ELMO_BOOL CMMCTCP::IsWritable(ELMO_INT32 iSock) throw (CMMCException)
{
    if ((_saddConnect.sin_family == 0) && (_saddConnect.sin_port == 0))
        CMMCPPGlobal::Instance()->MMCPPThrow("'IsWritable' (TCP) Is MMCTCP connection already established? ", 0, 0, -1, MMC_LIB_TCP_ISWRITABLE_INI_ERR, 0);

    return (this->IsPending(iSock)) ? false : true;
}

                                     // milliseconds
ELMO_BOOL CMMCTCP::IsReadable(ELMO_INT32 iSock, ELMO_INT32 iTimeOut) throw (CMMCException)
{

    if ((_saddConnect.sin_family == 0) && (_saddConnect.sin_port == 0))
        CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable' (TCP) Is MMCTCP connection already established? ", 0, 0, -1, MMC_LIB_TCP_ISREADABLE_INI_ERR, 0);

//  ELMO_INT32 connectRt;
//  ELMO_INT32 errnoRt;
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
//      CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable' (TCP) Is MMCTCP connection already establish? ", 0, 0, -1, -1, 0);
//      return MMCPP_RETERROR;
//  }

    return ::IsReadable(iSock, iTimeOut);
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
ELMO_INT32 CMMCTCP::IsPending(ELMO_INT32 iSock) throw (CMMCException)
{
    ELMO_INT32     iRet;
    struct timeval tDelay;
    fd_set fdWrite, fdCheck;
    ELMO_INT32     nfds;   /* should be one more than the maximum of any file descriptor in any of the sets */

    if (iSock == -1)
        CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' Is 'CMMCTCP' Class initialized?", 0, 0, -1, MMC_LIB_TCP_ISPENDING_INI_ERR, 0);

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
