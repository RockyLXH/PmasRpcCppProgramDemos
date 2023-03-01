/*
 * MMCPPIMU.cpp
 *
 *  Created on: 04/01/2018
 *      Author: zivb
 *      Update: 22Sep2019 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */


#include "OS_PlatformDependSetting.hpp"

#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)

#include "MMCPPIMU.hpp"


CMMCPPIMU::~CMMCPPIMU()
{
    //unmap
    if(m_pShmPtr)
    {
        munmap(m_pShmPtr,4096UL);
    }
    //close
    if (m_IMU_fd)
    {
        close(m_IMU_fd);
    }

    if (m_Buff)
    {
        delete []m_Buff;
    }
}

CMMCPPIMU::CMMCPPIMU()
{
    m_IMU_fd = 0;
    m_pShmPtr= NULL;
    m_Buff = NULL;
}

void CMMCPPIMU::init(ELMO_UINT8 cPort_index, ELMO_INT32 iBaud, ELMO_UINT32 iSize)
{
#ifdef _PLATINUM
    ELMO_INT32 fd;
//  unsigned char port_index = 0;
    pthread_t   comm_thread;
    struct sched_param schedp;

    m_ucPortIndex = cPort_index;
    m_iBaud = iBaud;
    m_iSize =  (iSize > STIM300_DATAGRAM_SIZE)?STIM300_DATAGRAM_SIZE:iSize;
    m_Buff = new ELMO_INT32[m_iSize];

    fd = open (STIM300_RX_DEV_1, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        printf ("error - %s\n", strerror (errno));
//      return -1; TODO: throw exception
    }
    set_interface_attribs_custom (fd, /*STIM300_BAUDRATE*/m_iBaud, 0);


    m_IMU_fd = open (NC_DEV, O_RDWR | O_SYNC);

    if (m_IMU_fd < 0)
    {
        printf ("error - %s\n", strerror (errno));
        //      return -1; TODO: throw exception
    }

    m_pShmPtr = (ELMO_PUINT8)mmap(0, 4096UL, PROT_READ|PROT_WRITE,       // not PROT_EXEC
                                                    MAP_SHARED | MAP_LOCKED,    // not MAP_FIXED | MAP_PRIVATE
                                                    m_IMU_fd, 4096UL);
    if (m_pShmPtr == MAP_FAILED)
    {
        close(m_IMU_fd);
        perror("Error mmapping the file");
        //      return -1; TODO: throw exception
    }

    pthread_create(&comm_thread, NULL,STIM300SyncThreadFunc, (ELMO_PVOID) /*&port_index*/ this);
    schedp.sched_priority = 91;
    if (pthread_setschedparam(comm_thread, SCHED_FIFO, &schedp))
    {
        printf("STIM300ThreadFunc - pthread_setschedparam failure %s\n", strerror(errno));
        //      return -1; TODO: throw exception
    }

    {
       cpu_set_t cpuset;
       CPU_ZERO(&cpuset);
       CPU_SET(0x0, &cpuset);

       pthread_t current_thread = pthread_self();
       pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    }
#endif
}


ELMO_INT32 CMMCPPIMU::set_interface_attribs_custom (ELMO_INT32 fd, ELMO_INT32 speed, ELMO_INT32 parity)
{
#ifdef _PLATINUM
        struct termios2 tio;

        memset (&tio, 0, sizeof tio);

        ioctl(fd, TCGETS2, &tio);

        tio.c_cflag &= ~CBAUD;
        tio.c_cflag |= BOTHER;
        tio.c_ispeed = speed;
        tio.c_ospeed = speed;


        tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
                                                        // disable IGNBRK for mismatched speed tests; otherwise receive break
                                                        // as \000 chars
        tio.c_iflag &= ~IGNBRK;                         // disable break processing
        tio.c_lflag = 0;                                // no signaling chars, no echo,
                                                        // no canonical processing
        tio.c_oflag = 0;                                // no remapping, no delays
        tio.c_cc[VMIN]  = 0;                            // read doesn't block
        tio.c_cc[VTIME] = 0;

        tio.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl

        tio.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
                                                        // enable reading
        tio.c_cflag &= ~(PARENB | PARODD);              // shut off parity
        tio.c_cflag |= parity;
        tio.c_cflag &= ~CSTOPB;
        tio.c_cflag &= ~CRTSCTS;

        {
            tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                    | INLCR | IGNCR | ICRNL | IXON);
            tio.c_oflag &= ~OPOST;
            tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
            tio.c_cflag &= ~(CSIZE | PARENB);
            tio.c_cflag |= CS8;
        }

        ioctl(fd, TCSETS2, &tio);
#endif
        return 0;
}



void *STIM300SyncThreadFunc(ELMO_PVOID arg)
{
#ifdef _PLATINUM
    ELMO_UINT32 port_datagram_buf_offset = 0;
    CMMCPPIMU *pCIMU = ((CMMCPPIMU *)arg);
    ELMO_UINT8 port_index  = pCIMU->m_ucPortIndex;//0;//= *((unsigned char *)arg);
    ELMO_INT32 fsmIdx = 0;

    ELMO_UINT32 op_code_offset;
    ELMO_UINT32 lf_code_offset;
    ELMO_UINT32 cr_code_offset;
    ELMO_UINT32 cnt_offset;
    ELMO_UINT32 payload_offset;
    ELMO_UINT32 datagram_valid_offset;

    ELMO_UINT8 op;
    ELMO_UINT8 lf;
    ELMO_UINT8 cr;
    ELMO_UINT8 payload_cnt;
    ELMO_UINT8 shm_cnt;

    ELMO_UINT8 stim300_datagram_payload_cnt = 0;
    ELMO_UINT8 stim300_datagram_shm_cnt = 0;

    ELMO_INT32 index = 0;
    ELMO_UINT8 flag = 0;

    ELMO_INT32 rc;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0x0, &cpuset);

    pthread_t current_thread = pthread_self();
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);

    printf("IMU Reading thread: %X starting\n", port_index);


    switch (port_index)
    {
        case 0:
        {
            port_datagram_buf_offset = STIM300_DATAGRAM_BASE_PORT_1;
            fsmIdx = 0;
        }
            break;
        case 1:
        {
            port_datagram_buf_offset = STIM300_DATAGRAM_BASE_PORT_2;
            fsmIdx = 1;
        }
            break;
        case 2:
        {
            port_datagram_buf_offset = STIM300_DATAGRAM_BASE_PORT_3;
            fsmIdx = 3;
        }
            break;
        case 3:
        {
            port_datagram_buf_offset = STIM300_DATAGRAM_BASE_PORT_4;
            fsmIdx = 2;
        }
            break;
        default:
        {
            printf("ERR - unrecognized port index!\n");
        }
    }

    op_code_offset = port_datagram_buf_offset + 0UL;
    cr_code_offset = port_datagram_buf_offset + 42UL;
    lf_code_offset = port_datagram_buf_offset + 43UL;
    cnt_offset = port_datagram_buf_offset + 44UL;
    payload_offset = port_datagram_buf_offset + STIM300_DATAGRAM_PAYLOAD_CNT_OFFSET;

    datagram_valid_offset = port_datagram_buf_offset + 45UL;

    ELMO_INT32 loopCnt = 30;


    while(1)
    {

        rc = ioctl(pCIMU->m_IMU_fd, NC_IOCUARTEVENT, &port_index);

        if (rc)
        {
            printf("IMU Reading problem  |(NC_IOCUARTEVENT) error: %s\n", strerror(errno));
            continue;
        }

        if(loopCnt > 0)     // Ignores the first 30 IMU readings, for initial setup
        {
            loopCnt--;
            continue;
        }

        op = pCIMU->m_pShmPtr[op_code_offset];          // g_IMU_map_ptr[port_datagram_buf_offset]
        cr = pCIMU->m_pShmPtr[cr_code_offset];          // g_IMU_map_ptr[port_datagram_buf_offset + 42UL]
        lf = pCIMU->m_pShmPtr[lf_code_offset];          // g_IMU_map_ptr[port_datagram_buf_offset + 43UL]

        payload_cnt = pCIMU->m_pShmPtr[payload_offset];// g_IMU_map_ptr[port_datagram_buf_offset + 35UL]
        shm_cnt = pCIMU->m_pShmPtr[cnt_offset];     // g_IMU_map_ptr[port_datagram_buf_offset + 44UL]

        memcpy((ELMO_PUINT8)(&pCIMU->m_Buff[fsmIdx]), (ELMO_PUINT8)(&(pCIMU->m_pShmPtr[port_datagram_buf_offset])), pCIMU->m_iSize);

        if (!(  op == 0xA5 &&
                cr == 0x0D  &&
                lf == 0x0A))
        {
            printf("IMU Reading problem  | sync err: port_index: %X [ID]: %X [CR]: %X [LF]: %X\n", port_index, op, cr, lf);
            continue;
        }
        else
        {
            if (!flag)
            {
                flag = 1;

                stim300_datagram_payload_cnt = payload_cnt;
                stim300_datagram_shm_cnt = shm_cnt;

                stim300_datagram_shm_cnt++;
                stim300_datagram_payload_cnt++;
            }
            else
            {

                    stim300_datagram_shm_cnt = shm_cnt;
                    stim300_datagram_payload_cnt = payload_cnt;
                    stim300_datagram_shm_cnt++;
                    stim300_datagram_payload_cnt++;

            }
            index++;
            if (index >=1000)
            {
                index = 0 ;
//              printf ("1000 msgs received \n") ;
            }
        }
//      if (fsmIdx==1)
//          status_err_led_down();
    }
#endif
    return 0;
}

#endif  /* #if   (OS_PLATFORM == LINUXIPC32_PLATFORM) */

