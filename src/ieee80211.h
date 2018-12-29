#ifndef __IEEE_802_11__
#define __IEEE_802_11__

#include <stdint.h>

typedef enum Types {
	TYPE_MANAGEMENT = 0, TYPE_CONTROL = 1, TYPE_DATA = 2
} Types;

typedef enum DATA_Subtypes {
	DATA_SUBTYPE_QOS = 8,
};

typedef struct FrameControl {
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
} FrameControl;

typedef struct SequenceControl {
	uint16_t FragmentNumber :4;
	uint16_t SequenceNumber :12;
} SequenceControl;

typedef struct QoSControl {
	// TODO: write QoS Control bits
	uint16_t TODO;
} QoSControl;

#define MAX_DATA_SIZE (100)

typedef struct DataQoSFrame_FromDSToSTA {
	FrameControl FrameControl;
	uint16_t Duration;
	uint8_t DestinationMAC[6];
	uint8_t BSSIDMAC[6];
	uint8_t SourceMAC[6];
	SequenceControl SequenceControl;
	QoSControl QoSControl;

	uint8_t Data[MAX_DATA_SIZE];
} DataQoSFrame_FromDSToSTA;

#endif // __IEEE_802_11__
