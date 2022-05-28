#include "../userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "process.h"
#include "threads/interrupt.h"

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
get_int(int **esp) 
{
  validate_void_ptr(*esp);
  return *((*esp)++);
}

char *
get_char_ptr(char ***esp) 
{
  validate_void_ptr(*esp);
  
  return *((*esp)++);
}

void *
get_void_ptr(void ***esp) 
{
  validate_void_ptr(*esp);

  return *((*esp)++);
}

void
validate_void_ptr (const void *pt)
{
  if (PHYS_BASE < pt || pt < 0x08048000 || pagedir_get_page(thread_current()->pagedir, pt) == NULL)
  {
    exit(-1);
  }
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
    exit(get_int((int **)(&(f->esp))));
    break;
  }
  case SYS_EXEC:
  {
    f->eax = process_execute(get_char_ptr((char ***)(&(f->esp))));
    break;
  }
  case SYS_WAIT:
  {
    f->eax = wait(get_int((int **)(&f->esp)));
    break;
  }
}  
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
    if(DEBUG_WAIT) printf("<8>\n");
      parent->child_status = status;
      parent->child_waiting_on = -1;
      sema_up(&parent->sema_child_wait);
    }else{
      list_remove(&t->child_elem);
    }
  }
  thread_exit();
}
