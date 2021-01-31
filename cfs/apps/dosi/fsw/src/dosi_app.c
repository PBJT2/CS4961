/*******************************************************************************
** File: dosi_app.c
**
** Purpose:
**   This file contains the source code for the Dosi App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "dosi_app.h"
#include "dosi_app_perfids.h"
#include "dosi_app_msgids.h"


/*
** System Header files
*/
#include <string.h>

/*
** DOSI global data
*/
DOSI_AppData_t DOSI_AppData;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_AppMain() -- Application entry point and main process loop   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_AppMain(void)
{
    int32 Status;
    
    /*
    ** Register the Application with Executive Services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(DOSI_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Intialization fails, set the RunStatus to CFE_ES_APP_ERROR
    ** and the App will not enter the RunLoop.
    */
    Status = DOSI_AppInit();
    if ( Status != CFE_SUCCESS )
    {
        DOSI_AppData.RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** Application Main Loop. Call CFE_ES_RunLoop to check for changes
    ** in the Application's status. If there is a request to kill this
    ** App, it will be passed in through the RunLoop call.
    */
    while ( CFE_ES_RunLoop(&DOSI_AppData.RunStatus) == TRUE )
    {
    
        /*
        ** Performance Log Exit Stamp.
        */
        CFE_ES_PerfLogExit(DOSI_APPMAIN_PERF_ID);
   
        /*
        ** Pend on the arrival of the next Software Bus message.
        */
        Status = CFE_SB_RcvMsg(&DOSI_AppData.MsgPtr, DOSI_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /*
        ** Performance Log Entry Stamp.
        */      
        CFE_ES_PerfLogEntry(DOSI_APPMAIN_PERF_ID);

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
            DOSI_AppPipe(DOSI_AppData.MsgPtr);
            
            /* 
            ** Update Critical Data Store. Because this data is only updated
            ** in one command, this could be moved the command processing function.
            */
            CFE_ES_CopyToCDS(DOSI_AppData.CDSHandle, &DOSI_AppData.WorkingCriticalData);

        }
        else
        {
            /*
            ** This is an example of exiting on an error.
            ** Note that a SB read error is not always going to 
            ** result in an app quitting. 
            */
            CFE_EVS_SendEvent(DOSI_PIPE_ERR_EID, CFE_EVS_ERROR,
                               "DOSI: SB Pipe Read Error, DOSI App Will Exit.");
            
            DOSI_AppData.RunStatus = CFE_ES_APP_ERROR;
         }
                    
    } /* end while */

    /*
    ** Performance Log Exit Stamp.
    */
    CFE_ES_PerfLogExit(DOSI_APPMAIN_PERF_ID);

    /*
    ** Exit the Application.
    */
    CFE_ES_ExitApp(DOSI_AppData.RunStatus);
    
} /* End of DOSI_AppMain() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_AppInit() -- DOSI initialization                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DOSI_AppInit(void)
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
       DOSI_AppData.WorkingCriticalData.DataPtOne   = 1;
       DOSI_AppData.WorkingCriticalData.DataPtTwo   = 2;
       DOSI_AppData.WorkingCriticalData.DataPtThree = 3;
       DOSI_AppData.WorkingCriticalData.DataPtFour  = 4;
       DOSI_AppData.WorkingCriticalData.DataPtFive  = 5;
    }
    
    /*
    ** Setup the RunStatus variable
    */
    DOSI_AppData.RunStatus = CFE_ES_APP_RUN;
    
    /*
    ** Initialize app command execution counters.
    */
    DOSI_AppData.CmdCounter = 0;
    DOSI_AppData.ErrCounter = 0;

    /*
    ** Initialize app configuration data.
    */
    strcpy(DOSI_AppData.PipeName, "DOSI_CMD_PIPE");
    DOSI_AppData.PipeDepth = DOSI_PIPE_DEPTH;

    DOSI_AppData.LimitHK   = DOSI_LIMIT_HK;
    DOSI_AppData.LimitCmd  = DOSI_LIMIT_CMD;

    /*
    ** Initialize event filter table for envents we want to filter.
    */
    DOSI_AppData.EventFilters[0].EventID = DOSI_PROCESS_INF_EID;
    DOSI_AppData.EventFilters[0].Mask    = CFE_EVS_EVERY_FOURTH_ONE;
    
    DOSI_AppData.EventFilters[1].EventID = DOSI_RESET_INF_EID;
    DOSI_AppData.EventFilters[1].Mask    = CFE_EVS_NO_FILTER;
    
    DOSI_AppData.EventFilters[2].EventID = DOSI_CC1_ERR_EID;
    DOSI_AppData.EventFilters[2].Mask    = CFE_EVS_EVERY_OTHER_TWO;
    
    DOSI_AppData.EventFilters[3].EventID = DOSI_LEN_ERR_EID;
    DOSI_AppData.EventFilters[3].Mask    = CFE_EVS_FIRST_8_STOP;


    /*
    ** Register event filter table.
    */
    Status = CFE_EVS_Register(DOSI_AppData.EventFilters, 4,
                               CFE_EVS_BINARY_FILTER);
    
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSI App: Error Registering Events, RC = 0x%08X\n", Status);
       return ( Status );
    }
             
    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_SB_InitMsg(&DOSI_AppData.HkPacket, DOSI_HK_TLM_MID, sizeof(DOSI_HkPacket_t), TRUE);
   
    /*
    ** Create Software Bus message pipe.
    */
    Status = CFE_SB_CreatePipe(&DOSI_AppData.CmdPipe, DOSI_AppData.PipeDepth, DOSI_AppData.PipeName);
    if ( Status != CFE_SUCCESS )
    {
       /*
       ** Could use an event at this point
       */
       CFE_ES_WriteToSysLog("DOSI App: Error Creating SB Pipe, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(DOSI_SEND_HK_MID,DOSI_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSI App: Error Subscribing to HK Request, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to DOSI ground command packets
    */
    Status = CFE_SB_Subscribe(DOSI_CMD_MID,DOSI_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSI App: Error Subscribing to DOSI Command, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Register tables with cFE and load default data
    */
    Status = CFE_TBL_Register(&DOSI_AppData.TblHandles[0], "MyFirstTbl",
                              sizeof(DOSI_Tbl_1_t), CFE_TBL_OPT_DEFAULT,
                              DOSI_FirstTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSI App: Error Registering Table 1, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
       Status = CFE_TBL_Load(DOSI_AppData.TblHandles[0], CFE_TBL_SRC_FILE, DOSI_FIRST_TBL_FILE);
    }
    
    Status = CFE_TBL_Register(&DOSI_AppData.TblHandles[1], "MySecondTbl",
                              sizeof(DOSI_Tbl_2_t), CFE_TBL_OPT_DEFAULT,
                              DOSI_SecondTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSI App: Error Registering Table 2, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
      Status = CFE_TBL_Load(DOSI_AppData.TblHandles[1], CFE_TBL_SRC_FILE, DOSI_SECOND_TBL_FILE);
    }
                 

    /*
    ** Create and manage the Critical Data Store 
    */
    Status = CFE_ES_RegisterCDS(&DOSI_AppData.CDSHandle, sizeof(DOSI_CdsDataType_t), DOSI_CDS_NAME);
    if(Status == CFE_SUCCESS)
    {
       /* 
       ** Setup Initial contents of Critical Data Store 
       */
       CFE_ES_CopyToCDS(DOSI_AppData.CDSHandle, &DOSI_AppData.WorkingCriticalData);
       
    }
    else if(Status == CFE_ES_CDS_ALREADY_EXISTS)
    {
       /* 
       ** Critical Data Store already existed, we need to get a copy 
       ** of its current contents to see if we can use it
       */
       Status = CFE_ES_RestoreFromCDS(&DOSI_AppData.WorkingCriticalData, DOSI_AppData.CDSHandle);
       if(Status == CFE_SUCCESS)
       {
          /*
          ** Perform any logical verifications, if necessary, to validate data 
          */
          CFE_ES_WriteToSysLog("DOSI App CDS data preserved\n");
       }
       else
       {
          /* 
          ** Restore Failied, Perform baseline initialization 
          */
          DOSI_AppData.WorkingCriticalData.DataPtOne   = 1;
          DOSI_AppData.WorkingCriticalData.DataPtTwo   = 2;
          DOSI_AppData.WorkingCriticalData.DataPtThree = 3;
          DOSI_AppData.WorkingCriticalData.DataPtFour  = 4;
          DOSI_AppData.WorkingCriticalData.DataPtFive  = 5;
          CFE_ES_WriteToSysLog("Failed to Restore CDS. Re-Initialized CDS Data.\n");
       }
    }
    else 
    {
       /* 
       ** Error creating my critical data store 
       */
       CFE_ES_WriteToSysLog("DOSI: Failed to create CDS (Err=0x%08x)", Status);
       return(Status);
    }

    /*
    ** Application startup event message.
    */
    CFE_EVS_SendEvent(DOSI_INIT_INF_EID,CFE_EVS_INFORMATION, "DOSI: Application Initialized");
                         
    return(CFE_SUCCESS);

} /* End of DOSI_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_AppPipe() -- Process command pipe message                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_AppPipe(CFE_SB_MsgPtr_t msg)
{
    CFE_SB_MsgId_t MessageID;
    uint16 CommandCode;

    MessageID = CFE_SB_GetMsgId(msg);
    switch (MessageID)
    {
        /*
        ** Housekeeping telemetry request.
        */
        case DOSI_SEND_HK_MID:
            DOSI_HousekeepingCmd(msg);
            break;

        /*
        ** DOSI ground commands.
        */
        case DOSI_CMD_MID:

            CommandCode = CFE_SB_GetCmdCode(msg);
            switch (CommandCode)
            {

		case DOSI_INJECT_CC:
                    DOSI_InjectCmd(msg);
                    break;
 
                case DOSI_NOOP_CC:
                    DOSI_NoopCmd(msg);
                    break;

                case DOSI_RESET_CC:
                    DOSI_ResetCmd(msg);
                    break;
                    
                case DOSI_PROCESS_CC:
                    DOSI_RoutineProcessingCmd(msg);
                    break;

                default:
                    CFE_EVS_SendEvent(DOSI_CC1_ERR_EID, CFE_EVS_ERROR,
                     "Invalid ground command code: ID = 0x%X, CC = %d",
                                      MessageID, CommandCode);
                    break;
            }
            break;

        default:

            CFE_EVS_SendEvent(DOSI_MID_ERR_EID, CFE_EVS_ERROR,
                             "Invalid command pipe message ID: 0x%X",
                              MessageID);
            break;
    }

    return;

} /* End of DOSI_AppPipe() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_HousekeepingCmd() -- On-board command (HK request)           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_HousekeepingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSI_NoArgsCmd_t);
    uint16 i;

    /*
    ** Verify command packet length
    */
    if (DOSI_VerifyCmdLength(msg, ExpectedLength))
    {
        /*
        ** Get command execution counters
        */
        DOSI_AppData.HkPacket.CmdCounter = DOSI_AppData.CmdCounter;
        DOSI_AppData.HkPacket.ErrCounter = DOSI_AppData.ErrCounter;

        /*
        ** Send housekeeping telemetry packet
        */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &DOSI_AppData.HkPacket);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &DOSI_AppData.HkPacket);

        /*
        ** Manage any pending table loads, validations, etc.
        */
        for (i=0; i<DOSI_NUM_TABLES; i++)
        {
            CFE_TBL_Manage(DOSI_AppData.TblHandles[i]);
        }
        
        /*
        ** This command does not affect the command execution counter
        */
        
    } /* end if */

    return;

} /* End of DOSI_HousekeepingCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_InjectCmd() -- DOSI ground command (Injection)             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_InjectCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSI_NoArgsCmd_t);

    char command1[] = "cd ";
    char *command2 = getenv("HOME");
    char command3[] = "/OpenSatKit-master/cfs/apps/dosi/fsw/src; ./floodcmd";
    char command[100];
    snprintf(command, 100, "%s%s%s", command1, command2, command3);
    
    /*
    ** Verify command packet length...
    ** Execute the flooding attack from another file
    ** Can also try to change the environment here
    */
    if (DOSI_VerifyCmdLength(msg, ExpectedLength))
    {
        DOSI_AppData.CmdCounter++;

        CFE_EVS_SendEvent (DOSI_INJECT_INF_EID, CFE_EVS_INFORMATION,
        "Injecting Network Flooding Attack");

	//system("cd /home/osk/OpenSatKit-master/cfs/apps/dosi/fsw/src; ./floodcmd");
        system(command);
    }

    return;

} /* End of DOSI_InjectCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_NoopCmd() -- DOSI ground command (NO-OP)                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_NoopCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSI_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (DOSI_VerifyCmdLength(msg, ExpectedLength))
    {
        DOSI_AppData.CmdCounter++;

        CFE_EVS_SendEvent(DOSI_NOOP_INF_EID, CFE_EVS_INFORMATION,
                         "DOSI Version 1.0.0: No-op command");
    }

    return;

} /* End of DOSI_NoopCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_ResetCmd() -- DOSI ground command (reset counters)             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_ResetCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSI_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (DOSI_VerifyCmdLength(msg, ExpectedLength))
    {
        DOSI_AppData.CmdCounter = 0;
        DOSI_AppData.ErrCounter = 0;

        CFE_EVS_SendEvent(DOSI_RESET_INF_EID, CFE_EVS_INFORMATION,
                         "DOSI: Reset Counters command");
    }

    return;

} /* End of DOSI_ResetCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  DOSI_RoutineProcessingCmd() - Common Processing for each cmd.    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSI_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSI_NoArgsCmd_t);
    DOSI_Tbl_1_t   *MyFirstTblPtr;
    DOSI_Tbl_2_t   *MySecondTblPtr;

    /*
    ** Verify command packet length
    */
    if (DOSI_VerifyCmdLength(msg, ExpectedLength))
    {
        /* Obtain access to table data addresses */
        CFE_TBL_GetAddress((void *)&MyFirstTblPtr, DOSI_AppData.TblHandles[0]);
        CFE_TBL_GetAddress((void *)&MySecondTblPtr, DOSI_AppData.TblHandles[1]);
        
        /* Perform routine processing accessing table data via pointers */
        /*                            .                                 */
        /*                            .                                 */
        /*                            .                                 */
        
        /* Once completed with using tables, release addresses          */
        CFE_TBL_ReleaseAddress(DOSI_AppData.TblHandles[0]);
        CFE_TBL_ReleaseAddress(DOSI_AppData.TblHandles[1]);
        
        /*
        ** Update Critical variables. These variables will be saved
        ** in the Critical Data Store and preserved on a processor reset.
        */
        DOSI_AppData.WorkingCriticalData.DataPtOne++;
        DOSI_AppData.WorkingCriticalData.DataPtTwo++;
        DOSI_AppData.WorkingCriticalData.DataPtThree++;
        DOSI_AppData.WorkingCriticalData.DataPtFour++;
        DOSI_AppData.WorkingCriticalData.DataPtFive++;
        
        CFE_EVS_SendEvent(DOSI_PROCESS_INF_EID,CFE_EVS_INFORMATION, "DOSI: Routine Processing Command");
        
        DOSI_AppData.CmdCounter++;
    }

    return;

} /* End of DOSI_RoutineProcessingCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_VerifyCmdLength() -- Verify command packet length            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean DOSI_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(DOSI_LEN_ERR_EID, CFE_EVS_ERROR,
           "DOSI: Invalid cmd pkt: ID = 0x%X,  CC = %d, Len = %d",
                          MessageID, CommandCode, ActualLength);
        result = FALSE;
        DOSI_AppData.ErrCounter++;
    }

    return(result);

} /* End of DOSI_VerifyCmdLength() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_FirstTblValidationFunc() -- Verify contents of First Table   */
/*                                buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DOSI_FirstTblValidationFunc(void *TblData)
{
    int32              ReturnCode = CFE_SUCCESS;
    DOSI_Tbl_1_t *TblDataPtr = (DOSI_Tbl_1_t *)TblData;
    
    if (TblDataPtr->TblElement1 > DOSI_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = DOSI_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    
    return ReturnCode;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSI_SecondTblValidationFunc() -- Verify contents of Second Table */
/*                                 buffer contents                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DOSI_SecondTblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    DOSI_Tbl_2_t *TblDataPtr = (DOSI_Tbl_2_t *)TblData;
    
    if (TblDataPtr->TblElement3 > DOSI_TBL_ELEMENT_3_MAX)
    {
        /* Third element is out of range, return an appropriate error code */
        ReturnCode = DOSI_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    

    return ReturnCode;
}

/************************/
/*  End of File Comment */
/************************/
