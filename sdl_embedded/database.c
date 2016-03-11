#include <p24fxxxx.h>
#include <string.h>
#include "database.h"
#include <stdint.h>
#include "fingerPrintReader.h"

User userList[DATABASE_SIZE];
uint8_t newEnrolled[DATABASE_SIZE] = {0xFF};
uint8_t newDeleted[DATABASE_SIZE] = {0xFF};

void deleteDatabase (void) {
    memset(userList, 0, sizeof(User));
    clearList();
}

void clearList (void) {
    memset(newEnrolled, 0xFF, sizeof(newEnrolled));
    memset(newDeleted, 0xFF, sizeof(newDeleted));
}

void saveNewUser (uint8_t isAdmin, uint8_t id) {
    userList[id].enroll = 0b1;
    userList[id].admin = isAdmin ? 0b1 : 0b0;
    memset(&userList[id].schedule, 0xFF, sizeof(userList[id].schedule));
    uint8_t i = 0;
    while (newEnrolled[i++] != 0xFF);
    newEnrolled[i-1] = id;
}

void deleteUser (uint8_t id) {
    if (userList[id].enroll == 0b1) {
        memset(&userList[id], 0, sizeof(userList[id]));
    }
    uint8_t i = 0;
    while (newDeleted[i++] != 0xFF);
    newDeleted[i-1] = id;
}

uint8_t findEmptySpot (void) {
    uint8_t id;
    for (id = 0; id < DATABASE_SIZE; id++) {
        if (userList[id].enroll == 0) return id;
    }
    return DATABASE_FULL;
}

void updateSch(uint8_t index, uint8_t sch, uint8_t id){
    userList[id].schedule[index] = sch;
}
