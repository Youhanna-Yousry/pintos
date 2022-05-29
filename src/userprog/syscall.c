#include "../userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "process.h"
#include "threads/interrupt.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"

#define SYS_CALL false

static void syscall_handler(struct intr_frame *);

static struct lock files_sync_lock; /*lock for sync between files*/

int get_int(int **esp);                 /*get int from the stack*/
char *get_char_ptr(char ***esp);        /*get character pointer from stack*/
void *get_void_ptr(void ***esp);        /*get void void pointer from stack*/
void validate_void_ptr(const void *pt); /*check if the pointer is valid*/

/*wrappers*/
static bool create_wrapper (struct intr_frame *f);
static bool remove_wrapper (struct intr_frame *f);
static int open_wrapper (struct intr_frame *f);

/*system calls*/
static bool create (const char* file, unsigned initiall_size);
static bool remove (const char *file);
static int open (const char *file);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int get_int(int **esp)
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

void validate_void_ptr(const void *pt)
{
  if (PHYS_BASE < pt || pt < 0x08048000 || pagedir_get_page(thread_current()->pagedir, pt) == NULL)
  {
    exit(-1);
  }
}

static void
syscall_handler(struct intr_frame *f)
{
  // printf ("system call!\n");

  if (SYS_CALL)
    printf("<1>\n");

  switch (get_int((int **)(&(f->esp))))
  {
  case SYS_HALT:
  {

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
  case SYS_CREATE:
  {
    f->eax = create_wrapper (f);
    break;
  }
  case SYS_REMOVE:
  {
    f->eax = remove_wrapper (f);
    break;
  }
  case SYS_OPEN:
  {
    f->eax = open_wrapper (f);
    break;
  }
  case SYS_FILESIZE:
  {

    break;
  }
  case SYS_READ:
  {

    break;
  }
  case SYS_WRITE:
  {
    int fd = get_int((int **)(&f->esp));
    if (SYS_CALL)
      printf("<2>fd: %d\n", fd);

    void *buffer = get_void_ptr((void ***)&f->esp);
    if (SYS_CALL)
      printf("<3>\n");

    unsigned size = (unsigned)get_int((int **)(&f->esp));
    if (SYS_CALL)
      printf("<4> size: %d\n", size);

    if (fd == STDOUT_FILENO)
      putbuf(buffer, size);

    if (SYS_CALL)
      printf("<5>\n");
    break;
  }
  case SYS_SEEK:
  {

    break;
  }
  case SYS_TELL:
  {
    break;
  }
  case SYS_CLOSE:
  {
    break;
  }
  }
}

static bool
create_wrapper (struct intr_frame *f) {
  char *file = (char *) get_char_ptr((char ***)(&f->esp));
  unsigned initial_size = (unsigned) get_int((int **)(&f->esp));
  return create(file, initial_size);
}

/*
Creates a file, returns true in case of success and false otherwise.
*/
static bool
create (const char* file, unsigned initiall_size) {
  lock_acquire (&files_sync_lock);
  bool isCreatedSuccessfully = filesys_create ((char* ) file, (off_t) initiall_size);
  lock_release (&files_sync_lock);
  return isCreatedSuccessfully;
}

static bool
remove_wrapper (struct intr_frame *f) {
  char *file = (char *) get_char_ptr ((char ***)(&f->esp));
  return remove (file);
}

/*
Removes a file, returns true in case of success and false otherwise.
Should we remove open file form open_file list ???
You should implement the standard Unix semantics for files.
That is, when a file is removed any process which has
a file descriptor for that file may continue to use that descriptor.
This means that they can read and write from the file.
The file will not have a name, and no other processes will
be able to open it, but it will continue to exist until all
file descriptors referring to the file are closed or the machine shuts down. 
*/
static bool
remove (const char *file) {
  lock_acquire (&files_sync_lock);
  bool isRmovedSuccessfully = filesys_remove ((char* ) file);
  lock_release (&files_sync_lock);
  return isRmovedSuccessfully;
}

static int
open_wrapper (struct intr_frame *f) {
  char *file = (char *) get_char_ptr ((char ***)(&f->esp));
  return open(file);
}

/*
Opens a file
  -> In case of success, the open file (struct) is added to the list of opened
 files of the corresponding thread and its file descriptor is returned.
  -> In case of failure, the function returns -1
*/
static int 
open (const char *file) {
  lock_acquire (&files_sync_lock);
  struct file *openedFile = (struct file *) filesys_open ((char* ) file);
  if (openedFile == NULL) {
    lock_release (&files_sync_lock);
    return -1; 
  }
  int fd = thread_current ()->fd_last++;
  struct open_file *my_file = (struct open_file *) malloc (sizeof (struct open_file));
  my_file->fd = fd;
  my_file->fp = openedFile;
  list_push_back(&thread_current ()->open_files, &my_file->elem); 
  lock_release (&files_sync_lock);
  return fd;
}

int wait(tid_t tid)
{ // Exchange with process_wait() implementation?
  return process_wait(tid);
}

/*
Terminates the current user program, returning status to the kernel. If the process's
parent waits for it, this is the status that will be returned. Conventionally,
a status of 0 indicates success and nonzero values indicate errors.
*/
void exit(int status)
{
  struct thread *t = thread_current();
  struct thread *parent = t->parent_thread;

  printf("%s: exit(%d)\n", t->name, status);

  if (parent != NULL)
  {

    if (parent->child_waiting_on == t->tid)
    {
      if (DEBUG_WAIT)
        printf("<8>\n");
      parent->child_status = status;
      parent->child_waiting_on = -1;
      sema_up(&parent->sema_child_wait);
    }
    else
    {
      list_remove(&t->child_elem);
    }
  }
  thread_exit();
}
