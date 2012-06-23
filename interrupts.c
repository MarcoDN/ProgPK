/*
 * intHandler.c
 *
 *  Created on: Jun 12, 2012
 *      Author: michele
 */


#include "const.h"
#include "asl.e"
#include "pcb.e"

#include "libumps.h"
#include "types11.h"
#include "sysvars.h"

#include "scheduler.h"
#include "traps.h"
#include "syscalls.h"
#include "interrupts.h"

#include "utilTest.h"
#include "copy.h"
/*macro use on terminal interrupt*/
#define IntlineNo 7
#define devAddrBase(i) 0x10000050 + ((IntlineNo -3 )* 0x80)+(i *0x10)
#define statusReg_R(i) devAddrBase(i) + 0x0
#define statusReg_T(i) devAddrBase(i) + 0x8
#define commandReg_R(i) devAddrBase(i) + 0x4
#define commandReg_T(i) devAddrBase(i) + 0xC
#define lineTerminal PENDING_BITMAP_START + (WORD_SIZE * (IntlineNo - 3))
extern state_t new_old_areas[16][8];

/* Devices's semaphores. */
semd_t *terminalRead, *terminalWrite;

void intHandler() {

	int int_cause;
	int *bitMapDevice;
	int devNumb;

	int cpu = getPRID(),cause = getCAUSE();
	pcb_t* current = getRunningProcess(cpu);
	if (cpu > 0)
		copyState((&new_old_areas[cpu][1]),(&current->p_s));
	else
		copyState(((state_t*)INT_OLDAREA),(&current->p_s));

	enqueueProcess(current,cpu);

	/* Se è presente un processo sulla CPU, salviamo il suo stato */
	if(current != NULL)
		copyState(((state_t*)INT_OLDAREA),(&current->p_s));
	/* Linea 2 Interval Timer Interrupt + Gestione PSEUDO CLOCK ****************************/
	if (CAUSE_IP_GET(int_cause, INT_TIMER)) {

		/*puntatore al semaforo*/
		semd_t pseudoClockSem = getSemd(MAXPROC);
		while(!CAS(&sem_esclusion[MAXPROC],FREE,BUSY)) ;/*appena trova free lo metto a busy*/
		/*se la coda non è vuota li risvegliamo*/
		while(!(emptyProcQ(pseudoClockSem->s_procQ))) {
			pcb_t* pWake = removeBlocked(MAXPROC);
			enqueueProcess(pWake,cpu);/*metto in readyq il processo*/
			pseudoClockSem->s_value = pseudoClockSem->s_value + 1;/*incremento il valore del semaforo*/
		}
		pseudoClockSem->s_value = pseudoClockSem->s_value + 1;/*incremento il valore del semaforo*/
		CAS(&sem_esclusion[MAXPROC],BUSY,FREE);/*setto a free*/




	}else if (CAUSE_IP_GET(cause,INT_TERMINAL)) { /* terminal interrupt */
		/*accesso al bitmap degli interrupt dalla riga dei terminali*/
		unsigned int * interruptingDevBitMapSet = lineTerminal;
		/*maschera di controllo*/
		unsigned int mask = 0x1;
		/*flag di controllo se diventa false vuol dire che non sono presenti interrupt*/
		unsigned char flag = TRUE;
		int i = 0;
		int * readCmd,transCmd;
		readCmd    = (int *) commandReg_R(i);
		transCmd    = (int *) commandReg_T(i);



		do{
			if((*interruptingDevBitMapSet & mask) != 0 ){
				/*ritorna il valore del contatore*/

				/*setta flag a false*/
				flag= FALSE;
			}
			else{
				i++;
			}


		}while(flag);

		/*lettura del carattere*/
		if(statusReg_R(i) == DEV_TTRS_S_CHARTRSM ){
			/*indice del semaforo*/
			int term_r;
			term_r = TERMINAL_SEM_R(i);
			/*puntatore al semaforo*/
			semd_t *s = getSemd(term_r);
			V(s,term_r);
			/*DEV_C_ACK*/
			(*readCmd)= DEV_C_ACK;

		}
		/*trasmissione carattere*/
		/* Se è un carattere trasmesso */
		else if(statusReg_T(i) == DEV_TTRS_S_TRSMERR)
		{
			/*indice del semaforo*/
			int term_t;
			term_t = TERMINAL_SEM_W(i);
			/*puntatore al semaforo*/
			semd_t *s = getSemd(term_t);
			V(s,term_t);
			/*DEV_C_ACK*/
			(*transCmd) = DEV_C_ACK ;
		}


	}
	else{
		/*incrementiamo il tempo della cpu*/
		current->cpu_time = current->cpu_time + SCHED_TIME_SLICE;
		/*rimettiamo in coda ready*/
		enqueueProcess(current,cpu);/*metto in readyq il processo*/
	}

	restartScheduler();

}
