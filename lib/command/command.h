#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "project_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "reSensor.h"
#include "reTgSend.h"

enum cmd_perm_level_type_t
{
    CMD_PERMISSION_DENIED,
    CMD_PERMISSION_USER,
    CMD_PERMISSION_ADMIN,
    CMD_PERMISSION_SU,
};

bool cmdTaskCreate(QueueHandle_t *cmdQueue);
u_int8_t permission_check(tgUpdateMessage_t *msg);
char *cmdProcessor(tgUpdateMessage_t *msg, u_int8_t perm_level);
void printFileInfo(tgUpdateMessage_t *msg);
char *statesGetDebugHeap();

#endif // __COMMAND_H__