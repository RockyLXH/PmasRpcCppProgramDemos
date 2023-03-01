/*
 * CMMCModbusBuffer.cpp
 *
 *  Created on: 26/04/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCModbusBuffer.hpp"

CMMCModbusBuffer::CMMCModbusBuffer() {
    m_inBuffer.refCnt = 0;
    m_inBuffer.startRef=0;
}
CMMCModbusBuffer::CMMCModbusBuffer(ELMO_INT32 iOffset, ELMO_INT32 iCount) {
    m_inBuffer.refCnt = iCount;
    m_inBuffer.startRef=iOffset;
}
CMMCModbusBuffer::~CMMCModbusBuffer() {}

void CMMCModbusBuffer::GetValue (ELMO_INT32 iIndex, ELMO_FLOAT & fValue)
{
    union
    {
        ELMO_FLOAT fvalue;
        ELMO_ULINT32 ulvalue;
    } unValue;
    unValue.ulvalue = (ELMO_UINT16)m_inBuffer.regArr[iIndex+1];
    unValue.ulvalue <<= 16;
    unValue.ulvalue |= (ELMO_LINT32)((ELMO_UINT16)m_inBuffer.regArr[iIndex]);
    fValue = unValue.fvalue;
}

void CMMCModbusBuffer::GetValue (ELMO_INT32 iIndex, ELMO_LINT32 & lValue)
{
    lValue = (ELMO_UINT16)m_inBuffer.regArr[iIndex] | (ELMO_LINT32)(m_inBuffer.regArr[iIndex+1] << 16);
}
void CMMCModbusBuffer::GetValue (ELMO_INT32 iIndex, ELMO_INT16 & sValue)
{
    sValue = m_inBuffer.regArr[iIndex];
}

void CMMCModbusBuffer::SetValue (ELMO_INT32 iIndex, ELMO_FLOAT fValue) {
    TypeToModbusShortArr(iIndex,  fValue) ;
}
void CMMCModbusBuffer::SetValue (ELMO_INT32 iIndex, ELMO_LINT32 lValue) {
    TypeToModbusShortArr(iIndex,  lValue) ;
}
void CMMCModbusBuffer::SetValue (ELMO_INT32 iIndex, ELMO_INT16 sValue) {
    m_inBuffer.regArr[iIndex]  =  sValue;
}

void CMMCModbusBuffer::TypeToModbusShortArr(ELMO_INT32 iIndex,  ELMO_FLOAT fVal) {
    union
    {
        ELMO_FLOAT fvalue;
        ELMO_ULINT32 ulvalue;
    } unValue;
    unValue.fvalue = fVal;
    m_inBuffer.regArr[iIndex]  =  (ELMO_INT16) (unValue.ulvalue & 0xFFFF);
    m_inBuffer.regArr[iIndex+1]= (ELMO_INT16)((unValue.ulvalue >> 16) & 0xFFFF);
}

void CMMCModbusBuffer::TypeToModbusShortArr(ELMO_INT32 iIndex,  ELMO_LINT32 lVal) {
    m_inBuffer.regArr[iIndex]  =  (ELMO_INT16) (lVal & 0xFFFF);
    m_inBuffer.regArr[iIndex+1]= (ELMO_INT16)((lVal >> 16) & 0xFFFF);
}

void CMMCModbusBuffer::TypeToModbusShortArr(ELMO_INT32 iIndex,  ELMO_INT16 sVal) {
    m_inBuffer.regArr[iIndex]  =  sVal;
}
