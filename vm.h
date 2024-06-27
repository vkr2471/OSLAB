#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <math.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <math.h>
// if we use valid or invalid bit in page table entry,what is the need for the no of pages in the page table entry it table length
//how do you proportnatly allocate the number of frames to the processes
//can different process share the same frame??
// assigment told to do local place replcament stratgey so the answer to the above quiestion is no.


#define TOPROCESS 20
#define FROMPROCESS 10
#define MAX_BUFFER_SIZE 100
#define MAX_PAGES 1000
#define TOSCH 10
#define FROMSCH 20  
#define TOMMU 10
#define FROMMMU 20 
#define INT_MAX 1e9
#define PSEND_TYPE 10
#define MMUTOPRO 20
#define INVALID_PAGE_REF -2
#define PAGEFAULT -1
#define PROCESS_OVER -9
#define PAGEFAULT_HANDLED 5
#define TERMINATED 10



typedef struct {
	int frameno;
	int isvalid;
	int count;
}ptbentry;

typedef struct {
	pid_t pid;
	int m;
	int f_cnt;
	int f_allo;
}pcb;

typedef struct 
{
	int current;
	int flist[];
}freelist;
struct msgbuf
{
	long mtype;
	int id;
	int pageno;
};

struct mmutopbuf
{
	long mtype;
	int frameno;
};

struct mmutosch
{
	long mtype;
	char mbuf[1];
};

typedef struct mmumsgbuf_send {
	long    mtype;          /* Message type */
	int id;
	int pageno;
} mmumsgbuf_send;

typedef struct mmumsgbuf_recv {
	long    mtype;          /* Message type */
	int frameno;
} mmumsgbuf_recv;

typedef struct mymsgbuf {
	long    mtype;          
	int id;
} mymsgbuf;

typedef struct _mmutosch {
	long    mtype;
	char mbuf[1];
} mmutosch;

