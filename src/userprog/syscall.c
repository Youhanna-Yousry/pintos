#include "../userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "process.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "../filesys/file.h"
#include "../filesys/filesys.h"
#include "../devices/shutdown.h"
#include "../devices/input.h"
#include "pagedir.h"

#define SYS_CALL false

static void syscall_handler(struct intr_frame *);

static struct lock files_sync_lock; /*lock for sync between files*/

/*helping functions*/
static int get_int(int **esp);                 /*get int from the stack*/
static char *get_char_ptr(char ***esp);        /*get character pointer from stack*/
static void *get_void_ptr(void ***esp);        /*get void void pointer from stack*/
static void validate_void_ptr(const void *pt); /*check if the pointer is valid*/
static struct open_file *get_file(int fd);     /*returns the open_file struct of the given file descriptor*/

/*wrappers*/
static void halt_wrapper(void);
static void exit_wrapper(struct intr_frame *f);
static tid_t exec_wrapper(struct intr_frame *f);
static int wait_wrapper(struct intr_frame *f);
static bool create_wrapper(struct intr_frame *f);
static bool remove_wrapper(struct intr_frame *f);
static int open_wrapper(struct intr_frame *f);
static int file_size_wrapper(struct intr_frame *f);
static int read_wrapper(struct intr_frame *f);
static int write_wrapper(struct intr_frame *f);
static void seek_wrapper(struct intr_frame *f);
static unsigned tell_wrapper(struct intr_frame *f);
static void close_wrapper(struct intr_frame *f);

/*system calls*/
static void halt(void);
static tid_t exec (const char *cmd_line);
static int wait(tid_t tid);
static bool create(const char *file, unsigned initiall_size);
static bool remove(const char *file);
static int open(const char *file);
static int file_size(int fd);
static int read(int fd, void *buffer, unsigned size);
static int write(int fd, const void *buffer, unsigned size);
static void seek(int fd, unsigned position);
static unsigned tell(int fd);
static void close(int fd);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&files_sync_lock);
}

static int
get_int(int **esp)
{
  validate_void_ptr(*esp);
  return *((*esp)++);
}

static char *
get_char_ptr(char ***esp)
{
  validate_void_ptr(*esp);

  return *((*esp)++);
}

static void *
get_void_ptr(void ***esp)
{
  validate_void_ptr(*esp);

  return *((*esp)++);
}

static void
validate_void_ptr(const void *pt)
{
  if (!is_user_vaddr(pt) || pagedir_get_page(thread_current()->pagedir, pt) == NULL)
  {
    exit(-1);
  }
}

static struct open_file *
get_file(int fd)
{
  struct thread *t = thread_current();
  struct open_file *fp = NULL;
  for (struct list_elem *elem = list_begin(&t->open_files);
       elem != list_end(&t->open_files); elem = list_next(elem))
  {
    fp = list_entry(elem, struct open_file, elem);
    if (fp->fd == fd)
    {
      break;
    }
  }
  if (fp == NULL)
    exit(-1);
  return fp;
}

static void
syscall_handler(struct intr_frame *f)
{
  switch (get_int((int **)(&(f->esp))))
  {
  case SYS_HALT:
  {
    halt_wrapper();
    break;
  }
  case SYS_EXIT:
  {
    exit_wrapper(f);
    break;
  }
  case SYS_EXEC:
  {
    f->eax = exec_wrapper(f);
    break;
  }

  case SYS_WAIT:
  {
    f->eax = wait_wrapper(f);
    break;
  }
  case SYS_CREATE:
  {
    f->eax = create_wrapper(f);
    break;
  }
  case SYS_REMOVE:
  {
    f->eax = remove_wrapper(f);
    break;
  }
  case SYS_OPEN:
  {
    f->eax = open_wrapper(f);
    break;
  }
  case SYS_FILESIZE:
  {
    f->eax = file_size_wrapper(f);
    break;
  }
  case SYS_READ:
  {
    f->eax = read_wrapper(f);
    break;
  }
  case SYS_WRITE:
  {
    f->eax = write_wrapper(f);
    break;
  }
  case SYS_SEEK:
  {
    seek_wrapper(f);
    break;
  }
  case SYS_TELL:
  {
    f->eax = tell_wrapper(f);
    break;
  }
  case SYS_CLOSE:
  {
    close_wrapper(f);
    break;
  }
  }
}

static void
halt_wrapper(void)
{
  halt();
}

/*
  Terminates Pintos by calling shutdown_power_off()
*/
static void
halt(void)
{
  shutdown_power_off();
}

static void
exit_wrapper(struct intr_frame *f)
{
  int status = get_int((int **)(&f->esp));
  exit(status);
}

/*
Terminates the current user program, returning status to the kernel. If the process's
parent waits for it, this is the status that will be returned. Conventionally,
a status of 0 indicates success and nonzero values indicate errors.
*/
void exit(int status)
{
  struct thread *t = thread_current();
  t->exit_code = status;
  thread_exit();
}

static tid_t
exec_wrapper(struct intr_frame *f)
{
  const char *cmd_line = get_char_ptr((char ***)(&(f->esp)));
  return exec(cmd_line);
}

/*
  Runs the executable whose name is given in cmd line, passing any given arguments,
  and returns the new process's program id (pid). In case of failure returns -1.
*/
tid_t exec(const char *cmd_line)
{
  return process_execute(cmd_line);
}

static int
wait_wrapper(struct intr_frame *f)
{
  int tid = get_int((int **)(&f->esp));
  return wait(tid);
}

/*
  Waits for a child process pid and retrieves the child's exit status.
  Calls process_wait() function, It returns -1 if the child was killed
  by the kernal, or child doesn't belong to the parent.
*/
static int
wait(tid_t tid)
{
  return process_wait(tid);
}

static bool
create_wrapper(struct intr_frame *f)
{
  char *file = (char *)get_char_ptr((char ***)(&f->esp));
  unsigned initial_size = (unsigned)get_int((int **)(&f->esp));
  return create(file, initial_size);
}

/*
Creates a file, returns true in case of success and false otherwise.
*/
static bool
create(const char *file, unsigned initiall_size)
{
  if (strcmp(file, "") == 0)
    exit(-1);
  lock_acquire(&files_sync_lock);
  bool isCreatedSuccessfully = filesys_create((char *)file, (off_t)initiall_size);
  lock_release(&files_sync_lock);
  return isCreatedSuccessfully;
}

static bool
remove_wrapper(struct intr_frame *f)
{
  char *file = (char *)get_char_ptr((char ***)(&f->esp));
  return remove(file);
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
remove(const char *file)
{
  lock_acquire(&files_sync_lock);
  bool isRmovedSuccessfully = filesys_remove((char *)file);
  lock_release(&files_sync_lock);
  return isRmovedSuccessfully;
}

static int
open_wrapper(struct intr_frame *f)
{
  char *file = (char *)get_char_ptr((char ***)(&f->esp));
  return open(file);
}

/*
Opens a file
  -> In case of success, the open file (struct) is added to the list of opened
 files of the corresponding thread and its file descriptor is returned.
  -> In case of failure, the function returns -1
*/
static int
open(const char *file)
{
  if (file == NULL)
    exit(-1);
  lock_acquire(&files_sync_lock);
  struct file *openedFile = (struct file *)filesys_open((char *)file);
  if (openedFile == NULL)
  {
    lock_release(&files_sync_lock);
    return -1;
  }
  int fd = thread_current()->fd_last++;
  struct open_file *my_file = (struct open_file *)malloc(sizeof(struct open_file));
  my_file->fd = fd;
  my_file->fp = openedFile;
  list_push_back(&thread_current()->open_files, &my_file->elem);
  lock_release(&files_sync_lock);
  return fd;
}

static int
file_size_wrapper(struct intr_frame *f)
{
  return file_size(get_int((int **)(&(f->esp))));
}

/*Returns the size, in bytes, of the file open as fd.*/
static int
file_size(int fd)
{
  struct open_file *fp = get_file(fd);
  lock_acquire(&files_sync_lock);
  int size = file_length(fp->fp);
  lock_release(&files_sync_lock);

  return size;
}

static int
read_wrapper(struct intr_frame *f)
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

  validate_void_ptr(buffer + size);
  return read(fd, buffer, size);
}

/*Reads size bytes from the file open as fd into buffer. Returns the number of bytes
actually read (0 at end of file), or -1 if the file could not be read (due to a condition
other than end of file). Fd 0 reads from the keyboard.*/

static int
read(int fd, void *buffer, unsigned size)
{
  int rv = -1;
  if (fd == STDOUT_FILENO)
  {
    // fd == 1: negative area -> not possible
  }
  else if (fd == STDIN_FILENO)
  {
    unsigned i = 0;
    while (i < size)
    {
      lock_acquire(&files_sync_lock);
      *((int *)buffer) = input_getc();
      lock_release(&files_sync_lock);
      buffer++;
      i++;
    }
    rv = i;
  }
  else
  {
    struct open_file *fp = get_file(fd);
    lock_acquire(&files_sync_lock);
    rv = file_read(fp->fp, buffer, size);
    lock_release(&files_sync_lock);
  }
  return rv;
}

static int
write_wrapper(struct intr_frame *f)
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


  validate_void_ptr(buffer + size);

  return write(fd, buffer, size);
}


/*
Writes size bytes from buffer to the open file fd. Returns the number of bytes actually
written, which may be less than size if some bytes could not be written.
Writing past end-of-file would normally extend the file, but file growth is not imple-
mented by the basic file system. The expected behavior is to write as many bytes as
possible up to end-of-file and return the actual number written, or 0 if no bytes could
be written at all.
Fd 1 writes to the console. Your code to write to the console should write all of buffer
in one call to putbuf(), at least as long as size is not bigger than a few hundred
bytes. (It is reasonable to break up larger buffers.) Otherwise, lines of text output
by different processes may end up in
*/
static int
write(int fd, const void *buffer, unsigned size)
{
  int rv = 0;
  if (fd == STDOUT_FILENO)
  {
    lock_acquire(&files_sync_lock);
    putbuf(buffer, size);
    lock_release(&files_sync_lock);
    rv = size;
  }
  else if (fd == STDIN_FILENO)
  {
    // fd == 0: negative area -> not possible
  }
  else
  {
    struct open_file *fp = get_file(fd);
    lock_acquire(&files_sync_lock);
    rv = file_write(fp->fp, buffer, size);
    lock_release(&files_sync_lock);
  }
  return rv;
}

static void
seek_wrapper(struct intr_frame *f)
{
  int fd = get_int((int **)(&f->esp));
  unsigned position = (unsigned)get_int((int **)(&f->esp));
  seek(fd, position);
}

/*
  Changes the next byte to be read or written in open file fd to position, expressed in
  bytes from the beginning of the file. (Thus, a position of 0 is the file's start.)
*/
static void
seek(int fd, unsigned position)
{
  struct open_file *fp = get_file(fd);
  off_t new_pos = (off_t)position;
  lock_acquire(&files_sync_lock);
  file_seek(fp->fp, new_pos);
  lock_release(&files_sync_lock);
}

static unsigned
tell_wrapper(struct intr_frame *f)
{
  int fd = get_int((int **)(&f->esp));
  return tell(fd);
}

/*
  Returns the position of the next byte to be read or written in open file fd, expressed
  in bytes from the beginning of the file.
*/
static unsigned
tell(int fd)
{
  struct open_file *fp = get_file(fd);
  lock_acquire(&files_sync_lock);
  unsigned rv = file_tell(fp->fp);
  lock_release(&files_sync_lock);
  return rv;
}

static void
close_wrapper(struct intr_frame *f)
{
  int fd = get_int((int **)(&f->esp));
  close(fd);
}

/*
  Closes file descriptor fd. Exiting or terminating a process implicitly closes 
  all its open file descriptors, as if by calling this function for each one.
*/
static void
close(int fd)
{
  struct open_file *fp = get_file(fd);
  list_remove(&fp->elem);
  lock_acquire(&files_sync_lock);
  file_close(fp->fp);
  lock_release(&files_sync_lock);
  free(fp);
}