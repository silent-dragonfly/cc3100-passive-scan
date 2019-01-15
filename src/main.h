#ifndef __MAIN_H__
#define __MAIN_H__

#include "simplelink.h"
#include "sl_common.h"

#include "ieee80211.h"

#include "helpers.h"
#include "event_handlers.h"
#include "passive-scan.h"


int addBeaconRxFilter();
int enableBeaconRxFilter();
int disableBeaconRxFilter();

// global variables
#ifndef __MAIN_C__
extern _u32 g_Status;
extern _u32 g_PingPacketsRecv;
extern _u32 g_GatewayIP;
#else
_u32 g_Status = 0;
_u32 g_PingPacketsRecv = 0;
_u32 g_GatewayIP = 0;
#endif

#endif
