#include "shared.h"

int main() {
  const key_t msgkey = ftok("ser", 'A');
  if (msgkey < 0) {
    perror("ftok");
    return 1;
  }

  const int qid = msgget(msgkey, PERM | IPC_CREAT);
  if (qid < 0) {
    perror("msgget");
    return 1;
  }

  Message msg = {
    .mtype = 1
  };

  while(1) {
    if (read_message(qid, msg.mtype, &msg) < 0) {
      perror("read_message");
      return 1;
    }

    printf("Запрос: \"%s\"\n", msg.buff);

    ssize_t j = 0;
    for (ssize_t i = 0; i != MAXBUFF - 1; i++) {
      char c = msg.buff[i];
      if (c == '\0') {
        break;
      }
      if (c >= '0' && c <= '9'){
        msg.buff[j++] = msg.buff[i];
      }
    }

    char is_bye = !strcmp(msg.buff, "bye");
    if (!is_bye) {
      if (j == 0) {
        strcpy(msg.buff, "Нет");
      } else {
        msg.buff[j] = '\0';
      }
    }

    printf("Ответ: \"%s\"\n", msg.buff);
    if ((send_message(qid, &msg)) < 0) {
      perror("send_message");
      return 1;
    }

    if (is_bye) {
      return 0;
    }
  }
}
