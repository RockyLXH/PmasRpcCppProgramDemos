/*
 * CMMCErrorCore.cpp
 *
 *  Created on: 05/07/2011
 *      Author: yuvall
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCErrorCorr.hpp"
#include "MMC_ErrorCorr_API.h"

CMMCErrorCorr::~CMMCErrorCorr() {
}

/**
 * \fn	LoadErrorCorrTable(MMC_LOADERRORTABLE_IN& stInParams, int* iErrLineNum)
 * \brief	this method loads an error correction table.
 * \param	stInParams	table parameters
 * \param	iErrLineNum out address to retrieve line number in case of loading error (failure).
 * \return	0 on success, otherwise error or exception thrown.
 */
ELMO_INT32 CMMCErrorCorr::LoadErrorCorrTable(MMC_LOADERRORTABLE_IN& stInParams, ELMO_PINT32 iErrLineNum) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    MMC_LOADERRORTABLE_OUT  stOutParams;

    if ( (rc = MMC_LoadErrorCorrTableCmd(m_uiConnHndl, &stInParams, &stOutParams)) !=0 ) {
		if (iErrLineNum!=NULL)
			*iErrLineNum = stOutParams.iProblematicLine;
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_LoadErrorCorrTableCmd:", m_uiConnHndl, 0, rc, stOutParams.usErrorID,  stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("LoadErrorCorrTable OK")
    return rc ;
 }

ELMO_INT32 CMMCErrorCorr::UnloadErrorCorrTable(NC_ERROR_TABLE_NUMBER eTableNumber) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    MMC_UNLOADERRORTABLE_IN stInParams;
    MMC_UNLOADERRORTABLE_OUT stOutParams;

    stInParams.eTableNumber = eTableNumber;

    if ( (rc = MMC_UnloadErrorCorrTableCmd(m_uiConnHndl, &stInParams, &stOutParams)) !=0 ) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_UnloadErrorCorrTableCmd:", m_uiConnHndl, 0, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("UnloadErrorCorrTable OK")
    return rc;
}

ELMO_INT32 CMMCErrorCorr::EnableErrorCorrTable(NC_ERROR_TABLE_NUMBER eTableNumber) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    MMC_ENABLEERRORTABLE_IN stInParams;
    MMC_ENABLEERRORTABLE_OUT stOutParams;

    stInParams.eTableNumber = eTableNumber;

    if ( (rc = MMC_EnableErrorCorrTableCmd(m_uiConnHndl, &stInParams, &stOutParams)) !=0 ) {
            CMMCPPGlobal::Instance()->MMCPPThrow("MMC_EnableErrorCorrTableCmd:", m_uiConnHndl, 0, rc, stOutParams.usErrorID,  stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("EnableErrorCorrTable OK")
    return rc;
}

ELMO_INT32 CMMCErrorCorr::DisableErrorCorrTable(NC_ERROR_TABLE_NUMBER eTableNumber) throw (CMMCException) {
    ELMO_INT32 rc = 0;
    MMC_DISABLEERRORTABLE_IN stInParams;
    MMC_DISABLEERRORTABLE_OUT stOutParams;

    stInParams.eTableNumber = eTableNumber;

    if ( (rc = MMC_DisableErrorCorrTableCmd(m_uiConnHndl, &stInParams, &stOutParams)) !=0 ) {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_DisableErrorCorrTableCmd:", m_uiConnHndl, 0, rc, stOutParams.usErrorID,  stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("DisableErrorCorrTable OK")
    return rc;
}



/**
 * \fn	LoadErrorCorrTable(char pPathToETFile[NC_MAX_ET_FILE_PATH_LENGTH], NC_ERROR_TABLE_NUMBER eETNumber, double dMaxCorrectionDelta = 0, int* iErrLineNum=NULL)
 * \brief	this method loads an error correction table.
 * \param	pPathToETFile path to table file on target controller.
 * \param	eETNumber table  index in journal.
 * \param	dMaxCorrectionDelta maximum allowed aberration(error delta).
 * \param	iErrLineNum out address to retrieve line number in case of loading error (failure).
 * 			default is NULL for backward compatibility.
 * \return	0 on success, otherwise error or exception thrown.
 */
void CMMCErrorCorr::LoadErrorCorrTable(ELMO_INT8 pPathToETFile[NC_MAX_ET_FILE_PATH_LENGTH], NC_ERROR_TABLE_NUMBER eETNumber, ELMO_DOUBLE dMaxCorrectionDelta, ELMO_PINT32 iErrLineNum) throw (CMMCException)
{
	MMC_LOADERRORTABLE_IN	stInParams;
	//
	strcpy(stInParams.pPathToETFile,pPathToETFile);
	stInParams.eETNumber = eETNumber;
	stInParams.dMaxCorrectionDelta = dMaxCorrectionDelta;
	//
	LoadErrorCorrTable(stInParams, iErrLineNum);
	//
	return;
 }

/**
 * \fn	LoadErrorCorrTable(NC_ERROR_TABLE_NUMBER eETNumber, double dMaxCorrectionDelta = 0, int* iErrLineNum=NULL)
 * \brief	this method loads an error correction table.
 * \param	eETNumber table  index in journal.
 * \param	dMaxCorrectionDelta maximum allowed aberration(error delta).  default is 0 as for no aberration allowed.
 * \param	iErrLineNum out address to retrieve line number in case of loading error (failure).
 * 			default is NULL for backward compatibility.
 * \return	0 on success, otherwise error or exception thrown.
 */
void CMMCErrorCorr::LoadErrorCorrTable(NC_ERROR_TABLE_NUMBER eETNumber, ELMO_DOUBLE dMaxCorrectionDelta, ELMO_PINT32 iErrLineNum) throw (CMMCException) 
{
    MMC_LOADERRORTABLE_IN   stInParams;
    //
    stInParams.pPathToETFile[0] = 0;
    stInParams.eETNumber = eETNumber;
    stInParams.dMaxCorrectionDelta = dMaxCorrectionDelta;
    //
    LoadErrorCorrTable(stInParams, iErrLineNum);
    //
    return;
 }

void CMMCErrorCorr::GetErrorTableStatus(NC_ERROR_TABLE_NUMBER eTableNumber, MMC_GETERRORTABLESTATUS_OUT &stOutParams)
{
    ELMO_INT32 rc = 0;
    MMC_GETERRORTABLESTATUS_IN stInParams;
    //
    stInParams.eTableNumber = eTableNumber;
    //
    if ( (rc = MMC_GetErrorTableStatusCmd(m_uiConnHndl, &stInParams, &stOutParams)) !=0 )
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetErrorTableStatusCmd:", m_uiConnHndl, 0, rc, stOutParams.usErrorID,  stOutParams.usStatus);
    }
    //
    // Benji: 10.2.2014.
    // If the table is not loaded, then it is not enabled either ...
    if(!stOutParams.ucIsTableLoaded)
    {
        stOutParams.ucIsTableEnabled = 0 ;
    }
    _MMCPP_TRACE_INFO("GetErrorTableStatus OK")
}

ELMO_BOOL CMMCErrorCorr::IsTableEnabled(NC_ERROR_TABLE_NUMBER eETNumber) throw (CMMCException)
{
    //
    MMC_GETERRORTABLESTATUS_OUT stOutParams;
    //
    GetErrorTableStatus(eETNumber,stOutParams);
    //
    if(!stOutParams.ucIsTableEnabled)
    {
        return false;
    }
    else
    {
        return true;
    }   
}

ELMO_BOOL CMMCErrorCorr::IsTableLoaded(NC_ERROR_TABLE_NUMBER eETNumber) throw (CMMCException)
{   
    //
    MMC_GETERRORTABLESTATUS_OUT stOutParams;
    //
    GetErrorTableStatus(eETNumber,stOutParams);
    //
    if(!stOutParams.ucIsTableLoaded)
    {
        return false;
    }
    else
    {
        return true;
    }   
}
