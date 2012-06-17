/*
 * traps.c
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#include "types11.h"

#include "copy.h"
#include "sysvars.h"
#include "scheduler.h"

extern state_t new_old_areas[MAX_CPU][8];

void trapHandler() {
	int cause_exCode, KoUMode;

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(((state_t*)PGMTRAP_OLDAREA),(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/*TODO: chiedo il controllo su questa cosa perchè non sono propriamente sicura che sia la giusta area di memoria a cui puntare.*/
	/*Recupero il bit di modalità della sysBp Old Area (controllo se Kernel o User mode)*/
	/*KoUMode = ((sysBp_old->status) & STATUS_KUp) >> 0x3;*/

	/*Recupero il codice del tipo di eccezione scatetanata*/
	cause_exCode = CAUSE_EXCCODE_GET(sysBp_old->cause);
	
	/*Carico il processo nella coda dei processi ready*/
	enqueueProcess(current,cpu);

	/*Riavvio lo scheduler*/
	restartScheduler();

}

void tlbHandler() {

}

