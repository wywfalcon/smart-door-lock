/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__PIC24E__)
    	#include <p24Exxxx.h>
    #elif defined (__PIC24F__)||defined (__PIC24FK__)
	#include <p24Fxxxx.h>
    #elif defined(__PIC24H__)
	#include <p24Hxxxx.h>
    #endif
#endif

#include <stdint.h>          /* For uint32_t definition */
#include <stdbool.h>         /* For true/false definition */

#include "user.h"            /* variables/params used by user.c */
#include "miscHardware.h"
#include "database.h"
#include "debug.h"
#include "event.h"
#include "fingerPrintReader.h"
#include "miscHardware.h"
#include "wifi.h"

uint8_t systemReady = 0;

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

/* TODO Initialize User Ports/Peripherals/Project here */

void InitApp(void)
{
    /* start up light */
    toggleLED(0);
    toggleLED(1);
    delay(1000);
    toggleLED(0);
    toggleLED(1);

    /* Initialize IO ports and peripherals */  //delete afterwards
    systemReady = 1;
    inputStart();
    timerSetup();
    WifiStart(SERVER_MODE);
    deleteDatabase();
    FPRStart();
    sendFPRCommand(FPR_DELETE_ALL, ZERO);
    FPRStop();
    
    /* Initialize peripherals */
}

