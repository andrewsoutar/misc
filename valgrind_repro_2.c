#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/syscall.h>
#include <linux/io_uring.h>

int main() {
  sigset_t oldmask;
  struct io_uring_params p = { 0 };
  int fd = syscall(SYS_io_uring_setup, 1, &p);
  if (fd < 0)
    err(EXIT_FAILURE, "io_uring_setup");
  if (sigprocmask(SIG_BLOCK, NULL, &oldmask))
    err(EXIT_FAILURE, "sigprocmask");
  syscall(SYS_io_uring_enter, fd, 0, 1, IORING_ENTER_GETEVENTS, &oldmask, _NSIG / 8);
  for (;;);
}
