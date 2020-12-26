#define _GNU_SOURCE

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define SPLICE_SIZE 1024*1024

atomic_ullong counter;
atomic_bool use_pipe;

void *splice_thread(void *unused __attribute__((unused))) {
  if (use_pipe) {
    int pipefd[2];
    pipe(pipefd);
    fcntl(pipefd[0], F_SETPIPE_SZ, SPLICE_SIZE);
    for (;;) {
      splice(STDIN_FILENO, NULL, pipefd[1], NULL, SPLICE_SIZE, 0);
      counter += splice(pipefd[0], NULL, STDOUT_FILENO, NULL, SPLICE_SIZE, 0);
    }
  } else {
    for (;;) {
      counter += splice(STDIN_FILENO, NULL, STDOUT_FILENO, NULL, SPLICE_SIZE, 0);
    }
  }
}

int main() {
  pthread_t thread[8];
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, SPLICE_SIZE);
  ssize_t res = splice(STDIN_FILENO, NULL, STDOUT_FILENO, NULL, SPLICE_SIZE, 0);
  use_pipe = (res == -1 && errno == EINVAL);
  if (res > 0)
    counter += res;
  for (size_t i = 0; i < 1; ++i)
    pthread_create(&thread[i], NULL, splice_thread, NULL);

  for (;;) {
    sleep(1);
    fprintf(stderr, "%llu M/s\n", atomic_exchange(&counter, 0) / 1024 / 1024);
  }
}
