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
/* SBEI_INJECT command                                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_SBEI_InjectCmd(CFE_SB_MsgPtr_t msg)
{
   //Note: Can only access one function at a time or the system crashes

   //Peek
   MM_PeekCmd_t    *PeekPtr;
   PeekPtr = ((MM_PeekCmd_t *)msg);
   // MM_SBEI_Peek(PeekPtr);


   //Poke
   MM_PokeCmd_t *PokePtr;
   PokePtr = ((MM_PokeCmd_t *)msg);
   MM_SBEI_Poke(PokePtr);

   return;

} /* end MM_SBEI_InjectCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Executes Memory Peek on SBEI_AppData for 8 bits of memory       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_SBEI_Peek(MM_PeekCmd_t *PeekPtr){
   uint32 DestAddress;
   char adress[] = "SBEI_AppData";

   //Takes inputted address and resolves it
   strcpy(PeekPtr->SrcSymAddress.SymName, adress);
   CFS_ResolveSymAddr(&(PeekPtr->SrcSymAddress), &DestAddress);

   //Data Sizze of 8 Bits
   PeekPtr->DataSize = 8;
   MM_PeekMem(PeekPtr, DestAddress);

   return;
} /* end MM_SBEI_Peek */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                                           */
/* Executes Memory Poke on SBEI_AppData for 8 bits of memory. Inputs random data value       */
/*                                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void MM_SBEI_Poke(MM_PokeCmd_t *PokePtr){
   uint32 DestAddress;
   char adress[] = "SBEI_AppData";

   //Random function
   time_t t;
   srand((unsigned) time(&t));

   //Takes inputted address and resolves it
   strcpy(PokePtr->DestSymAddress.SymName, adress);
   CFS_ResolveSymAddr(&(PokePtr->DestSymAddress), &DestAddress);

   //Data Value being inputted
   PokePtr->Data = rand();

   //Data Size of 8 Bits
   PokePtr->DataSize = 8;
   MM_PokeMem(PokePtr, DestAddress);

   return;
} /*end MM_SBEI_Peek

/************************/
/*  End of File Comment */
/************************/