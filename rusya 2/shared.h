#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum {
  MAXBUFF = 80,
  PERM = 0666,
};

typedef struct {
  long mtype;
  char buff[MAXBUFF];
} Message;

/// создает очередь сообщений.
int open_queue( key_t keyval ) {
  return msgget ( keyval, IPC_CREAT | 0660 );
}

/// помещает строку сообщения и передает это сообщение.
int send_message( int qid, Message *qbuf ) {
  return msgsnd( qid, qbuf, MAXBUFF, 0);
}

// читает строку сообщения.
int read_message(int qid, long type, Message* qbuf) {
  return msgrcv( qid, qbuf, MAXBUFF, type, 0 );
}

/// удаляет очередь сообщений.
int remove_queue( int qid ) {
  return msgctl( qid, IPC_RMID, 0);
}
