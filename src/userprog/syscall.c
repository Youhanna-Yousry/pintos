#include "../userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

#define SYS_CALL false



static void syscall_handler (struct intr_frame *);

static struct lock files_sync_lock;       /*lock for sync between files*/

int get_int(int **esp);                   /*get int from the stack*/
char *get_char_ptr (char ***esp);         /*get character pointer from stack*/
void *get_void_ptr (void ***esp);         /*get void void pointer from stack*/
void validate_void_ptr (const void *pt);  /*check if the pointer is valid*/

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int
get_int(int **esp) {
  // printf("before esp : %p\n", *esp);
  // *esp += 1; 
  // printf("after esp : %p\n", *esp);
  // int value = **esp;

  return *((*esp)++);
}

char *
get_char_ptr(char ***esp) {
  // char *ptr = (char *)*esp;
  // *esp += 1; 
  return *((*esp)++);
}

void *
get_void_ptr(void ***esp) {
  return *((*esp)++);
}

static void
syscall_handler (struct intr_frame *f) 
{ 
  // printf ("system call!\n");
  
  if(SYS_CALL) printf("<1>\n");

  switch(get_int((int **)(&(f->esp))))
{
  case SYS_WRITE:
  {
    int fd = get_int((int **)(&f->esp));
    if(SYS_CALL) printf("<2>fd: %d\n", fd);

    void *buffer = get_void_ptr((void ***)&f->esp);
    if(SYS_CALL) printf("<3>\n");

    unsigned size = (unsigned) get_int((int **) (&f->esp));
    if(SYS_CALL) printf("<4> size: %d\n", size);

    if(fd == STDOUT_FILENO) putbuf(buffer, size);

    if(SYS_CALL) printf("<5>\n");
    break;
  }
  case SYS_EXIT:
  {
    process_exit();
    break;
  }
  case SYS_EXEC:
  {
    process_execute(get_char_ptr((char ***)(&(f->esp))));
    break;
  }
  
  case SYS_WAIT:
  {
    process_wait(0);
    break;
  }
}  
}
