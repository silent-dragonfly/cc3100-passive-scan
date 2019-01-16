#include <windows.h>
#include <limits.h>
#include "passive-scan.h"

#define MAX_PACKET_SIZE (1472)
uint8_t BUFFER[MAX_PACKET_SIZE];

static LONG time_ms() {
    SYSTEMTIME time;
    GetSystemTime(&time);
    return (time.wSecond * 1000) + time.wMilliseconds;
}

static bool isBeaconFrame(ManagementBeaconFrame_t *beaconFrame) {
    FrameControl_t *fc = &beaconFrame->FrameControl;

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

    SSID_t *ssid = &beaconFrame->SSID;
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

int16_t passiveScan(uint8_t Count, PassiveScanEntry_t * pEntries,
        LONG timeout_msec) {

    LONG timeout_msec_per_channel = timeout_msec / MAX_CHANNEL;
    _i32 retVal = 0;
    uint8_t iCurEntry = 0;
    _u32 channel = 1;

    _i16 SockID = sl_Socket(SL_AF_RF, SL_SOCK_RAW, channel);
    if (SockID < 0) {
        DEBUG("Failed to create L1 Socket: retVal %d", SockID);
        return SockID;
    }

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
        while (time_ms() - startTime < timeout_msec_per_channel && iCurEntry < Count) {

            retVal = sl_Recv(SockID, BUFFER, MAX_PACKET_SIZE, 0);

            if (retVal < 0) {
                if (retVal == SL_EAGAIN) {
                    continue;
                }

                DEBUG("Failed to sl_Recv: retVal %d", retVal);
                return retVal;
            }

            SlTransceiverRxOverHead_t *rxHeader = BUFFER;
            ManagementBeaconFrame_t *frame = (ManagementBeaconFrame_t *) (BUFFER
                    + sizeof(SlTransceiverRxOverHead_t));

            if (!isBeaconFrame(frame)) {
                continue;
            }

            // First looking for entry about its network
            PassiveScanEntry_t * entry = NULL;
            for (int i = 0; i < iCurEntry; i++) {
                if (memcmp(pEntries[i].ssid, frame->SSID.SSID, frame->SSID.Length) == 0) {
                    entry = &pEntries[i];
                    break;
                }
            }

            if (entry == NULL) { // we find new network
                entry = &pEntries[iCurEntry++];

                SSID_t *ssid = &frame->SSID;
                memcpy(entry->ssid, ssid->SSID, ssid->Length);
                entry->ssid[ssid->Length] = '\0';
                entry->ssid_len = ssid->Length;

                // TODO: find secure type from package
                entry->sec_type = SL_SEC_TYPE_OPEN;
            }

            // Second, looking for BSS-entry which this frame can correspond to
            BSS_t *bss = NULL;
            BSS_t *bss_list = entry->bss_list;
            for (int i = 0; i < entry->bss_list_len; i++) {
                if (memcmp(bss_list[i].bssid, frame->BSSIDMAC, SL_BSSID_LENGTH) == 0) {
                    bss = &bss_list[i];
                    break;
                }
            }

            if (bss == NULL) { // New BSS for this network
                bss = &(entry->bss_list[entry->bss_list_len++])
                        ;

                memcpy(bss->bssid, frame->BSSIDMAC, SL_BSSID_LENGTH);
                bss->rssi = rxHeader->rssi;
                bss->channel_list[bss->channel_list_len++] = channel;
                continue;
            }

            // Here we are updating the existing BSS entry
            if (bss->rssi > rxHeader->rssi) {
                bss->rssi = rxHeader->rssi;
            }

            bool isChannelSaved = false;
            for (int i = 0; i < bss->channel_list_len; i++) {
                if (bss->channel_list[i] == channel) {
                    isChannelSaved = true;
                    break;
                }
            }
            if (!isChannelSaved) {
                bss->channel_list[bss->channel_list_len++] = channel;
            }
        }
    }

    retVal = sl_Close(SockID);
    if (retVal < 0) {
        DEBUG("Can not close Socket properly: retVal %d", retVal);
    }

    return iCurEntry;
}
