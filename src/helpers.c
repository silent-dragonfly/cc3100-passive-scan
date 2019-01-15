#include "main.h"

void _SlNonOsMainLoopTask(void) {
}

void stopWDT() {
}

void initClk() {
}

void displayVersion() {
	SlVersionFull ver = { 0 };

	_u8 configOpt = SL_DEVICE_GENERAL_VERSION;
	_u8 configLen = sizeof(ver);
	_u8 retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt,
			&configLen, (_u8 *) (&ver));
	assert(retVal >= 0);

	DEBUG("NWP version: %d.%d.%d.%d", ver.NwpVersion[0], ver.NwpVersion[1],
			ver.NwpVersion[2], ver.NwpVersion[3]);
	DEBUG("MAC version: %d.%d.%d.%d", ver.ChipFwAndPhyVersion.FwVersion[0],
			ver.ChipFwAndPhyVersion.FwVersion[1],
			ver.ChipFwAndPhyVersion.FwVersion[2],
			ver.ChipFwAndPhyVersion.FwVersion[3]);
	DEBUG("PHY version: %d.%d.%d.%d", ver.ChipFwAndPhyVersion.PhyVersion[0],
			ver.ChipFwAndPhyVersion.PhyVersion[1],
			ver.ChipFwAndPhyVersion.PhyVersion[2],
			ver.ChipFwAndPhyVersion.PhyVersion[3]);
}

_i32 initializeAppVariables() {
	g_Status = 0;
	g_PingPacketsRecv = 0;
	g_GatewayIP = 0;

	return SUCCESS;
}

void displayBanner() {
	DEBUG("");
	DEBUG("Template for CC3100 application - Version");
	DEBUG(APPLICATION_VERSION);
	DEBUG(
			"*******************************************************************************");
}

/*!
 \brief This function configure the SimpleLink device in its default state. It:
 - Sets the mode to STATION
 - Configures connection policy to Auto and AutoSmartConfig
 - Deletes all the stored profiles
 - Enables DHCP
 - Disables Scan policy
 - Sets Tx power to maximum
 - Sets power policy to normal
 - Unregisters mDNS services
 - Remove all filters

 \param[in]      none

 \return         On success, zero is returned. On error, negative is returned
 */
_i32 configureSimpleLinkToDefaultState() {
	SlVersionFull ver = { 0 };
	_WlanRxFilterOperationCommandBuff_t RxFilterIdMask = { 0 };

	_u8 val = 1;
	_u8 configOpt = 0;
	_u8 configLen = 0;

	_i32 retVal = -1;
	_i32 mode = -1;

	mode = sl_Start(0, 0, 0);
	ASSERT_ON_ERROR(mode);

	/* If the device is not in station-mode, try configuring it in station-mode */
	if (ROLE_STA != mode) {
		if (ROLE_AP == mode) {
			/* If the device is in AP mode, we need to wait for this event before doing anything */
			while (!IS_IP_ACQUIRED(g_Status)) {
				_SlNonOsMainLoopTask();
			}
		}

		/* Switch to STA role and restart */
		retVal = sl_WlanSetMode(ROLE_STA);
		ASSERT_ON_ERROR(retVal);

		retVal = sl_Stop(SL_STOP_TIMEOUT);
		ASSERT_ON_ERROR(retVal);

		retVal = sl_Start(0, 0, 0);
		ASSERT_ON_ERROR(retVal);

		/* Check if the device is in station again */
		if (ROLE_STA != retVal) {
			/* We don't want to proceed if the device is not coming up in station-mode */
			ASSERT_ON_ERROR(DEVICE_NOT_IN_STATION_MODE);
		}
	}

	/* Get the device's version-information */
	configOpt = SL_DEVICE_GENERAL_VERSION;
	configLen = sizeof(ver);
	retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen,
			(_u8 *) (&ver));
	ASSERT_ON_ERROR(retVal);

	/* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
	retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
			SL_CONNECTION_POLICY(0, 0, 0, 0, 0), NULL, 0);
	ASSERT_ON_ERROR(retVal);

	/* Remove all profiles */
	retVal = sl_WlanProfileDel(0xFF);
	ASSERT_ON_ERROR(retVal);

	/*
	 * Device in station-mode. Disconnect previous connection if any
	 * The function returns 0 if 'Disconnected done', negative number if already disconnected
	 * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
	 */
	retVal = sl_WlanDisconnect();
	if (0 == retVal) {
		/* Wait */
		while (IS_CONNECTED(g_Status)) {
			_SlNonOsMainLoopTask();
		}
	}

	/* Enable DHCP client*/
	retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE, 1, 1, &val);
	ASSERT_ON_ERROR(retVal);

	/* Disable scan */
	configOpt = SL_SCAN_POLICY(0);
	retVal = sl_WlanPolicySet(SL_POLICY_SCAN, configOpt, NULL, 0);
	ASSERT_ON_ERROR(retVal);

	/* Set Tx power level for station mode
	 Number between 0-15, as dB offset from max power - 0 will set maximum power */
	_u8 power = 0;
	retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
			WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, sizeof(power), &power);
	ASSERT_ON_ERROR(retVal);

	/* Set PM policy to normal */
	retVal = sl_WlanPolicySet(SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0);
	ASSERT_ON_ERROR(retVal);

	/* Unregister mDNS services */
	retVal = sl_NetAppMDNSUnRegisterService(0, 0);
	ASSERT_ON_ERROR(retVal);

	/* Remove  all 64 filters (8*8) */
	pal_Memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
	retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *) &RxFilterIdMask,
			sizeof(_WlanRxFilterOperationCommandBuff_t));
	ASSERT_ON_ERROR(retVal);

	retVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(retVal);

	retVal = initializeAppVariables();
	ASSERT_ON_ERROR(retVal);

	return retVal; /* Success */
}

_i32 establishConnectionWithAP() {
	SlSecParams_t secParams = { 0 };
	_i32 retVal = 0;

	secParams.Key = (_i8 *) PASSKEY;
	secParams.KeyLen = pal_Strlen(PASSKEY);
	secParams.Type = SEC_TYPE;

	DEBUG("Connect to %s", SSID_NAME);
	retVal = sl_WlanConnect((_i8 *) SSID_NAME, pal_Strlen(SSID_NAME), 0,
			&secParams, 0);
	ASSERT_ON_ERROR(retVal);

	/* Wait */
	while ((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status))) {
		_SlNonOsMainLoopTask();
	}

	return SUCCESS;
}

#define PING_INTERVAL 1000 /* In msecs */
#define PING_TIMEOUT 3000  /* In msecs */
#define PING_PKT_SIZE 20   /* In bytes */
#define NO_OF_ATTEMPTS 3

#define HOST_NAME "www.ti.com"

_i32 checkLanConnection() {
	SlPingStartCommand_t pingParams = { 0 };
	SlPingReport_t pingReport = { 0 };

	_i32 retVal = -1;

	CLR_STATUS_BIT(g_Status, STATUS_BIT_PING_DONE);
	g_PingPacketsRecv = 0;

	/* Set the ping parameters */
	pingParams.PingIntervalTime = PING_INTERVAL;
	pingParams.PingSize = PING_PKT_SIZE;
	pingParams.PingRequestTimeout = PING_TIMEOUT;
	pingParams.TotalNumberOfAttempts = NO_OF_ATTEMPTS;
	pingParams.Flags = 0;
	pingParams.Ip = g_GatewayIP;

	/* Check for LAN connection */
	retVal = sl_NetAppPingStart((SlPingStartCommand_t *) &pingParams,
			SL_AF_INET, (SlPingReport_t *) &pingReport, SimpleLinkPingReport);
	ASSERT_ON_ERROR(retVal);

	/* Wait */
	while (!IS_PING_DONE(g_Status)) {
		_SlNonOsMainLoopTask();
	}

	if (0 == g_PingPacketsRecv) {
		/* Problem with LAN connection */
		ASSERT_ON_ERROR(LAN_CONNECTION_FAILED);
	}

	/* LAN connection is successful */
	return SUCCESS;
}

/*!
 \brief This function checks the internet connection by pinging
 the external-host (HOST_NAME)

 \param[in]  None

 \return     0 on success, negative error-code on error
 */
_i32 checkInternetConnection() {
	SlPingStartCommand_t pingParams = { 0 };
	SlPingReport_t pingReport = { 0 };

	_u32 ipAddr = 0;

	_i32 retVal = -1;

	CLR_STATUS_BIT(g_Status, STATUS_BIT_PING_DONE);
	g_PingPacketsRecv = 0;

	/* Set the ping parameters */
	pingParams.PingIntervalTime = PING_INTERVAL;
	pingParams.PingSize = PING_PKT_SIZE;
	pingParams.PingRequestTimeout = PING_TIMEOUT;
	pingParams.TotalNumberOfAttempts = NO_OF_ATTEMPTS;
	pingParams.Flags = 0;
	pingParams.Ip = g_GatewayIP;

	/* Check for Internet connection */
	retVal = sl_NetAppDnsGetHostByName((_i8 *) HOST_NAME, pal_Strlen(HOST_NAME),
			&ipAddr, SL_AF_INET);
	ASSERT_ON_ERROR(retVal);

	/* Replace the ping address to match HOST_NAME's IP address */
	pingParams.Ip = ipAddr;

	/* Try to ping HOST_NAME */
	retVal = sl_NetAppPingStart((SlPingStartCommand_t *) &pingParams,
			SL_AF_INET, (SlPingReport_t *) &pingReport, SimpleLinkPingReport);
	ASSERT_ON_ERROR(retVal);

	/* Wait */
	while (!IS_PING_DONE(g_Status)) {
		_SlNonOsMainLoopTask();
	}

	if (0 == g_PingPacketsRecv) {
		/* Problem with internet connection*/
		ASSERT_ON_ERROR(INTERNET_CONNECTION_FAILED);
	}

	/* Internet connection is successful */
	return SUCCESS;
}
