#include "command.h"

static const char* logTAG = "CMD";
static const char* cmdTaskName = "cmd_processor";
TaskHandle_t _cmdTask;
QueueHandle_t *_cmdQueueP = nullptr;

void cmdTaskExec(void *pvParameters)
{
  tgUpdateMessage_t *cmdMsg;

  while (true) {
    if (xQueueReceive(*_cmdQueueP, &cmdMsg, portMAX_DELAY) == pdPASS) {

      tgUpdateMessage_t cmdMsgReceived;
      memset(&cmdMsgReceived, 0, sizeof(tgUpdateMessage_t));

      cmdMsgReceived.text = nullptr;
      cmdMsgReceived.file = nullptr;
      cmdMsgReceived.chat_id = cmdMsg->chat_id;
      cmdMsgReceived.from_id = cmdMsg->from_id;
      cmdMsgReceived.date = cmdMsg->date;
      cmdMsgReceived.update_id = cmdMsg->update_id;
      cmdMsgReceived.type = cmdMsg->type;
      u_int8_t perm_level = permission_check(&cmdMsgReceived);
      char *answer = nullptr;

      if (perm_level > CMD_PERMISSION_DENIED) {
        switch(cmdMsgReceived.type) {
            case TG_MESSAGE_TEXT:
                cmdMsgReceived.text = cmdMsg->text;
                rlog_d(logTAG, "New command received: %s", cmdMsg->text);

                answer = cmdProcessor(&cmdMsgReceived, perm_level);
                break;
            case TG_MESSAGE_DOCUMENT:
                cmdMsgReceived.file = cmdMsg->file;
                printFileInfo(&cmdMsgReceived);
                break;
            case TG_MESSAGE_UNKNOWN:
                //rlog_d(logTAG, "HTTP_EVENT_HEADER_SENT");
                break;
        };

      }
      if (answer) {
          rlog_d(logTAG, "answer is: %s", answer);
          tgSend(MK_MAIN, MP_CRITICAL, "Результат команды ", CONFIG_TELEGRAM_DEVICE,"%s", answer);
          //free(answer);
      } else {
          rlog_d(logTAG, "answer is empty:");
      }

      //if (cmdMsg->text) free(cmdMsg->text);
      if (cmdMsg->file) free(cmdMsg->file);
      free(cmdMsg);
      cmdMsg = nullptr;
      //if (answer) free(answer);

    };
  };
}

bool cmdTaskCreate(QueueHandle_t *cmdQueue)
{
  _cmdQueueP = cmdQueue;

  if (!_cmdTask) {
    if (!*_cmdQueueP) {
      rloga_e("The _cmdQueueP is not available!");
      //eventLoopPostError(RE_SYS_TELEGRAM_ERROR, ESP_FAIL);
      return false;
    };
    
    xTaskCreatePinnedToCore(cmdTaskExec, cmdTaskName, 4096, nullptr, CONFIG_TASK_PRIORITY_TELEGRAM, &_cmdTask, CONFIG_TASK_CORE_TELEGRAM);
    if (!_cmdTask) {
      rloga_e("Failed to create command processor task!");
      //eventLoopPostError(RE_SYS_TELEGRAM_ERROR, ESP_FAIL);
      return false;
    }
    else {
      rloga_i("Task [ %s ] has been successfully started", cmdTaskName);
      //eventLoopPostError(RE_SYS_TELEGRAM_ERROR, ESP_OK);
      return true;
    };
  }
  else {
    return true;
  };
}

u_int8_t permission_check(tgUpdateMessage_t *msg) {
  rlog_d(logTAG, "PERM Received chat_id: %" PRId64 ", from_id: %" PRId64 ", date: %d",
          msg->chat_id, msg->from_id, msg->date);
  return CMD_PERMISSION_SU;
}

char * cmdProcessor(tgUpdateMessage_t *msg, u_int8_t perm_level) {
  const char fmt[] = "Пришла команда:%s";
  int sz = snprintf(NULL, 0, fmt, (const char*) msg->text);
  char *buf = (char*)esp_calloc(1, sz + 1);
  snprintf(buf, sz + 1, fmt, (const char*) msg->text);
  rlog_d(logTAG, "cmdProcessor return: %s", buf);
  return buf;
}

void printFileInfo(tgUpdateMessage_t *msg) {
  if (msg->file) {
    rlog_d(logTAG, "printFileInfo name: %s,caption: %s, size: %d, id: %s",
      msg->file->name, (msg->file->caption) ? msg->file->caption : "NO CAP", msg->file->size, msg->file->id);
  } else {
    rlog_d(logTAG, "no msg->file");
  }

}
