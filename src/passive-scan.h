#ifndef __PASSIVE_SCAN_H__
#define __PASSIVE_SCAN_H__

#include <stdint.h>

#include "simplelink.h"

int16_t passiveScan(uint8_t Count, Sl_WlanNetworkEntry_t * ptEntries);

#endif // __PASSIVE_SCAN_H__
