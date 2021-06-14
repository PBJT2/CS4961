/*******************************************************************************
** File: sbei_app.h
**
** Purpose:
**   This file is main hdr file for the SBEI application.
**
**
*******************************************************************************/

#ifndef _sbei_app_h_
#define _sbei_app_h_

/*
**   Include Files:
*/

#include "cfe.h"
#include "sbei_tbldefs.h"


/*
** Event message ID's.
*/
#define SBEI_INIT_INF_EID      1    /* start up message "informational" */

#define SBEI_NOOP_INF_EID      2    /* processed command "informational" */
#define SBEI_RESET_INF_EID     3
#define SBEI_PROCESS_INF_EID   4
  
#define SBEI_MID_ERR_EID       5    /* invalid command packet "error" */
#define SBEI_CC1_ERR_EID       6
#define SBEI_LEN_ERR_EID       7
#define SBEI_PIPE_ERR_EID      8

#define SBEI_EVT_COUNT         8    /* count of event message ID's */

/*
** Command packet command codes
*/
#define SBEI_NOOP_CC           0    /* no-op command */
#define SBEI_RESET_CC          1    /* reset counters */
#define SBEI_PROCESS_CC        2    /* Perform Routine Processing */

/*
** Table defines
*/
#define SBEI_NUM_TABLES        2    /* Number of Tables */

#define SBEI_FIRST_TBL_FILE  "/cf/sbei_tbl_1.tbl"
#define SBEI_SECOND_TBL_FILE "/cf/sbei_tbl_2.tbl"

#define SBEI_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE    1  
#define SBEI_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE   -1

#define SBEI_TBL_ELEMENT_1_MAX  10  
#define SBEI_TBL_ELEMENT_3_MAX  30  

/*
** Software Bus defines
*/
#define SBEI_PIPE_DEPTH        12   /* Depth of the Command Pipe for Application */
#define SBEI_LIMIT_HK          2    /* Limit of HouseKeeping Requests on Pipe for Application */
#define SBEI_LIMIT_CMD         4    /* Limit of Commands on pipe for Application */

/*
** SBEI Critical Data Store defines
*/
#define SBEI_CDS_NAME            "SBEICDS"


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

} SBEI_CdsDataType_t;


/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} SBEI_NoArgsCmd_t;

/*************************************************************************/

/*
** Type definition (SBEI housekeeping)
*/
typedef struct
{
  uint8                 TlmHeader[CFE_SB_TLM_HDR_SIZE];

  /*
  ** Command interface counters
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

} OS_PACK SBEI_HkPacket_t;

/*************************************************************************/

/*
** Type definition (SBEI app global data)
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
  SBEI_HkPacket_t         HkPacket;

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
  SBEI_CdsDataType_t      WorkingCriticalData; /* Define a copy of critical data that can be used during Application execution */
  CFE_ES_CDSHandle_t    CDSHandle;           /* Handle to CDS memory block */

  /*
  ** Initialization data (not reported in housekeeping)
  */
  char                  PipeName[16];
  uint16                PipeDepth;

  uint8                 LimitHK;
  uint8                 LimitCmd;

  CFE_EVS_BinFilter_t   EventFilters[SBEI_EVT_COUNT];
  CFE_TBL_Handle_t      TblHandles[SBEI_NUM_TABLES];

} SBEI_AppData_t;

/*************************************************************************/

/*
** Local function prototypes
**
** Note: Except for the entry point (SBEI_AppMain), these
**       functions are not called from any other source module.
*/
void    SBEI_AppMain(void);
int32   SBEI_AppInit(void);
void    SBEI_AppPipe(CFE_SB_MsgPtr_t msg);

void    SBEI_HousekeepingCmd(CFE_SB_MsgPtr_t msg);

void    SBEI_NoopCmd(CFE_SB_MsgPtr_t msg);
void    SBEI_ResetCmd(CFE_SB_MsgPtr_t msg);
void    SBEI_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg);

boolean SBEI_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

int32   SBEI_FirstTblValidationFunc(void *TblData);
int32   SBEI_SecondTblValidationFunc(void *TblData);


#endif /* _sbei_app_h_ */

/************************/
/*  End of File Comment */
/************************/



