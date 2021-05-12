/*

	NOTES:

	For Invalid Command Sequences (ICS's), I had a function called ICS_Detect()
	This function basically looks into the command log bin and creates an array of executed commands.
	Then the function calls other subfunctions, the Invalid Command Sequence methods.
	If the subfunction detects an ICS, it messages saying that an invalid command sequence has been detected.

	Known Issues (sorry):
	+ The code is very messy. Trust me, the one I was working on in VirtualBox is even messier than this.
	+ The current directory of OSK, for me while I was working on this:
		/home/grandpa/OpenSatKit-master/
	Back then, I wanted it where the OSK can detect where the directory is located, since I knew directories vary from users.
	Unfortunately, I couldn't figure it out, so the directory of OSK is hardcoded into what my directory is.
	+ Only 3 Invalid Command Sequence methods. A small amount. Finding out other kinds of ICS's was tricky.
	+ There may be loopholes to avoiding ICS's that the methods weren't able to catch. It was too complex to try to figure them out.
	+ Complex ways of executing commands (like executing FM commands in the middle of MD commands with a MM command in between), might crash the system.

	example_app.c was developed from the Create App command from OSK.
	The rest were developed afterwards.

	For reference, they are:

		void EXAMPLE_IcsDetectCmd(CFE_SB_MsgPtr_t msg)
		int ICS_EXAMPLE_Steps(int charCount, int arrSize, char cmds2DArr[arrSize][charCount])
		int ICS_DisabledApp_CmdExecuted(int charCount, int arrSize, char cmds2DArr[arrSize][charCount], char logPath[200])
		int ICS_MM_PokePeek(int charCount, int arrSize, char cmds2DArr[arrSize][charCount])

	and

		case EXAMPLE_ICSDETECT_CC:
			EXAMPLE_IcsDetectCmd(msg);
			break;
		from "void EXAMPLE_AppPipe(CFE_SB_MsgPtr_t msg)"

*/

/*******************************************************************************
** File: example_app.c
**
** Purpose:
**   This file contains the source code for the Example App.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "example_app.h"
#include "example_app_perfids.h"
#include "example_app_msgids.h"


/*
** System Header files
*/
#include <string.h>

/*
** EXAMPLE global data
*/
EXAMPLE_AppData_t EXAMPLE_AppData;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_AppMain() -- Application entry point and main process loop   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EXAMPLE_AppMain(void)
{
    int32 Status;
    
    /*
    ** Register the Application with Executive Services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(EXAMPLE_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Intialization fails, set the RunStatus to CFE_ES_APP_ERROR
    ** and the App will not enter the RunLoop.
    */
    Status = EXAMPLE_AppInit();
    if ( Status != CFE_SUCCESS )
    {
        EXAMPLE_AppData.RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** Application Main Loop. Call CFE_ES_RunLoop to check for changes
    ** in the Application's status. If there is a request to kill this
    ** App, it will be passed in through the RunLoop call.
    */
    while ( CFE_ES_RunLoop(&EXAMPLE_AppData.RunStatus) == TRUE )
    {
    
        /*
        ** Performance Log Exit Stamp.
        */
        CFE_ES_PerfLogExit(EXAMPLE_APPMAIN_PERF_ID);
   
        /*
        ** Pend on the arrival of the next Software Bus message.
        */
        Status = CFE_SB_RcvMsg(&EXAMPLE_AppData.MsgPtr, EXAMPLE_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /*
        ** Performance Log Entry Stamp.
        */      
        CFE_ES_PerfLogEntry(EXAMPLE_APPMAIN_PERF_ID);

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
            EXAMPLE_AppPipe(EXAMPLE_AppData.MsgPtr);
            
            /* 
            ** Update Critical Data Store. Because this data is only updated
            ** in one command, this could be moved the command processing function.
            */
            CFE_ES_CopyToCDS(EXAMPLE_AppData.CDSHandle, &EXAMPLE_AppData.WorkingCriticalData);

        }
        else
        {
            /*
            ** This is an example of exiting on an error.
            ** Note that a SB read error is not always going to 
            ** result in an app quitting. 
            */
            CFE_EVS_SendEvent(EXAMPLE_PIPE_ERR_EID, CFE_EVS_ERROR,
                               "EXAMPLE: SB Pipe Read Error, EXAMPLE App Will Exit.");
            
            EXAMPLE_AppData.RunStatus = CFE_ES_APP_ERROR;
         }
                    
    } /* end while */

    /*
    ** Performance Log Exit Stamp.
    */
    CFE_ES_PerfLogExit(EXAMPLE_APPMAIN_PERF_ID);

    /*
    ** Exit the Application.
    */
    CFE_ES_ExitApp(EXAMPLE_AppData.RunStatus);
    
} /* End of EXAMPLE_AppMain() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_AppInit() -- EXAMPLE initialization                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 EXAMPLE_AppInit(void)
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
       EXAMPLE_AppData.WorkingCriticalData.DataPtOne   = 1;
       EXAMPLE_AppData.WorkingCriticalData.DataPtTwo   = 2;
       EXAMPLE_AppData.WorkingCriticalData.DataPtThree = 3;
       EXAMPLE_AppData.WorkingCriticalData.DataPtFour  = 4;
       EXAMPLE_AppData.WorkingCriticalData.DataPtFive  = 5;
    }
    
    /*
    ** Setup the RunStatus variable
    */
    EXAMPLE_AppData.RunStatus = CFE_ES_APP_RUN;
    
    /*
    ** Initialize app command execution counters.
    */
    EXAMPLE_AppData.CmdCounter = 0;
    EXAMPLE_AppData.ErrCounter = 0;

    /*
    ** Initialize app configuration data.
    */
    strcpy(EXAMPLE_AppData.PipeName, "EXAMPLE_CMD_PIPE");
    EXAMPLE_AppData.PipeDepth = EXAMPLE_PIPE_DEPTH;

    EXAMPLE_AppData.LimitHK   = EXAMPLE_LIMIT_HK;
    EXAMPLE_AppData.LimitCmd  = EXAMPLE_LIMIT_CMD;

    /*
    ** Initialize event filter table for envents we want to filter.
    */
    EXAMPLE_AppData.EventFilters[0].EventID = EXAMPLE_PROCESS_INF_EID;
    EXAMPLE_AppData.EventFilters[0].Mask    = CFE_EVS_EVERY_FOURTH_ONE;
    
    EXAMPLE_AppData.EventFilters[1].EventID = EXAMPLE_RESET_INF_EID;
    EXAMPLE_AppData.EventFilters[1].Mask    = CFE_EVS_NO_FILTER;
    
    EXAMPLE_AppData.EventFilters[2].EventID = EXAMPLE_CC1_ERR_EID;
    EXAMPLE_AppData.EventFilters[2].Mask    = CFE_EVS_EVERY_OTHER_TWO;
    
    EXAMPLE_AppData.EventFilters[3].EventID = EXAMPLE_LEN_ERR_EID;
    EXAMPLE_AppData.EventFilters[3].Mask    = CFE_EVS_FIRST_8_STOP;


    /*
    ** Register event filter table.
    */
    Status = CFE_EVS_Register(EXAMPLE_AppData.EventFilters, 4,
                               CFE_EVS_BINARY_FILTER);
    
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("EXAMPLE App: Error Registering Events, RC = 0x%08X\n", Status);
       return ( Status );
    }
             
    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_SB_InitMsg(&EXAMPLE_AppData.HkPacket, EXAMPLE_HK_TLM_MID, sizeof(EXAMPLE_HkPacket_t), TRUE);
   
    /*
    ** Create Software Bus message pipe.
    */
    Status = CFE_SB_CreatePipe(&EXAMPLE_AppData.CmdPipe, EXAMPLE_AppData.PipeDepth, EXAMPLE_AppData.PipeName);
    if ( Status != CFE_SUCCESS )
    {
       /*
       ** Could use an event at this point
       */
       CFE_ES_WriteToSysLog("EXAMPLE App: Error Creating SB Pipe, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    Status = CFE_SB_Subscribe(EXAMPLE_SEND_HK_MID,EXAMPLE_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("EXAMPLE App: Error Subscribing to HK Request, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Subscribe to EXAMPLE ground command packets
    */
    Status = CFE_SB_Subscribe(EXAMPLE_CMD_MID,EXAMPLE_AppData.CmdPipe);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("EXAMPLE App: Error Subscribing to EXAMPLE Command, RC = 0x%08X\n", Status);
       return ( Status );
    }

    /*
    ** Register tables with cFE and load default data
    */
    Status = CFE_TBL_Register(&EXAMPLE_AppData.TblHandles[0], "MyFirstTbl",
                              sizeof(EXAMPLE_Tbl_1_t), CFE_TBL_OPT_DEFAULT,
                              EXAMPLE_FirstTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("EXAMPLE App: Error Registering Table 1, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
       Status = CFE_TBL_Load(EXAMPLE_AppData.TblHandles[0], CFE_TBL_SRC_FILE, EXAMPLE_FIRST_TBL_FILE);
    }
    
    Status = CFE_TBL_Register(&EXAMPLE_AppData.TblHandles[1], "MySecondTbl",
                              sizeof(EXAMPLE_Tbl_2_t), CFE_TBL_OPT_DEFAULT,
                              EXAMPLE_SecondTblValidationFunc);
    if ( Status != CFE_SUCCESS )
    {
       CFE_ES_WriteToSysLog("EXAMPLE App: Error Registering Table 2, RC = 0x%08X\n", Status);
       return ( Status );
    }
    else
    {
      Status = CFE_TBL_Load(EXAMPLE_AppData.TblHandles[1], CFE_TBL_SRC_FILE, EXAMPLE_SECOND_TBL_FILE);
    }
                 

    /*
    ** Create and manage the Critical Data Store 
    */
    Status = CFE_ES_RegisterCDS(&EXAMPLE_AppData.CDSHandle, sizeof(EXAMPLE_CdsDataType_t), EXAMPLE_CDS_NAME);
    if(Status == CFE_SUCCESS)
    {
       /* 
       ** Setup Initial contents of Critical Data Store 
       */
       CFE_ES_CopyToCDS(EXAMPLE_AppData.CDSHandle, &EXAMPLE_AppData.WorkingCriticalData);
       
    }
    else if(Status == CFE_ES_CDS_ALREADY_EXISTS)
    {
       /* 
       ** Critical Data Store already existed, we need to get a copy 
       ** of its current contents to see if we can use it
       */
       Status = CFE_ES_RestoreFromCDS(&EXAMPLE_AppData.WorkingCriticalData, EXAMPLE_AppData.CDSHandle);
       if(Status == CFE_SUCCESS)
       {
          /*
          ** Perform any logical verifications, if necessary, to validate data 
          */
          CFE_ES_WriteToSysLog("EXAMPLE App CDS data preserved\n");
       }
       else
       {
          /* 
          ** Restore Failied, Perform baseline initialization 
          */
          EXAMPLE_AppData.WorkingCriticalData.DataPtOne   = 1;
          EXAMPLE_AppData.WorkingCriticalData.DataPtTwo   = 2;
          EXAMPLE_AppData.WorkingCriticalData.DataPtThree = 3;
          EXAMPLE_AppData.WorkingCriticalData.DataPtFour  = 4;
          EXAMPLE_AppData.WorkingCriticalData.DataPtFive  = 5;
          CFE_ES_WriteToSysLog("Failed to Restore CDS. Re-Initialized CDS Data.\n");
       }
    }
    else 
    {
       /* 
       ** Error creating my critical data store 
       */
       CFE_ES_WriteToSysLog("EXAMPLE: Failed to create CDS (Err=0x%08x)", Status);
       return(Status);
    }

    /*
    ** Application startup event message.
    */
    CFE_EVS_SendEvent(EXAMPLE_INIT_INF_EID,CFE_EVS_INFORMATION, "EXAMPLE: Application Initialized");
                         
    return(CFE_SUCCESS);

} /* End of EXAMPLE_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_AppPipe() -- Process command pipe message                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EXAMPLE_AppPipe(CFE_SB_MsgPtr_t msg)
{
    CFE_SB_MsgId_t MessageID;
    uint16 CommandCode;

    MessageID = CFE_SB_GetMsgId(msg);
    switch (MessageID)
    {
        /*
        ** Housekeeping telemetry request.
        */
        case EXAMPLE_SEND_HK_MID:
            EXAMPLE_HousekeepingCmd(msg);
            break;

        /*
        ** EXAMPLE ground commands.
        */
        case EXAMPLE_CMD_MID:

            CommandCode = CFE_SB_GetCmdCode(msg);
            switch (CommandCode)
            {
                case EXAMPLE_NOOP_CC:
                    EXAMPLE_NoopCmd(msg);
                    break;

                case EXAMPLE_RESET_CC:
                    EXAMPLE_ResetCmd(msg);
                    break;
                    
                case EXAMPLE_PROCESS_CC:
                    EXAMPLE_RoutineProcessingCmd(msg);
                    break;

                case EXAMPLE_ICSDETECT_CC:
                    EXAMPLE_IcsDetectCmd(msg);
                    break;

                default:
                    CFE_EVS_SendEvent(EXAMPLE_CC1_ERR_EID, CFE_EVS_ERROR,
                     "Invalid ground command code: ID = 0x%X, CC = %d",
                                      MessageID, CommandCode);
                    break;
            }
            break;

        default:

            CFE_EVS_SendEvent(EXAMPLE_MID_ERR_EID, CFE_EVS_ERROR,
                             "Invalid command pipe message ID: 0x%X",
                              MessageID);
            break;
    }

    return;

} /* End of EXAMPLE_AppPipe() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_HousekeepingCmd() -- On-board command (HK request)           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EXAMPLE_HousekeepingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(EXAMPLE_NoArgsCmd_t);
    uint16 i;

    /*
    ** Verify command packet length
    */
    if (EXAMPLE_VerifyCmdLength(msg, ExpectedLength))
    {
        /*
        ** Get command execution counters
        */
        EXAMPLE_AppData.HkPacket.CmdCounter = EXAMPLE_AppData.CmdCounter;
        EXAMPLE_AppData.HkPacket.ErrCounter = EXAMPLE_AppData.ErrCounter;

        /*
        ** Send housekeeping telemetry packet
        */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &EXAMPLE_AppData.HkPacket);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &EXAMPLE_AppData.HkPacket);

        /*
        ** Manage any pending table loads, validations, etc.
        */
        for (i=0; i<EXAMPLE_NUM_TABLES; i++)
        {
            CFE_TBL_Manage(EXAMPLE_AppData.TblHandles[i]);
        }
        
        /*
        ** This command does not affect the command execution counter
        */
        
    } /* end if */

    return;

} /* End of EXAMPLE_HousekeepingCmd() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_NoopCmd() -- EXAMPLE ground command (NO-OP)                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EXAMPLE_NoopCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(EXAMPLE_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (EXAMPLE_VerifyCmdLength(msg, ExpectedLength))
    {
        EXAMPLE_AppData.CmdCounter++;

        CFE_EVS_SendEvent(EXAMPLE_NOOP_INF_EID, CFE_EVS_INFORMATION,
                         "EXAMPLE Version 1.0.0: No-op command");
    }

    return;

} /* End of EXAMPLE_NoopCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_ResetCmd() -- EXAMPLE ground command (reset counters)             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EXAMPLE_ResetCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(EXAMPLE_NoArgsCmd_t);

    /*
    ** Verify command packet length...
    */
    if (EXAMPLE_VerifyCmdLength(msg, ExpectedLength))
    {
        EXAMPLE_AppData.CmdCounter = 0;
        EXAMPLE_AppData.ErrCounter = 0;

        CFE_EVS_SendEvent(EXAMPLE_RESET_INF_EID, CFE_EVS_INFORMATION,
                         "EXAMPLE: Reset Counters command");
    }

    return;

} /* End of EXAMPLE_ResetCmd() */

/* ********************************************************************************************************** */
/* EXAMPLE_IcsDetectCmd() - Check the logs and see for anything related to sequences that is invalid as well. */
/* ********************************************************************************************************** */

void EXAMPLE_IcsDetectCmd(CFE_SB_MsgPtr_t msg)
{

/* First, verify the packet length! */

EXAMPLE_IcsDetectCmd_t *idMsg = (EXAMPLE_IcsDetectCmd_t*) msg;
if (EXAMPLE_VerifyCmdLength(msg, sizeof(EXAMPLE_IcsDetectCmd_t)))
{

	EXAMPLE_AppData.CmdCounter++;

	/*
		What kind of command log am I looking for?
		WIP: Select if you want the latest cmd log or a specific cmd log generated in COSMOS.
	*/

	char * detectMethod0;
	char detectMethod[100];
	detectMethod0 = idMsg->DetectType;
	strcpy(detectMethod, detectMethod0);

	char * logInput0;
	char logInput[100];
	logInput0 = idMsg->CmdLogFilename;
	strcpy(logInput, logInput0);

	char cmdLine[50] = "filename=";

	if ( strcmp(detectMethod, "SPECIFIC") == 0 )
	{

		/* Find specific filename log */
		char cmdLineB[100] = "$(find /home/grandpa/OpenSatKit-master/cosmos/outputs/logs -iname \"";
		strcat(cmdLineB, logInput);
		char cmdLineB2[100] = "\"); ";
		strcat(cmdLineB, cmdLineB2);
		strcat(cmdLine, cmdLineB); // Detect Type = Find specific filename log

	}
	else
	{

		/* Find recent nonempty log */
		char cmdLineA[500] = "$(find /home/grandpa/OpenSatKit-master/cosmos/outputs/logs -iname \"*_cmd.bin\" -size +0b -type f -printf \"%f\n\" | sort -r | head -n 1); ";
		strcat(cmdLine, cmdLineA); // Detect Type = Find most recent nonempty log

	}

	char cmdLine2[500] = "echo \"Anaylzing: $filename\"; cd /home/grandpa/OpenSatKit-master/cosmos/outputs/logs; logContents=$(strings -n 1 $filename); ";
	char cmdLine3[500] = "cd /home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf; echo $logContents > ics_bin.txt; ";

	strcat(cmdLine, cmdLine2);
	strcat(cmdLine, cmdLine3);

	/* Obtain the command log file! */
	system(cmdLine);



	/*
		With the command log obtained, now create an array.
		This array will contain the ordered list of executed commands.
		But first, find the executed commands!
	*/

	FILE * textFile;
	
	// ics_bin.txt == "A text file version of the command log, which was originally in bin format."
	char textPath[200] = "/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/ics_bin.txt";
	
	// ics_cmdlog.txt == "A simplified version of ics_bin.txt. Used to create arrays."
	char logPath[200] = "/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/ics_cmdlog.txt";

	/* Debug Text File, should the system crash, the debug text file will notify when was the last action of the function before the system crashed. */

	FILE * debugFile1;
	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Obtained the neccessary command log.", debugFile1);
	fclose(debugFile1);



	/* Count the number of characters the file has!!! */

	CFE_EVS_SendEvent(EXAMPLE_ICSDETECT_INF_EID, CFE_EVS_INFORMATION, "Creating a new txt file: ics_bin.txt");
	
	textFile = fopen(textPath, "r");
	int charCount = 0;
	char ch;

	for( ch = getc(textFile) ; ch != EOF ; ch = getc(textFile) )
		charCount++;

	fclose(textFile);

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Obtained number of characters from log.", debugFile1);
	fclose(debugFile1);



	/* Find the start of the "executed command" history! */

	textFile = fopen(textPath, "r");
	char initChar[charCount];
	int startIndex = 0;
	int numOfLines = 0;
	boolean endProcess = false;

	while ( fscanf(textFile, " %s", initChar) == 1 )
	{

		numOfLines++;
		puts(initChar);

		if ( (strcmp(initChar, "OpenSatKit") == 0) && (endProcess == false) ) {
			startIndex = numOfLines;
			endProcess = true;
		}

	}

	startIndex = startIndex + 2;

	fclose(textFile);

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Obtained number of lines from log.", debugFile1);
	fclose(debugFile1);



	/* When do the separators pop up? */

	textFile = fopen(textPath, "r");

	char lineText2[charCount];
	int lineIndex2 = 0;
	int commandCount = 0;

	int sepArr[numOfLines];

	while ( fscanf(textFile, " %s", lineText2) == 1 )
	{

		lineIndex2++;

		sepArr[lineIndex2] = 0;

		puts(lineText2);

		char firstLetter[2] = "";
		strncpy(firstLetter, lineText2, 1);

		if ( strcmp(firstLetter, "`") == 0 )
		{

			commandCount++;
			sepArr[lineIndex2] = lineIndex2;

		}

	}

	fclose(textFile);

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Obtained locations of separators from log.", debugFile1);
	fclose(debugFile1);



	/* Put the command history into a string: cmdStr */

	textFile = fopen(textPath, "r");
	char cmdChar[charCount];
	int startCat = 0;
	char cmdStr[charCount];
	strcat(cmdStr, " ");

	char separatorChar[10] = "$$$$$";

	int cmdStrIndex = 0;

	while ( fscanf(textFile, " %s", cmdChar) == 1 ) {

		startCat++;
		cmdStrIndex++;

		if (startCat > startIndex)
		{

			puts(cmdChar);

			if ( sepArr[cmdStrIndex] != 0 )
			{
				//current line is a separator
				char cmdStrCat1[200] = " ";
				strcat(cmdStrCat1, separatorChar);
				strcat(cmdStr, cmdStrCat1);
			}
			else
			{
				//current line is NOT a separator
				char cmdStrCat2[200] = " ";
				strcat(cmdStrCat2, cmdChar);
				strcat(cmdStr, cmdStrCat2);
			}

		}

	}

	fclose(textFile);

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Converted log into string.", debugFile1);
	fclose(debugFile1);



	/* Now paste the contents of cmdStr into ics_cmdlog.txt! */

	CFE_EVS_SendEvent(EXAMPLE_ICSDETECT_INF_EID, CFE_EVS_INFORMATION, "Creating a new txt file: ics_cmdlog.txt");
	textFile = fopen(logPath, "w+");
	fputs(cmdStr, textFile);
	fclose(textFile);

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Pasted string into a new text file.", debugFile1);
	fclose(debugFile1);



	/* Convert the command history into a 2D array, using ics_cmdlog.txt */

	textFile = fopen(logPath, "r");

	int arrSize = commandCount-1;
	char str2DArr[arrSize][charCount];
	int str2DArrIndex = 0;

	char arrString[charCount];
	boolean firstTime = TRUE;

	char scannedStr[charCount];
	while ( fscanf(textFile, " %s", scannedStr) == 1 )
	{

		puts(scannedStr);

		// Current word is a seperator
		if ( strcmp(scannedStr, separatorChar) == 0 )
		{

			if (firstTime == FALSE)
			{
				strcpy(str2DArr[str2DArrIndex], arrString);
				str2DArrIndex++;
				strcpy(arrString, " ");
			}
			if (firstTime == TRUE)
			{
				strcpy(arrString, " ");
				firstTime = FALSE;
			}

		}

		// Current word is something besides a seperator
		if ( ( strcmp(scannedStr, separatorChar) != 0 ) && (firstTime == FALSE) )
		{
			char stringCat[charCount];
			strcpy(stringCat, " ");
			strcat(stringCat, scannedStr);
			strcat(arrString, stringCat);
		}

	}

	// Final string concatenation
	strcpy(str2DArr[str2DArrIndex], arrString);

	fclose(textFile);

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Converted text file into 2D Array.", debugFile1);
	fclose(debugFile1);



	/* Now make a simplified version of that mentioned 2D array */

	char scanText[charCount];
	char *token;

	char cmds2DArr[arrSize][charCount];
	int cmds2DArrIndex = 0;
	char cmds2DArrText[charCount];
	boolean appLocated = false;
	boolean inProgress = true;

	for ( int i = 0 ; i < arrSize ; i++ )
	{

		strcpy(scanText, str2DArr[i]);

		token = strtok(scanText, " ");

		while ( token != NULL )
		{

			// Need to find a better way to detect the app names!
			if
			(
				(
				(strcmp(token, "BM") == 0) || (strcmp(token, "CF") == 0) || (strcmp(token, "CS") == 0) || (strcmp(token, "DS") == 0) || 
				(strcmp(token, "EXAMPLE") == 0) || (strcmp(token, "F42") == 0) || (strcmp(token, "FM") == 0) || (strcmp(token, "I42") == 0) || 
				(strcmp(token, "ISIM") == 0) || (strcmp(token, "KIT_CI") == 0) || (strcmp(token, "KIT_SCH") == 0) || (strcmp(token, "KIT_TO") == 0) || 
				(strcmp(token, "LC") == 0) || (strcmp(token, "MD") == 0) || (strcmp(token, "MM") == 0) || (strcmp(token, "SC") == 0) || 
				(strcmp(token, "HK") == 0) || (strcmp(token, "TFTP") == 0) || (strcmp(token, "COSMOS") == 0) || (strcmp(token, "CFE_ES") == 0) || 
				(strcmp(token, "CFE_EVS") == 0) || (strcmp(token, "CFE_SB") == 0) || (strcmp(token, "CFE_TBL") == 0) || (strcmp(token, "CFE_TIME") == 0) || 
				(strcmp(token, "SYSTEM") == 0)
				)
				&& (inProgress)
			)
			{
				strcpy(cmds2DArr[cmds2DArrIndex], " ");
				strcpy(cmds2DArr[cmds2DArrIndex], token);
				strcat(cmds2DArr[cmds2DArrIndex], "-");
				appLocated = true;
			}

			token = strtok(NULL, " ");

			if (appLocated)
			{
				strcat(cmds2DArr[cmds2DArrIndex], token);
				cmds2DArrIndex++;
				appLocated = false;
				inProgress = false;
			}

		}

		inProgress = true;

	}

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Simplified the 2D Array.", debugFile1);
	fclose(debugFile1);





	/* Invalid Command Sequence Detection Method (for EXAMPLE app's step 1 to step 4 sequence order) */

	/*
	return value is an int, which is either 1, 0, or -1
		1 == Valid Command Sequence,
		0 == INVALID COMMAND SEQUENCE,
		-1 == Command Sequence Not Executed At All
	In hindsight, a boolean (true = no ICS, false = yes ICS) would be useful.
	But use int in the possible emergency where I need to notify the user 
	that the sequence wasn't executed to begin with, meaning no ICS detected there by default.
	*/

	/* EXAMPLE app has 4 commands that have to be executed in the right order! */
  /*
	int ics_example_steps = -1;
	ics_example_steps = ICS_EXAMPLE_Steps(charCount, arrSize, cmds2DArr);
	
	if (ics_example_steps == 0)
		CFE_EVS_SendEvent(EXAMPLE_ICSDETECT_INF_EID, CFE_EVS_INFORMATION, "Invalid Command Sequence Detected: EXAMPLE Steps cmds 1 to 4 not executed in the right order.");
  */



	/* Commands executed must come from enabled apps, not disabled ones! */
	int ics_disabledCmds = -1;
	ics_disabledCmds = ICS_DisabledApp_CmdExecuted(charCount, arrSize, cmds2DArr, logPath);
	
	if (ics_disabledCmds == 0)
		CFE_EVS_SendEvent(EXAMPLE_ICSDETECT_INF_EID, CFE_EVS_INFORMATION, "Invalid Command Sequence Detected: Commands executed for disabled apps.");



	/* Memory Manager has two cmds: Poke and Peek. Make sure the order is: Poke > Peek */
	int ics_pokepeek = -1;
	ics_pokepeek = ICS_MM_PokePeek(charCount, arrSize, cmds2DArr);
	
	if (ics_pokepeek == 0)
		CFE_EVS_SendEvent(EXAMPLE_ICSDETECT_INF_EID, CFE_EVS_INFORMATION, "Invalid Command Sequence Detected: MM should have Poke executed before Peek.");

	debugFile1 = fopen("/home/grandpa/OpenSatKit-master/cfs/build/exe/cpu1/cf/debug_ics.txt", "w+");
	fputs("Last Action: Ran ICS Detection Methods, function was fully executed without any runtime errors!", debugFile1);
	fclose(debugFile1);

}

return;

} /* End of EXAMPLE_IcsDetectCmd() */





/*
	My plan is to have multiple functions.
	Each one relates to finding a specific ICS to detect.
	As of right now, I have only 3 ICS detection methods:
	1) The 4 Step commands from Example app are executed in the wrong sequence order.
	2) Commands from disabled apps are executed.
	3) Peek from Memory Manager is executed before Poke from Memory Manager.
*/



/* ********************************************************************************************* */
/* ICS_EXAMPLE_Steps() - EXAMPLE app has 4 commands that have to be executed in the right order! */
/* ********************************************************************************************* */
/*
int ICS_EXAMPLE_Steps(int charCount, int arrSize, char cmds2DArr[arrSize][charCount])
{

	// Did I execute the command sequence at all?

	char scanCmds1[charCount];
	boolean step1Done = false;
	boolean step2Done = false;
	boolean step3Done = false;
	boolean step4Done = false;

	for ( int i = 0 ; i < arrSize ; i++ )
	{
		strcpy(scanCmds1, cmds2DArr[i]);
		if (strcmp(scanCmds1, "EXAMPLE-STEP_ONE") == 0)
			step1Done = true;
		if (strcmp(scanCmds1, "EXAMPLE-STEP_TWO") == 0)
			step2Done = true;
		if (strcmp(scanCmds1, "EXAMPLE-STEP_THREE") == 0)
			step3Done = true;
		if (strcmp(scanCmds1, "EXAMPLE-STEP_FOUR") == 0)
			step4Done = true;
	}

	if ( !( (step1Done) && (step2Done) && (step3Done) && (step4Done) ) )
		return -1;

	// Create an array based on the order of the steps executed!

	char stepsInputSeq[4][20];
	int stepsInputSeqIndex = 0;

	char scanCmds[charCount];
	for ( int i = 0 ; i < arrSize ; i++ )
	{

		strcpy(scanCmds, cmds2DArr[i]);

		if
		(
			(strcmp(scanCmds, "EXAMPLE-STEP_ONE") == 0) || (strcmp(scanCmds, "EXAMPLE-STEP_TWO") == 0) ||
			(strcmp(scanCmds, "EXAMPLE-STEP_THREE") == 0) || (strcmp(scanCmds, "EXAMPLE-STEP_FOUR") == 0)
		)
		{
			strcpy(stepsInputSeq[stepsInputSeqIndex], scanCmds);
			stepsInputSeqIndex++;
		}

	}

	// Is the array based on the order of the steps executed CORRECT?!

	char stepsCorrectSeq[4][20] =
	{ "EXAMPLE-STEP_ONE", "EXAMPLE-STEP_TWO", "EXAMPLE-STEP_THREE", "EXAMPLE-STEP_FOUR" };

	boolean allSame = true;

	for (int i=0 ; i<4 ; i++)
	{
		if ( strcmp(stepsCorrectSeq[i], stepsInputSeq[i]) != 0 )
		allSame = false;
	}

	if (allSame)
		return 1;
	else
		return 0;

} // End of ICS_EXAMPLE_Steps()
*/


/* ************************************************************************************************* */
/* ICS_DisabledApp_CmdExecuted() - Commands executed must come from enabled apps, not disabled ones! */
/* ************************************************************************************************* */

int ICS_DisabledApp_CmdExecuted(int charCount, int arrSize, char cmds2DArr[arrSize][charCount], char logPath[200])
{

	/* Did I execute the command sequence at all? */

	char scanCmds1[charCount];
	boolean stopAppDone = false;
	boolean startAppDone = false;

	for ( int i = 0 ; i < arrSize ; i++ )
	{
		strcpy(scanCmds1, cmds2DArr[i]);
		if (strcmp(scanCmds1, "CFE_ES-STOP_APP") == 0)
			stopAppDone = true;
		if (strcmp(scanCmds1, "CFE_ES-START_APP") == 0)
			startAppDone = true;
	}

	if ( !( (stopAppDone) && (startAppDone) ) )
		return -1;

	/* Find instances of apps affected by STOP_APP or START_APP */

	FILE * cmdLog;
	cmdLog = fopen(logPath, "r");

	char token[charCount];
	char stoppedApp[50];
	boolean stopAppDetect = false;
	boolean startAppDetect = false;
	boolean valid_sequence = true;

	while ( fscanf(cmdLog, " %s", token) == 1 )
	{

		puts(token);

		if ( strcmp(token, "STOP_APP") == 0 )
			stopAppDetect = true;

		if ( strcmp(token, "START_APP") == 0 )
			startAppDetect = true;

		if
		(
			(strcmp(token, "BM") == 0) || (strcmp(token, "CF") == 0) || (strcmp(token, "CS") == 0) || (strcmp(token, "DS") == 0) || 
			(strcmp(token, "EXAMPLE") == 0) || (strcmp(token, "F42") == 0) || (strcmp(token, "FM") == 0) || (strcmp(token, "I42") == 0) || 
			(strcmp(token, "ISIM") == 0) || (strcmp(token, "KIT_CI") == 0) || (strcmp(token, "KIT_SCH") == 0) || (strcmp(token, "KIT_TO") == 0) || 
			(strcmp(token, "LC") == 0) || (strcmp(token, "MD") == 0) || (strcmp(token, "MM") == 0) || (strcmp(token, "SC") == 0) || 
			(strcmp(token, "HK") == 0) || (strcmp(token, "TFTP") == 0) || (strcmp(token, "COSMOS") == 0) || (strcmp(token, "CFE_ES") == 0) || 
			(strcmp(token, "CFE_EVS") == 0) || (strcmp(token, "CFE_SB") == 0) || (strcmp(token, "CFE_TBL") == 0) || (strcmp(token, "CFE_TIME") == 0) || 
			(strcmp(token, "SYSTEM") == 0)
		)
		{

			if ( (strcmp(token, stoppedApp) == 0) && (!startAppDetect) )
				valid_sequence = false;

			if (stopAppDetect)
			{
				strcpy(stoppedApp, token);
				stopAppDetect = false;
			}

			if (startAppDetect)
			{
				strcpy(stoppedApp, " ");
				startAppDetect = false;
			}

		}

	}

	fclose(cmdLog);

	if (valid_sequence)
		return 1;
	else
		return 0;

} /* End of ICS_DisabledApp_CmdExecuted() */



/* **************************************************************************************************** */
/* ICS_MM_PokePeek() - Memory Manager has two cmds: Poke and Peek. Make sure the order is: Poke > Peek. */
/* **************************************************************************************************** */

int ICS_MM_PokePeek(int charCount, int arrSize, char cmds2DArr[arrSize][charCount])
{

	/* Did I execute the command sequence at all? */

	char scanCmds1[charCount];
	boolean pokeDone = false;
	boolean peekDone = false;

	for ( int i = 0 ; i < arrSize ; i++ )
	{
		strcpy(scanCmds1, cmds2DArr[i]);
		if (strcmp(scanCmds1, "MM-POKE_MEM") == 0)
			pokeDone = true;
		if (strcmp(scanCmds1, "MM-PEEK_MEM") == 0)
			peekDone = true;
	}

	if ( !( (pokeDone) && (peekDone) ) )
		return -1;

	/* Create an array based on the order of the steps executed! */

	char mmSeq[2][15];
	int mmSeqIndex = 0;

	char scanCmds[charCount];
	for ( int i = 0 ; i < arrSize ; i++ )
	{

		strcpy(scanCmds, cmds2DArr[i]);

		if
		(
			(strcmp(scanCmds, "MM-POKE_MEM") == 0) || (strcmp(scanCmds, "MM-PEEK_MEM") == 0)
		)
		{
			strcpy(mmSeq[mmSeqIndex], scanCmds);
			mmSeqIndex++;
		}

	}

	/* Is the array based on the order of the steps executed CORRECT?! */

	char mmCorrectSeq[2][15] = { "MM-POKE_MEM", "MM-PEEK_MEM" };
	boolean validSeq = true;

	for (int i=0 ; i<2 ; i++)
	{
		if ( strcmp(mmCorrectSeq[i], mmSeq[i]) != 0 )
			validSeq = false;
	}

	if (validSeq)
		return 1;
	else
		return 0;

} /* End of ICS_MM_PokePeek() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  EXAMPLE_RoutineProcessingCmd() - Common Processing for each cmd.    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EXAMPLE_RoutineProcessingCmd(CFE_SB_MsgPtr_t msg)
{
    uint16 ExpectedLength = sizeof(EXAMPLE_NoArgsCmd_t);
    EXAMPLE_Tbl_1_t   *MyFirstTblPtr;
    EXAMPLE_Tbl_2_t   *MySecondTblPtr;

    /*
    ** Verify command packet length
    */
    if (EXAMPLE_VerifyCmdLength(msg, ExpectedLength))
    {
        /* Obtain access to table data addresses */
        CFE_TBL_GetAddress((void *)&MyFirstTblPtr, EXAMPLE_AppData.TblHandles[0]);
        CFE_TBL_GetAddress((void *)&MySecondTblPtr, EXAMPLE_AppData.TblHandles[1]);
        
        /* Perform routine processing accessing table data via pointers */
        /*                            .                                 */
        /*                            .                                 */
        /*                            .                                 */
        
        /* Once completed with using tables, release addresses          */
        CFE_TBL_ReleaseAddress(EXAMPLE_AppData.TblHandles[0]);
        CFE_TBL_ReleaseAddress(EXAMPLE_AppData.TblHandles[1]);
        
        /*
        ** Update Critical variables. These variables will be saved
        ** in the Critical Data Store and preserved on a processor reset.
        */
        EXAMPLE_AppData.WorkingCriticalData.DataPtOne++;
        EXAMPLE_AppData.WorkingCriticalData.DataPtTwo++;
        EXAMPLE_AppData.WorkingCriticalData.DataPtThree++;
        EXAMPLE_AppData.WorkingCriticalData.DataPtFour++;
        EXAMPLE_AppData.WorkingCriticalData.DataPtFive++;
        
        CFE_EVS_SendEvent(EXAMPLE_PROCESS_INF_EID,CFE_EVS_INFORMATION, "EXAMPLE: Routine Processing Command");
        
        EXAMPLE_AppData.CmdCounter++;
    }

    return;

} /* End of EXAMPLE_RoutineProcessingCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_VerifyCmdLength() -- Verify command packet length            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

boolean EXAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
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

        CFE_EVS_SendEvent(EXAMPLE_LEN_ERR_EID, CFE_EVS_ERROR,
           "EXAMPLE: Invalid cmd pkt: ID = 0x%X,  CC = %d, Actual Length = %d, Expected Length = %d",
                          MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        EXAMPLE_AppData.ErrCounter++;
    }

    return(result);

} /* End of EXAMPLE_VerifyCmdLength() */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_FirstTblValidationFunc() -- Verify contents of First Table   */
/*                                buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 EXAMPLE_FirstTblValidationFunc(void *TblData)
{
    int32              ReturnCode = CFE_SUCCESS;
    EXAMPLE_Tbl_1_t *TblDataPtr = (EXAMPLE_Tbl_1_t *)TblData;
    
    if (TblDataPtr->TblElement1 > EXAMPLE_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = EXAMPLE_TBL_1_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    
    return ReturnCode;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* EXAMPLE_SecondTblValidationFunc() -- Verify contents of Second Table */
/*                                 buffer contents                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 EXAMPLE_SecondTblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    EXAMPLE_Tbl_2_t *TblDataPtr = (EXAMPLE_Tbl_2_t *)TblData;
    
    if (TblDataPtr->TblElement3 > EXAMPLE_TBL_ELEMENT_3_MAX)
    {
        /* Third element is out of range, return an appropriate error code */
        ReturnCode = EXAMPLE_TBL_2_ELEMENT_OUT_OF_RANGE_ERR_CODE;
    }
    

    return ReturnCode;
}

/************************/
/*  End of File Comment */
/************************/
