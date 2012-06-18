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
#include "interrupts.h"


extern state_t new_old_areas[MAX_CPU][8];

unsigned int sem_esclusion[MAXPROC]; //tutti inizializzati e free
//le chiamate ai semafori, P e V, devono essere eseguite senza essere interrompibili ed in mutua esclusione.
//quindi prima di iniziare un'operazione su semaforo, bisogna che nessun'altra cpu stia facendo quella operazione
//su quel semaforo. Ho quindi una variabile globale per ogni semaforo, che mi indica se sia libero o occupato

void P(semd_t *s, int key, pcb_t *caller) {
	while(!CAS(&sem_esclusion[key], FREE, BUSY)) ; //aspetto fino a quando si è liberato il semaforo

	s->s_value = (s->s_value)--;
	if (s->s_value <= 0) {

		//se il semaforo ha valore 0 o minore, il processo si blocca
		//devo estrarlo dalla coda ready e metterlo in quella del seamforo;
		caller->p_semkey = key;
		insertBlocked(key, caller);
		//devo inserire adesso questo semaforo nella asl
	}

	CAS(&sem_esclusion[key], BUSY, FREE);
	return;

}



void V(semd_t *s, int key) {
	while(!CAS(&sem_esclusion[key], FREE, BUSY)) ;
	s->s_value = (s->s_value)++;
	int cpu = getPRID();

	//se il semaforo ha dei processi bloccati in coda
	if(!(emptyProcQ(&s->s_procQ))) {
		pcb_t* wake_pcb = removeBlocked(key);
		enqueueProcess(wake_pcb, cpu);
	}
	else {/*togliere il semaforo dalla ASL*/}
	CAS(&sem_esclusion[key], BUSY, FREE);
}

int passerenIO(semd_t *sem_dev) {

}

void sysHandler() {

	int cpu = getPRID(),cause = getCAUSE();
	pcb_t* current = getRunningProcess(cpu);
	copyState(((state_t *)SYSBK_OLDAREA),(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE; //incremento il chiamante della syscall di un'istruzione

	//se è una system call, quindi EXC_CODE == 8
	if ((CAUSE_EXCCODE_GET(cause)) == EXC_SYSCALL) {

		//prendo lo state del padre prima della chiamata
		//devo leggere nei registri dello state_t del padre per estrarre i vari valori del figlio


		switch (current->p_s.reg_a0) {

		//CREATEPROCESS
		case 1: {

			state_t* son_state = (state_t*) current->p_s.reg_a1; //indirizzo dello state_t dato del processo figlio

			//CAS
			pcb_t* son = allocPcb(); //estraggo nuovo pcb per il processo figlio

			//non posso più creare processi, ho finito tutti i pcb disponibili
			if (son == NULL) {
				current->p_s.reg_v0 = -1;
				return;
			}

			son->priority = current->p_s.reg_a2; //priorità del processo figlio
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
			insertChild(current, son);
			assignProcess(son);

			//rimetto anche il pcb del padre nella coda ready
			//e gli restituisco il valore

			current->p_s.reg_v0 = 0;
			enqueueProcess(current, cpu);
			restartScheduler();

			 }
		break;



		//CREATEBROTHER
		case 2: {
			state_t* brother_state = (state_t*) current->p_s.reg_a1;

			//CAS
			pcb_t* brother = allocPcb(); //estraggo nuovo pcb per il processo fratello

			//non posso più creare processi, ho finito tutti i pcb disponibili
			if (brother == NULL) {
				current->p_s.reg_v0 = -1;
				return;
			}

			brother->priority = current->p_s.reg_a2; //priorità del processo figlio
			// copio nel struttura state_t del pc, tutti i campi dello state_t del processo figlio che ci viene dato
			brother->p_s.pc_epc = brother_state ->pc_epc;
			brother->p_s.entry_hi = brother_state ->entry_hi;
			brother->p_s.cause = brother_state -> cause;
			brother->p_s.status = brother_state -> status;
			brother->p_s.hi = brother_state -> hi;
			brother->p_s.lo = brother_state -> lo;
			int i;
			for(i = 0; i < 29; i++) {
				brother->p_s.gpr[i] = brother_state->gpr[i];
			}

			//adesso inserisco tale nuovo pc formato come figlio del padre del pcb chiamante.
			//quindi come fratello del chiamante
			//inserico il nuovo processo in una coda ready
			insertChild(current->p_parent, brother);
			assignProcess(brother);

			current->p_s.reg_v0 = 0;
			enqueueProcess(current, cpu);
			restartScheduler();

		} break;





		//TERMINATEPROCESS
		case 3: {

		} break;


		//PASSEREN
		case 5:	{
			int key = current->p_s.reg_a1; //ottengo la key del semaforo
			semd_t* sem = getSemd(key);
			P(sem, key, current);
			restartScheduler();
		} break;



		//VERHOGEN
		case 4: {
			int key = current->p_s.reg_a1; //ottengo la key del semaforo
			semd_t* sem = getSemd(key); //ottengo puntatore al semaforo di quella key
			V(sem, key);
			restartScheduler();
		} break;



		//GETCPUTIME
		case 6: {
			unsigned int rest = getTIMER(); //controllo a quanto è arrivato il timer ora

			//per capire quanti microsecondi ha usato il processo
			//sottraggo il tempo rimasto dal timeslice totale
			unsigned int used = SCHED_TIME_SLICE - rest;

			current->p_s.reg_v0 = used;
			restartScheduler();
		} break;


		//WAITCLOCK
		case 7: {
			//nel main abbiamo associato il semaforo di key 0 allo pseudo_clock_timer
			//ed abbiamo chiamato il semaoforo psClock_timer
			P(psClock_timer, 0, current);
			//soft_block_counter++;
			restartScheduler();
		} break;



		//WAITIO
		case 8: {
/*
			//abbiamo una sola linea di device, i terminali che sono la 7.
			//se invocate altrove, PANIC();
			if(current->p_s.reg_a1 == 7) {

				//se è in lettura
				if(current->p_s.reg_a3) {
					current->p_s.reg_v0 = passerenIO(terminalSem[current->p_s.reg_a2][1]);
				}

				else {
					current->p_s.reg_v0 = passerenIO(terminalSem[current->p_s.reg_a2][0]);
				}

			}

			else {PANIC();}*/

		} break;


		/* Leggendo la guida Kaya sembrerebbe che vada caricato un nuovo gestore per quell'eccezzione
		 * relativo ad un processo. Cioè per quel pcb. In modo che se da quel momento in avanti quel pcb scatena
		 * un'eccezzione venga caricato il nuovo gestore e non quello standard. Tuttavia nella
		 * struttura dati pcb_t non c'è nessun campo per inserire nuovi gestori (a differenza dell'anno scorso).
		 * quindi bisogna sostituire i gestori originali proprio nelle new area delle cpu? */


		//SPECPRGVEC
		case 9: {


		} break;




		//SPECTLBVEC
		case 10: {


		} break;




		//SPECSYSVECT
		case 11: {
		

		} break;




		} //fine switch
	} //fine if((CAUSE_EXCCODE_GET(cause)) == EXC_SYSCALL)
	/* Ramo else: Breakpoint. Dovrebbe essere l'unica alternativa. */
	else if ((CAUSE_EXCCODE_GET(cause)) == EXC_BREAKPOINT) {

		copyState(((state_t*)SYSBK_OLDAREA),(&current->p_s));
		current->p_s.pc_epc += WORD_SIZE;

		enqueueProcess(current,cpu);

		restartScheduler();

	}
	return;
}
