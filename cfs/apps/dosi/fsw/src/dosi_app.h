/*******************************************************************************
** File: dosi_app.h
**
** Purpose:
**   This file is main hdr file for the DOSI application.
**
**
*******************************************************************************/

#ifndef _dosi_app_h_
#define _dosi_app_h_

/*
**   Include Files:
*/

#include "cfe.h"
#include "dosi_tbldefs.h"
#include "dosi_version.h"


/*
** Event message ID's.
*/
#define DOSI_INIT_INF_EID      1    /* start up message "informational" */

#define DOSI_NOOP_INF_EID      2    /* processed command "informational" */
#define DOSI_RESET_INF_EID     3
#define DOSI_PROCESS_INF_EID   4
  
#define DOSI_MID_ERR_EID       5    /* invalid command packet "error" */
#define DOSI_CC1_ERR_EID       6
#define DOSI_LEN_ERR_EID       7
#define DOSI_PIPE_ERR_EID      8
#define DOSI_INJECT_INF_EID    9

#define DOSI_EVT_COUNT         9    /* count of event message ID's */

/*
** Command packet command codes
*/
#define DOSI_NOOP_CC           0    /* no-op command */
#define DOSI_RESET_CC          1    /* reset counters */
#define DOSI_PROCESS_CC        2    /* Perform Routine Processing */
#define DOSI_INJECT_CC         3 

/*
** Table defines
*/
#define DOSI_NUM_TABLES        2    /* Number of Tables */

#define DOSI_FIRST_TBL_FILE  "/cf/dosi_tbl_1.tbl"
#define DOSI_SECOND_TBL_FILE "/cf/dosi_tbl_2.tbl"

#define DOSI_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE    1  
#define DOSI_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE   -1

#define DOSI_TBL_ELEMENT_1_MAX  10  
#define DOSI_TBL_ELEMENT_3_MAX  30  

/*
** Software Bus defines
*/
#define DOSI_PIPE_DEPTH        12   /* Depth of the Command Pipe for Application */
#define DOSI_LIMIT_HK          2    /* Limit of HouseKeeping Requests on Pipe for Application */
#define DOSI_LIMIT_CMD         4    /* Limit of Commands on pipe for Application */

/*
** DOSI Critical Data Store defines
*/
#define DOSI_CDS_NAME            "DOSICDS"


/*
** Type definition (Critical Data Store data)
*/
typedef struct
{
  uint32  DataPtOne;    /* Values stored in my CDS */
  uint32  DataPtTwo;
  uint32  DataPtThree;
  uint32  DataPtFour;
  uint32  DataPtFive;

} DOSI_CdsDataType_t;


/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} DOSI_NoArgsCmd_t;

/*************************************************************************/

/*
** Type definition (Inject command)
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} DOSI_InjectCmd_t;

/*************************************************************************/


/*
** Type definition (DOSI housekeeping)
*/
typedef struct
{
  uint8                 TlmHeader[CFE_SB_TLM_HDR_SIZE];

  /*
  ** Command interface counters
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

} OS_PACK DOSI_HkPacket_t;

/*************************************************************************/

/*
** Type definition (DOSI app global data)
*/
typedef struct
{
  /*
  ** Command interface counters.
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

  /*
  ** Housekeeping telemetry packet
  */
  DOSI_HkPacket_t         HkPacket;

  /*
  ** Operational data (not reported in housekeeping).
  */
  CFE_SB_MsgPtr_t       MsgPtr;
  CFE_SB_PipeId_t       CmdPipe;
  
  /*
  ** RunStatus variable used in the main processing loop
  */
  uint32                RunStatus;

  /*
  ** Critical Data store variables
  */
  DOSI_CdsDataType_t      WorkingCriticalData; /* Define a copy of critical data that can be used during Application execution */
  CFE_ES_CDSHandle_t    CDSHandle;           /* Handle to CDS memory block */

  /*
  ** Initialization data (not reported in housekeeping)
  */
  char                  PipeName[16];
  uint16                PipeDepth;

  uint8                 LimitHK;
  uint8                 LimitCmd;

  CFE_EVS_BinFilter_t   EventFilters[DOSI_EVT_COUNT];
  CFE_TBL_Handle_t      TblHandles[DOSI_NUM_TABLES];

} DOSI_AppData_t;

/*************************************************************************/

/*
** Local function prototypes
**
** Note: Except for the entry point (DOSI_AppMain), these
**       functions are not called from any other source module.
*/
void    DOSI_AppMain(void);
int32   DOSI_AppInit(void);
void    DOSI_AppPipe(CFE_SB_MsgPtr_t msg);

void    DOSI_HousekeepingCmd(CFE_SB_MsgPtr_t msg);

void    DOSI_InjectCmd(CFE_SB_MsgPtr_t msg);
void    DOSI_NoopCmd(CFE_SB_MsgPtr_t msg);
void    DOSI_ResetCmd(CFE_SB_MsgPtr_t msg);
void    DOSI_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg);

boolean DOSI_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

int32   DOSI_FirstTblValidationFunc(void *TblData);
int32   DOSI_SecondTblValidationFunc(void *TblData);


#endif /* _dosi_app_h_ */

/************************/
/*  End of File Comment */
/************************/



