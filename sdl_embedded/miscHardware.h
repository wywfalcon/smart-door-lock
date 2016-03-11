#include <stdint.h>
#include "debug.h"

#define FP 8000000                      //FP denotes the instruction cycle clock frequency (FOSC/2)
#define BAUDRATE 9600
#define BRGVAL ((FP/BAUDRATE)/16)-1

// Parse States
#define DEFAULT 0
#define PARSE_WIFI 1
#define PARSE_REQ 2
#define PARSE_PAGE 3

// Request Types
#define GET 7
#define POST_ENROLL 8
#define POST_DELETE 9
#define POST_SET_WIFI 10
#define GET_PAGE 11
#define PARSE_CONTENT 12
#define GET_CONTENT 13
#define PARSE_UPDATE 14

extern uint8_t systemReady;
extern uint8_t sec, dSec;
extern uint16_t minute, dMinute;
extern uint8_t day, dDay;
extern uint8_t productID;
extern uint8_t updateFlag;


void delay (uint16_t t);
void setCurrTime(int d, int h, int m, int s);
uint8_t changeThirtyMinBlock(int currTime);
void UARTStartWifi (void);
void UARTStartFPR (void);
void timerSetup (void);
void setTimeout (uint16_t delay);
uint8_t isTimeout (void);
uint8_t receiveMsgWifi ();
uint8_t receiveMsgFPR ();
void wifiSendChar (char data);
void FPRSendChar (uint8_t data);
void inputStart ();
uint8_t buttonIsPressed ();
uint8_t switchIsOn ();
void lockDeadBolt(uint8_t lockIt);
void turnOnFPR(int turnOn);
void turnOnWifiChip(int turnOn);
void setTimeout (uint16_t delay);
uint8_t isTimeout (void);
