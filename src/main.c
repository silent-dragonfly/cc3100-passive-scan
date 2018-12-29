#define __MAIN_C__
#include "main.h"

int main(int argc, char **argv) {

	_i32 retVal = configureSimpleLinkToDefaultState();

	if (retVal < 0) {
		DEBUG("[ERROR] Failed to configure the device in its default state");
		system("PAUSE");
		return -1;
	}
	DEBUG("Device is configured in default state");

	retVal = sl_Start(0, 0, 0);

	if ((retVal < 0) || (ROLE_STA != retVal)) {
		DEBUG("[ERROR] Failed to start the device as STATION");
		system("PAUSE");
		return -1;
	}
	DEBUG("Device started as STATION");

#define SL_SCAN_DISABLE 0
	retVal = sl_WlanPolicySet(SL_POLICY_SCAN, SL_SCAN_DISABLE, NULL, 0);

	if (retVal < 0) {
		DEBUG("[ERROR] Failed to disable SL_POLICY_SCAN");
		system("PAUSE");
		return -1;
	}
	DEBUG("Default Active Scan is disabled");

	retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
			SL_CONNECTION_POLICY(0, 0, 0, 0, 0), NULL, 0);

	if (retVal < 0) {
		DEBUG("[ERROR] Failed to clear WLAN_CONNECTION_POLICY");
		system("PAUSE");
		return -1;
	}

	retVal = sl_WlanDisconnect();

	if (retVal == 0) {
		DEBUG("Disconnected from AP");
	} else {
		// already disconnected
	}
	DEBUG("Connection policy is cleared and CC3100 has been disconnected");

	_u32 numOfEntry = 20;
	PassiveScanEntry_t netEntries[numOfEntry];

	memset((void*) netEntries, 0, sizeof(netEntries));

	retVal = passiveScan(numOfEntry, netEntries, 1);

	if (retVal < 0) {
		DEBUG("[ERROR] Failed while passive scan");
		system("PAUSE");
		return -1;
	}

	printf("    SSID    \tRSSI\tCH\tBSSID(s)\n");
	for (int i = 0; i < retVal; i++) {
		printf("%s\t%d\t", netEntries[i].ssid, netEntries[i].rssi);

		for(int j = 0; j < netEntries[i].channel_list_len; j++) {
			printf("%u, ", netEntries[i].channel_list[j]);
		}
		printf("\n");

		BSSID_t * bssids = netEntries[i].bssid_list;
		for (int j = 0; j < netEntries[i].bssid_list_len; j++) {
			printf("\t%02X:%02X:%02X:%02X:%02X:%02X\n", bssids[j].data[0],
					bssids[j].data[1], bssids[j].data[2], bssids[j].data[3],
					bssids[j].data[4], bssids[j].data[5]);
		}
		printf("\n");
	}

	retVal = sl_Stop(SL_STOP_TIMEOUT);
	if (retVal < 0) {
		DEBUG("[ERROR] Can not stop device properly");
	}

	system("PAUSE");
	return 0;
}
