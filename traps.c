/*
 * traps.c
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#include "types11.h"
#include "const.h"
#include "copy.h"
#include "sysvars.h"
#include "scheduler.h"

/*Macro per accesso alla OldArea come stato del processore*/
#define progtrap_oldarea	((state_t *) PGMTRAP_OLDAREA)
#define sysbp_oldarea  		((state_t *) SYSBK_OLDAREA)
#define tbltrap_oldarea		((state_t *) TBL_OLDAREA)

extern state_t new_old_areas[MAX_CPU][8];

void trapHandler() {
	int cause_exCode, KoUMode;

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(((state_t*)PGMTRAP_OLDAREA),(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/*Recupero il bit di modalità della sysBp Old Area (controllo se Kernel o User mode)*/
	KoUMode = ((sysbp_oldarea->status) & STATUS_KUp) >> 3;

	/*Recupero il codice del tipo di eccezione scatetanata*/
	cause_exCode = CAUSE_EXCCODE_GET(sysbp_oldarea->cause);

	/*Carico il processo nella coda dei processi ready*/
	enqueueProcess(current,cpu);

	/*Riavvio lo scheduler*/
	restartScheduler();

}

void tlbHandler() {

}

