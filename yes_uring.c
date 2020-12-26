#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

#include <inttypes.h>
#include <unistd.h>
#include <syscall.h>
#include <sys/mman.h>

#include <liburing.h>

int main() {
#define BUF_SIZE 32768
  char *buffer = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  for (size_t i = 0; i < BUF_SIZE; i += 2)
    buffer[i] = 'y', buffer[i+1] = '\n';

  struct io_uring ring;
  io_uring_queue_init(64, &ring, 0);
  const struct iovec iovec = {buffer, BUF_SIZE};
  io_uring_register_buffers(&ring, &iovec, 1);
  io_uring_register_files(&ring, (int[]) {STDOUT_FILENO}, 1);

  uint64_t entries = *ring.sq.kring_entries;
  uint64_t mask = *ring.sq.kring_mask;
  for (size_t i = 0; i < entries; ++i) {
    ring.sq.sqes[i].opcode = IORING_OP_WRITE_FIXED;
    ring.sq.sqes[i].flags = IOSQE_FIXED_FILE | IOSQE_IO_DRAIN;
    ring.sq.sqes[i].ioprio = 0;
    ring.sq.sqes[i].fd = 0;
    ring.sq.sqes[i].addr = (uintptr_t) buffer;
    ring.sq.sqes[i].len = BUF_SIZE;
  }
  for (size_t i = 0; i < entries; ++i)
    ring.sq.array[i] = i;
  for (size_t i = 0;;) {
    uint64_t max = io_uring_smp_load_acquire(ring.sq.khead) + entries;
    uint64_t dif = max - i;
    for (; i < max; ++i)
      ring.sq.sqes[i & mask].off = BUF_SIZE * i;
    io_uring_smp_store_release(ring.sq.ktail, i);
    syscall(SYS_io_uring_enter, ring.ring_fd, dif, 0, 0, NULL, _NSIG / 8);
    io_uring_smp_store_release(ring.cq.khead, io_uring_smp_load_acquire(ring.cq.ktail));
  }
}
