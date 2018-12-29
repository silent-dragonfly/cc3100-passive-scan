#ifndef __PASSIVE_SCAN_H__
#define __PASSIVE_SCAN_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "simplelink.h"
#include "helpers.h"
#include "ieee80211.h"

int16_t passiveScan(uint8_t Count, Sl_WlanNetworkEntry_t * ptEntries, time_t timeout_sec);

#endif // __PASSIVE_SCAN_H__
