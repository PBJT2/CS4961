/*******************************************************************************
** File: dosd_app.h
**
** Purpose:
**   This file is main hdr file for the DOSD application.
**
**
*******************************************************************************/

#ifndef _dosd_app_h_
#define _dosd_app_h_

/*
**   Include Files:
*/

#include "cfe.h"
#include "dosd_tbldefs.h"
#include "dosd_version.h"


/*
** Event message ID's.
*/
#define DOSD_INIT_INF_EID      1    /* start up message "informational" */

#define DOSD_NOOP_INF_EID      2    /* processed command "informational" */
#define DOSD_RESET_INF_EID     3
#define DOSD_PROCESS_INF_EID   4
  
#define DOSD_MID_ERR_EID       5    /* invalid command packet "error" */
#define DOSD_CC1_ERR_EID       6
#define DOSD_LEN_ERR_EID       7
#define DOSD_PIPE_ERR_EID      8

#define DOSD_DETECT_INF_EID   9

#define DOSD_EVT_COUNT         9    /* count of event message ID's */




/*
** Command packet command codes
*/
#define DOSD_NOOP_CC           0    /* no-op command */
#define DOSD_RESET_CC          1    /* reset counters */
#define DOSD_PROCESS_CC        2    /* Perform Routine Processing */
#define DOSD_DETECT_CC        3
/*
** Table defines
*/
#define DOSD_NUM_TABLES        2    /* Number of Tables */

#define DOSD_FIRST_TBL_FILE  "/cf/dosd_tbl_1.tbl"
#define DOSD_SECOND_TBL_FILE "/cf/dosd_tbl_2.tbl"

#define DOSD_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE    1  
#define DOSD_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE   -1

#define DOSD_TBL_ELEMENT_1_MAX  10  
#define DOSD_TBL_ELEMENT_3_MAX  30  

/*
** Software Bus defines
*/
#define DOSD_PIPE_DEPTH        12   /* Depth of the Command Pipe for Application */
#define DOSD_LIMIT_HK          2    /* Limit of HouseKeeping Requests on Pipe for Application */
#define DOSD_LIMIT_CMD         4    /* Limit of Commands on pipe for Application */

/*
** DOSD Critical Data Store defines
*/
#define DOSD_CDS_NAME            "DOSDCDS"


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

} DOSD_CdsDataType_t;


/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} DOSD_NoArgsCmd_t;

/*************************************************************************/

/*
** Type definition (generic "detection" command)
*/
typedef struct
{
  uint8                 CmdHeader[CFE_SB_CMD_HDR_SIZE];

  uint16  		ConnectionState;                        //to disconnect or stay connected with TFTP

} DOSD_DetectCmd_t;

/*************************************************************************/

/*
** Type definition (DOSD housekeeping)
*/
typedef struct
{
  uint8                 TlmHeader[CFE_SB_TLM_HDR_SIZE];

  /*
  ** Command interface counters
  */
  uint8                 CmdCounter;
  uint8                 ErrCounter;

} OS_PACK DOSD_HkPacket_t;

/*************************************************************************/

/*
** Type definition (DOSD app global data)
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
  DOSD_HkPacket_t         HkPacket;

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
  DOSD_CdsDataType_t      WorkingCriticalData; /* Define a copy of critical data that can be used during Application execution */
  CFE_ES_CDSHandle_t    CDSHandle;           /* Handle to CDS memory block */

  /*
  ** Initialization data (not reported in housekeeping)
  */
  char                  PipeName[16];
  uint16                PipeDepth;

  uint8                 LimitHK;
  uint8                 LimitCmd;

  CFE_EVS_BinFilter_t   EventFilters[DOSD_EVT_COUNT];
  CFE_TBL_Handle_t      TblHandles[DOSD_NUM_TABLES];

  uint8   ConnectionState;                     /**< \TFTP enable/disable state */

} DOSD_AppData_t;

/*************************************************************************/

/*
** Local function prototypes
**
** Note: Except for the entry point (DOSD_AppMain), these
**       functions are not called from any other source module.
*/
void    DOSD_AppMain(void);
int32   DOSD_AppInit(void);
void    DOSD_AppPipe(CFE_SB_MsgPtr_t msg);

void    DOSD_HousekeepingCmd(CFE_SB_MsgPtr_t msg);

void    DOSD_DetectCmd(CFE_SB_MsgPtr_t msg);
void    DOSD_NoopCmd(CFE_SB_MsgPtr_t msg);
void    DOSD_ResetCmd(CFE_SB_MsgPtr_t msg);
void    DOSD_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg);

boolean DOSD_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

int32   DOSD_FirstTblValidationFunc(void *TblData);
int32   DOSD_SecondTblValidationFunc(void *TblData);


#endif /* _dosd_app_h_ */

/************************/
/*  End of File Comment */
/************************/



