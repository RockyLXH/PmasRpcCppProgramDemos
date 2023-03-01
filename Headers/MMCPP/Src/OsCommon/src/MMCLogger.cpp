/*
 * CMMCLogger.cpp
 *
 *  Created on: 26Apr2015
 *      Author: Haim H.
 *      Update: 06May2015   Haim H.
 *      Update: 14May2015   Haim H. implement meetting action items.
 *      Update: 26Sep2016   Haim H. in function 'LogMsgRoundFileCreation()' adds system call ('su') for change log file
 *                                  owner:group b4 opens (for append/create)
 *
 * Description: See detail in CMMCLogger.h
 */


#include "OS_PlatformDependSetting.hpp"

//#include "stdafx.h"
//#ifndef WIN32
#if (OS_PLATFORM == LINUXIPC32_PLATFORM)

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cerrno>

#include "MMCLogger.hpp"

 
        /* +20: buffer also for command (su -c... ) */
#define    CMD_BUFF_LEN (LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN+20)   


                /* Server Configure function:                                   */
                /* Set Server Configure parameters to defaults.                 */
                /* !!! NOT EFECT RUNNING SERVER, EFECT STARTINGUP SERVER        */
 void   MMCLogger::LogMsgConfigureToDefault(void)
 // =============================================
 {
                /* Actual logging file path and name = the defaults */
    snprintf(_pcLogMsgFileAndPathStorageName, LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN, "%s%s%s", LOG_MSG_FLASH_FILE_PATH, LOG_MSG_DEF_LOG_FILE_NAME, LOG_MSG_EAS_FILE_EXTENTION);
                /* default output size is enLOG_MSG_SIZE_SMALL (adding short contex description to log message) */
                /* also possible: enLOG_MSG_SIZE_NORM; Smaller [-S])                                            */
    _enLogMsgNormalOrSmallerMsgSize = enLOG_MSG_SIZE_SMALL;
                /* Log Message File Location (path) Volatile | Flash */
    _enLogMsgVolatileOrFlash = enLOG_MSG_FLASH_FILE_PATH;
                /* Max File size Before Rotate */
    _iLogMsgMaxFileSizeB4RotateKb = LOG_MSG_DEF_FILE_SIZEKB;
                /* Number of rotated logs file to keep */
    _iLogMsgNumRotateFilesKeep = LOG_MSG_DEF_ROTATE_FILES;
                /* !! LOCAL !! Mask for file collecting level 1-8. Collect if the send bit is NOT set.  */
                /* E.g. for set the level=4; 4 & 5 not collects; 0,1,2,3,6,7                            */
                /* If send to remote Configure, send message not effect from this setting.              */
    _iLogMsgServerLocalLevel = LOG_MSG_DEF_SRV_LOCAL_LEVEL;
                /* Remote Host Ip */
    _pcLogMsgRemoteHostIp[0] = 0;
                /* whether logging local when send logging to remote */
    _bReportAlsoLocal = false;
                /* Remote port */
    _iRemoteHostPort = LOG_MSG_DEF_PORT;
 }


                /* Server Configure function:                                   */
                /* Define Actual Logging file path & name                       */
                /* Add extention to Base file name (or user deliver name)       */
                /* (The EAS search/find logging files according this extention).*/
                /* If not specifay name or its NULL => use the defaults name.   */
                /* Return 0 when everything ok and as expecting.                */
                /* If not set using the defaults.                               */
 ELMO_INT8    MMCLogger::LogMsgConfigureFilePathAndName(ELMO_PINT8 pcLogMsgFileName,
                                       enLOG_MSG_LOGGING_FILE_LOCATION_PATH enLogMsgLoggingFileLocation)
 // ====================================================================================================
 {
    ELMO_PINT8 cpLogFilePath;
    ELMO_PINT8 cpLogFileName;



    if (enLogMsgLoggingFileLocation == enLOG_MSG_FLASH_FILE_PATH)
    {
        cpLogFilePath = LOG_MSG_FLASH_FILE_PATH;
    }
    else
    {
        cpLogFilePath = LOG_MSG_VOLATILE_FILE_PATH;
    }

    if (pcLogMsgFileName == NULL)
    {
        cpLogFileName = LOG_MSG_DEF_LOG_FILE_NAME;
    }
    else
    {
        cpLogFileName = pcLogMsgFileName;
    }


                                /* Keep requsting (user) file name (in scope along the class is)    */
                                /* Add extention (for use by EAS search/find the files)             */
    snprintf(_pcLogMsgFileAndPathStorageName, LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN, "%s%s%s", cpLogFilePath, cpLogFileName, LOG_MSG_EAS_FILE_EXTENTION);

    return (MMC_OK);
 }

                /* Server Configure function:                                   */
                /* Remove (Delete) specific (or defaults) logging files.        */
                /* Add extention to Base file name (or user deliver name)       */
                /* (The EAS search/find logging files according this extention).*/
                /* If not specifay name or its NULL => use the defaults name.   */
                /* Return 0 when everything ok and as expecting.                */
                /* If not called using the defaults.                            */
 ELMO_INT8    MMCLogger::LogMsgConfigureRmoveLogFiles(ELMO_PINT8 pcLogMsgFileName,
                                                enLOG_MSG_LOGGING_FILE_LOCATION_PATH enLogMsgLoggingFileLocation)
 // =============================================================================================================
 {
    ELMO_PINT8	cpLogFilePath;
    ELMO_PINT8  cpLogFileName;
    ELMO_INT8   iRtVal;
    ELMO_INT8   pcLogMsgFileAndPathName[LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN];
    ELMO_INT8   pcCmdbuffer[CMD_BUFF_LEN];



    if (enLogMsgLoggingFileLocation == enLOG_MSG_FLASH_FILE_PATH)
    {
        cpLogFilePath = LOG_MSG_FLASH_FILE_PATH;
    }
    else
    {
        cpLogFilePath = LOG_MSG_VOLATILE_FILE_PATH;
    }

    if (pcLogMsgFileName == NULL)
    {
        cpLogFileName = LOG_MSG_DEF_LOG_FILE_NAME;
    }
    else
    {
        cpLogFileName = pcLogMsgFileName;
    }

                /* Ensure the process can delete the files.                         */
                /* The file may creates by others process - chage 'owner' to 'user' */
    snprintf(pcLogMsgFileAndPathName, LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN, "%s%s%s", cpLogFilePath, cpLogFileName, LOG_MSG_EAS_FILE_EXTENTION); 
    iRtVal = access(pcLogMsgFileAndPathName, F_OK);
    if (iRtVal != -1)   /* When TRUE => exist base file... */
    {
                /* Change file 'Owner' Command like:                                */
                /*   su -c "chown user:user /mnt/suuf/usr/LogMsg.log*"              */
        snprintf(pcCmdbuffer, CMD_BUFF_LEN, "su -c \"chown user:user %s*\"", pcLogMsgFileAndPathName);
        iRtVal = system(pcCmdbuffer);
        iRtVal = WEXITSTATUS(iRtVal);
        if (iRtVal != 0)
        {
            printf("\n *** %s %s cmd:<%s> failed errno=%d \n", __FILE__, __func__, pcCmdbuffer, errno);
        }


                /* Delete the files (it exist)                                      */
        snprintf(pcCmdbuffer, CMD_BUFF_LEN, "exec rm -f %s*", pcLogMsgFileAndPathName);
    
    
        iRtVal = system(pcCmdbuffer);
        iRtVal = WEXITSTATUS(iRtVal);
        if (iRtVal != 0)
        {
            printf("\n *** %s %s cmd:<%s> failed errno=%d \n", __FILE__, __func__, pcCmdbuffer, errno);
        }
    }
    else
    {
        iRtVal = MMC_OK;
    }

    return (iRtVal);
 }
 
                /* Server Configure function:                                   */
                /* Normal / Smaller logging output - Set logging message size   */
                /* Return 0 when everything ok and as expecting.                */
                /* If not called using the defaults.                            */
 ELMO_INT8    MMCLogger::LogMsgConfigureLogMsgSize(enLOG_MSG_SIZE_ENUM enLogMsgSize)
 // ================================================================================
 {
    ELMO_INT32     rtVal = MMC_OK;

    if ((enLogMsgSize == enLOG_MSG_SIZE_NORM) || (enLogMsgSize == enLOG_MSG_SIZE_SMALL))
    {
        _enLogMsgNormalOrSmallerMsgSize = enLogMsgSize;
    }
    else
    {
        rtVal = LOG_MSG_ILLEGAL_PRM_MsgSIZE;
    }

    return (rtVal);
 }


                /* Server Configure function:                                   */
                /* Define (set) the Max File size before rotate:                */
                /* Range: LOG_MSG_MIN_FILE_SIZEKB to LOG_MSG_MAX_FILE_SIZEKB    */
                /* Default: LOG_MSG_DEF_FILE_SIZEKB                             */
                /* Return 0 when everything ok and as expecting.                */
                /* If not called using the defaults.                            */
 ELMO_INT32    MMCLogger::LogMsgConfigureMaxFileSizeB4Rotate(int iFileSizeB4RotateKB)
 // =================================================================================
 {
    ELMO_INT32     rtVal = MMC_OK;

    if ((LOG_MSG_MIN_FILE_SIZEKB <= iFileSizeB4RotateKB) && (LOG_MSG_MAX_FILE_SIZEKB >= iFileSizeB4RotateKB))
    {
        _iLogMsgMaxFileSizeB4RotateKb = iFileSizeB4RotateKB;
    }
    else
    {
        rtVal = LOG_MSG_ILLEGAL_PRM_SizeB4ROTATE;
    }

    return (rtVal);
 }


                /* Server Configure function:                                   */
                /* Set Number of rotated logs file to keep:                     */
                /* Range: LOG_MSG_MIN_ROTATE_FILES to LOG_MSG_MAX_ROTATE_FILES  */
                /* E.g1:  Files for iNumRotateFiles=1 LogMsg, LogMsg.0          */
                /* E.g2:  Files for iNumRotateFiles=2 LogMsg, LogMsg.0 LogMsg.1 */
                /* E.g3:  LogMsgNumRotateFiles(3);                              */
                /*      Cause cyclic write to files:                            */
                /*          LogMsg, LogMsg.0, LogMsg.1, LogMsg.2                */
                /* Return 0 when everything ok and as expecting.                */
                /* If not called using the defaults.                            */
 ELMO_INT32    MMCLogger::LogMsgConfigureNumRotateFiles(ELMO_INT32 iNumRotateFiles)
 // ===============================================================================
 {
    ELMO_INT32     rtVal = MMC_OK;

    if ((LOG_MSG_MIN_ROTATE_FILES <= iNumRotateFiles) && (LOG_MSG_MAX_ROTATE_FILES >= iNumRotateFiles))
    {
        _iLogMsgNumRotateFilesKeep = iNumRotateFiles;
    }
    else
    {
        rtVal = LOG_MSG_ILLEGAL_PRM_ROTATEFILES;
    }

    return (rtVal);
 }


                /* Server Configure function:                                   */
                /* Set Logging to remote (Logging also localy when / if explicit*/
                /* request), define remote IP and Port:                         */
                /* If (pcRemoteHostIp == NULL) Configure for NO LOG TO REMOTE   */
                /* !!! Any Configure take affect only on next servers start !!! */
                /* Return 0 when everything ok and as expecting.                */
 ELMO_INT32 MMCLogger::LogMsgConfigureToRemoteHost(ELMO_PINT8	pcRemoteHostIp,
                        ELMO_BOOL	bReportAlsoLocal,
                        ELMO_INT32  iRemoteHostPort)
 // ===========================================================================
 {
    ELMO_INT32     rtVal = MMC_OK;

    if (pcRemoteHostIp == NULL)
    {
        _pcLogMsgRemoteHostIp[0] = 0;
        return(MMC_OK);
    }

    if (LogMsgIsValidIp(pcRemoteHostIp) == false)
    {
        return (LOG_MSG_ILLEGAL_PRM_REMOTEHOST);
    }

                /* Keep remote host IP. */
    strncpy(_pcLogMsgRemoteHostIp, pcRemoteHostIp, LOG_MSG_REMOTE_IP_STORAGE_LEN);

                /* whether logging local when send logging to remote */
    _bReportAlsoLocal = bReportAlsoLocal;
                /* Remote port */
    _iRemoteHostPort = iRemoteHostPort;

    return (rtVal);
 }

                /* Configure and Start server daemon, all parameters should exist in call   */
                /* If MsgFileName is NULL => use the defaults name.                         */
                /* If pcRemoteHostIp is NULL => no log to remote.                           */
                /* The behavior is like all of related config function are call with the    */
                /* relevant parameters and then calling to start server.                    */
 ELMO_INT32    MMCLogger::LogMsgConfigureAndStartLoggerServer(ELMO_PINT8            pcLogMsgFileName,
                                            enLOG_MSG_LOGGING_FILE_LOCATION_PATH    enLogMsgLoggingFileLocation,
                                            enLOG_MSG_SIZE_ENUM                     enLogMsgSize,
                                            ELMO_INT32                              iFileSizeB4RotateKB,
                                            ELMO_INT32                              iNumRotateFiles,
                                            ELMO_PINT8                              pcRemoteHostIp,
                                            ELMO_BOOL                               bReportAlsoLocal,
                                            ELMO_INT32                              iRemoteHostPort)
 // ==============================================================================================================
 {
     ELMO_INT32     iRtVal = MMC_OK;

     iRtVal = LogMsgStopLoggerServer();
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

     iRtVal = LogMsgConfigureFilePathAndName(pcLogMsgFileName, enLogMsgLoggingFileLocation);
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

     iRtVal = LogMsgConfigureLogMsgSize(enLogMsgSize);
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

     iRtVal = LogMsgConfigureMaxFileSizeB4Rotate(iFileSizeB4RotateKB);
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

     iRtVal = LogMsgConfigureNumRotateFiles(iNumRotateFiles);
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

     iRtVal = LogMsgConfigureToRemoteHost(pcRemoteHostIp, bReportAlsoLocal, iRemoteHostPort);
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

     iRtVal = LogMsgStartLoggerServer();
     if (iRtVal != 0) goto ExitLogMsgConfigureAndStartLoggerServer;

 ExitLogMsgConfigureAndStartLoggerServer:
     return (iRtVal);
 }

                /* Activate Logger Server with configured values (defaults      */
                /* or from Configure calls).                                    */
                /* This call should start the message collector server (daemon) */
                /* which stays running also after the application exit.         */
                /* Call to configure methoe (above) after Start has no          */
                /* effect on the running server !!!                             */
                /* Return 0 when everything ok and as expecting.                */
 ELMO_INT32 MMCLogger::LogMsgStartLoggerServer()
 // ============================================
 {
    ELMO_INT32   iRtVal = MMC_OK;
    ELMO_PINT8   pS_Arg = "";
    ELMO_PINT8   pL_Arg = "";
    ELMO_INT8    SyslogdActivationCommand[SYSLOGD_ACTIVATION_CMD_LEN];



 iRtVal = LogMsgStopLoggerServer();
 if (iRtVal != 0)
 {
     return (iRtVal);
 }
                /* In case there more then 1 file in round (<fileName>.1 or more)       */
                /* on first round the orginal code try to rename existing files and     */
                /* faile... (at this moment there is no <fileName>.0 for rename to <>.1)*/
                /* for correct this problem this function creates the empty files...    */
                /* Return 0 if OK                                                       */
 iRtVal = LogMsgRoundFileCreation();
 if (iRtVal != 0)
 {
     return (iRtVal);
 }

                /*
                 * syslogd activation command:
                 *
                 *   -n              Run in foreground
                 *   -O FILE         Log to given file (default=/var/log/messages)
                 *   -l n            Set local log level mask
                 *   -S              Smaller logging output
                 *   -s SIZE         Max size (KB) before rotate (default=200KB, 0=off)
                 *   -b NUM          Number of rotated logs to keep (default=1, max=99, 0=purge)
                 *   -R HOST[:PORT]  Log to IP or hostname on PORT (default PORT=514/UDP)
                 *   -L              Log locally and via network (default is network only if -R)
                 *   -C[size(KiB)]   Log to shared mem buffer (read it using logread)
                 */

    if (_enLogMsgNormalOrSmallerMsgSize == enLOG_MSG_SIZE_SMALL)
    {
        pS_Arg = " -S";
    }

    if (_bReportAlsoLocal)
    {
        pL_Arg = " -L";
    }


    if (_pcLogMsgRemoteHostIp[0] == 0)
    {
                /* No Report to Remote */
        snprintf(SyslogdActivationCommand, SYSLOGD_ACTIVATION_CMD_LEN, "syslogd -O %s%s -s %d -b %d -l %d",
                                                    _pcLogMsgFileAndPathStorageName,
                                                    pS_Arg,
                                                    _iLogMsgMaxFileSizeB4RotateKb,
                                                    _iLogMsgNumRotateFilesKeep,
                                                    _iLogMsgServerLocalLevel);
    }
    else
    {
                /* Report to Remote */
        snprintf(SyslogdActivationCommand, SYSLOGD_ACTIVATION_CMD_LEN, "syslogd -O %s%s -s %d -b %d -l %d -R %s:%d%s",
                                                    _pcLogMsgFileAndPathStorageName,
                                                    pS_Arg,
                                                    _iLogMsgMaxFileSizeB4RotateKb,
                                                    _iLogMsgNumRotateFilesKeep,
                                                    _iLogMsgServerLocalLevel,
                                                    _pcLogMsgRemoteHostIp,
                                                    _iRemoteHostPort,
                                                    pL_Arg);
    }

    printf("\n %s: Argument for starting the server: <%s>\n", __FILE__, SyslogdActivationCommand);

    // iRtVal =  system(SyslogdActivationCommand);
    // iRtVal = WEXITSTATUS(iRtVal);

    CMMCPPGlobal::Instance()->UserCommandControlExe(_uiConnHndl, SyslogdActivationCommand,
                        (MC_COMMAND_OPERATION)98/*eMMC_COMMAND_OPERATION_REMOVE*/, iRtVal); //START

    return (iRtVal);
 }


                /* Stop logging server:                                         */
                /* This call should stop collecting logging message by server   */
                /* (stop the Server daemon) which stays running also after the  */
                /* application logout/exit.                                     */
                /* Return 0 when everything ok and as expecting.                */
 ELMO_INT32 MMCLogger::LogMsgStopLoggerServer()
 // ====================================
 {
    ELMO_INT32 rtVal = MMC_OK;
    ELMO_INT32 loopsOf500us = 0;
    // char sysCmdBuf[100];

    // sprintf(sysCmdBuf, "killall -9 %s ", "syslogd");

    // rtVal = system(sysCmdBuf);
    // rtVal = WEXITSTATUS(rtVal);
                                     
    CMMCPPGlobal::Instance()->UserCommandControlExe(_uiConnHndl, "syslogd", eMMC_COMMAND_OPERATION_ISRUNNING, rtVal);
    if (rtVal != 0)
    {
        CMMCPPGlobal::Instance()->UserCommandControlExe(_uiConnHndl, "syslogd", (MC_COMMAND_OPERATION)99, rtVal);//STOP
        do
        {
            CMMCPPGlobal::Instance()->UserCommandControlExe(_uiConnHndl, "syslogd", eMMC_COMMAND_OPERATION_ISRUNNING, rtVal);
            if (rtVal == 1)
            {
                usleep(500);
                loopsOf500us++;
            }
        }
        while ((loopsOf500us < 10) && (rtVal == 1));
    }


    return (rtVal);
 }


 /*
  * FACILITY_PRIORITY:
  * ==================
  * The combination of FACILITY and PRIORITY is also call 'logLevel'
  * The macro LOG_MAKEPRI generates logLevel from a facility and a priority
  * Example: LOG_MAKEPRI(LOG_USER, LOG_WARNING)
  *
  * Legal values for facility:
  *      LOG_AUTH LOG_AUTHPRIV LOG_CRON LOG_DAEMON LOG_FTP LOG_KERN LOG_LOCAL0 - through LOG_LOCAL7
  *      LOG_LPR LOG_MAIL LOG_NEWS LOG_SYSLOG LOG_USER (default) LOG_UUCP
  *
  * Legal values for the priority:
  *      LOG_EMERG LOG_ALERT LOG_CRIT LOG_ERR LOG_WARNING LOG_NOTICE LOG_INFO LOG_DEBUG
  */
                /* Send logging Message to server:                              */
                /* if the server already running (started) the message should   */
                /* rgister in logging file / send to remote or both acording to */
                /* server Configure.                                            */
                /* Message builds according to format as in printf(forma,...)   */
                /* Return 0 when everything ok and as expecting.                */
                /*          E.g:                                                */
                /*          int     iSeq=0;                                     */
                /*          …                                                   */
                /*          iSeq++;                                             */
                /*          LogMsg(LOG_CRIT, "Good Message Number %d", iSeq);   */
                /* !!! DEFAULT LEVEL !!!                                        */
 ELMO_INT32 MMCLogger::LogMsg(               const ELMO_PINT8 format, ...)
 // ======================================================================
 {
    va_list 	args;
    ELMO_INT32  facilityPriority;

                /* See above for 'FACILITY_PRIORITY:' */
    facilityPriority = LOG_MAKEPRI(LOG_USER, LOG_MSG_DEF_LOG_MSG_LEVEL);
    
    va_start(args, format);
    (void) vsyslog(facilityPriority, format, args);
    va_end(args);

    return (MMC_OK);
 }

                /* ABOVE FUNCTION FOR call with DEFAULT LEVEL                   */
                /* THIS FUCTION FOR USER SPECIFIC LEVEL.                        */
 ELMO_INT32 MMCLogger::LogMsg(ELMO_INT32 SendLevel, const ELMO_PINT8 format, ...)
 // =============================================================================
 {
    va_list 	args;
    ELMO_INT32  facilityPriority;


    if ((LOG_EMERG > SendLevel) || (LOG_DEBUG < SendLevel))
        return(LOG_MSG_ILLEGAL_PRM_LOGMSG);

                /* See above for 'FACILITY_PRIORITY:' */
    facilityPriority = LOG_MAKEPRI(LOG_USER, SendLevel);

    va_start(args, format);
    (void) vsyslog(facilityPriority, format, args);
    va_end(args);

    return (MMC_OK);
 }


 ELMO_BOOL MMCLogger::LogMsgIsValidIp(ELMO_PINT8 IpAdd)
 // ===================================================
 {
 ELMO_UINT32	b1, b2, b3, b4;
 ELMO_UINT8  	ch;

 if (sscanf(IpAdd, "%3u.%3u.%3u.%3u%c", &b1, &b2, &b3, &b4, &ch) != 4)
 return (ELMO_FALSE);

 if ((b1 | b2 | b3 | b4) > 255)                     return(ELMO_FALSE);
 if (strspn(IpAdd, "0123456789.") < strlen(IpAdd))  return(ELMO_FALSE);

 return (ELMO_TRUE);
 }

                /* In case there more then 1 file in round (<fileName>.1 or more)       */
                /* on first round the orginal code try to rename existing files and     */
                /* faile... (at this moment there is no <fileName>.0 for rename to <>.1)*/
                /* for correct this problem this function creates the empty files...    */
                /* return 0 if OK.                                                      */
 ELMO_INT32    MMCLogger::LogMsgRoundFileCreation()
 // ===============================================
 {
    ELMO_INT32  iFileRotInd;
    ELMO_INT8	pcLogMsgFileAndPathStorageName[LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN];
    ELMO_INT8   pcCmdbuffer[CMD_BUFF_LEN];
    ELMO_INT32  iRtVal = MMC_OK;
    FILE *  	pFile;


    if ( (_pcLogMsgRemoteHostIp[0] != 0) &&     /* Requst report to remote (has host IP)*/
         (_bReportAlsoLocal == ELMO_FALSE) )    /* whether logging local when send logging to remote */
    {
                /* When: (report remote) and (Not report Local) there is no local files */
        return(MMC_OK);
    }

                /* After 'syslogd' opens/creates a new file (roll up to next*/
                /* file) the 'owner:group' of the created file is as        */
                /* 'syslogd' and probebly its 'root:user'. The process      */
                /* issue the start (if run under 'user' it has no permition */
                /* for open for "a". Here changing the files 'owner' (if    */
                /* exist) to "root"                                         */
                /* For indication if at least one log file exist, the 'Base */
                /* log file name' is using.                                 */
    iRtVal = access(_pcLogMsgFileAndPathStorageName, F_OK);
    if (iRtVal != -1)   /* When TRUE => exist base file... */
    {
                /* Command like:                                            */
                /*   su -c "chown user:user /mnt/suuf/usr/LogMsg.log*"      */
        snprintf(pcCmdbuffer, CMD_BUFF_LEN, "su -c \"chown user:user %s*\"", _pcLogMsgFileAndPathStorageName);
        iRtVal = system(pcCmdbuffer);
        iRtVal = WEXITSTATUS(iRtVal);
        if (iRtVal != 0)
        {
            printf("\n *** %s %s cmd:<%s> failed errno=%d \n", __FILE__, __func__, pcCmdbuffer, errno);
        }
    }
    iRtVal = MMC_OK;

                /* If file not exist => create it           */
                /* E.g.:                                    */
                /*  RotateFilesKeep = 2;                    */
                /*  Base file name  = "/mnt/jffs/usr/LogMsg"*/
                /*  Files should exist:                     */
                /*      "/mnt/jffs/usr/LogMsg.log"          */
                /*      "/mnt/jffs/usr/LogMsg.log.0"        */
                /*      "/mnt/jffs/usr/LogMsg.log.1"        */
    for (iFileRotInd = 0; iFileRotInd <= _iLogMsgNumRotateFilesKeep; iFileRotInd++)
    {
                /* File name for looking                    */
                /* "BaseFileName"  is with no index         */
        if (iFileRotInd == 0)
        {
            snprintf(pcLogMsgFileAndPathStorageName, LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN, "%s", _pcLogMsgFileAndPathStorageName);
        }
        else
        {
            snprintf(pcLogMsgFileAndPathStorageName, LOG_MSG_FILE_AND_PATH_STORAGE_NAME_LEN, "%s.%d", _pcLogMsgFileAndPathStorageName, iFileRotInd-1);
        }

                /* The file is created if it does not exist */
        pFile = fopen(pcLogMsgFileAndPathStorageName, "a");
        if (pFile == NULL)
        {
                /* for debug */
            printf("\n *** %s %s: failed fopen(%s, \"a\"); errno=%d \n", __FILE__, __func__, pcLogMsgFileAndPathStorageName, errno);
            iRtVal = errno;
            return (iRtVal);
        }
        fclose (pFile);
    }

    return (iRtVal);
 }


#endif

