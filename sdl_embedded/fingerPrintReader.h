#include <stdint.h>

// FPR commands
#define ZERO 0x0000
#define BEST 0x0001
#define FPR_OPEN 0x01
#define FPR_LED 0x12
#define FPR_IS_PRESSED 0x26
#define FPR_CAPTURE 0x60
#define FPR_IDENTIFY 0x51
#define FPR_ENROLL_START 0x22
#define FPR_ENROLL1 0x23
#define FPR_ENROLL2 0x24
#define FPR_ENROLL3 0x25
#define FPR_DELETE_ALL 0x41
#define FPR_DELETE_ID 0x40

// FPR waiting states
#define PRESS 1
#define REMOVE 0

// FPR responses
#define ACK 0x30
#define NACK 0x31

// FPR State
#define NO_ERROR 0
#define PARSE_FPR_MSG 1
#define RECEIVE_FAIL 2
#define RETRY 3
#define FINGER_NOT_FOUND 0xFF

// Action
#define IDENTIFY 1
#define ENROLL 2
#define DELETE 3
#define OPEN_DOOR 4
#define TIMEOUT_LOCK 5
#define DOOR_IS_CLOSED 6
#define DOOR_IS_OPENED 7
#define DELETE_USER 8
#define LOCK_DOOR 9

extern uint8_t errFPR;
extern uint8_t FPRState;
extern uint8_t action;
extern uint16_t commandSent;
extern uint8_t receiveBufferFPR[12];
extern uint8_t pendingUserID;

void FPRStart (void);
void FPRStop (void);
void generateFPRHeader (void);
void getCheckSum (void);
uint8_t checkCheckSum ();
void sendFPRCommand (uint16_t command, uint32_t parameter);
void clearReceiveBuffer (void);
uint32_t getFPRResponseParameter (void);
uint16_t getFPRResponse (void);
void waitFinger (uint8_t place);
uint8_t isAuthorized (void);
void identify (void);
void enroll (void);
