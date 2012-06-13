/*
 * syscalls.c
 *
 *  Created on: 12/giu/2012
 *      Author: kira
 */
#include "libumps.h"
#include "const.h"
#include "base.h"
#include "uMPStypes.h"
#include "asl.e"
#include "pcb.e"

void sysHandler() {

	int cause = getCAUSE();

	//le system calls differiscono dal contenuto dei breakpoints a seconda del valore della
	//parte di registro cause detta ExcCode. Quindi ci sono due punti possibili: o è una system call,
	//oppure un break point

	/* Con STST ottengo lo state_t del processo che ha scatenato la sys call e devo leggerne i registri cause,
	a0 (numero syscall), a1 a2 e a3 per i parametri. A seconda di quanto trovato nel registro a0, faccio
	lo switch del numero della sys call.*/

	//se è una system call, quindi EXC_CODE == 8
	if((CAUSE_EXCCODE_GET(cause)) == EXC_SYSCALL) {

		state_t actual;
		STST(&actual); //estraggo lo stato del processo attuale
		switch(actual.reg_a0) {
			case 0x1:
				state_t *son_state = actual.reg_a1; //indirizzo dello state_t dato del processo figlio
					 pcb_t* son = allocPcb(); //estraggo nuovo pcb per il processo figlio
					 son->priority = actual.reg_a2; //priorità del processo figlio
					 // copio nel struttura state_t del pc, tutti i campi dello state_t del processo figlio che ci viene dato
					 son->p_s.pc_epc = son_state ->pc_epc;
					 son->p_s.entry_hi = son_state ->entry_hi;
					 son->p_s.cause = son_state -> cause;
					 son->p_s.status = son_state -> status;
					 son->p_s.hi = son_state -> hi;
					 son->p_s.lo = son_state -> lo;
					 int i;
					 for(i=0;i<=30;i++) {
						 son->p_s.gpr[i] = son_state->gpr[i];
					 }
					 //adesso inserisco tale nuovo pc formato come figlio del pcb padre chiamante. Ma come lo ottengo quest'ultimo?
					 insertChild(NULL, son);
					 //adesso da qui devo inserire il pcb appena creato in una coda ready dei processori.
					 //le esaminiamo tutte e scegliamo di inserirlo nella coda a lunghezza più corta
					 break;
		}
	}
}
