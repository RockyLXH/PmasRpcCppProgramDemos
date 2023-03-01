/*
 * MMCMessage.cpp
 *
 *  Created on: 7/8/2011
 *      Author: dmitryb
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *              0.4.0 Updated 11Sep2017 Haim H.
 */

#include "OS_PlatformDependSetting.hpp"
#include "MMCMessage.hpp"
#include "MMCPPGlobal.hpp"
#include "MMC_definitions.h"

/*! \fn int ThrowMessage(char* cFileName,int iRetval,int iErrorID)
* \brief    The function display message to the user regarding some specific errorID and return value from function (take the data from source file)
*           This function can be executed by user (public)
* \param    iRetval - return value from API function
* \param    iErrorID - errorID that descibes some specific error or warning, this value received from OUTPUT structure of the API function
* \return - void
*/
void CMMCMessage::MessageHandler(ELMO_INT32 iRetval,ELMO_INT32 iErrorID)
{
    fstream pFile;
    //
    this->iRetval   = iRetval;
    this->iErrorID  = iErrorID;
    //
    // Update the message type
    UpdateMessageType();
    //
    if(eCurrentMessageType == MT_UNKNOWN_ERROR)
    {
        cout << "The combination of Return value and ErrorID is not defined." << endl;
        return;
    }
    //
    eConnectionType = CMMCPPGlobal::Instance()->GetConnectionType();
    //
    OpenMessageFile(pFile,(ELMO_PINT8)sMessageFileName.data());
    if(pFile.is_open() == false)
    {
        cout << "Error\\Warning messages XML file is missing" << endl;
        return;
    }
    GetAndPrintMessage(pFile);
    //
    CloseMessageFile(pFile);
    //
    return;
}
//
/*! \fn GetAndPrintMessage(FILE* pFile)
* \brief    This is internal function (private), the function receives FileDescriptor
*           where located data base about error and warning messages and and print
*           the relevant message (accordingly to the iRetval and iErrorID that
*           already assigned inside the class)
* \param    pFile - File Descriptor of source file (XML)
* \return - void
*/
void CMMCMessage::GetAndPrintMessage(fstream &pFile)
{
    //
    eCurrentHeaderType  =   HT_UNKNOWN_HEADER;
    bFOUND              =   false;
    //
    sMessage.clear();
    //
    ParseMessage(pFile);
    //
    if(sMessage.empty())
    {
        cout << endl << "Can not find message for this error\\warning." << endl;
    }
    else
    {
        cout << endl << sMessage << endl;
    }
    //
    return;
}
//
/*! \fn UpdateMessageType()
* \brief    This function update the internal variable that describes the
*           message type (eNUM) accordingly to the iRetval and iErrorID that
*           already assigned inside the class)
* \param    void
* \return   void
*/
void CMMCMessage::UpdateMessageType()
{
    //
    if((iRetval == -1) && (0 >= iErrorID) && (iErrorID > -1000))
    {
        eCurrentMessageType = MT_GMAS_ERROR;
        return;
    }
    //
    // In case when the retval is less then -1, it means that it is
    // a library error
    if(iRetval < -1)
    {
        eCurrentMessageType = MT_LIB_ERROR;
        return;
    }
    //
    if((iRetval > 0) && (iErrorID >= 1000) && (iErrorID < 2000))
    {
        eCurrentMessageType = MT_PROFILER_WARNING;
        return;
    }
    //
    if((iRetval > 0) && (iErrorID >= 2000) && (iErrorID < 3000))
    {
        eCurrentMessageType = MT_GMAS_WARNING;
        return;
    }
    //
    if((iRetval == -1) && (iErrorID <= -1000) && (iErrorID > -2000))
    {
        eCurrentMessageType = MT_PROFILER_ERROR;
        return;
    }
    //
    eCurrentMessageType = MT_UNKNOWN_ERROR;
    return;
}
//
/*! \fn OpenMessageFile(char* cFileName)
* \brief    This function opens tha XML source file.
*           If the connection is IPR take the file from default place.
*           If the connection is RPC take the file from the given path.
* \param    cFileName - path where the XML source file located (in RPC case)
* \return   FILE    -   File Descriptor of the opened file
*/
void CMMCMessage::OpenMessageFile(fstream &pFile, ELMO_PINT8 cFileName)
{
    //
    if(eConnectionType == CT_IPC)
    {
        //
        pFile.open(DEFAULT_MESSAGE_FILE_NAME, fstream::in);
        return;
    }
    //
    if(cFileName == NULL)
    {
        cout << "Missing filepath to source XML message file" << endl;
        return;
    }
    //
    pFile.open(cFileName,fstream::in);
    return;
    //
}
//
/*! \fn CloseMessageFile(FILE* pFile)
* \brief    This function close the opened file
* \param    pFile - File Descriptor that we want to close
* \return   void
*/
void CMMCMessage::CloseMessageFile(fstream &pFile)
{
    if(pFile.is_open())
    {
        pFile.close();
    }
    return;
}
//  /*! \fn ParseMessage(fstream &pFile)
//  * \brief    This function parse the error\warning message and enter the message to the sMessage variable
//  * \param   pFile    -   fstream class object, the object hold the XML source file
//  * \return   -   void
//  */
void CMMCMessage::ParseMessage(fstream &pFile)
{
    ELMO_INT8    cBufferForLine[MAX_CHARACTERS_IN_LINE];
    ELMO_INT8    cErrorID[10];
    string      sTempString;
    ELMO_INT32   iFoundIndex;
    //
    //
    eCurrentHeaderType = HT_UNKNOWN_HEADER;
    //
    ELMO_BOOL bDONE = false;
    while(!bDONE)
    {
        //
        if(pFile.eof())
        {
            bDONE = true;
            sTempString.clear();
            continue;
        }

        pFile.getline(cBufferForLine,MAX_CHARACTERS_IN_LINE-1);
        //
        if(bFOUND == false)
        {
            sTempString.clear();
        }
        //
        sTempString.append(cBufferForLine);
        sTempString.append("\n");
        //
        switch(eCurrentHeaderType)
        {
        //
        case HT_UNKNOWN_HEADER:
            //
            if((iFoundIndex = sTempString.find("<ErrorString>")) >= 0)
            {
                eCurrentHeaderType = HT_ERROR_STRING_HEADER;
            }
            //
            //
        break;
        //
        case HT_ERROR_STRING_HEADER:
            //
            if(bFOUND == true)
            {
                eCurrentHeaderType = HT_FINISH_HEADER;
                continue;
            }
            //
            if((iFoundIndex = sTempString.find("<ID>")) >= 0)
            {
                eCurrentHeaderType = HT_ID_HEADER;
            }
            //
            if(eCurrentMessageType != MT_LIB_ERROR)
            {
                sprintf(cErrorID," %d ",iErrorID);
            }
            else
            {
                // No difference between Library errors and other errors
                sprintf(cErrorID," %d ",iRetval);
            }
            //
            iFoundIndex = sTempString.find(cErrorID,0);
            if(iFoundIndex >= 0)
            {
                bFOUND = true;
            }
            //
            if((bFOUND == true) && (((iFoundIndex = sTempString.find("</ID>")) >= 0)))
            {
                sTempString.clear();
                sTempString.append("Description:");
                continue;
            }
            //
            if(bFOUND == true)
            {
                sTempString.clear();
            }
            //
        break;
        //
        case HT_ID_HEADER:
            //
            if((iFoundIndex = sTempString.find("<Text>")) >= 0)
            {
                eCurrentHeaderType = HT_TEXT_HEADER;
            }
            //
            if(eCurrentMessageType != MT_LIB_ERROR)
            {
                sprintf(cErrorID," %d ",iErrorID);
            }
            else
            {
                // No difference between Library errors and other errors
                sprintf(cErrorID," %d ",iRetval);
            }
            //
            iFoundIndex = sTempString.find(cErrorID,0);
            if(iFoundIndex >= 0)
            {
                bFOUND = true;
                sTempString.clear();
            }
            //
            if((((iFoundIndex = sTempString.find("</ID>")) >= 0)) && (bFOUND == true))
            {
                sTempString.append("Description:");
            }
            if((((iFoundIndex = sTempString.find("</Text>")) >= 0)) && (bFOUND == true))
            {
                sTempString.append("Resolution:");
            }
            //
        break;
        //
        case HT_TEXT_HEADER:
            //
            if((iFoundIndex = sTempString.find("<Resolution>")) >= 0)
            {
                eCurrentHeaderType = HT_RESOLUTION_HEADER;
            }
            //
            if((((iFoundIndex = sTempString.find("</Text>")) >= 0)) && (bFOUND == true) && (((iFoundIndex = sTempString.find("Resolution:")) < 0)))
            {
                sTempString.append("Resolution:");
            }
            //
        break;
        //
        case HT_RESOLUTION_HEADER:
            //
            if((iFoundIndex = sTempString.find("<ErrorString>")) >= 0)
            {
                eCurrentHeaderType = HT_ERROR_STRING_HEADER;
            }
            //
            if((((iFoundIndex = sTempString.find("</Resolution>")) >= 0)) && (bFOUND == true))
            {
                bDONE = true;
                continue;
            }
            //
        break;
        //
        case HT_FINISH_HEADER:
            //
            bDONE = true;
            continue;
            //
        break;
        //
        case HT_ERROR_IN_XML:
            //
            bDONE = true;
            sTempString.clear();
            continue;
            //
        break;
        //
        default:

        break;
        }
    }
    //
    //If the message not found , exit
    if(bFOUND == false)
    {
        sMessage.clear();
        return;
    }
    //
    // Lets do some fine tuning
    //
    //
    ELMO_PINT8 sTempForTuning;
    ELMO_PINT8 cFoundIndex;
    sTempForTuning = (ELMO_PINT8)malloc(sTempString.length() + 1);
    strcpy(sTempForTuning,sTempString.data());
    sTempForTuning[sTempString.length()]= '\0';
    //
    // Remove headers
    if((cFoundIndex = strstr(sTempForTuning,(const ELMO_PINT8)"<Text>")) != NULL)
    {
        for(ELMO_INT32 i = 0; (ELMO_UINT32)i < 6; i++)
        {
            cFoundIndex[i] = (ELMO_INT8)' ';
        }
    }
    //
    if((cFoundIndex = strstr(sTempForTuning,(const ELMO_PINT8)"</Text>")) != NULL)
    {
        for(ELMO_INT32 i = 0; (ELMO_UINT32)i < 7; i++)
        {
            cFoundIndex[i] = (ELMO_INT8)' ';
        }
    }
    //
    if((cFoundIndex = strstr(sTempForTuning,(const ELMO_PINT8)"<Resolution>")) != NULL)
    {
        for(ELMO_INT32 i = 0; (ELMO_UINT32)i < 12; i++)
        {
            cFoundIndex[i] = (ELMO_INT8)' ';
        }
    }
    //
    if((cFoundIndex = strstr(sTempForTuning,(const ELMO_PINT8)"</Resolution>")) != NULL)
    {
        for(ELMO_INT32 i = 0; (ELMO_UINT32)i < 13; i++)
        {
            cFoundIndex[i] = (ELMO_INT8)' ';
        }
    }
    //
    //
    sMessage.clear();
    sMessage.insert(0,sTempForTuning);
    free(sTempForTuning);
    return;
}
