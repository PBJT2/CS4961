/*************************************************************************
** File:
**   $Id: mm_sbei.h
**
** Purpose: 
**   Specification for the CFS Memory Manager SBEI_Inject ground commands.
** 
*************************************************************************/
#ifndef _mm_sbei_
#define _mm_sbei_

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "mm_msg.h"
#include "mm_filedefs.h"

/************************************************************************/
/** \brief Process single bit error inject command
**  
**  \par Description
**       Processes the single bit error inject command that will read 
**		 the SBEI memory location and flip 1 bit of the 8 bit sequence
**		
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
**  \param [in]   msg   A #CFE_SB_MsgPtr_t pointer that
**                      references the software bus message 
**
**  \sa #MM_SBEI_INJECT_CC
**
*************************************************************************/
void MM_SBEI_InjectCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Bit Flip
**  
**  \par Description
**       Support function for #MM_SBEI_InjectCmd. This routine will flip
**		 a random bit in the SBEI
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
**  \param [in]   ByteValue     A #uint8 ByteValue that was read in the 
**								SBEI memory
**
**  \param [in]   DestAddress   The source address for the flip operation 
** 
*************************************************************************/
void MM_SBEI_Flip(uint8 ByteValue, uint32 DestAddress);

#endif /* _mm_sbei_ */

/************************/
/*  End of File Comment */
/************************/
