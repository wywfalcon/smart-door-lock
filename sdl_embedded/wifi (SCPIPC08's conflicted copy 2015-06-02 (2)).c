#include <p24fxxxx.h>
#include <string.h>
#include "wifi.h"
#include "database.h"
#include "miscHardware.h"
#include "fingerPrintReader.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define PRODUCT_ID "1.1.1.1"

uint8_t reqType = DEFAULT;
uint8_t reqParseState = DEFAULT;
uint8_t delimStart, delimEnd;
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

void WifiStart (int8_t m) {
    turnOnWifiChip(TRUE);
    delay(500);                    //when the wifi chip turns on, the reset messege shows up so it's good to have a delay to clear the message first
    mode = m;
    if (m == SERVER_MODE) {
        refreshConnection();
    } else if (m == CLIENT_MODE) {
        setupClientMode();
    }
}

void refreshConnection (void) {
    UARTStartWifi();
    sendWifiCommand("AT+CIPMUX=1", 1);
    delay(50);
    sendWifiCommand("AT+CIPSERVER=1,80", 1);
    delay(50);
}

void setupClientMode (void) {
    UARTStartWifi();                //enables the UART even in client mode during the update process
    networkSet = TRUE;
    sendWifiCommand("AT+CIPSERVER=0", 1);
    delay(50);
    sendWifiCommand("AT+RST", 1);
    delay(1500);
    UARTStartWifi();
    sendWifiCommand("AT+CIPMUX=1", 1);
    delay(50);
    networkSet = FALSE;
    setTimeout(15);
    sendNetworkInfo();
    while (!networkSet) {
        if (isTimeout()) {
            setTimeout(15);
            sendNetworkInfo();
        }
    }
    delay(50);
    networkSet = FALSE;
    setTimeout(10);
    sendWifiCommand("AT+CIPSTART=0,\"TCP\",\"elegant-moment-843.appspot.com\",80",1);
    while (!networkSet) {
        if (isTimeout()) {
            setTimeout(10);
            sendWifiCommand("AT+CIPSTART=0,\"TCP\",\"elegant-moment-843.appspot.com\",80",1);
        }
    }
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
        "<html lang=\"en\"><head><title>Local SDL</title><style>*{font-family:Trebuchet MS;"
        "color:white;}html,body{margin:0 auto;min-width:600px;min-height:100%;background:"
        "linear-gradient(white,#E35930);}table{table-layout:fixed;margin:auto;}.stretch,i"
        "nput[type=\"text\"],input[type=\"password\"]{width:100%;}.center{text-align:center;}"
        ".right{text-align:right;}td,input{background:rgba(0,0,0,.3);padding:3%;}.label{p"
        "adding:3%;}.bigFont{font-size:400%;}#content{font-size:300%;}input{font-size:115"
        "%;}.btn{cursor:pointer;}.btn td:hover{background:rgba(0,0,0,.5);}.btn td.selecte"
        "d{background:rgba(0,0,0,.7);}</style></head><body><table class=\"stretch\"><tr cla"
        "ss=\"bigFont center\"><td colspan=\"3\">Smart Door Lock</td></tr><tr class=\"bigFont "
        "btn center\"><td onclick=\"addUser()\">+</td><td onclick=\"deleteUser()\">&#x2212</td"
        "><td onclick=\"setWifi()\">Wi-Fi</td></tr><tr><td id=\"content\" colspan=\"3\" class=\""
        "smallFont\">Select one of the options above</td></tr></table><script>var content="
        "document.getElementById('content'),xhr=new XMLHttpRequest();var addUser=function"
        "(){req('POST','/enroll',null);},deleteUser=function(){req('POST','/delete','id=1"
        "99');},setWifi=function(){content.innerHTML='<div class=\"label\">Network Name<inp"
        "ut id=\"ssid\" type=\"text\"></div>'+'<div class=\"label\">Password<input id=\"netPW\" t"
        "ype=\"password\"></div>'+'<div class=\"label right\"><input onclick=\"updateWifi()\" t"
        "ype=\"button\" value=\"Update\"></div>';},updateWifi=function(){var ssid=document.ge"
        "tElementById('ssid'),netPW=document.getElementById('netPW');req('POST','/setWifi"
        "','nssid='+ssid.value+'&passw='+netPW.value+'$');},req=function(method,page,data){xh"
        "r.open(method,page,true);xhr.onreadystatechange=function(){if(xhr.readyState===4"
        "){content.innerHTML='<p>'+xhr.responseText.replace('$','')+'</p>';if(xhr.respons"
        "eText.indexOf('$')===-1&&xhr.responseText.indexOf('Link')===-1)setTimeout(functio"
        "n(){req(method,page,data)},1000);}};xhr.send(data);};</script></body></html> "
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
    sendResponse("Error: Try again. ");
    wifiEnrollProgress = 0;
    reqType = DEFAULT;
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) { // WiFi
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
            } else if (rec == 'L' && mode == CLIENT_MODE) {
                memset(bufferWifi, 0, sizeof(bufferWifi));
                reqParseState = PARSE_WIFI;
                delimEnd = 'd';
            } else if (rec == '2' && mode == CLIENT_MODE) {
                memset(bufferWifi, 0, sizeof(bufferWifi));
                reqParseState = PARSE_WIFI;
                delimEnd = ' ';
            }
            break;
        case PARSE_WIFI:
            if (rec == 'K' && mode == CLIENT_MODE) {
                networkSet = TRUE;
                reqParseState = DEFAULT;
                IFS0bits.U1RXIF = 0;
                return;
            } else if ((rec == delimEnd || i >= WIFI_BUFFER_SIZE) && mode == CLIENT_MODE) {
                if (strncmp(bufferWifi, "Linke", 5) == 0) {
                    networkSet = TRUE;
                    reqParseState = DEFAULT;
                    IFS0bits.U1RXIF = 0;
                    return;
                } else if (strncmp(bufferWifi, "00", 2) == 0) {
                    reqParseState = GET_CONTENT;
                    IFS0bits.U1RXIF = 0;
                    delimStart = 'd'; delimEnd = 'y';
                }
//                else if (strncmp(bufferWifi, "a", 1) == 0) {
//                    //networkSet = TRUE;
//                    //go from there?
//                    reqParseState = DEFAULT;
//                    IFS0bits.U1RXIF = 0;
//                    return;
//                }
            } else if ((rec == delimEnd || i >= WIFI_BUFFER_SIZE) && mode == SERVER_MODE) {
                i = 0;
                if (strncmp(bufferWifi, "+IPD", 4) == 0) {
                    reqParseState = PARSE_REQ;
                    delimStart = ':'; delimEnd = ' ';
                } else reqParseState = DEFAULT;
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
            } else if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
                loadingContent = FALSE;
                if (strncmp(bufferWifi, "GET", 3) == 0) {
                    reqType = GET;
                    reqParseState = PARSE_PAGE;
                    delimStart = '/'; delimEnd = ' ';
                } else if (strncmp(bufferWifi, "POST", 4) == 0) {
                    reqParseState = PARSE_PAGE;
                    delimStart = '/'; delimEnd = ' ';
                }
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
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
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
                loadingContent = TRUE;
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
                loadingContent = FALSE;
                if (strstr(bufferWifi, "ssi") != NULL && mode == SERVER_MODE) {
                    delimEnd = '$';
                    reqParseState = PARSE_CONTENT;
                } else if (strcmp(bufferWifi, "a") == 0 && mode == CLIENT_MODE) {
                    reqParseState = PARSE_UPDATE;
                    delimStart = '='; delimEnd = '&';
                }
                loadingContent = TRUE;
                i = 0;
            }
            break;
        case PARSE_CONTENT:
            if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
                loadingContent = FALSE;
                reqType = POST_SET_WIFI;
                reqParseState = DEFAULT;
            }
            break;
        case PARSE_UPDATE:
            if (rec == delimStart) {
                i = 0;
                IFS0bits.U1RXIF = 0;
                return;
            } else if (rec == delimEnd || i >= WIFI_BUFFER_SIZE) {
                switch (contentParseState) {
                    case 0: //day
                        day = atoi(bufferWifi);
                        delimEnd = '.';
                        break;
                    case 1: // time
                        updateDate();
                        break;
                    case 2: // productID
                        break;
                    case 3: // del
                        break;
                    case 4: // sch
                        break;
                }
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

void updateDate (void) {
    
}
