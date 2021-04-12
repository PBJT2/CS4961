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
void MM_SBEI_Flip(uint8 ByteValue, uint32 DestAddress);

#endif /* _mm_sbei_ */

/************************/
/*  End of File Comment */
/************************/
