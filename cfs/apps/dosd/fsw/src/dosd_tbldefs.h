/************************************************************************
** Dosd Table Definitions
**
*/

#ifndef _dosd_tbldefs_
#define _dosd_tbldefs_


#include "cfe.h"


/*
** Definition of Table Data Structures
*/
typedef struct
{
  uint8                 TblElement1;
  uint16                TblElement2;
  uint32                TblElement3;

} OS_PACK DOSD_Tbl_1_t;

typedef struct
{
  int32                 TblElement1;
  int16                 TblElement2;
  int8                  TblElement3;

} OS_PACK DOSD_Tbl_2_t;


#endif  /* _dosd_tbldefs_ */

/************************/
/*  End of File Comment */
/************************/
