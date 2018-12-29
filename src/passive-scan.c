#include "passive-scan.h"

#define MAX_PACKET_SIZE (1472)
uint8_t BUFFER[MAX_PACKET_SIZE];

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
		time_t timeout_sec_per_channel) {

	_i32 retVal = 0;
	uint8_t iCurEntry = 0;
	_u32 channel = 1;

	_i16 SockID = sl_Socket(SL_AF_RF, SL_SOCK_RAW, channel);
	if (SockID < 0) {
		DEBUG("Failed to create L1 Socket: retVal %d", SockID);
		return SockID;
	}
	DEBUG("L1 socket has been created");

	while (true) {

		time_t startTime = time(NULL);
		while (time(NULL) - startTime < timeout_sec_per_channel
				&& iCurEntry < Count) {
			retVal = sl_Recv(SockID, BUFFER, MAX_PACKET_SIZE, 0);

			if (retVal < 0) {
				DEBUG("Failed to sl_Recv: retVal %d", retVal);
				return retVal;
			}

			SlTransceiverRxOverHead_t *rxHeader = BUFFER;
			ManagementBeaconFrame_t *beaconFrame =
					(ManagementBeaconFrame_t *) (BUFFER
							+ sizeof(SlTransceiverRxOverHead_t));
			if (!isBeaconFrame(beaconFrame)) {
				continue;
			}

			{
				bool isBssidSaved = false;

				PassiveScanEntry_t * targetEntry = NULL;
				for (int i = 0; i < iCurEntry; i++) {
					BSSID_t *bssids = pEntries[i].bssid_list;

					if (memcmp(pEntries[i].ssid, beaconFrame->SSID.SSID,
							beaconFrame->SSID.Length) == 0) {
						targetEntry = &pEntries[i];
					}

					for (int j = 0; j < pEntries[i].bssid_list_len; j++) {
						if (memcmp(bssids[j].data, beaconFrame->BSSIDMAC,
						SL_BSSID_LENGTH) == 0) {
							isBssidSaved = true;

							bool isChannelSaved = false;
							for (int k = 0; k < pEntries[i].channel_list_len; k++) {
								if (channel == pEntries[i].channel_list[k]) {
									isChannelSaved = true;
									break;
								}
							}

							if (!isChannelSaved) {
								pEntries[i].channel_list[pEntries[i].channel_list_len] = channel;
								pEntries[i].channel_list_len++;
							}

							break;
						}
					}

					if (isBssidSaved) {
						break;
					}
				}

				if (isBssidSaved) {
					continue;
				}

				if (targetEntry != NULL) {
					if (targetEntry->bssid_list_len >= MAX_BSSID) {
						DEBUG("[WARNING] list of BSSID is full");
						continue;
					}

					memcpy(
							targetEntry->bssid_list[targetEntry->bssid_list_len].data,
							beaconFrame->BSSIDMAC, SL_BSSID_LENGTH);
					targetEntry->bssid_list_len++;
					continue;
				}
			}

			// extract the information to the pEnties
			PassiveScanEntry_t *entry = &pEntries[iCurEntry++];

			SSID_t *ssid = &beaconFrame->SSID;
			memcpy(entry->ssid, ssid->SSID, ssid->Length);
			entry->ssid[ssid->Length] = '\0';
			entry->ssid_len = ssid->Length;

			entry->channel_list[entry->channel_list_len] = channel;
			entry->channel_list_len++;

			entry->rssi = rxHeader->rssi;

			// TODO: find secure type from package
			entry->sec_type = SL_SEC_TYPE_OPEN;

			memcpy(entry->bssid_list[entry->bssid_list_len].data,
					beaconFrame->BSSIDMAC, SL_BSSID_LENGTH);
			entry->bssid_list_len++;
		}

		channel++;

		if (channel >= 14) {
			break;
		}

		SlSocklen_t optlen = sizeof(channel);
		retVal = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_CHANGE_CHANNEL,
				&channel, optlen);
		if (retVal < 0) {
			DEBUG("Failed to change to the channel %u: retVal %d", channel,
					retVal);
			return retVal;
		}
	}

	retVal = sl_Close(SockID);
	if (retVal < 0) {
		DEBUG("Can not close Socket properly: retVal %d", retVal);
	}

	return iCurEntry;
}
