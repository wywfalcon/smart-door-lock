#include <p24fxxxx.h>
#include <string.h>
#include "wifi.h"
#include "fingerPrintReader.h"
#include "database.h"
#include "miscHardware.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t sendBufferFPR[12];
uint8_t receiveBufferFPR[12];
uint16_t commandSent;
uint8_t waitPlace = PRESS;
uint8_t action = DEFAULT;
int8_t msgProgress = -1;
uint8_t enrollProgress = 0;
uint8_t pendingUserID;
uint8_t FPRState = DEFAULT;
uint8_t errFPR = NO_ERROR;

void FPRStart (void) {
    turnOnFPR(TRUE);
    UARTStartFPR();
    delay(200);
    sendFPRCommand(FPR_LED, 1);
    delay(200);
}

void FPRStop (void) {
    sendFPRCommand(FPR_LED, 0);
    delay(200);
    turnOnFPR(FALSE);
}

void generateFPRHeader (void) {
    sendBufferFPR[0] = 0x55;
    sendBufferFPR[1] = 0xAA;
    sendBufferFPR[2] = 0x01;
    sendBufferFPR[3] = 0x00;
}

void getCheckSum (void) {
    uint8_t i; uint16_t sum = 0x0000;
    for (i = 0; i < 10; i++) sum += sendBufferFPR[i];
    sendBufferFPR[10] = sum & 0x00FF;
    sendBufferFPR[11] = (sum >> 8) & 0x00FF;
}

uint8_t checkCheckSum () {
    uint8_t i; uint16_t sum = 0x00;
    for (i = 0; i < 10; i++) {
        sum += receiveBufferFPR[i];
    }
    if ((sum & 0xFF00) >> 8 == receiveBufferFPR[11] &&
         (sum & 0x00FF) == receiveBufferFPR[10]) {
        return 1;
    }
    return 0;
}

void sendFPRCommand (uint16_t command, uint32_t parameter) {
    FPRState = DEFAULT;
    clearReceiveBuffer();
    commandSent = command;
    generateFPRHeader();
    sendBufferFPR[4] = parameter & 0x000000FF;
    sendBufferFPR[5] = (parameter & 0x0000FF00) >> 8;
    sendBufferFPR[6] = (parameter & 0x00FF0000) >> 16;
    sendBufferFPR[7] = (parameter & 0xFF000000) >> 24;
    sendBufferFPR[8] = command & 0x00FF;
    sendBufferFPR[9] = (command >> 8) & 0x00FF;
    getCheckSum();
    uint8_t i;
    for (i = 0; i < 12; i++) {
        FPRSendChar(sendBufferFPR[i]);
    }
    delay(300);
}

void __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void) {  //Fingerprint reader
    if (!systemReady) { IFS1bits.U2RXIF = 0; return; }
    uint8_t rec = receiveMsgFPR();
    switch (msgProgress) {
        case -1:
            if (rec != 0x55) errFPR = RECEIVE_FAIL;
            break;
        case 0:
            if (rec != 0xAA) errFPR = RECEIVE_FAIL;
            break;
    }
    if (msgProgress == 10) {
        msgProgress = -1;
        FPRState = PARSE_FPR_MSG;
    } else if (errFPR) {
//toggleLED(1);
        msgProgress = -1;
        errFPR = DEFAULT;
        FPRState = DEFAULT;
    } else msgProgress++;
    if (msgProgress >= 0) receiveBufferFPR[msgProgress] = rec;
    IFS1bits.U2RXIF = 0;
}

void clearReceiveBuffer (void) {
    uint8_t i;
    for (i = 0; i < 12; i++) {
        receiveBufferFPR[i] = 0;
    }
}

//char debug[20]; uint8_t i;
//for (i = 0; i < 12; i++) {
//    sendWifiCommand(itoa(debug,receiveBufferFPR[i],16), 0);
//}

uint32_t getFPRResponseParameter (void) {
    uint32_t parameter = 0;
    parameter |= receiveBufferFPR[7];
    parameter <<= 8;
    parameter |= receiveBufferFPR[6];
    parameter <<= 8;
    parameter |= receiveBufferFPR[5];
    parameter <<= 8;
    parameter |= receiveBufferFPR[4];
    return parameter;
}

uint16_t getFPRResponse (void) {
    uint16_t response = 0;
    response |= receiveBufferFPR[9];
    response <<= 8;
    response |= receiveBufferFPR[8];
    return response;
}

void waitFinger (uint8_t place) {
    if (place) {
        while (getFPRResponseParameter() != 0) {
            sendFPRCommand(FPR_IS_PRESSED, ZERO);

        }
    } else {
        while (getFPRResponseParameter() == 0) {
            sendFPRCommand(FPR_IS_PRESSED, ZERO);

        }
    }
}

uint8_t isAuthorized (void) {
    int num = getFPRResponseParameter();
    if (receiveBufferFPR[8] == ACK && (((userList[num].schedule[changeThirtyMinBlock(minute)] >> (6 - day)) & 0x01) == 1)) return OPEN_DOOR;
    else{
        return FINGER_NOT_FOUND;
    }
}

void identify (void) {
    if (commandSent == FPR_IS_PRESSED) {
        waitFinger(PRESS);
        sendFPRCommand(FPR_CAPTURE, ZERO);
    } else if (commandSent == FPR_CAPTURE) {
        sendFPRCommand(FPR_IDENTIFY, ZERO);
        receiveBufferFPR[8] = 0;
    } else if (commandSent == FPR_IDENTIFY) {
        while(receiveBufferFPR[8] == 0){}
        action = isAuthorized();
        FPRStop();
    }
}

void enroll (void) {
    //fingerEnrollFlag = 1;
    if (getFPRResponse() == NACK) {
        sendEnrollError();
        enrollProgress = 0;
        commandSent = DEFAULT;
        FPRState = DEFAULT;
        FPRStop();
        return;
    }
    if (commandSent == FPR_ENROLL_START) {
        sendFPRCommand(FPR_IS_PRESSED, ZERO);
        waitPlace = PRESS;
    } else if (commandSent == FPR_IS_PRESSED) {
        sendEnrollInstructions();
        waitFinger(waitPlace);
        if (waitPlace == PRESS) {
            sendFPRCommand(FPR_CAPTURE, BEST);
        } else {
            waitPlace = PRESS;
            sendFPRCommand(FPR_IS_PRESSED, ZERO);
        }
    } else if (commandSent == FPR_CAPTURE) {
        switch (enrollProgress) {
            case 0: sendFPRCommand(FPR_ENROLL1, ZERO); break;
            case 1: sendFPRCommand(FPR_ENROLL2, ZERO); break;
            case 2: sendFPRCommand(FPR_ENROLL3, ZERO); break;
        }
    } else if (commandSent == FPR_ENROLL1 || commandSent == FPR_ENROLL2) {
        waitPlace = REMOVE;
        enrollProgress++;
        sendFPRCommand(FPR_IS_PRESSED, ZERO);
    } else if (commandSent == FPR_ENROLL3) {
        sendEnrollInstructions();
        enrollProgress = 0;
        commandSent = DEFAULT;
        FPRState = DEFAULT;
        saveNewUser(FALSE, pendingUserID);
        FPRStop();
       // fingerEnrollFlag = 0;
    }
}
