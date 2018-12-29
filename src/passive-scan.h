#ifndef __PASSIVE_SCAN_H__
#define __PASSIVE_SCAN_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "simplelink.h"
#include "helpers.h"
#include "ieee80211.h"

#define MAX_BSSID (20)
#define MAX_CHANNEL (13)

typedef struct BSSID_t {
	uint8_t data[SL_BSSID_LENGTH];
} BSSID_t;

typedef struct PassiveScanEntry_t {
	uint8_t ssid[MAXIMAL_SSID_LENGTH];
	uint8_t ssid_len;
	uint8_t sec_type;
	BSSID_t bssid_list[MAX_BSSID];
	uint8_t bssid_list_len;
	int8_t rssi;
	uint8_t channel_list[MAX_CHANNEL];
	uint8_t channel_list_len;
} PassiveScanEntry_t;

int16_t passiveScan(uint8_t Count, PassiveScanEntry_t * ptEntries,
		time_t timeout_sec_per_channel);

#endif // __PASSIVE_SCAN_H__
