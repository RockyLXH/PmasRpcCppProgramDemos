/*
 * MMCUtil.h
 *
 *  Created on: 21/07/2013
 *      Author: Haim Hillel
 *     Updated: HH: Change case of Including: MMCHsTftpAppExp.hpp...
 *
 *  Methods for upload and download file to / from GMAS over Tftp protocol.
 *  The methods using Gmas services for activate (remote) TFTP Client with requred parameters (file name, get or put...),
 *  (MMC_ C library MMC_ResImportFileCmd & MMC_ResExportFileCmd).
 *
 *  The TFTP package purchased (on Jul 2013) as C source (and demo app from):
 *           Hillstone Software. http://www.hillstone-software.com
 *  and porting in ElmoMc.
 *
 *      Update: 23Sep2019 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *
 */
 

#include "OS_PlatformDependSetting.hpp"


//#ifdef WIN32
// Meanwhile: External HS SW package did not ports for Linux
// 				limits the useing only for Windows 
//#if (OS_PLATFORM != LINUXIPC32_PLATFORM)
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))

#include "MMCUtil.hpp"
#include "MMCHsTftpAppExp.h"

#define DEF_SPEAK_LVL   "3"
extern ELMO_INT32 gSpeakLvl;



CMMCUtil::CMMCUtil()
{
    bReqServerStayRunning = FALSE;
}

CMMCUtil::~CMMCUtil()
{
    UtilStopTftpServer();
    bReqServerStayRunning = FALSE;
}

                        /* If the server not running, run it and flag it    */
                        /* for stay running.                                */
ELMO_INT32 CMMCUtil::UtilStartTftpServer(ELMO_PINT8 DesHostFilePath)
{
    ELMO_INT32 iErrId = 0;

    if (bReqServerStayRunning == FALSE)
    {
        iErrId = UtilActivateTftpServer(DesHostFilePath);
        bReqServerStayRunning = TRUE;
    }
    else
    {
        updHostPath(DesHostFilePath);
    }

    return (iErrId);
}

                        /* If Tftp server running stop it.                  */
void CMMCUtil::UtilStopTftpServer(void)
{
    if (bReqServerStayRunning == TRUE)
    {
        deinit();
        bReqServerStayRunning = FALSE;
    }
}

                        /* Download ("ImportFile") direction is "TO Gmas"   */
                        /* if FileName including path it remove (ignore),   */
                        /* the requsting path keeping on host and use       */
                        /* with file name for locate the file.              */
                        /* Path can be in Linux or Windows notation         */
ELMO_INT32 CMMCUtil::UtilDownloadFile(IN MMC_CONNECT_HNDL ConnHndl, ELMO_PINT8 FileName, ELMO_PINT8 DesHostFilePath, ELMO_PINT8 RpcHostIp, MMC_DOWNLOAD_TYPE_ENUM FileDownloadType)   throw (CMMCException)
{
    ELMO_UINT16             usStatus;       /* < Returned command status.   */
    ELMO_INT16              usErrorID;      /* < Returned command error ID. */
    MMC_RESIMPORTFILE_IN    ImportFileIn;
    MMC_RESIMPORTFILE_OUT   ImportFileOut;
    ELMO_INT32              errorSourc = 0;     /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
    ELMO_INT32              iErrId = 0;
//  unsigned int            runWaitLoopCountLim;
//  unsigned int            runWaitLoopCount;



                                /* Multiplay by 8 the timeout interval wait for remote response */
                                /* keep the current running value.                              */
//  setRemoteResponseWaitLim(8, &runWaitLoopCountLim, &runWaitLoopCount);

                        /* Activate the TFTP Server */
    if (bReqServerStayRunning == FALSE)
    {
        iErrId = UtilActivateTftpServer(DesHostFilePath);
        if (iErrId != 0)
        {
            errorSourc = 0;     /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
            goto exit_Util;
        }
    }
    else
    {
        updHostPath(DesHostFilePath);
    }


    errorSourc = 1;             /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
    strncpy(ImportFileIn.pFName,  FileName, 33);
    ImportFileIn.pFName[32] = '\0';
    strncpy(ImportFileIn.pServer, RpcHostIp, 17);
    ImportFileIn.ucDownloadType = FileDownloadType;

    iErrId = MMC_ResImportFileCmd(ConnHndl, &ImportFileIn, &ImportFileOut);
    usStatus  = ImportFileOut.usStatus;
    usErrorID = ImportFileOut.usErrorID;


                        /* Stop and clear server resource */
    if (bReqServerStayRunning == FALSE)
    {
        deinit();
    }


exit_Util:
                                /* Return to the kept timeout interval wait for remote response */
//  setRemoteResponseWaitLim(runWaitLoopCountLim, &runWaitLoopCountLim, &runWaitLoopCount);

    if (iErrId != 0)
    {
                        /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
        if (errorSourc == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("UtilDownloadFile: UtilActivateTftpServer error", NULL, -1, MMC_LIB_ERROR, iErrId, 0);
        }
        else
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("UtilDownloadFile: MMC_ResImportFileCmd error", ConnHndl, -1, iErrId, ImportFileOut.usErrorID, ImportFileOut.usStatus);
        }
    }
    return iErrId;
}

                        /* Upload ("ExportFile") direction "FROM Gmas out"  */
                        /* if FileName including path it remove (ignore),   */
                        /* the requsting path keeping on host and use       */
                        /* with file name for locate the file.              */
                        /* Path can be in Linux or Windows notation         */
ELMO_INT32 CMMCUtil::UtilUploadFile(IN MMC_CONNECT_HNDL ConnHndl, ELMO_PINT8 FileName, ELMO_PINT8 DesHostFilePath, ELMO_PINT8 RpcHostIp, MMC_DOWNLOAD_TYPE_ENUM FileDownloadType) throw (CMMCException)
{
    ELMO_UINT16             usStatus;       /* < Returned command status.   */
    ELMO_INT16              usErrorID;      /* < Returned command error ID. */
    MMC_RESEXPORTFILE_IN    ExportFileIn;
    MMC_RESEXPORTFILE_OUT   ExportFileOut;
    ELMO_INT32              errorSourc = 0;     /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
    ELMO_INT32              iErrId = 0;
//  unsigned int            runWaitLoopCountLim;
//  unsigned int            runWaitLoopCount;


                                /* Multiplay by 7 the timeout interval wait for remote response */
                                /* keep the current running value.                              */
//  setRemoteResponseWaitLim(7, &runWaitLoopCountLim, &runWaitLoopCount);

                        /* Activate the TFTP Server */
    if (bReqServerStayRunning == FALSE)
    {
        iErrId = UtilActivateTftpServer(DesHostFilePath);
        if (iErrId != 0)
        {
            errorSourc = 0;     /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
            goto exit_Util;
        }
    }
    else
    {
        updHostPath(DesHostFilePath);
    }

    errorSourc = 1;     /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
//  strncpy(ExportFileIn.pFilePath, DesHostFilePath, 101);
    ExportFileIn.pFilePath[0] = NULL;

    strncpy(ExportFileIn.pFName, FileName, 33);
    ExportFileIn.pFName[32] = '\0';
    strncpy(ExportFileIn.pServer, RpcHostIp, 17);
    ExportFileIn.ucDownloadType = FileDownloadType;

    iErrId = MMC_ResExportFileCmd(ConnHndl, &ExportFileIn, &ExportFileOut);
    usStatus  = ExportFileOut.usStatus;
    usErrorID = ExportFileOut.usErrorID;


                        /* Stop and clear server resource */
    if (bReqServerStayRunning == FALSE)
    {
        deinit();
    }

exit_Util:
                                /* Return to the kept timeout interval wait for remote response */
//  setRemoteResponseWaitLim(runWaitLoopCountLim, &runWaitLoopCountLim, &runWaitLoopCount);
    if (iErrId != 0)
    {
                        /* 0/1: 0 error from Tftp Util lib (1=>error from MMC_...) */
        if (errorSourc == 0)
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("UtilUploadFile: UtilActivateTftpServer error", NULL, -1, MMC_LIB_ERROR, iErrId, 0);
        }
        else
        {
            CMMCPPGlobal::Instance()->MMCPPThrow("UtilUploadFile: MMC_ResImportFileCmd error", ConnHndl, -1, iErrId, ExportFileOut.usErrorID, ExportFileOut.usStatus);
        }
    }
    return iErrId;
}

                        /* Set the TFTP low level services to new speak (report level), return the  */
                        /* running speak level.                                                     */
                        /* Range 0 (=HSTFTPAPP_SPEAK_LVL_ALL) to 5 (=HSTFTPAPP_SPEAK_LVL_NONE)      */
                        /* If value is out of range (Eg -1) set it same as DEF_SPEAK_LVL            */
                        /* (concider its the "default").                                            */
ELMO_INT32 CMMCUtil::UtilSetTftpSpeakLvl(ELMO_INT32 newSpeakLvl)
{
    ELMO_INT32 rungSpeakLvl;


    rungSpeakLvl = (ELMO_INT32)gSpeakLvl;
    if((newSpeakLvl >= 0) && (newSpeakLvl <=5))
    {
        gSpeakLvl = newSpeakLvl;
    }
    else
    {
        gSpeakLvl = 3;
    }

    return(rungSpeakLvl);
}


                        /* Activate the TFTP Server.                                */
                        /* =========================                                */
                        /* For detaile about activation argument look in            */
                        /* HsTftpApp.cpp file for entry name:                       */
                        /*                HSTFTPAPP_MAIN_EP_NAME                    */
                        /*                                                          */
                        /* Activation arguments:                                    */
                        /* First  ("HsTftpApp_main_ep") Activation function name.   */
                        /* Second ("-s")            Set activation type. "-s"       */
                        /*                              activate the Util as server.*/
                        /* Third ("3")              Set speak level. Range 0 to 5.  */
                        /*                              0=all; 5=None;              */
                        /*                              setting (gSpeakLvl) to 3.   */
                        /*                              (HSTFTPAPP_SPEAK_LVL_ERR)   */
                        /* Fifth ("d:/defaultServerFilesLocation") The VALUE is not */
                        /*                              in use, its keeping place   */
                        /*                              for actual path (replace    */
                        /*                              this value).                */
                        /* Attention:                                               */
                        /* ==========                                               */
                        /* argv & argc should be static (no automatic on stack),    */
                        /* they are reference also when 'UtilActivateTftpServer' not*/
                        /* in scope (already returned)                              */
ELMO_PINT8 argv[] = {{"HsTftpApp_main_ep"}, {"-s"}, {DEF_SPEAK_LVL}, {"d:/ServerFilesLocation/"}};
//char *argv[] = {{"HsTftpApp_main_ep"}, {"-s"}, {"0"}, {"d:/ServerFilesLocation/"}};
ELMO_INT32  argc    = sizeof(argv)/sizeof(ELMO_PINT8); /* number of elements in argv   */

ELMO_INT32 CMMCUtil::UtilActivateTftpServer(ELMO_PINT8 DesHostFilePath)
{
    ELMO_INT32 iErrId = 0;


    if (DesHostFilePath != NULL)
    {
                        /* Replace the default by requested host file path */
        argv[argc-1] = DesHostFilePath;
    }

#ifdef MMCPP_DEBUG
                        /* Set speak level for "HSTFTPAPP_SPEAK_LVL_ALL"*/
                        /* the same style as "MMCPP_TRACE"              */
    gSpeakLvl = 0;
#endif


    iErrId = HsTftpApp_main_ep(argc, argv);


    return (iErrId);
}

#endif      /* #if (OS_PLATFORM != LINUXIPC32_PLATFORM) */


