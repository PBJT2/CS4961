/*******************************************************************************
** File: sbei_app.c
**
** Purpose:
**   This file contains the source code for the Sbei App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "sbei_app.h"
#include "sbei_app_perfids.h"
#include "sbei_app_msgids.h"


/*
** System Header files
*/
#include <string.h>

/*
** SBEI global data
*/
SBEI_AppData_t SBEI_AppData;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_AppMain() -- Application entry point and main process loop */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SBEI_AppMain(void)
{
    int32 Status;
    
    /*
    ** Register the Application with Executive Services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SBEI_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Intialization fails, set the RunStatus to CFE_ES_APP_ERROR
    ** and the App will not enter the RunLoop.
    */
    Status = SBEI_AppInit();
    if ( Status != CFE_SUCCESS )
    {
        SBEI_AppData.RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** Application Main Loop. Call CFE_ES_RunLoop to check for changes
    ** in the Application's status. If there is a request to kill this
    ** App, it will be passed in through the RunLoop call.
    */
    while ( CFE_ES_RunLoop(&SBEI_AppData.RunStatus) == TRUE )
    {
    
        /*
        ** Performance Log Exit Stamp.
        */
        CFE_ES_PerfLogExit(SBEI_APPMAIN_PERF_ID);
   
        /*
        ** Pend on the arrival of the next Software Bus message.
        */
        Status = CFE_SB_RcvMsg(&SBEI_AppData.MsgPtr, SBEI_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /*
        ** Performance Log Entry Stamp.
        */      
        CFE_ES_PerfLogEntry(SBEI_APPMAIN_PERF_ID);

        /*
        ** Check the return status from the Software Bus.
        */        
        if (Status == CFE_SUCCESS)
        {
            /*
            ** Process Software Bus message. If there are fatal errors
            ** in command processing the command can alter the global 
            ** RunStatus variable to exit the main event loop.
            */
            SBEI_AppPipe(SBEI_AppData.MsgPtr);
            
            /* 
            ** Update Critical Data Store. Because this data is only updated
            ** in one command, this could be moved the command processing function.
            */
            CFE_ES_CopyToCDS(SBEI_AppData.CDSHandle, &SBEI_AppData.WorkingCriticalData);

        }
        else
        {
            /*
            ** This is an example of exiting on an error.
            ** Note that a SB read error is not always going to 
            ** result in an app quitting. 
            */
            CFE_EVS_SendEvent(SBEI_PIPE_ERR_EID, CFE_EVS_ERROR,
                               "SBEI: SB Pipe Read Error, SBEI App Will Exit.");
            
            SBEI_AppData.RunStatus = CFE_ES_APP_ERROR;
         }
                    
    } /* end while */

    /*
    ** Performance Log Exit Stamp.
    */
    CFE_ES_PerfLogExit(SBEI_APPMAIN_PERF_ID);

    /*
    ** Exit the Application.
    */
    CFE_ES_ExitApp(SBEI_AppData.RunStatus);
    
} /* End of SBEI_AppMain() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_AppInit() -- SBEI initialization                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SBEI_AppInit(void)
{
    int32  Status;
    int32  ResetType;
    uint32 ResetSubType;

    /* 
    ** Determine Reset Type
    */
    ResetType = CFE_ES_GetResetType(&ResetSubType);
    
    /*
    ** For a Poweron Reset, initialize the Critical variables.
    ** If it is a Processor Reset, these variables will be restored from
    ** the Critical Data Store later in this function.
    */
    if ( ResetType == CFE_ES_POWERON_RESET )
    {
       SBEI_AppData.WorkingCriticalData.DataPtOne   = 1;
       SBEI_AppData.WorkingCriticalData.DataPtTwo   = 2;
       SBEI_AppData.WorkingCriticalData.DataPtThree = 3;
       SBEI_AppData.WorkingCriticalData.DataPtFour  = 4;
       SBEI_AppData.WorkingCriticalData.DataPtFive  = 5;
    }
    
    /*
    ** Setup the RunStatus variable
    */
    SBEI_AppData.RunStatus = CFE_ES_APP_RUN;
    
    /*
    ** Initialize app command execution counters.
    */
    SBEI_AppData.CmdCounter = 0;
    SBEI_AppData.ErrCounter = 0;

    /*
    ** Initialize app configuration data.
    */
    strcpy(SBEI_AppData.PipeName, "SBEI_CMD_PIPE");
    SBEI_AppData.PipeDepth = SBEI_PIPE_DEPTH;

    SBEI_AppData.LimitHK   = SBEI_LIMIT_HK;
    SBEI_AppData.LimitCmd  = SBEI_LIMIT_CMD;

    /*
    ** Initialize event filter table for envents we want to filter.
    */
    SBEI_AppData.EventFilters[0].EventID = SBEI_PROCESS_INF_EID;
    SBEI_AppData.EventFilters[0].Mask    = CFE_EVS_EVERY_FOURTH_ONE;
    
    SBEI_AppData.EventFilters[1].EventID = SBEI_RESET_INF_EID;
    SBEI_AppData.EventFilters[1].Mask    = CFE_EVS_NO_FILTER;
    
    SBEI_AppData.EventFilters[2].EventID = SBEI_CC1_ERR_EID;
    SBEI_AppData.EventFilters[2].Mask    = CFE_EVS_EVERY_OTHER_TWO;
    
    SBEI_AppData.EventFilters[3].EventID = SBEI_LEN_ERR_EID;
    SBEI_AppData.EventFilters[3].Mask    = CFE_EVS_FIRST_8_STOP;


    /*
    ** Register event filter table.
    */
    Status = CFE_EVS_Register(SBEI_AppData.EventFilters, 4,
                               CFE_EVS_BINARY_FILTER);
    
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("SBEI App: Error Registering Events, RC = 0x%08X\n", Status);
       return ( Status );
    }
             
    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_SB_InitMsg(&SBEI_AppData.HkPacket, SBEI_HK_TLM_MID, sizeof(SBEI_HkPacket_t), TRUE);
   
    /*
    ** Create Software Bus message pipe.
    */
    Status = CFE_SB_CreatePipe(&SBEI_AppData.CmdPipe, SBEI_AppData.PipeDepth, SBEI_AppData.PipeName);
    if ( Status != CFE_SUCCESS )
    {
       /*
       ** Could use an event at this point
       */
       CFE_ES_WriteToSysLog("SBEI App: Error Creating SB Pipe, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(SBEI_SEND_HK_MID,SBEI_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("SBEI App: Error Subscribing to HK Request, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to SBEI ground command packets
    */
    Status = CFE_SB_Subscribe(SBEI_CMD_MID,SBEI_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("SBEI App: Error Subscribing to SBEI Command, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Register tables with cFE and load default data
    */
    Status = CFE_TBL_Register(&SBEI_AppData.TblHandles[0], "MyFirstTbl",
                              sizeof(SBEI_Tbl_1_t), CFE_TBL_OPT_DEFAULT,
                              SBEI_FirstTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("SBEI App: Error Registering Table 1, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
       Status = CFE_TBL_Load(SBEI_AppData.TblHandles[0], CFE_TBL_SRC_FILE, SBEI_FIRST_TBL_FILE);
    }
    
    Status = CFE_TBL_Register(&SBEI_AppData.TblHandles[1], "MySecondTbl",
                              sizeof(SBEI_Tbl_2_t), CFE_TBL_OPT_DEFAULT,
                              SBEI_SecondTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("SBEI App: Error Registering Table 2, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
      Status = CFE_TBL_Load(SBEI_AppData.TblHandles[1], CFE_TBL_SRC_FILE, SBEI_SECOND_TBL_FILE);
    }
                 

    /*
    ** Create and manage the Critical Data Store 
    */
    Status = CFE_ES_RegisterCDS(&SBEI_AppData.CDSHandle, sizeof(SBEI_CdsDataType_t), SBEI_CDS_NAME);
    if(Status == CFE_SUCCESS)
    {
       /* 
       ** Setup Initial contents of Critical Data Store 
       */
       CFE_ES_CopyToCDS(SBEI_AppData.CDSHandle, &SBEI_AppData.WorkingCriticalData);
       
    }
    else if(Status == CFE_ES_CDS_ALREADY_EXISTS)
    {
       /* 
       ** Critical Data Store already existed, we need to get a copy 
       ** of its current contents to see if we can use it
       */
       Status = CFE_ES_RestoreFromCDS(&SBEI_AppData.WorkingCriticalData, SBEI_AppData.CDSHandle);
       if(Status == CFE_SUCCESS)
       {
          /*
          ** Perform any logical verifications, if necessary, to validate data 
          */
          CFE_ES_WriteToSysLog("SBEI App CDS data preserved\n");
       }
       else
       {
          /* 
          ** Restore Failied, Perform baseline initialization 
          */
          SBEI_AppData.WorkingCriticalData.DataPtOne   = 1;
          SBEI_AppData.WorkingCriticalData.DataPtTwo   = 2;
          SBEI_AppData.WorkingCriticalData.DataPtThree = 3;
          SBEI_AppData.WorkingCriticalData.DataPtFour  = 4;
          SBEI_AppData.WorkingCriticalData.DataPtFive  = 5;
          CFE_ES_WriteToSysLog("Failed to Restore CDS. Re-Initialized CDS Data.\n");
       }
    }
    else 
    {
       /* 
       ** Error creating my critical data store 
       */
       CFE_ES_WriteToSysLog("SBEI: Failed to create CDS (Err=0x%08x)", Status);
       return(Status);
    }

    /*
    ** Application startup event message.
    */
    CFE_EVS_SendEvent(SBEI_INIT_INF_EID,CFE_EVS_INFORMATION, "SBEI: Application Initialized");
                         
    return(CFE_SUCCESS);

} /* End of SBEI_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_AppPipe() -- Process command pipe message                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SBEI_AppPipe(CFE_SB_MsgPtr_t msg)
{
    CFE_SB_MsgId_t MessageID;
    uint16 CommandCode;

    MessageID = CFE_SB_GetMsgId(msg);
    switch (MessageID)
    {
        /*
        ** Housekeeping telemetry request.
        */
        case SBEI_SEND_HK_MID:
            SBEI_HousekeepingCmd(msg);
            break;

        /*
        ** SBEI ground commands.
        */
        case SBEI_CMD_MID:

            CommandCode = CFE_SB_GetCmdCode(msg);
            switch (CommandCode)
            {

                case SBEI_NOOP_CC:
                    SBEI_NoopCmd(msg);
                    break;

                case SBEI_RESET_CC:
                    SBEI_ResetCmd(msg);
                    break;
                    
                case SBEI_PROCESS_CC:
                    SBEI_RoutineProcessingCmd(msg);
                    break;

                default:
                    CFE_EVS_SendEvent(SBEI_CC1_ERR_EID, CFE_EVS_ERROR,
                     "Invalid ground command code: ID = 0x%X, CC = %d",
                                      MessageID, CommandCode);
                    break;
            }
            break;

        default:

            CFE_EVS_SendEvent(SBEI_MID_ERR_EID, CFE_EVS_ERROR,
                             "Invalid command pipe message ID: 0x%X",
                              MessageID);
            break;
    }

    return;

} /* End of SBEI_AppPipe() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_HousekeepingCmd() -- On-board command (HK request)           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SBEI_HousekeepingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(SBEI_NoArgsCmd_t);
    uint16 i;

    /*
    ** Verify command packet length
    */
    if (SBEI_VerifyCmdLength(msg, ExpectedLength))
    {
        /*
        ** Get command execution counters
        */
        SBEI_AppData.HkPacket.CmdCounter = SBEI_AppData.CmdCounter;
        SBEI_AppData.HkPacket.ErrCounter = SBEI_AppData.ErrCounter;

        /*
        ** Send housekeeping telemetry packet
        */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SBEI_AppData.HkPacket);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SBEI_AppData.HkPacket);

        /*
        ** Manage any pending table loads, validations, etc.
        */
        for (i=0; i<SBEI_NUM_TABLES; i++)
        {
            CFE_TBL_Manage(SBEI_AppData.TblHandles[i]);
        }
        
        /*
        ** This command does not affect the command execution counter
        */
        
    } /* end if */

    return;

} /* End of SBEI_HousekeepingCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_NoopCmd() -- SBEI ground command (NO-OP)                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SBEI_NoopCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(SBEI_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (SBEI_VerifyCmdLength(msg, ExpectedLength))
    {
        SBEI_AppData.CmdCounter++;

        CFE_EVS_SendEvent(SBEI_NOOP_INF_EID, CFE_EVS_INFORMATION,
                         "SBEI Version 1.0.0: No-op command");
    }

    return;

} /* End of SBEI_NoopCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_ResetCmd() -- SBEI ground command (reset counters)         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SBEI_ResetCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(SBEI_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (SBEI_VerifyCmdLength(msg, ExpectedLength))
    {
        SBEI_AppData.CmdCounter = 0;
        SBEI_AppData.ErrCounter = 0;

        CFE_EVS_SendEvent(SBEI_RESET_INF_EID, CFE_EVS_INFORMATION,
                         "SBEI: Reset Counters command");
    }

    return;

} /* End of SBEI_ResetCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  SBEI_RoutineProcessingCmd() - Common Processing for each cmd.    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SBEI_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(SBEI_NoArgsCmd_t);
    SBEI_Tbl_1_t   *MyFirstTblPtr;
    SBEI_Tbl_2_t   *MySecondTblPtr;

    /*
    ** Verify command packet length
    */
    if (SBEI_VerifyCmdLength(msg, ExpectedLength))
    {
        /* Obtain access to table data addresses */
        CFE_TBL_GetAddress((void *)&MyFirstTblPtr, SBEI_AppData.TblHandles[0]);
        CFE_TBL_GetAddress((void *)&MySecondTblPtr, SBEI_AppData.TblHandles[1]);
        
        /* Perform routine processing accessing table data via pointers */
        /*                            .                                 */
        /*                            .                                 */
        /*                            .                                 */
        
        /* Once completed with using tables, release addresses          */
        CFE_TBL_ReleaseAddress(SBEI_AppData.TblHandles[0]);
        CFE_TBL_ReleaseAddress(SBEI_AppData.TblHandles[1]);
        
        /*
        ** Update Critical variables. These variables will be saved
        ** in the Critical Data Store and preserved on a processor reset.
        */
        SBEI_AppData.WorkingCriticalData.DataPtOne++;
        SBEI_AppData.WorkingCriticalData.DataPtTwo++;
        SBEI_AppData.WorkingCriticalData.DataPtThree++;
        SBEI_AppData.WorkingCriticalData.DataPtFour++;
        SBEI_AppData.WorkingCriticalData.DataPtFive++;
        
        CFE_EVS_SendEvent(SBEI_PROCESS_INF_EID,CFE_EVS_INFORMATION, "SBEI: Routine Processing Command");

    }

    return;

} /* End of SBEI_RoutineProcessingCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_VerifyCmdLength() -- Verify command packet length            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean SBEI_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{
    boolean result = TRUE;
    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length...
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID = CFE_SB_GetMsgId(msg);
        uint16 CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(SBEI_LEN_ERR_EID, CFE_EVS_ERROR,
           "SBEI: Invalid cmd pkt: ID = 0x%X,  CC = %d, Len = %d",
                          MessageID, CommandCode, ActualLength);
        result = FALSE;
        SBEI_AppData.ErrCounter++;
    }

    return(result);

} /* End of SBEI_VerifyCmdLength() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_FirstTblValidationFunc() -- Verify contents of First Table   */
/*                                buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SBEI_FirstTblValidationFunc(void *TblData)
{
    int32              ReturnCode = CFE_SUCCESS;
    SBEI_Tbl_1_t *TblDataPtr = (SBEI_Tbl_1_t *)TblData;
    
    if (TblDataPtr->TblElement1 > SBEI_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = SBEI_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    
    return ReturnCode;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_SecondTblValidationFunc() -- Verify contents of Second Table */
/*                                 buffer contents                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 SBEI_SecondTblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    SBEI_Tbl_2_t *TblDataPtr = (SBEI_Tbl_2_t *)TblData;
    
    if (TblDataPtr->TblElement3 > SBEI_TBL_ELEMENT_3_MAX)
    {
        /* Third element is out of range, return an appropriate error code */
        ReturnCode = SBEI_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    

    return ReturnCode;
}

/************************/
/*  End of File Comment */
/************************/
