#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#define FIFO "fifo.1"
#define MAXBUFF (80)

int main() {
  int fd, n;
  char buff [MAXBUFF];
  int err = mknod("fifo.1", S_IFIFO | 0666, 0);
  if (err < 0) {
    printf("Error1 %d\n", errno);
    return 1;
  }
  if ((fd = open(FIFO, O_RDONLY)) < 0) {
    printf("Error2\n");
    return 1;
  }
  while ((n=read(fd, buff, MAXBUFF)) > 0) {
    if (write(1, buff, n) != n) {
      printf("Error3\n");
      return 1;
    }
  }
  close(fd);
  if (unlink(FIFO) < 0) {
    printf("Error4\n");
    return 1;
  }
  return 0;
}