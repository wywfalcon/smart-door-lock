#include <stdint.h>

#define TRUE 1
#define FALSE 0

#define SERVER_MODE 1
#define CLIENT_MODE 2
#define WIFI_BUFFER_SIZE 55
#define INFO_BUFFER_SIZE 20

extern uint8_t reqType;
extern uint8_t reqParseState;
extern char bufferWifi[WIFI_BUFFER_SIZE];
extern char ssid[INFO_BUFFER_SIZE];
extern char pswd[INFO_BUFFER_SIZE];
extern uint8_t wifiEnrollProgress;

void updateTime(char input[]);
void WifiStart (int8_t mode);
void refreshConnection(void);
void setupClientMode(void);
void saveWifiInfo (void);
void sendNetworkInfo (void);
void sendWifiCommand (char *command, uint8_t numOfBreaks);
void sendUpdate (void);
void sendResponse (char *msg);
void sendPage (void);
void sendEnrollInstructions (void);
void sendEnrollError (void);
void deleteID(char input[]);

void loadAddListString (void);
void loadDelListString (void);
int16_t searchForChar (char *string, char c, uint8_t startPosition);
