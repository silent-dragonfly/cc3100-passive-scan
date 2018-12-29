#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <stdio.h>
#include <assert.h>

// Functions from examples
#define APPLICATION_VERSION "1.3.0"
#define SL_STOP_TIMEOUT 0xFF

#define STATUS_BIT_PING_DONE 31
#define IS_PING_DONE(status_variable) GET_STATUS_BIT(status_variable, STATUS_BIT_PING_DONE)

void displayBanner();
_i32 initializeAppVariables();
_i32 configureSimpleLinkToDefaultState();
_i32 establishConnectionWithAP();
_i32 checkLanConnection();
_i32 checkInternetConnection();
_i32 initializeAppVariables();

typedef enum {
	/* Choosing this number to avoid overlap with host-driver's error codes */
	LAN_CONNECTION_FAILED = -0x7D0,
	INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
	DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,

	STATUS_CODE_MAX = -0xBB8
} e_AppStatusCodes;

// User's definition
#ifndef NDEBUG
#define DEBUG(FORMAT_STR, ...) fprintf(stderr, "[CC3100] %s:%d %s: " FORMAT_STR "\n", \
                                       __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define DEBUG(Fmt, ...)
#endif

#define RUN(call)            \
    do                       \
    {                        \
        _u8 retVal = call;   \
        assert(retVal >= 0); \
    } while (0)

void stopWDT();
void initClk();
void _SlNonOsMainLoopTask(void);
void displayVersion();
#endif /* __HELPERS_H__ */
