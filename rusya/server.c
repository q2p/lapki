#include <time.h>
#include "shared.h"

int main() {
  time_t t;
  srand((unsigned) time(&t));

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

    char is_bye = !strcmp(msg.buff, "bye");

    if (!is_bye) {
      ssize_t i1 = 0, i2 = 0;
      while(1) {
        char c = msg.buff[i2];
        if ((c == ' ' || c == '\0') && i1 != i2 && !(rand() & 0xFF)) {
          memmove(msg.buff+i1+3, msg.buff+i2, MAXBUFF-i2-1);
          memset(msg.buff+i1, 'M', 3);
          break;
        }
        if (c == '\0') {
          if (i2 == 0) {
            break;
          }
          i1 = 0;
          i2 = 0;
        } else if (c == ' ') {
          i1 = ++i2;
        } else {
          i2++;
        }
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
