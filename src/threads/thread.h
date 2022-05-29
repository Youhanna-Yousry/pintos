#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#define DEBUG_LOAD_AVG false
#define DEBUG_RECENT false
#define DEBUG_PRIORITY false
#define DEBUG_YIELD false

#define DEBUG_STACK false
#define DEBBUG_LOAD false
#define DEBUG_WAIT false
#define DEBUG_MULT false



#include <debug.h>
#include <list.h>
#include <stdint.h>
#include <string.h>
#include "fixed-point.h"
#include "synch.h"
// #include "../filesys/file.c"


/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    /* Advanced Scheduler */
    int nice;                           /* How likely the thread will give up the cpu.*/
    real recent_cpu;                   /*estimate of recent cpu usage by the thread.*/

   
   /* Priority donation implementation */
    int original_priority;
    struct lock *wait;                 /* Lock which thread waits for */
    struct list locks;                 /* List of owned locks */
#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif
   /*User program fields*/
   struct list open_files;                /*keeps track of opened file by the process*/
   struct list child_processes;           /*keeps track of all child processes*/
   struct thread *parent_thread;          /*points to the parent of the thread*/
   bool child_creataion_success;          /*status of child creation*/
   int child_status;                      /*status of child process that we wait on*/
   struct semaphore sema_child_wait;           /*used to wait for child process*/
   struct semaphore parent_child_sync;    /*synchonizes between parent and child*/
   struct file *executable_file;          /*executable of the process*/
   int fd_last;
   struct list_elem child_elem;           /*child element in parent children list*/
   tid_t child_waiting_on;                /* thread id of the child that thread waits on */

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

   /*Used in userprog*/
  struct child_process {
     tid_t pid;
     struct thread *t;
     struct list_elem elem;
  };
   /*opened files by the process*/
  struct open_file {
     int fd;
     struct file *fp;
     struct list_elem elem;
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;


void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);
void thread_calculate_priority(struct thread *t);
bool thread_find_greater_priority (struct thread *t);

void thread_update_priority_mlfqs (struct thread *, void *);

int thread_get_nice (void);
void thread_set_nice (int);

int thread_get_recent_cpu (void);
void thread_calculate_recent_cpu (struct thread *t);

int thread_get_load_avg (void);
void thread_calculate_load_avg (void);


bool thread_is_idle(struct thread *t);

bool thread_find_greater_priority (struct thread *t);

void thread_check_priority(void);
void thread_update_priority(struct thread * t);

void print_threads(struct list* l);

#endif /* threads/thread.h */
