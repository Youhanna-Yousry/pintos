#include "../userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "process.h"
#include "threads/interrupt.h"



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

  return *((*esp)++);
}

char *
get_char_ptr(char ***esp) {

  return *((*esp)++);
}

void *
get_void_ptr(void ***esp) {
  return *((*esp)++);
}

static void
syscall_handler (struct intr_frame *f) 
{

  switch(get_int((int **)(&(f->esp))))
{
  case SYS_WRITE:
  {
    int fd = *((int*)f->esp + 1);
    void* buffer = (void*)(*((int*)f->esp + 2));
    unsigned size = *((unsigned*)f->esp + 3);
    //run the syscall, a function of your own making
    //since this syscall returns a value, the return value should be
    // stored in f->eax
    // f->eax = write(fd, buffer, size);
    
    if(fd == STDOUT_FILENO) putbuf(buffer, size);
    break;
  }
  case SYS_EXIT:
  {
    exit(get_int((int **)(&(f->esp))));
    break;
  }
  case SYS_EXEC:
    process_execute(get_char_ptr((char ***)(&(f->esp))));
    printf("yaaaaaw\n");
    break;
}
  printf ("system call!\n");
  
}


int wait(tid_t tid){  //Exchange with process_wait() implementation?
  return process_wait(tid);
}

/*
Terminates the current user program, returning status to the kernel. If the process's
parent waits for it, this is the status that will be returned. Conventionally,
a status of 0 indicates success and nonzero values indicate errors.
*/
void exit(int status){
  struct thread * t = thread_current();
  struct thread * parent = t->parent_thread;

  printf("%s: exit(%d)\n" , t -> name , status);

  if(parent != NULL){
    if(parent->child_waiting_on == t->tid){
      parent->child_status = status;
      sema_up(&parent->sema_child_wait);
    }
  }
  thread_exit();
}
