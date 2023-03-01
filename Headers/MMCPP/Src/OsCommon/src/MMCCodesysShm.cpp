/*
 * MMCCodesysShm.cpp
 *
 *  Created on: 18/12/2017
 *      Author: zivb
 *      Update: 19Sep2019 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"

//#ifndef WIN32
#if   (OS_PLATFORM == LINUXIPC32_PLATFORM)



#include  "MMCCodesysShm.h"

CMMCCodesysShm::CMMCCodesysShm()
{
    m_dSize = 0;
    m_ShmPtr = NULL;

}

CMMCCodesysShm::~CMMCCodesysShm()
{
    ShmStop();
}

void CMMCCodesysShm::shmStart(const ELMO_INT8* ShmName, ELMO_DOUBLE dSize) throw (CMMCException)
{
    m_dSize = dSize;
    ELMO_INT32 fd = shm_open(ShmName, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    ftruncate(fd, dSize);
    m_ShmPtr = mmap(0, dSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return;
}

void CMMCCodesysShm::ShmStop()throw (CMMCException)
{
    if (m_ShmPtr)
    {
         munmap(m_ShmPtr, m_dSize);
    }

    shm_unlink(m_strShmName);
}

#endif
