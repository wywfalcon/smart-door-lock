#include <p24fxxxx.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "miscHardware.h"
#include "fingerPrintReader.h"
#include "wifi.h"


uint8_t updateFlag = TRUE;

/******************************************************************************/
/* Button                                                                     */
/******************************************************************************/

void inputStart () {
    TRISBbits.TRISB5 = 1;   //button          //except maybe this one
    TRISBbits.TRISB6 = 1;   //switch
    TRISBbits.TRISB15 = 0;  //deadbolt
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB12 = 0;  //FPR
    TRISBbits.TRISB11 = 0;  //Wifi
}

uint8_t buttonIsPressed () {
    if(PORTBbits.RB5 == 1){
        delay(50);
        if(PORTBbits.RB5 == 1){
            while(PORTBbits.RB5 == 1) Nop();
            return 1;
        }
    }
    return 0;
}

uint8_t switchIsOn () {
    return PORTBbits.RB6;
}

void lockDeadBolt (uint8_t lockIt) {
    if(lockIt == FALSE) LATBbits.LATB14 = 0;  //unlock else lock
    else LATBbits.LATB14 = 1;
    LATBbits.LATB15 = 1;
    delay(190);
    LATBbits.LATB15 = 0;
}

void turnOnFPR (int turnOn) {
    if(turnOn == TRUE){
        LATBbits.LATB12 = 1;
        delay(200);
        sendFPRCommand(FPR_OPEN, ZERO);
        delay(500);
    } else LATBbits.LATB12 = 0;
}

void turnOnWifiChip (int turnOn) {
    if(turnOn == TRUE) LATBbits.LATB11 = 1;
    else LATBbits.LATB11 = 0;
}


/******************************************************************************/
/* Timing                                                                     */
/******************************************************************************/

uint8_t sec = 0, dSec = 0;
uint16_t minute = 0, dMinute = 0;
uint8_t day = 0;

void delay (uint16_t t) {
    int i,j = 0;
    for(i = 0; i < t; i++){
        for(j = 0; j < 1500; j++){
            Nop();
        }
    }
}

void setCurrTime(int d, int h, int m, int s) { //day (0-6), hour(0-23), minute (0-59), seconds (0-59)
    minute = (1440*d)+(60*h) + m;
    if (minute >= 300) minute = minute - 480;
    else {
        minute = 9660 + minute;
        day = 6;
    }
    sec = s;
    return;
}

uint8_t changeThirtyMinBlock(int currTime) {
    return (currTime - (day*1440))/30;
}

void timerSetup (void) {
    T1CON = 0x00; //Stops the Timer1 and reset control reg.
    TMR1 = 0x00; //Clear contents of the timer register
    PR1 = 0x8CFF; //Load the Period register with the value 0x8CFF
    IPC0bits.T1IP = 0x01; //Setup Timer1 interrupt for desired priority level
    IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
    IEC0bits.T1IE = 1; //Enable Timer1 interrupts
    T1CON = 0x8000; //Start Timer1 with prescaler settings at 1:1 and
}

void __attribute__((__interrupt__, __shadow__, auto_psv)) _T1Interrupt(void) {
    static uint8_t i = 0;
    static uint8_t m = 1;

    if (i == 217){ //every second it blinks
        i = 0;
        if (sec == 60) { //every minute
            sec = 0;
            if (m == 60) { //every hour
                m = 0;
            } else {
                m++;
                if (minute == 10080) minute = 0;
                else minute++;
                if((minute % 25) == 0) updateFlag = FALSE;
            }
        } else {
            sec++;
            if(sec % 20 == 0){
                turnOffLED(0);
                turnOffLED(1);
            }
        }
    } else i++;
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
}

void setTimeout (uint16_t delay) {
    uint8_t extraMinute = 0;
    if ((sec + (delay % 60)) > 59) {
        extraMinute = 1;
        dSec = (sec + (delay % 60)) - 60;
    } else dSec =  sec + (delay % 60);
    dMinute = minute + (int)(delay / 60) + extraMinute;
}

uint8_t isTimeout (void) {
    if (sec >= dSec && minute >= dMinute) return 1;
    else return 0;
}

/*
 uint8_t getTime(uint8_t  i) {
 * if(i == 0){ secondTime = 0; return 0;}
 * else return secondTime;
 *
 * }
 */

/******************************************************************************/
/* UART                                                                       */
/******************************************************************************/

void UARTStartWifi (void) {
    U1BRG = BRGVAL;			// Set Baud Rate to 9600
    U1MODE = 0x8000;			// 8-bit data, no parity, 1st op-bit
    U1STA = 0x8400;			// clear status register
    U1MODEbits.UARTEN = 1;		// enable UART
    U1MODEbits.STSEL = 0;               // 1-stop bit
    U1MODEbits.PDSEL = 0;               // No Parity, 8-data bits
    U1MODEbits.ABAUD = 0;               // Auto-Baud disabled
    U1MODEbits.BRGH = 0;                // Standard-Speed mode
    IFS0bits.U1RXIF = 0;		// clear receive interrupt flag
    U1STAbits.UTXEN = 1;		// enable transmitter
    U1STAbits.UTXISEL0 = 0;             // Interrupt after one TX character is transmitted
    U1STAbits.UTXISEL1 = 0;
    U1STAbits.URXISEL0 = 0;             // Interrupt after one RX character is received
    U1STAbits.URXISEL1 = 0;
    IEC0bits.U1RXIE = 1;		// UART receiver interrupt enabled
    IEC0bits.U1TXIE = 1;		// UART transmitter interrupt enabled
    ANSBbits.ANSB1 = 0;                 // Turn pin from analog to digital
    ANSBbits.ANSB2 = 0;
}

void UARTStartFPR (void) {
    U2BRG = BRGVAL;			// Set Baud Rate to 9600
    U2MODE = 0x8000;			// 8-bit data, no parity, 1st op-bit
    U2STA = 0x8400;			// clear status register
    U2MODEbits.UARTEN = 1;		// enable UART
    U2MODEbits.STSEL = 0;               // 1-stop bit
    U2MODEbits.PDSEL = 0;               // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0;               // Auto-Baud disabled
    U2MODEbits.BRGH = 0;                // Standard-Speed mode
    IFS1bits.U2RXIF = 0;		// clear receive interrupt flag
    U2STAbits.UTXEN = 1;		// enable transmitter
    U2STAbits.UTXISEL0 = 0;             // Interrupt after one TX character is transmitted
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.URXISEL0 = 0;             // Interrupt after one RX character is received;
    U2STAbits.URXISEL1 = 0;
    IEC1bits.U2RXIE = 1;		// UART receiver interrupt enabled
    IEC1bits.U2TXIE = 1;		// UART transmitter interrupt enabled
}

void wifiSendChar (char data) {
        while (U1STAbits.UTXBF);        // transmit only if TX buffer is not full
        U1TXREG = data;			// send the character
        delay(1);
}

void FPRSendChar (uint8_t data) {
        while (U2STAbits.UTXBF);        // transmit only if TX buffer is not full
        U2TXREG = data;			// send the character
        delay(1);
}

uint8_t receiveMsgWifi () {
    uint8_t rec;
    if(U1STAbits.FERR == 1){
        return 0;
    }
    if(U1STAbits.OERR == 1) {
        U1STAbits.OERR = 0;
        return 0;
    }
    if(U1STAbits.URXDA == 1) {
        U1STAbits.URXDA = 0;
        rec = U1RXREG;
        return rec;
    }
    return 0;
}

uint8_t receiveMsgFPR () {
    uint8_t rec;
    if(U2STAbits.FERR == 1) {
        return 0;
    }
    if(U2STAbits.OERR == 1) {
        U2STAbits.OERR = 0;
        return 0;
    }
    if(U2STAbits.URXDA == 1) {
        U2STAbits.URXDA = 0;
        rec = U2RXREG;
        return rec;
    }
    return 0;
}

void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void) { // WiFi
    IFS0bits.U1TXIF = 0;    // Clear TX Interrupt flag
}

void __attribute__((interrupt, auto_psv)) _U2TXInterrupt(void) { // FPR
    IFS1bits.U2TXIF = 0;    // Clear TX Interrupt flag
}
