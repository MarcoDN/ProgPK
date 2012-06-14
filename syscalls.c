/*
 * syscalls.c
 *
 *  Created on: 12/giu/2012
 *      Author: kira
 */
#include "asl.e"
#include "base.h"
#include "const.h"
#include "const11.h"
#include "copy.h"
#include "libumps.h"
#include "pcb.e"
#include "sysvars.h"
#include "types11.h"
#include "uMPStypes.h"
#include "scheduler.h"

#define FREE 1
#define BUSY 0

extern state_t new_old_areas[MAX_CPU][8];

unsigned int sem_esclusion[MAXPROC]; //tutti inizializzati e free
//le chiamate ai semafori, P e V, devono essere eseguite senza essere interrompibili ed in mutua esclusione.
//quindi prima di iniziare un'operazione su semaforo, bisogna che nessun'altra cpu stia facendo quella operazione
//su quel semaforo. Ho quindi una variabile globale per ogni semaforo, che mi indica se sia libero o occupato

void sysHandler() {

	int cpu = getPRID(),cause = getCAUSE();
	copyState(((state_t*)SYSBK_OLDAREA),(&running[cpu]->p_s));

	//se è una system call, quindi EXC_CODE == 8
	if ((CAUSE_EXCCODE_GET(cause)) == EXC_SYSCALL) {

		//prendo lo state del padre prima della chiamata
		//devo leggere nei registri dello state_t del padre per estrarre i vari valori del figlio


		switch (running[cpu]->p_s.reg_a0) {

		//CREATEPROCESS
		case 1: {

			state_t* son_state = (state_t*) running[cpu]->p_s.reg_a1; //indirizzo dello state_t dato del processo figlio

			//CAS
			pcb_t* son = allocPcb(); //estraggo nuovo pcb per il processo figlio

			//non posso più creare processi, ho finito tutti i pcb disponibili
			if (son == NULL) {
				running[cpu]->p_s.reg_v0 = -1;
				return;
			}

			son->priority = running[cpu]->p_s.reg_a2; //priorità del processo figlio
			// copio nel struttura state_t del pc, tutti i campi dello state_t del processo figlio che ci viene dato
			son->p_s.pc_epc = son_state ->pc_epc;
			son->p_s.entry_hi = son_state ->entry_hi;
			son->p_s.cause = son_state -> cause;
			son->p_s.status = son_state -> status;
			son->p_s.hi = son_state -> hi;
			son->p_s.lo = son_state -> lo;
			int i;
			for(i = 0; i < 29; i++) {
				son->p_s.gpr[i] = son_state->gpr[i];
			}

			//adesso inserisco tale nuovo pc formato come figlio del pcb padre chiamante. Ma come lo ottengo quest'ultimo?
			//inserico il nuovo processo in una coda ready
			insertChild(running[cpu], son);
			assignProcess(son);

			//rimetto anche il pcb del padre nella coda ready
			//incremento il suo program counter
			//e gli restituisco il valore
			running[cpu]->p_s.pc_epc += WORD_SIZE;
			running[cpu]->p_s.reg_v0 = 0;
			assignProcess(running[cpu]);


			//CAS
			/* process_counter++ */

			 }
		break;

		//PASSEREN
		case 5:	{

			int key = running[cpu]->p_s.reg_a1; //ottengo la key del semaforo
			semd_t* sem = getSemd(key);

			while(!CAS(&sem_esclusion[key], FREE, BUSY)) ; //aspetto fino a quando si è liberato il semaforo
			sem->s_value = (sem->s_value)--;
			if (sem->s_value <= 0) {

				//se il semaforo ha valore 0 o minore, il processo si blocca
				//devo estrarlo dalla coda ready e metterlo in quella del seamforo;
				pcb_t *caller = running[cpu];
				caller->p_semkey = key;
				outProcQ(&readyQ[cpu],caller);
				insertBlocked(key, caller);
				//devo inserire adesso questo semaforo nella asl


			}

			CAS(&sem_esclusion[key], BUSY, FREE);

		} break;





		//VERHOGEN
		case 4: {
			int key = running[cpu]->p_s.reg_a1; //ottengo la key del semaforo
			semd_t* sem = getSemd(key); //ottengo puntatore al semaforo di quella key

			while(!CAS(&sem_esclusion[key], FREE, BUSY)) ;
			sem->s_value = (sem->s_value)++;

			//se il semaforo ha dei processi bloccati in coda
			if(!(emptyProcQ(&sem->s_procQ))) {
				pcb_t* wake_pcb = removeBlocked(key);
				assignProcess(wake_pcb);
			}
			else {/*togliere il semaforo dalla ASL*/}
			CAS(&sem_esclusion[key], BUSY, FREE);
		} break;



		} //fine switch
	} //fine if((CAUSE_EXCCODE_GET(cause)) == EXC_SYSCALL)
	/* Ramo else: Breakpoint. Dovrebbe essere l'unica alternativa. */
	else if ((CAUSE_EXCCODE_GET(cause)) == EXC_BREAKPOINT) {

		copyState(((state_t*)SYSBK_OLDAREA),(&running[cpu]->p_s));
		running[cpu]->p_s.pc_epc += WORD_SIZE;

		if (cpu==0)
			LDST(&scheduler);
		else
			INITCPU(cpu,&scheduler,new_old_areas[cpu]);

	}
	return;
}
