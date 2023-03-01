/*
 * MMCMotionAxis.cpp
 *
 *  Created on: 20/05/2012
 *      Author: Alex
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *				  0.4.0 Updated 11Sep2017 Haim H.
 */
 
#include "OS_PlatformDependSetting.hpp"
#include "MMCMotionAxis.hpp"


CMMCMotionAxis::CMMCMotionAxis()
{
}

CMMCMotionAxis::CMMCMotionAxis(MMC_CONNECT_HNDL uiConnHndl)
{
    SetConnHndl(uiConnHndl);
}
CMMCMotionAxis::~CMMCMotionAxis()
{
}

MC_PATH_REF CMMCMotionAxis::InitPVTTable(ELMO_ULINT32 ulMaxPoints,
                           ELMO_ULINT32 ulUnderflowThreshold,
                           ELMO_UINT8 ucIsCyclic,
                           ELMO_UINT8 ucIsPosAbsolute,
                           ELMO_UINT16 usDimension,MC_COORD_SYSTEM_ENUM eCoordSystem,
                           NC_MOTION_TABLE_TYPE_ENUM eTableMode) throw (CMMCException)
{
    MMC_INITTABLE_IN stInParam;
    MMC_INITTABLE_OUT stOutParam;
    ELMO_INT32 rc = NC_OK;
    //
    stInParam.eTableType = eTableMode;//eNC_TABLE_PVT_ARRAY; 28/12/2014 @YL
    stInParam.ucIsCyclic = ucIsCyclic;
    stInParam.ucIsDynamicMode = 1;
    stInParam.ucIsPosAbsolute = ucIsPosAbsolute;
    stInParam.ulMaxNumberOfPoints = ulMaxPoints;
    stInParam.ulUnderflowThreshold = ulUnderflowThreshold;
    stInParam.usDimension = usDimension;
    stInParam.usAxisRef = m_usAxisRef;
    stInParam.eCoordSystem = eCoordSystem;
    //
    rc = MMC_InitTableCmd(m_uiConnHndl, &stInParam, &stOutParam);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPVT::InitPVTTable", m_uiConnHndl, m_usAxisRef, (ELMO_INT32)(-1), stOutParam.usErrorID, stOutParam.usStatus);
    }
    return stOutParam.hMemHandle;
}

/*! \fn MC_PATH_REF InitPTTable()
    * \brief This function allocates and init PT (online spline) table on GMAS
    * \param ulMaxPoints - Maximum points for one table
    * \param ulUnderflowThreshold - Upon reaching this threshold, an event will be generated
    * \param ucIsCyclic - Is the table cyclic
    * \param ucIsDynamic - Is dynamic append allowed
    * \param ucIsPosAbsolute - Are the position units absolute
    * \param eSplineMode    - defines the way spline if calculated (FT/VT/CV_DWELL)
    * \parma ConstVelocity  - constant velocity is forced on all segments
    * \parma FixedType  - constant time (ms) is forced on all segments
    * \return void
*/
MC_PATH_REF CMMCMotionAxis::InitPTTable(ELMO_ULINT32 ulMaxPoints,
                           ELMO_ULINT32 ulUnderflowThreshold,
                           ELMO_UINT8 ucIsCyclic,
                           ELMO_UINT8 ucIsDynamic,
                           ELMO_UINT8 ucIsPosAbsolute,
                           ELMO_UINT16 usDimension,
                           MC_COORD_SYSTEM_ENUM eCoordSystem,
                           NC_ONLINE_SPLINE_MODE_ENUM eSplineMode,
                           ELMO_DOUBLE dbConstVelocity,
                           ELMO_DOUBLE dbFixedTime) throw (CMMCException)
{
    MMC_INITTABLEEX_IN stInParam;
    MMC_INITTABLEEX_OUT stOutParam;
    ELMO_INT32 rc = NC_OK;
    //
    stInParam.eTableType = eNC_TABLE_OLSPLN_ARRAY;
    stInParam.ucIsCyclic = ucIsCyclic;
    stInParam.ucIsDynamicMode = ucIsDynamic; 
    stInParam.ucIsPosAbsolute = ucIsPosAbsolute;
    stInParam.ulMaxNumberOfPoints = ulMaxPoints;
    stInParam.ulUnderflowThreshold = ulUnderflowThreshold;
    stInParam.usDimension = usDimension;
    stInParam.usAxisRef = m_usAxisRef;
    stInParam.eCoordSystem = eCoordSystem;
    stInParam.eSplineMode = eSplineMode;
    stInParam.dbConstVelocity = dbConstVelocity;
    stInParam.dbConstTime = dbFixedTime;
    //
    rc = MMC_InitTableExCmd(m_uiConnHndl, &stInParam, &stOutParam);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCPVT::InitPTTable", m_uiConnHndl, m_usAxisRef, (ELMO_INT32)(-1), stOutParam.usErrorID, stOutParam.usStatus);
    }
    return stOutParam.hMemHandle;
}




MC_PATH_REF CMMCMotionAxis::CamTableInit(ELMO_ULINT32 ulMaxPoints,
                           ELMO_UINT16 usDimension,
                           ELMO_UINT8 ucIsFixedGap,
                           CURVE_TYPE_ENUM eCurveType) throw (CMMCException)
{
    MMC_CAMTABLEINIT_IN stInParam;
    MMC_INITTABLE_OUT stOutParam;
    ELMO_INT32 rc = NC_OK;
    stInParam.ulMaxNumberOfPoints = ulMaxPoints;
    stInParam.ulUnderflowThreshold = 0;
    stInParam.usDimension = usDimension;
    stInParam.ucIsFixedGap = ucIsFixedGap;
    stInParam.eCurveType = eCurveType;
    rc = MMC_CamTableInitCmd(m_uiConnHndl, &stInParam, &stOutParam);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCMotionAxis::InitECAMTable", m_uiConnHndl, (ELMO_INT32)(-1), (ELMO_INT32)(-1), stOutParam.usErrorID, stOutParam.usStatus);
    }
    return stOutParam.hMemHandle;
}
ELMO_INT32 CMMCMotionAxis::CamOut()  throw (CMMCException)
{
    ELMO_INT32 rc=0;
    MMC_CAMOUT_IN stInParams;
    MMC_CAMOUT_OUT stOutParams;
    stInParams.ucExecute = 1;
    rc = MMC_CamOutCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamOutCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
    }
    return rc;
}

MC_PATH_REF CMMCMotionAxis::CamTableSelect(const MC_CamRef& CamTableDescr,
                    ELMO_UINT8 ucIsMasterPosAbsolute,
                    ELMO_UINT8 ucIsSlavePosAbsolute,
                    ELMO_UINT32 uiStartMode) throw (CMMCException)
{
    MMC_CAMTABLESELECT_IN stInParams;
    MMC_CAMTABLESELECT_OUT stOutParams;
    ELMO_INT32 rc;
    memcpy(&stInParams.CamTable, &CamTableDescr, sizeof(MC_CamRef));
    stInParams.ucIsMasterPosAbsolute = ucIsMasterPosAbsolute;
    stInParams.ucIsSlavePosAbsolute = ucIsSlavePosAbsolute;
    stInParams.uiStartMode = uiStartMode;
    rc = MMC_CamTableSelectCmd(m_uiConnHndl, &stInParams, &stOutParams);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamTableSelectCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    return stOutParams.hMemHandle;
}
ELMO_INT32 CMMCMotionAxis::CamTableAdd(MC_PATH_REF hMemHandle, ELMO_PDOUBLE dbTable, ELMO_UINT16 usColumns, ELMO_ULINT32 ulNumberOfPoints) throw (CMMCException)
{
    MMC_CAMTABLESET_IN stInParams;
    MMC_CAMTABLESET_OUT stOutParams;
    ELMO_INT32 iSize, iBytes, rc;
    stInParams.hMemHandle = hMemHandle;
    iSize = (usColumns * ulNumberOfPoints);
    iBytes =  (iSize > NC_ECAM_MAX_ARRAY_SIZE) ? NC_ECAM_MAX_ARRAY_SIZE * sizeof(ELMO_DOUBLE) :  iSize * sizeof(ELMO_DOUBLE);
    memcpy(stInParams.dbTable, dbTable, iBytes);
    stInParams.ulNumberOfPoints = ulNumberOfPoints;
	stInParams.ulStartIndex = 0; //irrelevant for auto append
    rc = MMC_CamTableAddCmd(m_uiConnHndl, &stInParams, &stOutParams);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamTableAddCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    return rc;
}

/**
* \fn    ELMO_INT32 CamTableAddEx(MC_PATH_REF hMemHandle, double *dbTable, unsigned short usColumns, ELMO_ULINT32 ulNumberOfPoELMO_INT32s)
* \brief    add multiple buffers, max size each.
* \param    hMemHandle table handler of a valid table.
* \param    dbTable CAM table represent as array of double.
* \param    usColumns number of columns. depends on the existence of master and/or curve type columns.
* \param    ulNumberOfPoELMO_INT32s number of rows (poELMO_INT32s) in table.
* \return   0 if completed successfully, otherwise error.
*/
ELMO_INT32 CMMCMotionAxis::CamTableAddEx(MC_PATH_REF hMemHandle, ELMO_PDOUBLE dbTable, ELMO_UINT16 usColumns, ELMO_ULINT32 ulNumberOfPoints)
{
    ELMO_INT32 rc = 0;
    /*
     * constants
     * NC_ECAM_MAX_ARRAY_SIZE max number of doubles to send on each iteration of TCP message
     */
    ELMO_INT32 iMaxRows = NC_ECAM_MAX_ARRAY_SIZE / usColumns;              //max number of poELMO_INT32s respective to max size of message
    ELMO_INT32 iIterations = 
        usColumns *  ulNumberOfPoints / NC_ECAM_MAX_ARRAY_SIZE +1;  //number of iteration to send the complete list of poELMO_INT32s (+1 for partial package at the end)
    /*
     * variables
     */
    ELMO_INT32 iRemainingRows;                                                 //remaining number of poELMO_INT32s (rows) to send
    ELMO_INT32 iRowsNumber;                                                    //number of poELMO_INT32s (rows) to send on current iteration.
    ELMO_INT32 iRowIndex = 0;                                                  //rows aggregator for in current array. reset when array is switched.
    for (ELMO_INT32 i = 0; (i < iIterations) && !rc; ++i) {
        if ( (iRemainingRows = (ulNumberOfPoints  - iRowIndex)) == 0)   //quit if nothing left
            break;
        iRowsNumber = (iRemainingRows > iMaxRows) ? iMaxRows :  iRemainingRows;
        rc = this->CamTableAdd(hMemHandle, &dbTable[iRowIndex*usColumns], usColumns, iRowsNumber);  //allwayes auto append
        iRowIndex += iRowsNumber;
    }

    return rc;
}


ELMO_INT32 CMMCMotionAxis::CamTableSet(MC_PATH_REF hMemHandle, ELMO_PDOUBLE dbTable, ELMO_UINT16 usColumns, ELMO_ULINT32 ulStartIndex, ELMO_ULINT32 ulNumberOfPoints) throw (CMMCException)
{
    MMC_CAMTABLESET_IN stInParams;
    MMC_CAMTABLESET_OUT stOutParams;
    ELMO_INT32 iSize, iBytes, rc;
    stInParams.hMemHandle = hMemHandle;
	iSize = (usColumns * ulNumberOfPoints);
	iBytes = (iSize > NC_ECAM_MAX_ARRAY_SIZE) ? NC_ECAM_MAX_ARRAY_SIZE * sizeof(ELMO_DOUBLE) :  iSize * sizeof(ELMO_DOUBLE);
	memcpy(stInParams.dbTable, dbTable, iBytes);
	stInParams.ulNumberOfPoints = ulNumberOfPoints;
	stInParams.ulStartIndex = ulStartIndex;
	rc = MMC_CamTableSetCmd(m_uiConnHndl, &stInParams, &stOutParams);
	if (NC_OK != rc)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamTableSetCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
	}
	return rc;
}
/**
* \fn   ELMO_INT32 CamGetStatus(unsigned long ulEndOfProfile,
                    unsigned long ulCurrentIndex,
                    unsigned long ulCycle,
                    unsigned short usStatus,
                    short sErrorID) throw (CMMCException)
* \brief    retrieve status of MC_CamIn FB on this axis and other parameters as given on this list
* \param    ulEndOfProfile counts number of exists from cam table.
* \param    ulCurrentIndex returned segment (index) in CAM table, wich is currently proccessed.
* \param    ulCycle     returned process cycle  of CamIn on this axis
* \param    usStatus    returned FB status
* \param    sErrorID    returned with error number if error
* \return   0 if completed successfully, otherwise error or thrown exception.
*/
ELMO_INT32 CMMCMotionAxis::CamGetStatus(ELMO_ULINT32& ulEndOfProfile,
                    ELMO_ULINT32& ulCurrentIndex,
                    ELMO_ULINT32& ulCycle,
                    ELMO_UINT16& usStatus,
                    ELMO_INT16& sErrorID) throw (CMMCException)
{
    ELMO_INT32 rc=0;
    MMC_CAMSTATUS_IN stInParams;
    MMC_CAMSTATUS_OUT stOutParams;
    rc = MMC_CamStatusCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamStatusCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
    } else {
        ulEndOfProfile = stOutParams.ulEndOfProfile;
        ulCurrentIndex = stOutParams.ulCurrentIndex;
        ulCycle = stOutParams.ulCycle;
        usStatus = stOutParams.usStatus;
        sErrorID = stOutParams.sErrorID;
    }
    return rc;
}

	/**
	* \fn	int CamGetStatus(unsigned long ulEndOfProfile,
					unsigned long ulCurrentIndex,
					unsigned long ulCycle,
					unsigned short usStatus,
					short sErrorID) throw (CMMCException)
	* \brief	retrieve status of MC_CamIn FB on this axis and other parameters as given on this list
	* \param	ulEndOfProfile counts number of exists from cam table.
	* \param	ulCurrentIndex returned segment (index) in CAM table, wich is currently proccessed.
	* \param	ulCycle		returned process cycle  of CamIn on this axis
	* \param	usStatus	returned FB status
	* \param	sErrorID	returned with error number if error
	* \param	uiCamTableID return table handler if FB is active.
	* \return	0 if completed successfully, otherwise error or thrown exception.
	*/
ELMO_INT32 CMMCMotionAxis::CamGetStatus(ELMO_ULINT32& ulEndOfProfile,
					ELMO_ULINT32& ulCurrentIndex,
					ELMO_ULINT32& ulCycle,
					ELMO_UINT16 & usStatus,
					ELMO_INT16 & sErrorID, ELMO_UINT32& uiCamTableID) throw (CMMCException)
{
	ELMO_INT32 rc=0;
	MMC_CAMSTATUS_IN stInParams;
	MMC_CAMSTATUS_OUT stOutParams;
	rc = MMC_CamStatusCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
	if (NC_OK != rc)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamStatusCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
	} else {
		ulEndOfProfile = stOutParams.ulEndOfProfile;
		ulCurrentIndex = stOutParams.ulCurrentIndex;
		ulCycle = stOutParams.ulCycle;
		usStatus = stOutParams.usStatus;
		sErrorID = stOutParams.sErrorID;
		uiCamTableID = stOutParams.uiCamTableID;
	}
	return rc;
}

/**
* \fn		int CamSetProperty() throw (CMMCException)
* \brief	modifies the specified property in ata structure of active ECAM FB.
* \param	eProperty	the property to be changed
* \param	value		new value to set the property
* \return	0 if completed successfully, otherwise error or thrown exception.
*/
int CMMCMotionAxis::CamSetProperty(ECAM_PROPERTIES_ENUM eProperty, ECAM_PERIODIC_ENUM ePeriodicMode) throw (CMMCException)
{
    ELMO_INT32 rc=0;
    MMC_CAMSETPROP_IN stInParams;
    MMC_CAMSETPROP_OUT stOutParams;

    stInParams.eProperty = eProperty;
    stInParams.value.ePeriodicMode = ePeriodicMode;
    stInParams.ucExecute =  1;

    if ( (rc = MMC_CamSetPropertyCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams)) != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CamSetProperty", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
    }
    return rc;
}

//#ifdef _DEBUG
//ELMO_INT32 CMMCMotionAxis::CamTablePrELMO_INT32(MC_PATH_REF hMemHandle, unsigned long ulStartIndex, unsigned long ulNumberOfPoELMO_INT32s) throw (CMMCException)
//{
//  MMC_CAMTABLESET_IN stInParams;
//  MMC_CAMTABLESET_OUT stOutParams;
//  ELMO_INT32 rc;
//  stInParams.hMemHandle = hMemHandle;
//  stInParams.ulNumberOfPoELMO_INT32s = ulNumberOfPoELMO_INT32s;
//  stInParams.ulStartIndex = ulStartIndex;
//  rc = MMC_CamTablePrELMO_INT32Cmd(m_uiConnHndl, &stInParams, &stOutParams);
//  if (NC_OK != rc)
//  {
//      CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamTablePrELMO_INT32Cmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
//  }
//  return 0;
//}
//#endif
ELMO_INT32 CMMCMotionAxis::CamIn(
        ELMO_UINT16 usMaster,
        MC_BUFFERED_MODE_ENUM eBufferMode,
        ELMO_UINT32 uiCamTableID,
        CURVE_TYPE_ENUM eCurveType,
        ELMO_UINT32 ucAutoOffset,
        ECAM_PERIODIC_ENUM ePeriodic,
        ELMO_DOUBLE dbMasterSyncPosition,
        ELMO_DOUBLE dbMasterStartDistance,
        ELMO_UINT32 uiStartMode,
        ELMO_DOUBLE dbMasterOffset,
        ELMO_DOUBLE dbSlaveOffset,
        ELMO_DOUBLE dbMasterScaling,
        ELMO_DOUBLE dbSlaveScaling,
        ECAM_VALUE_SRC_ENUM eMasterValueSource) throw (CMMCException)
{
    ELMO_INT32 rc=0;
    MMC_CAMIN_IN stInParams;
    MMC_CAMIN_OUT stOutParams;
    stInParams.usMaster = usMaster;
    stInParams.eBufferMode = eBufferMode;
    stInParams.uiStartMode = uiStartMode;
    stInParams.uiCamTableID = uiCamTableID;
    stInParams.ucAutoOffset = ucAutoOffset;
    stInParams.ePeriodicMode = (ECAM_PERIODIC_ENUM)ePeriodic;
    stInParams.dbMasterSyncPosition = dbMasterSyncPosition;
    stInParams.dbMasterOffset = dbMasterOffset;
    stInParams.dbSlaveOffset = dbSlaveOffset ;
    stInParams.dbMasterScaling = dbMasterScaling;
    stInParams.dbSlaveScaling = dbSlaveScaling;
    stInParams.dbMasterStartDistance = dbMasterStartDistance;
    stInParams.eMasterValueSource = eMasterValueSource;
    stInParams.eCurveType =  eCurveType;//eUserDefInterp (0)//eQuELMO_INT32icInterp (1);//eLinearInterp (2); 
    stInParams.ucExecute = 1;

    rc = MMC_CamInCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_CamInCmd", m_uiConnHndl, m_usAxisRef, rc, stOutParams.sErrorID, stOutParams.usStatus);
    }
    return rc;
}
MC_PATH_REF CMMCMotionAxis::LoadPVTTableFromFile(ELMO_PINT8 pFileName,
        MC_COORD_SYSTEM_ENUM eCoordSystem,
        NC_MOTION_TABLE_TYPE_ENUM eTableMode) throw (CMMCException)
{
    MMC_LOADTABLEFROMFILE_IN stInParam;
    MMC_LOADTABLE_OUT stLoadTableOut;
    ELMO_INT32 rc;
    //
    stInParam.eTableType = eTableMode; //eNC_TABLE_PVT_FILE; 28/12/2014 @YL
    stInParam.usAxisRef = m_usAxisRef;
    stInParam.eCoordSystem = eCoordSystem;
    strcpy(stInParam.pPathToTableFile, pFileName);
    //
    rc = MMC_LoadTableFromFileCmd(m_uiConnHndl, &stInParam, &stLoadTableOut);

    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("LoadPVTTableFromFile", m_uiConnHndl, m_usAxisRef, rc, stLoadTableOut.usErrorID, stLoadTableOut.usStatus);
    }
    return stLoadTableOut.hMemHandle;
}

void CMMCMotionAxis::MovePVT(MC_PATH_REF hMemHandle, MC_COORD_SYSTEM_ENUM eCoordSystem) throw (CMMCException)
{
    ELMO_INT32 rc = NC_OK;
    MMC_MOVETABLE_IN stMoveTableIn;
    MMC_MOVETABLE_OUT stMoveTableOut;
    //
    stMoveTableIn.eBufferMode = MC_BUFFERED_MODE;
    stMoveTableIn.eCoordSystem = eCoordSystem;
    stMoveTableIn.eTransitionMode = MC_TM_NONE_MODE;
    stMoveTableIn.hMemHandle = hMemHandle;
    stMoveTableIn.ucExecute = 1;
    stMoveTableIn.ucSuperImposed = 0;
    memset(stMoveTableIn.fTransitionParameter, 0, NC_MAX_NUM_AXES_IN_NODE * sizeof(ELMO_FLOAT));
    //
    rc = MMC_MoveTableCmd(m_uiConnHndl, m_usAxisRef, &stMoveTableIn, &stMoveTableOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCMotionAxis::MovePVT", m_uiConnHndl, m_usAxisRef, rc, stMoveTableOut.usErrorID, stMoveTableOut.usStatus);
    }
    //
    return;
}

void CMMCMotionAxis::UnloadPVTTable(MC_PATH_REF hMemHandle) throw (CMMCException)
{
    ELMO_INT32 rc = NC_OK;
    MMC_UNLOADTABLE_IN stUnloadTableIn;
    MMC_UNLOADTABLE_OUT stUnloadTableOut;
    //
    stUnloadTableIn.hMemHandle = hMemHandle;
    stUnloadTableIn.ucExecute = 1;
    //
    rc = MMC_UnloadTableCmd(m_uiConnHndl, &stUnloadTableIn, &stUnloadTableOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCMotionAxis::UnloadPVTTable",m_uiConnHndl, m_usAxisRef, rc, stUnloadTableOut.usErrorID, stUnloadTableOut.usStatus);
    }
    return;
}

/*! \fn void MovePT()
* \brief This function moves along the PT table (online spline)
* \param hMemHandle - table access handler (index to journal on GMAS)
* \param eCoordSystem - Coordinate system - not relevant in case of single axis
* \return 0 if completed successfully, otherwise error
*/
ELMO_INT32 CMMCMotionAxis::MovePT(MC_PATH_REF hMemHandle, MC_COORD_SYSTEM_ENUM eCoordSystem) throw (CMMCException)
{
    ELMO_INT32 rc = NC_OK;
    MMC_MOVETABLE_IN stMoveTableIn;
    MMC_MOVETABLE_OUT stMoveTableOut;
    //
    stMoveTableIn.eBufferMode = MC_BUFFERED_MODE;
    stMoveTableIn.eCoordSystem = eCoordSystem;
    stMoveTableIn.eTransitionMode = MC_TM_NONE_MODE;
    stMoveTableIn.hMemHandle = hMemHandle;
    stMoveTableIn.ucExecute = 1;
    stMoveTableIn.ucSuperImposed = 0;
    memset(stMoveTableIn.fTransitionParameter, 0, NC_MAX_NUM_AXES_IN_NODE * sizeof(ELMO_FLOAT));
    //
    rc = MMC_MoveTableCmd(m_uiConnHndl, m_usAxisRef, &stMoveTableIn, &stMoveTableOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("CMMCMotionAxis::MovePT", m_uiConnHndl, m_usAxisRef, rc, stMoveTableOut.usErrorID, stMoveTableOut.usStatus);
    }
    //
    return rc;
}


/*! \fn void AppendPTPoints()
* \brief This function appends points to an existing table
* \param hMemHandle - table handler
* \param dTable - and array contains ulNumberOfPoints points (rows)
* \param ulNumberOfPoints - numbe of points to append (set) to (in) table on GMAS.
* \param ucAutoAppend - flag for automatic append. if true, GMAS ignores ulStartIndex
                        and appends all points to the end of existing table.
* \param ulStartIndex - table segment on GMAS from which to start the append process.
* \param ucIsTimeAbsolute - tells GMAS whether the time column is absolute or relative (absolute or relative to previous row).
*                            if absolute the its relative to FB activation time.
* \return void
*/
void CMMCMotionAxis::AppendPTPoints(MC_PATH_REF hMemHandle,
                ELMO_DOUBLE dTable[NC_PVT_ECAM_MAX_ARRAY_SIZE],
                ELMO_ULINT32 ulNumberOfPoints,
                ELMO_UINT8 ucAutoAppend,
                ELMO_ULINT32 ulStartIndex,
                ELMO_UINT8 ucIsTimeAbsolute) throw (CMMCException)
{
    MMC_APPENDPOINTSTOTABLE_IN stInParam;           
    MMC_APPENDPOINTSTOTABLE_OUT stAppendToTableOut;
    ELMO_INT32 rc = NC_OK;
    //
    memcpy(stInParam.dTable, dTable, NC_PVT_ECAM_MAX_ARRAY_SIZE * sizeof(ELMO_DOUBLE));
    stInParam.eTableType = eNC_TABLE_OLSPLN_ARRAY;
    stInParam.hMemHandle = hMemHandle;
    stInParam.ucIsAutoAppend = ucAutoAppend;
    stInParam.ucIsTimeAbsolute = ucIsTimeAbsolute;
    stInParam.ulNumberOfPoints = ulNumberOfPoints;
    stInParam.ulStartIndex = ulStartIndex;
    stInParam.usAxisRef = m_usAxisRef;
    //
    rc = MMC_AppendPointsToTableCmd(m_uiConnHndl, &stInParam, &stAppendToTableOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("AppendPointsToTableCmn", m_uiConnHndl, m_usAxisRef, rc, stAppendToTableOut.usErrorID, stAppendToTableOut.usStatus);
    }
    return;
}

/*! \fn void LoadPTTable()
* \brief This function loads PT (online spline) table from file
* \param char* pFileName - File Name
* \param eCoordSystem coordinate system (ACS/MCS/PCS or NONE)
* \return   - handler (>=0) of loaded table if loaded successfully otherwise 
            throws an exception or returns an error (<0).
*/
MC_PATH_REF CMMCMotionAxis::LoadPTTableFromFile(ELMO_PINT8 pFileName,
            MC_COORD_SYSTEM_ENUM eCoordSystem) throw (CMMCException)
{
    return LoadPVTTableFromFile(pFileName, eCoordSystem, eNC_TABLE_OLSPLN_FILE);
}


void CMMCMotionAxis::AppendPVTPoints(MC_PATH_REF hMemHandle,
											ELMO_DOUBLE (&dTable)[NC_PVT_ECAM_MAX_ARRAY_SIZE],
											ELMO_ULINT32 ulNumberOfPoints,
											ELMO_ULINT32 ulStartIndex,
											ELMO_UINT8   ucIsAutoAppend,
											ELMO_UINT8   ucIsTimeAbsolute,
											NC_MOTION_TABLE_TYPE_ENUM eTableType) throw (CMMCException)
{
    MMC_APPENDPOINTSTOTABLE_IN stInParam;           
    MMC_APPENDPOINTSTOTABLE_OUT stAppendToTableOut;
    ELMO_INT32 rc = NC_OK;
    //
    memcpy(stInParam.dTable, dTable, NC_PVT_ECAM_MAX_ARRAY_SIZE * sizeof(ELMO_DOUBLE));
    stInParam.eTableType = eTableType;
    stInParam.hMemHandle = hMemHandle;
    stInParam.ucIsAutoAppend = ucIsAutoAppend;
    stInParam.ucIsTimeAbsolute = ucIsTimeAbsolute;
    stInParam.ulNumberOfPoints = ulNumberOfPoints;
    stInParam.ulStartIndex = ulStartIndex;
    stInParam.usAxisRef = m_usAxisRef;
    //
    rc = MMC_AppendPointsToTableCmd(m_uiConnHndl, &stInParam, &stAppendToTableOut);
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("AppendPointsToTableCmn", m_uiConnHndl, m_usAxisRef, rc, stAppendToTableOut.usErrorID, stAppendToTableOut.usStatus);
    }
    return;
}



/*! \fn void AppendPoint2sToTableCmn
    * \brief This function appends points to an existing table
    * \param CMMCAxis* - Pointer to axis
    * \param cName - Axis name
    * \param hConn - Connection handle
    * \return void
    */
void CMMCMotionAxis::AppendPVTPoints(MC_PATH_REF hMemHandle,
							ELMO_DOUBLE (&dTable)[NC_PVT_ECAM_MAX_ARRAY_SIZE],
							ELMO_ULINT32 ulNumberOfPoints,
							ELMO_UINT8   ucIsTimeAbsolute,
							NC_MOTION_TABLE_TYPE_ENUM eTableType) throw (CMMCException)
{
	AppendPVTPoints(hMemHandle,dTable,ulNumberOfPoints,0,1,ucIsTimeAbsolute, eTableType);
}


    /*! \fn void AppendPointsToTableCmn
    * \brief This function appends points to an existing table
    * \param CMMCAxis* - Pointer to axis
    * \param cName - Axis name
    * \param hConn - Connection handle
    * \return void
    */
void CMMCMotionAxis::AppendPVTPoints(MC_PATH_REF hMemHandle,
							ELMO_DOUBLE  (&dTable)[NC_PVT_ECAM_MAX_ARRAY_SIZE],
							ELMO_ULINT32 ulNumberOfPoints,
							ELMO_ULINT32 ulStartIndex,
							ELMO_UINT8   ucIsTimeAbsolute,
							NC_MOTION_TABLE_TYPE_ENUM eTableType) throw (CMMCException)
{
	AppendPVTPoints(hMemHandle,dTable,ulNumberOfPoints,ulStartIndex,0,ucIsTimeAbsolute, eTableType);
}



ELMO_UINT32 CMMCMotionAxis::GetPVTTableIndex(MC_PATH_REF hMemHandle)  throw (CMMCException)
{
    MMC_GETTABLEINDEX_IN stIn;
    MMC_GETTABLEINDEX_OUT stOut;
    ELMO_INT32 rc;
    //
    stIn.hMemHandle = hMemHandle;
    //
    rc = MMC_GetTableIndexCmd(m_uiConnHndl,&stIn, &stOut);      
    if (NC_OK != rc)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetTableIndexCmd", m_uiConnHndl, 0, rc, stOut.usErrorID, stOut.usStatus);
        return 0;
    }
    return stOut.ulCurrentIndex;
}

/*! \fn unsigned ELMO_INT32 GetFbDepth(const unsigned ELMO_INT32 uiHndl)
* \brief this method sends a command for receive number of function blocks in Node Queue .
* \param  uiHndl handler of which to Get F.B. Depth output parameters
* \return   return - number of function blocks on success, otherwise throws CMMCException.
*/
ELMO_UINT32 CMMCMotionAxis::GetFbDepth() throw (CMMCException)
{
    MMC_GETFBDEPTH_IN stInParams;
    MMC_GETFBDEPTH_OUT stOutParam;
    ELMO_INT32 rc;
    stInParams.uiHndl = m_usAxisRef;
    if ((rc=MMC_GetFbDepthCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetFbDepthCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("GetFbDepth OK")
    return stOutParam.uiFbInQ;
}


/*! \fn unsigned ELMO_INT32 EnableMotionEndedEvent()
* \brief This function Enable the event of ended motion
* \param
* \return
*/
void CMMCMotionAxis::EnableMotionEndedEvent() throw (CMMCException)
{

    MMC_ENABLEMOTIONENDEDEVENT_IN stInParams;
    MMC_ENABLEMOTIONENDEDEVENT_OUT stOutParam;
    ELMO_INT32 rc;
    if ((rc=MMC_EnableMotionEndedEventCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_EnableMotionEndedEventCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("EnableMotionEndedEvent OK")
    return;
}


/*! \fn unsigned ELMO_INT32 DisableMotionEndedEvent()
* \brief This function Disable the event of ended motion
* \param
* \return
*/
void CMMCMotionAxis::DisableMotionEndedEvent() throw (CMMCException)
{

    MMC_DISABLEMOTIONENDEDEVENT_IN stInParams;
    MMC_DISABLEMOTIONENDEDEVENT_OUT stOutParam;
    ELMO_INT32 rc;
    if ((rc=MMC_DisableMotionEndedEventCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParam))!=0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_DisableMotionEndedEventCmd:", m_uiConnHndl, m_usAxisRef, rc, stOutParam.usErrorID, stOutParam.usStatus);
    }

    _MMCPP_TRACE_INFO("DisableMotionEndedEvent OK")
    return;
}


void CMMCMotionAxis::Dwell(ELMO_ULINT32 ulDwellTimeMs) throw (CMMCException)
{
    MMC_DWELL_IN stInParams;
    MMC_DWELL_OUT stOutParams;
    ELMO_INT32 rc = 0;
    //
    stInParams.ulDwellTimeMs = ulDwellTimeMs;
    rc = MMC_DwellCmd(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_DwellCmd: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("DwellCmd OK")
    //
    return;
}

void CMMCMotionAxis::InsertNotificationFb(ELMO_INT32 iEventCode) throw (CMMCException)
{
    MMC_INSNOTIFICATIONFB_IN stInParams;
    MMC_INSNOTIFICATIONFB_OUT stOutParams;
    ELMO_INT32 rc = 0;
    //
    stInParams.iEventCode           = iEventCode;
    stInParams.usNotifyOnCondition  = 0 ;
    //
    rc = MMC_InsertNotificationFb(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_InsertNotificationFb: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MMC_InsertNotificationFb OK")
    //
    return;
}

void CMMCMotionAxis::InsertNotificationFb(MMC_INSNOTIFICATIONFB_IN stInParams) throw (CMMCException)
{
    MMC_INSNOTIFICATIONFB_OUT stOutParams;
    ELMO_INT32 rc = 0;
    //
    rc = MMC_InsertNotificationFb(m_uiConnHndl, m_usAxisRef, &stInParams, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_InsertNotificationFb: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MMC_InsertNotificationFb OK")
    //
    return;
}

void CMMCMotionAxis::WaitUntilConditionFB(MMC_WAITUNTILCONDITIONFB_IN stInput) throw (CMMCException)
{
    MMC_WAITUNTILCONDITIONFB_OUT stOutParams;
    ELMO_INT32 rc = 0;
    //
    rc = MMC_WaitUntilConditionFB(m_uiConnHndl, m_usAxisRef, &stInput, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WaitUntilConditionFB: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("WaitUntilConditionFB OK")
    //
    return;
}

void CMMCMotionAxis::WaitUntilConditionFBEX(MMC_WAITUNTILCONDITIONFBEX_IN stInput) throw (CMMCException)
{
    MMC_WAITUNTILCONDITIONFBEX_OUT stOutParams;
    ELMO_INT32 rc = 0;
    //
    rc = MMC_WaitUntilConditionFBEX(m_uiConnHndl, m_usAxisRef, &stInput, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_WaitUntilConditionFBEX: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("MMC_WaitUntilConditionFBEX OK")
    //
    return;
}


void CMMCMotionAxis::KillRepetitive() throw (CMMCException)
{
    MMC_KILLREPETITIVE_IN stInput;
    MMC_KILLREPETITIVE_OUT stOutParams;
    ELMO_INT32 rc = 0;
    //
    rc = MMC_KillRepetitive(m_uiConnHndl, m_usAxisRef, &stInput, &stOutParams);
    if (rc != 0)
    {
        CMMCPPGlobal::Instance()->MMCPPThrow("MMC_KillRepetitive: ", m_uiConnHndl, m_usAxisRef, rc, stOutParams.usErrorID, stOutParams.usStatus);
    }
    _MMCPP_TRACE_INFO("KillRepetitive OK")
    //
    return;
}


void CMMCMotionAxis::ReadGroupOfParameters(MMC_READGROUPOFPARAMETERSMEMBER sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters, ELMO_DOUBLE dbOutVal[GROUP_OF_PARAMETERS_MAXIMUM_SIZE])
{
    MMC_READGROUPOFPARAMETERS_IN stIn;
    MMC_READGROUPOFPARAMETERS_OUT stOut;
    ELMO_INT32 rc;
    //  
    stIn.ucNumberOfParameters = ucNumberOfParameters;
    memcpy((ELMO_PVOID)stIn.sParameters,(ELMO_PVOID)sParameters,sizeof(MMC_READGROUPOFPARAMETERSMEMBER)*ucNumberOfParameters);
    //  
    rc = MMC_ReadGroupOfParameters(m_uiConnHndl, &stIn, &stOut);
    if (rc != 0)
    {
        char cMsg[60];
        sprintf(cMsg, "MMC_ReadGroupOfParameters : <Problematic entry = %d>\n",stOut.ucProblematicEntry);
        CMMCPPGlobal::Instance()->MMCPPThrow(cMsg, m_uiConnHndl, m_usAxisRef, rc, stOut.usErrorID, stOut.usStatus);
    }
    else
    {
        memcpy((ELMO_PVOID)dbOutVal,stOut.dbValue,sizeof(ELMO_DOUBLE)*ucNumberOfParameters);
    }   
    //
    return;
}
void CMMCMotionAxis::WriteGroupOfParametersImmediate(MMC_WRITEGROUPOFPARAMETERSMEMBER sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters)
{
    WriteGroupOfParameters(eMMC_EXECUTION_MODE_IMMEDIATE,sParameters,ucNumberOfParameters);
    return;
}
void CMMCMotionAxis::WriteGroupOfParametersQueued(MMC_WRITEGROUPOFPARAMETERSMEMBER sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters)
{
    WriteGroupOfParameters(eMMC_EXECUTION_MODE_QUEUED,sParameters,ucNumberOfParameters);
    return;
}


void CMMCMotionAxis::WriteGroupOfParameters(MC_EXECUTION_MODE eExecutionMode,MMC_WRITEGROUPOFPARAMETERSMEMBER sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters)
{
    MMC_WRITEGROUPOFPARAMETERS_IN stIn;
    MMC_WRITEGROUPOFPARAMETERS_OUT stOut;
    ELMO_INT32 rc;
    //
    stIn.eExecutionMode = eExecutionMode;   
    stIn.ucNumberOfParameters = ucNumberOfParameters;
    stIn.ucExecute = 0; // currently not in use
    stIn.ucMode = 0; // currently not in use
    memcpy((ELMO_PVOID)stIn.sParameters,(ELMO_PVOID)sParameters,sizeof(MMC_WRITEGROUPOFPARAMETERSMEMBER)*ucNumberOfParameters);
    //
    rc = MMC_WriteGroupOfParameters(m_uiConnHndl, m_usAxisRef, &stIn, &stOut);
    if (rc != 0)
    {
        ELMO_INT8 cMsg[60];
        sprintf(cMsg, "MMC_WriteGroupOfParameters : <Problematic entry = %d>\n",stOut.ucProblematicEntry);
        CMMCPPGlobal::Instance()->MMCPPThrow(cMsg, m_uiConnHndl, m_usAxisRef, rc, stOut.usErrorID, stOut.usStatus);
    }
    return;
}

void CMMCMotionAxis::WriteGroupOfParametersImmediateEX(MMC_WRITEGROUPOFPARAMETERSMEMBEREX sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters)
{
    WriteGroupOfParametersEX(eMMC_EXECUTION_MODE_IMMEDIATE,sParameters,ucNumberOfParameters);
    return;
}
void CMMCMotionAxis::WriteGroupOfParametersQueuedEX(MMC_WRITEGROUPOFPARAMETERSMEMBEREX sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters)
{
    WriteGroupOfParametersEX(eMMC_EXECUTION_MODE_QUEUED,sParameters,ucNumberOfParameters);
    return;
}

void CMMCMotionAxis::WriteGroupOfParametersEX(MC_EXECUTION_MODE eExecutionMode,MMC_WRITEGROUPOFPARAMETERSMEMBEREX sParameters[GROUP_OF_PARAMETERS_MAXIMUM_SIZE],ELMO_UINT8 ucNumberOfParameters)
{
    MMC_WRITEGROUPOFPARAMETERSEX_IN stIn;
    MMC_WRITEGROUPOFPARAMETERSEX_OUT stOut;
    ELMO_INT32 rc;
    //
    stIn.eExecutionMode = eExecutionMode;
    stIn.ucNumberOfParameters = ucNumberOfParameters;
    stIn.ucExecute = 0; // currently not in use
    stIn.ucMode = 0; // currently not in use

    memcpy((ELMO_PVOID)stIn.sParameters,(ELMO_PVOID)sParameters,sizeof(MMC_WRITEGROUPOFPARAMETERSMEMBEREX)*ucNumberOfParameters);
    //
    rc = MMC_WriteGroupOfParametersEX(m_uiConnHndl, m_usAxisRef, &stIn, &stOut);
    if (rc != 0)
    {
        ELMO_INT8 cMsg[60];
        sprintf(cMsg, "MMC_WriteGroupOfParametersEX : <Problematic entry = %d>\n",stOut.ucProblematicEntry);
        CMMCPPGlobal::Instance()->MMCPPThrow(cMsg, m_uiConnHndl, m_usAxisRef, rc, stOut.usErrorID, stOut.usStatus);
    }
    return;
}

/**
* \fn int int GetTableList(NC_MOTION_TABLE_TYPE_ENUM eTableType, unsigned int (&uiHandlers)[MMC_MAX_JOURNAL_ENTRIES]);
* \brief this function get a list of table for given table type.
* \param  eTableType - type of tables to retrieve.
* \param  uiHandlers - buffer of table handlers to retriev by eTableType.
* \param  iNum - stores number of loaded tables of eTableType on successfull return.
* \return	return - 0 if success otherwise error or thrown CMMCException.
*/
int CMMCMotionAxis::GetTableList(NC_MOTION_TABLE_TYPE_ENUM eTableType, ELMO_UINT32 (&uiHandlers)[MMC_MAX_JOURNAL_ENTRIES], ELMO_INT32 & iNum) throw (CMMCException)
{
	MMC_TABLE_LIST_IN stInParam;
	MMC_TABLE_LIST_OUT stOutParam;
	int rc = 0;
	stInParam.eTableType = eTableType;
	if ( (rc = MMC_GetTableList(m_uiConnHndl, &stInParam, &stOutParam)) != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetTableList: ", m_uiConnHndl, 0, rc, stOutParam.sErrorID, stOutParam.usStatus);
	}
	ELMO_INT32 size = (stOutParam._iNumber <= MMC_MAX_JOURNAL_ENTRIES) ? stOutParam._iNumber : MMC_MAX_JOURNAL_ENTRIES;
	memcpy(uiHandlers, stOutParam._uiHandlers, size * sizeof(ELMO_INT32));
	iNum = stOutParam._iNumber;
	_MMCPP_TRACE_INFO("GetTableList OK")
	return rc;
}

/**
* \fn int GetTableInfo(unsigned int uiHandler, char (&name)[MMC_TABLE_FILE_LENGTH+1]) throw (CMMCException);
* \brief this function gets a table name (by table handler).
* \param  uiHandler - table's uiHandler to retrieve its name.
* \param  name - name of given table by uiHandler.
* \return	return - 0 if success otherwise error or thrown CMMCException.
*/
ELMO_INT32 CMMCMotionAxis::GetTableInfo(ELMO_UINT32 uiHandler, ELMO_INT8 (&name)[MMC_TABLE_FILE_LENGTH+1]) throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_TABLE_DATA_IN stInParam;
	MMC_TABLE_DATA_OUT stOutParam;

	stInParam.uiHandler = uiHandler;
	if ( (rc = MMC_GetTableInfo(m_uiConnHndl, &stInParam, &stOutParam)) != 0)
	{
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_GetTableInfo: ", m_uiConnHndl, m_usAxisRef, rc, stOutParam.sErrorID, stOutParam.usStatus);
	}
	strncpy(name, stOutParam._name, MMC_TABLE_FILE_LENGTH);
	name[MMC_TABLE_FILE_LENGTH] = '\0';
	_MMCPP_TRACE_INFO("GetTableInfo OK")
	return rc;
}

/**
* \fn int KillMotion(MC_PATH_REF hMemHandle)  throw (CMMCException);
* \brief this method stop profiler moiton on the spot.
* \return	return - 0 if success otherwise error or thrown CMMCException.
*/
ELMO_INT32 CMMCMotionAxis::KillMotion()  throw (CMMCException)
{
	ELMO_INT32 rc = 0;
	MMC_KILLMOTION_IN i_params;
	MMC_KILLMOTION_OUT o_params;
	memset(&o_params, 0, sizeof(MMC_KILLMOTION_OUT));
	if ((rc = MMC_KillMotion(m_uiConnHndl, m_usAxisRef, &i_params, &o_params)) < 0)
	{
		rc = o_params.sErrorID;
		CMMCPPGlobal::Instance()->MMCPPThrow("MMC_KillMotion: ", m_uiConnHndl, m_usAxisRef, rc, o_params.sErrorID, o_params.usStatus);
	}
	_MMCPP_TRACE_INFO("MMC_KillMotion OK")
	return rc;
}
