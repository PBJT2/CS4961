#include "mm_app.h"
#include "mm_dump.h"
#include "mm_events.h"
#include "mm_mem32.h"
#include "mm_mem16.h"
#include "mm_mem8.h"
#include "mm_utils.h"
#include "mm_mission_cfg.h"
#include "cfs_utils.h"
#include <string.h>
#include "mm_sbei.h"

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* SBEI_INJECT command (Currently: Will + 5 the initial read value)*/
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_SBEI_InjectCmd(CFE_SB_MsgPtr_t msg)
{  

   //Initialize all necessary variables
   uint8 ByteValue = 0;
   uint32 DestAddress;
   CFS_SymAddr_t     DestSymAddress;

   //String of the Dummy Application
   char address[] = "SBEI_AppData";

   //Copy the address and resolve it 
   strcpy(DestSymAddress.SymName, address);
   CFS_ResolveSymAddr(&DestSymAddress, &DestAddress);

   //Read the value of the SBEI_AppData
   CFE_PSP_MemRead8(DestAddress, &ByteValue);

   //Print out Value found in SBEI_AppData
   CFE_EVS_SendEvent(MM_POKE_BYTE_INF_EID, CFE_EVS_INFORMATION,
                     "Address: %X | Value: %X", DestAddress, ByteValue);


   //Add 5 to the current read value
   ByteValue = ByteValue + 5;

   //Write the value into the SBEI_AppData
   CFE_PSP_MemWrite8(DestAddress, ByteValue);

   return;

} /* end MM_SBEI_InjectCmd */

/************************/
/*  End of File Comment */
/************************/