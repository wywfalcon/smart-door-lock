#include <p24fxxxx.h>
#include <string.h>
#include "wifi.h"
#include "database.h"
#include "miscHardware.h"
#include "fingerPrintReader.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define PRODUCT_ID "1.1.1.1"

uint8_t reqType = DEFAULT;
uint8_t reqParseState = DEFAULT;
uint8_t delimStart, delimEnd, delimEnd2;
uint8_t loadingContent = FALSE;
uint8_t networkSet = FALSE;
char bufferWifi[WIFI_BUFFER_SIZE];
char ssid[INFO_BUFFER_SIZE];
char pswd[INFO_BUFFER_SIZE];
char addBuf[3], delBuf[3];
char addStr[3*DATABASE_SIZE];
char delStr[3*DATABASE_SIZE];
uint8_t mode;
uint8_t wifiEnrollProgress = 0;
uint8_t contentParseState = 0;
char buf[20];
struct tm tm;
time_t t;

void WifiStart (int8_t m) {
    turnOnWifiChip(TRUE);
    delay(1000);                    //when the wifi chip turns on, the reset messege shows up so it's good to have a delay to clear the message first
    mode = m;
    if (m == SERVER_MODE) {
        refreshConnection();
    } else if (m == CLIENT_MODE) {
        setupClientMode();
    }
}

void refreshConnection (void) {
    UARTStartWifi();
    delay(50);
    sendWifiCommand("AT+CIPMUX=1", 1);
    delay(50);
    sendWifiCommand("AT+CIPSERVER=1,80", 1);
    delay(50);
    sendWifiCommand("AT+CWQAP", 1);
    delay(50);
    reqParseState = DEFAULT;
}

void setupClientMode (void) {
    uint8_t errFlag = FALSE;
    UARTStartWifi();                //enables the UART even in client mode during the update process
    networkSet = TRUE;
    sendWifiCommand("AT+CIPSERVER=0", 1);
    delay(50);
    sendWifiCommand("AT+RST", 1);
    delay(2000);
    UARTStartWifi();
    sendWifiCommand("AT+CIPMUX=1", 1);
    delay(100);
    networkSet = FALSE;
    setTimeout(15);
    sendNetworkInfo();
    uint8_t i = 1;
    while (!networkSet && errFlag != TRUE) {
        if (isTimeout()) {
            setTimeout(15);
            sendNetworkInfo();
            i++;
            if(i >= 4) errFlag = TRUE;
        }
    }
    delay(50);
    sendWifiCommand("AT+CIPMUX=1", 1);
    delay(100);
    networkSet = FALSE;
    setTimeout(10);
    sendWifiCommand("AT+CIPSTART=0,\"TCP\",\"elegant-moment-843.appspot.com\",80",1);
    while (!networkSet && errFlag != TRUE) {
        if (isTimeout()) {
            setTimeout(10);
            sendWifiCommand("AT+CIPSTART=0,\"TCP\",\"elegant-moment-843.appspot.com\",80",1);
            i++;
            if(i >= 4) errFlag = TRUE;
        }
    }
    turnAllLEDOff();
    delay(50);
}

void sendNetworkInfo (void) {
    sendWifiCommand("AT+CWJAP=\"", 0);
    sendWifiCommand(ssid, 0);
    sendWifiCommand("\",\"", 0);
    sendWifiCommand(pswd, 0);
    sendWifiCommand("\"", 1);
}

void sendWifiCommand (char *command, uint8_t numOfBreaks) {  //for WIFI
    int i;
    turnAllLEDOff ();
    for (i = 0; command[i] != NULL; i++) {
        wifiSendChar(command[i]);
        toggleLED(0);                                        //Acts like blue light for wifi chip to show sending
    }
    for (i = 0; i < numOfBreaks; i++) {
        wifiSendChar('\r');
        wifiSendChar('\n');
    }
    turnAllLEDOff ();
}

void sendUpdate (void) {
    int i = 0, l = 0;
    char contentLength[5], ATlength[5];
    char *l1 = "AT+CIPSEND=0,",
         *l2 = "POST /admin HTTP/1.1",
         *l3 = "HOST: elegant-moment-843.appspot.com",
         *l4 = "Content-Length: ",
         *l5 = "productID=",
         *l6 = "&add=",
         *l7 = "&del=";
    loadAddListString();
    loadDelListString();
    l += strlen(l2) + 2;
    l += strlen(l3) + 2;
    l += strlen(l4);
    l += strlen(contentLength) + 4;
    l += strlen(l5) + strlen(PRODUCT_ID); i += strlen(l5) + strlen(PRODUCT_ID);
    if (addStr[0] != NULL) { i += strlen(l6); l += strlen(l6); }
    if (addStr[0] != NULL) { i += strlen(addStr); l += strlen(addStr); }
    if (delStr[0] != NULL) { i += strlen(l7); l += strlen(l7); }
    if (delStr[0] != NULL) { i += strlen(delStr); l += strlen(delStr); }
    l += 6;
    itoa(ATlength, l, 10);
    itoa(contentLength, i, 10);
    sendWifiCommand(l1, 0);
    sendWifiCommand(ATlength, 1);
    sendWifiCommand(l2, 1);
    sendWifiCommand(l3, 1);
    sendWifiCommand(l4, 0);
    sendWifiCommand(contentLength, 2);
    sendWifiCommand(l5, 0);
    sendWifiCommand(PRODUCT_ID, 0);
    if (addStr[0] != NULL) {
        sendWifiCommand(l6, 0);
        sendWifiCommand(addStr, 0);
    }
    if (delStr[0] != NULL) {
        sendWifiCommand(l7, 0);
        sendWifiCommand(delStr, 0);
    }
    sendWifiCommand("", 3);
}

void loadAddListString (void) {
    uint8_t i = 0;
    while (newEnrolled[i] != 0xFF) {
        itoa(addBuf,newEnrolled[i],10);
        if (i == 0) strcpy(addStr, addBuf);
        else {
            strcat(addStr, ",");
            strcat(addStr, addBuf);
        }
        i++;
    }
    if (i == 0) addStr[0] = NULL;
}

void loadDelListString (void) {
    uint8_t i = 0;
    while (newDeleted[i] != 0xFF) {
        itoa(delBuf,newDeleted[i],10);
        if (i == 0) strcpy(delStr, delBuf);
        else {
            strcat(delStr, ",");
            strcat(delStr, delBuf);
        }
        i++;
    }
    if (i == 0) delStr[0] = NULL;
}

void sendResponse (char *msg) {
    int i, l = 0;
    char contentLength[5], ATlength[5];
    for (i = 0; msg[i] != NULL; i++);
    itoa(contentLength, i-1, 10);
    char *l1 = "AT+CIPSEND=0,",
         *l2 = "HTTP/1.1 200 OK",
         *l3 = "Content-Type: text/html; charset=utf-8",
         *l4 = "Content-Length: ";
    l += strlen(l2) + 2;
    l += strlen(l3) + 2;
    l += strlen(l4);
    l += strlen(contentLength) + 4;
    l += strlen(msg);
    itoa(ATlength, l, 10);
    sendWifiCommand(l1, 0);
    sendWifiCommand(ATlength, 1);
    sendWifiCommand(l2, 1);
    sendWifiCommand(l3, 1);
    sendWifiCommand(l4, 0);
    sendWifiCommand(contentLength, 2);
    sendWifiCommand(msg, 1);
}

void sendPage (void) {
    delay(1000);
    sendResponse(
        "<html lang=\"en\"><head><title>Local SDL</title><style>*{font-family: Trebuchet MS"
        ";color: white;}html,body{margin: 0 auto;min-width: 600px;min-height: 100%;backgr"
        "ound: linear-gradient(white,#E35930);}table{table-layout: fixed;margin: auto;}.s"
        "tretch,input[type=\"text\"],input[type=\"password\"]{width: 100%;}.center{text-align"
        ": center;}.right{text-align: right;}td,input{background: rgba(0,0,0,.3);padding:"
        " 3%;}.label{padding: 3%;}.bigFont{font-size: 400%;}#content{font-size: 300%;}inp"
        "ut{font-size: 115%;}.btn{cursor: pointer;}.btn td:hover{background: rgba(0,0,0,."
        "5);}.btn td.selected{background: rgba(0,0,0,.7);}</style></head><body><table cla"
        "ss=\"stretch\"><tr class=\"bigFont center\"><td colspan=\"3\">Smart Door Lock</td></tr"
        "><tr class=\"bigFont btn center\"><td onclick=\"add()\">+</td><td onclick=\"del()\">&#"
        "x2212</td><td onclick=\"set()\">Wi-Fi</td></tr><tr><td id=\"content\" colspan=\"3\" cl"
        "ass=\"smallFont\">Select one of the options above</td></tr></table><script>var ctn"
        "=document.getElementById('content'),xhr=new XMLHttpRequest();var add=function(){"
        "req('POST','/enroll',null);},del=function(){req('POST','/delete','');},set=funct"
        "ion(){ctn.innerHTML='<div class=\"label\">Network Name<input id=\"ssid\" type=\"text\""
        "></div><div class=\"label\">Password<input id=\"netPW\" type=\"password\"></div><div c"
        "lass=\"label right\"><input onclick=\"update()\" type=\"button\" value=\"Update\"></div>"
        "';},update=function(){var ssid=document.getElementById('ssid'),netPW=document.ge"
        "tElementById('netPW');req('POST','/setWifi','nssid='+ssid.value+'&passw='+netPW."
        "value+'$');},req=function(method,page,data){xhr.open(method,page,true);xhr.onrea"
        "dystatechange=function(){if (xhr.readyState===4){var to,rt=xhr.responseText;ctn."
        "innerHTML='<p>'+rt.replace('$','')+'</p>';if (rt.ind"
        "exOf('$')===-1 && rt.indexOf('Link')===-1) to=setTimeout(function(){req(method,p"
        "age,data)},1000);}};xhr.send(data);};</script></body></html>"
    );
}

void sendEnrollInstructions (void) {
    delay(1000);
    switch (wifiEnrollProgress) {
        case 0:
            sendResponse("1. Place finger ");
            break;
        case 1:
            sendResponse("2. Remove finger ");
            break;
        case 2:
            sendResponse("3. Place finger ");
            break;
        case 3:
            sendResponse("4. Remove finger ");
            break;
        case 4:
            sendResponse("5. Place finger ");
            break;
        case 5:
            itoa(buf,pendingUserID,10);
            strcat(buf, " is enrolled$ ");
            delay(1000);
            sendResponse(buf);
            wifiEnrollProgress = 0;
            reqType = DEFAULT;
            return;
    };
    wifiEnrollProgress++;
}

void sendEnrollError (void) {
    delay(1000);
    sendResponse("Error: Try again.$ ");
    wifiEnrollProgress = 0;
    reqType = DEFAULT;
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) { // WiFi
    static uint8_t currID = -1;
    static uint8_t schIndex = 0;
    if (!systemReady) { IFS0bits.U1RXIF = 0; return; }
    uint8_t rec = receiveMsgWifi();
    static uint8_t i;
    switch (reqParseState) {
        case DEFAULT:
            i = 0;
            if (rec == '+' && mode == SERVER_MODE) {
                memset(bufferWifi, 0, sizeof(bufferWifi));
                reqParseState = PARSE_WIFI;
                delimEnd = ',';
            } else if (rec == 'O' && mode == CLIENT_MODE) {
                memset(bufferWifi, 0, sizeof(bufferWifi));
                reqParseState = PARSE_WIFI;
            }
              else if (rec == '+' && mode == CLIENT_MODE) {
                loadingContent = FALSE;
                memset(bufferWifi, 0, sizeof(bufferWifi));
                reqParseState = PARSE_WIFI;
                delimStart = 'H';
                delimEnd = '/';
//toggleLED(5);
            }
            break;
        case PARSE_WIFI:

            if (rec == 'K' && mode == CLIENT_MODE && delimEnd != ' ') {
                networkSet = TRUE;
                reqParseState = DEFAULT;
                IFS0bits.U1RXIF = 0;
                return;
            } else if(rec == delimStart && mode == CLIENT_MODE && loadingContent == FALSE){
//turnOnLED(7);
                loadingContent = TRUE;
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if ((rec == delimEnd || i >= WIFI_BUFFER_SIZE) && mode == CLIENT_MODE) {
//toggleLED(6);
                    if (strncmp(bufferWifi, "TTP", 3) == 0) {
///turnOnLED(8);
                        reqParseState = GET_CONTENT;
                        loadingContent = FALSE;
                        delimStart = 'r'; delimEnd = 'c';  //looking for productID
                        IFS0bits.U1RXIF = 0;
                        return;
                } else{
                        reqParseState = DEFAULT;
                        IFS0bits.U1RXIF = 0;
                        return;
                }
            } else if ((rec == delimEnd || i >= 4) && mode == SERVER_MODE) {
                i = 0;
                if (strncmp(bufferWifi, "+IPD", 4) == 0) {
                    reqParseState = PARSE_REQ;
                    delimStart = ':'; delimEnd = ' ';
                } else reqParseState = DEFAULT;
                memset(bufferWifi, 0, sizeof(bufferWifi));
                IFS0bits.U1RXIF = 0;
                return;
            }
            break;
        case PARSE_REQ:
            if (mode != SERVER_MODE) { reqParseState = DEFAULT; IFS0bits.U1RXIF = 0; return; }

            if (rec == delimStart && loadingContent == FALSE) {
                loadingContent = TRUE;
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd || i >= 5) {
                loadingContent = FALSE;
                if (strncmp(bufferWifi, "GET", 3) == 0) {
                    reqType = GET;
                    reqParseState = PARSE_PAGE;
                    delimStart = '/'; delimEnd = ' ';
                } else if (strncmp(bufferWifi, "POST", 4) == 0) {
                    reqParseState = PARSE_PAGE;
                    delimStart = '/'; delimEnd = ' ';
                } else {
                    reqParseState = DEFAULT;
                   
                }
                memset(bufferWifi, 0, sizeof(bufferWifi));
                IFS0bits.U1RXIF = 0;
                return;
            }
            break;
        case PARSE_PAGE:
            if (rec == delimStart && loadingContent == FALSE) {
                loadingContent = TRUE;
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd && i == 0 && reqType == GET) {
                reqType = GET_PAGE;
                reqParseState = DEFAULT;
                loadingContent = FALSE;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd || i >= 8) {
                loadingContent = FALSE;
                if (strncmp(bufferWifi, "enroll", 6) == 0) {
                    reqType = POST_ENROLL;
                    reqParseState = DEFAULT;
                    IFS0bits.U1RXIF = 0;
                    return;
                } else if (strncmp(bufferWifi, "delete", 6) == 0) {
                    reqType = POST_DELETE;
                    reqParseState = DEFAULT;
                    IFS0bits.U1RXIF = 0;
                    return;
                } else if (strncmp(bufferWifi, "setWifi", 7) == 0) {
                    reqParseState = GET_CONTENT;
                    delimStart = 'n'; delimEnd = 'd';
                    loadingContent = FALSE;
                    IFS0bits.U1RXIF = 0;
                    return;
                } else {
                    reqType = DEFAULT;
                    reqParseState = DEFAULT;
                    IFS0bits.U1RXIF = 0;
                    return;
                }
            }
            break;
        case GET_CONTENT:
            if (rec == delimStart && loadingContent == FALSE) {
//turnOnLED(9);
                loadingContent = TRUE;
                memset(bufferWifi, 0, sizeof(bufferWifi));
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
                loadingContent = FALSE;
                if (strstr(bufferWifi, "ssi") != NULL && mode == SERVER_MODE) {
                    delimEnd = '$';
                    reqParseState = PARSE_CONTENT;
                } else if (strcmp(bufferWifi, "odu") == 0 && mode == CLIENT_MODE) {
//turnOnLED(8);
                    reqParseState = PARSE_UPDATE;
                    contentParseState = 0;
                    delimStart = '='; delimEnd = '&';
                    loadingContent = FALSE;
                }
               // loadingContent = TRUE;
                i = 0;
            }
            break;
        case PARSE_CONTENT:
            if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
                loadingContent = FALSE;
                reqType = POST_SET_WIFI;
               // saveWifiInfo();
                reqParseState = DEFAULT;
            }
            break;
        case PARSE_UPDATE:
            if (rec == delimStart && loadingContent == FALSE) {
                loadingContent = TRUE;
                memset(bufferWifi, 0, sizeof(bufferWifi));
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == '$' || rec == delimEnd || i >= WIFI_BUFFER_SIZE || (contentParseState == 5 && rec != '$')) {
                switch (contentParseState) {
                    case 1: //day
                        day = atoi(bufferWifi);
                        delimEnd = '.';
                        loadingContent = FALSE;
                        contentParseState = 2;
                        break;
                    case 2: // time
//turnOnLED(8);
                        updateTime(bufferWifi);
                        delimEnd = '&';
                        loadingContent = FALSE;
                        contentParseState = 6;
                        break;
                    case 6:
                        loadingContent = FALSE;
                        contentParseState = 3;
                        break;

                    case 0: // productID
                        if(strcmp(bufferWifi, PRODUCT_ID) != 0 ){
//turnOnLED(9);
                            reqParseState = DEFAULT;
                            IFS0bits.U1RXIF = 0;
                            return;
                        }
                        delimEnd = '&';
                        loadingContent = FALSE;
                        contentParseState = 1;
                        break;
                    case 3: // del  //serach from = to , or &
//turnOnLED(9);
                         if (rec == '$'){
                            reqParseState = DEFAULT;
                            IFS0bits.U1RXIF = 0;
                            break;
                        }
                        if(i > 1 && delimStart != ';') deleteID(bufferWifi);
                        delimEnd = ',';
                        loadingContent = FALSE;
                        contentParseState = 4;
                        break;
                    case 4: // sch
                        if (rec == '$'){
                            reqParseState = DEFAULT;
                            IFS0bits.U1RXIF = 0;
                            break;
                        }
                        currID = atoi(bufferWifi);
                        i = 0;
                        schIndex = 0;
                        contentParseState = 5;
                        break;
                    case 5:
                        if (rec == '$'){
                            reqParseState = DEFAULT;
                            IFS0bits.U1RXIF = 0;
                            break;
                        }
                        updateSch(schIndex++, rec, currID);
                        if(schIndex == 48){
                            delimStart = ';';
                            delimEnd = ',';
                            contentParseState = 3;
                            schIndex = 0;
                            loadingContent = FALSE;
                        }
                        break;

                }
            } else if (rec == '$'){
                reqParseState = DEFAULT;
            }
            break;
    }
    bufferWifi[i++] = rec;
    IFS0bits.U1RXIF = 0;
}

int16_t searchForChar (char *string, char c, uint8_t startPosition) {
    uint8_t i;
    for (i = startPosition; i < strlen(string); i++) {
        if (string[i] == c) return i;
    }
    return -1;
}

void saveWifiInfo (void) {
    uint8_t j, start, end;
    start = searchForChar(bufferWifi, '=', 0)+1;
    end = searchForChar(bufferWifi, '&', start);
    for (j = 0; j < end - start; j++) {
        ssid[j] = bufferWifi[j + start];
    }
    start = searchForChar(bufferWifi, '=', end+1)+1;
    end = searchForChar(bufferWifi, '$', start);
    for (j = 0; j < end - start; j++) {
        pswd[j] = bufferWifi[j + start];
    }
}

void updateTime(char input[]){
    int time[3];
    int index = 0;
    char *token = strtok(input, ":");
    while(token){
        time[index++] = atoi(token);
        token = strtok(NULL,":");
    }
    setCurrTime(day, time[0], time[1], time[2]);
}

void deleteID(char input[]){
    char *token = strtok(input, ",");
    while(token){
        deleteUser(atoi(token));
        token = strtok(NULL,",");
    }
    action = DELETE_USER;
}
