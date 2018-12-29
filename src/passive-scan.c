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

int16_t passiveScan(uint8_t Count, Sl_WlanNetworkEntry_t * pEntries,
		time_t timeout_sec) {

	_i32 retVal = 0;
	uint8_t iCurEntry = 0;

	// TODO: ensure how transceiver choose the channel
#define CHANNEL (1)
	_i16 SockID = sl_Socket(SL_AF_RF, SL_SOCK_RAW, CHANNEL);
	if (SockID < 0) {
		DEBUG("Failed to create L1 Socket: retVal %d", SockID);
		return SockID;
	}
	DEBUG("L1 socket has been created");

	time_t startTime = time(NULL);

	while (time(NULL) - startTime < timeout_sec && iCurEntry < Count) {
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

		bool isBssidSaved = false;
		for (int i = 0; i < iCurEntry; i++) {
			if (memcmp(pEntries[i].bssid, beaconFrame->BSSIDMAC, SL_BSSID_LENGTH) == 0) {
				isBssidSaved = true;
				break;
			}
		}

		if (isBssidSaved) {
			continue;
		}

		// extract the information to the pEnties
		Sl_WlanNetworkEntry_t *entry = &pEntries[iCurEntry++];

		SSID_t *ssid = &beaconFrame->SSID;
		memcpy(entry->ssid, ssid->SSID, ssid->Length);
		entry->ssid[ssid->Length] = '\0';
		entry->ssid_len = ssid->Length;

		// TODO: find secure type from package
		entry->sec_type = SL_SEC_TYPE_OPEN;

		memcpy(entry->bssid, beaconFrame->BSSIDMAC, SL_BSSID_LENGTH);
		entry->rssi = rxHeader->rssi;
	}

	retVal = sl_Close(SockID);
	if (retVal < 0) {
		DEBUG("Can not close Socket properly: retVal %d", retVal);
	}

	return iCurEntry;
}
