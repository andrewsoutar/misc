#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/uio.h>

#define BUF_SIZE 1024*1024

int main() {
  char *buffer = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  for (size_t i = 0; i < BUF_SIZE; i += 2)
    buffer[i] = 'y', buffer[i+1] = '\n';
  struct iovec iov = {buffer, BUF_SIZE};

  int buffer_pipe[2];
  pipe(buffer_pipe);
  fcntl(buffer_pipe[0], F_SETPIPE_SZ, BUF_SIZE);
  vmsplice(buffer_pipe[1], &iov, 1, SPLICE_F_GIFT);

  if (tee(buffer_pipe[0], STDOUT_FILENO, BUF_SIZE, 0) == -1 && errno == EINVAL) {
    int intermediate_pipe[2];
    pipe(intermediate_pipe);
    for (;;) {
      tee(buffer_pipe[0], intermediate_pipe[1], BUF_SIZE, 0);
      splice(intermediate_pipe[0], NULL, STDOUT_FILENO, NULL, BUF_SIZE, SPLICE_F_MOVE);
    }
  }

  for (;;) {
    tee(buffer_pipe[0], STDOUT_FILENO, BUF_SIZE, 0);
  }
}
