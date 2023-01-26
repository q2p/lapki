#include "shared.h"

int main() {
  const key_t msgkey = ftok("ser", 'A');
  if (msgkey < 0) {
    perror("ftok");
    return 1;
  }

  const int qid = open_queue(msgkey);
  if (qid < 0) {
    printf("open_queue");
    return 1;
  }
  
  Message msg = {
    .mtype = 1
  };

  do {
    printf("Введите запрос: ");

    fgets(msg.buff, MAXBUFF, stdin);
    char* n_ptr = strchr(msg.buff, '\n');
    char* r_ptr = strchr(msg.buff, '\r');
    if (n_ptr) *n_ptr = '\0';
    if (r_ptr) *r_ptr = '\0';

    printf("Запрос: \"%s\"\n", msg.buff);
    if (send_message(qid, &msg)) {
      printf("send_message");
      return 1;
    }
    if (read_message(qid, msg.mtype, &msg) < 0) {
      printf("read_message");
      return 1;
    }

    printf("Ответ: \"%s\"\n", msg.buff);
  } while (strcmp(msg.buff, "bye") != 0);

  if (remove_queue(qid)) {
    printf("remove_queue");
    return 1;
  }

  return 0;
}
