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
	Sl_WlanNetworkEntry_t netEntries[numOfEntry];

	memset((void*) netEntries, 0, sizeof(netEntries));

	retVal = passiveScan(numOfEntry, netEntries, 10);

	if (retVal < 0) {
		DEBUG("[ERROR] Failed while passive scan");
		system("PAUSE");
		return -1;
	}

	printf("      BSSID      \tRSSI\tSSID\n");
	for (int i = 0; i < retVal; i++) {
		printf("%02X:%02X:%02X:%02X:%02X:%02X\t%4d\t%s\n",
				netEntries[i].bssid[0], netEntries[i].bssid[1],
				netEntries[i].bssid[2], netEntries[i].bssid[3],
				netEntries[i].bssid[4], netEntries[i].bssid[5],
				netEntries[i].rssi, netEntries[i].ssid);
	}

	retVal = sl_Stop(SL_STOP_TIMEOUT);
	if (retVal < 0) {
		DEBUG("[ERROR] Can not stop device properly");
	}

	system("PAUSE");
	return 0;
}
