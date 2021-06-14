/************************************************************************
** Sbei Table Definitions
**
*/

#ifndef _sbei_tbldefs_
#define _sbei_tbldefs_


#include "cfe.h"


/*
** Definition of Table Data Structures
*/
typedef struct
{
  uint8                 TblElement1;
  uint16                TblElement2;
  uint32                TblElement3;

} OS_PACK SBEI_Tbl_1_t;

typedef struct
{
  int32                 TblElement1;
  int16                 TblElement2;
  int8                  TblElement3;

} OS_PACK SBEI_Tbl_2_t;


#endif  /* _sbei_tbldefs_ */

/************************/
/*  End of File Comment */
/************************/
