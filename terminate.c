/*
 * terminate.c
 *
 *  Created on: 19/giu/2012
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

/* Se il pcb da uccidere ha dei figli,abbiamo tre casi:
 * - Se un figlio è in esecuzione du una CPU --> INIT_CPU e fare ripartire lo scheduler
 * - Se un figlio sta in una coda ready, rimuoverlo e fare freePcb
 * - Se un figlio sta nel semaforo, rimuoverlo dal semaforo ed incrementarne il valore*/

void terminateprocess(pcb_t * terproc, int idc) {

	pcb_t* nextkill = NULL; //prossimo pcb da essere killato, nel caso di progenie
	int i;

	if(!emptyProcQ(terproc->p_child)) {

		//finchè ci sono figli
		while(emptyChild(terproc)) {

			//estraggo primo figlio
			nextkill = removeChild(terproc);

			//punto 1: se il figlio è bloccato su semaforo
			if(nextkill->p_semkey != -1) {
				/*  solito problema, ottengo puntatore al semaforo. Lo tolgo dalla sua
				 * coda, ne incremento il valore. getSemd()?????*/
				freePcb(nextkill);
				continue;
			}



			//controlliamo se è in esecuzione su tutti i processori, tranne quello del chiamante
			for(i=0;i<=NCPUs;i++) {
				if(i != idc) {
					pcb_t * compare = getRunningProcess(i);
					//se dentro compare c'è nextkill, significa che un pcb figlio è in esecuzione
					//su quel processore i
					if (compare == nextkill) {
						INITCPU(i,&scheduler,new_old_areas[i]);
						freePcb(nextkill);
					}

				}
				break;
			}

			continue;
		}

	}

	freePcb(terproc);

	return;

}
