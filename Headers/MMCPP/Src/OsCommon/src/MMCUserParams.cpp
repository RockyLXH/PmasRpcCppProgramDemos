/*
 * MMCUserParams.cpp
 *
 *  Created on: 09/04/2013
 *      Author: HaimH
 *      Update: 30Oct2013
 *      Update: 16Jun2014 HH:
 *                  1: Change Open() / parse flags to:
 *                              parse_declaration_node|parse_no_data_nodes
 *                  2: Change Close(), add:
 *                              m_pDeepLvl0Nodes = NULL 
 *      Update: 24Feb2015 Haim H. for Integration Os (32/64 bit for LinuxIpc and Win, Vxworks and Linux Rpc)
 *		Update: 14Nov2017 HH:
 *					On Close() function, Change location of
 *								m_pDeepLvl0Nodes = NULL;
 *								for be non condition.
 *      Author: HaimH
 *
 *
 *  RAPIDXML - using tools:
 *  =======================
 *      The  RAPIDXML library (free tools source form downloaded) where used in order to parse the XML file.
 *      Current invoke version: RapidXml 1.13
 *      RapidXML Home: http://rapidxml.sourceforge.net/
 *      RapidXML Download: http://sourceforge.net/projects/rapidxml/files/latest/download
 *
 *      By default, RapidXml uses C++ exceptions to report errors.
 *      If this behaviour is undesirable, RAPIDXML_NO_EXCEPTIONS can be defined to suppress exception code.
 *      See parse_error class and parse_error_handler() function for more information.
 */
//
#define RAPIDXML_USING_VER      "RAPIDXML-1.13"
//
#define TAG_ATTR_NAME_LVL0_1    "xmlns:xsi"
#define TAG_ATTR_NAME_LVL0_2    "xsi:noNamespaceSchemaLocation"
//
#define TAG_NAME_LVL1_DESCRI    "FILE_DESCRIPTION"
//
#define ATTR_NAME_LVL1_1_NAM    "NAME"
#define ATTR_NAME_LVL1_2_VER    "VERSION"
                                        /* Root first attribute. */
#define UPXML_ID_VER_NAME       "version"
#define UPXML_ID_VER            "1.0"
#define UPXML_ID_ENCOD_NAME     "encoding"
#define UPXML_ID_ENCOD          "utf-8"
#define UPXML_ROOT_NAME         "root"
//
#include "OS_PlatformDependSetting.hpp"
//
#define   RAPIDXML_NO_EXCEPTIONS
//

#ifdef PROAUT_CHANGES
	//save compiler switches
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "MMCUserParams.hpp"
//
using namespace                 rapidxml;
using namespace                 std;
//
//
xml_document<>  g_doc;
//
// Set error as well as the Def Value for returned
#define SET_DEF_AND_ERROR_REP(P_Ret, P_Def, P_Warng, P_Err)     \
                                                                \
                if (bDefSetReq)                                 \
                {                                               \
                    P_Ret = P_Def;                              \
                    iRetCode = MMC_LIB_UPXML_WARNING;           \
                    iErrId = P_Warng;                           \
                }                                               \
                else                                            \
                {                                               \
                    iRetCode = MMC_LIB_UPXML_ERROR;             \
                    iErrId = P_Err;                             \
                }                                               \
//
// Only sets error.
#define SET_ERROR_REP(P_Warng, P_Err)                           \
                if (bDefSetReq)                                 \
                {                                               \
                    iRetCode = MMC_LIB_UPXML_WARNING;           \
                    iErrId = P_Warng;                           \
                }                                               \
                else                                            \
                {                                               \
                    iRetCode = MMC_LIB_UPXML_ERROR;             \
                    iErrId = P_Err;                             \
                }                                               \
//
#define REP_READ_GET_SEQUENCE_ERROR()                           \
                {                                               \
                    iErrId = MMC_LIB_UPXML_SEQUENCE;            \
                    if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)       \
                    {                                           \
                        printf(                                 \
                        "\n *** UPXML Failed,sequence error "); \
                            fflush(stdout); fflush(stderr);     \
                    }                                           \
                    CMMCPPGlobal::Instance()->MMCPPThrow(       \
						"UPXML Sequence err", (MMC_CONNECT_HNDL)-1, -1,	\
                        MMC_LIB_UPXML_ERROR, iErrId, 0);        \
                }                                               \
//
//
MMCUserParams::MMCUserParams(
	ELMO_UINT32 uiDefSetReqFlg,   /* Mark use def. val. when not found entry on XML data  */
	ELMO_UINT32 uiSpeakFlag)      /* Mark speak to consol the set level and below it.     */
	//         unsigned int uiSpeakFlag=SPK_LVL_INFO)
	//         unsigned int uiSpeakFlag=SPK_LVL_DBG)
	//         unsigned int uiSpeakFlag=SPK_LVL_NONE)
{
	m_uiDefSetReqFlg = uiDefSetReqFlg;
	/* Hierarchy value for consol prints        */
	/* Param val should be symbols SPK_LVL_*    */
	m_uiSpeakDbgFlg = uiSpeakFlag;
	/* Pointer to XML file in (readed) to Ram   */
	m_pXmlRamFileBuffer = NULL;
	/* Pointers to XML root (level0) tree node  */
	m_pDeepLvl0Nodes = NULL;
	/* Initialize sprintf Double format         */
	sprintf(m_sprintfDoubleFormat, "%s", SPRINTF_DEF_DOUBLE_FORMAT);
	sprintf(m_sprintfDoubleFormatAr, "%s%s", SPRINTF_DEF_DOUBLE_FORMAT, ",");
}

MMCUserParams::~MMCUserParams()
{
        if (m_pXmlRamFileBuffer != NULL)
        {
            delete [] m_pXmlRamFileBuffer;
            m_pXmlRamFileBuffer = NULL;
        }
        g_doc.clear(); 
}
                /*
                 * Related to RapidXml using:
                 * When exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS,
                 * this function is called to notify user about the error.
                 * This function cannot return. If it does, the results are undefined. 
                 * what    Human readable description of the error. 
                 * where   Pointer to character data where error was detected. 
                 */
//void rapidxml::parse_error_handler(const ELMO_PINT8 what, ELMO_PVOID where)
// HH: define in tream of rapidxml
void rapidxml::parse_error_handler(const char* what, void* where)
// ==============================================================
{
    printf("\n *** UPXML (RapidXml) Parse error /%s/,\n Error location on XML File:\n /%s/ ", what, (ELMO_PINT8)where);
    fflush(stdout); fflush(stderr);                                                                 \
//
	CMMCPPGlobal::Instance()->MMCPPThrow("UPXML XML file parse error", (MMC_CONNECT_HNDL)-1, -1,
										 MMC_LIB_UPXML_ERROR, MMC_LIB_UPXML_FILE_FORMAT, 0);
//
                /* In case the MMCPPThrow function setting cause return...          */
                /* According RapidXML doc, seem 'abort' | 'exit' should be call     */
                /* but I am not shure... (Maybe, on this case, only RapidXML funcs  */
                /* should not calling anymore)                                      */
    std::abort();
}
//
ELMO_INT32 MMCUserParams::Open(ELMO_PINT8 cFileName, ELMO_UINT32 uiFlags, ELMO_PINT8 cFilePath) throw (CMMCException)
// ==================================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   fileSize, MemAllocSize;
    ELMO_INT32   readLen;
    ELMO_INT32   fileNameLem;
    ELMO_INT8    fileName[MAX_XML_FILE_NAME_LEN];
                    /* File descriptor pointer */
    FILE *      stXmlFd;
//
    if (m_pXmlRamFileBuffer != NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()                                                                                       \
        return (iErrId);
    }
//
#if ((OS_PLATFORM == WIN32_PLATFORM) || (OS_PLATFORM == WIN64_PLATFORM))
    printf("\n !!! USER PARAM (XML): FUNCTIONS WAS NOT TESTED AND SHOULD NOT RUN ON Windows !!!! \n");
#endif
//
                    /* Decode the exception and default value beahver mode  */
                    /* Use bitwise flags directs how to use the default     */
                    /* if the requsted entry not found on file.             */
    m_uiDefSetReqFlg = uiFlags & UPXML_SET_DEF_REQ_FLG;
//
    if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
    {
        printf("\n UPXML (User Param XML): ^^^^^^^^^^^^^^^^^^^^^^  STR  ^^^^^^^^^^^^^^^^^^^^^^^^^ ");
        printf("\n Version: DATE=%s, TIME=%s ", __DATE__, __TIME__);
        printf("\n %s", RAPIDXML_USING_VER);
    }
//
    fileSize = readLen = 0;
    fileNameLem =  strlen(cFilePath);
    fileNameLem += strlen(cFileName);
    if ((fileNameLem-1) >= MAX_XML_FILE_NAME_LEN)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML Compose Read FILE NAME too LONG=%d (Max=%d) ", fileNameLem, MAX_XML_FILE_NAME_LEN);
            fflush(stdout); fflush(stderr);                                                                 \
        }
//
        iErrId = MMC_LIB_UPXML_LONG_NAME;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read file NAME too LONG ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
        return (iErrId);
    }
    sprintf(fileName, "%s%s", cFilePath, cFileName);
    if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
    {
        printf("\n UPXML read file name: <%s> ", fileName);
    }
//
#ifdef HH_NOT_IMPLEMENT_NOW
//
#include "rapidxml_utils.hpp"
ELMO_INT32 fuc_main()
{
                /* xmlFile object contains all of the data for the XML, */
                /* once it goes out of scope and is destroyed the doc   */
                /* variable is no longer safely usable.                 */
                /* If calling parse inside of a function, must somehow  */
                /* retain the xmlFile object in memory (global variable,*/
                /* new, etc) so that the doc remains valid              */
    rapidxml::file<> xmlFile("somefile.xml"); // Default template is char
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());
...
}
//
#endif  /* #ifdef HH_NOT_IMPLEMENT_NOW */
//
    stXmlFd = fopen(fileName, "r");
    if (NULL == stXmlFd)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML Failed fopen for read: (%s) ", fileName);
            fflush(stdout); fflush(stderr);                                                                 \
        }
//
        iErrId = MMC_LIB_UPXML_OPEN_FAILD;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Open, file name", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
//
        return (iErrId);
    }
//
                    /* Set for standing on EOF (Offset 0)   */
    fseek(stXmlFd, 0, SEEK_END);
    fileSize = ftell(stXmlFd);
                    /* Rewind, standing on BOF (Offset 0)   */
    fseek(stXmlFd, 0, SEEK_SET);
                    /* Add 0 after end of file data */
    MemAllocSize = fileSize+1;
    m_pXmlRamFileBuffer = new ELMO_INT8[MemAllocSize];
    if (m_pXmlRamFileBuffer == NULL)
    {
        fclose(stXmlFd);
//
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf ("\n *** UPXML Failed to allocate memory, size=%d ", MemAllocSize);
            fflush(stdout); fflush(stderr);                                                                 \
        }
//
        iErrId = MMC_LIB_UPXML_FAIL_ALLOC;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Open, new", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
        return (iErrId);
    }

    readLen = (ELMO_INT32)fread(m_pXmlRamFileBuffer, 1, fileSize, stXmlFd);
    m_pXmlRamFileBuffer[readLen] = 0;
//
    if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
    {
        printf("\n UPXML File: %s, Size=%d Bytes ", fileName, fileSize);
        printf("\n");
    }
//
    fclose(stXmlFd);
//
                            /* Parse the buffer using the xml file parsing library into     */
                            /* g_doc see rapidxml.hpp for the using flags (change of def):  */
                            /*       'parse_no_data_nodes'                                  */
    g_doc.parse<parse_declaration_node|parse_no_data_nodes>(&m_pXmlRamFileBuffer[0]);
                            /* Print the XML DATA structure */
    if (m_uiSpeakDbgFlg >= SPK_LVL_DBG)
    {
        printf("\n UPXML INPUT");
        printf("\n ===========\n");
                            /* 'print' is RAPIDXML function */
        print(std::cout, g_doc, 0);
    }
                            /* Find our root node */
    m_pDeepLvl0Nodes  = (ELMO_PVOID)(g_doc.first_node(UPXML_ROOT_NAME));
//
    if (m_pDeepLvl0Nodes != NULL)
    {
        return (MMC_OK);
    }
    else
    {
        delete [] m_pXmlRamFileBuffer;
        m_pXmlRamFileBuffer = NULL;
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf ("\n *** UPXML Parsing Failed - test file format ");
            fflush(stdout); fflush(stderr);                                                                 \
        }
//
        iErrId = MMC_LIB_UPXML_FILE_FORMAT;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Open, XML file form", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
        return (iErrId);
    }
}
//
//
                    /* Mark speak to consol the set level and below it. */
                    /* Param val should be constant symbols SPK_LVL_*   */
    void  MMCUserParams::setSpeakDbgLvl(ELMO_UINT32 uiSpeak_lvl)
// ============================================================
{
    printf("\n UPXML change Speak level from %d to %d ", m_uiSpeakDbgFlg, uiSpeak_lvl);
    m_uiSpeakDbgFlg = uiSpeak_lvl;
}
                    /* On XML elements from Lvl1, look for tag-NAME (fix program predefine Name pLvl1Name) has the      */
                    /* attribute value=pLvl1Valu, if found, under it (now we are in Lvl2) look for tag-NAME (fix program*/
                    /* predefine Name pLvl2Name) has the attribute value=pLvl2Valu, if found, under it (now we are in   */
                    /* Lvl3) look for tag-Name=pLvl3Name, if found the Value of this Lvl3 tags Name return to pBufPrt   */
                    /* and function return MMC_OK, if not found put NULL in *pBufPrt.                                   */
                    /*                                                                                                  */
                    /* BE AWARE: parameter 1 & 2 is for lvl1 & lvl2 Tag Attribute (not tags Name, the name is fix and   */
                    /* predefine [Hard codded along program running], but parameter 3 is NAME of the tag's              */
                    /* element (not atribute, not value)...                                                             */
                    /* If found return pointer to string location (text) = the DATA TEXT belong to element Name in      */
                    /* parameter pLvl3Name, return it to pBufPrt                                                        */
                    /*                                                                                                  */
                    /* REMARK: function prototype define in '.h' should not include RapidXML data type because the user */
                    /*  not recognize it... So, do not define 'void ** ppLvl1Nodes' as 'xml_node<> ** ppLvl1Nodes' etc. */
ELMO_INT32 MMCUserParams::getTagValue(ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,    ELMO_PINT8  pLvl3Name,    ELMO_PINT8* pBufPrt,
                                     ELMO_PVOID*  ppLvl1Nodes,ELMO_PVOID*  ppLvl2Nodes,  ELMO_PVOID* ppLvl3Nodes,
                                     ELMO_PINT8   pLvl1Name,  ELMO_PINT8   pLvl2Name)
// =====================================================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   iCmpRsult;
//
    xml_node<>* pm_pDeepLvl0Nodes = NULL;
    xml_node<>* pDeepLvl1Nodes    = NULL;
    xml_node<>* pDeepLvl2Nodes    = NULL;
    xml_node<>* pDeepLvl3Nodes    = NULL;
//
    * pBufPrt = NULL;
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
    iErrId = MMC_LIB_UPXML_NOT_FOUND; // Init -> not found
//
    pm_pDeepLvl0Nodes = (xml_node<> *)m_pDeepLvl0Nodes;
    pDeepLvl1Nodes = pm_pDeepLvl0Nodes->first_node(pLvl1Name);
    while (pDeepLvl1Nodes != NULL)
    {
        iCmpRsult = cmpAttrib(1, pDeepLvl1Nodes, pLvl1Valu);
        if (iCmpRsult != MMC_OK)
        {
            /* Predefine Tag Lvl1, has the given attribute did not found            */
            /* look for the sibling element of the same node along it have sibling  */
            pDeepLvl1Nodes = pDeepLvl1Nodes->next_sibling(pLvl1Name);
            continue;
        }
                 /* FOUND Predefine LEVEL 1 tag has REQUSTED ATTRIBUTE *\
                 \* ===================================================*/
        pDeepLvl2Nodes = pDeepLvl1Nodes->first_node(pLvl2Name);
        while (pDeepLvl2Nodes != NULL)
        {
            iCmpRsult = cmpAttrib(2, pDeepLvl2Nodes, pLvl2Valu);
            if (iCmpRsult != MMC_OK)
            {
                /* Predefine Tag Lvl2, has the given attribute did not found            */
                /* look for the sibling element of the same node along it have sibling  */
                pDeepLvl2Nodes = pDeepLvl2Nodes->next_sibling(pLvl2Name);
                continue;
            }
            /* FOUND Predefine LEVEL 2 tag has REQUSTED ATTRIBUTE *\
            \* ===================================================*/
            pDeepLvl3Nodes = pDeepLvl2Nodes->first_node(pLvl3Name);
            if (pDeepLvl3Nodes != NULL)
            {
                /* FOUND !!! LEVEL 3 Tag has given Name *\
                \* ==================================== */
                // handleNodes(3, pDeepLvl3Nodes);
                * pBufPrt = pDeepLvl3Nodes->value();
                iErrId = MMC_OK;
                if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
                    {
                        printf("\n UPXML FOUND ELEM: .../%s/%s/%s='%s'/ ", pLvl1Valu, pLvl2Valu, pLvl3Name, *pBufPrt);
                    }
            }
            else
            {
                if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
                {
                    printf("\n UPXML NOT FOUND: /%s NAME=%s/%s NAME=%s/'%s' ",
                                                      pLvl1Name, pLvl1Valu, pLvl2Name, pLvl2Valu, pLvl3Name);
                }
            }
            goto getTagValueDone;
        }
        if (pDeepLvl2Nodes == NULL)
        {
            if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
                {
                    printf("\n UPXML NOT FOUND: /%s NAME=%s/'%s'/  or /%s NAME=%s/%s NAME='%s' ",
                                                      pLvl1Name, pLvl1Valu, pLvl2Name, 
                                                      pLvl1Name, pLvl1Valu, pLvl2Name, pLvl2Valu);
                }
            goto getTagValueDone;
        }
    }
//
    if (pDeepLvl1Nodes == NULL)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
            {
                printf("\n UPXML NOT FOUND: /'%s'/  or /%s NAME='%s'/ ", pLvl1Name, pLvl1Name, pLvl1Valu);
            }
    }
//
getTagValueDone:
    if (ppLvl1Nodes != NULL)
        *ppLvl1Nodes = (ELMO_PVOID)pDeepLvl1Nodes;
//
    if (ppLvl2Nodes != NULL)
        *ppLvl2Nodes = (ELMO_PVOID)pDeepLvl2Nodes;
//
    if (ppLvl3Nodes != NULL)
        *ppLvl3Nodes = (ELMO_PVOID)pDeepLvl3Nodes;
//
    return (iErrId);
}
                    /* Return MMC_OK if the first attribute value of the pointed    */
                    /* node equal to value pointed by 'pLookForAttrVal'             */
ELMO_INT32 MMCUserParams::cmpAttrib(ELMO_UINT32 uiDeepLvl, ELMO_PVOID  pNodeV,  ELMO_PINT8 pLookForAttrVal)
// ========================================================================================================
{
    ELMO_PINT8           pNodeAttrVal;
    xml_attribute<> *   pNodeAttrib;
    ELMO_INT32           rtVal;
    xml_node<> *        pNode;
//
    pNode = (xml_node<> *)pNodeV;
    rtVal = MMC_LIB_UPXML_NOT_FOUND;    /* Not equal */
//
    pNodeAttrib = pNode->first_attribute();
    if(pNodeAttrib)
    {
        pNodeAttrVal = pNodeAttrib->value();
                    /* strcmp return 0 if equel */
        if (strcmp(pNodeAttrVal, pLookForAttrVal) == 0)
            rtVal = MMC_OK;
    }

    return (rtVal);
}
//
//
ELMO_INT32 MMCUserParams::Close()
// ==============================
{
    if (m_pXmlRamFileBuffer != NULL)
    {
        delete [] m_pXmlRamFileBuffer;
        m_pXmlRamFileBuffer = NULL;
    }
//
        m_pDeepLvl0Nodes = NULL;
//
    g_doc.clear(); 
//
    if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
    {
        printf("\n UPXML (User Param XML): vvvvvvvvvvvvvvvvvvvvvv  END  vvvvvvvvvvvvvvvvvvvvvvvvv ");
        printf("\n");
    }
    return (MMC_OK);
}
//
//
                        /* Read Single (one) Double */
ELMO_INT32 MMCUserParams::Read ( ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                ELMO_DOUBLE& dRetVal,    ELMO_DOUBLE  dDefault,
                                ELMO_DOUBLE  dMin,       ELMO_DOUBLE  dMax)             throw (CMMCException)
// ==========================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   iRetCode;
    ELMO_PINT8   pBufPrt;
    ELMO_PINT8   endptr;
    ELMO_BOOL    bDefSetReq;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
                            /* For shorter and efficient (use in the macro)... */
    bDefSetReq = (m_uiDefSetReqFlg & UPXML_SET_DEF_REQ_FLG) == UPXML_SET_DEF_REQ_FLG;
//
    iRetCode = MMC_OK;
    iErrId = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt, (ELMO_PVOID*)0, (ELMO_PVOID*)0, (ELMO_PVOID*)0);

    if(pBufPrt != NULL)
    {
        dRetVal = strtod (pBufPrt, &endptr);
        if(pBufPrt == endptr)
        {
            /* Found, but its value cannot translate to double...               */
            SET_DEF_AND_ERROR_REP (dRetVal, dDefault, MMC_LIB_UPXML_DEF_UNEXP_CHR, MMC_LIB_UPXML_UNEXP_CHR)
        }
        else
        {   /* If found and the value is between specific Min-Max values.       */
            /* If it is not, a default value will be set.                       */
            /* If a default value is set, no exception will be thrown           */
            if (dRetVal < dMin  || dRetVal > dMax)
            {
                SET_DEF_AND_ERROR_REP (dRetVal, dDefault, MMC_LIB_UPXML_DEF_NOT_IN_RANGE, MMC_LIB_UPXML_NOT_IN_RANGE)
            }
        }
    }
    else
    {
        SET_DEF_AND_ERROR_REP (dRetVal, dDefault, MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)
    }
//
    if (iErrId != MMC_OK)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read double", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
    return(iErrId);
}
//

                        /* Read Single (one) Long */
ELMO_INT32 MMCUserParams::Read  (   ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                    ELMO_LINT32& lRetVal,    ELMO_LINT32  lDefault,
                                    ELMO_LINT32  lMin,       ELMO_LINT32  lMax)         throw (CMMCException)
// ==========================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   iRetCode;
    ELMO_PINT8   pBufPrt;
    ELMO_PINT8   endptr;
    ELMO_BOOL    bDefSetReq;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
                            /* For shorter and efficient (use in the macro)... */
    bDefSetReq = (m_uiDefSetReqFlg & UPXML_SET_DEF_REQ_FLG) == UPXML_SET_DEF_REQ_FLG;
//
    iRetCode = MMC_OK;
    iErrId = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt, (ELMO_PVOID*)0, (ELMO_PVOID*)0, (ELMO_PVOID*)0);
    if(pBufPrt != NULL)
    {
        lRetVal = strtol (pBufPrt, &endptr, 0);
        if(pBufPrt == endptr)
        {
            /* Found, but its value cannot translate to long...                 */
            SET_DEF_AND_ERROR_REP (lRetVal, lDefault, MMC_LIB_UPXML_DEF_UNEXP_CHR, MMC_LIB_UPXML_UNEXP_CHR)
        }
        else
        {   /* If found and the value is between specific Min-Max values.       */
            /* If it is not, a default value will be set.                       */
            /* If a default value is set, no exception will be thrown           */
            if (lRetVal < lMin || lRetVal > lMax)
            {
                SET_DEF_AND_ERROR_REP (lRetVal, lDefault, MMC_LIB_UPXML_DEF_NOT_IN_RANGE, MMC_LIB_UPXML_NOT_IN_RANGE)
            }
        }
    }
    else
    {
        SET_DEF_AND_ERROR_REP (lRetVal, lDefault, MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)
    }
//
    if (iErrId != MMC_OK)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read long", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
    return(iErrId);
}
//
                    /* Return ONE parameter of type bool.               */
                    /* Ignore white space, expect True / False.         */
ELMO_INT32 MMCUserParams::Read  (ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                 ELMO_BOOL    &bRetVal,   ELMO_BOOL    bDefault)        throw (CMMCException)
// ==========================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   iRetCode;
    ELMO_INT32   idx;
    ELMO_PINT8   pBufPrt;
    ELMO_INT8    localBuf[20];
    ELMO_BOOL    bDefSetReq;

    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
                            /* For shorter and efficient (use in the macro)... */
    bDefSetReq = (m_uiDefSetReqFlg & UPXML_SET_DEF_REQ_FLG) == UPXML_SET_DEF_REQ_FLG;
//
    iRetCode = MMC_OK;
    iErrId = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt, (ELMO_PVOID*)0, (ELMO_PVOID*)0, (ELMO_PVOID*)0);
    if(pBufPrt != NULL)
    {
        sscanf(pBufPrt,  " %20s", localBuf);
        idx = 0;
        while (localBuf[idx])
          {
            localBuf[idx] = tolower(localBuf[idx]);
            idx++;
          }
        if (strstr(localBuf, TRUE_FILE_STRING) != 0)
            bRetVal = true;
        else if (strstr(localBuf, FALSE_FILE_STRING) != 0)
            bRetVal = false;
        else
        {
            SET_DEF_AND_ERROR_REP (bRetVal, bDefault, MMC_LIB_UPXML_DEF_UNEXP_CHR, MMC_LIB_UPXML_UNEXP_CHR)
        }
    }
    else
    {
        /* Not found the key...                                             */
        SET_DEF_AND_ERROR_REP (bRetVal, bDefault, MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)
    }
//
    if (iErrId != MMC_OK)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read boolean", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
//
    return(iErrId);
}
//
                        /* Read String */
ELMO_INT32 MMCUserParams::Read  (   ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                    ELMO_PINT8   pStr,       ELMO_LINT32  lLen)         throw (CMMCException)
// ==========================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   iRetCode;
    ELMO_PINT8   pBufPrt;
//
    iRetCode = MMC_OK;
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
    iErrId = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt, (ELMO_PVOID*)0, (ELMO_PVOID*)0, (ELMO_PVOID*)0);
//
    if(pBufPrt != NULL)
    {
        strncpy(pStr, pBufPrt, lLen);
    }
    else
    {
        /* Not found... */
        pStr[0] = 0;
        iRetCode = MMC_LIB_UPXML_ERROR;
        iErrId = MMC_LIB_UPXML_NOT_FOUND;           /* ENTRY NOT Found   */
//
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read string", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
	}
//
    return(iErrId);
}
//
                        /* Read Array of Double */
ELMO_INT32 MMCUserParams::ReadArr  (ELMO_PINT8    pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                    ELMO_DOUBLE   dRetVal[],  ELMO_DOUBLE  dDefault,
                                    ELMO_UINT32&  iActRdElm,  ELMO_UINT32  iReqRdElm,
                                    ELMO_DOUBLE   dMin,       ELMO_DOUBLE  dMax)         throw (CMMCException)
// ===========================================================================================================
{
    ELMO_INT32  iErrId;
    ELMO_INT32  iRetCode;
    ELMO_PINT8  pBufPrt;
    ELMO_PINT8  endptr;
    ELMO_BOOL   bDefSetReq;
    ELMO_UINT32 indActRd = 0;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
    iRetCode = MMC_OK;
//
    iActRdElm = 0;
                            /* For shorter and efficient (use in the macro)... */
    bDefSetReq = (m_uiDefSetReqFlg & UPXML_SET_DEF_REQ_FLG) == UPXML_SET_DEF_REQ_FLG;
    iErrId = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt, (ELMO_PVOID*)0, (ELMO_PVOID*)0, (ELMO_PVOID*)0);
    if(pBufPrt != NULL)
    {
        for(indActRd=0; indActRd < iReqRdElm; indActRd++)
        {
            dRetVal[indActRd] = strtod (pBufPrt, &endptr);
            if(pBufPrt == endptr)
            {
                SET_DEF_AND_ERROR_REP (dRetVal[indActRd], dDefault, MMC_LIB_UPXML_DEF_UNEXP_CHR, MMC_LIB_UPXML_UNEXP_CHR)
            }
            else
            {
                                            /* Update num of elem read from file, regards of*/
                                            /* they are in range.                           */
                iActRdElm = indActRd + 1;
//
                if (dRetVal[indActRd] < dMin || dRetVal[indActRd] > dMax) 
                {
                    SET_DEF_AND_ERROR_REP (dRetVal[indActRd], dDefault, MMC_LIB_UPXML_DEF_NOT_IN_RANGE, MMC_LIB_UPXML_NOT_IN_RANGE)
                }
            }
            pBufPrt = strchr(endptr, ',');
            if (pBufPrt != NULL)
            {
                pBufPrt++;
            }
            else
            {
											/* After last number can be but not must be comma... */
				if (indActRd+1 < iReqRdElm)
				{
					SET_ERROR_REP (MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)
//              	
					if (bDefSetReq)
					{
						for(indActRd=indActRd+1; indActRd < iReqRdElm; indActRd++)
						{
							dRetVal[indActRd] = dDefault;
						}
						break;
					}
                }
            }
        }
    }
    else
    {
        SET_ERROR_REP (MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)

        if (bDefSetReq)
        {
            for(indActRd=0; indActRd < iReqRdElm; indActRd++)
            {
                dRetVal[indActRd] = dDefault;
            }
        }
    }
//
    if (iErrId != MMC_OK)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read array double", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
//
    return(iErrId);
}
//
                        /* Read Array of Long */
ELMO_INT32 MMCUserParams::ReadArr  ( ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                    ELMO_LINT32  lRetVal[],  ELMO_LINT32  lDefault,
                                    ELMO_UINT32& iActRdElm,  ELMO_UINT32  iReqRdElm,
                                    ELMO_LINT32  lMin,       ELMO_LINT32  lMax)                   throw (CMMCException)
// ==================================================================================================================
{
    ELMO_INT32  iErrId;
    ELMO_INT32  iRetCode;
    ELMO_PINT8  pBufPrt;
    ELMO_PINT8  endptr;
    ELMO_BOOL   bDefSetReq;
    ELMO_UINT32 indActRd = 0;

    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
    iRetCode = MMC_OK;
//
    iActRdElm = 0;
                            /* For shorter and efficient (use in the macro)... */
    bDefSetReq = (m_uiDefSetReqFlg & UPXML_SET_DEF_REQ_FLG) == UPXML_SET_DEF_REQ_FLG;
//
    iErrId = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt, (ELMO_PVOID*)0, (ELMO_PVOID*)0, (ELMO_PVOID*)0);
    if(pBufPrt != NULL)
    {
        for(indActRd=0; indActRd < iReqRdElm; indActRd++)
        {
            lRetVal[indActRd] = strtol (pBufPrt, &endptr, 0);
            if(pBufPrt == endptr)
            {
                SET_DEF_AND_ERROR_REP (lRetVal[indActRd], lDefault, MMC_LIB_UPXML_DEF_UNEXP_CHR, MMC_LIB_UPXML_UNEXP_CHR)
            }
            else
            {
                                        /* Update num of elem read from file, regards of*/
                                        /* they are in range.                           */
                iActRdElm = indActRd + 1;

                if (lRetVal[indActRd] < lMin || lRetVal[indActRd] > lMax)
                {
                    SET_DEF_AND_ERROR_REP (lRetVal[indActRd], lDefault, MMC_LIB_UPXML_DEF_NOT_IN_RANGE, MMC_LIB_UPXML_NOT_IN_RANGE)
                }
            }
            pBufPrt = strchr(endptr, ',');
            if (pBufPrt != NULL)
            {
                pBufPrt++;
            }
            else
            {
											/* After last number can be but not must be comma... */
				if (indActRd+1 < iReqRdElm)
				{
					SET_ERROR_REP (MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)
//              	
					if (bDefSetReq)
					{
						for(indActRd=indActRd+1; indActRd < iReqRdElm; indActRd++)
						{
							lRetVal[indActRd] = lDefault;
						}
						break;
					}
                }
            }
        }
    }
    else
    {
        SET_ERROR_REP (MMC_LIB_UPXML_DEF_NOT_FOUND, MMC_LIB_UPXML_NOT_FOUND)
//
        if (bDefSetReq)
        {
            for(indActRd=0; indActRd < iReqRdElm; indActRd++)
            {
                lRetVal[indActRd] = lDefault;
            }
        }
    }
    if (iErrId != MMC_OK)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Read array double", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
//
    return(iErrId);
}
                            /* Return XML file identifier *\
                            \* ========================== */
                    /* Return the XML file root xsi ID values (attribute1 &         */
                    /* attribute2 (Location).                                       */
                    /* both buffer for return values are at least of size lLen      */
                    /* Eg, for:                                                     */
                    /*  <root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" */
                    /*          xsi:noNamespaceSchemaLocation="proposed.xsd">       */
                    /*      pAtt1 = "http://www.w3.org/2001/XMLSchema-instance"     */
                    /*      pAtt2 = "proposed.xsd"                                  */
                    /* return DONE only  if both of the looking for attr. found     */
                    /* lLen is the min buff size deliver for this Get operation.    */
ELMO_INT32 MMCUserParams::GetXmlFileRoot(ELMO_PINT8 pAtt1, ELMO_PINT8 pAtt2, ELMO_LINT32 lLen) throw (CMMCException)
// =============================================================================================================
{
    ELMO_INT32          iErrId;
    ELMO_INT32          iRetCode;
    ELMO_BOOL           firstFnd;
    ELMO_BOOL           secondFnd;
    ELMO_PINT8          pNodeAttrVal;
    xml_attribute<> *   pNodeAttrib;
    xml_node<> *        pm_pDeepLvl0Nodes;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
    pAtt1[0] = pAtt2[0] = 0;
    firstFnd = secondFnd = false;
//
    iRetCode = MMC_LIB_UPXML_ERROR;
    iErrId = MMC_LIB_UPXML_NOT_FOUND;
//
    if (m_pDeepLvl0Nodes != NULL)
    {
        pm_pDeepLvl0Nodes = (xml_node<> *)m_pDeepLvl0Nodes;

        pNodeAttrib = pm_pDeepLvl0Nodes->first_attribute(TAG_ATTR_NAME_LVL0_1);
        if (pNodeAttrib != 0)
        {
            firstFnd = true;
            pNodeAttrVal = pNodeAttrib->value();
            if (pNodeAttrVal)
                strncpy(pAtt1, pNodeAttrVal, lLen);
        }
        pNodeAttrib = pm_pDeepLvl0Nodes->first_attribute(TAG_ATTR_NAME_LVL0_2);
        if (pNodeAttrib != 0)
        {
            secondFnd = true;
            pNodeAttrVal = pNodeAttrib->value();
            if (pNodeAttrVal)
                strncpy(pAtt2, pNodeAttrVal, lLen);
        }
    }
//
    if (!firstFnd || !secondFnd)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML get root", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
    else
        iErrId = MMC_OK;
//
    return(iErrId);
}
//
                    /* Retur the XML "file description name" and XML file ver       */
                    /* both buffer for return values are at least of size lLen      */
                    /* return DONE only  if both of the looking for attr. found     */
ELMO_INT32 MMCUserParams::GetXmlFileDescrp(ELMO_PINT8 pAtt1, ELMO_PINT8 pAtt2, ELMO_LINT32 lLen) throw (CMMCException)
// ===============================================================================================================
{
    ELMO_INT32          iErrId;
    ELMO_INT32          iRetCode;
    ELMO_BOOL           firstFnd;
    ELMO_BOOL           secondFnd;
    ELMO_PINT8          pNodeAttrVal;
                        /* Pointers to nodes level1 */
    xml_node<> *        pDeepLvl1Nodes;
    xml_attribute<> *   pNodeAttrib;
    xml_node<> *        pm_pDeepLvl0Nodes;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
    pAtt1[0] = pAtt2[0] = 0;
    firstFnd = secondFnd = false;
//
    iRetCode = MMC_LIB_UPXML_ERROR;
    iErrId = MMC_LIB_UPXML_NOT_FOUND;
//
            /* Look on level 1 name TAG_NAME_LVL1_DESCRI                            */
    pm_pDeepLvl0Nodes = (xml_node<> *)m_pDeepLvl0Nodes;
    pDeepLvl1Nodes = pm_pDeepLvl0Nodes->first_node(TAG_NAME_LVL1_DESCRI);
//
            /* If found, look inside it for Attribute name ATTR_NAME_LVL1_1_NAM     */
    if (pDeepLvl1Nodes != NULL)
    {
        pNodeAttrib = pDeepLvl1Nodes->first_attribute(ATTR_NAME_LVL1_1_NAM);
        if (pNodeAttrib != 0)
        {
            firstFnd = true;
            pNodeAttrVal = pNodeAttrib->value();
            if (pNodeAttrVal)
                strncpy(pAtt1, pNodeAttrVal, lLen);
        }
//
            /* If found, look inside it for Attribute name ATTR_NAME_LVL1_2_VER         */
        pNodeAttrib = pDeepLvl1Nodes->first_attribute(ATTR_NAME_LVL1_2_VER);
        if (pNodeAttrib != 0)
        {
            secondFnd = true;
            pNodeAttrVal = pNodeAttrib->value();
            if (pNodeAttrVal)
                strncpy(pAtt2, pNodeAttrVal, lLen);
        }
    }
    if (!firstFnd || !secondFnd)
    {
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML get File Descrp", (MMC_CONNECT_HNDL)-1, -1, iRetCode, iErrId, 0);
    }
    else
        iErrId = MMC_OK;
//
    return(iErrId);
}
                    /* Print to stream using print function                         */
                    /* Can be call with string, NULL-ptr or 'PrintTreeToStdout()'   */
ELMO_INT32 MMCUserParams::PrintTreeToStdout(ELMO_PINT8 pcTitle)
// ============================================================
{
    ELMO_INT32         iErrId;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
    }
    else                /* Pointers to root node (level0)   */
    {
        printf("\n");
        if (pcTitle != NULL)
        {
            printf("%s", pcTitle);
        }
                        /* 'print' is RapidXml func.        */
                        /* 0 means default printing flags   */
        print(std::cout, g_doc, 0);
        printf("\n");
        iErrId = MMC_OK;
    }
//
    return(iErrId);
}
//
                                            /* BitWise Flag for Save                    */
                                            /* If Set 'Save' (write) ram tree into file */
                                            /* even such file already exist.            */
#define UPXML_SAVE_OVER_FLG     0x00010000
 
                                            /* BitWise Flag for Save                    */
                                            /* Flag bit (mask), for NOT Save (write) ram*/
                                            /* XML tree into file if file already exist */
#define UPXML_SAVE_NO_OVER_FLG  (~UPXML_SAVE_OVER_FLG)

ELMO_INT32 MMCUserParams::Save(ELMO_PINT8 cFileName, ELMO_PINT8 cFilePath) throw (CMMCException)
// =============================================================================================
{
    ELMO_INT32   rt_val;
    ELMO_UINT32  uiFlags;

    uiFlags = UPXML_SAVE_OVER_FLG;
    rt_val = Save_pr(cFileName, cFilePath, uiFlags);
//
    return (rt_val);
}
//
                    /* Private */
ELMO_INT32 MMCUserParams::Save_pr(ELMO_PINT8 cFileName, ELMO_PINT8 cFilePath, ELMO_UINT32 uiFlags) throw (CMMCException)
// =================================================================================================================
{
    ELMO_INT32   ReqWriteLen;
    ELMO_INT32   ActWriteLen;
    ELMO_INT32   fileNameLem;
    ELMO_INT8    fileName[MAX_XML_FILE_NAME_LEN];
                    /* File descriptor pointer */
    FILE *      stXmlFd;
    ELMO_INT32   iErrId = MMC_OK;
//
    std::string writeDataStr;
//
                        /* For insure pointers from XML tree to text pointing to valid location */
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()                                                                                       \
        return (iErrId);
    }
//
                        /* Construct the write file name */
    fileNameLem =  strlen(cFilePath);
    fileNameLem += strlen(cFileName);
    if ((fileNameLem-1) >= MAX_XML_FILE_NAME_LEN)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML Compose Write file name too long=%d (Max=%d) ", fileNameLem, MAX_XML_FILE_NAME_LEN);
            fflush(stdout); fflush(stderr);                                                                 \
        }
//
        iErrId = MMC_LIB_UPXML_LONG_NAME;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Write NAME too long ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
        return (iErrId);
    }
    sprintf(fileName, "%s%s", cFilePath, cFileName);
    if (m_uiSpeakDbgFlg >= SPK_LVL_INFO)
    {
        printf("\n UPXML write file name: <%s> ", fileName);
    }
//
    if ((uiFlags & UPXML_SAVE_OVER_FLG) == UPXML_SAVE_OVER_FLG)
    {
                    /* Over Write exist file or creat new one */
        stXmlFd = fopen(fileName, "w");
        if (NULL == stXmlFd)
        {
            iErrId = MMC_LIB_UPXML_SAVE_w_FAILD;
            if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
            {
                printf("\n *** UPXML Open/Creat file for Write fail (<%s> ReqWrFlag=0x%x errno=0x%x) ", fileName, uiFlags, errno);
                fflush(stdout); fflush(stderr);                                                                 \
            }
			CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Open/Creat file for write fail ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
            return (iErrId);
        }
    }
    else
    {
                    /* ERROR If file already exist */
        stXmlFd = fopen(fileName, "wx");
        if (NULL == stXmlFd)
        {
                    /* ERROR already exist */
            iErrId = MMC_LIB_UPXML_SAVE_wx_FAIL;
            if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
            {
                printf("\n *** UPXML Open Exist file for Write (<%s> ReqWrFlag=0x%x errno=0x%x) ", fileName, uiFlags, errno);
                fflush(stdout); fflush(stderr);                                                                 \
            }
			CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Open Exist file for write ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
            return (iErrId);
        }
    }
                    /* Print to string using output iterator */
    print(std::back_inserter(writeDataStr), g_doc, 0);
    ReqWriteLen = writeDataStr.length();
                    /* first to last char in string: writeDataStr.begin(); writeDataStr.end() */
    ActWriteLen = (ELMO_INT32)fwrite(writeDataStr.c_str(), 1, ReqWriteLen, stXmlFd);
//
    if (ActWriteLen != ReqWriteLen)
    {
        iErrId = MMC_LIB_UPXML_WRITE_FAILD;
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML Write Failed: (ActWriteLen=%d) != (ReqWriteLen=%d) errno=%d ", ActWriteLen, ReqWriteLen, errno);
            fflush(stdout); fflush(stderr);                                                                 \
        }
//
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Write faild ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
		return (iErrId);
	}
//
    iErrId = MMC_OK;
    fclose(stXmlFd);
//
    return (iErrId);
}
//
                    /* Delete Element of any type (if array all of its values. Not supporing Yet */
ELMO_INT32 MMCUserParams::RemoveElem(ELMO_PINT8 pLvl1Valu, ELMO_PINT8 pLvl2Valu, ELMO_PINT8 pLvl3Name)  throw (CMMCException)
// ======================================================================================================================
{
    return (MMC_LIB_UPXML_NOT_SUPPORTNG);
}
//
                    /* Write ONE parameter of type Double   */
ELMO_INT32 MMCUserParams::Write(ELMO_PINT8 pLvl1Valu, ELMO_PINT8 pLvl2Valu, ELMO_PINT8 pLvl3Name, ELMO_DOUBLE  dWrVal)  throw (CMMCException)
// =====================================================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   rt_val;
    ELMO_INT32   size;
    ELMO_PINT8   cPstr;
    ELMO_INT8    cValStr[MAX_CHARS_FOR_NUM*2];
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
                    /* Convert the element data to string */
                    /* put Null at end of string, ret the */
                    /* number of chars Not including Null */
    size = sprintf(cValStr, m_sprintfDoubleFormat, dWrVal);
    if (size <= 0)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML error format of ELMO_DOUBLE (size=%d) ", size);
            fflush(stdout); fflush(stderr);                                                                 \
        }
        return (MMC_LIB_UPXML_NUM_FORMAT);
    }
//
//  cPstr = g_doc.allocate_string(cValStr, size);
    cPstr = g_doc.allocate_string(cValStr);
    rt_val = UpdDomTree (pLvl1Valu, pLvl2Valu, pLvl3Name, cPstr, size);
//
    return (rt_val);
}
//
                    /* Write ONE parameter of type Long     */
ELMO_INT32 MMCUserParams::Write(ELMO_PINT8 pLvl1Valu, ELMO_PINT8   pLvl2Valu, ELMO_PINT8 pLvl3Name, ELMO_LINT32  lWrVal) throw (CMMCException)
// ======================================================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   rt_val;
    ELMO_INT32   size;
    ELMO_PINT8   cPstr;
    ELMO_INT8    cValStr[MAX_CHARS_FOR_NUM*2];
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
                    /* Convert the element data to string */
    size = sprintf(cValStr, "%d", (ELMO_INT32)lWrVal);
    if (size <= 0)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML error format of ELMO_LINT32 (size=%d) ", size);
            fflush(stdout); fflush(stderr);                                                                 \
        }
        return (MMC_LIB_UPXML_NUM_FORMAT);
    }
//
    cPstr = g_doc.allocate_string(cValStr, size);
    rt_val = UpdDomTree (pLvl1Valu, pLvl2Valu, pLvl3Name, cPstr, size);
//
    return (rt_val);
}
//
                    /* Write ONE parameter of type bool.    */
ELMO_INT32 MMCUserParams::Write(ELMO_PINT8 pLvl1Valu, ELMO_PINT8 pLvl2Valu, ELMO_PINT8 pLvl3Name, ELMO_BOOL bWrVal)  throw (CMMCException)
// ==================================================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   rt_val;
    ELMO_INT32   size;
    ELMO_PINT8   cPstr;
    ELMO_INT8    cValStr[MAX_CHARS_FOR_NUM*2];
//
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
                    /* Convert the element data to string */
    if (bWrVal == true)
    {
        size = sprintf(cValStr, "%s", TRUE_FILE_STRING);
    }
    else
    {
        size = sprintf(cValStr, "%s", FALSE_FILE_STRING);
    }
//
    if (size <= 0)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_ERROR)
        {
            printf("\n *** UPXML error format of boolean (size=%d) ", size);
            fflush(stdout); fflush(stderr);                                                                 \
        }
        return (MMC_LIB_UPXML_NUM_FORMAT);
    }
//
//  cPstr = g_doc.allocate_string(cValStr, size);
    cPstr = g_doc.allocate_string(cValStr);
    rt_val = UpdDomTree (pLvl1Valu, pLvl2Valu, pLvl3Name, cPstr, size);
//
    return (rt_val);
}
//
                    /* Write ONE parameter of type string, should be null terminated and the lLen including it  */
ELMO_INT32 MMCUserParams::Write(ELMO_PINT8  pLvl1Valu, ELMO_PINT8 pLvl2Valu, ELMO_PINT8 pLvl3Name, ELMO_PINT8 pStr, ELMO_LINT32 lLen) throw (CMMCException)
// ==================================================================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   rt_val;
    ELMO_PINT8   cPstr;
//
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
//
    cPstr = g_doc.allocate_string(pStr, (ELMO_INT32)lLen);
    rt_val = UpdDomTree (pLvl1Valu, pLvl2Valu, pLvl3Name, cPstr, (ELMO_INT32)lLen);
//
    return (rt_val);
}
//
                            /* Functions for Write Array of parameters values *\
                            \* ============================================== */
//
                    /* Write Array of parameter of type Double */
ELMO_INT32 MMCUserParams::WriteArr(  ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu,  ELMO_PINT8 pLvl3Name,
                                    ELMO_DOUBLE  dWrVal[],   ELMO_UINT32  iActWrElm)   throw (CMMCException)
// =========================================================================================================
{
    ELMO_INT32  iErrId;
    ELMO_INT32  rt_val;
    ELMO_INT32  size;
    ELMO_PINT8  cPstr;
    ELMO_UINT32 ind;
    ELMO_PINT8  cValStr;
//
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
//
    if (iActWrElm > MAX_WRITE_ARR_SIZE)
    {
        iErrId = MMC_LIB_UPXML_ILLEGAL_ARRYSIZE;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Illegal larray size ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
        return (iErrId);
    }
//
    size = (MAX_CHARS_FOR_NUM*2)*iActWrElm + 4;
    cValStr = new ELMO_INT8 [size];
//
    size = rt_val = 0;
                    /* Convert the element data to string */
    if (iActWrElm == 0)
    {
        cValStr[0] = cValStr[1] = 0;
    }
    else if (iActWrElm == 1)
    {
        size += sprintf(&(cValStr[size]), m_sprintfDoubleFormat, dWrVal[0]);
    }
    else
    {
        for (ind=0; ind < iActWrElm-1; ind++)
        {
                    /* Format including sperator at end */
            size += sprintf(&(cValStr[size]), m_sprintfDoubleFormatAr, dWrVal[ind]);
        }
                    /* Format without including sperator at end */
        size += sprintf(&(cValStr[size]), m_sprintfDoubleFormat, dWrVal[iActWrElm-1]);
    }
//
//
//  cPstr = g_doc.allocate_string(cValStr, size);
    cPstr = g_doc.allocate_string(cValStr);
    rt_val = UpdDomTree (pLvl1Valu, pLvl2Valu, pLvl3Name, cPstr, size);
//
    delete [] cValStr;
    return (rt_val);
}
//
                    /* Write Array of parameter of type Long */
ELMO_INT32 MMCUserParams::WriteArr( ELMO_PINT8   pLvl1Valu,  ELMO_PINT8   pLvl2Valu, ELMO_PINT8 pLvl3Name,
                                    ELMO_LINT32  lWrVal[],   ELMO_UINT32  iActWrElm)   throw (CMMCException)
// =========================================================================================================
{
    ELMO_INT32   iErrId;
    ELMO_INT32   rt_val;
    ELMO_INT32   size;
    ELMO_PINT8   cPstr;
    ELMO_UINT32  ind;
    ELMO_PINT8   cValStr;
//
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()
        return (iErrId);
    }
//
    if (iActWrElm > MAX_WRITE_ARR_SIZE)
    {
        iErrId = MMC_LIB_UPXML_ILLEGAL_ARRYSIZE;
		CMMCPPGlobal::Instance()->MMCPPThrow("UPXML Illegal darray size ", (MMC_CONNECT_HNDL)-1, -1, MMC_LIB_UPXML_ERROR, iErrId, 0);
        return (iErrId);
    }
//
    size = (MAX_CHARS_FOR_NUM*2)*iActWrElm + 4;
    cValStr = new ELMO_INT8 [size];
//
    size = rt_val = 0;
                    /* Convert the element data to string */
    if (iActWrElm == 0)
    {
        cValStr[0] = cValStr[1] = 0;
    }
    else if (iActWrElm == 1)
    {
        size += sprintf(&(cValStr[size]), "%d", (ELMO_INT32)lWrVal[0]);
    }
    else
    {
        for (ind=0; ind < iActWrElm-1; ind++)
        {
                    /* Format including sperator at end */
            size += sprintf(&(cValStr[size]), "%d,", (ELMO_INT32)lWrVal[ind]);
        }
                    /* Format without including sperator at end */
        size += sprintf(&(cValStr[size]), "%d", (ELMO_INT32)lWrVal[iActWrElm-1]);
    }
//
//  cPstr = g_doc.allocate_string(cValStr, size);
    cPstr = g_doc.allocate_string(cValStr);
    rt_val = UpdDomTree (pLvl1Valu, pLvl2Valu, pLvl3Name, cPstr, size);
//
    delete [] cValStr;
    return (rt_val);
}
//
                            /* Write XML file identifier *\
                            \* ========================= */
                    /* Create new XML tree in Ram. E.g when no file nore open...    */
                    /* with this function the user start build XML tree from scrach.*/
                    /* Shuld be the first write in sequnce of build new TREE.       */
                    /*                                                              */
                    /* Write (null terminated string) to XML file root (xsi ID      */
                    /* values) pAtt1 & xsi Location (pAtt2),                        */
                    /* Eg:                                                          */
                    /*      pAtt1 = "http://www.w3.org/2001/XMLSchema-instance"     */
                    /*      pAtt2 = "proposed.xsd"                                  */
                    /* Write XML file lines:                                        */
                    /*  <root xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" */
                    /*          xsi:noNamespaceSchemaLocation="proposed.xsd">       */
    ELMO_INT32 MMCUserParams::PutXmlFileRoot(ELMO_PINT8 pAtt1, ELMO_PINT8 pAtt2)  throw (CMMCException)
// ====================================================================================================
{
    ELMO_INT32          iErrId;
    xml_node<>*         decl;
    xml_node<>*         root;
    xml_attribute<>*    ver;
    xml_attribute<>*    encoding;
    xml_attribute<>*    attr;
//
    if (m_pDeepLvl0Nodes != NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()                                                                                       \
        return (iErrId);
    }
                    /* xml declaration */
    decl     = g_doc.allocate_node(node_declaration);
    ver      = g_doc.allocate_attribute(g_doc.allocate_string(UPXML_ID_VER_NAME),   g_doc.allocate_string(UPXML_ID_VER));
    encoding = g_doc.allocate_attribute(g_doc.allocate_string(UPXML_ID_ENCOD_NAME), g_doc.allocate_string(UPXML_ID_ENCOD));
//
    decl->append_attribute(ver);
    decl->append_attribute(encoding);
    g_doc.append_node(decl);
//
                    /* root node        */
    root = g_doc.allocate_node(node_element, UPXML_ROOT_NAME);
//
    attr = g_doc.allocate_attribute(TAG_ATTR_NAME_LVL0_1, g_doc.allocate_string(pAtt1));
    root->append_attribute(attr);
//
    attr = g_doc.allocate_attribute(TAG_ATTR_NAME_LVL0_2, g_doc.allocate_string(pAtt2));
    root->append_attribute(attr);
//
    g_doc.append_node(root);
//
                            /* Find our root node */
    m_pDeepLvl0Nodes  = (ELMO_PVOID)(g_doc.first_node(UPXML_ROOT_NAME));
//
    return (MMC_OK);
}
//
                    /* (pAtt1) and XML file ver (pAtt2).                            */
ELMO_INT32 MMCUserParams::PutXmlFileDescrp(ELMO_PINT8 pAtt1, ELMO_PINT8  pAtt2) throw (CMMCException)
// ==================================================================================================
{
    ELMO_INT32       iErrId;
    xml_node<>      * ppDescrpNodes;
    xml_attribute<> *attr;
//
    if (m_pDeepLvl0Nodes == NULL)
    {
        REP_READ_GET_SEQUENCE_ERROR()                                                                                       \
        return (iErrId);
    }
//
    ppDescrpNodes = g_doc.allocate_node(node_element, TAG_NAME_LVL1_DESCRI);
    attr          = g_doc.allocate_attribute(ATTRIBUTE_LVL1_NAME, g_doc.allocate_string(pAtt1));
    ppDescrpNodes->append_attribute(attr);

    attr          = g_doc.allocate_attribute(ATTR_NAME_LVL1_2_VER, g_doc.allocate_string(pAtt2));
    ppDescrpNodes->append_attribute(attr);

    ((xml_node<> *)m_pDeepLvl0Nodes)->append_node(ppDescrpNodes);
//
    return (MMC_OK);
}
//
//
                    /* cPstr a Null terminated string should be add to tree under       */
                    /* pLvl1Valu,   pLvl2Valu,   pLvl3Name, replace the value there.    */
                    /* if exist, create new node if missing.                            */
ELMO_INT32 MMCUserParams::UpdDomTree(ELMO_PINT8 pLvl1Valu, ELMO_PINT8  pLvl2Valu, ELMO_PINT8 pLvl3Name,
                                     ELMO_PINT8 cPstr,     ELMO_INT32  cPstrSize)
// ====================================================================================================
{
    ELMO_PINT8      pBufPrt;
//
    xml_node<>     * ppLvl1Nodes,
                   * ppLvl2Nodes,
                   * ppLvl3Nodes;
    xml_attribute<>* attr;
//
    ELMO_PVOID		ppLvl1NodesPV,
    				ppLvl2NodesPV,
    				ppLvl3NodesPV;
//
//...    ELMO_INT32      get_rt_val;
    ELMO_INT32      rt_val = MMC_OK;
//
//...    get_rt_val = getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt,
    getTagValue(pLvl1Valu, pLvl2Valu, pLvl3Name, &pBufPrt,
                           &ppLvl1NodesPV, &ppLvl2NodesPV, &ppLvl3NodesPV);
    //				/* Eliminate compiler 'strict-aliasing rules' warning... */
	ppLvl1Nodes = (rapidxml::xml_node<>*)ppLvl1NodesPV;
	ppLvl2Nodes = (rapidxml::xml_node<>*)ppLvl2NodesPV;
	ppLvl3Nodes = (rapidxml::xml_node<>*)ppLvl3NodesPV;
//
    if (ppLvl1Nodes == NULL)
    {
                    /* Add Node on exist tree, name  TAG_NAME_LVL1 attribute    */
                    /* name ATTRIBUTE_LVL1_NAME, value for attr. pLvl1Valu      */
        ppLvl1Nodes = g_doc.allocate_node(node_element, TAG_NAME_LVL1);
        ((xml_node<> *)m_pDeepLvl0Nodes)->append_node(ppLvl1Nodes);
        attr        = g_doc.allocate_attribute(ATTRIBUTE_LVL1_NAME, g_doc.allocate_string(pLvl1Valu));
        ppLvl1Nodes->append_attribute(attr);
//
        if (rt_val == MMC_OK)
        {
            rt_val = MMC_LIB_UPXML_UPDATE_L1_ELEM;
        }
        ppLvl2Nodes = NULL;
        ppLvl3Nodes = NULL;
    }
//
//
    if (ppLvl2Nodes == NULL)
    {
                    /* Add Node on exist tree, name  TAG_NAME_LVL2 attribute    */
                    /* name ATTRIBUTE_LVL2_NAME, value for attr. pLvl2Valu      */
        ppLvl2Nodes = g_doc.allocate_node(node_element, TAG_NAME_LVL2);
        ppLvl1Nodes->append_node(ppLvl2Nodes);
        attr        = g_doc.allocate_attribute(ATTRIBUTE_LVL2_NAME, g_doc.allocate_string(pLvl2Valu));
        ppLvl2Nodes->append_attribute(attr);
//
        if (rt_val == MMC_OK)
        {
            rt_val = MMC_LIB_UPXML_UPDATE_L2_ELEM;
        }
        ppLvl3Nodes = NULL;
    }
//
//
    if (ppLvl3Nodes == NULL)
    {
                    /* Add Node on exist tree, name  pLvl3Name No attribute */
        ppLvl3Nodes = g_doc.allocate_node(node_element, g_doc.allocate_string(pLvl3Name));
        ppLvl2Nodes->append_node(ppLvl3Nodes);

        if (rt_val == MMC_OK)
        {
            rt_val = MMC_LIB_UPXML_UPDATE_L3_ELEM;
        }
    }
//
                    /* Replace the value on tree pointed by ppLvl3Nodes by the  */
                    /* requsted string. Size not including the Null             */
     ppLvl3Nodes->value(cPstr, cPstrSize);
//
//
    return(rt_val);
}
//
                /* Related to given node pointer, pass on all of node attribute and node value. */
void MMCUserParams::handleNodes(ELMO_UINT32 uiDeepLvl, ELMO_PVOID  pNodeV) //  MMCUserParams::MMCUserParams::handleNodes 
// =====================================================================================================================
{
    ELMO_UINT32  iIdx;
    ELMO_PINT8   pName;
    xml_node<>* pNode;
//
    pNode = (xml_node<> *)pNodeV;
    pName = pNode->name();
    if(pName != NULL)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_DBG)
        {
            for(iIdx=0; iIdx < uiDeepLvl; iIdx++)
                printf("\t");
        }
        //
        handleNodeAttribute(uiDeepLvl, pNode);
        //
        handleNodeValue    (uiDeepLvl, pNode);
        //
        if (m_uiSpeakDbgFlg >= SPK_LVL_DBG)
            cout << endl;
    }
}
//
                            /* Handle attribute of given node */
void    MMCUserParams::handleNodeAttribute(ELMO_UINT32 uiDeepLvl, ELMO_PVOID  pNodeV)
// ===============================================================================
{
    ELMO_PINT8           pAttrVal;
    xml_attribute<> *   pNodeAttr;
    xml_node<> *        pNode;
//
    pNode = (xml_node<> *)pNodeV;
    pNodeAttr = pNode->first_attribute();
    while(pNodeAttr)
    {
        pAttrVal = pNodeAttr->value();
        if (m_uiSpeakDbgFlg >= SPK_LVL_DBG)
            printf(" %s", pAttrVal);
//
        pNodeAttr = pNodeAttr->next_attribute(NULL);
    }
}
//
                            /* Handle the data (Text) of given node */
void    MMCUserParams::handleNodeValue(ELMO_UINT32 uiDeepLvl, ELMO_PVOID  pNodeV)
// ============================================================================
{
    ELMO_PINT8   pAttribVal;
    xml_node<>* pNode;
//
//
    pNode = (xml_node<> *)pNodeV;
    pAttribVal = pNode->value();
//
    if (pAttribVal != NULL)
    {
        if (m_uiSpeakDbgFlg >= SPK_LVL_DBG)
        {
            printf(" %s ", pAttribVal);
        }
    }
}
//
                    /* Format for write double / float nuber    */
                    /* E.g: iFieldSize=8 iFractionSize=3 =>     */
                    /* format be: "%8.3f"                       */
                    /* Limitation: iFractionSize+1 < iFieldSize */
                    /*             iFractionSize + iFieldSize < */
                    /*                      MAX_CHARS_FOR_NUM   */
                    /*   other limits are according to Compiler */
                    /*      sprintf.                            */
ELMO_INT32 MMCUserParams::PutFloatFormat(ELMO_UINT32 iFieldSize, ELMO_UINT32 iFractionSize)
// =====================================================================================
{
    if ((iFractionSize+1 >= iFieldSize) || (iFractionSize+iFieldSize > MAX_CHARS_FOR_NUM))
    {
        return (MMC_LIB_UPXML_ILLEGAL_FLOAT_FORMAT);
    }

    sprintf(m_sprintfDoubleFormat  , "%%%d.%df" , iFieldSize, iFractionSize);
    sprintf(m_sprintfDoubleFormatAr, "%%%d.%df,", iFieldSize, iFractionSize);

    return 0;
}

#ifdef PROAUT_CHANGES
	//restore compiler switches
	#pragma GCC diagnostic pop
#endif

