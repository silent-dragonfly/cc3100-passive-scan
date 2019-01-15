#include "passive-scan.h"

#include <windows.h>

LONG time_ms() {
    SYSTEMTIME time;
    GetSystemTime(&time);
    return (time.wSecond * 1000) + time.wMilliseconds;
}

#define MAX_PACKET_SIZE (1472)
uint8_t BUFFER[MAX_PACKET_SIZE];

static bool isBeaconFrame(ieee80211_MgmBeaconFrame_t *beaconFrame) {
    ieee80211_FrameControl_t *fc = &beaconFrame->FrameControl;

    if (fc->Type != TYPE_MANAGEMENT) {
        return false;
    }

    if (fc->Subtype != MGM_SUBTYPE_BEACON) {
        return false;
    }

    if (fc->HTC_Order) {
        DEBUG("[WARNING] Found Beacon frame with HTC/Order flag on!!!");
        return false;
    }

    ieee80211_SSID_t *ssid = &beaconFrame->SSID;
    if (ssid->ElementID != ELEMID_SSID) {
        DEBUG("[WARNING] Wrong ElementID of SSID-element field");
        return false;
    }

    // Do not process beacons with wildcard SSID
    if (ssid->Length == 0) {
        return false;
    }

    return true;
}

int16_t passiveScan(uint8_t Count, Sl_WlanNetworkEntry_t * pEntries,
        LONG timeout_msec) {

    LONG timeout_per_channel = timeout_msec / MAX_CHANNEL;
    _i32 retVal = 0;
    uint8_t iCurEntry = 0;
    _u32 channel = 1;

    _i16 SockID = sl_Socket(SL_AF_RF, SL_SOCK_RAW, channel);
    if (SockID < 0) {
        DEBUG("Failed to create L1 Socket: retVal %d", SockID);
        return SockID;
    }
    DEBUG("L1 socket has been created");

    SlSockNonblocking_t enableOption = {
            .NonblockingEnabled = 1,
    };
    retVal = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING,
            (_u8 *) &enableOption, sizeof(enableOption));
    if (retVal < 0) {
        DEBUG("Failed sl_SetSockOpt: %d", retVal);
        return -1;
    }

    for (_u32 channel = 1; channel <= MAX_CHANNEL; channel++) {

        SlSocklen_t optlen = sizeof(channel);
        retVal = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_CHANGE_CHANNEL,
                &channel, optlen);
        if (retVal < 0) {
            DEBUG("Failed to change to the channel %u: retVal %d", channel,
                    retVal);
            return retVal;
        }

        LONG startTime = time_ms();

        while (time_ms() - startTime < timeout_per_channel && iCurEntry < Count) {
            retVal = sl_Recv(SockID, BUFFER, MAX_PACKET_SIZE, 0);

            if (retVal < 0) {
                if (retVal == SL_EAGAIN) {
                    continue;
                }
                DEBUG("Failed to sl_Recv: retVal %d", retVal);
                return retVal;
            }

            SlTransceiverRxOverHead_t *rxHeader = BUFFER;
            ieee80211_MgmBeaconFrame_t *beaconFrame =
                    (ieee80211_MgmBeaconFrame_t *) (BUFFER
                            + sizeof(SlTransceiverRxOverHead_t));
            if (!isBeaconFrame(beaconFrame)) {
                continue;
            }

            Sl_WlanNetworkEntry_t *targetEntry = NULL;

            // check uniqueness of saving BSSID
            for (int i = 0; i < iCurEntry; i++) {
                if (memcmp(pEntries[i].bssid, beaconFrame->BSSIDMAC, SL_BSSID_LENGTH) == 0) {
                    targetEntry = &pEntries[i];
                }
            }

            if (targetEntry == NULL) {
                targetEntry = &pEntries[iCurEntry++];
            }

            ieee80211_SSID_t *ssid = &beaconFrame->SSID;
            memcpy(targetEntry->ssid, ssid->SSID, ssid->Length);
            targetEntry->ssid[ssid->Length] = '\0';
            targetEntry->ssid_len = ssid->Length;

            targetEntry->rssi = rxHeader->rssi;

            // TODO: find secure type from package
            targetEntry->sec_type = SL_SEC_TYPE_OPEN;

            memcpy(targetEntry->bssid, beaconFrame->BSSIDMAC, SL_BSSID_LENGTH);
        }
    }

    retVal = sl_Close(SockID);
    if (retVal < 0) {
        DEBUG("Can not close Socket properly: retVal %d", retVal);
    }

    return iCurEntry;
}
