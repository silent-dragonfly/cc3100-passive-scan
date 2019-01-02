#ifndef __IEEE_802_11__
#define __IEEE_802_11__

#include <stdint.h>

typedef enum Types_e {
	TYPE_MANAGEMENT = 0, TYPE_CONTROL = 1, TYPE_DATA = 2
} Types_e;

typedef enum MANAGEMENT_Subtypes_e {
	MGM_SUBTYPE_ASSOC_REQ = 0,
	MGM_SUBTYPE_ASSOC_RES = 1,
	MGM_SUBTYPE_REASSOC_REQ = 2,
	MGM_SUBTYPE_REASSOC_RES = 3,
	MGM_SUBTYPE_PROBE_REQ = 4,
	MGM_SUBTYPE_PROBE_RES = 5,
	MGM_SUBTYPE_BEACON = 8,
} MANAGEMENT_Subtypes_e;

typedef enum DATA_Subtypes_e {
	DATA_SUBTYPE_QOS = 8,
} DATA_Subtypes_e;

typedef struct FrameControl_t {
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
} FrameControl_t;

typedef struct SequenceControl_t {
	uint16_t FragmentNumber :4;
	uint16_t SequenceNumber :12;
} SequenceControl_t;

typedef struct QoSControl {
	// TODO: write QoS Control bits
	uint16_t TODO;
} QoSControl;

// fields which are not an elements

typedef struct CapabilityInformation_t {
	uint16_t TODO;
} CapabilityInformation_t;

// elements
typedef enum ElementID_e {
	ELEMID_SSID = 0,
} ElementID_e;


#define MAX_SSID_LENGTH
typedef struct SSID_t {
	uint8_t ElementID;
	uint8_t Length;
	uint8_t SSID[MAX_SSID_LENGTH];
} SSID_t;

#define MAX_DATA_SIZE (100)

typedef struct ManagementBeaconFrame_t {
	FrameControl_t FrameControl;
	uint16_t Duration;
	uint8_t DestinationMAC[6];
	uint8_t SourceMAC[6];
	uint8_t BSSIDMAC[6];
	SequenceControl_t SequenceControl;

	// Fields
	uint64_t Timestamp; /* value of the timing synchronization function (TSF) timer */
	uint16_t BeaconInterval; /* in time units (TUs), TU equal to 1024 μs/microseconds/10^(-6)) */
	CapabilityInformation_t CapabilityInformation;

	// Elements
	SSID_t SSID;
} ManagementBeaconFrame_t;

typedef struct DataQoSFrame_FromDSToSTA_t {
	FrameControl_t FrameControl;
	uint16_t Duration;
	uint8_t DestinationMAC[6];
	uint8_t BSSIDMAC[6];
	uint8_t SourceMAC[6];
	SequenceControl_t SequenceControl;
	QoSControl QoSControl;

	uint8_t Data[MAX_DATA_SIZE];
} DataQoSFrame_FromDSToSTA_t;

#endif // __IEEE_802_11__
