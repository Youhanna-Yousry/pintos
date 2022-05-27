#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "../threads/synch.h"

void syscall_init (void);
static struct lock files_sync_lock;       /*lock for sync between files*/


#endif /* userprog/syscall.h */
