#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define DATABASE_SIZE 10
#define DATABASE_FULL 0xFF

typedef struct {
    uint8_t admin:1;
    uint8_t enroll:1;
    uint8_t schedule[48];
} User;

extern User userList[];
extern uint8_t newEnrolled[];
extern uint8_t newDeleted[];

void deleteDatabase (void);
void clearList (void);
void saveNewUser (uint8_t isAdmin, uint8_t id);
void deleteUser (uint8_t id);
uint8_t findEmptySpot (void);
void updateSch(uint8_t index, uint8_t sch, uint8_t id);
