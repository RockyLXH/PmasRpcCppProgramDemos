/*
 * MMCSerialPort.cpp
 *
 *  Created on: 23/05/2018
 *      Author: michaelf
 *      Update: 22Sep2019 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */
 
#include "OS_PlatformDependSetting.hpp"

 
#if defined(_PLATINUM) || defined(pmas) || defined(PMAS)

#include <asm/termios.h>
#include <asm/ioctl.h>

#include <stropts.h>
#include <linux/netlink.h>

#include "MSP_Kernel_IF.h"
#include "MMCSerialPort.h"
//#include "DebugInterfaces.h"


#define NO_DEBUG
#define NO_ENTRY_EXIT

#define MY_ERROR(format, arg...) fprintf(stderr, "### %s:%d E " format "\n", __FUNCTION__, __LINE__,## arg)
#define MY_INFO(format, arg...)  fprintf(stderr, "### %s:%d I " format "\n", __FUNCTION__, __LINE__,## arg)

#ifndef NO_DEBUG
    #define MY_DEBUG(format, arg...)  fprintf(stderr, "### %s:%d D " format "\n", __FUNCTION__, __LINE__,## arg)
#else
    #define MY_DEBUG(format, arg...)
#endif

#ifndef NO_ENTRY_EXIT
    #define MY_ENTRY(format, arg...) fprintf(stderr, "### %s:%d > " format "\n", __FUNCTION__, __LINE__,## arg)
    #define MY_EXIT(format, arg...)  fprintf(stderr, "### %s:%d < " format "\n", __FUNCTION__, __LINE__,## arg)
#else
    #define MY_ENTRY(format, arg...)
    #define MY_EXIT(format, arg...)
#endif

#define USE_BARRIER()
//#define USE_BARRIER() __sync_synchronize()


#define NETLINK_USER 31


/*
 * CMSP_InternalBase - abstract base Class for internal implementation of
 * Maestro Serial Protocol.
 *
 */
class CMSP_InternalBase
{
public:
    CMSP_InternalBase();
    virtual ~CMSP_InternalBase();
    virtual ELMO_INT32 Open(ELMO_INT32 ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg& cfg) = 0;
    virtual void Close() = 0;
    virtual ssize_t Write(const void *buf, size_t count) = 0;
    virtual ssize_t Read(ELMO_PVOID buf, size_t count) = 0;
    virtual ELMO_INT32 Timestamp(struct timespec *ts);
    ELMO_INT32 FdActiveGet() { return m_fd; };
    virtual ELMO_INT32 StatGet(struct SMaestroSerialPortStat &msps) = 0;

    ELMO_INT32 m_ttyNum;
    ELMO_INT32 m_fd;
    struct timespec lastPacketTimestamp;
    CMaestroSerialPort::SMaestroSerialPortCfg *m_cfg;
};

/*
 * CMSP_InternalNormal - internal implementation of Normal mode:
 * thin wrapper of /dev/ttyOx standard file operations
 *
 */
class CMSP_InternalNormal: public CMSP_InternalBase
{
public:
    CMSP_InternalNormal();
    virtual ~CMSP_InternalNormal();
    virtual ELMO_INT32 Open(ELMO_INT32 ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg& cfg);
    virtual void Close();
    virtual ssize_t Write(const void *buf, size_t count);
    virtual ssize_t Read(ELMO_PVOID buf, size_t count);
    virtual ELMO_INT32 StatGet(struct SMaestroSerialPortStat &msps) { return -1; };

    ELMO_INT32 set_interface_attribs_custom();
};

/*
 * CMSP_InternalEnhanced - internal implementation of Enhanced mode:
 * uses nv_drv_x for shared memory + DMA based transmission and reception and
 * ttyOx for UART control
 *
 */
class CMSP_InternalEnhanced: public CMSP_InternalBase
{
public:
    CMSP_InternalEnhanced(CMSP_InternalNormal* normal);
    virtual ~CMSP_InternalEnhanced();
    virtual ELMO_INT32 Open(int ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg& cfg);
    virtual void Close();
    virtual ssize_t Write(const void *buf, size_t count);
    virtual ssize_t Read(ELMO_PVOID buf, size_t count);
    virtual ELMO_INT32 StatGet(struct SMaestroSerialPortStat &msps);

    void netlink_open();
    void netlink_close();
    void netlink_send();
    CMSP_InternalNormal* m_normal;  // for UART control
    MSP_RB_Control *m_rbc;          // shared memory control block for current device
    MSP_RB_Data *m_rbd;             // DMA buffers in shared memory for current device
    uint32_t    m_shm_mmap_data_marker;

    struct ml_msg : public nlmsghdr {
        int32_t ttyNum;
    };
    struct ml_msg m_nlh;
    struct iovec m_iov;
    struct msghdr m_msg;

    static ELMO_INT32 m_openedDevs;        // reference count of opened devices
    static SMSP_SharedMemControl *shm_ctrl; // global shared memory control block
    static struct sockaddr_nl m_src_addr;
    static struct sockaddr_nl m_dest_addr;
    static ELMO_INT32 m_sock_fd;
};

/*
 * CMSP_Internal - internal implementation of CMaestroSerialPort:
 * class CMaestroSerialPort uses delegation to perform all of its operations
 * in this class. It is part of Bridge Pattern to separate out the interface
 * from its implementation and pImpl programming technique that removes
 * implementation details of a class from its object representation by placing
 * them in a separate class, accessed through an opaque pointer
 *
 */
class CMSP_Internal
{
public:
    CMSP_Internal();
    ~CMSP_Internal();
    ELMO_INT32 Open(ELMO_INT32 ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg &cfg);
    void Close();
    ELMO_INT32 ConfigurationGet(CMaestroSerialPort::SMaestroSerialPortCfg *cfg);
    ssize_t Write(const void *buf, size_t count);
    ssize_t Read(ELMO_PVOID buf, size_t count);
    ELMO_INT32 FdActiveGet();
    ELMO_INT32 Timestamp(struct timespec *ts);
    ELMO_INT32 StatGet(struct SMaestroSerialPortStat &msps);

    // private implementation
    ELMO_INT32 Ioctl(ELMO_INT32 request, ELMO_PINT8 argp);
    ELMO_INT32 Mode(CMaestroSerialPort::Mode mode);
    CMaestroSerialPort::Mode Mode();

    // normal exists always, enhanced exists only in enhanced mode
    // active points to current active mode (selected by last Open or Mode)
    CMSP_InternalBase* m_active;
    CMSP_InternalEnhanced* m_enhanced;
    CMSP_InternalNormal* m_normal;
    CMaestroSerialPort::SMaestroSerialPortCfg m_cfg;
};


ELMO_INT32 CMSP_InternalEnhanced::m_openedDevs = 0;
SMSP_SharedMemControl *CMSP_InternalEnhanced::shm_ctrl = NULL;
struct sockaddr_nl CMSP_InternalEnhanced::m_src_addr;
struct sockaddr_nl CMSP_InternalEnhanced::m_dest_addr;
ELMO_INT32 CMSP_InternalEnhanced::m_sock_fd = -1;


/*
 * Implementation of CMaestroSerialPort
 */
CMaestroSerialPort::CMaestroSerialPort()
{
    MY_ENTRY();
    pImpl.reset(new CMSP_Internal());
    MY_EXIT();
}

CMaestroSerialPort::~CMaestroSerialPort()
{
//  MY_ENTRY();
//  MY_EXIT();
}

ELMO_INT32 CMaestroSerialPort::Open(ELMO_INT32 ttyNum, SMaestroSerialPortCfg &cfg)
{
    MY_ENTRY("pImpl = %p", pImpl.get());
    ELMO_INT32 ret = pImpl->Open(ttyNum, cfg);
    MY_EXIT();
    return ret;
}

void CMaestroSerialPort::Close()
{
    MY_ENTRY();
    pImpl->Close();
    MY_EXIT();
}

ssize_t CMaestroSerialPort::Write(const void *buf, size_t count)
{
    return pImpl->Write(buf, count);
}

ssize_t CMaestroSerialPort::Read(ELMO_PVOID buf, size_t count)
{
    return pImpl->Read(buf, count);
}

ELMO_INT32 CMaestroSerialPort::Ioctl( request, ELMO_PINT8 argp)
{
    return pImpl->Ioctl(request, argp);
}

ELMO_INT32 CMaestroSerialPort::FdActiveGet()
{
    return pImpl->FdActiveGet();
}

ELMO_INT32 CMaestroSerialPort::Timestamp(struct timespec *ts)
{
    return pImpl->Timestamp(ts);
}

ELMO_INT32 CMaestroSerialPort::StatGet(struct SMaestroSerialPortStat &msps)
{
    return pImpl->StatGet(msps);
}

//////////////////////////////////////////////////////////////

/*
 * Implementation of CMSP_Internal
 */
CMSP_Internal::CMSP_Internal()
{
    MY_ENTRY();
    m_enhanced = NULL;
    m_normal = new CMSP_InternalNormal();
    m_active = NULL;
    MY_EXIT();
}

CMSP_Internal::~CMSP_Internal()
{
    MY_ENTRY();
    if ( m_enhanced != NULL ) {
//      MY_DEBUG();
        delete m_enhanced;
    }
    if ( m_normal != NULL ) {
//      MY_DEBUG();
        delete m_normal;
    }
    MY_EXIT();
}

ELMO_INT32 CMSP_Internal::Open(ELMO_INT32 ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg &cfg)
{
    ELMO_INT32 ret;
    MY_ENTRY("ttyNum %d mode %u speed %u rxFixedPacketSize %u", ttyNum, cfg.mode, cfg.speed, cfg.rxFixedPacketSize);

    m_cfg = cfg;

    if ( Mode(cfg.mode) < 0 ) {
        MY_EXIT("-1");
        return -1;
    }

    // re-open normal - maybe baud was changed
    MY_DEBUG("ttyNum %d mode %u speed %u rxFixedPacketSize %u", ttyNum, cfg.mode, cfg.speed, cfg.rxFixedPacketSize);
    ret = m_normal->Open(ttyNum, m_cfg);
    if ( ret < 0 ) {
        MY_EXIT("-1");
        return ret;
    }
    if ( m_active != NULL && m_active != m_normal ) {
        MY_DEBUG("ttyNum %d mode %u speed %u rxFixedPacketSize %u", ttyNum, cfg.mode, cfg.speed, cfg.rxFixedPacketSize);
        ret = m_active->Open(ttyNum, m_cfg);
    }
    MY_EXIT("%d: %d", ttyNum, ret);
    return ret;
}

void CMSP_Internal::Close()
{
    MY_ENTRY();
    if ( m_active != NULL && m_active == m_enhanced ) {
        MY_DEBUG("%d", m_enhanced->m_ttyNum);
        delete m_enhanced;
        m_enhanced = NULL;
        m_active = NULL;
    }
    MY_DEBUG("%d", m_normal->m_ttyNum);
    m_normal->Close();
    MY_EXIT();
}

ELMO_INT32 CMSP_Internal::ConfigurationGet(CMaestroSerialPort::SMaestroSerialPortCfg *cfg)
{
    ELMO_INT32 ret = 0;
    MY_ENTRY();

    if ( cfg ) {
        *cfg = m_cfg;
    }
    else {
        ret = -1;
    }
    MY_EXIT();
    return ret;
}

ELMO_INT32 CMSP_Internal::Mode(CMaestroSerialPort::Mode mode)
{
    MY_ENTRY("mode %d", mode);

    switch ( mode ) {
    case CMaestroSerialPort::Normal: // normal mode
        // on switch from enhanced to normal - close enhanced
        MY_DEBUG();
        if (  Mode() == CMaestroSerialPort::Enhanced ) {
            MY_DEBUG();
            m_active->Close();
        }
        MY_DEBUG();
        m_active = m_normal;
        break;
    case CMaestroSerialPort::Enhanced:  // enhanced mode
        // on switch from normal to enhanced - leave normal as is
        MY_DEBUG();
        if ( m_enhanced == NULL ) {
            MY_DEBUG();
            m_enhanced = new CMSP_InternalEnhanced(m_normal);
        }
        // copy internal data
        MY_DEBUG();
        m_active = m_enhanced;
        break;
    default:
        MY_EXIT("-1");
        return -1;
    }
    MY_EXIT("0");
    return 0;
}

CMaestroSerialPort::Mode CMSP_Internal::Mode()
{
    if ( m_active != NULL && m_active == m_enhanced ) {
        MY_DEBUG("enhanced");
        return CMaestroSerialPort::Enhanced;
    }
    MY_DEBUG("normal");
    return CMaestroSerialPort::Normal;
}

ssize_t CMSP_Internal::Write(const void *buf, size_t count)
{
    if ( m_active != NULL ) {
        return m_active->Write(buf, count);
    }
    MY_DEBUG("error");
    return -1;
}

ssize_t CMSP_Internal::Read(ELMO_PVOID buf, size_t count)
{
    if ( m_active != NULL ) {
        return m_active->Read(buf, count);
    }
    MY_DEBUG("error");
    return -1;
}

ELMO_INT32 CMSP_Internal::Ioctl(ELMO_INT32 request, ELMO_PINT8 argp)
{
    ELMO_INT32 fd = FdActiveGet();
    if ( fd < 0 ) {
        errno = -EBADF;
        MY_DEBUG("error");
        return fd;
    }
    return ioctl(fd, request, argp);
}

ELMO_INT32 CMSP_Internal::FdActiveGet()
{
    if ( m_active != NULL ) {
        return m_active->FdActiveGet();
    }
    MY_DEBUG("error");
    return -1;
}

ELMO_INT32 CMSP_Internal::Timestamp(struct timespec *ts)
{
    if ( m_active != NULL ) {
        return m_active->Timestamp(ts);
    }
    return -1;
}

ELMO_INT32 CMSP_Internal::StatGet(struct SMaestroSerialPortStat &msps)
{
    if ( m_enhanced != NULL ) {
        return m_enhanced->StatGet(msps);
    }
    return -1;
}

//////////////////////////////////////////////////////////////

/*
 * Implementation of CMSP_InternalBase
 */
CMSP_InternalBase::CMSP_InternalBase()
{
    MY_DEBUG();
    m_ttyNum = -1;
    m_fd = -1;
    memset(&lastPacketTimestamp, 0, sizeof(lastPacketTimestamp));
    m_cfg = NULL;
}

CMSP_InternalBase::~CMSP_InternalBase()
{
//  MY_ENTRY();
//  MY_EXIT();
}

ELMO_INT32 CMSP_InternalBase::Timestamp(struct timespec *ts)
{
    if ( ts == NULL ) {
        return -1;
    }
    if ( lastPacketTimestamp.tv_sec == 0 && lastPacketTimestamp.tv_nsec == 0 ) {
        return -1;
    }
    *ts = lastPacketTimestamp;
    return 0;
}

////////////////////////////////////////////////////////////////////

/*
 * Implementation of CMSP_InternalNormal
 */
CMSP_InternalNormal::CMSP_InternalNormal()
{
    MY_DEBUG();
}

CMSP_InternalNormal::~CMSP_InternalNormal()
{
    MY_ENTRY();
    Close();
    MY_EXIT();
}

ELMO_INT32 CMSP_InternalNormal::Open(ELMO_INT32 ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg& cfg)
{
    ELMO_INT8 ttyName[20];
    MY_ENTRY("%d: normal", ttyNum);

    Close();

    m_cfg = &cfg;

    snprintf(ttyName, sizeof(ttyName), "/dev/ttyO%d", ttyNum);
    m_fd = open(ttyName, O_RDWR | O_NOCTTY | O_SYNC);
    if ( m_fd < 0 ) {
        MY_EXIT("%d: m_fd %d", ttyNum, m_fd);
        return m_fd;
    }
    m_ttyNum = ttyNum;
    set_interface_attribs_custom();
    MY_EXIT("%d: normal m_fd %d rxFixedPacketSize %d", m_ttyNum, m_fd, m_cfg->rxFixedPacketSize);
    return m_fd;
}

ELMO_INT32 CMSP_InternalNormal::set_interface_attribs_custom()
{
    struct termios2 tio;

    if ( m_fd < 0 ) {
        MY_DEBUG();
        return -1;
    }

    MY_DEBUG();
    ioctl(m_fd, TCGETS2, &tio);

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = m_cfg->speed;
    tio.c_ospeed = m_cfg->speed;


    tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
                                                    // disable IGNBRK for mismatched speed tests; otherwise receive break
                                                    // as \000 chars
    tio.c_iflag &= ~IGNBRK;                         // disable break processing
    tio.c_lflag = 0;                                // no signaling chars, no echo,
                                                    // no canonical processing
    tio.c_oflag = 0;                                // no remapping, no delays
    if ( m_cfg->blockingMode ) {
        tio.c_cc[VMIN]  = 1;
        tio.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
    }
    else {
        tio.c_cc[VMIN]  = 0;                            // read doesn't block
        tio.c_cc[VTIME] = 0;
    }

    tio.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl

    tio.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
                                                    // enable reading
    tio.c_cflag &= ~(PARENB | PARODD | CMSPAR);     // shut off parity
    tio.c_cflag |= (unsigned)m_cfg->parity;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag |= (unsigned)m_cfg->stopBits;
    tio.c_cflag &= ~CRTSCTS;

    tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                            | INLCR | IGNCR | ICRNL | IXON);
    tio.c_oflag &= ~OPOST;
    tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    if ( m_cfg->lineMode ) {
        tio.c_lflag |= ICANON;
    }
    return ioctl(m_fd, TCSETS2, &tio);
}

void CMSP_InternalNormal::Close()
{
    MY_ENTRY("%d: normal m_fd %d", m_ttyNum, m_fd);
    if ( m_fd >= 0 ) {
        close(m_fd);
        m_fd = -1;
        m_ttyNum = -1;
        m_cfg = NULL;
    }
    MY_EXIT("%d normal", m_ttyNum);
}

ssize_t CMSP_InternalNormal::Write(const void *buf, size_t count)
{
    return write(m_fd, buf, count);
}

ssize_t CMSP_InternalNormal::Read(void *buf, size_t count)
{
    ssize_t len = read(m_fd, buf, count);
    if ( len > 0 ) {
        // save timestamp
    }
    return len;
}

////////////////////////////////////////////////////////////////////////////

/*
 * Implementation of CMSP_InternalEnhanced
 */
CMSP_InternalEnhanced::CMSP_InternalEnhanced(CMSP_InternalNormal* normal)
{
    m_normal = normal;
    m_rbc = NULL;
    m_rbd = NULL;
    m_shm_mmap_data_marker = 0;
    MY_DEBUG();
}

CMSP_InternalEnhanced::~CMSP_InternalEnhanced()
{
    MY_ENTRY();
    Close();
    MY_EXIT();
}

ELMO_INT32 CMSP_InternalEnhanced::Open(ELMO_INT32 ttyNum, CMaestroSerialPort::SMaestroSerialPortCfg &cfg)
{
    ELMO_INT8 mspDevName[20];
    MY_ENTRY("%d: enhanced speed %u rxFixedPacketSize %u", ttyNum, cfg.speed, cfg.rxFixedPacketSize);

    m_cfg = &cfg;
    m_ttyNum = ttyNum;

    MY_INFO("ttyNum %d mode %u speed %u rxFixedPacketSize %u", ttyNum, m_cfg->mode, m_cfg->speed, m_cfg->rxFixedPacketSize);

    m_normal->Close(); // tty should be opened AFTER nc_drv for setting hooks

    snprintf(mspDevName, sizeof(mspDevName), "/dev/" MSP_DEV_NAME_PFX "%d", ttyNum);
    m_fd = open(mspDevName, O_RDWR | O_NOCTTY | O_SYNC);
    if ( m_fd < 0 ) {

        m_cfg = NULL;
        MY_EXIT("m_fd %d", m_fd);
        return m_fd;
    }
    ++m_openedDevs;
    if ( m_openedDevs == 1 ) {
        // map shared memory
        ELMO_INT32 errnum_ctrl = 0;
        errno = 0;
//      status_led_init();
//      iopin_down();
//      iopin_up1();
        MY_INFO("allocation of shm_ctrl 0x%x, size %u", MSP_SHM_MMAP_CTRL_ID, sizeof(*shm_ctrl));
        shm_ctrl = (SMSP_SharedMemControl *)
                mmap((ELMO_PVOID)MSP_SHM_MMAP_CTRL_ID, sizeof(*shm_ctrl),
                        PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        errnum_ctrl = errno;
        if ( shm_ctrl == MAP_FAILED ) {
            // allocation of shared memory failed
            MY_ERROR("allocation of shared memory failed");
            if ( errnum_ctrl != 0 ) {
                MY_ERROR("ctrl: errno 0x%x %s", errnum_ctrl, strerror(errnum_ctrl));
            }
            Close();
            MY_EXIT("m_fd %d", m_fd);
            return m_fd;
        }
        // check version
        if ( shm_ctrl->magic != MSP_SHM_MAGIC ||
            shm_ctrl->version != MSP_SHM_VERSION ||
            shm_ctrl->size != sizeof(*shm_ctrl))
        {
            // wrong version !!!
            MY_ERROR("wrong version!!! magic 0x%x (0x%x) version 0x%x (0x%x) size %d (%d)",
                    shm_ctrl->magic, MSP_SHM_MAGIC,
                    shm_ctrl->version, MSP_SHM_VERSION,
                    shm_ctrl->size, sizeof(*shm_ctrl));
            Close();
            MY_EXIT("m_fd %d", m_fd);
            return m_fd;
        }
        netlink_open();
    }
    ELMO_INT32 errnum_data = 0;
    m_shm_mmap_data_marker = MSP_SHM_MMAP_DATA_ID + ttyNum * 0x1000;
    errno = 0;
    m_rbd = (MSP_RB_Data *)
            mmap((ELMO_PVOID)m_shm_mmap_data_marker, sizeof(*m_rbd),
                    PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    errnum_data = errno;
    if ( m_rbd == MAP_FAILED ) {
        // allocation of shared memory failed
        MY_ERROR("allocation of shared memory failed");
        if ( errnum_data != 0 ) {
            MY_ERROR("data: errno 0x%x %s", errnum_data, strerror(errnum_data));
        }
        Close();
        MY_EXIT("%d: m_fd %d", m_ttyNum, m_fd);
        return m_fd;
    }

    m_rbc = &shm_ctrl->rbc[ttyNum];

    // configure kernel module
    ioctl(m_fd, MSP_IOC_SET_PACKET_LENGTH, m_cfg->rxFixedPacketSize);
    ioctl(m_fd, MSP_IOC_SET_SPEED, m_cfg->speed);
    ioctl(m_fd, MSP_IOC_SET_TS_FORMAT, m_cfg->timestampFormat);
    ioctl(m_fd, MSP_IOC_SET_BLOCKING_MODE, m_cfg->blockingMode);

    // open normal AFTER opening enhanced:
    //  open of enhanced setup tty callbacks, and open of normal
    //  initializes tty registers using callbacks
    m_normal->Open(ttyNum, cfg);

    // All has been configured - Start !
    ioctl(m_fd, MSP_IOC_RX_START);

    MY_EXIT("%d: m_fd %d rxFixedPacketSize %d", m_ttyNum, m_fd, m_cfg->rxFixedPacketSize);
    return m_fd;

}

void CMSP_InternalEnhanced::Close()
{
    MY_ENTRY("%d: enhanced m_fd %d m_openedDevs %d", m_ttyNum, m_fd, m_openedDevs);
    if ( m_fd >= 0 ) {
        close(m_fd);
        m_fd = -1;
        m_cfg = NULL;
        m_ttyNum = -1;
        if ( m_rbd != MAP_FAILED ) {
            MY_DEBUG("unmap shm_data");
            munmap(m_rbd, sizeof(*m_rbd));
        }
        --m_openedDevs;
        if ( m_openedDevs == 0 ) {
            if ( shm_ctrl != MAP_FAILED ) {
                MY_DEBUG("unmap shm_ctrl");
                munmap(shm_ctrl, sizeof(*shm_ctrl));
            }
            netlink_close();
        }
    }
    MY_EXIT();
}

ELMO_INT32 CMSP_InternalEnhanced::StatGet(struct SMaestroSerialPortStat &msps)
{
    MY_ENTRY("%d", m_ttyNum);
    if ( m_fd < 0 ) {
        MY_ERROR("%d device is closed", m_ttyNum);
        MY_EXIT("%d", m_ttyNum);
        return -1;
    }

    struct MSP_Enhanced_Stat mes;

    memset(&mes, 0, sizeof(mes));
    ioctl(m_fd, MSP_IOC_GET_STAT, &mes);

    // for decoupling: use different structures and copy each member separately

    msps.tx_packets     = mes.tx_packets;
    msps.tx_dropped     = mes.tx_dropped;
    msps.rx_packets     = mes.rx_packets;
    msps.rx_dropped     = mes.rx_dropped;
    msps.rx_resync      = mes.rx_resync;
    msps.rx_skipped     = mes.rx_skipped;
    msps.speed          = mes.speed;
    msps.ts_format      = mes.ts_format;
    msps.rx_packet_len  = mes.rx_packet_len;
    msps.blocking_mode  = mes.blocking_mode;
    msps.rx_dma_burst   = mes.rx_dma_burst;
    msps.rx_fifo_trig   = mes.rx_fifo_trig;
    msps.tx_cpu         = mes.tx_cpu;
    msps.rx_cpu         = mes.rx_cpu;
    msps.kern_log_level = mes.kern_log_level;


    MY_EXIT("%d", m_ttyNum);
    return 0;
}

void CMSP_InternalEnhanced::netlink_open()
{
    MY_ENTRY("%d", m_ttyNum);
    m_sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if ( m_sock_fd < 0 ) {
        ELMO_INT32 err = errno;
        MY_ERROR("%d: socket error %d %s", m_ttyNum, err, strerror(err));
        MY_EXIT("%d: socket error %d %s", m_ttyNum, err, strerror(err));
        return;
    }

    memset(&m_src_addr, 0, sizeof(m_src_addr));
    m_src_addr.nl_family = AF_NETLINK;
    m_src_addr.nl_pid = getpid(); /* self pid */

    memset(&m_dest_addr, 0, sizeof(m_dest_addr));
    m_dest_addr.nl_family = AF_NETLINK;
    m_dest_addr.nl_pid = 0; /* For Linux Kernel */
    m_dest_addr.nl_groups = 0; /* unicast */

    if ( bind(m_sock_fd, (struct sockaddr*) &m_src_addr, sizeof(m_src_addr)) < 0 ) {
        ELMO_INT32 err = errno;
        MY_ERROR("%d: bind error %d %s", m_ttyNum, err, strerror(err));
        MY_EXIT("%d: bind error %d %s", m_ttyNum, err, strerror(err));
        return;
    }
    MY_EXIT("%d", m_ttyNum);
}

void CMSP_InternalEnhanced::netlink_close()
{
    MY_ENTRY("%d", m_ttyNum);
    if ( m_sock_fd >= 0 ) {
        close(m_sock_fd);
        m_sock_fd = -1;
    }
    MY_EXIT("%d", m_ttyNum);
}

void CMSP_InternalEnhanced::netlink_send()
{
    memset(&m_nlh, 0, sizeof(m_nlh));
    m_nlh.nlmsg_len = sizeof(m_nlh);
    m_nlh.nlmsg_pid = getpid();
    m_nlh.nlmsg_flags = 0;
    m_nlh.ttyNum = m_ttyNum;

    m_iov.iov_base = (ELMO_PVOID)&m_nlh;
    m_iov.iov_len = m_nlh.nlmsg_len;

    m_msg.msg_name = (ELMO_PVOID)&m_dest_addr;
    m_msg.msg_namelen = sizeof(m_dest_addr);
    m_msg.msg_iov = &m_iov;
    m_msg.msg_iovlen = 1;

    errno = 0;
    sendmsg(m_sock_fd, &m_msg, 0);
    if ( errno != 0 ) {
        ELMO_INT32 err = errno;
        MY_ERROR("%d: m_sock_fd %d errno %d %s", m_ttyNum, m_sock_fd, err, strerror(err));
    }
}

ssize_t CMSP_InternalEnhanced::Write(const void *buf, size_t count)
{
    MY_ENTRY("%d: count %d", m_ttyNum, count);
    if ( count > MSP_MAX_PACKET_SIZE ) {
        errno = -EINVAL;
        MY_EXIT("%d: count %d", m_ttyNum, -1);
        return -1;
    }
    if ( m_fd < 0 ) {
        errno = -EBADF;
        MY_EXIT("%d: count %d", m_ttyNum, -1);
        return -1;
    }

#if 0
    {
        const ELMO_INT8 *cbuf = (const ELMO_INT8 *)buf;
        size_t i;
        fprintf(stderr, "Write: count = %d buf = \"", count);
        for ( i = 0 ; i < count ; ++i ) {
            if ( cbuf[i] == '\n' ) {
                fprintf(stderr, "\\n");
            }
            else if ( cbuf[i] == '\r' ) {
                fprintf(stderr, "\\r");
            }
            else if ( cbuf[i] == '\0' ) {
                fprintf(stderr, "\\0");
            }
            else if ( cbuf[i] < 0x20 ) {
                fprintf(stderr, "\\x%02X", cbuf[i]);
            }
            else if ( (cbuf[i] & 0x80) != 0 ) {
                fprintf(stderr, "\\x%02X", cbuf[i]);
            }
            else {
                fprintf(stderr, "%c", cbuf[i]);
            }
        }
        fprintf(stderr, "\"\n");
    }
#endif

//  if ( m_ttyNum == 8 ) {
//      iopin_up();
//  }

    USE_BARRIER();
    if ( m_rbc->tbWrCnt != m_rbc->tbDoneCnt ) {
        ++m_rbc->tbWrSkipCnt;
    }
    memcpy(m_rbd->txBuf, buf, count);
    m_rbc->bytesInTxBuf = count;
    m_rbc->tbWrCnt = m_rbc->tbDoneCnt + 1;
    USE_BARRIER();
//  ioctl(m_fd, MSP_IOC_TX_START);
    netlink_send();

//  if ( m_cfg->blockingMode ) {
//      MY_DEBUG("%d: waiting...", m_ttyNum);
//      ioctl(m_fd, MSP_IOC_TX_WAIT_DONE);
//  }
    errno = 0;
//  if ( m_ttyNum == 8 ) {
//      iopin_up1();
//  }
    MY_DEBUG("%d: count %d", m_ttyNum, count);
    MY_EXIT("%d: count %d", m_ttyNum, count);
    return count;
}

ssize_t CMSP_InternalEnhanced::Read(ELMO_PVOID buf, size_t count)
{
    MY_ENTRY("%d: count %d rxFixedPacketSize %d", m_ttyNum, count, m_cfg->rxFixedPacketSize);
    if ( m_ttyNum == 0 ) {
//      static ELMO_INT32 cnt = 0;
//      ++cnt;
//      if ( (cnt & 1) == 0 ) {
//          iopin_up1();
//      }
//      else {
//          iopin_down1();
//      }
//      iopin_down1();
    }
    if ( count != m_cfg->rxFixedPacketSize ) {
        errno = -EINVAL;
        MY_EXIT("%d: count %d", m_ttyNum, -1);
        return -1;
    }
    if ( buf == NULL ) {
        errno = -EINVAL;
        MY_EXIT("%d: count %d", m_ttyNum, -1);
        return -1;
    }
    if ( m_fd < 0 ) {
        errno = -EBADF;
        MY_EXIT("%d: count %d", m_ttyNum, -1);
        return -1;
    }
    USE_BARRIER();
//  MY_DEBUG("rbRdIndex %d rbWrIndex %d", m_rbc->rbRdIndex, m_rbc->rbWrIndex);
    if ( m_rbc->rbRdIndex == m_rbc->rbWrIndex ) {
        // no packet yet available
        if ( m_cfg->blockingMode ) {
            MY_DEBUG("%d: waiting...", m_ttyNum);
            ioctl(m_fd, MSP_IOC_RX_WAIT_DATA_READY);
        }
    }
    USE_BARRIER();
//  MY_DEBUG("rbRdIndex %d rbWrIndex %d", m_rbc->rbRdIndex, m_rbc->rbWrIndex);
    if ( m_rbc->rbRdIndex == m_rbc->rbWrIndex ) {
        // still no packet available
        errno = -EAGAIN;
        MY_EXIT("%d: count %d", m_ttyNum, 0);
        return 0;
    }
//  MY_DEBUG("rbRdIndex %d rbWrIndex %d", m_rbc->rbRdIndex, m_rbc->rbWrIndex);
    // skip old packets in case new is already in memory
    while ( MSP_NEXT_RB_INDEX(m_rbc->rbRdIndex) != m_rbc->rbWrIndex ) {
        m_rbc->rbRdIndex = MSP_NEXT_RB_INDEX(m_rbc->rbRdIndex);
        ++m_rbc->rbRdSkipCnt;
        USE_BARRIER();
    }
//  MY_DEBUG("rbRdIndex %d rbWrIndex %d", m_rbc->rbRdIndex, m_rbc->rbWrIndex);
    count = m_rbc->slot[m_rbc->rbRdIndex].bytesInRxBuf;
    memcpy(buf, m_rbd->rxBuf[m_rbc->rbRdIndex], count);
    lastPacketTimestamp = m_rbc->slot[m_rbc->rbRdIndex].timestamp;
    m_rbc->rbRdIndex = MSP_NEXT_RB_INDEX(m_rbc->rbRdIndex);
    USE_BARRIER();
//  MY_DEBUG("rbRdIndex %d rbWrIndex %d", m_rbc->rbRdIndex, m_rbc->rbWrIndex);
    errno = 0;
    MY_EXIT("%d: count %d buf 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
            m_ttyNum, count,
            ((ELMO_PUINT8)buf)[0], ((ELMO_PUINT8)buf)[1],
            ((ELMO_PUINT8)buf)[2], ((ELMO_PUINT8)buf)[3],
            ((ELMO_PUINT8)buf)[4], ((ELMO_PUINT8)buf)[5],
            ((ELMO_PUINT8)buf)[6], ((ELMO_PUINT8)buf)[7],
            ((ELMO_PUINT8)buf)[8], ((ELMO_PUINT8)buf)[9]);
    return count;
}
#endif
