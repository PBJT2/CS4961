/*************************************************************************/
#ifndef _mm_sbei_
#define _mm_sbei_

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "mm_msg.h"
#include "mm_filedefs.h"

void MM_SBEI_InjectCmd(CFE_SB_MsgPtr_t MessagePtr);
void MM_SBEI_Poke(MM_PokeCmd_t *PokePtr);
void MM_SBEI_Peek(MM_PeekCmd_t *PeekPtr);

#endif /* _mm_sbei_ */

/************************/
/*  End of File Comment */
/************************/
