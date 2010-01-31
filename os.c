/*
 * os.c
 * Core operating system kernel code. Designed for the Rug Warrior
 * robot with the HC11 CPU.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */
 
#include "os.h"
#include "process.h"
#include "fifo.h"

/* 
  Process Table 
*/
typedef struct proc_str {
	PID pid;  	     /* Process ID. */ 
	unsigned int Name;   /* Name of process */ 
	unsigned int Level;  /* Scheduling level/queue */ 
	int  Arg;            /* Process argument */ 
	char *SP; 
	void(*PC)(void);
	proc_str* QueuePrev;
	proc_str* QueueNext;
} process;

process P[MAXPROCESS];        /* Main process table. */ 

process *PLast; /* Last Process to run */ 
process *PNext; /* Next Process to run */

process PSupervisor;

/*
  Periodic Process Queue
*/ 
int PPPLen;   /* Maximum of 16 periodic processes allowed to be in queue */
int PPP[];    /* The queue for periodic scheduling */
int PPPMax[]; /* Maximum CPU time in msec for each process */

/*
  Device Process Queue 
*/ 
process *DevP; 

/* 
  Sproatic Process Queue
*/ 
process *SpoP;

/* Stack Space */
char StackSpace[MAXPROCESS*WORKSPACE]; 
/* FIFOs */
fifo_t Fifos[MAXFIFO];

/* Gets incremented by an interrupt every x ms. */ 
unsigned long long Clock;
void ClockTick(void) __attribute__((interrupt)); /* http://www.gnu-m68hc11.org/wiki/index.php/Doc:compiler */ 


/* 
  Kernel entry point 
*/
void
main(int argc, char** argv)
{
	OS_Init();
	
	PPPLen = 1; 
	PPP    = {IDLE}; 
	PPPMax = {10  }; 

	/* Create processes here */
	
	OS_Start(); 
}
 
/* Initialize the OS */
void
OS_Init()
{	int i; 

	PLast     = INVALIDPID; 
	PNext     = INVALIDPID; 
	
	for (i = 0; i < MAXPROCESS; i++) {
		P[i]->pid = INVALIDPID; 
		P[i]->QueuePrev = 0; 
		P[i]->QueueNext = 0; 
	}	
	for (i = 0; i < MAXFIFO; i++) {
		Fifos[i]->fid = INVALIDFIFO; 
	}	
	
	Clock = 0;

	DevP = 0;
	SpoP = 0;
}
 
/* Actually start the OS */
void
OS_Start()
{

	while(1) {
		/* Check queues and find the next process to run. */
		
		
		

		/* Schedule processes:
				1. Check for devide process ready to run. 
				OR Check for a PERIODIC process ready to run. 
				OR Check for a SPORATIC process ready to run. 
			
				2. Schedule it. 
		
				3. 
		*/
	} 
}
 
void
OS_Abort()
{
	/* Kill those lesser peons and then shoot ourselves in the foot */
}
 
/*
* Create a process.
*
* Parameters:
* - f: Pointer to the function to execute.
* - arg: Initial argument for f, optionally used, retrieved by calling OS_GetParam().
* - level: The scheduling level for this process.
* - n: The "name" of this process. For device processes, n represents the 
*
* Returns:
* The PID of the new process.
*/
PID
OS_Create(void (*f)(void), int arg, unsigned int level, unsigned int n)
{	process *p; 
	int i; 
	
	/* Find an available process control block */ 
	for (i = 0; i < MAXPROCESS; i++) { 
		p = &P[i];
		if (p->pid == INVALIDPID) {
			p->pid = i+1; 		
			break; 
		}
	} 

	p->Name       = n; 
	p->Level      = level;
	p->Arg        = arg;
	
	/* Set the stack pointer to the last address in the workspace. */ 
	p->SP = (char*)((&StackSpace)+(WORKSPACE*(p->pid))-1); 
	/* Set the initial program counter to the address of the function representing the process. */ 
	p->PC = &f;

	/* Add Sporatic Processes to the Sporatic Queue */ 
	if      (p->level == SPORATIC) { SpoP = QueueAdd(p, SpoP); }
	/* Add Device Processes to the Device Queue */ 
	else if (p->level == DEVICE)   { DevP = QueueAdd(p, DevP); }
	
	return p->pid; 
}
 
 

/*
* End a process.
*/
void 
OS_Terminate() {
	PLast->pid = INVALIDPID;

	if      (PLast->level == SPORATIC) { SpoP = QueueRemove(PLast, SpoP); } 
	else if (PLast->level == DEVICE)   { DevP = QueueRemove(PLast, DevP); }
	
	/* Release Semiphores/FIFOS */ 
	
	OS_Yield();
} 

void 
OS_Yield() {
	/* Restore control to the kernel */ 
}

int
OS_GetParam() {
	return PLast->Arg; 
}

void 
Idle () {
	while (1); 
}

process *
QueueAdd(process *p, process *Queue) {
	/* The graph has one or more nodes. */ 
	if (Queue) {
		/* The graph has more than one existing node. */ 
		if (Queue->QueueNext && Queue->QueuePrev) {
			p->QueuePrev = Queue->QueuePrev; 
			p->QueueNext = Queue; 
			Queue->QueuePrev->QueueNext = p; 
			Queue->QueuePrev = p; 
		} 
		/* The graph has exactly one existing node. */ 
		else {
			Queue->QueueNext = p 
			Queue->QueuePrev = p
		}
	} 
	/* The graph has no existing nodes. */ 
	else { 
		Queue = &p; 
	} 
	return Queue; 
}

process *
QueueRemove(process *p, process *Queue) {
	/* The graph has one or more nodes. */ 
	if (Queue) { 
		/* Queue points to the node to be removed. */ 
		if (Queue == p) { 
			/* There is more than one node */ 
			if (Queue->QueueNext) { Queue = Queue->QueueNext; }
			/* There is only one node and it is the one to be removed. */ 
			else                  { return 0; }
		}
		/* The graph has only two nodes  */ 
		if (p->QueuePrev == p->QueueNext) {
			Queue->QueuePrev = 0; 
			Queue->QueueNext = 0;
		}
		/* The graph has more than two nodes. */ 
		else {
			p->QueuePrev->QueueNext = p->QueueNext; 
			p->QueueNext->QueuePrev = p->QueuePrev; 
		}
	}
	return Queue; 
}

/* Increments clock. Run by an interrupt every x ms. */ 
void 
ClockTick(void) {
	Clock++; 
}
