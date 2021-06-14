/*******************************************************************************
** File: example_app.h
**
** Purpose:
**   This file is main hdr file for the EXAMPLE application.
**
**
*******************************************************************************/

#ifndef _example_app_h_
#define _example_app_h_

/*
**   Include Files:
*/


#include "example_tbldefs.h"


/*
** Event message ID's.
*/
#define EXAMPLE_INIT_INF_EID      1    /* start up message "informational" */

#define EXAMPLE_NOOP_INF_EID      2    /* processed command "informational" */
#define EXAMPLE_RESET_INF_EID     3
#define EXAMPLE_PROCESS_INF_EID   4
  
#define EXAMPLE_MID_ERR_EID       5    /* invalid command packet "error" */
#define EXAMPLE_CC1_ERR_EID       6
#define EXAMPLE_LEN_ERR_EID       7
#define EXAMPLE_PIPE_ERR_EID      8

#define EXAMPLE_EVT_COUNT         8    /* count of event message ID's */

#define EXAMPLE_RUNAWAY_INF_EID   9

/*
** Command packet command codes
*/
#define EXAMPLE_NOOP_CC           0    /* no-op command */
#define EXAMPLE_RESET_CC          1    /* reset counters */
#define EXAMPLE_PROCESS_CC        2    /* Perform Routine Processing */
#define EXAMPLE_RUNAWAY_CC        3    /* Simulate a runaway task */

/*
** Table defines
*/
#define EXAMPLE_NUM_TABLES        2    /* Number of Tables */

#define EXAMPLE_FIRST_TBL_FILE  "/cf/example_tbl_1.tbl"
#define EXAMPLE_SECOND_TBL_FILE "/cf/example_tbl_2.tbl"

#define EXAMPLE_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE    1  
#define EXAMPLE_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE   -1

#define EXAMPLE_TBL_ELEMENT_1_MAX  10  
#define EXAMPLE_TBL_ELEMENT_3_MAX  30  

/*
** Software Bus defines
*/
#define EXAMPLE_PIPE_DEPTH        12   /* Depth of the Command Pipe for Application */
#define EXAMPLE_LIMIT_HK          2    /* Limit of HouseKeeping Requests on Pipe for Application */
#define EXAMPLE_LIMIT_CMD         4    /* Limit of Commands on pipe for Application */

/*
** EXAMPLE Critical Data Store defines
*/
#define EXAMPLE_CDS_NAME            "EXAMPLECDS"


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

} EXAMPLE_CdsDataType_t;


/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} EXAMPLE_NoArgsCmd_t;

/*************************************************************************/

/*
** Type definition (EXAMPLE housekeeping)
*/
typedef struct
{
  uint8                 TlmHeader[CFE_SB_TLM_HDR_SIZE];

  /*
  ** Command interface counters
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

} OS_PACK EXAMPLE_HkPacket_t;

/*************************************************************************/

/*
** Type definition (EXAMPLE app global data)
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
  EXAMPLE_HkPacket_t         HkPacket;

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
  EXAMPLE_CdsDataType_t      WorkingCriticalData; /* Define a copy of critical data that can be used during Application execution */
  CFE_ES_CDSHandle_t    CDSHandle;           /* Handle to CDS memory block */

  /*
  ** Initialization data (not reported in housekeeping)
  */
  char                  PipeName[16];
  uint16                PipeDepth;

  uint8                 LimitHK;
  uint8                 LimitCmd;

  CFE_EVS_BinFilter_t   EventFilters[EXAMPLE_EVT_COUNT];
  CFE_TBL_Handle_t      TblHandles[EXAMPLE_NUM_TABLES];

} EXAMPLE_AppData_t;

/*************************************************************************/

/*
** Local function prototypes
**
** Note: Except for the entry point (EXAMPLE_AppMain), these
**       functions are not called from any other source module.
*/
void    EXAMPLE_AppMain(void);
int32   EXAMPLE_AppInit(void);
void    EXAMPLE_AppPipe(CFE_SB_MsgPtr_t msg);

void    EXAMPLE_HousekeepingCmd(CFE_SB_MsgPtr_t msg);

void    EXAMPLE_NoopCmd(CFE_SB_MsgPtr_t msg);
void    EXAMPLE_ResetCmd(CFE_SB_MsgPtr_t msg);
void    EXAMPLE_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg);

boolean EXAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

int32   EXAMPLE_FirstTblValidationFunc(void *TblData);
int32   EXAMPLE_SecondTblValidationFunc(void *TblData);


#endif /* _example_app_h_ */

/************************/
/*  End of File Comment */
/************************/



