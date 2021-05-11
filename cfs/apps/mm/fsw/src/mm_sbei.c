/*************************************************************************
** File:
**   $Id: mm_sbei.c
**
** Purpose: 
**   Functions used to inject the Single Bit Error Anomaly
**
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_app.h"
#include "mm_events.h"
#include "cfs_utils.h"
#include "mm_sbei.h"
#include <string.h>
#include <time.h>

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Single Bit Error Inject Command                                 */
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

   MM_SBEI_Flip(ByteValue, DestAddress);

   return;

} /* end MM_SBEI_InjectCmd */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Flips a random bit in the SBEI_AppData                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_SBEI_Flip(uint8 ByteValue, uint32 DestAddress){
   //Initialize flip variable   
   uint8 flip = 0;

   //Variable showing which bit is flipped
   int bit = 0;

   //Initialize the rand()
   srand(time(0));


   //Switch statement for which bit 1-8 to flip
   switch(rand() % 8){
      case 0:
         flip = 1;
         ByteValue ^= flip;
         bit = 0;
         break;
      case 1:
         flip = 2;
         ByteValue ^= flip;
         bit = 1;
         break;      
      case 2:
         flip = 4;
         ByteValue ^= flip;
         bit = 2;
         break;
      case 3:
         flip = 8;
         ByteValue ^= flip;
         bit = 3;
         break;
      case 4:
         flip = 16;
         ByteValue ^= flip;
         bit = 4;
         break;
      case 5:
         flip = 32;
         ByteValue ^= flip;
         bit = 5;
         break;
      case 6: 
         flip = 64;
         ByteValue ^= flip;
         bit = 6;
         break;
      default :
         flip = 128;
         ByteValue ^= flip;
         bit = 7;
   }

   //Write the value into the SBEI_AppData
   CFE_PSP_MemWrite8(DestAddress, ByteValue);

   //Print out Value found in SBEI_AppData
   CFE_EVS_SendEvent(MM_SBEI_EID, CFE_EVS_INFORMATION,
                     "SBEI COMMAND: Bit Flipped = %d Data = %X", bit,  ByteValue);

   return;
   
} /* end MM_SBEI_Convert */

/************************/
/*  End of File Comment */
/************************/