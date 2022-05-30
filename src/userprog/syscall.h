#include "threads/thread.h"

#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
tid_t exec (const char *cmd_line);
void exit (int status);

#endif /* userprog/syscall.h */
