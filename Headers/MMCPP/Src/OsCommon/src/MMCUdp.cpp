/*
 * CMMCUDP.cpp
 *
 *  Created on: 28/06/2011
 *      Author: yuvall
 *      Update: 20Jan2014 HaimH add exceptions.
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
	#ifndef PROAUT_CHANGES
		#pragma warning(disable:4800)
	#endif
#endif

#include "OS_PlatformDependSetting.hpp"
#include "MMCUdp.hpp"

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

CMMCUDP::CMMCUDP() {
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    static ELMO_INT32 err = WSAStartup(MAKEWORD(2,0),&m_wsaData); //one-time initialization for a proccess.
    if(err != 0) {
        cout<<"Socket Initialization Error. Program aborted\n";
        return;
    }
#endif
    _iSock=-1;
}

CMMCUDP::~CMMCUDP() {

    if (_iSock!=-1)

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        closesocket(_iSock);
#else
        close(_iSock);
#endif

    _bClbkThreadAlive = false;

}

/*! \fn int SetSocketTimeout(int  iMilliseconds)
* \brief this function set socket to block only for iMilliseconds on receive operation.
* \param iMilliseconds - timeout in ms to wait on receive.
* \return   0 on success, otherwise -1 as an error
*/
ELMO_INT32 CMMCUDP::SetSocketTimeout(ELMO_INT32  iMilliseconds)
{
   struct timeval tv;

    tv.tv_sec = iMilliseconds / 1000 ;
    tv.tv_usec = ( iMilliseconds % 1000) * 1000  ;

   return setsockopt (_iSock, SOL_SOCKET, SO_RCVTIMEO, (ELMO_PINT8)&tv, sizeof tv);
}

/*! \fn int Create (char * cIP, int iPort) throw (CMMCException)
* \brief this function creates the socket.
* \param cIP - IP address of driver.
* \param iPort - port for socket connection.
* \return  0 on success, otherwise -1.
*/
ELMO_INT32 CMMCUDP::Create (ELMO_PINT8 cIP, ELMO_INT32 iPort, ELMO_INT32 iGMASPort, ELMO_INT32 iMaxSize) throw (CMMCException) {
    ELMO_INT32 rc;
    struct sockaddr_in sockAddrGMAS;

    rc = MMC_OK;

    _iMsgMaxSize = (iMaxSize > MMCUDP_MaxSize || iMaxSize < 0) ? MMCUDP_MaxSize :iMaxSize;

    memset ((ELMO_PINT8) &m_sockAddrOut, 0, sizeof(m_sockAddrOut));
    m_sockAddrOut.sin_family = AF_INET;
    m_sockAddrOut.sin_port = htons(iPort);

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    if( (m_sockAddrOut.sin_addr.s_addr = inet_addr(cIP)) == INADDR_NONE) {
#else
    if (inet_aton(cIP, &m_sockAddrOut.sin_addr)==0) {
#endif
        rc = -1;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (Is valid IP?)", 0, 0, -1, MMC_LIB_UDP_CREATE_IP_ERR, 0);
        return rc;
    }
//
    if ((_iSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        rc = -1;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' socket (is too many?)", 0, 0, -1, MMC_LIB_UDP_CREATE_SOCKET_ERR, 0);
        return rc;
    }
//
    if (iGMASPort == 0)
    {
        rc = -1;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' Port (is 0?)", 0, 0, -1, MMC_LIB_UDP_PORT_ERR, 0);
        return rc;
    }
    // If user chose to open the socket on spcific port on GMAS - then continue here.
    /* set address and port */
    memset ((ELMO_PINT8) &sockAddrGMAS, 0, sizeof(sockAddrGMAS));

    sockAddrGMAS.sin_family = AF_INET;
    sockAddrGMAS.sin_addr.s_addr = INADDR_ANY;
    sockAddrGMAS.sin_port = htons (iGMASPort);

    /* bind */
    if (bind (_iSock, (struct sockaddr *)(&sockAddrGMAS), sizeof (sockAddrGMAS)) == -1)
    {

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        closesocket(_iSock);
#else
        close (_iSock);
#endif

        _iSock = -1;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' bind", 0, 0, -1, MMC_LIB_UDP_CREATE_BIND_ERR, 0);
        return -1;
    }

    return rc;
}

/*! \fn int SendTo(char* msg, short sLength) throw (CMMCException)
* \brief this function sends data via socket.
* \param msg - message buffer to send..
* \param sLength - length (bytes) of data to send. must not be more then buffer length.
* \clrRcvVuf - if false do not clear recive buf before the send - expected faster return.
* \return result>=0 on success, otherwise throws CMMCException or -1.
*/
    ELMO_INT32 CMMCUDP::SendTo(ELMO_PINT8 msg, ELMO_INT16 sLength, ELMO_BOOL clrRcvBuf)
    {
        ELMO_INT32 rc = 0;

    #if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        ELMO_PINT8 cClrBuff = new ELMO_INT8[_iMsgMaxSize];
    #else
        ELMO_INT8 cClrBuff[_iMsgMaxSize];
    #endif

        if (clrRcvBuf == true)
        {
            while ( (rc = ReceiveFrom(cClrBuff, _iMsgMaxSize, 1)) > 0 ) ;//clear buffer on target device
        }
    
    #if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        delete cClrBuff;
    #endif
    
        rc = sendto( _iSock, msg, sLength, 0, (const sockaddr*)&m_sockAddrOut, sizeof(m_sockAddrOut));
        return rc;
    }

/*! \fn int GetIP(char* msg, short sLength) throw (CMMCException)
* \brief This function returns the ip of the connection
* \return the IP address as character array.
*/
    in_addr CMMCUDP::GetIP() {//throw (CMMCException) {

    //_MMCPP_TRACE_INFO("CMMCUDP::GetIP OK");
    return m_sockAddrOut.sin_addr;
}

/*! \fn int Close ()
* \brief this function closes the socket.
* \return   return - 0 on success, otherwise throws CMMCException or -1 is returned.
*/
ELMO_INT32 CMMCUDP::Close() {

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    return (_iSock!=-1)? ::closesocket(_iSock) : MMC_OK;
#else
    return (_iSock!=-1)? ::close(_iSock) : MMC_OK;
#endif
}

/*-------------------------------------------------------------------------
* new API for UDP class basically to comply with TCP API
*  iTimeOut milliseconds
*/
static ELMO_INT32 IsReadable(ELMO_INT32 iSock, ELMO_INT32 iTimeOut) throw (CMMCException) {
  ELMO_INT32 rt_val;
  fd_set fdRead;
  if (iSock == -1)
      CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable' Is 'CMMCUDP' Class initialized?", 0, 0, -1, MMC_LIB_UDP_ISREADABLE_INI_ERR, 0);

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

  rt_val = select(FD_SETSIZE, &fdRead, 0, 0, ptv);
  if (rt_val == MMCPP_SOCKERROR)
  {
    CMMCPPGlobal::Instance()->MMCPPThrow("'IsReadable' (UDP) Error On select ", 0, 0, rt_val, MMC_LIB_UDP_ISREADABLE_SEL_ERR, 0);

    #if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        fprintf(stderr, /*"%s*/"IsReadable: Wait for Data Timeout %d, \n", /*__func__,*/ WSAGetLastError());
    #else
        fprintf(stderr, /*"%s*/"IsReadable: Wait for Data Timeout, tv_usec = %d\n", /*__func__,*/ iTimeOut);
    #endif
    
    return 0;
  }

  rt_val = FD_ISSET(iSock,&fdRead);
  return (rt_val != 0);
} /* IsReadable */

ELMO_BOOL CMMCUDP::IsReadable(ELMO_INT32 iTimeOut) { // milliseconds

    return (::IsReadable(_iSock, iTimeOut));
}

ELMO_INT32 CMMCUDP::IsReady()
{
    return ::IsReadable(_iSock, 0L);
}

/**\!fn int Create (unsigned short usPort, SOCK_CLBK fnClbk=NULL, int iMsgMaxSize=512)
 * \brief creates UDP none blocking server (listener)
 * \param usPort port number to listen on (or to bind with)
 * \param fnClbk user call-back function. relevant only for call-back mode of operation.
 * \param iSock reference to socket file (descriptor)
 * \param iMsgMaxSize largest possible message (in bytes). relevant only for call-back mode of operation.
 * \return 0 on success, -1 otherwise. socket number (iSock) is update on success, otherwise -1;
 */
ELMO_INT32 CMMCUDP::Create (ELMO_UINT16 usPort, SOCK_CLBK fnClbk, ELMO_INT32 iMsgMaxSize) throw (CMMCException)
{
    ELMO_ULINT32 ulRequest;
    ELMO_INT32 rt_val;
    ELMO_INT32 opt = 1;
    ELMO_INT32 rc;
    struct sockaddr_in saddr;

    _fnClbk = fnClbk;

    _iMsgMaxSize = (iMsgMaxSize > MMCUDP_MaxSize || iMsgMaxSize < 0) ? MMCUDP_MaxSize : iMsgMaxSize;

    _usPort = usPort;
    _iSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_iSock == MMCPP_INVSOCKET)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (UDP) socket ", 0, 0, MMCPP_INVSOCKET, MMC_LIB_UDP_CREATE_SOCKET_ERR, 0);
        return MMCPP_RETERROR;
    }

    if (_fnClbk) {
        RunClbkThread();
        return MMC_OK;
    }

    rt_val = setsockopt(_iSock, SOL_SOCKET, SO_REUSEADDR, (ELMO_PINT8)&opt, sizeof(opt));
    if( rt_val < 0 )
    {
        perror("setsockopt");
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (UDP) fail in 'setsockopt' ", 0, 0, rt_val, MMC_LIB_UDP_CREATE_OPT_ERR, 0);
        return (MMCPP_RETERROR);
    }

    /* set non blocking mode */
    ulRequest = 1L;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    if ((rc = ioctlsocket (_iSock, FIONBIO, (u_long *)&ulRequest)) == MMCPP_SOCKERROR)
    {
        closesocket(_iSock);
#else
    if ((rc = ioctl (_iSock, FIONBIO, &ulRequest)) == MMCPP_SOCKERROR)
    {
        perror("ioctl error");
        close (_iSock);
#endif

        _iSock = MMCPP_INVSOCKET;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (UDP) fail in 'ioctl' ", 0, 0, rc, MMC_LIB_UDP_CREATE_CTRL_ERR, 0);
        return MMCPP_RETERROR;
    }

    /* set address and port */
    memset(&saddr, 0, sizeof (saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons (usPort);

    /* bind */
    if (bind (_iSock, (struct sockaddr *)(&saddr), sizeof (saddr)) == MMCPP_SOCKERROR)
    {

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        closesocket(_iSock);
#else
        close (_iSock);
#endif

        _iSock = MMCPP_INVSOCKET;
        CMMCPPGlobal::Instance()->MMCPPThrow("'Create' (UDP) fail in 'bind' ", 0, 0, MMCPP_SOCKERROR, MMC_LIB_UDP_CREATE_BIND_ERR, 0);
        return MMCPP_RETERROR;
    }

    return MMC_OK;
}

/**\!fn int Send (void * pData, unsigned short usSize, sockaddr_in* pSockaddr)
 * \brief sends udp message pointed by pData.
 * \param pData (IN) pointer to data to send
 * \param usSize message size.
 * \param pSockaddr pointer to socket address. default is NULL.
 *        On call-back mode it may be pointed to the socket address with data from last receive.
 * \return the number sent, or -1 for errors.
 */
ELMO_INT32 CMMCUDP::Send (ELMO_PVOID pData, ELMO_UINT16 usSize, sockaddr_in* pSockaddr) throw (CMMCException)
{
    ELMO_INT32 iRet;
    sockaddr_in *psaddr = (pSockaddr)?pSockaddr:&m_sockAddrOut;
    if (_iSock == MMCPP_INVSOCKET)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'Send' (Is Initialize?)", 0, 0, -1, MMC_LIB_UDP_SEND_SOCKINV_ERR, 0);
        return MMCPP_RETERROR;
    }

    if (usSize > _iMsgMaxSize)
        usSize = _iMsgMaxSize;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    iRet = sendto (_iSock, (const ELMO_INT8*)pData, usSize, 0, (const sockaddr*)psaddr, sizeof (struct sockaddr));
#else
    iRet = sendto (_iSock, pData, usSize, MSG_NOSIGNAL, (const sockaddr*)psaddr, sizeof (struct sockaddr));
#endif

    return iRet;
}

/*! \fn int RecvFrom(char* msg, short sLength, short sTimeoutMS) throw (CMMCException)
* \brief this function receives data from socket.
* \param msg - message buffer to receive data..
* \param sLength - length to read. must not be more then buffer length.
* \param sTimeoutMS - milliseconds to wait on receive.
* \return   return - result>=0 on success, otherwise throws CMMCException or result<0.
*/
ELMO_INT32 CMMCUDP::ReceiveFrom(ELMO_PINT8 msg, ELMO_INT16 sLength, ELMO_INT16 sTimeoutMS) throw (CMMCException) {
    ELMO_INT32 rc;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 rtErr;
#endif

    rc = SetSocketTimeout(sTimeoutMS);
    if (rc != MMC_OK)
        return (-1);

//  rc = recv(_iSock, msg,  sLength, MSG_PEEK);
//  if (rc < 0)
//  {
//#ifdef WIN32
//  rtErr = WSAGetLastError();
//  if (rtErr == WSAEMSGSIZE)
//      rc = sLength;
//  else
//#else      
//  rtErr = errno;
//#endif
//      return (-1);
//  }

    rc = recv(_iSock, msg,  sLength, 0);
    if (rc <= 0)
    {

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    rtErr = WSAGetLastError();
    if (rtErr == WSAEMSGSIZE)
        rc = sLength;
    else
#else      
//    rtErr = errno;
#endif

        return (-1);
    }

    return(rc);
}

/**\!fn int Receive (void * pData, unsigned short usSize, long lDelay, sockaddr_in* pSockaddr)
 * \brief receives UDP message pointed by pData.
 * \param pData (IN) pointer to buffer, which will store the received data.
 * \param usSize message size to read.
 * \param pSockaddr pointer to socket address. default is NULL.
 *        On call-back mode it shell be delivered to Send by call-back function for synchronous matters.
 * \return the number of bytes read or -1 for errors.
 */
ELMO_INT32 CMMCUDP::Receive (ELMO_PVOID pData, ELMO_UINT16 usSize, ELMO_INT32 lDelay, sockaddr_in* pSockaddr) throw (CMMCException)
{
    ELMO_INT32 iRet;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    ELMO_INT32 iAddrSize;
#else
    socklen_t iAddrSize;
#endif

    sockaddr_in *psaddr = (pSockaddr)?pSockaddr:&m_sockAddrOut;

    if (_iSock == MMCPP_INVSOCKET)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'Receive' (Is init?) ", 0, 0, -1, MMC_LIB_UDP_RECEIVE_SOCKINV_ERR, 0);
        return MMCPP_RETERROR;
    }

    if(!::IsReadable(_iSock, lDelay))
    {
        return 0;
    }
    memset(psaddr, 0x0, sizeof(sockaddr_in));
    iAddrSize = sizeof (struct sockaddr_in);
    if (usSize > _iMsgMaxSize)
        usSize = _iMsgMaxSize;
    iRet = recvfrom (_iSock, (ELMO_PINT8)pData, usSize, 0, (struct sockaddr *)psaddr, &iAddrSize);
    //((ELMO_PINT8)pData)[iRet]=0; //what if iRet is -1 or size of pData is exactly usSize
    //printf("Receive: %s, size = %d, iAddrSize = %d\n", (ELMO_PINT8)pData, iRet, iAddrSize);
    return iRet;
}

/**! \fn Connect(char* szAddr, unsigned short usPort, int& iSocket,bool& bWait)
 * \brief creates a non blocking socket connected to a remote server
 * \param szAddr (IN) IP address of the server
 * \param usPort (IN) Ethernet port number of the server
 * \parm iSocket (OUT) created socket if OK
 * \param bWait (OUT) set to TRUE if connect is not fully performed
 * \return: OK or ERROR
 */
ELMO_INT32 CMMCUDP::Connect (ELMO_PINT8 szAddr, ELMO_UINT16 usPort, ELMO_BOOL & bWait, ELMO_INT32 iMsgMaxSize) throw (CMMCException)
{
            ELMO_ULINT32 ulRequest;
    struct  sockaddr_in sadd;
            ELMO_INT32   rt;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
// HH: When in Visual "Solution Explorer"-><Project Name Ex: MMCPP_Lib>->Properties->Configuration Properties->
//      General->Character Set->"Use Unicode Character Set"... it define as char* if selected "NOT SET"

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
    if (rt != 1)
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' UDP Illegal IP ", 0, 0, rt, MMC_LIB_UDP_CONNECT_IP_ERR, 0);

    _iMsgMaxSize = (iMsgMaxSize > 0)?iMsgMaxSize:128;
    _iSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_iSock == MMCPP_INVSOCKET) {
        fprintf(stderr, /*"%s*/"Connect: socket error \n" /*__func__,*/);
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' (UDP) socket (is too many?)", 0, 0, MMCPP_INVSOCKET, MMC_LIB_UDP_CONNECT_SOCKET_ERR, 0);
        return MMCPP_RETERROR;
    }

    /* set non blocking mode */
    ulRequest = 1L;

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    if (ioctlsocket (_iSock, FIONBIO, (u_long *)&ulRequest) == MMCPP_SOCKERROR)
    {
        closesocket(_iSock);
#else
    if (ioctl (_iSock, FIONBIO, &ulRequest) == MMCPP_SOCKERROR)
    {
        close (_iSock);
#endif

        fprintf(stderr, /*"%s*/"Connect: ioctl error \n"/*, __func__*/);
        CMMCPPGlobal::Instance()->MMCPPThrow("'Connect' (UDP) faile on 'ioctl'", 0, 0, MMCPP_SOCKERROR, MMC_LIB_UDP_CONNECT_CTRL_ERR, 0);
        _iSock = MMCPP_INVSOCKET;
        return MMCPP_RETERROR;
    }

    /* set address and port */
    memset(&m_sockAddrOut, 0, sizeof(m_sockAddrOut));
    m_sockAddrOut.sin_family      = AF_INET;
    m_sockAddrOut.sin_port        = htons(usPort);
    m_sockAddrOut.sin_addr.s_addr = inet_addr(szAddr);
                /* Wait is false (no needs wait) while port is writeable */
    bWait = ! IsWritable();
    return MMC_OK;
}

/**! \fn bool IsWritable(int iSock);
* \brief checks for errors and whether or not connection is ready for write operation.
* \param iSock client socket connection to check.
* return true if writable, otherwise false.
*/
ELMO_BOOL CMMCUDP::IsWritable()
{
    return (this->IsPending(_iSock)) ? false : true;
}

/**! \fn int IsPending(int iSock, bool& bFail)
* \brief check pending connection on a non blocking socket
*  actuall checks for errors and whether or not connection is ready for write operation.
* \param iSock client socket connection to check.
* \param bFail true if error(socket must be closed then), false otherwise.
* \return: OK if connection complete / ERROR if fail or still pending
*/
ELMO_INT32 CMMCUDP::IsPending(ELMO_INT32 iSock) throw (CMMCException)
{
    ELMO_INT32     iRet;
    struct timeval tDelay;
    fd_set fdWrite, fdCheck;

    if (iSock == -1)
        CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' Is 'CMMCUDP' Class initialized?", 0, 0, -1, MMC_LIB_UDP_ISPENDING_INI_ERR, 0);

    /* check if data available */
    FD_ZERO (&fdWrite);
    FD_SET (iSock, &fdWrite);
    FD_ZERO (&fdCheck);
    FD_SET (iSock, &fdCheck);
    tDelay.tv_sec = tDelay.tv_usec = 0L; /* dont wait */
    iRet = select (FD_SETSIZE, NULL, &fdWrite, &fdCheck, &tDelay);
    if (iRet == MMCPP_SOCKERROR)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("'IsPending' failed on 'select' Class initialized?", 0, 0, -1, MMC_LIB_UDP_ISPENDING_SEL_ERR, 0);
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


#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    DWORD WINAPI fnUDPClbkThread( LPVOID lpParam )
    {
        ((CMMCUDP*)lpParam)->ClbkThread();
        return MMC_OK;
    }
#else
    ELMO_PVOID fnUDPClbkThread(ELMO_PVOID pObj)
    {
        ((CMMCUDP*)pObj)->ClbkThread();
        return pObj;
    }
#endif

ELMO_INT32 CMMCUDP::RunClbkThread()
{

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    DWORD ulThreadID;
    _thread = CreateThread( 
            NULL,               // default security attributes
            0,                  // use default stack size  
            fnUDPClbkThread,    // thread function name
            this,               // argument to thread function 
            0,                  // use default creation flags 
            &ulThreadID);       // returns the thread identifier 
#else
    pthread_attr_t attr;
//    struct sched_param sp;

    pthread_attr_init(&attr);               /* initialises a thread attributes object with the default value.*/
//    pthread_attr_getschedparam(&attr,&sp);    /* get the scheduling parameter default attributes */
//    if( getuid()==0)                      /* is this process run by root */
//    {
//        /* priority for time slice */
//        sp.sched_priority = 60;
//        pthread_attr_setschedparam(&attr,&sp);
//        pthread_attr_setschedpolicy(&attr, SCHED_RR);
//    }

    /* creation of new thread for call-back mode of operation*/
    pthread_create(&_thread, &attr, fnUDPClbkThread, this);

#endif

    return MMC_OK;
}

void CMMCUDP::ClbkThread()
{
    ELMO_INT32 rc, opt = 1;
    struct sockaddr_in saddr;
    ELMO_PUINT8 ucBuffer = new ELMO_UINT8[_iMsgMaxSize];

    fprintf(stderr,"ClbkThread: _usPort = %d\n", (ELMO_INT32)this->_usPort);

    if( setsockopt(_iSock, SOL_SOCKET, SO_REUSEADDR, (ELMO_PINT8)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        delete ucBuffer;
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

#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
        closesocket(_iSock);
#else
        close (_iSock);
#endif
        _iSock = MMCPP_INVSOCKET;
        delete ucBuffer;
        return;
    }

    fprintf(stderr, /*"%s*/"ClbkThread: Waiting for data...\n"/*, __func__*/);
    _bClbkThreadAlive = true;

    while(_bClbkThreadAlive)
    {
        /*if data is ready then receive it from socket and send user notification
         *  in order to process arrived data and send response. block on socket select.
         * */
        if ((rc = Receive(ucBuffer, _iMsgMaxSize, -1, &m_sockAddrOut)) > 0) {
            /*
             * when serving multiple clients user must save
             * within the call-back m_sockAddrOut parameters
             * in his own sockaddr_in storage, then he can use an instance
             * of his own for sending response.
             * (e.g. memcpy(&g_sockAddr, pSockaddr, sizeof(sockaddr_in));
             * */
            _fnClbk(_iSock, MMCPP_SOCK_READY_EVENT, ucBuffer, rc, &m_sockAddrOut);
        }
        else
        {
            fprintf(stderr, "ClbkThread: Receive nothing , Line %d\n", __LINE__);

		//#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
		//			Sleep(1000);    /* 1000 Milli Sec */
		//#else
		//			sleep(1);       /* 1          Sec */
		//#endif
		OS_PlatformSleepDelayMiliSec(1000);

        }
    }
    fprintf(stderr, /*"%s*/"ClbkThread: quit call-back mode thread.\n"/*, __func__*/);
    delete ucBuffer;
    return;
}
