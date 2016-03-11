#include <p24fxxxx.h>
#include <string.h>
#include "debug.h"
#include <stdint.h>

/******************************************************************************/
/* LED Control                                                                */
/******************************************************************************/

void turnOnLED (int8_t ledNumber) {
    switch (ledNumber) {
        case 0: TRISBbits.TRISB8 = 0; LATBbits.LATB8 = 1; break;
        case 1: TRISBbits.TRISB9 = 0; LATBbits.LATB9 = 1; break;
        case 2: TRISAbits.TRISA7 = 0; LATAbits.LATA7 = 1; break;
        case 3: TRISAbits.TRISA6 = 0; LATAbits.LATA6 = 1; break;
        case 4: TRISBbits.TRISB10 = 0; LATBbits.LATB10 = 1; break;
        case 5: TRISBbits.TRISB12 = 0; LATBbits.LATB12 = 1; break;
        case 6: TRISBbits.TRISB13 = 0; LATBbits.LATB13 = 1; break;
        case 7: TRISBbits.TRISB14 = 0; LATBbits.LATB14 = 1; break;
        case 8: TRISBbits.TRISB15 = 0; LATBbits.LATB15 = 1; break;
        case 9: TRISAbits.TRISA0 = 0; LATAbits.LATA0 = 1; break;
    }
}

void turnOffLED (int8_t ledNumber) {
    switch (ledNumber) {
        case 0: TRISBbits.TRISB8 = 0; LATBbits.LATB8 = 0; break;
        case 1: TRISBbits.TRISB9 = 0; LATBbits.LATB9 = 0; break;
        case 2: TRISAbits.TRISA7 = 0; LATAbits.LATA7 = 0; break;
        case 3: TRISAbits.TRISA6 = 0; LATAbits.LATA6 = 0; break;
        case 4: TRISBbits.TRISB10 = 0; LATBbits.LATB10 = 0; break;
        case 5: TRISBbits.TRISB12 = 0; LATBbits.LATB12 = 0; break;
        case 6: TRISBbits.TRISB13 = 0; LATBbits.LATB13 = 0; break;
        case 7: TRISBbits.TRISB14 = 0; LATBbits.LATB14 = 0; break;
        case 8: TRISBbits.TRISB15 = 0; LATBbits.LATB15 = 0; break;
        case 9: TRISAbits.TRISA0 = 0; LATAbits.LATA0 = 0; break;
    }
}

void toggleLED (int8_t ledNumber) {
    switch (ledNumber) {
        case 0: TRISBbits.TRISB8 = 0; LATBbits.LATB8 ^= 1; break;
        case 1: TRISBbits.TRISB9 = 0; LATBbits.LATB9 ^= 1; break;
        case 2: TRISAbits.TRISA7 = 0; LATAbits.LATA7 ^= 1; break;
        case 3: TRISAbits.TRISA6 = 0; LATAbits.LATA6 ^= 1; break;
        case 4: TRISBbits.TRISB10 = 0; LATBbits.LATB10 ^= 1; break;
        case 5: TRISBbits.TRISB12 = 0; LATBbits.LATB12 ^= 1; break;
        case 6: TRISBbits.TRISB13 = 0; LATBbits.LATB13 ^= 1; break;
        case 7: TRISBbits.TRISB14 = 0; LATBbits.LATB14 ^= 1; break;
        case 8: TRISBbits.TRISB15 = 0; LATBbits.LATB15 ^= 1; break;
        case 9: TRISAbits.TRISA0 = 0; LATAbits.LATA0 ^= 1; break;
    }
}

void turnAllLEDOff () {
        TRISBbits.TRISB8 = 0; LATBbits.LATB8 = 0;
        TRISBbits.TRISB9 = 0; LATBbits.LATB9 = 0;
        TRISAbits.TRISA7 = 0; LATAbits.LATA7 = 0;
        TRISAbits.TRISA6 = 0; LATAbits.LATA6 = 0;
//        TRISBbits.TRISB10 = 0; LATBbits.LATB10 = 0;
//        TRISBbits.TRISB12 = 0; LATBbits.LATB12 = 0;
//        TRISBbits.TRISB13 = 0; LATBbits.LATB13 = 0;
//        TRISBbits.TRISB14 = 0; LATBbits.LATB14 = 0;
//        TRISBbits.TRISB15 = 0; LATBbits.LATB15 = 0;
        TRISAbits.TRISA0 = 0; LATAbits.LATA0 = 0;
}
