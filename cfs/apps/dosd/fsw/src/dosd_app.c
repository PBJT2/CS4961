/*******************************************************************************
** File: dosd_app.c
**
** Purpose:
**   This file contains the source code for the Dosd App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "dosd_app.h"
#include "dosd_app_perfids.h"
#include "dosd_app_msgids.h"



/*
** System Header files
*/
#include <string.h>

/*
** DOSD global data
*/
DOSD_AppData_t DOSD_AppData;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_AppMain() -- Application entry point and main process loop   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_AppMain(void)
{
    int32 Status;
    
    /*
    ** Register the Application with Executive Services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(DOSD_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Intialization fails, set the RunStatus to CFE_ES_APP_ERROR
    ** and the App will not enter the RunLoop.
    */
    Status = DOSD_AppInit();
    if ( Status != CFE_SUCCESS )
    {
        DOSD_AppData.RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** Application Main Loop. Call CFE_ES_RunLoop to check for changes
    ** in the Application's status. If there is a request to kill this
    ** App, it will be passed in through the RunLoop call.
    */
    while ( CFE_ES_RunLoop(&DOSD_AppData.RunStatus) == TRUE )
    {
    
        /*
        ** Performance Log Exit Stamp.
        */
        CFE_ES_PerfLogExit(DOSD_APPMAIN_PERF_ID);
   
        /*
        ** Pend on the arrival of the next Software Bus message.
        */
        Status = CFE_SB_RcvMsg(&DOSD_AppData.MsgPtr, DOSD_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /*
        ** Performance Log Entry Stamp.
        */      
        CFE_ES_PerfLogEntry(DOSD_APPMAIN_PERF_ID);

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
            DOSD_AppPipe(DOSD_AppData.MsgPtr);
            
            /* 
            ** Update Critical Data Store. Because this data is only updated
            ** in one command, this could be moved the command processing function.
            */
            CFE_ES_CopyToCDS(DOSD_AppData.CDSHandle, &DOSD_AppData.WorkingCriticalData);

        }
        else
        {
            /*
            ** This is an example of exiting on an error.
            ** Note that a SB read error is not always going to 
            ** result in an app quitting. 
            */
            CFE_EVS_SendEvent(DOSD_PIPE_ERR_EID, CFE_EVS_ERROR,
                               "DOSD: SB Pipe Read Error, DOSD App Will Exit.");
            
            DOSD_AppData.RunStatus = CFE_ES_APP_ERROR;
         }
                    
    } /* end while */

    /*
    ** Performance Log Exit Stamp.
    */
    CFE_ES_PerfLogExit(DOSD_APPMAIN_PERF_ID);

    /*
    ** Exit the Application.
    */
    CFE_ES_ExitApp(DOSD_AppData.RunStatus);
    
} /* End of DOSD_AppMain() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_AppInit() -- DOSD initialization                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DOSD_AppInit(void)
{
    int32  Status;
    int32  ResetType;
    uint32 ResetSubType;

    //added this part from CS, check if this is necessary
    int32                                       Result = CFE_SUCCESS;
    Result = CFE_EVS_Register(NULL, 0, 0);

    if (Result != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("DOSD App: Error Registering For Event Services, RC = 0x%08X\n", (unsigned int)Result);
        return Result;
    }

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
       DOSD_AppData.WorkingCriticalData.DataPtOne   = 1;
       DOSD_AppData.WorkingCriticalData.DataPtTwo   = 2;
       DOSD_AppData.WorkingCriticalData.DataPtThree = 3;
       DOSD_AppData.WorkingCriticalData.DataPtFour  = 4;
       DOSD_AppData.WorkingCriticalData.DataPtFive  = 5;
    }
    
    /*
    ** Setup the RunStatus variable
    */
    DOSD_AppData.RunStatus = CFE_ES_APP_RUN;
    
    /*
    ** Initialize app command execution counters.
    */
    DOSD_AppData.CmdCounter = 0;
    DOSD_AppData.ErrCounter = 0;

    /*
    ** Initialize app configuration data.
    */
    strcpy(DOSD_AppData.PipeName, "DOSD_CMD_PIPE");
    DOSD_AppData.PipeDepth = DOSD_PIPE_DEPTH;

    DOSD_AppData.LimitHK   = DOSD_LIMIT_HK;
    DOSD_AppData.LimitCmd  = DOSD_LIMIT_CMD;


    /*TFTP Connection State*/
    DOSD_AppData.ConnectionState = TRUE;

    /*
    ** Initialize event filter table for envents we want to filter.
    */
    DOSD_AppData.EventFilters[0].EventID = DOSD_PROCESS_INF_EID;
    DOSD_AppData.EventFilters[0].Mask    = CFE_EVS_EVERY_FOURTH_ONE;
    
    DOSD_AppData.EventFilters[1].EventID = DOSD_RESET_INF_EID;
    DOSD_AppData.EventFilters[1].Mask    = CFE_EVS_NO_FILTER;
    
    DOSD_AppData.EventFilters[2].EventID = DOSD_CC1_ERR_EID;
    DOSD_AppData.EventFilters[2].Mask    = CFE_EVS_EVERY_OTHER_TWO;
    
    DOSD_AppData.EventFilters[3].EventID = DOSD_LEN_ERR_EID;
    DOSD_AppData.EventFilters[3].Mask    = CFE_EVS_FIRST_8_STOP;


    /*
    ** Register event filter table.
    */
    Status = CFE_EVS_Register(DOSD_AppData.EventFilters, 4,
                               CFE_EVS_BINARY_FILTER);
    
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSD App: Error Registering Events, RC = 0x%08X\n", Status);
       return ( Status );
    }
             
    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_SB_InitMsg(& DOSD_AppData.HkPacket, DOSD_HK_TLM_MID, sizeof(DOSD_HkPacket_t), TRUE);
   
    /*
    ** Create Software Bus message pipe.
    */
    Status = CFE_SB_CreatePipe(&DOSD_AppData.CmdPipe, DOSD_AppData.PipeDepth, DOSD_AppData.PipeName);
    if ( Status != CFE_SUCCESS )
    {
       /*
       ** Could use an event at this point
       */
       CFE_ES_WriteToSysLog("DOSD App: Error Creating SB Pipe, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(DOSD_SEND_HK_MID,DOSD_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSD App: Error Subscribing to HK Request, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to DOSD ground command packets
    */
    Status = CFE_SB_Subscribe(DOSD_CMD_MID,DOSD_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSD App: Error Subscribing to DOSD Command, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Register tables with cFE and load default data
    */
    Status = CFE_TBL_Register(&DOSD_AppData.TblHandles[0], "MyFirstTbl",
                              sizeof(DOSD_Tbl_1_t), CFE_TBL_OPT_DEFAULT,
                              DOSD_FirstTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSD App: Error Registering Table 1, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
       Status = CFE_TBL_Load(DOSD_AppData.TblHandles[0], CFE_TBL_SRC_FILE, DOSD_FIRST_TBL_FILE);
    }
    
    Status = CFE_TBL_Register(&DOSD_AppData.TblHandles[1], "MySecondTbl",
                              sizeof(DOSD_Tbl_2_t), CFE_TBL_OPT_DEFAULT,
                              DOSD_SecondTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("DOSD App: Error Registering Table 2, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
      Status = CFE_TBL_Load(DOSD_AppData.TblHandles[1], CFE_TBL_SRC_FILE, DOSD_SECOND_TBL_FILE);
    }
                 

    /*
    ** Create and manage the Critical Data Store 
    */
    Status = CFE_ES_RegisterCDS(&DOSD_AppData.CDSHandle, sizeof(DOSD_CdsDataType_t), DOSD_CDS_NAME);
    if(Status == CFE_SUCCESS)
    {
       /* 
       ** Setup Initial contents of Critical Data Store 
       */
       CFE_ES_CopyToCDS(DOSD_AppData.CDSHandle, &DOSD_AppData.WorkingCriticalData);
       
    }
    else if(Status == CFE_ES_CDS_ALREADY_EXISTS)
    {
       /* 
       ** Critical Data Store already existed, we need to get a copy 
       ** of its current contents to see if we can use it
       */
       Status = CFE_ES_RestoreFromCDS(&DOSD_AppData.WorkingCriticalData, DOSD_AppData.CDSHandle);
       if(Status == CFE_SUCCESS)
       {
          /*
          ** Perform any logical verifications, if necessary, to validate data 
          */
          CFE_ES_WriteToSysLog("DOSD App CDS data preserved\n");
       }
       else
       {
          /* 
          ** Restore Failied, Perform baseline initialization 
          */
          DOSD_AppData.WorkingCriticalData.DataPtOne   = 1;
          DOSD_AppData.WorkingCriticalData.DataPtTwo   = 2;
          DOSD_AppData.WorkingCriticalData.DataPtThree = 3;
          DOSD_AppData.WorkingCriticalData.DataPtFour  = 4;
          DOSD_AppData.WorkingCriticalData.DataPtFive  = 5;
          CFE_ES_WriteToSysLog("Failed to Restore CDS. Re-Initialized CDS Data.\n");
       }
    }
    else 
    {
       /* 
       ** Error creating my critical data store 
       */
       CFE_ES_WriteToSysLog("DOSD: Failed to create CDS (Err=0x%08x)", Status);
       return(Status);
    }

    /*
    ** Application startup event message.
    */
    CFE_EVS_SendEvent(DOSD_INIT_INF_EID,CFE_EVS_INFORMATION, "DOSD: Application Initialized");
                         
    return(CFE_SUCCESS);

} /* End of DOSD_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_AppPipe() -- Process command pipe message                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_AppPipe(CFE_SB_MsgPtr_t msg)
{
    CFE_SB_MsgId_t MessageID;
    uint16 CommandCode;
    MessageID = CFE_SB_GetMsgId(msg);

    switch (MessageID)
    {
        /*
        ** Housekeeping telemetry request.
        */
        case DOSD_SEND_HK_MID:
            DOSD_HousekeepingCmd(msg);
            break;

        /*
        ** DOSD ground commands.
        */
        case DOSD_CMD_MID:

            CommandCode = CFE_SB_GetCmdCode(msg);
            switch (CommandCode)
            {
                case DOSD_DETECT_CC:
                    DOSD_DetectCmd(msg);
                    break;

                case DOSD_NOOP_CC:
                    DOSD_NoopCmd(msg);
                    break;

                case DOSD_RESET_CC:
                    DOSD_ResetCmd(msg);
                    break;
                    
                case DOSD_PROCESS_CC:
                    DOSD_RoutineProcessingCmd(msg);
                    break;

                default:
                    CFE_EVS_SendEvent(DOSD_CC1_ERR_EID, CFE_EVS_ERROR,
                     "Invalid ground command code: ID = 0x%X, CC = %d",
                                      MessageID, CommandCode);
                    break;
            }
            break;

        default:

            CFE_EVS_SendEvent(DOSD_MID_ERR_EID, CFE_EVS_ERROR,
                             "Invalid command pipe message ID: 0x%X",
                              MessageID);
            break;
    }

    return;

} /* End of DOSD_AppPipe() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_HousekeepingCmd() -- On-board command (HK request)           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_HousekeepingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSD_NoArgsCmd_t);
    uint16 i;

    /*
    ** Verify command packet length
    */
    if (DOSD_VerifyCmdLength(msg, ExpectedLength))
    {
        /*
        ** Get command execution counters
        */
        DOSD_AppData.HkPacket.CmdCounter = DOSD_AppData.CmdCounter;
        DOSD_AppData.HkPacket.ErrCounter = DOSD_AppData.ErrCounter;

        /*
        ** Send housekeeping telemetry packet
        */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &DOSD_AppData.HkPacket);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &DOSD_AppData.HkPacket);
        /*
        ** Manage any pending table loads, validations, etc.
        */
        for (i=0; i<DOSD_NUM_TABLES; i++)
        {
            CFE_TBL_Manage(DOSD_AppData.TblHandles[i]);
        }
        
        /*
        ** This command does not affect the command execution counter
        */
        
    } /* end if */

    return;

} /* End of DOSD_HousekeepingCmd() */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_DetectCmd() -- DOSD ground command (Detect)                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_DetectCmd(CFE_SB_MsgPtr_t msg)
{
    DOSD_DetectCmd_t *DOSD_DetectCmd = (DOSD_DetectCmd_t *) msg;
    uint16 ExpectedLength = sizeof(DOSD_DetectCmd_t);
    FILE *file1;
    FILE *file2;
    int initByte;
    int byte;
    float rate;
    bool detected = false;
    /*
    ** Verify command packet length...
    ** Read initial byte and wait 1 sec for the second byte to get rates in kbps
    ** Threshold can be any rate depending on system. Once reached, output error and 
    ** stop TFTP app (id #15) 
    */
    if (DOSD_VerifyCmdLength(msg, ExpectedLength)) 
    { 
	CFE_EVS_SendEvent (DOSD_DETECT_INF_EID, CFE_EVS_INFORMATION,
        "Detection for Denial of Service Attack Activated");

	while (detected == false) {
	
	      file1 = fopen("/sys/class/net/enp0s3/statistics/rx_bytes", "r");
	      fscanf(file1, "%d\n", &initByte);

	      sleep(1);

	      file2 = fopen("/sys/class/net/enp0s3/statistics/rx_bytes", "r");
	      fscanf(file2, "%d\n", &byte);

	      rate = (byte-initByte)/1000.0;
	      //printf("%d : %d = %f\n", byte, initByte, rate);
	      //printf("%f\n", rate);

	      if(rate > 400.0) { 
		 CFE_EVS_SendEvent (DOSD_DETECT_INF_EID, CFE_EVS_INFORMATION, "Network Flooding Detected\n");
		 detected = true; 
			 
		 if(DOSD_DetectCmd->ConnectionState == FALSE) {
		    system("zenity --title 'Connection Error' --error --text 'Network Flooding Detected, Service Disconnected' 2> /dev/null");
		    CFE_ES_DeleteApp(15); 
		 } 
		 //else {
		   // system("zenity --title 'Connection Error' --error --text 'Network Flooding Detected, Network still connected' 2> /dev/null");
		 //}
	      }
	   }

        DOSD_AppData.CmdCounter++; //will show in HK after flooding detected
    }

    return;

} /* End of DOSD_DetectCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_NoopCmd() -- DOSD ground command (NO-OP)                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_NoopCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSD_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (DOSD_VerifyCmdLength(msg, ExpectedLength))
    {
        DOSD_AppData.CmdCounter++;

        CFE_EVS_SendEvent(DOSD_NOOP_INF_EID, CFE_EVS_INFORMATION,
                         "DOSD Version 1.0.0: No-op command");
    }

    return;

} /* End of DOSD_NoopCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_ResetCmd() -- DOSD ground command (reset counters)             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_ResetCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSD_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (DOSD_VerifyCmdLength(msg, ExpectedLength))
    {
        DOSD_AppData.CmdCounter = 0;
        DOSD_AppData.ErrCounter = 0;

        CFE_EVS_SendEvent(DOSD_RESET_INF_EID, CFE_EVS_INFORMATION,
                         "DOSD: Reset Counters command");
    }

    return;

} /* End of DOSD_ResetCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  DOSD_RoutineProcessingCmd() - Common Processing for each cmd.    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DOSD_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(DOSD_NoArgsCmd_t);
    DOSD_Tbl_1_t   *MyFirstTblPtr;
    DOSD_Tbl_2_t   *MySecondTblPtr;

    /*
    ** Verify command packet length
    */
    if (DOSD_VerifyCmdLength(msg, ExpectedLength))
    {
        /* Obtain access to table data addresses */
        CFE_TBL_GetAddress((void *)&MyFirstTblPtr, DOSD_AppData.TblHandles[0]);
        CFE_TBL_GetAddress((void *)&MySecondTblPtr, DOSD_AppData.TblHandles[1]);
        
        /* Perform routine processing accessing table data via pointers */
        /*                            .                                 */
        /*                            .                                 */
        /*                            .                                 */
        
        /* Once completed with using tables, release addresses          */
        CFE_TBL_ReleaseAddress(DOSD_AppData.TblHandles[0]);
        CFE_TBL_ReleaseAddress(DOSD_AppData.TblHandles[1]);
        
        /*
        ** Update Critical variables. These variables will be saved
        ** in the Critical Data Store and preserved on a processor reset.
        */
        DOSD_AppData.WorkingCriticalData.DataPtOne++;
        DOSD_AppData.WorkingCriticalData.DataPtTwo++;
        DOSD_AppData.WorkingCriticalData.DataPtThree++;
        DOSD_AppData.WorkingCriticalData.DataPtFour++;
        DOSD_AppData.WorkingCriticalData.DataPtFive++;
        
        CFE_EVS_SendEvent(DOSD_PROCESS_INF_EID,CFE_EVS_INFORMATION, "DOSD: Routine Processing Command");
        
        DOSD_AppData.CmdCounter++;
    }

    return;

} /* End of DOSD_RoutineProcessingCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_VerifyCmdLength() -- Verify command packet length            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean DOSD_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(DOSD_LEN_ERR_EID, CFE_EVS_ERROR,
           "DOSD: Invalid cmd pkt: ID = 0x%X,  CC = %d, Len = %d",
                          MessageID, CommandCode, ActualLength);
        result = FALSE;
        DOSD_AppData.ErrCounter++;
    }

    return(result);

} /* End of DOSD_VerifyCmdLength() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_FirstTblValidationFunc() -- Verify contents of First Table   */
/*                                buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DOSD_FirstTblValidationFunc(void *TblData)
{
    int32              ReturnCode = CFE_SUCCESS;
    DOSD_Tbl_1_t *TblDataPtr = (DOSD_Tbl_1_t *)TblData;
    
    if (TblDataPtr->TblElement1 > DOSD_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = DOSD_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    
    return ReturnCode;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DOSD_SecondTblValidationFunc() -- Verify contents of Second Table */
/*                                 buffer contents                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DOSD_SecondTblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    DOSD_Tbl_2_t *TblDataPtr = (DOSD_Tbl_2_t *)TblData;
    
    if (TblDataPtr->TblElement3 > DOSD_TBL_ELEMENT_3_MAX)
    {
        /* Third element is out of range, return an appropriate error code */
        ReturnCode = DOSD_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    

    return ReturnCode;
}

/************************/
/*  End of File Comment */
/************************/
