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
#define termIntLine 7
//#define devAddrBase(i) 0x10000050 + ((IntlineNo -3 )* 0x80)+(i *0x10)
//#define statusReg_R(i) devAddrBase(i) + RECV_STATE
//#define statusReg_T(i) devAddrBase(i) + TRANSM_STATE
//#define commandReg_R(i) devAddrBase(i) + RECV_CMD
//#define commandReg_T(i) devAddrBase(i) + TRANSM_CMD
#define lineTerminal 0x1000004C

//PENDING_BITMAP_START + (WORD_SIZE * (IntlineNo - 3))
extern state_t new_old_areas[16][8];
extern unsigned int sem_esclusion[NUMSEM];

unsigned char *devAddrBase(unsigned int i)
{
	return (unsigned char *) (0x10000050 + ((termIntLine - 3) * 0x80) + (i * 0x10));
}

unsigned char *commandReg_R(unsigned int i)
{
	return (unsigned char *) (devAddrBase(i) + RECV_CMD);
}

unsigned char *commandReg_T(unsigned int i)
{
	return (unsigned char *) (devAddrBase(i) + TRANSM_CMD);
}

unsigned char *statusReg_R(unsigned int i)
{
	return (unsigned char *) (((unsigned char *) devAddrBase(i)) + RECV_STATE);
}

unsigned char *statusReg_T(unsigned int i)
{
	return (unsigned char *)  devAddrBase(i) + TRANSM_STATE;
}

void tester(void)
{

}

void tester2(void)
{

}

void tester3(void)
{

}

void intHandler() {

	int cpu = getPRID(),cause = getCAUSE();

	pcb_t* current = getRunningProcess(cpu);

	if (cpu > 0)
		copyState((&new_old_areas[cpu][1]),(&current->p_s));
	else
		copyState(((state_t*)INT_OLDAREA),(&current->p_s));

	enqueueProcess(current,cpu);

	//tester();
	/* Se è presente un processo sulla CPU, salviamo il suo stato */
	if(current != NULL)
		copyState(((state_t*)INT_OLDAREA),(&current->p_s));

	//tester();

	/* Linea 2 Interval Timer Interrupt + Gestione PSEUDO CLOCK ****************************/
	if (CAUSE_IP_GET(cause, INT_TIMER)) {

		//	tester2();

		/*puntatore al semaforo*/
		semd_t *pseudoClockSem = getSemd(MAXPROC);
		while(!CAS(&sem_esclusion[MAXPROC],FREE,BUSY)) ;/*appena trova free lo metto a busy*/
		/*se la coda non è vuota li risvegliamo*/
		while(!(emptyProcQ(&pseudoClockSem->s_procQ))) {
			pcb_t* pWake = removeBlocked(MAXPROC);
			enqueueProcess(pWake,cpu);/*metto in readyq il processo*/
			pseudoClockSem->s_value = pseudoClockSem->s_value + 1;/*incremento il valore del semaforo*/
		}
		pseudoClockSem->s_value = pseudoClockSem->s_value + 1;/*incremento il valore del semaforo*/
		CAS(&sem_esclusion[MAXPROC],BUSY,FREE);/*setto a free*/




	}else if (CAUSE_IP_GET(cause,INT_TERMINAL)) { /* terminal interrupt */


		unsigned int  *interruptingDevBitMapSet = lineTerminal;
		/*maschera di controllo*/
		unsigned int mask = 0x1;
		/*flag di controllo se diventa false vuol dire che non sono presenti interrupt*/
		unsigned char flag = FALSE;

		int i = 0;
		unsigned int *readCmd  = (unsigned int *) commandReg_R(i);
		unsigned int *transCmd = (unsigned int *) commandReg_T(i);
		int ack = 0;

		tester();


		do{

			/*controllo per fare in modo che se ha finito gli 8 terminali non esegua più*/
			if(i < 8){

				if((*interruptingDevBitMapSet & mask) == 0 ){
					/*setta flag a false*/
					flag= TRUE;
				}
				else{

					i++;
					mask = mask << 1; /* Oppure mask <<=1  bit shifting*/
					mask = mask << 1;


				}

			}

		}while(flag);



		/*lettura del carattere*/
		if((*(statusReg_R(i)) == DEV_TRCV_S_CHARRECV) || (*(statusReg_R(i)) == DEV_TRCV_BUSY)){

			//tester3();
			/*indice del semaforo*///addokbuf("ciao");
			int term_r;
			term_r = TERMINAL_SEM_R(i);

			/*puntatore al semaforo*/
			semd_t *s = getSemd(term_r);

			V(s,term_r);

			/*DEV_C_ACK*/
			*readCmd= DEV_C_ACK;
		}
		/*trasmissione carattere*/
		/* Se è un carattere trasmesso */

		else if((*(statusReg_T(i))  == DEV_TTRS_S_CHARTRSM) || (*(statusReg_T(i))  == DEV_TTRS_BUSY))
		{
			/*indice del semaforo*/
			//tester2();

			int term_t;
			term_t = TERMINAL_SEM_W(i);
			/*puntatore al semaforo*/
			semd_t *s = getSemd(term_t);


			V(s,term_t);

			/*DEV_C_ACK*/
			*transCmd = DEV_C_ACK;

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
