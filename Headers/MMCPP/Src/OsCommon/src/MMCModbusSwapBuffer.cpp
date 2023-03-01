#/*
 * CMMCModbusSwapBuffer.cpp
 *
 *  Created on: 26/04/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *              0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCModbusSwapBuffer.hpp"


#ifdef PROAUT_CHANGES
	//save compiler switches
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


CMMCModbusSwapBuffer::CMMCModbusSwapBuffer() {
    m_inBuffer.refCnt=0;
    m_inBuffer.startRef = 0;
}
CMMCModbusSwapBuffer::CMMCModbusSwapBuffer(int iOffset, ELMO_INT32 iCount) {
    m_inBuffer.refCnt=iCount;
    m_inBuffer.startRef = iOffset;
}

void CMMCModbusSwapBuffer::SetValue (ELMO_INT32 iIndex, ELMO_LINT32 lValue) {
    m_inBuffer.regArr[iIndex+1]  =  (ELMO_INT16) (lValue & 0xFFFF);
    m_inBuffer.regArr[iIndex]= (ELMO_INT16)((lValue >> 16) & 0xFFFF);

}
void CMMCModbusSwapBuffer::SetValue (ELMO_INT32 iIndex, ELMO_FLOAT fValue) {
    //unsigned long ulValue = *((unsigned long *)(&fValue));

    // ELMO_ULINT32 ulValue ;
    union
        {
            ELMO_FLOAT fvalue;
            ELMO_ULINT32 ulvalue;
        } unValue;
        unValue.fvalue = fValue;

    // ulValue = unValue.ulvalue ;
    m_inBuffer.regArr[iIndex+1]  =  (ELMO_INT16) (unValue.ulvalue & 0xFFFF);
    m_inBuffer.regArr[iIndex]= (ELMO_INT16)((unValue.ulvalue >> 16) & 0xFFFF);
}

void CMMCModbusSwapBuffer::GetValue (ELMO_INT32 iIndex, ELMO_LINT32 & lValue)
{
    lValue = (ELMO_LINT32)m_inBuffer.regArr[iIndex+1] | (ELMO_LINT32)(m_inBuffer.regArr[iIndex] << 16);
}
void CMMCModbusSwapBuffer::GetValue (ELMO_INT32 iIndex, ELMO_FLOAT & fValue)
{
    union
    {
        ELMO_FLOAT fvalue;
        ELMO_ULINT32 ulvalue;
    }unValue;
    //unValue.fvalue = fValue ;
    ELMO_ULINT32 ulValue = (ELMO_UINT16)m_inBuffer.regArr[iIndex];
    //unsigned long ulValue = unValue.ulvalue ;
    ulValue <<= 16;
    ulValue |= (ELMO_ULINT32)((ELMO_UINT16)m_inBuffer.regArr[iIndex+1]);

    unValue.ulvalue = ulValue ;

    fValue = unValue.fvalue;
}


#ifdef PROAUT_CHANGES
	//restore compiler switches
	#pragma GCC diagnostic pop
#endif

