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

    DEBUG("Start passive scan");
    _u32 numOfEntry = 20;
    PassiveScanEntry_t netEntries[numOfEntry];
    memset((void*) netEntries, 0, sizeof(netEntries));
    retVal = passiveScan(numOfEntry, netEntries, MAX_CHANNEL * 1000);

    if (retVal < 0) {
        DEBUG("[ERROR] Failed while passive scan");
        system("PAUSE");
        return -1;
    }

    for (int i = 0; i < retVal; i++) {
        PassiveScanEntry_t *entry = &netEntries[i];
        printf("network SSID: %s\n", entry->ssid);

        printf("      BSSID      \tRSSI\tChannels\n");
        for (int j = 0; j < entry->bss_list_len; j++) {
            BSS_t *bss = &(entry->bss_list[j]);
            printf("%02X:%02X:%02X:%02X:%02X:%02X\t",
                    bss->bssid[0], bss->bssid[1], bss->bssid[2],
                    bss->bssid[3], bss->bssid[4], bss->bssid[5]);
            printf("%4d\t", bss->rssi);
            for (int k = 0; k < bss->channel_list_len - 1; k++) {
                printf("%d, ", bss->channel_list[k]);
            }
            printf("%d\n", bss->channel_list[bss->channel_list_len - 1]);
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
