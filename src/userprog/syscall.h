#include "threads/thread.h"

#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void exit(int status);
int wait(tid_t tid);

#endif /* userprog/syscall.h */
