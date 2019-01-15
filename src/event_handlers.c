#include "main.h"

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    if (pWlanEvent == NULL) {
        DEBUG("[WLAN EVENT] NULL Pointer Error");
        return;
    }

    switch (pWlanEvent->Event) {
    case SL_WLAN_CONNECT_EVENT: {
        SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
    }
        break;

    case SL_WLAN_DISCONNECT_EVENT: {
        slWlanConnectAsyncResponse_t *pEventData = NULL;

        CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

        pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

        if (SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION
                == pEventData->reason_code) {
            DEBUG("Device disconnected from the AP on application's request");
        } else {
            DEBUG("Device disconnected from the AP on an ERROR..!!");
        }
    }
        break;

    default: {
        DEBUG("[WLAN EVENT] Unexpected event");
    }
        break;
    }
}

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {
    if (pNetAppEvent == NULL) {
        DEBUG("[NETAPP EVENT] NULL Pointer Error");
        return;
    }

    switch (pNetAppEvent->Event) {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT: {
        SlIpV4AcquiredAsync_t *pEventData = NULL;

        SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

        pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
        g_GatewayIP = pEventData->gateway;
    }
        break;

    default: {
        DEBUG("[NETAPP EVENT] Unexpected event");
    }
        break;
    }
}

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
        SlHttpServerResponse_t *pHttpResponse) {
    DEBUG("[HTTP EVENT] Unexpected event");
}

void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent) {
    /*
     * Most of the general errors are not FATAL are are to be handled
     * appropriately by the application
     */
    DEBUG("[GENERAL EVENT]");

    switch (pDevEvent->Event) {
    case SL_DEVICE_GENERAL_ERROR_EVENT:
        DEBUG("[SL_DEVICE_GENERAL_ERROR_EVENT]");
        break;
    case SL_DEVICE_ABORT_ERROR_EVENT:
        DEBUG("[SL_DEVICE_ABORT_ERROR_EVENT]");
        sl_DeviceReportAbort abortEvent = pDevEvent->EventData.deviceReport;
        DEBUG("Abort Type: %u; Abort Data: %u", abortEvent.AbortType,
                abortEvent.AbortData);
        break;
    case SL_DEVICE_DRIVER_ASSERT_ERROR_EVENT:
        DEBUG("[SL_DEVICE_DRIVER_ASSERT_ERROR_EVENT]");
        break;
    case SL_DEVICE_DRIVER_TIMEOUT_CMD_COMPLETE:
        DEBUG("[SL_DEVICE_DRIVER_TIMEOUT_CMD_COMPLETE]");
        DEBUG("Device driver Repord: %u", pDevEvent->EventData.deviceDriverReport.info);
        break;
    case SL_DEVICE_DRIVER_TIMEOUT_SYNC_PATTERN:
        DEBUG("[SL_DEVICE_DRIVER_TIMEOUT_SYNC_PATTERN]");
        break;
    case SL_DEVICE_DRIVER_TIMEOUT_ASYNC_EVENT:
        DEBUG("[SL_DEVICE_DRIVER_TIMEOUT_ASYNC_EVENT]");
        break;
    case SL_DEVICE_ERROR_MAX:
        DEBUG("[SL_DEVICE_ERROR_MAX]");
        break;
    default:
        DEBUG("Unexpected event!!@!@!!");
    }
}

void SimpleLinkPingReport(SlPingReport_t *pPingReport) {
    SET_STATUS_BIT(g_Status, STATUS_BIT_PING_DONE);

    if (pPingReport == NULL) {
        DEBUG("[PING REPORT] NULL Pointer Error\r\n");
        return;
    }

    g_PingPacketsRecv = pPingReport->PacketsReceived;
}

void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {
    DEBUG("[SOCK EVENT] Unexpected event");
}
