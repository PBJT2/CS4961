/************************************************************************
** File:
**   $Id: dosd_app_msgids.h  $
**
** Purpose: 
**  Define Dosd App  Message IDs
**
** Notes:
**   1. Default to OSK test IDs. These IDs should be changed if the app
**      is integrated with the user's system.
**
*************************************************************************/
#ifndef _dosd_app_msgids_h_
#define _dosd_app_msgids_h_

/*
** Command Message IDs
*/

#define  DOSD_CMD_MID        0x1F4A //0x1FF0
#define  DOSD_SEND_HK_MID    0x1F4B //0x1FF1

/*
** Telemetry Message IDs
*/

#define  DOSD_HK_TLM_MID     0x1F4C //was 0x0FF0

#endif /* _dosd_app_msgids_h_ */
