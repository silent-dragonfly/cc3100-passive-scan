#ifndef __IEEE_802_11__
#define __IEEE_802_11__

#include <stdint.h>

typedef enum ieee80211_Types_e {
    TYPE_MANAGEMENT = 0,
    TYPE_CONTROL = 1,
    TYPE_DATA = 2
} ieee80211_Types_e;

typedef enum ieee80211_MgmSubtypes_e {
    MGM_SUBTYPE_ASSOC_REQ = 0,
    MGM_SUBTYPE_ASSOC_RES =  1,
    MGM_SUBTYPE_REASSOC_REQ = 2,
    MGM_SUBTYPE_REASSOC_RES = 3,
    MGM_SUBTYPE_PROBE_REQ = 4,
    MGM_SUBTYPE_PROBE_RES = 5,
    MGM_SUBTYPE_BEACON = 8,
} ieee80211_MgmSubtypes_e;

typedef enum ieee80211_DataSubtypes_e {
    DATA_SUBTYPE_QOS = 8,
} ieee80211_DataSubtypes_e;

typedef struct ieee80211_FrameControl_t {
    uint8_t ProtocolVersion :2;
    uint8_t Type :2;
    uint8_t Subtype :4;
    uint8_t ToDS :1;
    uint8_t FromDS :1;
    uint8_t MoreFragments :1;
    uint8_t Retry :1;
    uint8_t PowerManagement :1;
    uint8_t MoreData :1;
    uint8_t ProtectedFrame :1;
    uint8_t HTC_Order :1;
} ieee80211_FrameControl_t;

typedef struct ieee80211_SequenceControl_t {
    uint16_t FragmentNumber :4;
    uint16_t SequenceNumber :12;
} ieee80211_SequenceControl_t;

typedef struct ieee80211_QoSControl_t {
    // TODO: write QoS Control bits
    uint16_t TODO;
} ieee80211_QoSControl_t;

// fields which are not an elements

typedef struct ieee80211_CapabilityInformation_t {
    uint16_t TODO;
} ieee80211_CapabilityInformation_t;

// elements
typedef enum ieee80211_ElementID_e {
    ELEMID_SSID = 0,
} ieee80211_ElementID_e;

#define MAX_SSID_LENGTH 32
typedef struct ieee80211_SSID_t {
    uint8_t ElementID;
    uint8_t Length;
    uint8_t SSID[0];
} ieee80211_SSID_t;

typedef struct ieee80211_MgmBeaconFrame_t {
    ieee80211_FrameControl_t FrameControl;
    uint16_t Duration;
    uint8_t DestinationMAC[6];
    uint8_t SourceMAC[6];
    uint8_t BSSIDMAC[6];
    ieee80211_SequenceControl_t SequenceControl;

    // Fields
    uint64_t Timestamp; /* value of the timing synchronization function (TSF) timer */
    uint16_t BeaconInterval; /* in time units (TUs), TU equal to 1024 μs/microseconds/10^(-6)) */
    ieee80211_CapabilityInformation_t CapabilityInformation;

    // Elements
    ieee80211_SSID_t SSID;
} ieee80211_MgmBeaconFrame_t;

#endif // __IEEE_802_11__
