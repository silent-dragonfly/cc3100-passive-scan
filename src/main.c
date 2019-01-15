#define __MAIN_C__
#include "main.h"

int main(int argc, char **argv) {
    _i32 retVal = -1;

    retVal = configureSimpleLinkToDefaultState();
    if (retVal < 0) {
        DEBUG(" Failed to configure the device in its default state");
        return -1;
    }
    DEBUG(" Device is configured in default state");

    _u32 numOfEntry = 20;
    Sl_WlanNetworkEntry_t netEntries[numOfEntry];
    memset((void*) netEntries, 0, sizeof(netEntries));

    retVal = sl_Start(0, 0, 0);

    if ((retVal < 0) || (ROLE_STA != retVal)) {
        DEBUG("[ERROR] Failed to start the device as STATION");
        system("PAUSE");
        return -1;
    }
    DEBUG("Device started as STATION");

    retVal = addBeaconRxFilter();
    if (retVal < 0) {
        DEBUG("ERROR:addBeaconRxFilter: %d", retVal);
        return -1;
    }
    DEBUG("Beacon RXFilter added");

    retVal = enableBeaconRxFilter();
    if (retVal < 0) {
        DEBUG("ERROR:switchBeaconRxFilter: %d", retVal);
        return -1;
    }
    DEBUG("Filters are successfully enabled");

    _i32 readEntriesSize = passiveScan(numOfEntry, netEntries, 1000);

    if (readEntriesSize < 0) {
        DEBUG("[ERROR] Failed while passive scan");
        return -1;
    }
    DEBUG("Scan Process completed");

    retVal = disableBeaconRxFilter();
    if (retVal < 0) {
        DEBUG("ERROR:switchBeaconRxFilter: %d", retVal);
        return -1;
    }
    DEBUG("Filters are successfully disabled");

    printf("      BSSID      \tRSSI\tSSID\n");
    for (int i = 0; i < readEntriesSize; i++) {
        printf("%02X:%02X:%02X:%02X:%02X:%02X\t%d\t%s\n",
                netEntries[i].bssid[0], netEntries[i].bssid[1],
                netEntries[i].bssid[2], netEntries[i].bssid[3],
                netEntries[i].bssid[4], netEntries[i].bssid[5],
                netEntries[i].rssi, netEntries[i].ssid);
    }

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    if (retVal < 0) {
        DEBUG("[ERROR] Can not stop device properly");
        return -1;
    }

    Sleep(20000);
    return 0;
}
