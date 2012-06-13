/*
 * syscalls.c
 *
 *  Created on: 12/giu/2012
 *      Author: kira
 */
#include "libumps.h"
#include "types11.h"
#include "asl.e"
#include "pcb.e"

#include "sysvars.h"
#include "scheduler.h"

#include "copy.h"

#define FREE 1
#define BUSY 0

extern state_t new_old_areas[MAX_CPU][8];

unsigned int sem_esclusion[MAXPROC]; //tutti inizializzati e free
//le chiamate ai semafori, P e V, devono essere eseguite senza essere interrompibili ed in mutua esclusione.
//quindi prima di iniziare un'operazione su semaforo, bisogna che nessun'altra cpu stia facendo quella operazione
//su quel semaforo. Ho quindi una variabile globale per ogni semaforo, che mi indica se sia libero o occupato

void sysHandler() {

	int cpu = getPRID(),cause = getCAUSE();

	//le system calls differiscono dal contenuto dei breakpoints a seconda del valore della
	//parte di registro cause detta ExcCode. Quindi ci sono due punti possibili: o è una system call,
	//oppure un break point

	/* Con STST ottengo lo state_t del processo che ha scatenato la sys call e devo leggerne i registri cause,
	a0 (numero syscall), a1 a2 e a3 per i parametri. A seconda di quanto trovato nel registro a0, faccio
	lo switch del numero della sys call.*/

	//se è una system call, quindi EXC_CODE == 8
	if ((CAUSE_EXCCODE_GET(cause)) == EXC_SYSCALL) {

		state_t actual;
		STST(&actual); //estraggo lo stato del processo chiamante

		switch (actual.reg_a0) {

		//CREATEPROCESS
		case 1: {

			state_t* son_state = &actual; //indirizzo dello state_t dato del processo figlio

			//CAS
			pcb_t* son = allocPcb(); //estraggo nuovo pcb per il processo figlio

			//non posso più creare processi, ho finito tutti i pcb disponibili
			if (son == NULL) {
				actual.reg_v0 = -1;
				return;
			}

			son->priority = actual.reg_a2; //priorità del processo figlio
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

			insertChild(running[cpu], son);
			//adesso da qui devo inserire il pcb appena creato in una coda ready dei processori.
			//le esaminiamo tutte e scegliamo di inserirlo nella coda a lunghezza più corta

			//CAS
			/* process_counter++ */
			actual.reg_v0 = 0; }
		break;

		//PASSEREN
		case 5:	{

			int key = actual.reg_a1; //ottengo la key del semaforo
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
			int key = actual.reg_a1; //ottengo la key del semaforo
			semd_t* sem = getSemd(key); //ottengo puntatore al semaforo di quella key

			while(!CAS(&sem_esclusion[key], FREE, BUSY)) ;
			sem->s_value = (sem->s_value)++;

			//se il semaforo ha dei processi bloccati in coda
			if(!(emptyProcQ(&sem->s_procQ))) {
				pcb_t* wake_pcb = removeBlocked(key);
				//inserisci pcb in una ready queue, quella di lunghezza minore
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
