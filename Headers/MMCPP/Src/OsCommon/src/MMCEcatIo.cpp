/*
 * CMMCECATIO.cpp
 *
 *  Created on: 27/06/2011
 *      Author: yuvall
 */
// #include "stdafx.h"

#include "OS_PlatformDependSetting.hpp"
#include "MMCEcatIo.hpp"

CMMCECATIO::CMMCECATIO() {
    // TODO Auto-generated constructor stub

}

CMMCECATIO::~CMMCECATIO() {
    // TODO Auto-generated destructor stub
}


void CMMCECATIO::ECATIODisableDIChangedEvent() throw (CMMCException) {
    MMC_DISABLEDICHANGEDEVENT_IN stInParams;
    MMC_DISABLEDICHANGEDEVENT_OUT stOutParams;
    ELMO_INT32 rc = 0;

    if ((rc = MMC_ECATIODisableDIChangedEvent(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ECATIODisableDIChangedEvent failed", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ECATIODisableDIChangedEvent: %d, %s, %d, %d, %d\n", rc,  m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return;
}

void CMMCECATIO::ECATIOEnableDIChangedEvent() throw (CMMCException) {
    MMC_ENABLEDICHANGEDEVENT_IN stInParams;
    MMC_ENABLEDICHANGEDEVENT_OUT stOutParams;
    ELMO_INT32 rc = 0;

    if ((rc = MMC_ECATIOEnableDIChangedEvent(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ECATIOEnableDIChangedEvent failed", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ECATIOEnableDIChangedEvent: %d, %s, %d, %d, %d\n", rc,  m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return;
}

MMCPPULL_T CMMCECATIO::ECATIOReadDigitalInput() throw (CMMCException) {
    MMC_READDI_IN stInParams;
    MMC_READDI_OUT stOutParams;
    ELMO_INT32 rc = 0;

    if ((rc = MMC_ECATIOReadDigitalInput (m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ECATIOReadDigitalInput failed", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ECATIOReadDigitalInput: %d, %s, %d, %d, %d\n", rc,  m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return stOutParams.ulliDI;
}

void CMMCECATIO::ECATIOWriteDigitalOutput(MMCPPULL_T  ulliDO) throw (CMMCException) {
    MMC_WRITEDO_IN stInParams;
    MMC_WRITEDO_OUT stOutParams;
    int rc = 0;

    stInParams.ulliDO = ulliDO;
    if ((rc = MMC_ECATIOWriteDigitalOutput (m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ECATIOWriteDigitalOutput failed", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ECATIOWriteDigitalOutput: %d, %s, %d, %d, %d\n", rc,  m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return;
}

ELMO_INT16 CMMCECATIO::ECATIOReadAnalogInput(ELMO_UINT8 ucIndex) throw (CMMCException) {
    MMC_READAI_IN  stInParams;
    MMC_READAI_OUT  stOutParams;
    ELMO_INT32 rc = 0;

    stInParams.ucIndex = ucIndex;
    if ((rc = MMC_ECATIOReadAnalogInput (m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ECATIOReadAnalogInput failed", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ECATIOReadAnalogInput: %d, %s, %d, %d, %d\n", rc,  m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return stOutParams.sAI;
}

void CMMCECATIO::ECATIOWriteAnalogOutput(ELMO_UINT8 ucIndex, ELMO_INT16 sAOValue) throw (CMMCException) {
    MMC_WRITEAO_IN stInParams;
    MMC_WRITEAO_OUT stOutParams;

    stInParams.sAO = sAOValue;
    stInParams.ucIndex = ucIndex;

    ELMO_INT32 rc = 0;
    if ((rc = MMC_ECATIOWriteAnalogOutput (m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_ECATIOWriteAnalogOutput failed", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE("ECATIOWriteAnalogOutput: %d, %s, %d, %d, %d\n", rc,  m_strName, m_usAxisRef, stOutParams.usErrorID, stOutParams.usStatus);
    return;
}
